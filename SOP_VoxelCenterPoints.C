#include "SOP_VoxelCenterPoints.h"

#include <UT/UT_DSOVersion.h>
#include <UT/UT_Interrupt.h>
#include <OP/OP_Operator.h>
#include <OP/OP_OperatorTable.h>
#include <PRM/PRM_Include.h>
#include <GEO/GEO_Detail.h>
#include <GEO/GEO_PrimPoly.h>
#include <GU/GU_Detail.h>
#include <GU/GU_PrimVolume.h>
#include <CH/CH_Manager.h>

#define SOP_VOXELCENTERPOINTS_DEFAULT_ATTRIBUTE_NAME "vsp_value"
#define SOP_VOXELCENTERPOINTS_MAINTAIN_VOLUME_TRANSFORM "vsp_maintainvolumetransform"
#define SOP_VOXELCENTERPOINTS_CREATE_POINT_VALUE_ATTRIBUTE "vsp_createpointvalueattribute"
#define SOP_VOXELCENTERPOINTS_OVERRIDE_THE_ATTRIBUTE_NAME "vsp_overridetheattributename"
#define SOP_VOXELCENTERPOINTS_OVERRIDE_THE_ATTRIBUTE_NAME_VALUE "vsp_overridetheattributenamevalue"

static PRM_Name s_name_maintain_volume_transform(SOP_VOXELCENTERPOINTS_MAINTAIN_VOLUME_TRANSFORM, "Maintain Volume Transform");
static PRM_Name s_name_create_point_value_attribute(SOP_VOXELCENTERPOINTS_CREATE_POINT_VALUE_ATTRIBUTE, "Create Point Value Attribute");
static PRM_Name s_name_override_the_attribute_name(SOP_VOXELCENTERPOINTS_OVERRIDE_THE_ATTRIBUTE_NAME, "Override the Attribute Name");
static PRM_Name s_name_override_the_attribute_name_value(SOP_VOXELCENTERPOINTS_OVERRIDE_THE_ATTRIBUTE_NAME_VALUE, "Attribute Name");

static PRM_Default s_default_maintain_volume_transform(true);
static PRM_Default s_default_create_point_value_attribute(true);
static PRM_Default s_default_override_the_attribute_name(false);
static PRM_Default s_default_override_the_attribute_name_value(0.0f, SOP_VOXELCENTERPOINTS_DEFAULT_ATTRIBUTE_NAME);


PRM_Template
SOP_VoxelCenterPoints::myTemplateList[] = {
    PRM_Template(PRM_TOGGLE, 1, &s_name_maintain_volume_transform, &s_default_maintain_volume_transform),
    PRM_Template(PRM_TOGGLE, 1, &s_name_create_point_value_attribute, &s_default_create_point_value_attribute),
    PRM_Template(PRM_TOGGLE, 1, &s_name_override_the_attribute_name, &s_default_override_the_attribute_name),
    PRM_Template(PRM_STRING, 1, &s_name_override_the_attribute_name_value, &s_default_override_the_attribute_name_value),
    PRM_Template()
};


OP_Node*
SOP_VoxelCenterPoints::myConstructor(OP_Network* network, const char* name, OP_Operator* op)
{
    return new SOP_VoxelCenterPoints(network, name, op);
}


SOP_VoxelCenterPoints::SOP_VoxelCenterPoints(OP_Network* network, const char* name, OP_Operator* op) :
    SOP_Node(network, name, op)
{

}


SOP_VoxelCenterPoints::~SOP_VoxelCenterPoints()
{

}


bool
SOP_VoxelCenterPoints::updateParmsFlags()
{
    bool changed = SOP_Node::updateParmsFlags();
    bool override_attribute_name = overrideAttributeName(CHgetEvalTime());

    changed |= enableParm(SOP_VOXELCENTERPOINTS_OVERRIDE_THE_ATTRIBUTE_NAME_VALUE, override_attribute_name);
    return changed;
}


OP_ERROR
SOP_VoxelCenterPoints::cookMySop(OP_Context& context)
{
    fpreal t = context.getTime();
    UT_Interrupt* boss = UTgetInterrupt();

    if(lockInputs(context) >= UT_ERROR_ABORT)
    {
        return error();
    }

    const GU_Detail* input_gdp = inputGeo(0);
    const GA_PrimitiveGroup* prim_group = nullptr;
    GEO_Primitive* prim = nullptr;

    UT_Array<GEO_PrimVolume*> volumes;

    GA_FOR_ALL_OPT_GROUP_PRIMITIVES(input_gdp, prim_group, prim)
    {
        if(boss->opInterrupt())
        {
            break;
        }

        if(prim->getTypeId() == GEO_PRIMVOLUME)
        {
            GEO_PrimVolume* volume = (GEO_PrimVolume *) prim;
            volumes.append(volume);
        }
    }

    if(volumes.entries() > 0)
    {
        gdp->clearAndDestroy();
        processVolumes(input_gdp, volumes, t);
    }
    else
    {
        UT_WorkBuffer buf;
        buf.sprintf("No input volumes found.");
        addError(SOP_MESSAGE, buf.buffer());
    }

    unlockInputs();
    return error();
}


const char*
SOP_VoxelCenterPoints::inputLabel(unsigned int idx) const
{
    return "Voxels to Center Points";
}


void
SOP_VoxelCenterPoints::processVolumes(const GU_Detail* input_detail, const UT_Array<GEO_PrimVolume*>& volumes, fpreal t)
{
    bool keep_transform = maintainVolumeTransform(t);
    bool create_attribute = createPointValueAttribute(t);
    bool override_attribute_name = overrideAttributeName(t);

    GEO_PrimVolume* first_volume = volumes(0);
    UT_VoxelArrayReadHandleF volume_handle = first_volume->getVoxelHandle();
    UT_VoxelArrayF* volume_data = (UT_VoxelArrayF*) &*volume_handle;

    GA_ROHandleS attr_first_volume_name = GA_ROHandleS(input_detail->findPrimitiveAttribute("name"));
    UT_String first_volume_name;
    if(attr_first_volume_name.isValid())
    {
        first_volume_name = attr_first_volume_name.get(first_volume->getMapOffset());
    }

    if(override_attribute_name)
    {
        UT_String override_name;
        evalString(override_name, SOP_VOXELCENTERPOINTS_OVERRIDE_THE_ATTRIBUTE_NAME_VALUE, 0, t);

        if(!override_name || override_name.length() == 0)
        {
            if(!first_volume_name)
            {
                first_volume_name = SOP_VOXELCENTERPOINTS_DEFAULT_ATTRIBUTE_NAME;
            }
        }
        else
        {
            first_volume_name = override_name;
        }
    }

    if(!first_volume_name || !first_volume_name.isValidVariableName())
    {
        first_volume_name = SOP_VOXELCENTERPOINTS_DEFAULT_ATTRIBUTE_NAME;
    }

    UT_Matrix3 volume_transform = first_volume->getTransform();
    UT_Vector3F volume_scales(1.0f, 1.0f, 1.0f);
    volume_transform.extractScales(volume_scales);

    UT_Vector3 voxel_size(1.0f, 1.0f, 1.0f);
    if(keep_transform)
    {
        voxel_size = first_volume->getVoxelSize();
    }

    UT_Vector3 voxel_half_size(voxel_size / 2.0f);

    if(voxel_size.x() < SYS_FTOLERANCE ||
        voxel_size.y() < SYS_FTOLERANCE ||
        voxel_size.z() < SYS_FTOLERANCE)
    {
        addWarning(SOP_MESSAGE, "Zero voxel size detected.");
        return;
    }

    UT_Vector3 volume_size(volume_data->getXRes(), volume_data->getYRes(), volume_data->getZRes());
    UT_Vector3 volume_corner(-volume_size * voxel_half_size);

    GA_RWHandleF attr_point_value;
    if(create_attribute)
    {
        attr_point_value = GA_RWHandleF(gdp->findFloatTuple(GA_ATTRIB_POINT, first_volume_name, 1));
        if(!attr_point_value.isValid())
        {
            attr_point_value.bind(gdp->addFloatTuple(GA_ATTRIB_POINT, first_volume_name, 1));
        }
    }

    for(int idx_z = 0; idx_z < volume_size.z(); ++idx_z)
    {
        for(int idx_y = 0; idx_y < volume_size.y(); ++idx_y)
        {
            for(int idx_x = 0; idx_x < volume_size.x(); ++idx_x)
            {
                float value = volume_handle->getValue(idx_x, idx_y, idx_z);
                if(value > 0.0f)
                {
                    UT_Vector3 point_data(
                        volume_corner.x() + idx_x * voxel_size.x() + voxel_half_size.x(),
                        volume_corner.y() + idx_y * voxel_size.y() + voxel_half_size.y(),
                        volume_corner.z() + idx_z * voxel_size.z() + voxel_half_size.z());

                    if(keep_transform)
                    {
                        point_data *= volume_transform;
                    }

                    GA_Offset point_offset = gdp->appendPointOffset();
                    gdp->setPos3(point_offset, point_data);

                    if(create_attribute && attr_point_value.isValid())
                    {
                        attr_point_value.set(point_offset, value);
                    }
                }
            }
        }
    }
}


bool
SOP_VoxelCenterPoints::maintainVolumeTransform(fpreal t) const
{
    return evalInt(SOP_VOXELCENTERPOINTS_MAINTAIN_VOLUME_TRANSFORM, 0, t);
}


bool
SOP_VoxelCenterPoints::createPointValueAttribute(fpreal t) const
{
    return evalInt(SOP_VOXELCENTERPOINTS_CREATE_POINT_VALUE_ATTRIBUTE, 0, t);
}


bool
SOP_VoxelCenterPoints::overrideAttributeName(fpreal t) const
{
    return evalInt(SOP_VOXELCENTERPOINTS_OVERRIDE_THE_ATTRIBUTE_NAME, 0, t);
}


void
newSopOperator(OP_OperatorTable* table)
{
    table->addOperator(new OP_Operator("voxelcenterpoints", "Voxel Center Points", SOP_VoxelCenterPoints::myConstructor,
        SOP_VoxelCenterPoints::myTemplateList, 1, 1, 0));
}

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


#define SOP_VOXELCENTERPOINTS_MAINTAIN_VOLUME_TRANSFORM "vsp_maintainvolumetransform"
#define SOP_VOXELCENTERPOINTS_CREATE_POINT_VALUE_ATTRIBUTE "vsp_createpointvalueattribute"

static PRM_Name s_name_maintain_volume_transform(SOP_VOXELCENTERPOINTS_MAINTAIN_VOLUME_TRANSFORM, "Maintain Volume Transform");
static PRM_Name s_name_create_point_value_attribute(SOP_VOXELCENTERPOINTS_CREATE_POINT_VALUE_ATTRIBUTE, "Create Point Value Attribute");

static PRM_Default s_default_maintain_volume_transform(true);
static PRM_Default s_default_create_point_value_attribute(true);


PRM_Template
SOP_VoxelCenterPoints::myTemplateList[] = {
    PRM_Template(PRM_TOGGLE, 1, &s_name_maintain_volume_transform, &s_default_maintain_volume_transform),
    PRM_Template(PRM_TOGGLE, 1, &s_name_create_point_value_attribute, &s_default_create_point_value_attribute),
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
        processVolumes(volumes, t);
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
SOP_VoxelCenterPoints::processVolumes(const UT_Array<GEO_PrimVolume*>& volumes, fpreal t)
{
    GEO_PrimVolume* first_volume = volumes(0);
    UT_VoxelArrayReadHandleF volume_handle = first_volume->getVoxelHandle();
    UT_VoxelArrayF* volume_data = (UT_VoxelArrayF*) &*volume_handle;

    bool keep_transform = maintainVolumeTransform(t);
    bool create_attribute = createPointValueAttribute(t);

    UT_Matrix3 volume_transform = first_volume->getTransform();
    UT_Vector3F volume_scales;
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
        return;
    }

    UT_Vector3 volume_size(volume_data->getXRes(), volume_data->getYRes(), volume_data->getZRes());
    UT_Vector3 volume_corner(-volume_size * voxel_half_size);

    for(int idx_z = 0; idx_z < volume_size.z(); ++idx_z)
    {
        for(int idx_y = 0; idx_y < volume_size.y(); ++idx_y)
        {
            for(int idx_x = 0; idx_x < volume_size.x(); ++idx_x)
            {
                float value = (*volume_data)(idx_x, idx_y, idx_z);
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


void
newSopOperator(OP_OperatorTable* table)
{
    table->addOperator(new OP_Operator("voxelcenterpoints", "Voxel Center Points", SOP_VoxelCenterPoints::myConstructor,
        SOP_VoxelCenterPoints::myTemplateList, 1, 1, 0));
}

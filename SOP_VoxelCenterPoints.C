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


PRM_Template
SOP_VoxelCenterPoints::myTemplateList[] = {
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

    //UT_Vector3 volume_size = first_volume->getVoxelSize();

    const UT_Matrix3& volume_transform = first_volume->getTransform();

    UT_Matrix3 volume_transform_temp(volume_transform);
    UT_Vector3F volume_scale;
    volume_transform_temp.extractScales(volume_scale);

    //UT_Matrix3 volume_rotation;
    //volume_transform.extractRotation(volume_rotation);

    int volume_size_x = volume_data->getXRes();
    int volume_size_y = volume_data->getYRes();
    int volume_size_z = volume_data->getZRes();

    float boundary_x = -volume_size_x * volume_scale.x() / 2.0f;
    float boundary_y = -volume_size_y * volume_scale.y() / 2.0f;
    float boundary_z = -volume_size_z * volume_scale.z() / 2.0f;

    float step_x = 0.5f * volume_scale.x();
    float step_y = 0.5f * volume_scale.y();
    float step_z = 0.5f * volume_scale.z();

    for(int idx_z = 0; idx_z < volume_size_z; ++idx_z)
    {
        for(int idx_y = 0; idx_y < volume_size_y; ++idx_y)
        {
            for(int idx_x = 0; idx_x < volume_size_x; ++idx_x)
            {
                float value = (*volume_data)(idx_x, idx_y, idx_z);
                if(value > 0.0f)
                {
                    UT_Vector3 point_data(
                        boundary_x + step_x + idx_x * volume_scale.x(),
                        boundary_y + step_y + idx_y * volume_scale.y(),
                        boundary_z + step_z + idx_z * volume_scale.z());

                    //point_data *= volume_transform_temp;
                    //point_data *= volume_rotation;

                    GA_Offset point_offset = gdp->appendPointOffset();
                    gdp->setPos3(point_offset, point_data);
                }
            }
        }
    }

    //gdp->setTransform(volume_transform);
}


void
newSopOperator(OP_OperatorTable* table)
{
    table->addOperator(new OP_Operator("voxelcenterpoints", "Voxel Center Points", SOP_VoxelCenterPoints::myConstructor,
        SOP_VoxelCenterPoints::myTemplateList, 1, 1, 0));
}

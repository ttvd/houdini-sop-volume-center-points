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
    SOP_Node(network, name, op),
    m_group(nullptr)
{

}


SOP_VoxelCenterPoints::~SOP_VoxelCenterPoints()
{

}


OP_ERROR
SOP_VoxelCenterPoints::cookMySop(OP_Context& context)
{
    if(lockInputs(context) >= UT_ERROR_ABORT)
    {
        return error();
    }

    duplicateSource(0, context);

    if(cookInputGroups(context) >= UT_ERROR_ABORT)
    {
        unlockInputs();
        return error();
    }

    GEO_Primitive* prim = nullptr;
    fpreal t = context.getTime();
    UT_Interrupt* boss = UTgetInterrupt();

    UT_Array<GEO_PrimVolume*> volumes;

    GA_FOR_ALL_OPT_GROUP_PRIMITIVES(gdp, m_group, prim)
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


OP_ERROR
SOP_VoxelCenterPoints::cookInputGroups(OP_Context& context, int alone)
{
    return cookInputPrimitiveGroups(context, m_group, m_detail_group_pair, alone);
}


void
SOP_VoxelCenterPoints::processVolumes(const UT_Array<GEO_PrimVolume*>& volumes, fpreal t)
{
    GEO_PrimVolume* first_volume = volumes(0);
    UT_VoxelArrayReadHandleF volume_handle = first_volume->getVoxelHandle();

    //UT_Vector3 volume_size = first_volume->getVoxelSize();

    int volume_size_x = volume_handle->getXRes();
    int volume_size_y = volume_handle->getYRes();
    int volume_size_z = volume_handle->getZRes();
}


void
newSopOperator(OP_OperatorTable* table)
{
    table->addOperator(new OP_Operator("voxelcenterpoints", "Voxel Center Points", SOP_VoxelCenterPoints::myConstructor,
        SOP_VoxelCenterPoints::myTemplateList, 1, 1, 0));
}

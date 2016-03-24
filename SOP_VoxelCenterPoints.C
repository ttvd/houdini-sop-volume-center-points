#include "SOP_VoxelCenterPoints.h"

#include <UT/UT_DSOVersion.h>
#include <UT/UT_Interrupt.h>
#include <OP/OP_Operator.h>
#include <OP/OP_OperatorTable.h>
#include <PRM/PRM_Include.h>
#include <GEO/GEO_Detail.h>
#include <GEO/GEO_PrimPoly.h>
#include <GU/GU_Detail.h>

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
    if(error() >= UT_ERROR_ABORT)
    {
        return error();
    }

    gdp->clearAndDestroy();

    UT_AutoInterrupt boss("Creating Voxel Center Points");
    if(boss.wasInterrupted())
    {
        return error();
    }

    fpreal t = context.getTime();

    return error();
}


const char*
SOP_VoxelCenterPoints::inputLabel(unsigned int idx) const
{
    return "Voxels to Center Points";
}


void
newSopOperator(OP_OperatorTable* table)
{
    table->addOperator(new OP_Operator("voxelcenterpoints", "Voxel Center Points", SOP_VoxelCenterPoints::myConstructor,
        SOP_VoxelCenterPoints::myTemplateList, 1, 1, 0));
}

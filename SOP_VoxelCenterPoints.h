#pragma once

#include <SOP/SOP_API.h>
#include <SOP/SOP_Node.h>

class GEO_PrimVolume;

class SOP_API SOP_VoxelCenterPoints : public SOP_Node
{
    public:

        static OP_Node* myConstructor(OP_Network* network, const char* name, OP_Operator* op);
        static PRM_Template myTemplateList[];

    public:

        virtual OP_ERROR cookInputGroups(OP_Context& context, int alone = 0);

    protected:

        SOP_VoxelCenterPoints(OP_Network* network, const char* name, OP_Operator* op);
        virtual ~SOP_VoxelCenterPoints();

    protected:

        virtual const char* inputLabel(unsigned int idx) const;
        virtual OP_ERROR cookMySop(OP_Context& context);

    protected:

        void processVolumes(const UT_Array<GEO_PrimVolume*>& volumes, fpreal t);

    private:

        GU_DetailGroupPair m_detail_group_pair;
        const GA_PrimitiveGroup* m_group;
};

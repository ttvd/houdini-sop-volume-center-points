#pragma once

#include <SOP/SOP_API.h>
#include <SOP/SOP_Node.h>

class GEO_PrimVolume;

class SOP_VolumeCenterPoints : public SOP_Node
{
    public:

        static OP_Node* myConstructor(OP_Network* network, const char* name, OP_Operator* op);
        static PRM_Template myTemplateList[];

    public:

        virtual bool updateParmsFlags();

    protected:

        SOP_VolumeCenterPoints(OP_Network* network, const char* name, OP_Operator* op);
        virtual ~SOP_VolumeCenterPoints();

    protected:

        virtual const char* inputLabel(unsigned int idx) const;
        virtual OP_ERROR cookMySop(OP_Context& context);

    protected:

        void processVolumes(const GU_Detail* input_detail, const UT_Array<GEO_PrimVolume*>& volumes, fpreal t);

    protected:

        bool maintainVolumeTransform(fpreal t) const;
        bool createPointValueAttribute(fpreal t) const;
        bool overrideAttributeName(fpreal t) const;
};

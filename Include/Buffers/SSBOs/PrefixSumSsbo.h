#pragma once

#include "Include/Buffers/SSBOs/SsboBase.h"

/*------------------------------------------------------------------------------------------------
Description:
    Encapsulates the SSBO that is used for calculating prefix sums as part of the parallel radix 
    sorting algorithm.

    Note: "Prefix scan", "prefix sum", same thing.
Creator:    John Cox, 3/2017
------------------------------------------------------------------------------------------------*/
class PrefixSumSsbo : public SsboBase
{
public:
    PrefixSumSsbo(unsigned int numDataEntries);
    virtual ~PrefixSumSsbo() = default;
    using SHARED_PTR = std::shared_ptr<PrefixSumSsbo>;

    void ConfigureConstantUniforms(unsigned int computeProgramId) const override;
    unsigned int NumPerGroupPrefixSums() const;
    unsigned int NumDataEntries() const;

private:
    unsigned int _numPerGroupPrefixSums;
    unsigned int _numDataEntries;
};

#pragma once

#include <memory>
#include <string>

#include "Include/SSBOs/SsboBase.h"
#include "Include/SSBOs/PrefixSumSsbo.h"
#include "Include/SSBOs/IntermediateDataSsbo.h"
#include "Include/SSBOs/ParticleSsbo.h"
#include "Include/SSBOs/ParticleCopySsbo.h"

namespace ShaderControllers
{
    /*--------------------------------------------------------------------------------------------
    Description:
        This compute controller is responsible for performing a parallel Radix sort of an SSBO
        according to a structure-specific element.  For example, suppose there is a Particle
        structure with position, velocity, mass, etc.  If I want to sort the particles in 3D 
        space according to a Z-order curve, then I will want to sort the particles by Morton 
        codes.

        Sorting by parallel Radix sort requires going over all the bits in the data to be sorted
        one by one, each time:
            (1) Getting a single bit value
            (2) Performing a parallel prefix scan by work group
            (3) Performing a parallel prefix scan over all the work group sums
            (4) Sorting the data according to the prefix sums.

        If I want to sort the original structures, then I can't just sort by some integer.  I 
        need to associate the data that is being sorted with the original structure.  Enter the
        IntermediateData structure, which stores a uint (data to sort over, such as a
        3D-position-derived Morton code) and the index into the buffer that the structure 
        originally came from.

        This class handles the multiple compute shaders that need to be called at each step of 
        the sorting process.  The sorting process requires knowing how big the original buffer 
        is and exactly which buffer is being sorted, so an instance of this class will only be 
        useful for a single OriginalDataSsbo.  Compute shaders are not as flexible as CPU-bound 
        shaders, so you have to hold their hand, and the consequence is high coupling.

        // TODO: now that I am sorting particles instead of a simple, 1-integer structure like I was in the GpuRadixSort demo, I can sort 1,000,000 particles by position in X milliseconds
        The benefit is that it can sort 1,000,000 structures in less than 6 milliseconds (at 
        least for the OriginalData structures that I'm using in this demo).
    Creator:    John Cox, 3/2017
    --------------------------------------------------------------------------------------------*/
    class ParallelSort
    {
    public:
        ParallelSort(ParticleSsbo::CONST_SHARED_PTR particleSsboToSort);
        ~ParallelSort();

        void SortWithProfiling() const;
        void SortWithoutProfiling() const;

    private:
        unsigned int _computeShaderId;
        int _unifLocSortStage;
        int _unifLocBitNumber;

        // these are unique to this class and are needed for sorting
        ParticleCopySsbo::SHARED_PTR _particleCopySsbo;
        IntermediateDataSsbo::SHARED_PTR _intermediateDataSsbo;
        PrefixSumSsbo::SHARED_PTR _prefixSumSsbo;

        ParticleSsbo::CONST_SHARED_PTR _particleSsboToSort;
    };
}

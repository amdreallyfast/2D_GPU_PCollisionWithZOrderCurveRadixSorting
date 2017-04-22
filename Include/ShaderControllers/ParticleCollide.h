#pragma once

#include "Include/SSBOs/ParticleSsbo.h"

namespace ShaderControllers
{
    /*--------------------------------------------------------------------------------------------
    Description:
        Encapsulates particle collision handling.  

    Creator:    John Cox, ??
    --------------------------------------------------------------------------------------------*/
    class ParticleCollide
    {
    public:
        ParticleCollide(ParticleSsbo::SHARED_PTR &ssboToWorkWith);
        ~ParticleCollide();

        void DetectAndResolveCollisions();

    private:
        unsigned int _totalParticleCount;
        unsigned int _computeProgramId;

        int _unifLocIndexOffsetBy0Or1;
    };
}

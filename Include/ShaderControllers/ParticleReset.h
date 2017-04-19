#pragma once

#include <vector>
#include <memory>

#include "Include/SSBOs/ParticleSsbo.h"
#include "Include/Particles/IParticleEmitter.h"
#include "Include/Particles/ParticleEmitterPoint.h"
#include "Include/Particles/ParticleEmitterBar.h"


namespace ShaderControllers
{
    /*--------------------------------------------------------------------------------------------
    Description:
        Encapsulate particle reseting via compute shader.  Resetting involves taking inactive 
        particles and giving them a new position near a particle emitter plus giving them a new 
        velocity.  
        
        Particles can currently (4-15-2017) be emitted from two types of emitters:
        (1) Point emitters eject particles in all directions
        (2) Bar emitters eject particles outwards from a 2D plane 

        These was deemed different enough to justify splitting the once-one shader into two, one
        for each type of emitter.  This shader controller has the info for both.

        Note: When this class goes "poof", it won't delete the emitter pointers.  The emitters 
        were considered to be external entities.  This compute shader controller only needs to 
        read values from them.  If the user outside this class wants to rotate them or move them 
        at runtime, then they are free to do so.
    Creator:    John Cox, 11-24-2016 (restructured to use composite shaders in 4/2017)
    --------------------------------------------------------------------------------------------*/
    class ParticleReset
    {
    public:
        ParticleReset(const ParticleSsbo::SHARED_PTR &ssboToReset);
        ~ParticleReset();

        bool AddEmitter(const IParticleEmitter::CONST_PTR &pEmitter);

        void ResetParticles(unsigned int particlesPerEmitterPerFrame);

    private:
        unsigned int _totalParticleCount;
        unsigned int _computeProgramIdBarEmitters;
        unsigned int _computeProgramIdPointEmitters;

        // this atomic counter is used to enforce the number of emitted particles per emitter 
        // per frame 
        unsigned int _particleResetAtomicCounterBufferId;

        // some of these uniforms had to be split into two versions to accomodate both shaders

        // specific to point emitter
        int _unifLocPointEmitterCenter;
        int _unifLocPointMaxParticleEmitCount;
        int _unifLocPointMinParticleVelocity;
        int _unifLocPointDeltaParticleVelocity;

        // specific to bar emitter
        int _unifLocBarEmitterP1;
        int _unifLocBarEmitterP2;
        int _unifLocBarEmitterEmitDir;
        int _unifLocBarMaxParticleEmitCount;
        int _unifLocBarMinParticleVelocity;
        int _unifLocBarDeltaParticleVelocity;

        // all the updating heavy lifting goes on in the compute shader, so CPU cache coherency 
        // is not a concern for emitter storage on the CPU side and a std::vector<...> is 
        // acceptable
        // Note: The compute shader has no concept of inheritance.  Rather than store a single 
        // collection of IParticleEmitter pointers and cast them to either point or bar emitters 
        // on every update, just store them separately.
        static const int MAX_EMITTERS = 4;
        std::vector<ParticleEmitterPoint::CONST_PTR> _pointEmitters;
        std::vector<ParticleEmitterBar::CONST_PTR> _barEmitters;
    };
}

#pragma once

#include <vector>
#include <memory>

#include "Include/Particles/IParticleEmitter.h"
#include "Include/Particles/ParticleEmitterPoint.h"
#include "Include/Particles/ParticleEmitterBar.h"


namespace ShaderControllers
{
    /*--------------------------------------------------------------------------------------------
    Description:
        Encapsulate particle reseting via compute shader.  Resetting involves taking inactive 
        particles and giving them a new position near a particle emitter plus giving them a new 
        velocity.  There is one shader that is dedicated to the task of resetting particles, and 
        this shader is set up to summon and communicate with that particular shader.

        Note: This class is not concerned with the particle SSBO.  It is concerned with uniforms 
        and summoning the shader.  SSBO setup is performed in the appropriate SSBO object.

        Note: When this class goes "poof", it won't delete the emitter pointers.  
    Creator:    John Cox, 11-24-2016 (restructured to use composite shaders in 4/2017)
    --------------------------------------------------------------------------------------------*/
    class ParticleReset
    {
    public:
        ParticleReset(unsigned int numParticles, const std::string &computeShaderKey);
        ~ParticleReset();

        bool AddEmitter(std::shared_ptr<const IParticleEmitter> pEmitter);

        void ResetParticles(unsigned int particlesPerEmitterPerFrame);

    private:
        unsigned int _totalParticleCount;
        unsigned int _computeProgramId;

        // this atomic counter is used to enforce the number of emitted particles per emitter 
        // per frame 
        unsigned int _particleResetAtomicCounterBufferId;

        // unlike most OpenGL IDs, uniform locations are GLint
        int _unifLocParticleCount;
        int _unifLocMaxParticleEmitCount;
        int _unifLocMinParticleVelocity;
        int _unifLocDeltaParticleVelocity;
        int _unifLocUsePointEmitter;
        int _unifLocPointEmitterCenter;
        int _unifLocBarEmitterP1;
        int _unifLocBarEmitterP2;
        int _unifLocBarEmitterEmitDir;

        // all the updating heavy lifting goes on in the compute shader, so CPU cache coherency 
        // is not a concern for emitter storage on the CPU side and a std::vector<...> is 
        // acceptable
        // Note: The compute shader has no concept of inheritance.  Rather than store a single 
        // collection of IParticleEmitter pointers and cast them to either point or bar emitters 
        // on every update, just store them separately.
        static const int MAX_EMITTERS = 4;
        std::vector<const ParticleEmitterPoint *> _pointEmitters;
        std::vector<const ParticleEmitterBar *> _barEmitters;
    };
}

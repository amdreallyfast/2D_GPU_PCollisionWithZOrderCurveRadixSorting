#pragma once

#include "Include/Buffers/SSBOs/ParticleSsbo.h"
#include "Include/Buffers/AtomicCounterBuffer.h"

#include "ThirdParty/glm/vec4.hpp"

namespace ShaderControllers
{
    /*--------------------------------------------------------------------------------------------
    Description:
        Encapsulates the following particle updates via compute shader:
        (1) Updates particle positions based on their velocity in the previous frame.
        (2) If any particles have gone out of bounds, flag them as inactive.
        (3) Emit as many particles for this frame as each emitter allows.

        There is one compute shader that does this, and this class is built to communicate with 
        and summon that particular shader.

        Note: This class is not concerned with the particle SSBO.  It is concerned with uniforms 
        and summoning the shader.  SSBO setup is performed in the appropriate SSBO object.
    Creator:    John Cox, 11-24-2016 (restructured to use composite shaders in 4/2017)
    --------------------------------------------------------------------------------------------*/
    class ParticleUpdate
    {
    public:
        ParticleUpdate(const ParticleSsbo::SHARED_PTR &ssboToUpdate, 
            const glm::vec4 &particleRegionCenter,
            float particleRegionRadius);
        ~ParticleUpdate();

        void Update(float deltaTimeSec, std::unique_ptr<PersistentAtomicCounterBuffer> &counter);
        unsigned int NumActiveParticles() const;

    private:
        unsigned int _totalParticleCount;
        unsigned int _activeParticleCount;
        unsigned int _computeProgramId;
        
        // these uniforms are specific to this shader
        int _unifLocParticleRegionCenter;
        int _unifLocParticleRegionRadiusSqr;
        int _unifLocDeltaTimeSec;

        //// the atomic counter is used to count the total number of active particles after this 
        //// update
        //// Also Note: The copy buffer is necessary to avoid trashing OpenGL's beautifully 
        //// synchronized pipeline.  Experiments showed that, after particle updating, mapping a 
        //// pointer to the atomic counter dropped frame rates from ~60fps -> ~3fps.  Ouch.  But 
        //// now I've learned about buffer copying, so now the buffer mapping happens on a buffer 
        //// that is not part of the compute shader's pipeline, and frame rates are back up to 
        //// ~60fps.  Lovely :)
        //unsigned int _activeParticlesAtomicCounterBufferId;
        //unsigned int _copyBufferId;

    };
}

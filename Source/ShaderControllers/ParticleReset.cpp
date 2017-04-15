#include "Include/ShaderControllers/ParticleReset.h"

#include "Shaders/ShaderStorage.h"

#include "ThirdParty/glload/include/glload/gl_4_4.h"
#include "ThirdParty/glm/gtc/type_ptr.hpp"


namespace ShaderControllers
{
    /*----------------------------------------------------------------------------------------
    Description:
        Gives members initial values.
        
        Constructs the ParticleResetPoint and ParticleResetBar compute shaders out of the 
        necessary shader pieces.
        Looks up all uniforms in the resultant shaders.
    Parameters: 
        ssboToReset     ParticleUpdate will tell the SSBO to configure its buffer size 
                        uniforms for the compute shader.
    Returns:    None
    Creator:    John Cox, 4/2017
    ----------------------------------------------------------------------------------------*/
    ParticleReset::ParticleReset(const ParticleSsbo::SHARED_PTR &ssboToReset)
    {

    }
    
    
    /*--------------------------------------------------------------------------------------------
    Description:
        Cleans up buffers that were allocated in this object.
    Parameters: None
    Returns:    None
    Creator:    John Cox (11-24-2016)
    --------------------------------------------------------------------------------------------*/
    ParticleReset::~ParticleReset()
    {

    }

    /*--------------------------------------------------------------------------------------------
    Description:
        Adds a point emitter to internal storage.  These are used to initialize particles.  If 
        there multiple emitters, then the update will need to perform multiple calls to the 
        compute shader, each with different emitter information.

        If, for some reason, the particle emitter cannot be cast to either a point emitter or a 
        bar emitter, then the emitter will not be added to either particle emitter collection 
        and "false" is returned. 

        Note: Particles are evenly split between all emitters.
    Parameters:
        pEmitter    A pointer to a "particle emitter" interface.
    Returns:    
        True if the emitter was added, otherwise false.
    Creator:    John Cox (9-18-2016)    (created prior to this class in an earlier design)
    --------------------------------------------------------------------------------------------*/
    bool ParticleReset::AddEmitter(const IParticleEmitter::SHARED_PTR &pEmitter)
    {
        const ParticleEmitterPoint::SHARED_PTR pointEmitter = std::dynamic_pointer_cast<ParticleEmitterPoint>(pEmitter);
        const ParticleEmitterBar::SHARED_PTR barEmitter = std::dynamic_pointer_cast<ParticleEmitterBar>(pEmitter);

        if (pointEmitter != nullptr)
        {
            _pointEmitters.push_back(pointEmitter);
            return true;
        }
        else if (barEmitter != nullptr)
        {
            _barEmitters.push_back(barEmitter);
            return true;
        }

        // neither point emitter nor bar emitter; don't know what it is
        return false;
    }

    /*--------------------------------------------------------------------------------------------
    Description:
        Dispatches a shader for each emitter, resetting up to particlesPerEmitterPerFrame for 
        each emitter.
    Parameters:    
        particlesPerEmitterPerFrame     Limits the number of particles that are reset per frame 
                                        so that they don't all spawn at once.
    Returns:    None
    Creator:    John Cox (10-10-2016)
                (created in an earlier class, but later split into a dedicated class)
    --------------------------------------------------------------------------------------------*/
    void ParticleReset::ResetParticles(unsigned int particlesPerEmitterPerFrame)
    {

    }


}

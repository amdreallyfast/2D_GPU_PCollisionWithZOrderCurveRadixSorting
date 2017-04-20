#include "Include/ShaderControllers/ParticleReset.h"

#include <string>

#include "Shaders/ShaderStorage.h"
#include "Shaders/ComputeHeaders/ComputeShaderWorkGroupSizes.comp"
#include "Shaders/ComputeHeaders/SsboBufferBindings.comp"

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
    ParticleReset::ParticleReset(const ParticleSsbo::SHARED_PTR &ssboToReset) :
        _totalParticleCount(0),
        _computeProgramIdBarEmitters(0),
        _computeProgramIdPointEmitters(0),
        _particleResetAtomicCounterBufferId(0),
        _unifLocPointEmitterCenter(-1),
        _unifLocPointMaxParticleEmitCount(-1),
        _unifLocPointMinParticleVelocity(-1),
        _unifLocPointDeltaParticleVelocity(-1),
        _unifLocBarEmitterP1(-1),
        _unifLocBarEmitterP2(-1),
        _unifLocBarEmitterEmitDir(-1),
        _unifLocBarMaxParticleEmitCount(-1),
        _unifLocBarMinParticleVelocity(-1),
        _unifLocBarDeltaParticleVelocity(-1)
    {
        _totalParticleCount = ssboToReset->NumItems();

        // construct the compute shader
        ShaderStorage &shaderStorageRef = ShaderStorage::GetInstance();
        std::string shaderKey;

        // first make the particle reset shader for point emitters
        shaderKey = "particle reset point emitter";
        shaderStorageRef.NewCompositeShader(shaderKey);
        shaderStorageRef.AddPartialShaderFile(shaderKey, "Shaders/ComputeHeaders/Version.comp");
        shaderStorageRef.AddPartialShaderFile(shaderKey, "Shaders/ComputeHeaders/ComputeShaderWorkGroupSizes.comp");
        shaderStorageRef.AddPartialShaderFile(shaderKey, "Shaders/ComputeHeaders/SsboBufferBindings.comp");
        shaderStorageRef.AddPartialShaderFile(shaderKey, "Shaders/ComputeHeaders/CrossShaderUniformLocations.comp");
        shaderStorageRef.AddPartialShaderFile(shaderKey, "Shaders/ParticleBuffer.comp");
        shaderStorageRef.AddPartialShaderFile(shaderKey, "Shaders/Random.comp");
        shaderStorageRef.AddPartialShaderFile(shaderKey, "Shaders/NewVelocityBetweenMinAndMax.comp");
        shaderStorageRef.AddPartialShaderFile(shaderKey, "Shaders/QuickNormalize.comp");
        shaderStorageRef.AddPartialShaderFile(shaderKey, "Shaders/LinearBlend.comp");
        shaderStorageRef.AddPartialShaderFile(shaderKey, "Shaders/ParticleResetPointEmitter.comp");
        shaderStorageRef.CompileCompositeShader(shaderKey, GL_COMPUTE_SHADER);
        shaderStorageRef.LinkShader(shaderKey);
        _computeProgramIdPointEmitters = shaderStorageRef.GetShaderProgram(shaderKey);
        ssboToReset->ConfigureConstantUniforms(_computeProgramIdPointEmitters);

        // for ParticleResetPointEmitter.comp
        _unifLocPointEmitterCenter = shaderStorageRef.GetUniformLocation(shaderKey, "uPointEmitterCenter");
        _unifLocPointMaxParticleEmitCount = shaderStorageRef.GetUniformLocation(shaderKey, "uMaxParticleEmitCount");

        // for NewVelocityBetweenMinAndMax.comp
        _unifLocPointMinParticleVelocity = shaderStorageRef.GetUniformLocation(shaderKey, "uMinParticleVelocity");
        _unifLocPointDeltaParticleVelocity = shaderStorageRef.GetUniformLocation(shaderKey, "uDeltaParticleVelocity");

        // now for the bar emitters
        shaderKey = "particle reset bar emitter";
        shaderStorageRef.NewCompositeShader(shaderKey);
        shaderStorageRef.AddPartialShaderFile(shaderKey, "Shaders/ComputeHeaders/Version.comp");
        shaderStorageRef.AddPartialShaderFile(shaderKey, "Shaders/ComputeHeaders/ComputeShaderWorkGroupSizes.comp");
        shaderStorageRef.AddPartialShaderFile(shaderKey, "Shaders/ComputeHeaders/SsboBufferBindings.comp");
        shaderStorageRef.AddPartialShaderFile(shaderKey, "Shaders/ComputeHeaders/CrossShaderUniformLocations.comp");
        shaderStorageRef.AddPartialShaderFile(shaderKey, "Shaders/ParticleBuffer.comp");
        shaderStorageRef.AddPartialShaderFile(shaderKey, "Shaders/Random.comp");
        shaderStorageRef.AddPartialShaderFile(shaderKey, "Shaders/NewVelocityBetweenMinAndMax.comp");
        shaderStorageRef.AddPartialShaderFile(shaderKey, "Shaders/QuickNormalize.comp");
        shaderStorageRef.AddPartialShaderFile(shaderKey, "Shaders/LinearBlend.comp");
        shaderStorageRef.AddPartialShaderFile(shaderKey, "Shaders/ParticleResetBarEmitter.comp");
        shaderStorageRef.CompileCompositeShader(shaderKey, GL_COMPUTE_SHADER);
        shaderStorageRef.LinkShader(shaderKey);
        _computeProgramIdBarEmitters = shaderStorageRef.GetShaderProgram(shaderKey);
        ssboToReset->ConfigureConstantUniforms(_computeProgramIdBarEmitters);

        // for ParticleResetBarEmitter.comp
        _unifLocBarEmitterP1 = shaderStorageRef.GetUniformLocation(shaderKey, "uBarEmitterP1");
        _unifLocBarEmitterP2 = shaderStorageRef.GetUniformLocation(shaderKey, "uBarEmitterP2");
        _unifLocBarEmitterEmitDir = shaderStorageRef.GetUniformLocation(shaderKey, "uBarEmitterEmitDir");
        _unifLocBarMaxParticleEmitCount = shaderStorageRef.GetUniformLocation(shaderKey, "uMaxParticleEmitCount");

        // for NewVelocityBetweenMinAndMax.comp
        // Note: This function requirs a min and max, and in going with two shaders, both of 
        // which use this function, the min and max uniforms may have different locations in the 
        // different shaders, so they have to be stored separately.
        _unifLocBarMinParticleVelocity = shaderStorageRef.GetUniformLocation(shaderKey, "uMinParticleVelocity");
        _unifLocBarDeltaParticleVelocity = shaderStorageRef.GetUniformLocation(shaderKey, "uDeltaParticleVelocity");

        // uniform values are set in ResetParticles(...)
        
        // generate the atomic counters for the number of active particles
        // Note: Atomic counter initialization courtesy of geeks3D (and my use of 
        // glBufferData(...) instead of glMapBuffer(...)
        // http://www.geeks3d.com/20120309/opengl-4-2-atomic-counter-demo-rendering-order-of-fragments/
        glGenBuffers(1, &_particleResetAtomicCounterBufferId);
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, _particleResetAtomicCounterBufferId);
        GLuint atomicCounterResetValue = 0;
        glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), &atomicCounterResetValue, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

        // don't need to have a bound program or bound buffer to set the buffer base
        // Note: It seems that atomic counters must be bound where they are declared and cannot 
        // be bound dynamically like the ParticleSsbo and PolygonSsbo.  So remember to use the 
        // SAME buffer binding base as specified in the shader.
        glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, ATOMIC_COUNTER_BUFFER_BINDING, _particleResetAtomicCounterBufferId);
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
        glDeleteBuffers(1, &_particleResetAtomicCounterBufferId);
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
    bool ParticleReset::AddEmitter(const IParticleEmitter::CONST_PTR &pEmitter)
    {
        ParticleEmitterPoint::CONST_PTR pointEmitter = std::dynamic_pointer_cast<const ParticleEmitterPoint>(pEmitter);
        ParticleEmitterBar::CONST_PTR barEmitter = std::dynamic_pointer_cast<const ParticleEmitterBar>(pEmitter);

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

        Particles are spread out evenly between all the emitters (or at least as best as 
        possible; technically the first emitter gets first dibs at the inactive particles, then 
        the second emitter, etc.).
    Parameters:    
        particlesPerEmitterPerFrame     Limits the number of particles that are reset per frame 
                                        so that they don't all spawn at once.
    Returns:    None
    Creator:    John Cox (10-10-2016)
                Originally from an even earlier class.  The "particle reset" and 
                "particle update" are the two oldest compute shaders in my demos.
    --------------------------------------------------------------------------------------------*/
    void ParticleReset::ResetParticles(unsigned int particlesPerEmitterPerFrame)
    {
        // TODO: use std::chrono to profile



        if (_pointEmitters.empty() && _barEmitters.empty())
        {
            // nothing to do
            return;
        }

        GLuint atomicCounterResetValue = 0;

        // spreading the particles evenly between multiple emitters is done by letting all the 
        // particle emitters have a go at all the inactive particles
        // Note: Yes, this algorithm is such that emitters resetting particles have to travers 
        // through the entire particle collection, but there isn't a way of telling the CPU 
        // where they were when the last particle was reset.  Also, after the "particles per 
        // emitter per frame" limit is reached, the vast majority of the threads will simply 
        // return, so it's actually pretty fast.
        // TODO: profile
        GLuint numWorkGroupsX = (_totalParticleCount / PARTICLE_OPERATIONS_WORK_GROUP_SIZE_X) + 1;
        GLuint numWorkGroupsY = 1;
        GLuint numWorkGroupsZ = 1;

        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, _particleResetAtomicCounterBufferId);
            
        // give all point emitters a chance to reactivate inactive particles at their positions
        glUseProgram(_computeProgramIdPointEmitters);
        glUniform1ui(_unifLocPointMaxParticleEmitCount, particlesPerEmitterPerFrame);
        for (size_t pointEmitterCount = 0; pointEmitterCount < _pointEmitters.size(); pointEmitterCount++)
        {
            // reset everything necessary to control the emission parameters for this emitter
            glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), (void *)&atomicCounterResetValue);

            ParticleEmitterPoint::CONST_PTR &emitter = _pointEmitters[pointEmitterCount];

            glUniform1f(_unifLocPointMinParticleVelocity, emitter->GetMinVelocity());
            glUniform1f(_unifLocPointDeltaParticleVelocity, emitter->GetDeltaVelocity());
            glUniform4fv(_unifLocPointEmitterCenter, 1, glm::value_ptr(emitter->GetPos()));

            // compute ALL the resets! (then make the results visible to the next use of the 
            // SSBO and to vertext buffer)
            glDispatchCompute(numWorkGroupsX, numWorkGroupsY, numWorkGroupsZ);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
        }

        // and now for any bar emitters
        glUseProgram(_computeProgramIdBarEmitters);
        glUniform1ui(_unifLocBarMaxParticleEmitCount, particlesPerEmitterPerFrame);
        for (size_t barEmitterCount = 0; barEmitterCount < _barEmitters.size(); barEmitterCount++)
        {
            glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), (void *)&atomicCounterResetValue);

            ParticleEmitterBar::CONST_PTR &emitter = _barEmitters[barEmitterCount];

            glUniform1f(_unifLocBarMinParticleVelocity, emitter->GetMinVelocity());
            glUniform1f(_unifLocBarDeltaParticleVelocity, emitter->GetDeltaVelocity());

            // each bar needs to upload three position vectors (p1, p2, and emit direction)
            glUniform4fv(_unifLocBarEmitterP1, 1, glm::value_ptr(emitter->GetBarStart()));
            glUniform4fv(_unifLocBarEmitterP2, 1, glm::value_ptr(emitter->GetBarEnd()));
            glUniform4fv(_unifLocBarEmitterEmitDir, 1, glm::value_ptr(emitter->GetEmitDir()));

            // MOAR resets!
            glDispatchCompute(numWorkGroupsX, numWorkGroupsY, numWorkGroupsZ);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
        }

        // cleanup
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
        glUseProgram(0);
    }


}

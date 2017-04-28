#include "Include/ShaderControllers/ParticleUpdate.h"

#include <string>

#include "Shaders/ShaderStorage.h"
#include "Shaders/ComputeHeaders/ComputeShaderWorkGroupSizes.comp"
#include "Shaders/ComputeHeaders/SsboBufferBindings.comp"

#include "ThirdParty/glload/include/glload/gl_4_4.h"
#include "ThirdParty/glm/gtc/type_ptr.hpp"



namespace ShaderControllers
{
    /*--------------------------------------------------------------------------------------------
    Description:
        Gives members initial values.
        
        Constructs the ParticleUpdate compute shader out of the necessary shader pieces.
        Looks up all uniforms in the resultant ParticleUpdate shader.
    Parameters: 
        ssboToUpdate    ParticleUpdate will tell the SSBO to configure its buffer size uniforms 
                        for the compute shader.
        particleRegionCenter    Used in conjunction with radius to tell when a particle goes 
        particleRegionRedius    out of bounds.
    Returns:    None
    Creator:    John Cox, 4/2017
    --------------------------------------------------------------------------------------------*/
    ParticleUpdate::ParticleUpdate(const ParticleSsbo::SHARED_PTR &ssboToUpdate,
        const glm::vec4 &particleRegionCenter,
        float particleRegionRadius) :
        _totalParticleCount(0),
        _activeParticleCount(0),
        _computeProgramId(0),
        _unifLocParticleRegionCenter(-1),
        _unifLocParticleRegionRadiusSqr(-1),
        _unifLocDeltaTimeSec(-1)
        //_activeParticlesAtomicCounterBufferId(0),
        //_copyBufferId(0)
    {
        _totalParticleCount = ssboToUpdate->NumItems();

        // construct the compute shader
        ShaderStorage &shaderStorageRef = ShaderStorage::GetInstance();

        std::string shaderKey = "particle update";
        shaderStorageRef.NewCompositeShader(shaderKey);
        shaderStorageRef.AddPartialShaderFile(shaderKey, "Shaders/ComputeHeaders/Version.comp");
        shaderStorageRef.AddPartialShaderFile(shaderKey, "Shaders/ComputeHeaders/ComputeShaderWorkGroupSizes.comp");
        shaderStorageRef.AddPartialShaderFile(shaderKey, "Shaders/ComputeHeaders/SsboBufferBindings.comp");
        shaderStorageRef.AddPartialShaderFile(shaderKey, "Shaders/ComputeHeaders/CrossShaderUniformLocations.comp");
        shaderStorageRef.AddPartialShaderFile(shaderKey, "Shaders/ParticleBuffer.comp");
        shaderStorageRef.AddPartialShaderFile(shaderKey, "Shaders/ParticleUpdate.comp");
        shaderStorageRef.CompileCompositeShader(shaderKey, GL_COMPUTE_SHADER);
        shaderStorageRef.LinkShader(shaderKey);
        _computeProgramId = shaderStorageRef.GetShaderProgram(shaderKey);
        ssboToUpdate->ConfigureConstantUniforms(_computeProgramId);

        _unifLocParticleRegionCenter = shaderStorageRef.GetUniformLocation(shaderKey, "uParticleRegionCenter");
        _unifLocParticleRegionRadiusSqr = shaderStorageRef.GetUniformLocation(shaderKey, "uParticleRegionRadiusSqr");
        _unifLocDeltaTimeSec = shaderStorageRef.GetUniformLocation(shaderKey, "uDeltaTimeSec");

        // set uniform values and generate the atomic counters for the number of active particles
        glUseProgram(_computeProgramId);
        glUniform4fv(_unifLocParticleRegionCenter, 1, glm::value_ptr(particleRegionCenter));
        glUniform1f(_unifLocParticleRegionRadiusSqr, particleRegionRadius * particleRegionRadius);
        // delta time set in Update(...)

        // atomic counter initialization courtesy of geeks3D (and my use of glBufferData(...) 
        // instead of glMapBuffer(...)
        // http://www.geeks3d.com/20120309/opengl-4-2-atomic-counter-demo-rendering-order-of-fragments/

        //// particle counter
        //glGenBuffers(1, &_activeParticlesAtomicCounterBufferId);
        //glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, _activeParticlesAtomicCounterBufferId);
        //GLuint atomicCounterResetValue = 0;
        //glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), (void *)&atomicCounterResetValue, GL_DYNAMIC_DRAW);
        //glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

        //// the atomic counter copy buffer follows suit
        //// Note: At this point (initialization), it doesn't matter what buffer this ID is bound 
        //// to.  I bind it to GL_COPY_WRITE_BUFFER instead of GL_ATOMIC_COUNTER_BUFFER as an 
        //// indicator to myself that this buffer is meant to be written into via a memory copy.
        //glGenBuffers(1, &_copyBufferId);
        //glBindBuffer(GL_COPY_WRITE_BUFFER, _copyBufferId);
        //glBufferData(GL_COPY_WRITE_BUFFER, sizeof(GLuint), 0, GL_DYNAMIC_COPY);
        //glBindBuffer(GL_COPY_WRITE_BUFFER, 0);

        //// cleanup
        //glUseProgram(0);

        //// don't need to have a program or bound buffer to set the buffer base
        //// Note: It seems that atomic counters must be bound where they are declared and cannot 
        //// be bound dynamically like the ParticleSsbo and PolygonSsbo.  So remember to use the 
        //// SAME buffer binding base as specified in the shader.
        //glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, PARTICLE_UPDATE_ATOMIC_COUNTER_BUFFER_BINDING, _activeParticlesAtomicCounterBufferId);

        //// Note: Do NOT bind a buffer base for the "particle counter copy" atomic counter 
        //// because it is not used in the shader itself.  It is instead meant to copy the 
        //// atomic counter buffer before the copy is mapped to a system memory pointer.  Doing 
        //// this with the actual atomic counter caused a horrific performance drop.  It appeared 
        //// to completely trash the instruction pipeline.
    }

    /*--------------------------------------------------------------------------------------------
    Description:
        Cleans up buffers and shader programs that were created for this shader controller.
    Parameters: None
    Returns:    None
    Creator:    John Cox, 4/2017
    --------------------------------------------------------------------------------------------*/
    ParticleUpdate::~ParticleUpdate()
    {
        glDeleteProgram(_computeProgramId);
        //glDeleteBuffers(1, &_activeParticlesAtomicCounterBufferId);
        //glDeleteBuffers(1, &_copyBufferId);
    }

    /*--------------------------------------------------------------------------------------------
    Description:
        Resets the "num active particles" atomic counter, dispatches the shader, and reads the 
        number of active particles after the shader finished.
    
        The number of work groups is based on the maximum number of particles.
    Parameters:    
        deltaTimeSec    Self-explanatory
    Returns:    None
    Creator:    John Cox (10-10-2016)
    --------------------------------------------------------------------------------------------*/
    void ParticleUpdate::Update(float deltaTimeSec, std::unique_ptr<PersistentAtomicCounterBuffer> &counter)
    {
        // spread out the particles between lots of work items, but keep it 1-dimensional 
        // because the particle buffer is a 1-dimensional array
        // Note: +1 because integer division drops the remainder, and I want all the particles 
        // to have a shot.
        GLuint numWorkGroupsX = (_totalParticleCount / PARTICLE_OPERATIONS_WORK_GROUP_SIZE_X) + 1;
        GLuint numWorkGroupsY = 1;
        GLuint numWorkGroupsZ = 1;

        glUseProgram(_computeProgramId);

        glUniform1f(_unifLocDeltaTimeSec, deltaTimeSec);
        counter->ResetCounter();
        //glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, _activeParticlesAtomicCounterBufferId);
        //GLuint atomicCounterResetValue = 0;
        //glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), (void *)&atomicCounterResetValue);
        glDispatchCompute(numWorkGroupsX, numWorkGroupsY, numWorkGroupsZ);

        // the results of the moved particles need to be visible to the next compute shader that 
        // accesses the buffer, vertex data sourced from the particle buffer need to reflect the 
        // updated movements, and reads from atomic counters must be visible as well (for number 
        // of active particles)
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);

        // cleanup
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
        glUseProgram(0);

        // now that all active particles have updated, check how many active particles exist 
        _activeParticleCount = counter->GetValue();

        //// Note: Thanks to this post for prompting me to learn about buffer copying to solve 
        //// this "extract atomic counter from compute shader" issue.
        //// (http://gamedev.stackexchange.com/questions/93726/what-is-the-fastest-way-of-reading-an-atomic-counter)
        //glBindBuffer(GL_COPY_READ_BUFFER, _activeParticlesAtomicCounterBufferId);
        //glBindBuffer(GL_COPY_WRITE_BUFFER, _copyBufferId);
        //glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, sizeof(GLuint));
        //void *bufferPtr = glMapBufferRange(GL_COPY_WRITE_BUFFER, 0, sizeof(GLuint), GL_MAP_READ_BIT);
        //GLuint *particleCountPtr = static_cast<GLuint *>(bufferPtr);
        //_activeParticleCount = *particleCountPtr;
        //glUnmapBuffer(GL_COPY_WRITE_BUFFER);

        ////cleanup
        //glBindBuffer(GL_COPY_READ_BUFFER, 0);
        //glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
    }

    /*--------------------------------------------------------------------------------------------
    Description:
        A simple getter for the number of particles that were active on the last Update(...) 
        call.
        
        Useful for performance comparison with CPU version.
    Parameters: None
    Returns:    None
    Creator:    John Cox (1-7-2017)
    --------------------------------------------------------------------------------------------*/
    unsigned int ParticleUpdate::NumActiveParticles() const
    {
        return _activeParticleCount;
    }

}

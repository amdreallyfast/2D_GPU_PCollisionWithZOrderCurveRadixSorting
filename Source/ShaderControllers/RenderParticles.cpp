#include "Include/ShaderControllers/RenderParticles.h"

#include <string>
#include "Shaders/ShaderStorage.h"

#include "ThirdParty/glload/include/glload/gl_4_4.h"

namespace ShaderControllers
{
    /*--------------------------------------------------------------------------------------------
    Description:
        Gives members initial values.
        
        Constructs the RenderParticles shader out of ParticleRender.vert and ParticleRender.frag.
    Parameters: None
    Returns:    None
    Creator:    John Cox, 4/2017
    --------------------------------------------------------------------------------------------*/
    RenderParticles::RenderParticles() :
        _renderProgramId(0)
    {
        // construct the compute shader
        ShaderStorage &shaderStorageRef = ShaderStorage::GetInstance();

        std::string shaderKey = "particle render";
        shaderStorageRef.NewCompositeShader(shaderKey);
        shaderStorageRef.AddPartialShaderFile(shaderKey, "Shaders/ComputeHeaders/Version.comp");
        shaderStorageRef.AddPartialShaderFile(shaderKey, "Shaders/CountNearbyParticlesLimits.comp");
        shaderStorageRef.AddPartialShaderFile(shaderKey, "Shaders/ParticleRender.vert");
        shaderStorageRef.CompileCompositeShader(shaderKey, GL_VERTEX_SHADER);
        shaderStorageRef.AddAndCompileShaderFile(shaderKey, "Shaders/ParticleRender.frag", GL_FRAGMENT_SHADER);
        shaderStorageRef.LinkShader(shaderKey);
        _renderProgramId = shaderStorageRef.GetShaderProgram(shaderKey);
    }

    /*--------------------------------------------------------------------------------------------
    Description:
        Cleans up the shader that was created for this shader controller.
    Parameters: None
    Returns:    None
    Creator:    John Cox, 4/2017
    --------------------------------------------------------------------------------------------*/
    RenderParticles::~RenderParticles()
    {
        glDeleteProgram(_renderProgramId);
    }

    /*--------------------------------------------------------------------------------------------
    Description:
        Only this shader controller knows the program ID, so only it can tell the SSBO to 
        configure itself for rendering with this render program.
    Parameters: 
        particleSsboToRender    Contains the VAO ID, draw style, and number of vertices.
    Returns:    None
    Creator:    John Cox, 5/2017
    --------------------------------------------------------------------------------------------*/
    void RenderParticles::ConfigureSsboForRendering(const ParticleSsbo::SHARED_PTR &configureThis)
    {
        // particles are points, so every vertex is a point
        configureThis->ConfigureRender(_renderProgramId, GL_POINTS);
    }

    /*--------------------------------------------------------------------------------------------
    Description:
        Binds the VAO for the particle SSBO, then calls glDrawArrays(...).
    Parameters: 
        particleSsboToRender    Contains the VAO ID, draw style, and number of vertices.
    Returns:    None
    Creator:    John Cox, 4/2017
    --------------------------------------------------------------------------------------------*/
    void RenderParticles::Render(const ParticleSsbo::SHARED_PTR &particleSsboToRender) const
    {
        glUseProgram(_renderProgramId);
        glBindVertexArray(particleSsboToRender->VaoId());

        // in the case of particles, "num items" == "num vertices", so either getter is fine
        glDrawArrays(particleSsboToRender->DrawStyle(), 0, particleSsboToRender->NumVertices());
        glBindVertexArray(0);
        glUseProgram(0);
    }

}

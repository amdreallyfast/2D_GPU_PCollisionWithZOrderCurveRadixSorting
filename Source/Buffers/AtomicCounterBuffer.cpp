#include "Include/Buffers/AtomicCounterBuffer.h"

#include "Shaders/ComputeHeaders/SsboBufferBindings.comp"

#include "ThirdParty/glload/include/glload/gl_4_4.h"

// TODO: header
PersistentAtomicCounterBuffer::PersistentAtomicCounterBuffer() :
    _bufferId(0),
    _bufferPtr(0)
{
    glGenBuffers(1, &_bufferId);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, _bufferId);
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, ATOMIC_COUNTER_BUFFER_BINDING, _bufferId);
    GLuint atomicCounterResetValue = 0;
    //glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), &atomicCounterResetValue, GL_DYNAMIC_DRAW);
    //glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW);

    // ??is it faster to use "write only? the OpenGL status message says that it will use DMA CACHED memory for read|write but "SYSTEM_HEAP ... (fast)" when only using write??; but if I write only, then I obviously can't read it
    //GLuint flags = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
    GLuint flags = GL_MAP_WRITE_BIT| GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
    //glBufferStorage(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), nullptr, flags);
    glBufferStorage(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), &atomicCounterResetValue, flags);

    // force cast to unsigned int pointer because I know that it is a buffer of unsigned integers
    void *voidPtr = glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), flags);
    _bufferPtr = static_cast<unsigned int *>(voidPtr);
    _bufferPtr[0] = 0;

    // Note: It seems that atomic counters must be bound where they are declared and cannot 
    // be bound dynamically like the ParticleSsbo and PolygonSsbo.  So remember to use the 
    // SAME buffer binding base as specified in the shader.
    // Also Note: Don't need to have a program or bound buffer to set the buffer base.  This buffer is persistently mapped and bound, but base buffer binding can be done without.
    //glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, ATOMIC_COUNTER_BUFFER_BINDING, _bufferId);

    

}

// TODO: header
PersistentAtomicCounterBuffer::~PersistentAtomicCounterBuffer()
{
    // unsynchronize
    glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
    glDeleteBuffers(1, &_bufferId);
}

#include <stdio.h>

// TODO: header
void PersistentAtomicCounterBuffer::ResetCounter() const
{
    GLsync writeSyncFence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    //printf("value is %u\n", *_bufferPtr);

    GLenum waitReturn = GL_UNSIGNALED;
    while (waitReturn != GL_ALREADY_SIGNALED)// && waitReturn != GL_CONDITION_SATISFIED)
    {
        waitReturn = glClientWaitSync(writeSyncFence, GL_SYNC_FLUSH_COMMANDS_BIT, 0);
    }
    _bufferPtr[0] = 0;
    glMemoryBarrier(GL_ATOMIC_COUNTER_BARRIER_BIT);
    //glClientWaitSync(theThing, GL_SYNC_FLUSH_COMMANDS_BIT, 1000000000);
    
}

// TODO: header
unsigned int PersistentAtomicCounterBuffer::GetValue() const
{
    GLsync readSyncFence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    while (glClientWaitSync(readSyncFence, GL_SYNC_FLUSH_COMMANDS_BIT, 0) != GL_ALREADY_SIGNALED);

    // according to Khronos, the the sync object will be automatically deleted if no glWaitSync(...) or glClientWaitSync(...) commands are blocking on that sync object
    // Note: See https://www.khronos.org/opengl/wiki/GLAPI/glDeleteSync.
    //glDeleteSync(readSyncFence);

    return *_bufferPtr;
}


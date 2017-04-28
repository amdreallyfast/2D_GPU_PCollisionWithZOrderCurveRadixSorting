#pragma once

/*------------------------------------------------------------------------------------------------
Description:
    At this time (4-28-2017) there are a couple compute shaders ("reset particles" and "update particles") that use an atomic counter.  Up until now I have been using an atomic counter for each shader, binding the buffer prior to use and using glBufferSubData(...) to reset the counter to 0, and maybe using glMapBufferSubRange(...) later to get the value of the counter.

    Now I want to using a persistently mapped buffer.  The GL_ATOMIC_COUNTER_BUFFER is not a common buffer target, and it is only used in compute shaders, and I want to write to it and read from frequently (albeit only 1 uint on each write or read), so I will try to make a single atomic counter buffer that any compute shader can use.

    I am taking my information from the Steam Dev Days 2014 talk, 
    "Beyond Porting: How Modern OpenGL Can Radically Reduce Driver Overhead".  
    https://www.youtube.com/watch?v=-bCeNzgiJ8I&index=21&list=PLckFgM6dUP2hc4iy-IdKFtqR9TeZWMPjm
    Start @8:30
    
    It can also be found under GDC 2014 "Approach Zero Driver Overhead".
    https://www.youtube.com/watch?v=K70QbvzB6II
    Start @11:08
Creator:    John Cox, 4/2017
------------------------------------------------------------------------------------------------*/
class PersistentAtomicCounterBuffer
{
public:
    PersistentAtomicCounterBuffer();
    ~PersistentAtomicCounterBuffer();

    void ResetCounter() const;
    unsigned int GetValue() const;

private:
    unsigned int _bufferId;
    unsigned int *_bufferPtr;
};

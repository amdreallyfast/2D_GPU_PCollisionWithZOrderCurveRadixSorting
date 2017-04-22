#version 440

// Note: The vec2's are in window space (both X and Y on the range [-1,+1])
// Also Note: The vec2s are provided as vec4s on the CPU side and specified as such in the 
// vertex array attributes, but it is ok to only take them as a vec2, as I am doing for this 
// 2D demo.
layout (location = 0) in vec4 pos;  
layout (location = 1) in vec4 vel;  
layout (location = 2) in float mass;
layout (location = 3) in float collisionRadius;
layout (location = 4) in uint mortonCode;
layout (location = 5) in uint hasCollidedAlreadyThisFrame;
layout (location = 6) in int isActive;

// must have the same name as its corresponding "in" item in the frag shader
smooth out vec4 particleColor;

void main()
{
    //particleColor = vec4(0.5f, 1.0f, 1.0f, 1.0f);

    if (pos.x == 0.0f)
    {
        // red
        particleColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
    }
    else if (pos.y == 0.0f)
    {
        // yellow
        particleColor = vec4(0.0f, 1.0f, 1.0f, 1.0f);
    }
    else
    {
        // cyan
        particleColor = vec4(0.5f, 1.0f, 1.0f, 1.0f);
    }

    {
        ////gl_Position = vec4(-0.3f, 0.3f, 0.0f, 1.0f);
        ////gl_Position = normalize(vec4(pos.xyz, 1.0f));
        //vec4 nPos = normalize(vec4(pos.xyz, 0.0f));
        //if (length(pos.xyz) > 1)
        //{
        //    gl_Position = vec4(-0.3f, -0.3f, 0.0f, 1.0f);
        //}
        //else
        //{
        //    gl_Position = vec4(+0.3f, -0.3f, 0.0f, 1.0f);
        //}

        vec4 nPos = vec4(pos.x - 0.5f, pos.y - 0.5f, pos.z, 1.0f);
        nPos = normalize(nPos);
        gl_Position = nPos;
        
    }
    
    //gl_Position = normalize(pos);
    
}


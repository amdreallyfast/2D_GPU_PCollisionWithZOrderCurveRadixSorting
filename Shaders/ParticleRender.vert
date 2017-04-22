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
    if (isActive == 0)
    {
        // invisible (alpha = 0), but "fully transparent" does not mean "no color", it merely 
        // means that the color of this thing will be added to the thing behind it (see Z 
        // adjustment later)
        //particleColor = vec4(0.0f, 0.0f, 1.0f, 0.0f);   // blue
        particleColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);   // black
        gl_Position = vec4(pos.xy, -0.6f, 1.0f);
    }
    else
    {
        if (hasCollidedAlreadyThisFrame == 0)
        {
            // cyan
            particleColor = vec4(0.5f, 1.0f, 1.0f, 1.0f);
        }
        else
        {
            // had a collision => red
            particleColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
        }
//        else if (hasCollidedAlreadyThisFrame == 13)
//        {
//            // particle is active, but no collision => green
//            particleColor = vec4(0.0f, 1.0f, 0.0f, 1.0f);
//        }
//        else if (hasCollidedAlreadyThisFrame == 7)
//        {
//            // particle is active, collision with neighbor => red
//            particleColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
//        }
////        else    // ??condition??
////        {
////            // blue
////            particleColor = vec4(0.0f, 0.0f, 1.0f, 1.0f);
////        }

        //particleColor = vec4(0.5f, 1.0f, 1.0f, 1.0f);
    
        // Note: The W position seems to be used as a scaling factor (I must have forgotten this 
        // from the graphical math; it's been awhile since I examined it in detail).  If I do any 
        // position normalization, I should make sure that gl_Position's W value is always 1.
        gl_Position = vec4(pos.xy, -0.7f, 1.0f);    
    }
}


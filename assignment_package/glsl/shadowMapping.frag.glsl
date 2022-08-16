#version 330
// ^ Change this to version 130 if you have compatibility issues

// This is a fragment shader. If you've opened this file first, please
// open and read lambert.vert.glsl before reading on.
// Unlike the vertex shader, the fragment shader actually does compute
// the shading of geometry. For every pixel in your program's output
// screen, the fragment shader is run for every bit of geometry that
// particular pixel overlaps. By implicitly interpolating the position
// data passed into the fragment shader by the vertex shader, the fragment shader
// can compute what color to apply to its pixel based on things like vertex
// position, light position, and vertex color.

in vec4 fs_Pos;

// These are the interpolated values out of the rasterizer, so you can't know
// their specific values without knowing the vertices that contributed to them
//out vec4 out_Col; // This is the final output color that you will see on your
                  // screen for the pixel that is currently being processed.

layout(location = 0) out vec4 fragmentdepth;

void main()
{
        //float depth = gl_FragCoord.z;
        //float depth = (gl_FragCoord.z-0.9995)*1000;
        float depth = (fs_Pos.z-150)/200;
        //fragmentdepth = vec3(depth/100);
        //out_Col = vec4(0.5);
        fragmentdepth = vec4(depth, depth, depth, 1);
}

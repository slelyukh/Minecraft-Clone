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

uniform sampler2D u_Texture;
uniform sampler2D u_NMap;
uniform float u_Time;
uniform sampler2D u_shadowMap;

// These are the interpolated values out of the rasterizer, so you can't know
// their specific values without knowing the vertices that contributed to them
in vec4 fs_Pos;
in vec4 fs_Nor;
in vec4 fs_LightVec;
in vec4 fs_UV;
in vec4 fs_lsPos;

out vec4 out_Col; // This is the final output color that you will see on your
                  // screen for the pixel that is currently being processed.

vec2 timeOffsetUV() {
        vec2 uv = vec2(0, 0);
        int animationT = int(u_Time)%8;
        if (animationT == 1 || animationT == 7) {
            uv = vec2(0.0625, 0);
        } else if (animationT == 2 || animationT == 6) {
            uv = vec2(0.125, 0);
        } else if (animationT == 3 || animationT == 5) {
            uv = vec2(0.0625, -0.0625);
        } else if (animationT == 4) {
            uv = vec2(0.125, -0.0625);
        }
        return uv;
}

mat3 orientationMatrix(vec3 n) {
    vec3 b;
    vec3 t;
    if (abs(n.x) > abs(n.y)) {
        t = vec3(-n.z, 0, n.x);
        t = normalize(t);
    } else {
        t = vec3(0, n.z, -n.y);
        t = normalize(t);
    }
    b = cross(n, t);
    return mat3(t, b, n);
}


void main()
{
    // Material base color (before shading)


        vec2 UV = vec2(fs_UV);

        if (fs_UV.z == 1) { //animated flag
            UV += timeOffsetUV();
        }

        vec4 diffuseColor = texture(u_Texture, UV);
        //vec4 diffuseColor = texture(u_shadowMap, UV);

        vec3 surfaceNormal  = texture(u_NMap, UV).rgb;
        vec3 normal;
        if (surfaceNormal == vec3(1, 1, 1)) {
            normal = vec3(fs_Nor);
        } else {
                surfaceNormal = normalize(surfaceNormal*2.0 - 1.0);
                mat3 orientation = orientationMatrix(vec3(fs_Nor));
                normal = orientation*surfaceNormal;
        }

        // Calculate the diffuse term for Lambert shading
        float diffuseTerm = dot(normalize(normal), normalize(vec3(fs_LightVec)));
        // Avoid negative lighting values
        diffuseTerm = clamp(diffuseTerm, 0, 1);

        mat4 biasMatrix = mat4(
        0.5, 0.0, 0.0, 0.0,
        0.0, 0.5, 0.0, 0.0,
        0.0, 0.0, 0.5, 0.0,
        0.5, 0.5, 0.5, 1.0
        );

        //float depth = (fs_lsPos.z-0.9995)*1000;
        float depth = (fs_lsPos.z-150)/200;
        out_Col = vec4(depth, depth, depth, 1);
        vec4 depthCoord = fs_lsPos/fs_lsPos.w;
        depthCoord.x = depthCoord.x/2+0.5;
        depthCoord.y = depthCoord.y/2+0.5;
        //out_Col = texture(u_shadowMap, depthCoord.xy);
        float mapDepth = texture(u_shadowMap, depthCoord.xy).z;

        float bias = 0.005;
        if (depth-bias > mapDepth) {
            diffuseTerm /= 10;
        }

        float ambientTerm = 0.2;

        float lightIntensity = diffuseTerm + ambientTerm;   //Add a small float value to the color multiplier
                                                            //to simulate ambient lighting. This ensures that faces that are not
                                                            //lit by our point light are not completely black.

        // Compute final shaded color
        out_Col = vec4(diffuseColor.rgb * lightIntensity, diffuseColor.a);
}

#version 330

in vec4 fs_UV;

out vec4 color;

uniform sampler2D u_Texture;

void main()
{
    vec2 uv = vec2(fs_UV.xy);
    vec4 c = texture(u_Texture, uv);
    float grey = .21 * c.r + .72 * c.g + .07 * c.b;
    float dist = sqrt(pow((fs_UV.x - .5),2) + pow((fs_UV.y - .5),2));
    grey *= -pow((dist+.4),6)+1;
    //color = vec4(grey, grey, grey, 1.f);
    color = vec4(c.r, c.g, c.b, 0.f);
}

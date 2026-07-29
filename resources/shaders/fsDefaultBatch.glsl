#version 330 core
out vec4 FragColor;


in vec2 TexCoord;
in vec4 Colour;
flat in int TexIndex;

uniform sampler2D textures[32];

void main()
{
    vec4 col = Colour;
    if (TexIndex >= 0)
    {
        int i = clamp(TexIndex, 0, 31);
        col = texture(textures[TexIndex], TexCoord);
    }

    //FragColor = vec4(1,0,0,1);
    FragColor = col;
}
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

// instance data
layout (location = 2) in vec3 iWorldPos;
layout (location = 3) in vec2 iSize;
layout (location = 4) in vec4 iColour;
layout (location = 5) in float iTexIndex;
layout (location = 6) in vec2 i_uv0;
layout (location = 7) in vec2 i_uv1;

out vec2 TexCoord;
out vec4 Colour;
flat out int TexIndex;

uniform mat4 proj;

void main()
{
	vec3 worldPos = iWorldPos + vec3(vec2(aPos.x, aPos.y) * iSize, 0.0f);

	
	TexCoord = mix(i_uv0, i_uv1, aTexCoord);
	Colour = iColour;
	TexIndex = int(iTexIndex);

	gl_Position = proj * vec4(worldPos, 1.0);
}
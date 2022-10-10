#version 450 core
layout (location = 0) in vec4 inColor;
layout (location = 1) in vec2 inCoords;
layout (binding = 1) uniform sampler2D inSampler;

layout (location = 0) out vec4 FragColor;

void main()
{
	FragColor = texture(inSampler, inCoords);
}

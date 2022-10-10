// NOTE(__LUNA__): Automate shader compliation within program using libshaderc
#version 450 core
// NOTE(__LUNA__): Implicit conversion from vec3 to vec4
layout (location = 0) in vec4 inPos;
layout (location = 1) in vec4 inColor;
layout (location = 2) in vec2 inCoords;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec2 outCoords;

layout (binding = 0) uniform UBO {
	mat4 model;
	mat4 view;
	mat4 proj;
} ubo;

void main()
{
	gl_Position = ubo.proj * ubo.view * ubo.model * inPos;
	outColor = inColor;
	outCoords = inCoords;
}

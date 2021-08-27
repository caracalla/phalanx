#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
	mat4 model;
	mat4 view;
	mat4 projection;
} ubo;

layout(location = 0) in vec3 inPosition;
// if inPosition was something like a dvec3 64 bit vector, it would use two
// slots insteaad of one, and inColor would need to be at location = 2
// (does that mean the max size of a "location" is 128 bits?

// Per https://www.khronos.org/opengl/wiki/Layout_Qualifier_(GLSL):
// "Scalars and vector types that are not doubles all take up one location. The
// double and dvec2 types also take one location, while dvec3 and dvec4 take up
// 2 locations. Structs take up locations based on their member types, in-order.
// Arrays also take up locations based on their array sizes."
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
	// gl_VertexIndex contains current vertex index
	// gl_Position is the output clip coordinate??

	gl_Position =
			ubo.projection * ubo.view * ubo.model * vec4(inPosition, 1.0);

	fragColor = inColor;
	fragTexCoord = inTexCoord;
}

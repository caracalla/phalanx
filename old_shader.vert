#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec3 fragColor;

// top left:    -1, -1
// top right:    1, -1
// bottom left: -1,  1
// bottom right: 1,  1

vec2 positions[6] = vec2[](
  // upper left
  vec2(-1.0, -1.0), // top left
  vec2(1.0, 1.0),   // bottom right
  vec2(-1.0, 1.0),  // bottom left

  // bottom right
  vec2(-1.0, -1.0), // top left
  vec2(1.0, -1.0),  // top right
  vec2(1.0, 1.0)    // bottom right
);

vec3 colors[6] = vec3[](
  vec3(1.0, 0.0, 0.0),
  vec3(0.0, 1.0, 0.0),
  vec3(0.0, 0.0, 1.0),
  vec3(1.0, 0.0, 0.0),
  vec3(0.0, 0.0, 1.0),
  vec3(0.0, 1.0, 0.0)
);

void main() {
  gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
  fragColor = colors[gl_VertexIndex];
}

#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
  // colors supplied per-vertex
  // outColor = vec4(fragColor, 1.0);

  // color based on pixel position
  // outColor = vec4(gl_FragCoord.x / 800, gl_FragCoord.y / 600, 0.0, 1.0);

  // use the supplied texture
  outColor = texture(texSampler, fragTexCoord);

  // merge texture and color
  // outColor = vec4(fragColor * texture(texSampler, fragTexCoord).rgb, 1.0);
}

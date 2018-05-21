#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
layout(std140, binding = 0) uniform buf
{
  vec4 position[3];
  vec4 color[3];
} ubuf;

layout(location = 0) out vec4 color;

void main()
{
  color = ubuf.color[gl_VertexIndex];
  gl_Position = ubuf.position[gl_VertexIndex];
}

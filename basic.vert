#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
layout(std140, binding = 0) uniform buf
{
  vec4 position[3];
} ubuf;

void main()
{
  gl_Position = ubuf.position[gl_VertexIndex];
}

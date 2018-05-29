#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
layout(std140, binding = 0) uniform buf
{
  mat4 obj_to_world;
  mat4 world_to_view;
  mat4 view_to_clip;
} ubuf;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec4 in_color;
layout(location = 0) out vec4 out_color;

void main()
{
  out_color = in_color;
  gl_Position = vec4(in_position, 1.0f);
}

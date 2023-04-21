#version 410 core

layout (location = 0) in vec3 pos_in;

out vec2 test_texture_coords;

uniform mat4 PVM;

uniform float ground_scale;

void main()
{
    test_texture_coords = ((pos_in.xy + 1.0f) / 2.0f) * ground_scale;
    gl_Position = PVM * vec4(pos_in, 1.0f);
}


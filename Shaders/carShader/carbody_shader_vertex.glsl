#version 410 core

layout (location = 0) in vec3 pos_in;
layout (location = 2) in vec3 normal_in;

uniform mat4 PVM;

void main()
{
    gl_Position = PVM * vec4(pos_in, 1.0);
}


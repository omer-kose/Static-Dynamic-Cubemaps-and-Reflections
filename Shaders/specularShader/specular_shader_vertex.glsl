#version 410 core

layout (location = 0) in vec3 pos_in;
layout (location = 2) in vec3 normal_in;

uniform mat4 model;
uniform mat4 PVM;
uniform mat3 normal_transformation;

out vec3 normal;
out vec3 frag_pos;

void main()
{
    frag_pos = vec3(model * vec4(pos_in, 1.0f));
    normal = normal_transformation * normal_in;
    
    gl_Position = PVM * vec4(pos_in, 1.0);
}



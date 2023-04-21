#version 410 core

in vec2 test_texture_coords;

out vec4 FragColor;

uniform sampler2D ground_texture;

void main()
{
    FragColor = texture(ground_texture, test_texture_coords);
}


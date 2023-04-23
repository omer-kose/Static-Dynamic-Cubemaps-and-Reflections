#version 410 core

in vec3 frag_pos;
in vec3 normal;

out vec4 FragColor;

uniform vec3 light_pos;
uniform vec3 light_color;
uniform vec3 view_pos;

uniform vec3 objectColor;


void main()
{
    // ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * light_color;
      
    // diffuse
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(light_pos - frag_pos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * light_color;
    
    vec3 result = (ambient + diffuse) * objectColor;
    FragColor = vec4(result, 1.0f);
    
    //FragColor = texture(environment_map, normal);
}




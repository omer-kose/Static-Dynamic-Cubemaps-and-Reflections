#version 410 core

in vec3 frag_pos;
in vec3 normal;

out vec4 FragColor;

uniform vec3 light_pos;
uniform vec3 light_color;
uniform vec3 view_pos;

uniform samplerCube environment_map;

void main()
{
    vec3 objectColor = vec3(0.75);
    // ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * light_color;
      
    // diffuse
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(light_pos - frag_pos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * light_color;
    
    // specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(view_pos - frag_pos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * light_color;
        
    //vec3 result = (ambient + diffuse + specular) * objectColor;
    
    vec3 reflectedView = reflect(-viewDir, norm);
    vec4 reflectedColor = texture(environment_map, reflectedView);
    vec3 result = (ambient + diffuse) * objectColor;
    result = mix(result, vec3(reflectedColor), 0.15) + specular * objectColor;
    FragColor = vec4(result, 1.0f);
    
    //FragColor = texture(environment_map, normal);
}


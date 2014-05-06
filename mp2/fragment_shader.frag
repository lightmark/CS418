#version 330 core

in vec3 vertex_norm;
in vec4 vertex_world;
uniform mat3 M_inv;
uniform mat4 V_inv;

out vec4 outColor;

uniform struct Light {
    vec3 direction;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
} light;

uniform struct Material {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float shininess;
} material;

void main()
{
    //calculate normal in world coordinates
    vec3 viewPosition = vec3(V_inv * vec4(0.0, 0.0, 0.0, 1.0));
    vec3 viewDirection = normalize(viewPosition - vec3(vertex_world));
    //vec3 viewDirection = vec3(0,0,1);
    vec3 normal = normalize(transpose(M_inv)* vertex_norm);

    //calculate the location of this fragment (pixel) in world coordinates
    vec3 surfaceToLight = normalize(-light.direction);

    vec4 diffuseReflection = max(dot(surfaceToLight, normal),0.0) * light.diffuse * material.diffuse;
    vec4 ambientReflection = light.ambient * material.ambient;
    vec4 specularReflection;
    if (dot(normal, -light.direction) < 0.0) // light source on the wrong side?
    {
        specularReflection = vec4(0.0, 0.0, 0.0, 0.0); // no specular reflection
    }
    else // light source on the right side
    {
        specularReflection = light.specular * material.specular * pow(max(0.0, dot(reflect(light.direction, normal), viewDirection)), material.shininess);
    }

    outColor = ambientReflection + diffuseReflection + specularReflection;

    if (viewPosition.z < 0.0) {
        outColor = outColor * vec4(0.4,0.4,1.0,1.0);
    }
}


#version 330 core

in vec3 vertex_world;
//in vec3 vertex_norm;
in vec2 Texcoord;
in mat3 TBN;

out vec4 outColor;
uniform sampler2D surface;
uniform sampler2D env;
uniform sampler2D normal_map;

uniform mat3 M_inv;
uniform mat4 V_inv;
void main()
{
    //calculate normal in world coordinates
    vec3 lightPosition = vec3(1,1,1);
    vec3 viewPosition = vec3(V_inv * vec4(0.0, 0.0, 0.0, 1.0));
    vec3 viewDirection = normalize(viewPosition - vertex_world);
    vec3 normal_tangentspace =  normalize(texture(normal_map, Texcoord).rgb*2.0 - 1.0);
    vec3 normal_modelspace = normalize(TBN * normal_tangentspace);
    vec3 normal = normalize(transpose(M_inv) * normal_modelspace);

    //calculate the location of this fragment (pixel) in world coordinates
    vec3 surfaceToLight = normalize(lightPosition - vertex_world);

    vec4 diffuseReflection = max(dot(surfaceToLight, normal),0.0) * vec4(0.7, 0.7, 0.7, 0.7);
    vec4 ambientReflection = vec4(0.4, 0.4, 0.4, 0.4);
    vec4 specularReflection;
    if (dot(normal, surfaceToLight) < 0.0) // light source on the wrong side?
    {
        specularReflection = vec4(0.0, 0.0, 0.0, 0.0); // no specular reflection
    }
    else // light source on the right side
    {
        specularReflection = vec4(1, 1, 1, 1) * pow(max(0.0, dot(reflect(-surfaceToLight, normal), viewDirection)), 0.8);
    }

    vec4 color = ambientReflection + diffuseReflection + specularReflection;
    vec4 reflection = texture(env, vec2(normal.x/2 + 0.5, 1 - ((normal.y)/2 + 0.5)));
    outColor = texture(surface, Texcoord) * reflection * color;

}


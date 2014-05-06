#version 330 core

in vec3 position;
in vec3 norm;
in vec2 texcoord;
in vec3 tangent;
in vec3 bitangent;

out vec3 vertex_world;
out vec2 Texcoord;
//out vec3 vertex_norm;
out mat3 TBN;

uniform mat4 MVP;
uniform mat4 M;


void main()
{
    gl_Position = MVP * vec4(position, 1.0);
    vertex_world = vec3(M * vec4(position, 1.0));
    Texcoord = texcoord;
    //vertex_norm = norm;
    TBN = mat3(tangent, bitangent, norm);
}

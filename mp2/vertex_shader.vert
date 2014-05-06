#version 330 core

in vec3 position;
in vec3 norm;

uniform mat4 MVP;
uniform mat4 M;

out vec3 vertex_norm;
out vec4 vertex_world;

void main()
{
    gl_Position = MVP * vec4(position, 1.0);
    vertex_norm = norm;
    vertex_world = M * vec4(position, 1.0);
}


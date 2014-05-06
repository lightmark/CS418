#version 330 core

in vec3 position;
uniform mat4 MVP;
uniform float time;

void main()
{
    float PI = 3.141592627;
    float newY = 0, newZ  = 0;
    if (position.x == -0.6) {
        newY = position.y + 0.2 * sin(time * 2*PI);
    }
    if (position.x == -0.2) {
        newY = position.y + 0.2 * sin(time * 2*PI + PI/2);
    }
    if (position.x == 0.2) {
        newY = position.y + 0.2 * sin(time * 2*PI + PI);
    }
    if (position.x == 0.6) {
        newY = position.y + 0.2 * sin(time * 2*PI - PI/2);
    }

    //if (position.x == -0.6) {
    //    newZ = position.z + 0.2 * cos(time * 2*PI);
    //}
    //if (position.x == -0.2) {
    //    newZ = position.z + 0.2 * cos(time * 2*PI + PI/2);
    //}
    //if (position.x == 0.2) {
    //    newZ = position.z + 0.2 * cos(time * 2*PI + PI);
    //}
    //if (position.x == 0.6) {
    //    newZ = position.z + 0.2 * cos(time * 2*PI - PI/2);
    //}
    gl_Position = MVP * vec4(position.x, newY, position.z , 1.0);
}

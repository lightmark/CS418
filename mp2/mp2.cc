#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <ctime>
#include <cmath>
#include "shader.h"
#include "mp2.h"

#define PI 3.14159265

GLfloat sealevel;
static float speed = 0.005;
static int nFPS = 30;
static float fAspect = 1;
static glm::vec3 forwardVector = glm::vec3(-1.0f, 0.0f ,0.0f);
static glm::vec3 upVector = glm::vec3(0.0f, 0.0f, 1.0f);
static glm::mat4 viewMat;

// error callback function
static void error_callback(int error, const char* description)
{
    fputs(description, stderr);
}

// window resize callback function
static void window_reshape_callback(GLFWwindow *window, int width, int height) {
    fAspect = width / height;
}

// key event callback function
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    glm::vec3 axis;
    switch (key) {
        case GLFW_KEY_ESCAPE:
            if (action == GLFW_PRESS)
                glfwSetWindowShouldClose(window, GL_TRUE);
            break;
        case GLFW_KEY_UP:
            if (action == GLFW_REPEAT || action == GLFW_PRESS)
                viewMat = glm::rotate(glm::mat4(), -5.0f, glm::vec3(1, 0, 0)) * viewMat;
            break;
        case GLFW_KEY_DOWN:
            if (action == GLFW_REPEAT || action == GLFW_PRESS)
                viewMat = glm::rotate(glm::mat4(), 5.0f, glm::vec3(1, 0, 0)) * viewMat;
            break;
        case GLFW_KEY_LEFT:
            if (action == GLFW_REPEAT || action == GLFW_PRESS)
                viewMat = glm::rotate(glm::mat4(), -5.0f, glm::vec3(0, 1, 0)) * viewMat;
            break;
        case GLFW_KEY_RIGHT:
            if (action == GLFW_REPEAT || action == GLFW_PRESS)
                viewMat = glm::rotate(glm::mat4(), 5.0f, glm::vec3(0, 1, 0)) * viewMat;
            break;
        case GLFW_KEY_P:
            if (action == GLFW_PRESS)
                speed = 0.005 - speed;
            break;
    }
}

// make buffers for different targets
static GLuint make_buffer(GLenum target, const void* buffer_data, GLsizei buffer_size) {
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(target, buffer);
    glBufferData(target, buffer_size, buffer_data, GL_STATIC_DRAW);
    return buffer;
}

int main(void)
{
    GLFWwindow* window;
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);
    // Set the Context version to be 3.2+
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    // Create the window
    window = glfwCreateWindow(480, 480, "flight", NULL, NULL);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    // Set the context to be the created window
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
      fprintf(stderr, "Failed to initialize GLEW\n");
      return -1;
    }

    // Begin to program
    // generate terrain data
    makemountain();

    // compile the shader program
    GLuint shaderProgram = LoadShaders("vertex_shader.vert", "fragment_shader.frag");
    glUseProgram(shaderProgram);

    GLuint posAttrib = glGetAttribLocation(shaderProgram, "position");
    GLuint normAttrib = glGetAttribLocation(shaderProgram, "norm");

    // Store the vertex array object which stores the attributes mapping
    GLuint vao[2];
    glGenVertexArrays(2, vao);

    // vao for terrain
    glBindVertexArray(vao[0]);
    // Get the position attribute and enable
    GLuint verts_vbo = make_buffer(GL_ARRAY_BUFFER, verts,res*res*3*sizeof(GLfloat));
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // Get the norm attribute and enable
    GLuint norms_vbo = make_buffer(GL_ARRAY_BUFFER, norms, res*res*3*sizeof(GLfloat));
    glEnableVertexAttribArray(normAttrib);
    glVertexAttribPointer(normAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // Set the element buffer
    GLuint veo = make_buffer(GL_ELEMENT_ARRAY_BUFFER, faces, 6*(res-1)*(res-1)*sizeof(GLuint));

    // make the vertex buffer
    sealevel = 0.0f;
    GLfloat sea_verts[] = {
        -5.0f, -5.0f, sealevel, 0.0f, 0.0f, 1.0f,
         5.0f, -5.0f, sealevel, 0.0f, 0.0f, 1.0f,
        -5.0f,  5.0f, sealevel, 0.0f, 0.0f, 1.0f,
         5.0f,  5.0f, sealevel, 0.0f, 0.0f, 1.0f
    };

    // vao for sea
    glBindVertexArray(vao[1]);
    GLuint sea_vbo = make_buffer(GL_ARRAY_BUFFER, sea_verts, sizeof(sea_verts));
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 6*sizeof(GLfloat), 0);
    glEnableVertexAttribArray(normAttrib);
    glVertexAttribPointer(normAttrib, 3, GL_FLOAT, GL_FALSE, 6*sizeof(GLfloat), (GLvoid*)(3*sizeof(GLfloat)));
    glBindVertexArray(0);


    // Projection matrix : 90° Field of View, 1:1 ratio, display range : 0.1 unit <-> 10 units
    glm::mat4 projMat = glm::perspective(90.0f, fAspect, 0.01f, 10.0f);
    // Camera matrix
    glm::vec3 planePosition = glm::vec3(0.5,0,0.5);
    viewMat = glm::lookAt(planePosition, planePosition + forwardVector, upVector);

    glm::mat4 modelMat = glm::mat4(1.0f);
    glm::mat3 modelMatInv = glm::inverse(glm::mat3(modelMat));

    // Get all the uniform identifier in our shader program
    GLuint MVPUniform = glGetUniformLocation(shaderProgram, "MVP");
    GLuint M_invUniform = glGetUniformLocation(shaderProgram, "M_inv");
    GLuint V_invUniform = glGetUniformLocation(shaderProgram, "V_inv");
    //GLuint timeUniform = glGetUniformLocation(shaderProgram, "time");

    GLuint MUniform = glGetUniformLocation(shaderProgram, "M");
    glUniformMatrix4fv(MUniform, 1, GL_FALSE, glm::value_ptr(modelMat));
    glUniformMatrix3fv(M_invUniform, 1, GL_FALSE, glm::value_ptr(modelMatInv));

    // light
    GLuint light_direction_uniform = glGetUniformLocation(shaderProgram, "light.direction");
    GLuint light_ambient_uniform = glGetUniformLocation(shaderProgram, "light.ambient");
    GLuint light_diffuse_uniform = glGetUniformLocation(shaderProgram, "light.diffuse");
    GLuint light_specular_uniform = glGetUniformLocation(shaderProgram, "light.specular");

    glm::vec3 direction = glm::vec3(0.0,0.0,-1.0);
    glm::vec4 amb = glm::vec4(1.0,1.0,1.0,1.0);
    glm::vec4 diff = glm::vec4(1.0,1.0,1.0,1.0);
    glm::vec4 spec = glm::vec4(1.0,1.0,1.0,1.0);

    glUniform3fv(light_direction_uniform, 1, glm::value_ptr(direction));
    glUniform4fv(light_ambient_uniform, 1, glm::value_ptr(amb));
    glUniform4fv(light_diffuse_uniform, 1, glm::value_ptr(diff));
    glUniform4fv(light_specular_uniform, 1, glm::value_ptr(spec));

    // dirt doesn't glisten
    glm::vec4 tanamb  = glm::vec4(0.2,0.15,0.1,1.0);
    glm::vec4 tandiff = glm::vec4(0.4,0.3,0.2,1.0);
    glm::vec4 tanspec = glm::vec4(0.4,0.3,0.2,1.0);
    GLfloat tanshininess = 50.0;

    // Single polygon, will only have highlight if light hits a vertex just right
    glm::vec4 seaamb  = glm::vec4(0.0,0.0,0.2,0.2);
    glm::vec4 seadiff = glm::vec4(0.0,0.0,0.8,0.2);
    glm::vec4 seaspec = glm::vec4(0.5,0.5,1.0,0.4);
    GLfloat seashininess = 10.0;

    GLuint material_shininess_uniform = glGetUniformLocation(shaderProgram, "material.shininess");
    GLuint material_ambient_uniform = glGetUniformLocation(shaderProgram, "material.ambient");
    GLuint material_diffuse_uniform = glGetUniformLocation(shaderProgram, "material.diffuse");
    GLuint material_specular_uniform = glGetUniformLocation(shaderProgram, "material.specular");

    GLfloat fRotateAngle = 1.0f;
    clock_t startClock=0,curClock;
    float time = 0;
    // Enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // configurations
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearColor(0.5, 0.5, 1.0, 0.0);	// sky

    //rendering
    while (!glfwWindowShouldClose(window)) {
        // Clear the color buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        curClock=clock();

	float elapsed=(curClock-startClock)/(float)CLOCKS_PER_SEC;
	if(elapsed>1/nFPS){
		startClock=curClock;
                // translate the view coordinate
                viewMat = glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, 0.0f, speed)) * viewMat;
	}

        // Our ModelViewProjection : multiplication of our 3 matrices
        glm::mat4 MVPMat = projMat * viewMat * modelMat;
        glUniformMatrix4fv(MVPUniform, 1, GL_FALSE, glm::value_ptr(MVPMat));
        glm::mat4 V_inv = glm::inverse(viewMat);
        glUniformMatrix4fv(V_invUniform, 1, GL_FALSE, glm::value_ptr(V_inv));

        // Begin to draw all the polygons
        glBindVertexArray(vao[0]);
        glUniform1f(material_shininess_uniform, tanshininess);
        glUniform4fv(material_ambient_uniform, 1, glm::value_ptr(tanamb));
        glUniform4fv(material_diffuse_uniform, 1, glm::value_ptr(tandiff));
        glUniform4fv(material_specular_uniform, 1, glm::value_ptr(tanspec));
        glDrawElements(GL_TRIANGLES, 6*(res-1)*(res-1), GL_UNSIGNED_INT, 0);

        glBindVertexArray(vao[1]);
        glUniform1f( material_shininess_uniform, seashininess);
        glUniform4fv(material_ambient_uniform, 1, glm::value_ptr(seaamb));
        glUniform4fv(material_diffuse_uniform, 1, glm::value_ptr(seadiff));
        glUniform4fv(material_specular_uniform, 1, glm::value_ptr(seaspec));
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        // buffer swapping
        glfwSwapBuffers(window);
        // Poll events
        glfwPollEvents();
    }

    // clean
    glDeleteProgram(shaderProgram);
    glDeleteBuffers(1, &verts_vbo);
    glDeleteBuffers(1, &norms_vbo);
    glDeleteBuffers(1, &sea_vbo);
    glDeleteVertexArrays(2, vao);
    glfwDestroyWindow(window);
    glfwTerminate();

    exit(EXIT_SUCCESS);
}

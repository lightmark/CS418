#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <ctime>
#include <cmath>
#include "shader.h"

#define PI 3.14159265

int nFPS = 30;
bool onlyEdge = false;
float fAspect = 1;

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
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    else if (key == GLFW_KEY_E && (action == GLFW_REPEAT || action == GLFW_PRESS))
        onlyEdge = true;
    else
        onlyEdge = false;
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

    // Initialize GLFW
    if (!glfwInit())
        exit(EXIT_FAILURE);
    // Set the Context version to be 3.2+
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create the window
    window = glfwCreateWindow(480, 480, "Dancing I", NULL, NULL);
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
    // Store the vertex array object which stores the attributes mapping
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // make the vertices buffer
    GLfloat IBufferData[] = {
                             //       1                    2
        -0.6f,  0.6f,  0.1f, //0       ********************
        -0.6f,  1.0f,  0.1f, //1       *                  *
         0.6f,  1.0f,  0.1f, //2       *******      *******
         0.6f,  0.6f,  0.1f, //3      0    11*      *4     3
         0.2f,  0.6f,  0.1f, //4             *      *
         0.2f, -0.6f,  0.1f, //5             *      *
         0.6f, -0.6f,  0.1f, //6             *      *
         0.6f, -1.0f,  0.1f, //7             *      *
        -0.6f, -1.0f,  0.1f, //8      9    10*      *5     6
        -0.6f, -0.6f,  0.1f, //9       *******      *******
        -0.2f, -0.6f,  0.1f, //10      *                  *
        -0.2f,  0.6f,  0.1f, //11      ********************
                             //       8                    7
                             //
                             //      13                    14
        -0.6f,  0.6f, -0.1f, //12      ********************
        -0.6f,  1.0f, -0.1f, //13      *                  *
         0.6f,  1.0f, -0.1f, //14      *******      *******
         0.6f,  0.6f, -0.1f, //15    12    23*      *16    15
         0.2f,  0.6f, -0.1f, //16            *      *
         0.2f, -0.6f, -0.1f, //17            *      *
         0.6f, -0.6f, -0.1f, //18            *      *
         0.6f, -1.0f, -0.1f, //19            *      *
        -0.6f, -1.0f, -0.1f, //20    21    22*      *17    18
        -0.6f, -0.6f, -0.1f, //21      *******      *******
        -0.2f, -0.6f, -0.1f, //22      *                  *
        -0.2f,  0.6f, -0.1f  //23      ********************
                             //      20                    19
    };


    // make the vertex buffer
    GLuint VBO = make_buffer(GL_ARRAY_BUFFER, IBufferData, sizeof(IBufferData));

    // Set the element buffer
    GLuint elementBufferData[] = {
        0, 1, 11, 2, 4, 3,
        4, 11, 10, 5,
        6, 7, 5, 8, 10, 9,
        12, 13, 23, 14, 16, 15,
        16, 23, 22, 17,
        18, 19, 17, 20, 22, 21,

        0, 1, 12, 13,
        1, 2, 13, 14,
        2, 3, 14, 15,
        3, 4, 15, 16,
        4, 5, 16, 17,
        5, 6, 17, 18,
        6, 7, 18, 19,
        7, 8, 19, 20,
        8, 9, 20, 21,
        9, 10, 21, 22,
        10, 11, 22, 23,
        11, 0, 23, 0

    };
    GLuint veo = make_buffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferData, sizeof(elementBufferData));

    // compile the shader program
    GLuint shaderProgram = LoadShaders("vertex_shader.vert", "fragment_shader.frag");
    glUseProgram(shaderProgram);

    // Get the position attribute and enable
    GLuint posAttrib = glGetAttribLocation(shaderProgram, "position");
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(posAttrib);

    // Projection matrix : 45° Field of View, 1:1 ratio, display range : 0.1 unit <-> 100 units
    glm::mat4 projMat = glm::perspective(45.0f, fAspect, 0.1f, 100.0f);
    // Camera matrix
    glm::mat4 viewMat = glm::lookAt(
            glm::vec3(0,0,3), // Camera is at (0,0,3), in World Space
            glm::vec3(0,0,0), // and looks at the origin
            glm::vec3(0,1,0)  // +Y is up
            );
    // Model matrix : a scale matrix (0.7)
    glm::mat4 modelMat  = glm::scale(
            glm::mat4(1.0f),
            glm::vec3(0.7f, 0.7f, 0.7f)
            );

    // Get all the uniform identifier in our shader program
    GLuint MVPUniform = glGetUniformLocation(shaderProgram, "MVP");
    GLuint colorUniform = glGetUniformLocation(shaderProgram, "color");
    GLuint timeUniform = glGetUniformLocation(shaderProgram, "time");
    // Set the skeleton line width
    glLineWidth(2.5f);

    // Enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    GLfloat fRotateAngle = 1.0f;
    clock_t startClock=0,curClock;
    float time = 0;

    while (!glfwWindowShouldClose(window)) {
        // Clear the color buffer
        glClearColor(0.0f, 0.2f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        curClock=clock();

        // Rotate I versus y-axis
	float elapsed=(curClock-startClock)/(float)CLOCKS_PER_SEC;
	if(elapsed>1/nFPS){
		startClock=curClock;
                modelMat = glm::rotate(
                    modelMat,
                    fRotateAngle,
                    glm::vec3(0.0f, 1.0f, 0.0f)
                    );
                //set the time value for sine function
                time = (time + 0.01) -(int)(time +0.01);
                glUniform1f(timeUniform, time);
	}


        projMat = glm::perspective(45.0f, fAspect, 0.1f, 100.0f);
        // Our ModelViewProjection : multiplication of our 3 matrices
        glm::mat4 MVPMat = projMat * viewMat * modelMat;
        glUniformMatrix4fv(MVPUniform, 1, GL_FALSE, glm::value_ptr(MVPMat));

        // choose from two modes: line or fill
        if (onlyEdge == true) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            // White color
            glUniform4f(colorUniform, 1.0f, 1.0f, 1.0f, 1.0f);
            }
        else {
            // Orange color
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glUniform4f(colorUniform, 1.0f, 0.5f, 0.0f, 0.6f);
        }

        // Begin to draw all the polygons
        glDrawElements(GL_TRIANGLE_STRIP, 6, GL_UNSIGNED_INT, 0);
        glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, (GLvoid*)(6* sizeof(GLuint)));
        glDrawElements(GL_TRIANGLE_STRIP, 6, GL_UNSIGNED_INT, (GLvoid*)(10* sizeof(GLuint)));
        glDrawElements(GL_TRIANGLE_STRIP, 6, GL_UNSIGNED_INT,  (GLvoid*)(16* sizeof(GLuint)));
        glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, (GLvoid*)(22* sizeof(GLuint)));
        glDrawElements(GL_TRIANGLE_STRIP, 6, GL_UNSIGNED_INT, (GLvoid*)(26* sizeof(GLuint)));

        for (int i = 0; i != 12; i++) {
            glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, (GLvoid*)((32+ 4*i)* sizeof(GLuint)));
        }

        // buffer swapping
        glfwSwapBuffers(window);
        // Poll events
        glfwPollEvents();
    }

    // clean
    glDisableVertexAttribArray(posAttrib);
    glDeleteProgram(shaderProgram);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &vao);
    glfwDestroyWindow(window);
    glfwTerminate();

    exit(EXIT_SUCCESS);
}

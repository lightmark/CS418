#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <stdio.h>
#include <ctime>
#include <cmath>
#include "soil.h"
#include "shader.h"

#define PI 3.14159265

static int nFPS = 30;
static float fAspect = 1;
static bool pause = false;
static glm::mat4 viewMat;
static glm::mat4 modelMat;

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
        case GLFW_KEY_F:
            if (action == GLFW_REPEAT || action == GLFW_PRESS)
                viewMat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.05f)) * viewMat;
            break;
        case GLFW_KEY_B:
            if (action == GLFW_REPEAT || action == GLFW_PRESS)
                viewMat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -0.05f)) * viewMat;
            break;
        case GLFW_KEY_P:
            if (action == GLFW_REPEAT || action == GLFW_PRESS)
                pause = !pause;
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

// load the obj file
void load_obj(const char* filename, std::vector<glm::vec3> &vertices, std::vector<unsigned int> &vertex_indices)
{
    std::ifstream obj_file(filename);

    if (!obj_file)
    {
        std::cerr << "error: unable to open the obj file:" << filename << std::endl;
        exit(1);
    }

    std::string line;
    while (std::getline(obj_file, line))
    {
        std::istringstream stream(line);
        char mode = '\0';

        stream >> mode;
        if (mode == 'v')
        {
            glm::vec3 vertex;
            stream >> vertex.x >> vertex.y >> vertex.z;
            vertices.push_back(vertex);
        }
        else if (mode == 'f')
        {
            GLuint vertex_index[3];
            stream >> vertex_index[0] >> vertex_index[1] >> vertex_index[2];
            vertex_indices.push_back(vertex_index[0]);
            vertex_indices.push_back(vertex_index[1]);
            vertex_indices.push_back(vertex_index[2]);
        }
    }
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
    window = glfwCreateWindow(480, 480, "teapot", NULL, NULL);
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
    std::vector<glm::vec3> vertices;
    std::vector<unsigned int> indices;
    load_obj("teapot_0.obj", vertices, indices);
    glm::vec3 *normals = new glm::vec3[vertices.size()];

    // initialization
    for (int i = 0; i != vertices.size(); i++)
        normals[i] = glm::vec3(0);
    // calculate the normals
    for (int i = 0; i != indices.size(); i += 3)
    {
        glm::vec3 a = vertices[indices[i] - 1];
        glm::vec3 b = vertices[indices[i + 1] - 1];
        glm::vec3 c = vertices[indices[i + 2] - 1];
        glm::vec3 normal = glm::normalize(glm::cross(b - a, c - a));
        normals[indices[i] - 1] += normal;
        normals[indices[i + 1] - 1] += normal;
        normals[indices[i + 2] - 1] += normal;
    }
    for (int i = 0; i != vertices.size(); i++)
        normals[i] = glm::normalize(normals[i]);

    // get the maximum y value
    GLfloat max_y = 0;
    for (std::vector<glm::vec3>::iterator iter = vertices.begin(); iter != vertices.end(); iter++)
    {
        if (iter->y > max_y)
            max_y = iter->y;
    }
    GLfloat *verts = new GLfloat[vertices.size()*8];
    // create vbo buffer data
    for (std::vector<glm::vec3>::size_type i = 0; i != vertices.size(); i++)
    {
        int index = i*8;
        verts[index] = vertices[i].x;
        verts[index + 1] = vertices[i].y;
        verts[index + 2] = vertices[i].z;
        verts[index + 3] = normals[i].x;
        verts[index + 4] = normals[i].y;
        verts[index + 5] = normals[i].z;
        verts[index + 6] = atan2(vertices[i].z, vertices[i].x) / (2* PI) + 0.5;
        verts[index + 7] = vertices[i].y / max_y;
    }
    delete normals;

    // prepare for element buffer
    GLuint *faces = new GLuint[indices.size()];
    for (std::vector<glm::vec3>::size_type i = 0; i != indices.size(); i++)
    {
        faces[i] = indices[i] - 1;
    }

    // normal mapping
    glm::vec3 *tangents = new glm::vec3[vertices.size()];
    glm::vec3 *bitangents = new glm::vec3[vertices.size()];
    for (int i = 0; i != vertices.size(); i++)
        tangents[i] = bitangents[i] = glm::vec3(0);
    for (int i = 0; i != indices.size(); i+=3)
    {
        // Shortcuts for vertices
        glm::vec3 & v0 = vertices[faces[i]];
        glm::vec3 & v1 = vertices[faces[i+1]];
        glm::vec3 & v2 = vertices[faces[i+2]];

        // Shortcuts for UVs
        glm::vec2 uv0 = glm::vec2(verts[faces[i]*8 + 6], verts[faces[i]*8 + 7]);
        glm::vec2 uv1 = glm::vec2(verts[faces[i+1]*8 + 6], verts[faces[i+1]*8 + 7]);
        glm::vec2 uv2 = glm::vec2(verts[faces[i+2]*8 + 6], verts[faces[i+2]*8 + 7]);

        // Edges of the triangle : postion delta
        glm::vec3 deltaPos1 = v1-v0;
        glm::vec3 deltaPos2 = v2-v0;

        // UV delta
        glm::vec2 deltaUV1 = uv1-uv0;
        glm::vec2 deltaUV2 = uv2-uv0;

        float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
        glm::vec3 tangent = (deltaPos1 * deltaUV2.y   - deltaPos2 * deltaUV1.y)*r;
        glm::vec3 bitangent = (deltaPos2 * deltaUV1.x   - deltaPos1 * deltaUV2.x)*r;

        tangents[faces[i]] += tangent;
        bitangents[faces[i]] += bitangent;
    }

    //GLfloat *tangent_buffer = new GLfloat[vertices.size() * 3];
    //GLfloat *bitangent_buffer = new GLfloat[vertices.size() * 3];
    for (int i = 0; i != vertices.size(); i++ )
    {
        tangents[i] = glm::normalize(tangents[i]);
        bitangents[i] = glm::normalize(bitangents[i]);
        //tangent_buffer[3*i] = t.x;
        //tangent_buffer[3*i + 1] = t.y;
        //tangent_buffer[3*i + 2] = t.z;
        //bitangent_buffer[3*i] = b.x;
        //bitangent_buffer[3*i + 1] = b.y;
        //bitangent_buffer[3*i + 2] = b.z;
    }

    // compile the shader program
    GLuint shaderProgram = LoadShaders("vertex_shader.vert", "fragment_shader.frag");
    glUseProgram(shaderProgram);



    // Store the vertex array object which stores the attributes mapping
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Get the position attribute and enable
    GLuint vbo = make_buffer(GL_ARRAY_BUFFER, verts, 8*vertices.size()*sizeof(GLfloat));
    delete verts;
    GLuint posAttrib = glGetAttribLocation(shaderProgram, "position");
    GLuint normAttrib = glGetAttribLocation(shaderProgram, "norm");
    GLuint texAttrib = glGetAttribLocation(shaderProgram, "texcoord");
    glEnableVertexAttribArray(posAttrib);
    glEnableVertexAttribArray(normAttrib);
    glEnableVertexAttribArray(texAttrib);
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), 0);
    glVertexAttribPointer(normAttrib, 3, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), (void*)(3*sizeof(GLfloat)));
    glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), (void*)(6*sizeof(GLfloat)));

    GLuint tangent_buffer = make_buffer(GL_ARRAY_BUFFER, tangents, vertices.size() * sizeof(glm::vec3));
    GLuint tangentAttrib = glGetAttribLocation(shaderProgram, "tangent");
    glEnableVertexAttribArray(tangentAttrib);
    glVertexAttribPointer(tangentAttrib, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    GLuint bitangent_buffer = make_buffer(GL_ARRAY_BUFFER, bitangents, vertices.size() * sizeof(glm::vec3));
    GLuint bitangentAttrib = glGetAttribLocation(shaderProgram, "bitangent");
    glEnableVertexAttribArray(bitangentAttrib);
    glVertexAttribPointer(bitangentAttrib, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // Set the element buffer
    GLuint veo = make_buffer(GL_ELEMENT_ARRAY_BUFFER, faces, indices.size()*sizeof(GLuint));
    delete faces;

    // Load texture
    GLuint tex[3];
    glGenTextures(3, tex);

    int width, height;
    unsigned char* image;

    // load the textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex[0]);
    image = SOIL_load_image("qinghua.jpg", &width, &height, 0, SOIL_LOAD_RGB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    SOIL_free_image_data(image);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glUniform1i(glGetUniformLocation(shaderProgram, "surface"), 0);

    // the texture from environment mapping
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, tex[1]);
    image = SOIL_load_image("sphere.jpg", &width, &height, 0, SOIL_LOAD_RGB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    SOIL_free_image_data(image);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glUniform1i(glGetUniformLocation(shaderProgram, "env"), 1);

    // normal mapping texture
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, tex[2]);
    image = SOIL_load_image("normal.jpg", &width, &height, 0, SOIL_LOAD_RGB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    SOIL_free_image_data(image);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glUniform1i(glGetUniformLocation(shaderProgram, "normal_map"), 2);

    // Projection matrix : 90° Field of View, 1:1 ratio, display range : 0.01 unit <-> 10 units
    glm::vec3 upVector = glm::vec3(0, 1, 0);
    glm::mat4 projMat = glm::perspective(90.0f, fAspect, 0.01f, 10.0f);
    viewMat = glm::lookAt(glm::vec3(0.0, 0.5,1),
                                    glm::vec3(0.0, 0.3, 0.0),
                                    upVector);
    // Camera matrix
    modelMat = glm::scale(glm::mat4(1.0f),glm::vec3(0.2f));

    // Get all the uniform identifier in our shader program
    GLuint MVPUniform = glGetUniformLocation(shaderProgram, "MVP");
    GLuint M_invUniform = glGetUniformLocation(shaderProgram, "M_inv");
    GLuint V_invUniform = glGetUniformLocation(shaderProgram, "V_inv");
    GLuint MUniform = glGetUniformLocation(shaderProgram, "M");

    GLfloat fRotateAngle = 1.0f;
    clock_t startClock=0,curClock;
    float time = 0;

    // configurations
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearColor(1.0, 1.0, 1.0, 0.0);	// sky

    //rendering
    while (!glfwWindowShouldClose(window)) {
        // Clear the color buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        curClock=clock();

	float elapsed=(curClock-startClock)/(float)CLOCKS_PER_SEC;
	if(elapsed>1/nFPS){
		startClock=curClock;
                // translate the view coordinate
                if (pause == false)
                    modelMat = glm::rotate(modelMat, 1.0f, upVector);
        }

        // Our ModelViewProjection : multiplication of our 3 matrices
        glm::mat4 MVPMat = projMat * viewMat * modelMat;
        glUniformMatrix4fv(MVPUniform, 1, GL_FALSE, glm::value_ptr(MVPMat));
        glm::mat3 inverseModel = glm::inverse(glm::mat3(modelMat));
        glUniformMatrix3fv(M_invUniform, 1, GL_FALSE, glm::value_ptr(inverseModel));
        glm::mat4 inverseView = glm::inverse(viewMat);
        glUniformMatrix4fv(V_invUniform, 1, GL_FALSE, glm::value_ptr(inverseView));
        glUniformMatrix4fv(MUniform, 1, GL_FALSE, glm::value_ptr(modelMat));

        // Begin to draw all the polygons
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

        // buffer swapping
        glfwSwapBuffers(window);
        // Poll events
        glfwPollEvents();
    }

    // clean
    glDeleteProgram(shaderProgram);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &veo);
    glDeleteVertexArrays(1, &vao);
    glfwDestroyWindow(window);
    glfwTerminate();

    exit(EXIT_SUCCESS);
}

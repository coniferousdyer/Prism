#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <stdlib.h>
#include <time.h>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
void reset();
void key_was_pressed(GLFWwindow *window, int key, int scancode, int action, int mods);
void generateColor(float &r, float &g, float &b);

// Settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;
bool OBJECT_SET_TO_ROTATE = false;
bool CAMERA_SET_TO_REVOLVE = false;
bool PREVIOUS_WAS_TRANSLATE = false;

glm::mat4 model = glm::mat4(1.0f);
glm::mat4 view;
glm::vec3 origin = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 c = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraPos = glm::vec3(c.x, c.y, c.z + 3.0f);
glm::vec3 cameraTarget = glm::vec3(c.x, c.y, c.z);
glm::vec3 cameraFront = cameraTarget - cameraPos;
glm::vec3 cameraUp = glm::vec3(c.x, c.y + 1.0f, c.z);
glm::mat4 identity = glm::mat4(1.0f);
float angle = 0.0f;

const char *vertexShaderSource = "#version 330 core\n"
                                 "layout (location = 0) in vec3 aPos;\n"
                                 "layout (location = 1) in vec3 aColor;\n"
                                 "uniform mat4 model;\n"
                                 "uniform mat4 view;\n"
                                 "uniform mat4 projection;\n"
                                 "out vec3 inColor;\n"
                                 "void main()\n"
                                 "{\n"
                                 "   gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
                                 "   inColor = aColor;\n"
                                 "}\0";

const char *fragmentShaderSource = "#version 330 core\n"
                                   "out vec4 FragColor;\n"
                                   "in vec3 inColor;\n"
                                   "void main()\n"
                                   "{\n"
                                   "   FragColor = vec4(inColor, 1.0f);\n"
                                   "}\n\0";

int main(int argc, char *argv[])
{
    srand(time(0));
    int n = atoi(argv[1]);

    // GLFW: Initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // GLFW window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Assignment 0", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // GLAD: Load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Build and compile our shader program
    // ------------------------------------
    // Vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // Check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }
    // Fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // Check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }

    // Link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                  << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float points[2 * n][3];
    float vertices[36 * (n - 2) + 36 * n];
    float r, g, b;

    // Storing points
    for (int i = 0; i < n; i++)
    {
        points[i][0] = c.x + cos(2 * M_PI * i / n) * 0.5f;
        points[i][1] = c.y + sin(2 * M_PI * i / n) * 0.5f;
        points[i][2] = c.z + 0.5f;
    }

    for (int i = 0; i < n; i++)
    {
        points[n + i][0] = c.x + cos(2 * M_PI * i / n) * 0.5f;
        points[n + i][1] = c.y + sin(2 * M_PI * i / n) * 0.5f;
        points[n + i][2] = c.z - 0.5f;
    }

    // Generating figure
    generateColor(r, g, b);

    for (int i = 0; i < n - 2; i++)
    {
        vertices[18 * i] = points[0][0];
        vertices[18 * i + 1] = points[0][1];
        vertices[18 * i + 2] = points[0][2];
        vertices[18 * i + 3] = r;
        vertices[18 * i + 4] = g;
        vertices[18 * i + 5] = b;

        vertices[18 * i + 6] = points[i + 1][0];
        vertices[18 * i + 7] = points[i + 1][1];
        vertices[18 * i + 8] = points[i + 1][2];
        vertices[18 * i + 9] = r;
        vertices[18 * i + 10] = g;
        vertices[18 * i + 11] = b;

        vertices[18 * i + 12] = points[i + 2][0];
        vertices[18 * i + 13] = points[i + 2][1];
        vertices[18 * i + 14] = points[i + 2][2];
        vertices[18 * i + 15] = r;
        vertices[18 * i + 16] = g;
        vertices[18 * i + 17] = b;
    }

    generateColor(r, g, b);

    for (int i = 0; i < n - 2; i++)
    {
        vertices[18 * (n - 2) + 18 * i] = points[n][0];
        vertices[18 * (n - 2) + 18 * i + 1] = points[n][1];
        vertices[18 * (n - 2) + 18 * i + 2] = points[n][2];
        vertices[18 * (n - 2) + 18 * i + 3] = r;
        vertices[18 * (n - 2) + 18 * i + 4] = g;
        vertices[18 * (n - 2) + 18 * i + 5] = b;

        vertices[18 * (n - 2) + 18 * i + 6] = points[i + 1 + n][0];
        vertices[18 * (n - 2) + 18 * i + 7] = points[i + 1 + n][1];
        vertices[18 * (n - 2) + 18 * i + 8] = points[i + 1 + n][2];
        vertices[18 * (n - 2) + 18 * i + 9] = r;
        vertices[18 * (n - 2) + 18 * i + 10] = g;
        vertices[18 * (n - 2) + 18 * i + 11] = b;

        vertices[18 * (n - 2) + 18 * i + 12] = points[i + 2 + n][0];
        vertices[18 * (n - 2) + 18 * i + 13] = points[i + 2 + n][1];
        vertices[18 * (n - 2) + 18 * i + 14] = points[i + 2 + n][2];
        vertices[18 * (n - 2) + 18 * i + 15] = r;
        vertices[18 * (n - 2) + 18 * i + 16] = g;
        vertices[18 * (n - 2) + 18 * i + 17] = b;
    }

    for (int i = 0; i < n - 1; i++)
    {
        generateColor(r, g, b);

        int beg = i, end = i + n + 1;
        vertices[36 * (n - 2) + 36 * i] = points[beg][0];
        vertices[36 * (n - 2) + 36 * i + 1] = points[beg][1];
        vertices[36 * (n - 2) + 36 * i + 2] = points[beg][2];
        vertices[36 * (n - 2) + 36 * i + 3] = r;
        vertices[36 * (n - 2) + 36 * i + 4] = g;
        vertices[36 * (n - 2) + 36 * i + 5] = b;

        vertices[36 * (n - 2) + 36 * i + 6] = points[beg + 1][0];
        vertices[36 * (n - 2) + 36 * i + 7] = points[beg + 1][1];
        vertices[36 * (n - 2) + 36 * i + 8] = points[beg + 1][2];
        vertices[36 * (n - 2) + 36 * i + 9] = r;
        vertices[36 * (n - 2) + 36 * i + 10] = g;
        vertices[36 * (n - 2) + 36 * i + 11] = b;

        vertices[36 * (n - 2) + 36 * i + 12] = points[end][0];
        vertices[36 * (n - 2) + 36 * i + 13] = points[end][1];
        vertices[36 * (n - 2) + 36 * i + 14] = points[end][2];
        vertices[36 * (n - 2) + 36 * i + 15] = r;
        vertices[36 * (n - 2) + 36 * i + 16] = g;
        vertices[36 * (n - 2) + 36 * i + 17] = b;

        vertices[36 * (n - 2) + 36 * i + 18] = points[beg][0];
        vertices[36 * (n - 2) + 36 * i + 19] = points[beg][1];
        vertices[36 * (n - 2) + 36 * i + 20] = points[beg][2];
        vertices[36 * (n - 2) + 36 * i + 21] = r;
        vertices[36 * (n - 2) + 36 * i + 22] = g;
        vertices[36 * (n - 2) + 36 * i + 23] = b;

        vertices[36 * (n - 2) + 36 * i + 24] = points[end - 1][0];
        vertices[36 * (n - 2) + 36 * i + 25] = points[end - 1][1];
        vertices[36 * (n - 2) + 36 * i + 26] = points[end - 1][2];
        vertices[36 * (n - 2) + 36 * i + 27] = r;
        vertices[36 * (n - 2) + 36 * i + 28] = g;
        vertices[36 * (n - 2) + 36 * i + 29] = b;

        vertices[36 * (n - 2) + 36 * i + 30] = points[end][0];
        vertices[36 * (n - 2) + 36 * i + 31] = points[end][1];
        vertices[36 * (n - 2) + 36 * i + 32] = points[end][2];
        vertices[36 * (n - 2) + 36 * i + 33] = r;
        vertices[36 * (n - 2) + 36 * i + 34] = g;
        vertices[36 * (n - 2) + 36 * i + 35] = b;
    }

    generateColor(r, g, b);

    vertices[36 * (n - 2) + 36 * (n - 1)] = points[n - 1][0];
    vertices[36 * (n - 2) + 36 * (n - 1) + 1] = points[n - 1][1];
    vertices[36 * (n - 2) + 36 * (n - 1) + 2] = points[n - 1][2];
    vertices[36 * (n - 2) + 36 * (n - 1) + 3] = r;
    vertices[36 * (n - 2) + 36 * (n - 1) + 4] = g;
    vertices[36 * (n - 2) + 36 * (n - 1) + 5] = b;

    vertices[36 * (n - 2) + 36 * (n - 1) + 6] = points[0][0];
    vertices[36 * (n - 2) + 36 * (n - 1) + 7] = points[0][1];
    vertices[36 * (n - 2) + 36 * (n - 1) + 8] = points[0][2];
    vertices[36 * (n - 2) + 36 * (n - 1) + 9] = r;
    vertices[36 * (n - 2) + 36 * (n - 1) + 10] = g;
    vertices[36 * (n - 2) + 36 * (n - 1) + 11] = b;

    vertices[36 * (n - 2) + 36 * (n - 1) + 12] = points[n][0];
    vertices[36 * (n - 2) + 36 * (n - 1) + 13] = points[n][1];
    vertices[36 * (n - 2) + 36 * (n - 1) + 14] = points[n][2];
    vertices[36 * (n - 2) + 36 * (n - 1) + 15] = r;
    vertices[36 * (n - 2) + 36 * (n - 1) + 16] = g;
    vertices[36 * (n - 2) + 36 * (n - 1) + 17] = b;

    vertices[36 * (n - 2) + 36 * (n - 1) + 18] = points[n - 1][0];
    vertices[36 * (n - 2) + 36 * (n - 1) + 19] = points[n - 1][1];
    vertices[36 * (n - 2) + 36 * (n - 1) + 20] = points[n - 1][2];
    vertices[36 * (n - 2) + 36 * (n - 1) + 21] = r;
    vertices[36 * (n - 2) + 36 * (n - 1) + 22] = g;
    vertices[36 * (n - 2) + 36 * (n - 1) + 23] = b;

    vertices[36 * (n - 2) + 36 * (n - 1) + 24] = points[2 * n - 1][0];
    vertices[36 * (n - 2) + 36 * (n - 1) + 25] = points[2 * n - 1][1];
    vertices[36 * (n - 2) + 36 * (n - 1) + 26] = points[2 * n - 1][2];
    vertices[36 * (n - 2) + 36 * (n - 1) + 27] = r;
    vertices[36 * (n - 2) + 36 * (n - 1) + 28] = g;
    vertices[36 * (n - 2) + 36 * (n - 1) + 29] = b;

    vertices[36 * (n - 2) + 36 * (n - 1) + 30] = points[n][0];
    vertices[36 * (n - 2) + 36 * (n - 1) + 31] = points[n][1];
    vertices[36 * (n - 2) + 36 * (n - 1) + 32] = points[n][2];
    vertices[36 * (n - 2) + 36 * (n - 1) + 33] = r;
    vertices[36 * (n - 2) + 36 * (n - 1) + 34] = g;
    vertices[36 * (n - 2) + 36 * (n - 1) + 35] = b;

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    // Bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);

    glfwSetKeyCallback(window, key_was_pressed);

    // Render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        if (OBJECT_SET_TO_ROTATE)
            angle += 0.05f;

        model = glm::translate(identity, c);
        model = glm::rotate(model, angle, glm::vec3(1.0f, 0.0f, 0.0f));

        if (CAMERA_SET_TO_REVOLVE)
            cameraPos += glm::normalize(glm::cross(cameraTarget - cameraPos, cameraUp)) * 0.05f;

        // Input
        // -----
        processInput(window);

        // Render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (!PREVIOUS_WAS_TRANSLATE)
            view = glm::lookAt(cameraPos, cameraTarget, cameraUp);

        glm::mat4 projection;
        projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_WIDTH, 0.1f, 100.0f);

        // Draw figure
        glUseProgram(shaderProgram);
        unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        unsigned int projectionLoc = glGetUniformLocation(shaderProgram, "projection");
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36 * (n - 2) + 36 * n);

        // GLFW: Swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // De-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    // GLFW: Terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// Generate random RGB values
void generateColor(float &r, float &g, float &b)
{
    r = (float)rand() / RAND_MAX;
    g = (float)rand() / RAND_MAX;
    b = (float)rand() / RAND_MAX;
}

// Snap camera back to centre of prism
void reset()
{
    cameraTarget = c;
}

// Process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    const float cameraSpeed = 0.05f;

    // Part B - 1
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        if (PREVIOUS_WAS_TRANSLATE)
            reset();

        cameraPos += cameraSpeed * glm::vec3(0.0f, 1.0f, 0.0f);
        PREVIOUS_WAS_TRANSLATE = false;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        if (PREVIOUS_WAS_TRANSLATE)
            reset();

        cameraPos -= cameraSpeed * glm::vec3(0.0f, 1.0f, 0.0f);
        PREVIOUS_WAS_TRANSLATE = false;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        if (PREVIOUS_WAS_TRANSLATE)
            reset();

        cameraPos -= cameraSpeed * glm::vec3(1.0f, 0.0f, 0.0f);
        PREVIOUS_WAS_TRANSLATE = false;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        if (PREVIOUS_WAS_TRANSLATE)
            reset();

        cameraPos += cameraSpeed * glm::vec3(1.0f, 0.0f, 0.0f);
        PREVIOUS_WAS_TRANSLATE = false;
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        if (PREVIOUS_WAS_TRANSLATE)
            reset();

        cameraPos += cameraSpeed * glm::vec3(0.0f, 0.0f, 1.0f);
        PREVIOUS_WAS_TRANSLATE = false;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        if (PREVIOUS_WAS_TRANSLATE)
            reset();

        cameraPos -= cameraSpeed * glm::vec3(0.0f, 0.0f, 1.0f);
        PREVIOUS_WAS_TRANSLATE = false;
    }

    // Part B - 2
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
    {
        glm::vec3 currentRight = glm::normalize(glm::cross(cameraTarget - cameraPos, cameraUp));

        model = glm::translate(model, 0.05f * currentRight);
        c += 0.05f * currentRight;
        PREVIOUS_WAS_TRANSLATE = true;
    }
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
    {
        glm::vec3 currentRight = glm::normalize(glm::cross(cameraTarget - cameraPos, cameraUp));

        model = glm::translate(model, -0.05f * currentRight);
        c -= 0.05f * currentRight;
        PREVIOUS_WAS_TRANSLATE = true;
    }
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
    {
        model = glm::translate(model, 0.05f * (cameraTarget - cameraPos));
        c += 0.05f * (cameraTarget - cameraPos);
        PREVIOUS_WAS_TRANSLATE = true;
    }
    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
    {
        model = glm::translate(model, -0.05f * (cameraTarget - cameraPos));
        c -= 0.05f * (cameraTarget - cameraPos);
        PREVIOUS_WAS_TRANSLATE = true;
    }
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
    {
        glm::vec3 currentRight = glm::normalize(glm::cross(cameraTarget - cameraPos, cameraUp));
        glm::vec3 currentUp = glm::normalize(glm::cross(currentRight, cameraTarget - cameraPos));

        model = glm::translate(model, 0.05f * currentUp);
        c += 0.05f * currentUp;
        PREVIOUS_WAS_TRANSLATE = true;
    }
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
    {
        glm::vec3 currentRight = glm::normalize(glm::cross(cameraTarget - cameraPos, cameraUp));
        glm::vec3 currentUp = glm::normalize(glm::cross(currentRight, cameraTarget - cameraPos));

        model = glm::translate(model, -0.05f * currentUp);
        c -= 0.05f * currentUp;
        PREVIOUS_WAS_TRANSLATE = true;
    }

    // Part B - 3
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
    {
        if (PREVIOUS_WAS_TRANSLATE)
            reset();

        cameraPos = glm::vec3(1.0f, 2.0f, 3.0f);
        PREVIOUS_WAS_TRANSLATE = false;
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
    {
        if (PREVIOUS_WAS_TRANSLATE)
            reset();

        cameraPos = glm::vec3(3.0f, 2.0f, 1.0f);
        PREVIOUS_WAS_TRANSLATE = false;
    }
}

// Part B - 4 and 5
void key_was_pressed(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        if (PREVIOUS_WAS_TRANSLATE)
            reset();

        OBJECT_SET_TO_ROTATE = !OBJECT_SET_TO_ROTATE;
        PREVIOUS_WAS_TRANSLATE = false;
    }

    if (key == GLFW_KEY_T && action == GLFW_PRESS)
    {
        if (PREVIOUS_WAS_TRANSLATE)
            reset();

        CAMERA_SET_TO_REVOLVE = !CAMERA_SET_TO_REVOLVE;
        PREVIOUS_WAS_TRANSLATE = false;
    }
}

// GLFW: Whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

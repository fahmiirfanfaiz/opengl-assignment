#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <tiny_obj_loader.cc>

const unsigned int SCREEN_WIDTH = 1200;
const unsigned int SCREEN_HEIGHT = 800;

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);

int main()
{
    // [1] Setting up basic info for OpenGL Version
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // For Mac OS X
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // [2] Create a simple window
    GLFWwindow *window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Latihan OpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Gagal membuat GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // [3] Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Tidak bisa menginitialisasi GLAD" << std::endl;
        return -1;
    }

    // [4] Set callback to adjust viewport on window resize
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // [5] Render loop: tempat menggambar
    while (!glfwWindowShouldClose(window))
    {
        // [6] Manage input from user
        processInput(window);

        // [7] Set the window color
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // [8] Clean up and exit
    glfwTerminate();
    return 0;
}

// Callback untuk menyesuaikan viewport
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// Fungsi untuk mengolah input
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}
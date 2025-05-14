// src/Main.cpp
#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

// resize callback
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// helper: baca seluruh file ke string
std::string readFile(const char* path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "ERROR: failed to open " << path << "\n";
        return "";
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

// compile satu shader (vertex atau fragment)
GLuint compileShader(GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint ok;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char buf[512];
        glGetShaderInfoLog(shader, 512, nullptr, buf);
        std::cerr << "Shader compile error: " << buf << "\n";
    }
    return shader;
}

// link vertex & fragment jadi program
GLuint createProgram(const char* vertPath, const char* fragPath) {
    std::string vsSrc = readFile(vertPath);
    std::string fsSrc = readFile(fragPath);
    GLuint vsh = compileShader(GL_VERTEX_SHADER,   vsSrc.c_str());
    GLuint fsh = compileShader(GL_FRAGMENT_SHADER, fsSrc.c_str());

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vsh);
    glAttachShader(prog, fsh);
    glLinkProgram(prog);

    // check link status
    GLint ok;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        char buf[512];
        glGetProgramInfoLog(prog, 512, nullptr, buf);
        std::cerr << "Program link error: " << buf << "\n";
    }

    glDeleteShader(vsh);
    glDeleteShader(fsh);
    return prog;
}

// load OBJ via tinyobjloader ke dua array: verts (pos+normal) & idxs
bool loadOBJ(const char* path,
             std::vector<float>& verts,
             std::vector<unsigned int>& idxs)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> mats;
    std::string warn, err;
    bool ok = tinyobj::LoadObj(&attrib, &shapes, &mats, &warn, &err, path);
    if (!warn.empty()) std::cout << "WARN: " << warn << "\n";
    if (!err.empty())  std::cerr << "ERR: " << err << "\n";
    if (!ok) return false;

    // gabungkan semua shapes
    for (auto& sh : shapes) {
        for (auto& idx : sh.mesh.indices) {
            // posisi
            verts.push_back(attrib.vertices[3*idx.vertex_index + 0]);
            verts.push_back(attrib.vertices[3*idx.vertex_index + 1]);
            verts.push_back(attrib.vertices[3*idx.vertex_index + 2]);
            // normal
            if (idx.normal_index >= 0) {
                verts.push_back(attrib.normals[3*idx.normal_index + 0]);
                verts.push_back(attrib.normals[3*idx.normal_index + 1]);
                verts.push_back(attrib.normals[3*idx.normal_index + 2]);
            } else {
                verts.insert(verts.end(), {0.0f, 0.0f, 0.0f});
            }
            idxs.push_back((unsigned int)idxs.size());
        }
    }
    return true;
}

int main() {
    // --- Init GLFW & GLAD ---
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Phong Shading", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }
    glEnable(GL_DEPTH_TEST);

    // --- Compile & Link Shader Phong ---
    GLuint shader = createProgram(
        "../shaders/vertex.vs",
        "../shaders/fragment.fs"
    );

    // --- Load 3 OBJ Models ---
    const char* modelPaths[3] = {
        "../models/model1.obj",
        "../models/model2.obj",
        "../models/model3.obj"
    };
    std::vector<std::vector<float>> allVerts(3);
    std::vector<std::vector<unsigned int>> allIdxs(3);
    for (int i = 0; i < 3; ++i) {
        if (!loadOBJ(modelPaths[i], allVerts[i], allIdxs[i])) {
            std::cerr << "Failed to load " << modelPaths[i] << "\n";
            return -1;
        }
    }

    // --- Setup VAO/VBO/EBO untuk tiap model ---
    GLuint VAO[3], VBO[3], EBO[3];
    glGenVertexArrays(3, VAO);
    glGenBuffers(3, VBO);
    glGenBuffers(3, EBO);

    for (int i = 0; i < 3; ++i) {
        glBindVertexArray(VAO[i]);
        glBindBuffer(GL_ARRAY_BUFFER, VBO[i]);
        glBufferData(GL_ARRAY_BUFFER,
                     allVerts[i].size() * sizeof(float),
                     allVerts[i].data(),
                     GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[i]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     allIdxs[i].size() * sizeof(unsigned int),
                     allIdxs[i].data(),
                     GL_STATIC_DRAW);

        // layout 0 = position, layout 1 = normal
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                              6 * sizeof(float),
                              (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }

    // camera & kontrol
    glm::vec3 camPos(0.0f, 0.0f, 5.0f);
    bool ortho = false;

    // uniform material Phong
    const float ambientStrength  = 0.1f;
    const float specularStrength = 0.5f;
    const float shininess        = 32.0f;

    // --- Render Loop ---
    while (!glfwWindowShouldClose(window)) {
        // ESC untuk keluar
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // O = orthogonal, P = perspektif
        if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) ortho = true;
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) ortho = false;

        // clear
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // aktifkan shader
        glUseProgram(shader);

        // --- Kirim camera matrices ---
        glm::mat4 view = glm::lookAt(camPos, glm::vec3(0.0f), glm::vec3(0,1,0));
        glm::mat4 proj = ortho
            ? glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, 0.1f, 100.0f)
            : glm::perspective(glm::radians(45.0f),
                               800.0f/600.0f, 0.1f, 100.0f);
        glUniformMatrix4fv(glGetUniformLocation(shader, "view"),
                           1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shader, "projection"),
                           1, GL_FALSE, glm::value_ptr(proj));

        // --- Kirim Phong uniforms ---
        glUniform3f(glGetUniformLocation(shader, "lightPos"),   2.0f, 2.0f, 2.0f);
        glUniform3f(glGetUniformLocation(shader, "viewPos"),
                    camPos.x, camPos.y, camPos.z);
        glUniform3f(glGetUniformLocation(shader, "lightColor"), 1.0f, 1.0f, 1.0f);

        glUniform1f(glGetUniformLocation(shader, "ambientStrength"),  ambientStrength);
        glUniform1f(glGetUniformLocation(shader, "specularStrength"), specularStrength);
        glUniform1f(glGetUniformLocation(shader, "shininess"),        shininess);

        // gambar tiap model dengan transform & warna berbeda
        for (int i = 0; i < 3; ++i) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-2.0f + 2.0f * i, 0.0f, 0.0f));
            glUniformMatrix4fv(glGetUniformLocation(shader, "model"),
                               1, GL_FALSE, glm::value_ptr(model));

            // objectColor per model
            if (i == 0) glUniform3f(glGetUniformLocation(shader, "objectColor"), 1.0f, 0.5f, 0.3f);
            else if (i == 1) glUniform3f(glGetUniformLocation(shader, "objectColor"), 0.2f, 0.8f, 0.3f);
            else           glUniform3f(glGetUniformLocation(shader, "objectColor"), 0.3f, 0.3f, 0.8f);

            glBindVertexArray(VAO[i]);
            glDrawElements(GL_TRIANGLES,
                           (GLsizei)allIdxs[i].size(),
                           GL_UNSIGNED_INT, nullptr);
        }

        // swap & poll
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // cleanup
    glDeleteVertexArrays(3, VAO);
    glDeleteBuffers(3, VBO);
    glDeleteBuffers(3, EBO);
    glDeleteProgram(shader);
    glfwTerminate();
    return 0;
}

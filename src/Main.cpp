#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "object.h"
#include "Camera.h"
#include "Light.h"

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

#define WINDOW_TITLE "Title"
#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080

int main() {
    // Initialize and configure (glfw)
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window (glfw)
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Load all OpenGL function pointers (glad)
    if (!gladLoadGL(glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primary);

    // Calculate center position
    int xpos = (mode->width - WINDOW_WIDTH) / 2;
    int ypos = (mode->height - WINDOW_HEIGHT) / 2;

    glfwSetWindowPos(window, xpos, ypos);
    glfwMakeContextCurrent(window);

    // Configure global opengl state
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    Camera camera(glm::vec3(0.5f, 0.5f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    Shader Shader("shaders/Shader.vs", "shaders/Shader.fs");

    std::vector<Object*> sceneObjects;
    std::vector<Light> sceneLight;

    Object WorldAxis("assets/WorldAxis.obj", &Shader);
    WorldAxis.scale = glm::vec3(0.2f);
    sceneObjects.push_back(&WorldAxis);

    Object Cube("assets/Cube.obj", &Shader);
    Cube.position = glm::vec3(-3.0f,  -0.5f,  -5.0f);
    Cube.rotation = glm::vec3(20.0f, 15.0f, 0.0f);
    Cube.scale = glm::vec3(0.5f);
    sceneObjects.push_back(&Cube);

    Object Monkey("assets/Monkey.obj", &Shader);
    Monkey.position = glm::vec3(5.0f,  0.0f,  -7.0f);
    Monkey.scale = glm::vec3(0.8f);
    sceneObjects.push_back(&Monkey);

    glm::vec3 LightPos = glm::vec3(5.0f, -1.0f, -6.0f);
    Object LightCube("assets/Cube.obj", &Shader);
    LightCube.position = LightPos;
    LightCube.scale = glm::vec3(0.1f);
    //sceneObjects.push_back(&LightCube);
    sceneLight.push_back({LightPos, glm::vec3(0.0f, 1.0f, 0.0f), 1.0f});

    double lastTime = glfwGetTime();
    double DeltaTime = 0.0;

    while (!glfwWindowShouldClose(window)) {
        double currentTime = glfwGetTime();
        DeltaTime = currentTime - lastTime;
        lastTime = currentTime;

        // Input
        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        float moveSpeed = 2.0f;
        float turnSpeed = 60.0f;
        float angle = glm::radians(turnSpeed * DeltaTime);
        glm::vec3 worldUp(0.0f, 1.0f, 0.0f);

        // Movement
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.position += camera.Forward() * static_cast<float>(moveSpeed * DeltaTime);

        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.position -= camera.Forward() * static_cast<float>(moveSpeed * DeltaTime);

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.position -= camera.Right() * static_cast<float>(moveSpeed * DeltaTime);

        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.position += camera.Right() * static_cast<float>(moveSpeed * DeltaTime);

        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            camera.position.y += moveSpeed * DeltaTime;

        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            camera.position.y -= moveSpeed * DeltaTime;

        // Pitch
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
            camera.rotation = glm::angleAxis(angle, glm::normalize(camera.Right())) * camera.rotation;
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
            camera.rotation = glm::angleAxis(-angle, glm::normalize(camera.Right())) * camera.rotation;

        // Yaw
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
            camera.rotation = glm::angleAxis(angle, worldUp) * camera.rotation;
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            camera.rotation = glm::angleAxis(-angle, worldUp) * camera.rotation;

        // Roll
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            camera.rotation = glm::angleAxis(-angle, glm::normalize(camera.Forward())) * camera.rotation;
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            camera.rotation = glm::angleAxis(angle, glm::normalize(camera.Forward())) * camera.rotation;

        // Reset Roll
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            glm::mat4 lookAtMat = glm::lookAt(glm::vec3(0.0f), camera.Forward(), worldUp);
            camera.rotation = glm::quat_cast(glm::inverse(lookAtMat));
        }

        camera.rotation = glm::normalize(camera.rotation);

        // Logic

        // Draw
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::mat4(1.0f);
        projection = glm::perspective(glm::radians(45.0f), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f);

        for (Object *obj : sceneObjects) {
            obj->draw(view, projection, sceneLight);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}

// Callback for whenever the window size changed (by OS or user resize)
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}
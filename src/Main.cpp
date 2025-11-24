#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Object.h"
#include "Camera.h"
#include "Light.h"

#include <iostream>
#include <algorithm>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

#define WINDOW_TITLE "Title"
int window_width = 1920;
int window_height = 1080;

int main() {
    // Initialize and configure (glfw)
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window (glfw)
    GLFWwindow* window = glfwCreateWindow(window_width, window_height, WINDOW_TITLE, NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Load all OpenGL function pointers (glad)
    if (!gladLoadGL(glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primary);

    // Calculate center position
    int xpos = (mode->width - window_width) / 2;
    int ypos = (mode->height - window_height) / 2;

    glfwSetWindowPos(window, xpos, ypos);
    glfwMakeContextCurrent(window);

    // Configure global opengl state
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    Camera camera(glm::vec3(0.5f, 0.5f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    camera.projectionMatrix = glm::perspective(
            glm::radians(45.0f), (float)window_width / (float)window_height,
            camera.nearPlane, camera.farPlane);
    glfwSetWindowUserPointer(window, &camera);

    Shader Shader("shaders/Shader.vs", "shaders/Shader.fs");
    std::vector<Object*> sceneObjects;
    std::vector<Light> sceneLights;

    auto light = [&Shader, &sceneLights, &sceneObjects](glm::vec3 Pos, glm::vec3 Color, float Intensity) {
        sceneObjects.push_back(new Object("assets/Light.obj", &Shader));
        sceneObjects.back()->position = Pos;
        sceneObjects.back()->scale = glm::vec3(0.5f);
        sceneObjects.back()->useLighting = false;
        sceneLights.push_back({Pos, Color, Intensity});

        // Change the color of the light
        size_t stride = 13;
        size_t diffuseOffset = 9;
        auto& verts = sceneObjects.back()->vertices;
        for (size_t i = 0; i < verts.size(); i += stride) {
            verts[i + diffuseOffset + 0] = Color.r;
            verts[i + diffuseOffset + 1] = Color.g;
            verts[i + diffuseOffset + 2] = Color.b;
        }
        // Re-upload the vertex data
        glBindBuffer(GL_ARRAY_BUFFER, sceneObjects.back()->VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0,
            sceneObjects.back()->vertices.size() * sizeof(float),
            sceneObjects.back()->vertices.data());
    };

    Object WorldAxis("assets/WorldAxis.obj", &Shader);
    WorldAxis.scale = glm::vec3(0.2f);
    WorldAxis.useLighting = false;
    sceneObjects.push_back(&WorldAxis);

    Object Cube("assets/Cube.obj", &Shader);
    Cube.position = glm::vec3(-3.0f,  -0.5f,  -5.0f);
    Cube.rotation = glm::vec3(20.0f, 15.0f, 0.0f);
    Cube.scale = glm::vec3(0.5f);
    sceneObjects.push_back(&Cube);

    light(glm::vec3(-1.5f,  0.0f,  -4.0f), glm::vec3(0.0f, 0.0f, 1.0f), 4.0f);

    Object Monkey("assets/Monkey.obj", &Shader);
    Monkey.position = glm::vec3(5.0f,  0.0f,  -7.0f);
    Monkey.scale = glm::vec3(0.8f);
    sceneObjects.push_back(&Monkey);

    light(glm::vec3(5.0f, -1.0f, -6.0f), glm::vec3(0.0f, 1.0f, 0.0f), 1.0f);

    Object AlphaCube("assets/AlphaCube.obj", &Shader);
    AlphaCube.position = glm::vec3(0.5f, 0.5f, -5.0f);
    AlphaCube.scale = glm::vec3(0.3f);
    sceneObjects.push_back(&AlphaCube);

    Object Dragon("assets/Dragon.obj", &Shader);
    Dragon.position = glm::vec3(-1.0f, -2.0f, -10.0f);
    sceneObjects.push_back(&Dragon);

    light(glm::vec3(-1.0f, -1.0f, -9.0f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0f);

    std::vector<Object*> opaqueObjects;
    std::vector<Object*> transparentObjects;
    for (Object* obj : sceneObjects) {
        if (obj->hasTransparency) {
            transparentObjects.push_back(obj);
        } else {
            opaqueObjects.push_back(obj);
        }
    }

    double lastTime = glfwGetTime();
    double DeltaTime = 0.0;

    while (!glfwWindowShouldClose(window)) {
        double currentTime = glfwGetTime();
        DeltaTime = currentTime - lastTime;
        lastTime = currentTime;

        // Input
        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // Speed boost
        float moveSpeed = camera.moveSpeed;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            moveSpeed *= 3.0f;

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

        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
            camera.position.y -= moveSpeed * DeltaTime;

        float lerpSpeed = 5.0f;
        glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
        float angle = glm::radians(camera.turnSpeed * DeltaTime);

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
            glm::quat targetRotation = glm::quat_cast(glm::inverse(lookAtMat));
            camera.rotation = glm::slerp(camera.rotation, targetRotation, static_cast<float>(lerpSpeed * DeltaTime));
        }

        camera.rotation = glm::normalize(camera.rotation);

        // Logic

        // Draw
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = camera.projectionMatrix;

        // Draw opaque objects
        for (Object* obj : opaqueObjects) {
            obj->draw(view, projection, sceneLights);
        }

        // Sort translucent objects back to front
        glm::vec3 camPos = camera.position;
        std::sort(transparentObjects.begin(), transparentObjects.end(),
            [camPos](Object* a, Object* b) {
                float distA = glm::length(camPos - a->position);
                float distB = glm::length(camPos - b->position);
                return distA > distB; // Farthest first
            });


        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);

        // Draw translucent objects
        for (Object* obj : transparentObjects) {
            obj->draw(view, projection, sceneLights);
        }

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}

// Callback for whenever the window size changed (by OS or user resize)
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    if (height == 0) height = 1;
    glViewport(0, 0, width, height);

    Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    camera->projectionMatrix = glm::perspective(
        glm::radians(45.0f), (float)width / (float)height,
        camera->nearPlane, camera->farPlane);

    window_width = width;
    window_height = height;
}

// Callback for whenever the mouse is moved
bool firstMouse = true;
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    static double lastX = xpos, lastY = ypos;

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    double xoffset = xpos - lastX;
    double yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    // Apply rotation
    glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
    Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    camera->rotation = glm::angleAxis((float)-xoffset * camera->sensitivity, worldUp) * camera->rotation;
    camera->rotation = glm::angleAxis((float)yoffset * camera->sensitivity, glm::normalize(camera->Right())) * camera->rotation;
}

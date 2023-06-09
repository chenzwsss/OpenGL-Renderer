#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <stb_image.h>

#include <string>
#include <iostream>

#include "base/RenderCamera.hpp"
#include "utility/ResourceManager.h"
#include "graphic/GLShaderProgram.h"

#include "base/Skybox.h"

#include "utility/ImGuiRenderer.h"

#include "base/glTFModel.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void process_input(GLFWwindow *window);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow* window, double x, double y);

// void renderSphere();

// glfw window
GLFWwindow* window;

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// camera
RenderCamera camera;
float last_x = SCR_WIDTH / 2.0f;
float last_y = SCR_HEIGHT / 2.0f;

// timing
float delta_time = 0.0f;
float last_frame = 0.0f;

const std::string WINDOW_NAME = "opengl_renderer";

GLuint m_uboMatrices{ 0 };

struct MouseButtons {
    bool left = false;
    bool right = false;
    bool middle = false;
} mouseButtons;

struct LightSource {
    glm::vec3 color = glm::vec3(1.0f);
    glm::vec3 rotation = glm::vec3(75.0f, 40.0f, 0.0f);
} lightSource;

int main() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, WINDOW_NAME.c_str(), NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // initial ImGui
    ImGuiRenderer::getInstance().setupImGui(window);

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    // stbi_set_flip_vertically_on_load(true);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    // set depth function to less than AND equal for Skybox depth trick.
    glDepthFunc(GL_LEQUAL);
    // enable seamless cubemap sampling for lower mip levels in the pre-filter map.
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    // Create uniform buffer object for projection and view matrices
    glGenBuffers(1, &m_uboMatrices);
    glBindBuffer(GL_UNIFORM_BUFFER, m_uboMatrices);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), nullptr, GL_STATIC_DRAW);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, m_uboMatrices, 0, 2 * sizeof(glm::mat4));
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_uboMatrices);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // camera
    camera.type = RenderCamera::camera_type::lookat;
    camera.setPerspective(45.0f, (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 256.0f);
    camera.rotation_speed = 0.1f;
    camera.movement_speed = 0.1f;
    camera.setPosition({ 0.0f, 0.0f, -3.0f });
    camera.setRotation({ 0.0f, 0.0f, 0.0f });

    // shader program
    GLShaderProgram gltf_shader("glTF Shader", {
        {"shaders/glsl/mesh.vert", "vertex"},
        {"shaders/glsl/wireframe.geometry", "geometry"},
        {"shaders/glsl/mesh.frag", "fragment"}
    });
    gltf_shader.bind();
    gltf_shader.setUniformi("albedoMap", 0);

    GLShaderProgram skybox_shader{"Skybox Shader", {
        {"shaders/glsl/skybox.vert", "vertex"},
        {"shaders/glsl/skybox.frag", "fragment"}
    }};

    // pbr_shader.bind();
    // pbr_shader.setUniformi("irradianceMap", 0);
    // pbr_shader.setUniformi("prefilterMap", 1);
    // pbr_shader.setUniformi("brdfLUT", 2);
    // pbr_shader.setUniformi("albedoMap", 3);
    // pbr_shader.setUniformi("normalMap", 4);
    // pbr_shader.setUniformi("metallicMap", 5);
    // pbr_shader.setUniformi("roughnessMap", 6);

    skybox_shader.bind();
    skybox_shader.setUniformi("environmentMap", 0);

    // model
    // model model_nanosuit("data/nanosuit/nanosuit.obj", "nanosuit");

    glTFModel g_m("models/DamagedHelmet/glTF-Embedded/DamagedHelmet.gltf");

    // Skybox
    Skybox env_skybox;
    env_skybox.init("textures/hdr/hdriHaven4k.hdr", 512);

    // initialize static shader uniforms before rendering
    // --------------------------------------------------
    glm::mat4 projection = camera.matrices.perspective;
    glBindBuffer(GL_UNIFORM_BUFFER, m_uboMatrices);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // then before rendering, configure the viewport to the original framebuffer's screen dimensions
    int scr_width, scr_height;
    glfwGetFramebufferSize(window, &scr_width, &scr_height);
    glViewport(0, 0, scr_width, scr_height);

    // set light direction
    // glm::vec4 lightDir = glm::vec4(
    //     sin(glm::radians(lightSource.rotation.x)) * cos(glm::radians(lightSource.rotation.y)),
    //     sin(glm::radians(lightSource.rotation.y)),
    //     cos(glm::radians(lightSource.rotation.x)) * cos(glm::radians(lightSource.rotation.y)),
    //     0.0f);
    // pbr_shader.bind();
    // pbr_shader.setUniform("lightDir", lightDir);
    // pbr_shader.setUniform("lightColor", lightSource.color);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // --------------------
        float current_frame = static_cast<float>(glfwGetTime());
        delta_time = current_frame - last_frame;
        last_frame = current_frame;

        // input
        // -----
        process_input(window);

        camera.update(delta_time);

        // render
        // ------
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // render scene, supplying the convoluted irradiance map to the final shader.
        // ------------------------------------------------------------------------------------------
        glm::mat4 view = camera.matrices.view;
        glBindBuffer(GL_UNIFORM_BUFFER, m_uboMatrices);
        glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        // bind pre-computed IBL data
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, env_skybox.getIrradianceMap());
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, env_skybox.getPrefilterMap());
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, env_skybox.getBRDFLUT());

       /* glm::vec3 camPos = glm::vec3(
            camera.position.z * sin(glm::radians(camera.rotation.y)) * cos(glm::radians(camera.rotation.x)),
            -camera.position.z * sin(glm::radians(camera.rotation.x)),
            -camera.position.z * cos(glm::radians(camera.rotation.y)) * cos(glm::radians(camera.rotation.x))
        );*/

        // pbr_shader.bind();
        // // set camera postion
        // pbr_shader.setUniform("camPos", camPos);
        // // set uniform render wireframe
        // pbr_shader.setUniformi("render_wireframe", (int)ImGuiRenderer::render_wireframe);
        // render model
        // model_nanosuit.translate(glm::vec3(0.0f, -7.0f, 1.0f));
        // model_nanosuit.scale(glm::vec3(0.8f));
        // model_nanosuit.draw(pbr_shader);
        gltf_shader.bind();
        gltf_shader.setUniformi("render_wireframe", (int)ImGuiRenderer::render_wireframe);
        g_m.draw(gltf_shader);

        // render Skybox (render as last to prevent overdraw)
        skybox_shader.bind();
        // skybox_shader.setUniform("view", view);
        env_skybox.draw();

        // render ImGui
        ImGuiRenderer::getInstance().renderImGui();

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // ImGui Cleanup
    ImGuiRenderer::getInstance().destroyImGui();

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void process_input(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    if ((width > 0.0f) && (height > 0.0f)) {
        camera.updateAspectRatio((float)width / (float)height);
    }

    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS)
            mouseButtons.left = true;
        else if (action == GLFW_RELEASE)
            mouseButtons.left = false;
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS)
            mouseButtons.right = true;
        else if (action == GLFW_RELEASE)
            mouseButtons.right = false;
    } else if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
        if (action == GLFW_PRESS)
            mouseButtons.middle = true;
        else if (action == GLFW_RELEASE)
            mouseButtons.middle = false;
    }
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void cursor_position_callback(GLFWwindow* window, double x, double y) {
    float xpos = static_cast<float>(x);
    float ypos = static_cast<float>(y);

    float dx = xpos - last_x;
    float dy = ypos - last_y;

    last_x = xpos;
    last_y = ypos;

    if (mouseButtons.left) {
        camera.rotate(glm::vec3(dy * camera.rotation_speed, dx * camera.rotation_speed, 0.0f));
    }
    if (mouseButtons.right) {
        camera.translate(glm::vec3(-0.0f, 0.0f, dy * .005f));
    }
    if (mouseButtons.middle) {
        camera.translate(glm::vec3(dx * 0.005f, -dy * 0.005f, 0.0f));
    }
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.translate(glm::vec3(0.0f, 0.0f, (float)yoffset));
}

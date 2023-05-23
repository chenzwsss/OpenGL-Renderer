#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <string>
#include <iostream>

#include "base/render_camera.hpp"
#include "model.hpp"
#include "utility/resource_manager.h"
#include "graphic/gl_shader_program.h"

#include "base/skybox.h"

GLFWwindow* window;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow* window, double x, double y);

// void renderSphere();

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

bool bloom = true;
float exposure = 1.0f;

// camera
render_camera camera;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

std::string WINDOW_NAME = "OpenGL_Renderer";

struct {
    bool left = false;
    bool right = false;
    bool middle = false;
} mouseButtons;

void setupImGui() {
    // Setup Dear ImGui content
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");
}

void renderImGui()
{
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    {
        ImGui::Begin("OPGL Status");
        ImGuiIO& io = ImGui::GetIO();
        ImGui::Text("%.1f FPS(%.3f ms/frame)", io.Framerate, 1000.0f / io.Framerate);
        ImGui::End();
    }
    ImGui::Render();
    // ImGui draw
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

int main() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, WINDOW_NAME.c_str(), NULL, NULL);
    if (window == NULL)
    {
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
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // initial ImGui
    setupImGui();

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    // set depth function to less than AND equal for skybox depth trick.
    glDepthFunc(GL_LEQUAL);
    // enable seamless cubemap sampling for lower mip levels in the pre-filter map.
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    // camera
    camera.type = render_camera::CameraType::lookat;
    camera.setPerspective(45.0f, (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 256.0f);
    camera.rotationSpeed = 0.1f;
    camera.movementSpeed = 0.1f;
    camera.setPosition({ 0.0f, 0.0f, -5.0f });
    camera.setRotation({ 0.0f, 0.0f, 0.0f });

    // shaders
    gl_shader_program pbrShader{"PBR Shader", {
        {"shaders/pbr.vs", "vertex"},
        {"shaders/pbr.fs", "fragment"}
    }};

    gl_shader_program skyboxShader{"Skybox Shader", {
        {"shaders/skyboxvs.glsl", "vertex"},
        {"shaders/skyboxps.glsl", "fragment"}
    }};

    pbrShader.bind();
    pbrShader.setUniformi("irradianceMap", 0);
    pbrShader.setUniformi("prefilterMap", 1);
    pbrShader.setUniformi("brdfLUT", 2);
    pbrShader.setUniformi("albedoMap", 3);
    pbrShader.setUniformi("normalMap", 4);
    pbrShader.setUniformi("metallicMap", 5);
    pbrShader.setUniformi("roughnessMap", 6);

    skyboxShader.bind();
    skyboxShader.setUniformi("environmentMap", 0);

    // Titanium
    //model human_model(resource_manager::getAssetPath() + "models/nanosuit/nanosuit.obj");

    // lights
    // ------
    glm::vec3 lightPosition = glm::vec3(-10.0f, 10.0f, 10.0f);
    glm::vec3 lightColor = glm::vec3(300.0f, 300.0f, 300.0f);

    // skybox
    skybox env_skybox;
    env_skybox.init("textures/hdr/newport_loft.hdr", 2048);

    // initialize static shader uniforms before rendering
    // --------------------------------------------------
    glm::mat4 projection = camera.matrices.perspective;
    pbrShader.bind();
    pbrShader.setUniform("projection", projection);
    skyboxShader.bind();
    skyboxShader.setUniform("projection", projection);

    // then before rendering, configure the viewport to the original framebuffer's screen dimensions
    int scrWidth, scrHeight;
    glfwGetFramebufferSize(window, &scrWidth, &scrHeight);
    glViewport(0, 0, scrWidth, scrHeight);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // render scene, supplying the convoluted irradiance map to the final shader.
        // ------------------------------------------------------------------------------------------
        pbrShader.bind();
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = camera.matrices.view;
        pbrShader.setUniform("view", view);
        pbrShader.setUniform("camPos", camera.position);

        // bind pre-computed IBL data
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, env_skybox.GetIrradianceMap());
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, env_skybox.GetPrefilterMap());
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, env_skybox.GetBRDFLUT());

        //// model
        //model = glm::mat4(1.0f);
        //model = glm::translate(model, glm::vec3(-3.0, 0.0, 2.0));
        //pbrShader.setUniform("model", model);
        //human_model.Draw(pbrShader);

        glm::vec3 newPos = lightPosition + glm::vec3(sin(glfwGetTime() * 5.0) * 5.0, 0.0, 0.0);
        pbrShader.setUniform("lightPosition", newPos);
        pbrShader.setUniform("lightColor", lightColor);

        // render skybox (render as last to prevent overdraw)
        skyboxShader.bind();
        skyboxShader.setUniform("view", view);
        env_skybox.draw();

        // render ImGui
        renderImGui();

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // ImGui Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
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
void cursor_position_callback(GLFWwindow* window, double x, double y)
{
    float xpos = static_cast<float>(x);
    float ypos = static_cast<float>(y);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float dx = xpos - lastX;
    float dy = ypos - lastY;

    lastX = xpos;
    lastY = ypos;

    if (mouseButtons.left) {
        camera.rotate(glm::vec3(dy * camera.rotationSpeed, dx * camera.rotationSpeed, 0.0f));
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
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.translate(glm::vec3(0.0f, 0.0f, (float)yoffset));
}

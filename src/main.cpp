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

#include "base/render_camera.hpp"
#include "base/model.h"
#include "utility/resource_manager.h"
#include "graphic/gl_shader_program.h"

#include "base/skybox.h"

GLFWwindow* window;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void process_input(GLFWwindow *window);
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
float last_x = SCR_WIDTH / 2.0f;
float last_y = SCR_HEIGHT / 2.0f;
bool first_mouse = true;

// timing
float delta_time = 0.0f;
float last_frame = 0.0f;

std::string WINDOW_NAME = "opengl_renderer";

struct {
    bool left = false;
    bool right = false;
    bool middle = false;
} mouse_buttons;

void setup_imgui() {
    // Setup Dear ImGui content
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");
}

void render_imgui()
{
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    {
        ImGui::Begin("opengl status");
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
    setup_imgui();

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
    camera.type = render_camera::camera_type::lookat;
    camera.set_perspective(45.0f, (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 256.0f);
    camera.rotation_speed = 0.1f;
    camera.movement_speed = 0.1f;
    camera.set_position({ 0.0f, 0.0f, -5.0f });
    camera.set_rotation({ 0.0f, 0.0f, 0.0f });

    // shaders
    gl_shader_program pbr_shader{"PBR Shader", {
        {"shaders/pbr.vs", "vertex"},
        {"shaders/pbr.fs", "fragment"}
    }};

    gl_shader_program skybox_shader{"Skybox Shader", {
        {"shaders/skybox_vs.glsl", "vertex"},
        {"shaders/skybox_ps.glsl", "fragment"}
    }};

    pbr_shader.bind();
    pbr_shader.set_uniform_i("albedoMap", 0);
    /*pbr_shader.set_uniform_i("irradianceMap", 0);
    pbr_shader.set_uniform_i("prefilterMap", 1);
    pbr_shader.set_uniform_i("brdfLUT", 2);
    pbr_shader.set_uniform_i("albedoMap", 3);
    pbr_shader.set_uniform_i("normalMap", 4);
    pbr_shader.set_uniform_i("metallicMap", 5);
    pbr_shader.set_uniform_i("roughnessMap", 6);*/

    skybox_shader.bind();
    skybox_shader.set_uniform_i("environmentMap", 0);

    // model
    model model_nanosuit("models/nanosuit/nanosuit.obj", "nanosuit");

    // lights
    // ------
    glm::vec3 light_position = glm::vec3(-10.0f, 10.0f, 10.0f);
    glm::vec3 light_color = glm::vec3(300.0f, 300.0f, 300.0f);

    // skybox
    skybox env_skybox;
    env_skybox.init("textures/hdr/newport_loft.hdr", 2048);

    // initialize static shader uniforms before rendering
    // --------------------------------------------------
    glm::mat4 projection = camera.matrices.perspective;
    pbr_shader.bind();
    pbr_shader.set_uniform("projection", projection);
    skybox_shader.bind();
    skybox_shader.set_uniform("projection", projection);

    // then before rendering, configure the viewport to the original framebuffer's screen dimensions
    int scr_width, scr_height;
    glfwGetFramebufferSize(window, &scr_width, &scr_height);
    glViewport(0, 0, scr_width, scr_height);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float current_frame = static_cast<float>(glfwGetTime());
        delta_time = current_frame - last_frame;
        last_frame = current_frame;

        // input
        // -----
        process_input(window);

        // render
        // ------
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // render scene, supplying the convoluted irradiance map to the final shader.
        // ------------------------------------------------------------------------------------------
        pbr_shader.bind();
        glm::mat4 view = camera.matrices.view;
        pbr_shader.set_uniform("view", view);
        //pbr_shader.set_uniform("camPos", camera.position);

        //// bind pre-computed IBL data
        //glActiveTexture(GL_TEXTURE0);
        //glBindTexture(GL_TEXTURE_CUBE_MAP, env_skybox.get_irradiance_map());
        //glActiveTexture(GL_TEXTURE1);
        //glBindTexture(GL_TEXTURE_CUBE_MAP, env_skybox.get_prefilter_map());
        //glActiveTexture(GL_TEXTURE2);
        //glBindTexture(GL_TEXTURE_2D, env_skybox.get_brdf_lut());

        // model
        model_nanosuit.translate(glm::vec3(0.0f, -7.0f, 0.0f));
        model_nanosuit.scale(glm::vec3(0.2f));
        model_nanosuit.draw(pbr_shader);

        glm::vec3 new_pos = light_position + glm::vec3(sin(glfwGetTime() * 5.0) * 5.0, 0.0, 0.0);
        /*pbr_shader.set_uniform("lightPosition", new_pos);
        pbr_shader.set_uniform("lightColor", light_color);*/

        // render skybox (render as last to prevent overdraw)
        skybox_shader.bind();
        skybox_shader.set_uniform("view", view);
        env_skybox.draw();

        // render ImGui
        render_imgui();

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
void process_input(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    if ((width > 0.0f) && (height > 0.0f)) {
        camera.update_aspect_ratio((float)width / (float)height);
    }

    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS)
            mouse_buttons.left = true;
        else if (action == GLFW_RELEASE)
            mouse_buttons.left = false;
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS)
            mouse_buttons.right = true;
        else if (action == GLFW_RELEASE)
            mouse_buttons.right = false;
    } else if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
        if (action == GLFW_PRESS)
            mouse_buttons.middle = true;
        else if (action == GLFW_RELEASE)
            mouse_buttons.middle = false;
    }
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void cursor_position_callback(GLFWwindow* window, double x, double y)
{
    float xpos = static_cast<float>(x);
    float ypos = static_cast<float>(y);

    if (first_mouse)
    {
        last_x = xpos;
        last_y = ypos;
        first_mouse = false;
    }

    float dx = xpos - last_x;
    float dy = ypos - last_y;

    last_x = xpos;
    last_y = ypos;

    if (mouse_buttons.left) {
        camera.rotate(glm::vec3(dy * camera.rotation_speed, dx * camera.rotation_speed, 0.0f));
    }
    if (mouse_buttons.right) {
        camera.translate(glm::vec3(-0.0f, 0.0f, dy * .005f));
    }
    if (mouse_buttons.middle) {
        camera.translate(glm::vec3(dx * 0.005f, -dy * 0.005f, 0.0f));
    }
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.translate(glm::vec3(0.0f, 0.0f, (float)yoffset));
}

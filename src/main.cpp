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

#include "utility/imgui_renderer.h"

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
render_camera camera;
float last_x = SCR_WIDTH / 2.0f;
float last_y = SCR_HEIGHT / 2.0f;

// timing
float delta_time = 0.0f;
float last_frame = 0.0f;

const std::string WINDOW_NAME = "opengl_renderer";

GLuint m_uboMatrices{ 0 };

//struct MaterialData {
//    float metallic_factor;
//    float roughness_factor;
//    int normal_texture_set;
//    int metallic_texture_set;
//    int roughness_texture_set;
//} materialData;
//GLuint m_materialData{ 0 };

struct {
    bool left = false;
    bool right = false;
    bool middle = false;
} mouse_buttons;

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
    imgui_renderer::get_instance().setup_imgui(window);

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    // set depth function to less than AND equal for skybox depth trick.
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
    camera.type = render_camera::camera_type::lookat;
    camera.set_perspective(45.0f, (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 256.0f);
    camera.rotation_speed = 0.1f;
    camera.movement_speed = 0.1f;
    camera.set_position({ 0.0f, 0.0f, -5.0f });
    camera.set_rotation({ 0.0f, 0.0f, 0.0f });

    // shaders
    gl_shader_program pbr_shader{"PBR Shader", {
        {"shaders/pbr_vs.glsl", "vertex"},
        {"shaders/pbr_ps.glsl", "fragment"}
    }};

    gl_shader_program skybox_shader{"Skybox Shader", {
        {"shaders/skybox_vs.glsl", "vertex"},
        {"shaders/skybox_ps.glsl", "fragment"}
    }};

    pbr_shader.bind();
    pbr_shader.set_uniform_i("albedoMap", 0);
    pbr_shader.set_uniform_i("normalMap", 1);
    pbr_shader.set_uniform_i("metallicMap", 2);
    pbr_shader.set_uniform_i("roughnessMap", 3);
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
    glBindBuffer(GL_UNIFORM_BUFFER, m_uboMatrices);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // then before rendering, configure the viewport to the original framebuffer's screen dimensions
    int scr_width, scr_height;
    glfwGetFramebufferSize(window, &scr_width, &scr_height);
    glViewport(0, 0, scr_width, scr_height);

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
        // pbr_shader.set_uniform("view", view);
        //pbr_shader.set_uniform("camPos", camera.position);

        //// bind pre-computed IBL data
        //glActiveTexture(GL_TEXTURE0);
        //glBindTexture(GL_TEXTURE_CUBE_MAP, env_skybox.get_irradiance_map());
        //glActiveTexture(GL_TEXTURE1);
        //glBindTexture(GL_TEXTURE_CUBE_MAP, env_skybox.get_prefilter_map());
        //glActiveTexture(GL_TEXTURE2);
        //glBindTexture(GL_TEXTURE_2D, env_skybox.get_brdf_lut());

        pbr_shader.bind();

        // model
        model_nanosuit.translate(glm::vec3(0.0f, -7.0f, 0.0f));
        model_nanosuit.scale(glm::vec3(0.2f));
        model_nanosuit.draw(pbr_shader);

        glm::vec3 new_pos = light_position + glm::vec3(sin(glfwGetTime() * 5.0) * 5.0, 0.0, 0.0);
        /*pbr_shader.set_uniform("lightPosition", new_pos);
        pbr_shader.set_uniform("lightColor", light_color);*/

        // render skybox (render as last to prevent overdraw)
        skybox_shader.bind();
        // skybox_shader.set_uniform("view", view);
        env_skybox.draw();

        // render ImGui
        imgui_renderer::get_instance().render_imgui();

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // ImGui Cleanup
    imgui_renderer::get_instance().destroy_imgui();

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
void cursor_position_callback(GLFWwindow* window, double x, double y) {
    float xpos = static_cast<float>(x);
    float ypos = static_cast<float>(y);

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
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.translate(glm::vec3(0.0f, 0.0f, (float)yoffset));
}

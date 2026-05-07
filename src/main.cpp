#include <iostream>
#include <glad/glad.h>
#include <print>
#include <GLFW/glfw3.h>
#include <fstream>
#include <sstream>
#include <math.h>
#include <cmath>
#include <algorithm>
#include "physics.h"
#include "utils.h"

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

const int circle_steps = 100;
const double circle_angle = 2.0 * M_PI / circle_steps;
const int vertex_stride_floats = 3; // x, y, alpha

std::vector<Body> bodies; // global variable to hold the bodies in the simulation

void draw_circle(double center_x_m, double center_y_m, double radius_m, unsigned int VBO, unsigned int VAO) {
    // radius is in meters and stays in world space; the camera controls view scale
    std::vector<float> vertices;
    vertices.reserve((circle_steps + 2) * vertex_stride_floats);
    vertices.push_back(static_cast<float>(center_x_m));
    vertices.push_back(static_cast<float>(center_y_m));
    vertices.push_back(1.0f);
    for (int i = 0; i <= circle_steps; i++) {
        const double x = center_x_m + radius_m * std::cos(circle_angle * i);
        const double y = center_y_m + radius_m * std::sin(circle_angle * i);

        vertices.push_back(static_cast<float>(x));
        vertices.push_back(static_cast<float>(y));
        vertices.push_back(1.0f);
    }

    // bind the vertex data to the buffer and draw the circle
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());
    glDrawArrays(GL_TRIANGLE_FAN, 0, vertices.size()/vertex_stride_floats);
}

static unsigned int compile_shader(const char* source, unsigned int type) { // type is either GL_VERTEX_SHADER or GL_FRAGMENT_SHADER (GLenum)
    unsigned int id = glCreateShader(type);
    glShaderSource(id, 1, &source, nullptr);
    glCompileShader(id);

    int res;
    glGetShaderiv(id, GL_COMPILE_STATUS, &res);
    if (res == GL_FALSE) {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = new char[length];
        glGetShaderInfoLog(id, length, &length, message);
        std::println(
            stderr,
            "Failed to compile {} shader\n {}",
            type == GL_VERTEX_SHADER ? "vertex" : "fragment",
            message
        );
        delete[] message;
        glDeleteShader(id);
        return 0;
    }

    return id;
}

static unsigned int create_shader(const char* vertex_shader, const char* fragment_shader) {
    unsigned int program = glCreateProgram();
    unsigned int vs = compile_shader(vertex_shader, GL_VERTEX_SHADER);
    unsigned int fs = compile_shader(fragment_shader, GL_FRAGMENT_SHADER);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // core - only modern functions

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // for macOS
#endif

    GLFWwindow* window = glfwCreateWindow(1000, 1000, "N Body Simulation", nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    gladLoadGL();

    // init imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    ImGui::StyleColorsDark();

    glViewport(0,0,1000,1000);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    Camera cam;
    cam.update_projection_matrix(1000, 1000);
    
    glClearColor(0.098f, 0.098f, 0.098f, 1.0f); // set clear color to (Dark Slate Gray) (#191919)

    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, 100000*sizeof(float), nullptr, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0); // void* is used to specify that the data starts at the beginning of the buffer
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(2 * sizeof(float)));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    auto vertex_shader = read_file("./shaders/body/vertex.glsl");
    auto fragment_shader = read_file(("./shaders/body/sun/fragment.glsl"));
    const char* vertex_shader_cstr = vertex_shader.c_str();
    const char* fragment_shader_cstr = fragment_shader.c_str();

    unsigned int sun_shader_program = create_shader(vertex_shader_cstr, fragment_shader_cstr);
    int sun_vp_loc = glGetUniformLocation(sun_shader_program, "vp");

    fragment_shader = read_file("./shaders/body/earth/fragment.glsl");
    fragment_shader_cstr = fragment_shader.c_str();
    unsigned int earth_shader_program = create_shader(vertex_shader_cstr, fragment_shader_cstr);
    int earth_vp_loc = glGetUniformLocation(earth_shader_program, "vp");

    fragment_shader = read_file("./shaders/body/ice/fragment.glsl");
    fragment_shader_cstr = fragment_shader.c_str();
    unsigned int ice_shader_program = create_shader(vertex_shader_cstr, fragment_shader_cstr);
    int ice_vp_loc = glGetUniformLocation(ice_shader_program, "vp");

    auto trail_vertex_shader = read_file("./shaders/trail/vertex.glsl");
    auto trail_fragment_shader = read_file("./shaders/trail/fragment.glsl");
    const char* trail_vertex_shader_cstr = trail_vertex_shader.c_str();
    const char* trail_fragment_shader_cstr = trail_fragment_shader.c_str();
    unsigned int trail_shader_program = create_shader(trail_vertex_shader_cstr, trail_fragment_shader_cstr);
    int trail_vp_loc = glGetUniformLocation(trail_shader_program, "vp");

    int sun_center_loc = glGetUniformLocation(sun_shader_program, "circleCenter");
    int sun_radius_loc = glGetUniformLocation(sun_shader_program, "radius");

    int earth_center_loc = glGetUniformLocation(earth_shader_program, "circleCenter");
    int earth_radius_loc = glGetUniformLocation(earth_shader_program, "radius");

    int ice_center_loc = glGetUniformLocation(ice_shader_program, "circleCenter");  
    int ice_radius_loc = glGetUniformLocation(ice_shader_program, "radius");

    load_config();
    accn_init(bodies);
    camera_init(cam, bodies, 1.0f);

    double last_time = glfwGetTime();
    double time_scale = 86400*7;
    int frame = 0;
    while (!glfwWindowShouldClose(window)) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        double dt = compute_dt(bodies)/1e6; // compute_dt gives microseconds until next update
        double current_time = glfwGetTime();
        double delta = current_time - last_time;
        last_time = current_time;
        if (time_scale > 1e5) dt *= 10;
        frame++;
        
        if (frame % 30 == 0) { // every 30 frames, record the positions for trails
            for (auto& body : bodies) {
                body.trail_vertices.push_back(static_cast<float>(body.pos.x));
                body.trail_vertices.push_back(static_cast<float>(body.pos.y));
                body.trail_vertices.push_back(1.0f); // alpha value for the trail vertex

                int points = body.trail_vertices.size() / 3;
                for (int i = 0; i < points; i++) {
                    int alpha_index = i * 3 + 2;
                    body.trail_vertices[alpha_index] *= 0.95f; // fade out the trail over time
                }

                // std::println("Trail vertex added: ({}, {})", body.pos.x, body.pos.y);

                if (body.trail_vertices.size() > 2000) { // keep only the last 1000 positions (2000 floats)
                    body.trail_vertices.erase(body.trail_vertices.begin(), body.trail_vertices.begin() + 3);
                }
            }
        }

        double sim_time = delta * time_scale; // scale the real time delta to simulation time
        
        int steps = ceil(sim_time / dt); // number of physics updates to perform
        steps = min(steps, 200); // cap the number of steps to prevent large number of calcs in one frame
        double sub_dt = sim_time / steps; // time step for each physics update
        if (steps < 1) steps = 1;
        for (int i = 0; i < steps; i++) {
            update(bodies, sub_dt);
            // std::println("Coordinates: ({}, {}) and ({}, {})", bodies[0].pos.x, bodies[0].pos.y, bodies[1].pos.x, bodies[1].pos.y);
        }
        glClear(GL_COLOR_BUFFER_BIT); // clear the color buffer

        float speed = 0.01f / cam.zoom;
        if (glfwGetKey(window, GLFW_KEY_W)) cam.set_pos(cam.pan_x, cam.pan_y + speed);
        if (glfwGetKey(window, GLFW_KEY_S)) cam.set_pos(cam.pan_x, cam.pan_y - speed);
        if (glfwGetKey(window, GLFW_KEY_A)) cam.set_pos(cam.pan_x - speed, cam.pan_y);
        if (glfwGetKey(window, GLFW_KEY_D)) cam.set_pos(cam.pan_x + speed, cam.pan_y);

        if (glfwGetKey(window, GLFW_KEY_Q)) cam.set_zoom(cam.zoom * 1.01f);
        if (glfwGetKey(window, GLFW_KEY_E)) cam.set_zoom(cam.zoom * 0.99f);

        cam.update_vp();

        glUseProgram(trail_shader_program);
        glUniformMatrix4fv(trail_vp_loc, 1, GL_FALSE, cam.vp);

        for (const auto& body : bodies) {
            if (body.trail_vertices.size() > 3) {
                glBindVertexArray(VAO);
                glBindBuffer(GL_ARRAY_BUFFER, VBO);
                glBufferSubData(GL_ARRAY_BUFFER, 0, body.trail_vertices.size() * sizeof(float), body.trail_vertices.data());
                glUniformMatrix4fv(trail_vp_loc, 1, GL_FALSE, cam.vp);
                glDrawArrays(GL_LINE_STRIP, 0, body.trail_vertices.size()/vertex_stride_floats);
            }
        }

        for (const auto& body: bodies) {
            if (body.type == "sun") {
                glUseProgram(sun_shader_program);
                glUniform2f(sun_center_loc, (float)body.pos.x, (float)body.pos.y);
                glUniform1f(sun_radius_loc, (float)body.radius);

                glUniformMatrix4fv(sun_vp_loc, 1, GL_FALSE, cam.vp);
                draw_circle(body.pos.x, body.pos.y, (float)body.radius, VBO, VAO);
            } else if (body.type == "earth") {
                glUseProgram(earth_shader_program);
                glUniform2f(earth_center_loc, (float)body.pos.x, (float)body.pos.y);
                glUniform1f(earth_radius_loc, (float)body.radius);

                glUniformMatrix4fv(earth_vp_loc, 1, GL_FALSE, cam.vp);
                draw_circle(body.pos.x, body.pos.y, (float)body.radius, VBO, VAO);
            } else if (body.type == "ice") {
                glUseProgram(ice_shader_program);
                glUniform2f(ice_center_loc, (float)body.pos.x, (float)body.pos.y);
                glUniform1f(ice_radius_loc, (float)body.radius);
                
                glUniformMatrix4fv(ice_vp_loc, 1, GL_FALSE, cam.vp);
                draw_circle(body.pos.x, body.pos.y, (float)body.radius, VBO, VAO);
            }
        }

        ImGui::Begin(
            "Simulation Stats",
            nullptr,
            ImGuiWindowFlags_NoBackground |
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoCollapse
        );
        ImGui::SetNextWindowPos(ImVec2(400, 400), ImGuiCond_Always);
        ImGui::Text("Time scale: %.2e seconds per real second", time_scale);
        ImGui::Text("Number of bodies: %d", static_cast<int>(bodies.size()));
        ImGui::Text("Total energy: %.2e J", total_energy(bodies));
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window); // swap the front and back buffers
        glfwPollEvents(); // check for events (like key presses, mouse movement, etc.)
    }

    glDeleteProgram(sun_shader_program);
    glDeleteProgram(earth_shader_program);
    glDeleteProgram(ice_shader_program);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
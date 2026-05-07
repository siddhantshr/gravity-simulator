#include "utils.h"
#include "physics.h"

std::string read_file(const char* path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << path << "\n";
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void camera_init(Camera& cam, const std::vector<Body>& bodies, float aspect_ratio) {
    if (bodies.empty()) return;

    double min_x = bodies[0].pos.x;
    double max_x = bodies[0].pos.x;
    double min_y = bodies[0].pos.y;
    double max_y = bodies[0].pos.y;
    double body_radius_m = 0.0;

    for (const auto& body : bodies) {
        min_x = std::min(min_x, body.pos.x);
        max_x = std::max(max_x, body.pos.x);
        min_y = std::min(min_y, body.pos.y);
        max_y = std::max(max_y, body.pos.y);
        body_radius_m = std::max(body_radius_m, body.radius);
    }

    min_x -= body_radius_m;
    max_x += body_radius_m;
    min_y -= body_radius_m;
    max_y += body_radius_m;

    const double cx = (min_x + max_x) * 0.5;
    const double cy = (min_y + max_y) * 0.5;
    cam.set_pos((float)cx, (float)cy);

    const double margin = 1.2;
    double half_w = (max_x - min_x) * 0.5;
    double half_h = (max_y - min_y) * 0.5;

    const double min_half = std::max(body_radius_m * 1.5, 1.0);
    half_w = std::max(half_w * margin, min_half);
    half_h = std::max(half_h * margin, min_half);

    const float safe_aspect = (aspect_ratio > 0.0f) ? aspect_ratio : 1.0f;
    const float zoom_x = static_cast<float>(safe_aspect / half_w);
    const float zoom_y = static_cast<float>(1.0 / half_h);
    cam.zoom = std::min(zoom_x, zoom_y);
    cam.update_projection_matrix(safe_aspect, 1.0f);
}

void load_config() {
    YAML::Node config = YAML::LoadFile("./configuration.yaml");
    if (!config["bodies"] || !config["bodies"].IsSequence()) {
        std::cerr << "Invalid config: 'bodies' key is missing or not a sequence\n";
        return;
    }
    for (const auto& body : config["bodies"]) {
        bodies.emplace_back(
            body["mass"].as<double>(),
            body["pos"].as<std::vector<double>>(),
            body["vel"].as<std::vector<double>>(),
            body["radius"].as<double>(),
            body["type"].as<std::string>()
        );
    }
}
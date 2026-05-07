#pragma once
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include "physics.h"
#include <yaml-cpp/yaml.h>

extern std::vector<Body> bodies; // global variable to hold the bodies in the simulation

class Camera {
    float view_matrix[16];
    float projection_matrix[16];
public:
    float zoom;
    float pan_x;
    float pan_y;
    float vp[16]; // view projection matrix

    Camera() : zoom(1.0f), pan_x(0.0f), pan_y(0.0f) {} // zoom > 1 means zoom in, zoom < 1 means zoom out

    void set_pos(float x, float y) {
        pan_x = x;
        pan_y = y;
        update_view_matrix();
    }

    void set_zoom(float z) {
        zoom = z;
        update_projection_matrix();
    }

    void update_view_matrix() {
        // x' = x - pan_x
        // y' = y - pan_y

        // my pos is (x,y,0,1) and i want to move it to (x-pan_x, y-pan_y, 0, 1)
        // so the view matrix should be:
        // [1, 0, 0, -pan_x]
        // [0, 1, 0, -pan_y]
        // [0, 0, 1, 0]
        // [0, 0, 0, 1]

        // identity matrix
        for (int i = 0; i < 16; i++) view_matrix[i] = 0; // initialize to zero
        view_matrix[0] = 1;
        view_matrix[5] = 1;
        view_matrix[10] = 1;       
        view_matrix[15] = 1;

        view_matrix[12] = -pan_x;
        view_matrix[13] = -pan_y;
    }

    void update_projection_matrix(float width = 1000, float height = 1000) {
        // left -> -1
        // right -> 1
        // top -> 1
        // bottom -> -1

        // x' = Ax + B
        // -1 = Al + b
        // 1 = Ar + b
        // A = 2/(r-l)
        // B = -(r+l)/(r-l)

        float aspect_ratio = width / height;
        float left = -aspect_ratio / zoom;
        float right = aspect_ratio / zoom;
        float top = 1.0f / zoom;
        float bottom = -1.0f / zoom;


        // initialize to zero
        for (int i = 0; i < 16; i++) projection_matrix[i] = 0;

        projection_matrix[0] = 2.0f / (right - left);
        projection_matrix[5] = 2.0f / (top - bottom);
        projection_matrix[10] = -1.0f; // for 2D rendering
        projection_matrix[15] = 1.0f;
        projection_matrix[12] = -(right + left) / (right - left);
        projection_matrix[13] = -(top + bottom) / (top - bottom);
    }

    void multiply(const float* a, const float* b, float* result) {
        // Column-major 4x4 multiply (OpenGL layout): result = a * b.
        float tmp[16];
        for (int col = 0; col < 4; col++) {
            for (int row = 0; row < 4; row++) {
                tmp[col * 4 + row] = 0.0f;
                for (int k = 0; k < 4; k++) {
                    tmp[col * 4 + row] += a[k * 4 + row] * b[col * 4 + k];
                }
            }
        }
        for (int i = 0; i < 16; i++) result[i] = tmp[i];
    }

    void update_vp() {
        multiply(projection_matrix, view_matrix, vp);
    }
};

std::string read_file(const char* path);
void camera_init(Camera& cam, const std::vector<Body>& bodies, float aspect_ratio = 1.0f);
void load_config();
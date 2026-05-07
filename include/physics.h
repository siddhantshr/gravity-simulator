#pragma once
#include <vector>
#include <cmath>
using namespace std;

class Vec {
public:
    double x;
    double y;

    Vec(double _x, double _y) {
        x = _x;
        y = _y;
    }

    Vec() {
        x = 0;
        y = 0;
    }

    Vec operator+(const Vec& other) const {
        return Vec(x + other.x, y + other.y);
    }

    Vec& operator+=(const Vec& other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    Vec operator*(double scalar) const {
        return Vec(x * scalar, y * scalar);
    }

    Vec operator-(const Vec& other) const {
        return Vec(x - other.x, y - other.y);
    }

    Vec operator/(double scalar) const {
        return Vec(x / scalar, y / scalar);
    }

    double norm(bool soften) const {
        return sqrt(x * x + y * y + (soften ? 1e-10 : 0)); // add small value to prevent division by zero
    }
};

class Body {
public:
    double mass;
    Vec pos;
    Vec vel;
    Vec acc;
    double radius; // for rendering, not used in physics calculations
    std::string type; // RGB values for rendering
    vector<float> trail_vertices; // to store previous positions for rendering trails
    
    Body(double m, vector<double> p, vector<double> v, double r, std::string t) 
        : mass(m), pos(p[0], p[1]), vel(v[0], v[1]), acc({0.0, 0.0}), radius(r), type(t) {}
};

Vec compute_force(const Body& b1, const Body& b2);
unsigned long long compute_dt(const vector<Body>& bodies);
double total_energy(const vector<Body>& bodies);
void accn_init(vector<Body>& bodies);
void update(
    vector<Body>& bodies,
    double dt
);
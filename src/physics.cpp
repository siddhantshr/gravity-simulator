#include <iostream>
#include <vector>
#include <cmath>
#include <thread>
#include <print>
#include <cfloat>
#include <chrono>
#include "physics.h"
using namespace std;

Vec compute_force(const Body& b1, const Body& b2) {
    Vec r = b2.pos - b1.pos;
    // F vec = (Gm1m2 (r2 - r1 vec))/||r||^3
    double _dist = r.norm(true); // add small value to prevent division by zero
    double _const = (6.67430e-11)*b1.mass*b2.mass;
    _const /= _dist*_dist*_dist;

    Vec force = r * _const;

    return force;
}

unsigned long long compute_dt(const vector<Body>& bodies) {
    double min_dist = DBL_MAX;
    int num_bodies = bodies.size();
    for (int i = 0; i < num_bodies; i++) {
        for (int j = i + 1; j < num_bodies; j++) {
            Vec r = bodies[j].pos - bodies[i].pos;
            double dist = r.norm(false);
            min_dist = min(min_dist, dist);
        }
    }

    if (min_dist > 1e6) { // if bodies are very far apart, we can use a larger dt
        return 100000; // 0.1 seconds in microseconds
    }
    if (min_dist < 100) { // if bodies are very close, we need a smaller dt
        return 1000; // 0.001 seconds in microseconds
    }

    unsigned long long dt_micro = (100000)*pow((min_dist/1000), 1.1); // r0 = 1000, x = 1.5 are arbitrary values that can be tuned (dt in microseconds)

    return min(dt_micro, 1000000ULL); // cap dt at 1 second in microseconds
}

double total_energy(const vector<Body>& bodies) {
    double energy = 0;
    int num_bodies = bodies.size();
    for (int i = 0; i < num_bodies; i++) {
        energy += 0.5 * bodies[i].mass * (bodies[i].vel.x * bodies[i].vel.x + bodies[i].vel.y * bodies[i].vel.y); // kinetic energy
        for (int j = i + 1; j < num_bodies; j++) {
            Vec r = bodies[j].pos - bodies[i].pos;
            energy -= (6.67430e-11)*bodies[i].mass*bodies[j].mass / r.norm(false); // potential energy
        }
    }
    return energy;
}

void accn_init(vector<Body>& bodies) {
    int num_bodies = bodies.size();
    for (int i = 0; i < num_bodies; i++) {
        for (int j = i + 1; j < num_bodies; j++) {
            auto force = compute_force(bodies[i], bodies[j]);
            bodies[i].acc += force / bodies[i].mass;
            bodies[j].acc += force * (-1) / bodies[j].mass;
        }    
    }
}

void update(
    vector<Body>& bodies,
    double dt
) {
    int num_bodies = bodies.size();
    // try to implement velocity verlet integration
    // x(t + dt) = x(t) + v(t)*dt + 0.5*a(t)*dt^2
    // v(t + dt) = v(t) + 0.5*(a(t) + a(t + dt))*dt

    for (int i = 0; i < num_bodies; i++) {
        bodies[i].pos += bodies[i].vel * dt; // update position with velocity
        bodies[i].pos += bodies[i].acc * (0.5 * dt * dt); // update position with acceleration
    }

    vector<Vec> temp(num_bodies, Vec(0,0)); // temporary vector to store new accelerations
    for (int i = 0; i < num_bodies; i++) { // a(t+dt) is computed after updating positions
        for (int j = i + 1; j < num_bodies; j++) {
            auto force = compute_force(bodies[i], bodies[j]); 
            temp[i] += force / bodies[i].mass;
            temp[j] += force * (-1) / bodies[j].mass;
        }    
    }

    for (int i = 0; i < num_bodies; i++) {
        bodies[i].vel += (temp[i] + bodies[i].acc) * dt * 0.5; // update velocity with acceleration
    }

    // update accelerations for next iteration
    for (int i = 0; i < num_bodies; i++) {
        bodies[i].acc = temp[i];
    }
}

// int main() {
//     // update loop updates every 0.01 seconds
//     vector<Body> bodies;
//     bodies.push_back(Body(1e12, {0,0}, {1000,0}, {0,0}));
//     bodies.push_back(Body(1e12, {100000,0}, {0,1000}, {0,0}));
//     bodies.push_back(Body(1e12, {50000,86600}, {0,0}, {0,0}));
//     accn_init(bodies);

//     while (true) {
//         println("Current state:");
//         int idx = 1;
//         for (const auto& body : bodies) {
//             println("Body: {} Mass: {} Position: ({}, {}) Velocity: ({}, {}) Acceleration: ({}, {})", 
//                 idx, body.mass, body.pos.x, body.pos.y, body.vel.x, body.vel.y, body.acc.x, body.acc.y);
//             println("");
//             idx++;
//         }
//         println("Total energy: {}", total_energy(bodies));
//         println("----------------");

//         unsigned long long dt = compute_dt(bodies);

//         update(bodies, dt/1000000.0); // convert dt from microseconds to seconds
//         std::this_thread::sleep_for(std::chrono::microseconds(dt)); // sleep for dt microseconds
//     }
// }
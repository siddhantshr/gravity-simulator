[AI generated README.md]

# Gravity Simulator

A real-time N-body gravitational simulation visualizer built with C++, OpenGL, and GLFW. Watch planetary bodies orbit under Newtonian gravity with adaptive timesteps and visual trails.

## Features

- **Real-time N-body simulation** using Newton's law of universal gravitation
- **Velocity Verlet integration** for accurate and stable physics
- **Adaptive timesteps** that adjust based on minimum inter-body distance
- **OpenGL rendering** with camera pan/zoom controls
- **Visual trails** showing body trajectories with fade-out effect
- **Configuration-driven** — load initial bodies from YAML
- **Efficient builds** — Dear ImGui compiled into static library

## Building

### Prerequisites
- macOS 11+ (clang++ with C++23 support)
- GLFW3 static library (`lib/libglfw3.a`)
- yaml-cpp static library (`lib/libyaml-cpp.a`)
- Dear ImGui (built automatically into `lib/libimgui.a`)

### Build Instructions

```bash
# Clean build
make clean && make

# Run the simulator
make run

# Or run directly
./build/app
```

The makefile will:
1. Compile the app and link against all libraries
2. Output binary to `build/app`

## Running the Simulator

### Controls

| Key | Action |
|-----|--------|
| `W` | Pan camera up |
| `S` | Pan camera down |
| `A` | Pan camera left |
| `D` | Pan camera right |
| `Q` | Zoom in |
| `E` | Zoom out |
| `ESC` | Close window |

### Configuration

Edit `configuration.yaml` to define initial bodies:

```yaml
bodies:
  - mass: 1.989e30        # kg
    pos: [0.0, 0.0]       # meters
    vel: [0.0, 0.0]       # m/s
    radius: 6.963e9       # meters (display radius)
    type: sun             # label (optional)
  
  - mass: 5.972e24
    pos: [1.496e11, 0.0]
    vel: [0.0, 29780.0]
    radius: 6.371e8
    type: earth
```

The simulator loads `configuration.yaml` on startup and initializes all bodies with their gravitational accelerations.

## Physics Methods

### 1. Gravitational Force Calculation

The simulator applies Newton's law of universal gravitation:

$$\vec{F} = \frac{G m_1 m_2}{|\vec{r}|^3} \vec{r}$$

where:
- **G** = 6.67430×10⁻¹¹ m³/(kg·s²) (gravitational constant)
- **m₁, m₂** = masses of two bodies
- **r** = position vector from body 1 to body 2

**Implementation:** [src/physics.cpp](src/physics.cpp#L11) — computes the magnitude-normalized force vector with softening (1e-10 added to distance norm) to prevent singularities.

### 2. Velocity Verlet Integration

Uses a two-step integration scheme for stability and energy conservation:

$$x(t + \Delta t) = x(t) + v(t) \Delta t + \frac{1}{2} a(t) \Delta t^2$$

$$v(t + \Delta t) = v(t) + \frac{1}{2} [a(t) + a(t + \Delta t)] \Delta t$$

**Why Verlet?**
- Symplectic integrator (conserves energy better than Euler)
- Second-order accuracy
- Avoids storing old velocities (compact memory)

**Implementation:** [src/physics.cpp](src/physics.cpp#L81) — positions are updated first, then accelerations are recomputed at the new configuration, and velocities are updated with the average acceleration.

### 3. Adaptive Timestep

The simulator adjusts the timestep dynamically based on the **minimum inter-body distance** to balance accuracy and performance:

let $d_{min} = \min_{i \neq j} |\vec{r}_i - \vec{r}_j|$

Then compute the timestep as:
$$dt = \min\left(10^6, \max\left(10^3, 10^5 \cdot \left(\frac{d_{min}}{10^6}\right)^{1.1}\right)\right)$$

where:

- $dt$ is in microseconds
- $d_{min}$ is the minimum pairwise distance between bodies

**Purpose:** Keeps the integrator accurate during close approaches while running fast when bodies are far apart.

### 4. Gravitational Acceleration Initialization

All bodies start with acceleration computed from every other body:

$$a_i = \sum_{j \neq i} \frac{G m_j}{|\vec{r}_{ij}|^3} \vec{r}_{ij}$$

This ensures the first physics step is accurate without relying on assumptions about initial velocities.

## Project Structure

```
gravity/
├── README.md              # This file
├── configuration.yaml     # Initial body configuration
├── makefile              # Build rules
├── include/
│   ├── physics.h         # N-body physics interface
│   ├── utils.h           # Camera and file utilities
│   ├── imgui/            # Dear ImGui headers (precompiled into lib/libimgui.a)
│   └── glfw/             # GLFW headers
├── lib/
│   ├── libglfw3.a        # GLFW static library
│   ├── libyaml-cpp.a     # YAML parsing library
│   ├── libimgui.a        # Dear ImGui (built by makefile)
│   └── imgui/            # ImGui source (compiled)
├── src/
│   ├── main.cpp          # Renderer, camera, input handling
│   ├── physics.cpp       # Force, integration, timestep logic
│   ├── utils.cpp         # Camera math, file I/O
│   └── glad.c            # OpenGL function loader
├── shaders/
│   ├── body/
│   │   ├── vertex.glsl   # Body rendering vertex shader
│   │   └── fragment.glsl # Body rendering fragment shader
│   └── trail/
│       ├── vertex.glsl   # Trail rendering vertex shader (with alpha)
│       └── fragment.glsl # Trail rendering fragment shader (with alpha blending)
└── build/                # Build outputs (created by make)
    ├── app               # Compiled binary
    ├── libimgui.a        # Compiled ImGui archive
    └── imgui/            # ImGui object files
```

## Rendering Details

### Camera System
- Orthographic projection with zoom and pan
- View-projection matrix transforms world-space bodies to NDC (-1 to 1)
- Camera initialized to fit all bodies + 20% margin in view

### Body Rendering
- Circles rendered as triangle fans (centered on body position)
- Radius passed in meters (world space)
- Full opacity (alpha = 1.0)

### Trail Rendering
- Stores last 1000 (x, y, alpha) vertices per body
- Alpha fades from 0 (oldest) to 1 (newest)
- GL_BLEND enabled with SRC_ALPHA / ONE_MINUS_SRC_ALPHA blending
- LINE_STRIP topology

## Performance Notes

- **O(N²)** force calculation (all-pairs)
- Typical 60 FPS @ 3–4 bodies with adaptive timesteps
- Each frame can run multiple physics substeps depending on `time_scale` and `dt`
- Shader compilation happens on startup

## Example: Earth–Sun System

Using configuration.yaml with the Sun and Earth:
- Mass ratio ≈ 333,000:1
- Orbital velocity ≈ 29.78 km/s at 1 AU
- Verlet integration gives stable Kepler orbit over weeks of simulated time
- Visual trail shows near-perfect ellipse (minor eccentricity in config)

## Future Enhancements

- Barnes–Hut or FMM for $O(N \log N)$ force calculation
- Collision detection and merging
- Configurable rendering (particle systems, halos)
- Pause/resume and time-scale UI controls
- Multi-threaded force calculation
- Support for 3D rendering

## References

- Newton's Law of Universal Gravitation: https://en.wikipedia.org/wiki/Newton%27s_law_of_universal_gravitation
- Velocity Verlet Integration: https://en.wikipedia.org/wiki/Verlet_integration
- N-body Problem: https://en.wikipedia.org/wiki/N-body_problem
- GLFW Documentation: https://www.glfw.org/documentation.html
- Dear ImGui: https://github.com/ocornut/imgui

## License

See [LICENSE](LICENSE).

[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/jeregon1/Graphics)

# Graphics Project Structure

This project is organized to support a modular ray tracing and rendering pipeline, with support for various rendering algorithms, scene descriptions, and image output formats. Below is an overview of the main directories and files:

## Directory Structure

- **src/**: Source code for the core rendering engine and utilities.
  - `main.cpp`: Main entry point for the renderer.
  - `scene.cpp`: Scene management, including object and light setup, YAML parsing, and photon mapping radiance estimation.
  - `object3D.cpp`, `material.cpp`, `geometry.cpp`: Implementations for 3D objects, materials, and geometric utilities.
  - `pinholeCamera.cpp`: Camera model for rendering.
  - `parallel_renderer.cpp`: Parallel rendering logic.
  - `rendering_strategy.cpp`: This is where the different rendering strategies have their main method implemented.`
  - `render_config.cpp`: Configuration for rendering algorithms and modes.
  - `acceleration.cpp`: Acceleration structures (e.g., KD-tree) for efficient ray tracing. But we have not used it at the end.
  - `kernel.cpp`: Kernels for photon mapping.
  - `toneMapping.cpp`: Tone mapping operators for image output.
  - `Image.cpp`: Image I/O (PPM, BMP) and manipulation.

- **include/**: C++ header files for all major classes and utilities, matching the source files in `src/`.

- **test/**: Test programs for different components and features.
  - `test_p2.cpp`: Tests for image I/O and tone mapping.
  - `test_p3.cpp`: Comprehensive tests for ray tracing, path tracing, parallelization, and acceleration structures.
  - `test_bmp.cpp`: Tests for BMP image format I/O and conversion.
  - `test_geometry.cpp`: Tests for geometric primitives (spheres, planes, triangles, quads).
  - `test_intersect.cpp`: Tests for ray-object intersection calculations.
  - `test_parallel.cpp`: Tests for parallel rendering with multi-threading.
  - `test_yaml.cpp`: Tests for YAML-based scene and configuration loading/saving.

- **assets/**: Example input images (PPM) and zipped datasets for testing and demonstration.

- **configs/**: YAML configuration files for rendering settings.

- **scenes/**: YAML scene descriptions, including geometry, materials, lights, and camera setups.

- **test_outputs/**: Output directory for generated images and YAML files from tests.

- **build/**: Compiled object files and binaries.

- **Makefile**: Build instructions for the project.
- **README.md**: This file.

## Key Concepts

- **Rendering Algorithms**: Supports ray tracing, path tracing, and photon mapping.
- **Parallelization**: Multi-threaded rendering with configurable region types and thread counts.
- **Acceleration Structures**: Optional KD-tree for efficient intersection tests in complex scenes.
- **YAML Support**: Scenes and render configs can be loaded and saved in YAML format for flexibility.
- **Image Output**: Supports PPM and BMP formats, with tone mapping and post-processing.
- **Bilateral Filter**: Supports addition of bilateral filter in indirect light
for better looking and faster images.

## Getting Started

1. Build the project using `make`.
2. To render images run `./build/main --help` for usage instructions.
3. To define scenes, use the YAML format described in `scenes/scene_yaml_format.md`.
4. To change the rendering configuration, edit the YAML file `default_pathtracing_config.yaml`.
5. To run only tonemap operators use `./build/tonemap` for instructions.


For more details, see the comments in the source and test files.

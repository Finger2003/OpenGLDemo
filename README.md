# OpenGLDemo #
A demonstration application built using OpenGL that loads and displays 3D models with interactive controls and advanced rendering features. The application utilizes the Phong lighting model and supports switching between Phong and Blinn shading models.The scene includes multiple dynamic light sources, a moving model, and a reflective mirror object.

The app allows users to switch between multiple camera perspectives, adjust fog intensity, and toggle between day and night scenes, all while offering flexible camera movement for an immersive experience.

## Features ##
- ### Phong Lighting Model ###
    - The app uses the Phong lighting model for realistic light reflection and shading.

    - Supports switching between **Phong** and **Blinn shading** models for different lighting effects.

    - Three Light Sources:
        - **Point Light:** A point light source for localized lighting.
        - **Spotlight:** Shines from the moving model.
        - **Directional Light:** Simulates sunlight during the day.

- ### Interactive Camera Views ###
    - Choose from 4 different camera modes:
        - **Completely still camera:** A fixed position and angle camera.       
        - **Still but following camera:** Remains stationary but tracks the movement of the moving model.
        - **Following camera from behind:** Follows the moving model from behind, simulating a trailing perspective.
        - **Free camera:** Move and look around the scene freely using keyboard and mouse controls.

- ### Mirror Reflection ###
    - One of the models in the scene behaves like a mirror, dynamically reflecting the surrounding environment.

- ### Moving Model ###
    - A model moves along a predefined trajectory in the scene. The user can rotate this model relative to its base angle (tangential to its trajectory).

- ### Fog Effect ###
    - Add and adjust fog intensity in the scene for atmospheric effects.
    - Customize fog levels for different visibility and mood settings.

- ### Day/Night Mode ###
    - Switch between day and night modes for varied lighting and ambiance.

## Controls ##
### Camera Movement (Free Camera Mode) ###
- **W, A, S, D:** Move the camera forward, left, backward, and right.

- **Left Shift, Left Control:** Move the camera up and down.

- **Mouse:** Look around by rotating the camera in all directions.

### Fog Control ###
- **F, G:** Increase or decrease fog intensity.

### Moving Model Rotation ###
- **Up, Down, Right, Left Arrows:** Rotate the moving model around its base angle (tangential to its path).

### Camera Switching ###
- **J, K:** Switch between the 4 camera views (completely still, still but following, following from behind, free camera).

### Day/Night Mode ###
- **N:** Toggle between day and night lighting in the scene.

### Shading Model ###
- **B:** Switch between Phong and Blinn shading models for different lighting effects.


## Attribution ##
This application includes **modified code** from **Joey de Vries' LearnOpenGL** repository. The original code and certain assets have been adapted and expanded for this application. The original code, along with its license, is available [here](https://github.com/JoeyDeVries/LearnOpenGL). Author's personal twitter handle: https://twitter.com/JoeyDeVriez

Additionally, the application uses the following third-party libraries, **compiled for Windows**:
- [**GLFW:**](https://www.glfw.org/https://www.glfw.org/) A multi-platform library for creating windows and handling input.
- [**Assimp:**](https://assimp.org/) Open Asset Import Library for loading and processing 3D models.
- [**Glad:**](https://glad.dav1d.de/) A loader for OpenGL extensions and function pointers, included as `glad.c`.

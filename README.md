# Wisteria Engine

Wisteria Engine is a simple 3D game engine written in C++ and OpenGL. Its design is made to be a declarative battery-included alternative to the traditional imperative OpenGL programming. It is inspired by the Unity3D Scripting API, but provides a code-first approach and interoperability with low-level OpenGL code.

This project is still in its early stages, and is not yet ready for production use. However, to showcase its capabilities, a simple 3D scene of a mountainscape at night with waterfalls, a lake, skybox, reflections and lights has been implemented. 

At this moment, the engine is capable of two rendering modes: forward rendering with manually written lighting shaders, and deffered rendering with pre-built lighting utilities. 

## Features
- Meshes and materials
- Textures and skyboxes
- Cameras (main and additional, multiple cameras at once, camera switching etc.)
- Lights in deffered rendering mode
- Rendering to Textures and Framebuffers
- Framebuffers
- Full-fledged hierarchical game object system with scriptable behaviours and events
- Minimal Physics Engine
- Asset Management
- Basic Collision Detection
- Basic Particle System
- Helpful shader utilities

## Future Plans
- Better shader management and utilities, with respect to the rendering mode
- More advanced lighting and shadowing techniques
- More advanced collision detection and physics
- More advanced particle system

## Note
- At this point, the engine is written on top of [GFX Framework](https://github.com/UPB-Graphics/gfx-framework), but it is planned to be made standalone in the future.
- More documentation and examples will be added in the future.
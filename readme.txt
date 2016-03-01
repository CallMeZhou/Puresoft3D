Puresoft 3D Feature Plan

Summary

'Puresoft 3D' is a software simulated programmable graphics pipline programme. It mostly inherits the approaches and imitates the behaviors of modern programmable graphics pipline hardwares, also known as 3D acceleration cards, by which it renders 3D scenes at a technical level close to modern 3D games.

Purpose

The major goal of this project is to advertise myself on the internet in order to get better opportunity for career. When all features (without asterisk) below are completed, it will be published on Github. Presenting it in job interview is an effective but passive way to show it, however I may have to think of other more proactive ways for self advertising.

Features

Framework functions [V]
    Vertex mesh projection and rendering [V]
        Vertex transformation [V]
        Rasterization [V]
        Interpolation [V]
        Fragment filling [V]
        Bitblt [V]
    VAO / VBO [V]
    FBO [V]
    Uniform [V]
    Vertex shader [V]
    Fragment shader [V]
    Texture [V]
    Parallel processing [-]
        * Parallel vertex processing [ ]
        Parallel fragment processing [V]
Shader functions
    Realtime lightening (Phong) [V]
    Vertex colour [V]
    Diffuse texture / multi-texture blending [V]
    Spacularity texture [V]
    Bump mapping [V]
    Skybox mapping [V]
    Environment reflection mapping [ ]
    Shadow [ ]
        * Volume shadow [ ]
        * Shadow mapping [ ]
    Deferred randering [ ]
Content functions
    Simple scene (by OBJX) [V]
    Scene editor (at least a simplest version) [ ]
    Complex scene (by FBX) [ ]
    * Skeleton animation [ ]

Asterisk features

Some features above have asterisk sign, meaning they are not planned as a must. Here are reasons for why they are just nice to have.

Parallel vertex processing

I only have a machine with 2 cores / 4 threads CPU, and fragment processing is usually much more heavy then vertex processing, so I have to leave 1 thread for vertices and use the rest 3 for fragments. Processing vertices by more threads needs more cores indeed.

Environment reflection mapping

It is impossible for CPU simulated pipeline to have performance as good as a common 3D card. This feature is obviously too heavy for CPU.

Shadow mapping

Both shadowing methods are way too heavy for CPU. They both require high buffer fill rate.

Skeleton animation

Well...less interesting but very time consuming I guess. I have to publish the project sooner.
# Microsoft's C++ devcontainer base — includes gcc, g++, cmake, git, gdb
FROM mcr.microsoft.com/devcontainers/cpp:1-ubuntu-24.04

# System libraries required by raylib on Linux (X11 + OpenGL)
RUN apt-get update && apt-get install -y --no-install-recommends \
    libx11-dev \
    libxrandr-dev \
    libxinerama-dev \
    libxcursor-dev \
    libxi-dev \
    libxext-dev \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    libasound2-dev \
    ninja-build \
    && rm -rf /var/lib/apt/lists/*

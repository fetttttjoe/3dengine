# Intuitive Modeler - Engine Starter Project

This is a starter C++ project for building a 3D application using VS Code, CMake, GLFW, and OpenGL on Windows.

## Prerequisites

1.  **Visual Studio Code** with the **C/C++** and **CMake Tools** extensions.
2.  **Visual Studio Build Tools** (the C++ compiler). Make sure the "Desktop development with C++" workload is installed.
3.  **CMake** (added to the system PATH).

## How to Build and Run

The process is designed to be simple using the VS Code CMake Tools extension.

1.  **Open the Project**: Open the root folder (`IntuitiveModeler`) in VS Code.
2.  **Select a Kit**:
    - The first time you open the project, CMake Tools will try to scan for compilers ("kits").
    - A prompt may appear at the bottom of the screen. Choose a **Visual Studio** based kit (e.g., `Visual Studio Build Tools 2022 Release - amd64`).
    - If you don't see the prompt, press `Ctrl+Shift+P` to open the command palette, type `CMake: Select a Kit`, and choose from the list.
3.  **Configure the Project**:
    - Press `Ctrl+Shift+P` and type `CMake: Configure`.
    - This will run CMake and download the dependencies (GLFW, GLAD) for the first time. You will see output in the "Output" panel. This might take a minute.
4.  **Build the Project**:
    - Press `Ctrl+Shift+B`.
    - VS Code will pop up a dialog asking which build task to run. Select **"CMake: build"**.
    - This compiles your code and creates `IntuitiveModeler.exe` in a new `build` folder.
5.  **Run with the Debugger**:
    - Go to the "Run and Debug" panel on the left sidebar (the play button with a bug).
    - Click the green play button next to **"Debug (C++ / MSVC)"** at the top.
    - Your application window should appear, displaying an orange triangle.

You are now ready to start building your engine!

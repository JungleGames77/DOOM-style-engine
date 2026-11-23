# DoomCPP 
Visual Studio 2022, C++17, SFML 3.0.2, x64.

## Setup
1. Run `C:\vcpkg\vcpkg.exe install sfml:x64-windows`.
2. Run `C:\vcpkg\vcpkg.exe integrate install` once.
3. Open `DoomCPP.sln`, select Debug/x64, build and run.

## Controls
- WASD: move
- Left / Right arrows: rotate
- Tab: show / hide 2D ray map overlay
- Escape: quit

The 2D overlay uses the same DDA ray-wall intersection method as the main 3D projection. It shows 41 sampled rays for clarity, while the renderer casts 960 rays (one per screen column). This is a Wolfenstein-style raycaster, not the original Doom rendering engine.

No external textures are required in this version. SFML provides window/input/pixel presentation; raycasting logic is custom C++.

# Blind Box Battle Arena

A two-player OpenGL battle game created for a Computer Graphics group project. Players choose a character, battle in a 3D arena, and use distinct movement and combat skills.

## Highlights

- Two-player local battle system
- Playable character selection and abilities
- Health, timer, cooldown, and hit-detection mechanics
- Character animation, lighting, shading, texture mapping, and visual effects
- Battle-history tracking

## Tech Stack

- C++
- OpenGL, GLU, GLUT
- Code::Blocks / MinGW
- Windows

## Run Locally

1. Install Code::Blocks with a MinGW C++ compiler.
2. Ensure OpenGL, GLU, and GLUT dependencies are available.
3. Open `Project_TCG6223_Group3_CodeBlocks/CGProject.cbp`.
4. Keep the `data/` folder, texture assets, and `glut32.dll` alongside the executable.
5. Build and run.

## Controls

| Action | Control |
| --- | --- |
| Start / confirm selection | Enter or Space |
| Pause / resume | P / C |
| Player 1 movement | W, A, S, D |
| Player 2 movement | Arrow keys |
| Player 1 skills | G, H |
| Player 2 skills | . / |
| Camera zoom | Mouse wheel or + / - |
| Camera rotation | Left mouse drag |
| Toggle wireframe / axes / lighting | F1 / F2 / F3 |
| Exit | Esc |

## Project Structure

```text
Project_TCG6223_Group3_CodeBlocks/
├─ CGProject.cbp
├─ CGLabmain.cpp
├─ THirono*.cpp / .hpp
├─ skill.cpp
├─ data/
└─ bin/Release/
```

## Project Context

This was a collaborative academic project. My work focused on contributing to the game implementation and the complete playable submission.
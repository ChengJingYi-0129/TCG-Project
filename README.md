# TCG6223 Blind Box Battle Arena

This project is created for TCG6223 Computer Graphics group project.

The project is an OpenGL blind box battle game inspired by POP MART characters. The two main characters are Twinkle-Twinkle Star Apple and Hirono The Rose. Both players can move in the battle arena and use different skills to attack each other.

## Team Members

| No. | Student ID | Name |
|---|---|---|
| 1 | 242UT244B7 | CHENG JING YI |
| 2 | 243UT246VF | HAU SHUNXIN |
| 3 | 243UT246VL | MANDY ONG WEI XUEN |
| 4 | 252UT253PR | TAI CHENG ZHENG |

## Requirements

- Windows 10
- Code::Blocks
- MinGW C++ Compiler
- OpenGL, GLU and GLUT library

## How to Run

1. Open `CGProject.cbp` by using Code::Blocks.
2. Build and run the project by using MinGW C++ compiler.
3. Make sure the model files, `data` folder, texture image and `glut32.dll` are inside the project folder.
4. The release executable can also be found in `bin/Release/CGProject.exe`.

## Game Controls

| Function | Control |
|---|---|
| Start game / Confirm selection | Enter or Space |
| Back | B |
| Pause battle | P |
| Continue battle | C |
| Player 1 movement | W, A, S, D |
| Player 1 sprint | Left Shift |
| Player 1 Skill 1 | G |
| Player 1 Skill 2 | H |
| Player 2 movement | Arrow Keys |
| Player 2 sprint | Right Ctrl |
| Player 2 Skill 1 | . or > |
| Player 2 Skill 2 | / or ? |
| Zoom in / out | +, - or Mouse Wheel |
| Rotate view | Left mouse drag |
| Roll view | Right mouse drag |
| Toggle wireframe / fill | F1 |
| Toggle axis | F2 |
| Toggle lighting | F3 |
| Exit program | Esc |

## Game Features

- Two playable blind box characters
- Character selection screen
- Two player battle system
- Health point and timer system
- Character walking animation
- Apple Star and Apple Tree skills
- Vine Spike and Vine Tornado skills
- Skill cooldown and hit detection
- Lighting, shading and texture mapping
- Star field and meteor shower background effects
- Battle history system

## Project Structure
Project_TCG6223_Group3_CodeBlocks/
├─ CGProject.cbp
├─ CGLabmain.cpp
├─ CGLabmain.hpp
├─ THirono.cpp
├─ THirono.hpp
├─ THironoArena.cpp
├─ THironoBattle.cpp
├─ THironoCharacter.cpp
├─ THironoSkill.cpp
├─ skill.cpp
├─ character1_*.txt
├─ character2_*.txt
├─ 699pic_1ikoqx_xy.jpg
├─ glut32.dll
├─ data/
└─ bin/Release/
   ├─ CGProject.exe
   ├─ glut32.dll
   ├─ 699pic_1ikoqx_xy.jpg
   └─ data/

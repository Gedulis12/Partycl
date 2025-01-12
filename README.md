# Partycl

2D particle simulation written in C with SDL library. Inspired by [Pezza's Work video](https://www.youtube.com/watch?v=9IULfQH7E90&t=322s)

To build on linux run `./build.sh` and run via `./main`

Controlls:
|Button|Action|
|---|---|
|Middle mouse button| spawns particles rapidly|
|`Enter`| Spawns one particle|
|Left mouse button| attracts particles to the cursor|
|Right mouse button| pushess particles away from cursor|
|`r`| removes all particles|

Project is not complete, improvements are still needed:
- currently physics calculations are framerate dependant;
- performance is quite bad, simulations starts running below 60FPS at around ~1500 particles on my machine. Need to implement collision detection in "grid like fashion" instead of comparing positions of each particle with eachother;

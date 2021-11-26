# 453- Assignment 2
#### A game that has a player playing as a spaceship capturing diamonds, 17th October 2021
#### By **Jeremy Kimotho**
## Description
An application that uses images and opengl to create a simple point and attack game. The player is tasked with capturing four diamond and is congratulated if they achieve this feat. <br />
<br />
The vertices of the objects are passed onto the vertex shader at the start of the execution of the program and then never again. The rest of the movement and rotation seen by the user is a result of transformations executed by the vertex shader using matrices. These matrices are already coded in the vertex shader and uniforms are used to edit the matrices.
## Setup/Installation Requirements
* cmake -H. -Bbuild
* cd build 
* make
* ./453-skeleton

The first instruction requires cmake to be installed on the system.
## Gameplay Instructions 

| Move | Controls |
| --- | --- |
| Rotate | Left click location on screen with mouse |
| Proceed Forward | Press and hold up key |
| Return Backward | Press and hold down key |
| Attack | Move close enough to diamonds |
| Reset | Press j key |
## Technologies Used
* C++
* OpenGL 4
* imGUI
## Support and contact details
Email: projectsjeremy1000@gmail.com




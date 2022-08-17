# Minecraft-Clone
 
![MC pic1 (2)](https://user-images.githubusercontent.com/76859592/185042622-7f58d18d-ff80-4c58-8235-b309a13f9720.jpg)

This Minecraft clone is a final project I did for my Computer Graphics (CIS 460) course at UPenn. I worked in a team of three students where I worked on Multithreading, Procedural terrain and assets generation, and player physics. This project was done entirely in C++ and GLSL using the Qt GUI library and the OpenGL GLM library. We had approximately one month to work on this project.

 
 ## Features ##
- **Multithreading** Multithreaded dynamic rendering and derendering of terrain based on player location along with multithreaded filling of blocks based on procedural terrain functions to enable much faster performance. [(in-depth README)](https://github.com/slelyukh/Minecraft-Clone/blob/main/Feature%20ReadMEs/MultiThreadingREADME.txt)
- **Procedural Terrain** Nine different biomes were randomly generated using combinations of random noise functions such as Perlin and Worley noise with Smoothstep functions to allow properly sized areas of various biomes as well as fluid interpolation between biomes. [(in-depth README)](https://github.com/slelyukh/Minecraft-Clone/blob/main/Feature%20ReadMEs/AdditionalBiomesREADME.txt)
- **Procedural Assets** Trees, Cacti, Igloos, Pyramids, Boulders and Obsidian patches were added to various biomes through the clever use of Worley noise to ensure natural looking placement of assets. [(in-depth README)](https://github.com/slelyukh/Minecraft-Clone/blob/main/Feature%20ReadMEs/proceduralAssetsREADME.txt)
- **Player Physics** Fluid player physics such as collision with blocks without getting stuck to them along with support for placement and removal of blocks. [(in-depth README)](https://github.com/slelyukh/Minecraft-Clone/blob/main/Feature%20ReadMEs/proceduralAssetsREADME.txt)

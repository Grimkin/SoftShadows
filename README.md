# Real-Time Soft Shadows

This project is for Microsoft Visual Studio 2015 and requires at least DirectX 11 to run.

It is licensed under the FreeBSD license. For more info, read license.txt.

## Overview

This project can create real-time soft shadows. That means the shadows aren't precalculated but they are calculated dynamically every frame. 

A simple ray caster is used to create the shadows. Only a single shadow ray is shot per pixel and frame. The direction of the ray is altered for every frame. For each ray it is tested whether it hit something or not. Hitting something means that the starting point of the ray is in the shadows for the current ray direction. The starting point is lit if the current ray hit nothing. The results of consecutive frames are combined with an exponential moving average. This is nearly equal to testing multiple rays in a single frame with the advantage of only testing a single ray per frame. 

A voxel representation of the scene is used instead of the normal polygon representation to trace the shadow rays. This is due to the fact that a sparse voxel representation occupies less memory than a BVH, which is needed for testing against the polygon representation, and is faster to test against. The normal polygon scene is voxelized beforehand using a binary surface voxelizer on the graphics card to obtain a voxel representation of the scene. The voxel representation is used to build a 4^3-tree and the storage requirement is reduced by combining equal parts of the tree. This reduces the memory requirement by 70% to 90%.

The final soft shadow is obtained by a screen-space bilateral Gaussian smooth to get rid of the jittering artifacts.

## Usage

The project contains a simple scene with five pillars and plane. This scene will be loaded and voxelized with a resolution of 2048 if nothing changed. The scene, the resolution and more can be changed in main.config. Including using a coarser approximation when combining similar parts of the tree, storing and loading a voxelization or a complete voxel tree and enabling/disabling renormalization and backface culling.

A different scene can be loaded by changing "Pillar" at "String SceneName Pillar" to something else. Other preconfigured scenes are: 

* "Simple": A single sphere with a plane
* "Walls": A axis aligned wall and a not-axis aligned wall and a plane

The following scenes are not included in the Github, due to size restrictions. They can be downloaded separately and need to be put in "Assets\Geometries" of the execution folder. They also require a higher resolution for good results. This could take some time or could lead to time outs from the graphics card if the time out is left to the standard value. So it is advised to also download the given tree data, store it in the execution folder and load it. To load the tree, the "main.config" needs to be altered. "Bool LoadTree false" needs to be altered to "Bool LoadTree true" and "Dragon_16k.tr" at "String TreeLoadPath Dragon_16k.tr" needs to be altered to the downloaded tree data file. Nothing else needs to be altered. The resolution of the downloaded voxel files is 16k^3.

* "Outpost": A complex example scene from the Unreal Engine. <a href="https://1drv.ms/u/s!AghMjxborjfjvRPQK9iMAD0T_i_V" target="_blank" >Mesh</a>(172MB), <a href="https://1drv.ms/u/s!AghMjxborjfjvRTLgbXsaRYAHzol" target="_blank">Voxel-Data</a>(266MB)
* "Dragon": The XYZ-dragon with a plane. <a href="https://1drv.ms/u/s!AghMjxborjfjvRCvyCRgiZ4dZsfc" target="_blank" >Mesh</a>(20MB), <a href="https://1drv.ms/u/s!AghMjxborjfjvRI5BCipQ-VStIuM" target="_blank">Voxel-Data</a>(98MB)
* "SanMiguel": The SanMiguel Scene with lots of small details on the inside. <a href="https://1drv.ms/u/s!AghMjxborjfjvRWvgV2QlGq2nrA_" target="_blank" >Mesh</a>(505MB), <a href="https://1drv.ms/u/s!AghMjxborjfjvRH2A57YQf7p9a8k" target="_blank">Voxel-Data</a>(50MB)

Other geometries can also be loaded. By changing "Pillar" at "String SceneName Pillar" to the name of the geometry. Either .obj or .mesh files can be loaded that are stored in "Assets\Geometries". For large and complex scenes, it is advised to convert .obj-files to .mesh-files using "GeometryConverter.exe". It is a command line program that converts .obj-files in .mesh-files and combines on standard multiple meshes within a .obj-file to a single mesh in the resulting .mesh-file.

The usage is as follows:

	GeometryConverter.exe inputfile [-o outputfile] [-c <combineMeshes=true>]

If no output file is specified, it uses the name of the input file + .mesh as output file name.

The position, scale and rotation of the loaded mesh can be altered by changing "Float3 ScenePosition -2 -2 0", "Float3 SceneScale 2 2 2" and "Float3 SceneRotation 90 0 0".

The main program is Engine.exe. It only reads the main.config and ignores command line arguments.

### Hotkeys

* WASD / Arrow keys:	Moving the camera
* PicUp / PicDown:		Moving the camera up and down in world space
* RMB + Mouse move:		Altering camera view
* F:					Taking a screenshot without the UI
* U:					Disabling the UI
* F1:					Switching to the voxel view
* F5:					Storing the current camera position and orientation in a save file
* F6:					Loading camera position and orientation from the save file
* F10:					Switch to full screen
* Alt+F4:				Exiting application

### UI

* FPS: Shows the current FPS of the program
* Max Iteration(Voxel View Mode): In the voxel view mode it is shown how many iteration steps it took to find the hit with the voxel structure. A lighter color means many steps had to be taken to find the hit, a darker color means less steps needed to be taken. This option changes the number of iteration steps needed so that the full color is shown. For higher resolutions this number needs to be higher than the initial 20 to see any structures.
* Light Size Angle: Controls the size of the light source in degrees. The current program only supports a sun like light source. I.e. a light source that is infinitely far away from the scene. For the sun this value would be 0.5 degrees since the appears to be only 0.5 degrees big from the earth.
* Vertical Light Angle: Controls the vertical angle of the light source
* Horizontal Light Angle: Controls the horizontal angle of the light source
* Radius Modifier: Controls a parameter for the Gaussian Blur that controls the distance at which it should smooth with the maximum value
* Depth Threshold: Controls a parameter for the Gaussian Blur that controls at which depth difference the smoothing needs to be stopped
* Deactivate/Activate Shadows: Deactivates/Activates the Shadow calculations
* Use Approximation/Normal: Uses an approximation for the shadow calculation, which enhances the rendering speed but decreases quality of the shadow slightly, or use the normal approach which doesn't use an approximation
* Deactivate/Activate Reprojection: Deactivates/Activates the temporal reprojection which makes sure that the correct previous shadow value is sampled
* Deactivate/Activate Smoothing: Deactivates/Activates the screen space Gaussian Blur
* Embree Trace: Create a comparison image with shadows of the current view using embree. The result is stored as EmbreeResult0.png in the execution folder. Note the computation may take a little while.
* Shadow Calculation: The time needed to trace the shadow rays in the current frame
* Gaussian Smoothing: The time needed for the screen-space Gaussian Blur
* Render Time: The complete time needed to render the scene

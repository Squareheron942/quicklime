# quicklime <img src="https://github.com/Squareheron942/quicklime/assets/82814680/7ebc38b2-f0a1-424c-8a5c-a55221204f53" alt="quicklime logo" width="120" height="120" align="right">

## What is it?

quicklime is a 3D game engine for Nintendo 3DS written in C++, with the intention of providing many of the utilities needed to create nearly any game. It uses an entity-component system provided by the entt library in order to allow flexibility of usage, and provides several custom file types including a scene descriptor, binary model format (including material definitions an natived skeletal animation support), and metadata format.

Currently, it is a work in progress, with only the backend being mostly complete, and the front-end in a much lesser state of development. The intention is to provide an easy-to-use graphical interface for development similar to that of other game engines such as Unity and Godot.

## How to build

Dependencies:

citro3D, libctru, ffmpeg, python3, ffmpeg-python, devkitpro 3ds-dev package

Set the python install location in the makefile, then run `make` in the project directory. The first build will error when compiling the c++ code (due to how the makefile is set up, will be fixed in final release when an app is made), and the second will succeed.

## How to use the engine

The makefile contains various settings, you mostly don't need to mess with them other than the python variable which should be set to your python 3 install location


### Scripts
To create a new script, make a new class extending the `Script` class (defined in `script.h`) which is where all the code will go, and place it in your asset folder.
It is very similar to Unity scripts, look at the example scripts to see how to write it. (once those are added)
Some stuff is not yet implemented (FixedUpdate() for example as well as just multithreading in general)

### Materials

#### Material source code
To create a material, copy one of the examples and edit the code as you need.
You can store the data to be passed to the constructor in a .slmtl file, which needs to be placed in the same location as the .slmdl model and said model needs to reference it by name.
Eventually I might make an app for this (currently in the works but not done). Then, you can get back those parameters however you choose, since the model loader will just give you a file pointer to whatever file is specified in the model. 
Again, see the example materials to see how to read a file.

#### Material files
These contain the data to be input to the material code. This will vary depending on the material chosen, which is specified via a null-terminated string at the beginning of the file.

### Models

Models can be imported by adding the collada model to the asset folder, then when building, a script will generate a .slmdl model and .slmtl material. You can then optionally move those models back to the original folder and removing the collada model, and you can edit the material file to select what code is being used. 

Models and their materials MUST be in the same directory, otherwise it won't be found (and the game will crash :D).

### Objects
Objects can be added to the scene by adding them to the scene file. Check the docs to see the syntax of that.

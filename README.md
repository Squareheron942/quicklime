# SL Dev Repo

## WARNING

PLEASE DO NOT EDIT MAIN BRANCH DIRECTLY

GOOGLE HOW TO CREATE BRANCHES AND ONLY EVER COMMIT TO THOSE

BRANCHES ARE FOR WHEN YOU ARE DEVELOPING SOMETHING SPECIFIC, THAT MEANS ONE BRANCH FOR ONE FEATURE

ONLY WHEN YOU ARE DONE THAT FEATURE DO YOU PUSH IT TO THE MAIN BRANCH, AND MAKE SURE IT ACTUALLY WORKS PROPERLY

## How to use the engine

The makefile contains various settings, you don't need to mess with them


### Scripts
To create a new script, make a new header in /include/scripts which is where all the code will go. 
It is very similar to Unity scripts, look at the example scripts to see how to write it.
Some stuff is not yet implemented (FixedUpdate() for example as well as just multithreading in general)

### Materials
To create a material, copy one of the examples and edit the code as you need.
You can store the data to be passed to the constructor in a .slmtl file, which needs to be placed in the same location as the .slmdl model and said model needs to reference it by name.
Eventually I might make an app for this but not yet. Then, you can get back those parameters however you choose, since the model loader will just give you a file pointer to whatever file is specified in the model. 
Again, see the example materials to see how to read a file.

### Objects
All the components and scripts need to be attached to an object manually currently, once the scene system is implemented it will be done via a scene file.
So, not much to say yet here

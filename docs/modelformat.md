# [MODEL NAME] FORMAT

## Binary

Each file represents a group of 3D objects as well as the materials to draw them with

### Sections

#### Header

Total size
Number of objects

#### Data

##### Materials

Specify a vshader
Specify a material class by name, give it the parameters needed through the binary data section

Example:



##### Objects

Number of vertices

struct vertex { float position[3]; float texcoord[2]; float normal[3]; };

Each file is basically just the binary representation of that, repeated for each vertex

So:

bytes 0-3 represent (vertex 0).position[0]
bytes 4-7 represent (vertex 0).position[1]
bytes 8-11 represent (vertex 0).position[2]

bytes 12-15 represent (vertex 0).texcoord[0]
bytes 16-19 represent (vertex 0).texcoord[1]

bytes 20-23 represent (vertex 0).normal[0]
bytes 24-27 represent (vertex 0).normal[1]
bytes 28-31 represent (vertex 0).normal[2]

bytes 32-63 represent vertex 1

etc...

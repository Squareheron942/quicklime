# SL model format

Each file represents a single 3D object as well as the name of the material to dictate how it is drawn.

Models must be centred on (0, 0, 0):

This means if you have for example a cube, it should be positioned so that the centre of the cube is at (0, 0, 0).

This should just be as close as possible, if it's not perfect it likely won't cause issues but the further away it is the weirder it will look with frustum culling. To have an object default as offset from (0, 0, 0), set its position in the scene file instead. The model converter script might do the first part for you (depends if i implement it) but the second part is entirely on the dev to do. If I ever have a UI thing, it should make it pretty obvious that this needs doing

## Sections

### Header

Contains:

- Vertex size
- Number of vertices
- Number of attributes
- Attribute types and sizes
- Radius of bounding sphere (used for frustum culling)
- Extra information on attributes (TBD)
- Number of bones
- Bone array in binary format (specifically formatted as 2 vectors for position and rotation)


#### Materials

Specify a material file by name, which must be in the same directory and be named `<name>.slmtl`

Example:

```
data
└───scene1
│   │   scene1.scene
│   │
│   └───models
│       │   cube.slmdl
│       │   cubemat.slmtl
│       │   ...
│   
└───scene2
    │   scene2.scene
    │   ...
```

`cube.slmdl` will have the name `cubemat` stored in it, which will then search for the file `cubemat.slmtl`.

##### Material files

`.slmdl` files contain the material name and the data to give it. The former is just a null terminated c style string, which maps to one of the existing material classes. 

The rest of the file entirely depends on the material chosen but usually contains binary data like color information as well as texture file names.

##### Textures

Textures, due to a limitation of tex3DS, need to all have unique names, as they must all be stored in one folder with no name conflicts. 

#### Object section

This section is basically just the binary representation of an array of vertices, following the layout given in the header

So:

With a vertex laid out as { position[3], texcoord[2], normal[3]} (32 bytes long)

- bytes 0-3 represent (vertex 0).position[0]
- bytes 4-7 represent (vertex 0).position[1]
- bytes 8-11 represent (vertex 0).position[2]

- bytes 12-15 represent (vertex 0).texcoord[0]
- bytes 16-19 represent (vertex 0).texcoord[1]

- bytes 20-23 represent (vertex 0).normal[0]
- bytes 24-27 represent (vertex 0).normal[1]
- bytes 28-31 represent (vertex 0).normal[2]

- bytes 32-63 represent vertex 1

etc...

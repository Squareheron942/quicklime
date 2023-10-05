# [MODEL NAME] FORMAT

## Binary

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

## Text

line 1: number of vertices
line 2: number of objects/vertex groups (n)
lines 3-(2n+3): number of vertices on one line, material file on the other line
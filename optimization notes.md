# Optimization testing notes

All of these tests were done with the orginal plaza model (the 171k poly one) using the subset of textures that I had.

- Texture res when in VRAM barely matters, between everything being 8x8 or 1024x1024 is around +3-5ms
- However, textures being in FCRAM is very detrimental to performance (+10ms). Always put them in VRAM if possible
- Vertex shader calculations for lighting barely matter, disabling them didn't even change GPU time by 1ms

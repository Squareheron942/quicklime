# Shaders

Ok writing down my thoughts on how to do this

Remove shader subclasses
Instead, set unif with script
Shader file becomes a binary file which is read by shader main class, includes all the settings in shader file
make shader metadata file from shader file

struct for shader would look something like this

```
{
tevsetting[5]
alpha test
alpha test func
...
}
```

Have both default (to access engine state) uniforms and user-defined ones

Examples of default uniforms:
QL_MATRIX_MVP	Current model * view * projection matrix.
QL_MATRIX_MV	Current model * view matrix.
QL_MATRIX_V	Current view matrix.
QL_MATRIX_P	Current projection matrix.
QL_MATRIX_VP	Current view * projection matrix.
ql_ObjectToWorld	Current model matrix.
ql_WorldToObject    Inverse of current model matrix
\_Time	float4	Time since level load (t, sin(t), cos(t), dt), use to animate things inside the shaders.
\_ScreenParams	float4	x is the width of the camera’s target texture in pixels
, y is the height of the camera’s target texture in pixels, z is 1.0 + 1.0/width and w is 1.0 + 1.0/height.

Object drawing order priority
1. shader
2. queue
3. depth
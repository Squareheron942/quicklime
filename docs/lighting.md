## Why is there a doc on this?

Well basically lights are goofy on 3DS:

The diffuse and ambient lighting colors are determined by a data member that also includes the emmission and specular colors (which are specific to each object).


Plus, you can select the light color itself, which also affects the colors


Generally, don't mess with the colors too much (leave them as some shade of grey) because the light still affects everything (if you have a red light and an object where diffuse = {0.6, 0.6, 0.6} it will still appear lit red)

You'll want to mess with the light colors instead, since it affects everything in the scene
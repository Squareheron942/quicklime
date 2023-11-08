General layout:

```
/* scene1.scene */
cube[
    scripts[
        script1, 
        script2
    ],
    components[
        transform{`data`},
        mesh{`data`}
    ]
]
```

Normal sections:

Object is on base level (no brackets around it)
Name maps to a corresponding class
Use square brackets to specify scripts and components
Spaces, newlines and tabs will be ignored

Data sections:

Basically just acts as the parameters for constructing an object, always formatted as: 
- left curly bracket
- 32 bit size specifier
- data
- right curly bracket

Use a hex editor to write this part out
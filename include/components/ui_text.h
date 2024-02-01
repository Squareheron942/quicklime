#pragma once
#include "citro2d.h"
#include "citro3d.h"

class GameObject;
class transform;

class Text {
    protected: 
    C2D_Font font;
    C2D_Text txt;
    C2D_TextBuf textBuf;
    char* text;
    transform* trans;
    int textlen;
    GameObject* parent;

    public:
    Text(GameObject& parent, const void* args);
    void Render();
    ~Text();
};
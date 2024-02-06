#include "ui_text.h"
#include "citro2d.h"
#include "citro3d.h"
#include "componentmanager.h"
#include "gameobject.h"
#include "transform.h"
#include "renderertypes.h"

namespace {
    struct text_params{
        // float x = 0;
        // float y = 0;
        // float z = 0;
        // float scaleX = 1;
        // float scaleY = 1;
    };
};



Text::Text(GameObject& parent, const void* args) {
    // text_params formatted_args;
    if (args) {
        // formatted_args = *(text_params*)args;
        textlen = strlen((char*)args);
        text = new char[textlen];
        memcpy(text, args, strlen((char*)args)); // copy the string data into the 
    }
    font = C2D_FontLoad("romfs:/assets/Splat.bcfnt");
    textBuf = C2D_TextBufNew(textlen);
    C2D_TextFontParse(&txt, font, textBuf, text);
    C2D_TextOptimize(&txt);
    this->parent = &parent;
    this->parent->renderer |= RENDERER_TEXT;
}

Text::~Text(){
    C2D_FontFree(font);
    C2D_TextBufDelete(textBuf);
    parent->renderer &= ~RENDERER_TEXT;
    delete text;
}

void Text::Render() {
    C2D_Prepare();
    // C2D_DrawText(&txt, 0, , , , trans->scale.x, trans->scale.y);
    // C2D_DrawText(&txt, 0, trans->position.x, trans->position.y, trans->position.z, 1, 1);
    C2D_Flush();
}

COMPONENT_REGISTER(Text)
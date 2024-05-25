#include "ui_text.h"
#include "componentmanager.h"
#include "gameobject.h"
#include "transform.h"
#include "renderertypes.h"
#include "string.h"
#include "memory.h"

namespace {
    struct text_params {
        float x = 0;
        float y = 0;
        float z = 0;
        float scaleX = 1;
        float scaleY = 1;
    };
};



Text::Text(GameObject& parent, const void* args) {
    // text_params formatted_args;
    if (args) {
        // formatted_args = *(text_params*)args;
        textlen = strlen((char*)args) + 1;
        text = new char[textlen];
        memcpy(text, args, textlen); // copy the string data into the array
    }
    font = C2D_FontLoad("romfs:/assets/Splat.bcfnt"); //TODO add system to prevent duplicate fonts (RAM usage)
    textBuf = C2D_TextBufNew(textlen);
    C2D_TextFontParse(&txt, font, textBuf, text);
    C2D_TextOptimize(&txt);
    this->parent = &parent;
    this->parent->renderer |= RENDERER_TEXT;
    Console::log("Text constructor");
}

Text::Text(Text& t): font(t.font), txt(t.txt), textBuf(t.textBuf), text(t.text), trans(t.trans), textlen(t.textlen), parent(t.parent) {
	Console::log("Text copy constructor");
}

Text::~Text(){
	Console::log("ui text destructor");
    C2D_FontFree(font);
    C2D_TextBufDelete(textBuf);
    parent->renderer &= ~RENDERER_TEXT; // remove text rendering flag
    delete text;
}

void Text::Render() {
	trans = parent->getComponent<transform>();
    C2D_Prepare();
    if (trans) C2D_DrawText(&txt, 0, trans->position.x, trans->position.y, trans->position.z, trans->scale.x, trans->scale.y);
    C2D_Flush();
}

COMPONENT_REGISTER(Text)
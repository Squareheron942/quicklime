#pragma once

#ifndef BIT
#define BIT(n) 1 << n
#endif

enum renderertypes {
    RENDERER_MESH = BIT(1),
    RENDERER_TEXT = BIT(2)
};
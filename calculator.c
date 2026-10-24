#include "apps.h"


void Calculator_Draw(Rectangle area)
{
    DrawRectangleRec(area, RAYWHITE);
    DrawText("Calculator - coming soon", (int)area.x + 20, (int)area.y + 20, 20, DARKGRAY);
}

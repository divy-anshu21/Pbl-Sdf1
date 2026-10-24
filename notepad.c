#include "apps.h"


void Notepad_Draw(Rectangle area)
{
    DrawRectangleRec(area, RAYWHITE);
    DrawText("Notepad - coming soon", (int)area.x + 20, (int)area.y + 20, 20, DARKGRAY);
}

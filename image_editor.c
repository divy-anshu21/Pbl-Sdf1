#include "apps.h"


void ImageEditor_Draw(Rectangle area)
{
    DrawRectangleRec(area, RAYWHITE);
    DrawText("Image Editor - coming soon", (int)area.x + 20, (int)area.y + 20, 20, DARKGRAY);
}

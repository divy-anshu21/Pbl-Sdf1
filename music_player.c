#include "apps.h"


void MusicPlayer_Draw(Rectangle area)
{
    DrawRectangleRec(area, RAYWHITE);
    DrawText("Music Player - coming soon", (int)area.x + 20, (int)area.y + 20, 20, DARKGRAY);
}

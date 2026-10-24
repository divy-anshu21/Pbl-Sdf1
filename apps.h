#ifndef APPS_H
#define APPS_H

#include "raylib.h"


typedef enum {
    APP_CALCULATOR = 0,
    APP_IMAGE_EDITOR,
    APP_MUSIC_PLAYER,
    APP_NOTEPAD,
    APP_COUNT
} AppId;


void Calculator_Draw(Rectangle area);
void ImageEditor_Draw(Rectangle area);
void MusicPlayer_Draw(Rectangle area);
void Notepad_Draw(Rectangle area);

#endif
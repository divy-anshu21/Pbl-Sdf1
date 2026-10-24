#include <stdio.h>
#include <time.h>
#include "raylib.h"
#include "apps.h"
#define SCREEN_W     1280
#define SCREEN_H     720
#define TASKBAR_H    44
#define ICON_SIZE    64      
#define CELL_W       110     
#define CELL_H       100
#define LABEL_FONT   18
#define FONT_ATLAS_SIZE 32
static Font labelFont = { 0 };
static bool labelFontLoaded = false;
typedef struct {
    const char *name;         
    const char *file;         
    Color       fallback;     
    AppId       app;          
    Texture2D   texture;
    bool        hasTexture;
    Rectangle   bounds;       
} DesktopIcon;

static DesktopIcon icons[APP_COUNT] = {
    { "Calculator",   "assets/calculator.png",   { 150, 150, 160, 255 }, APP_CALCULATOR   },
    { "Image Editor", "assets/image_editor.png", { 230, 140,  60, 255 }, APP_IMAGE_EDITOR },
    { "Music Player", "assets/music_player.png", {  60, 120, 220, 255 }, APP_MUSIC_PLAYER },
    { "Notepad",      "assets/notepad.png",      { 120, 190, 220, 255 }, APP_NOTEPAD      },
};

 
static void DrawOutlinedText(const char *text, Vector2 position,
                             float fontSize, float spacing, Color color)
{
    static const Vector2 outlineOffsets[] = {
        { -1, -1 }, { 0, -1 }, { 1, -1 },
        { -1,  0 },             { 1,  0 },
        { -1,  1 }, { 0,  1 }, { 1,  1 }
    };
    const Color outline = { 0, 0, 0, 100 };

    if (labelFontLoaded) {
        for (int i = 0; i < 8; i++) {
            Vector2 outlinePosition = {
                position.x + outlineOffsets[i].x,
                position.y + outlineOffsets[i].y
            };
            DrawTextEx(labelFont, text, outlinePosition, fontSize, spacing, outline);
        }
        DrawTextEx(labelFont, text, position, fontSize, spacing, color);
    } else {
        for (int i = 0; i < 8; i++) {
            DrawText(text,
                     (int)(position.x + outlineOffsets[i].x),
                     (int)(position.y + outlineOffsets[i].y),
                     (int)fontSize, outline);
        }
        DrawText(text, (int)position.x, (int)position.y, (int)fontSize, color);
    }
}

 


static Texture2D LoadIfExists(const char *path, bool *found)
{
    Texture2D t = { 0 };
    *found = FileExists(path);
    if (*found) {
        t = LoadTexture(path);
        GenTextureMipmaps(&t);
        SetTextureFilter(t, TEXTURE_FILTER_TRILINEAR);
    }
    return t;
}

static void LoadIcons(void)
{
    for (int i = 0; i < APP_COUNT; i++) {
        icons[i].texture = LoadIfExists(icons[i].file, &icons[i].hasTexture);

         
        icons[i].bounds = (Rectangle){ 10, 20 + i * (CELL_H + 20), CELL_W, CELL_H };
    }
}

static void UnloadIcons(void)
{
    for (int i = 0; i < APP_COUNT; i++)
        if (icons[i].hasTexture) UnloadTexture(icons[i].texture);
}

 


static void DrawWallpaper(Texture2D wallpaper, bool hasWallpaper)
{
    if (hasWallpaper) {
         
        Rectangle src = { 0, 0, (float)wallpaper.width, (float)wallpaper.height };
        Rectangle dst = { 0, 0, SCREEN_W, SCREEN_H };
        DrawTexturePro(wallpaper, src, dst, (Vector2){ 0, 0 }, 0.0f, WHITE);
        return;
    }

     
    DrawRectangleGradientV(0, 0, SCREEN_W, SCREEN_H,
    (Color){ 40, 110, 230, 255 }, (Color){ 170, 210, 250, 255 });
    DrawEllipse(300, 220, 140, 40, (Color){ 255, 255, 255, 200 });
    DrawEllipse(900, 160, 200, 50, (Color){ 255, 255, 255, 200 });
    DrawEllipse(1050, 330, 150, 40, (Color){ 255, 255, 255, 200 });
    DrawEllipse(450, SCREEN_H + 120, 1100, 330, (Color){ 70, 160, 40, 255 });
    DrawEllipse(900, SCREEN_H + 160, 800, 280, (Color){ 55, 140, 30, 255 });
}

 


static void DrawIcon(const DesktopIcon *ic)
{
     
    Rectangle img = {
        ic->bounds.x + (ic->bounds.width - ICON_SIZE) / 2,
        ic->bounds.y,
        ICON_SIZE, ICON_SIZE
    };
    if (ic->hasTexture) {
        Rectangle src = { 0, 0, (float)ic->texture.width, (float)ic->texture.height };
        DrawTexturePro(ic->texture, src, img, (Vector2){ 0, 0 }, 0.0f, WHITE);
    } else {
         
        DrawRectangleRounded(img, 0.2f, 8, ic->fallback);
        DrawRectangleLinesEx(img, 2, (Color){ 255, 255, 255, 180 });
        char letter[2] = { ic->name[0], '\0' };
        float letterWidth = labelFontLoaded
            ? MeasureTextEx(labelFont, letter, 32, 1.0f).x
            : (float)MeasureText(letter, 32);
        Vector2 letterPosition = {
            img.x + (img.width - letterWidth) / 2,
            img.y + 16
        };
        DrawOutlinedText(letter, letterPosition, 32, 1.0f, WHITE);
    }

     
    float ty = ic->bounds.y + ICON_SIZE + 8;
    if (labelFontLoaded) {
        Vector2 size = MeasureTextEx(labelFont, ic->name, LABEL_FONT, 1.0f);
        float tx = ic->bounds.x + (ic->bounds.width - size.x) / 2;
        DrawOutlinedText(ic->name, (Vector2){ tx, ty }, LABEL_FONT, 1.0f, WHITE);
    } else {
        int textWidth = MeasureText(ic->name, LABEL_FONT);
        int tx = (int)(ic->bounds.x + (ic->bounds.width - textWidth) / 2);
        DrawOutlinedText(ic->name, (Vector2){ (float)tx, ty },
                         LABEL_FONT, 1.0f, WHITE);
    }
}

 


static void DrawTaskbar(Texture2D startButton, bool hasStartButton)
{
    int y = SCREEN_H - TASKBAR_H;

    DrawRectangle(0, y, SCREEN_W, TASKBAR_H, (Color){ 22, 32, 52, 255 });
    DrawRectangle(0, y, SCREEN_W, 2, (Color){ 70, 100, 150, 255 });

     
    if (hasStartButton) {
        Rectangle src = { 0, 0, (float)startButton.width, (float)startButton.height };
        Rectangle dst = { 11, y + 3, 38, 38 };
        DrawTexturePro(startButton, src, dst, (Vector2){ 0, 0 }, 0.0f, WHITE);
    } else {
        DrawCircle(30, y + TASKBAR_H / 2 + 1, 17, (Color){ 20, 90, 30, 255 });
        DrawCircle(30, y + TASKBAR_H / 2 + 1, 14, (Color){ 60, 180, 70, 255 });
    }

     
    time_t now = time(NULL);
    struct tm *lt = localtime(&now);
    char clock[16];
    strftime(clock, sizeof(clock), "%H:%M", lt);

    const float clockFontSize = 22.0f;
    int boxW;
    float clockY;
    Vector2 clockSize = { 0 };
    if (labelFontLoaded) {
        clockSize = MeasureTextEx(labelFont, clock, clockFontSize, 1.0f);
        boxW = (int)clockSize.x + 40;
        clockY = y + (TASKBAR_H - clockSize.y) / 2.0f;
    } else {
        boxW = MeasureText(clock, (int)clockFontSize) + 40;
        clockY = y + 11;
    }
    DrawRectangle(SCREEN_W - boxW, y + 2, boxW, TASKBAR_H - 2, (Color){ 15, 22, 38, 255 });
    Vector2 clockPosition = {
        (float)(SCREEN_W - boxW + 20),
        (float)((int)clockY)
    };
    if (labelFontLoaded) {
        DrawTextEx(labelFont, clock, clockPosition, clockFontSize, 1.0f, WHITE);
    } else {
        DrawText(clock, (int)clockPosition.x, (int)clockPosition.y,
                 (int)clockFontSize, WHITE);
    }
}

static Rectangle AppWindowBounds(void)
{
    return (Rectangle){ 100, 50, 1080, SCREEN_H - TASKBAR_H - 80 };
}

static Rectangle AppCloseButtonBounds(void)
{
    Rectangle window = AppWindowBounds();
    return (Rectangle){ window.x + window.width - 38, window.y + 3, 32, 26 };
}

static void DrawAppScreen(AppId app)
{
    Rectangle window = AppWindowBounds();
    Rectangle titleBar = { window.x, window.y, window.width, 32 };
    Rectangle content = {
        window.x + 2, window.y + 34, window.width - 4, window.height - 36
    };
    Rectangle closeButton = AppCloseButtonBounds();
    bool closeHovered = CheckCollisionPointRec(GetMousePosition(), closeButton);

    DrawRectangleRec(window, (Color){ 25, 42, 68, 255 });
    DrawRectangleRec(titleBar, (Color){ 35, 75, 125, 255 });
    DrawOutlinedText(icons[app].name, (Vector2){ window.x + 12, window.y + 7 },
                     18, 1.0f, WHITE);
    DrawRectangleRec(closeButton, closeHovered ? RED : (Color){ 90, 105, 125, 255 });
    DrawOutlinedText("X", (Vector2){ closeButton.x + 11, closeButton.y + 3 },
                     18, 1.0f, WHITE);

    switch (app) {
        case APP_CALCULATOR:   Calculator_Draw(content); break;
        case APP_IMAGE_EDITOR: ImageEditor_Draw(content); break;
        case APP_MUSIC_PLAYER: MusicPlayer_Draw(content); break;
        case APP_NOTEPAD:      Notepad_Draw(content); break;
        default: break;
    }
}

 


int main(void)
{
    InitWindow(SCREEN_W, SCREEN_H, "Mini Desktop OS Simulation in C (Raylib)");
    SetTargetFPS(60);

     
    ChangeDirectory(GetApplicationDirectory());

    if (FileExists("assets/windows_vista.png")) {
        Image windowIcon = LoadImage("assets/windows_vista.png");
        if (windowIcon.data != NULL) {
            SetWindowIcon(windowIcon);
            UnloadImage(windowIcon);
        }
    }

     
    labelFont = LoadFontEx("assets/SEGUISB.TTF", FONT_ATLAS_SIZE, NULL, 0);
    labelFontLoaded = (labelFont.texture.id != 0);

    bool hasWallpaper;
    Texture2D wallpaper = LoadIfExists("assets/wallpaper.png", &hasWallpaper);
    bool hasStartButton;
    Texture2D startButton = LoadIfExists("assets/windows_vista.png", &hasStartButton);
    LoadIcons();

    bool appOpen = false;
    bool context_menu_open = false;
    AppId activeApp = APP_CALCULATOR;
    Rectangle box;

    while (!WindowShouldClose())
    {
         


        if (appOpen) {
            if (IsKeyPressed(KEY_ESCAPE)) appOpen = false;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
                CheckCollisionPointRec(GetMousePosition(), AppCloseButtonBounds())) {
                appOpen = false;
            }
        } else if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            context_menu_open= false;
            Vector2 mouse = GetMousePosition();
            for (int i = 0; i < APP_COUNT; i++) {
                if (CheckCollisionPointRec(mouse, icons[i].bounds)) {
                    activeApp = icons[i].app;
                    appOpen = true;
                    break;
                }
            }
        }
        else if(IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)){
            Vector2 mouse_right = GetMousePosition();
            for(int i=0;i< APP_COUNT;i++)
            {
                if(CheckCollisionPointRec(mouse_right , icons[i].bounds)){
                    box.x = mouse_right.x;
                    box.y = mouse_right.y;
                    box.width = 150;
                    box.height = 107;
                    context_menu_open = true;
                    activeApp = icons[i].app;
                }
            } 
        }

         
        BeginDrawing();
            ClearBackground(BLACK);
            DrawWallpaper(wallpaper, hasWallpaper);
            if (appOpen) {
                DrawAppScreen(activeApp);
            } else {
                for (int i = 0; i < APP_COUNT; i++) DrawIcon(&icons[i]);
            }
            if(context_menu_open){
                DrawRectangleRec(box, GRAY);
                DrawOutlinedText("Open",(Vector2){box.x+8,box.y+8},25,1.0F,WHITE);
                DrawOutlinedText("Rename",(Vector2){box.x+8,box.y+(8*2)+25},25,1.0F,WHITE);
                DrawOutlinedText("Delete",(Vector2){box.x+8,box.y+8+25+8+25+8},25,1.0F,WHITE);
                
            }
            DrawTaskbar(startButton, hasStartButton);
        EndDrawing();
    }

    UnloadIcons();
    if (hasWallpaper) UnloadTexture(wallpaper);
    if (hasStartButton) UnloadTexture(startButton);
    if (labelFontLoaded) UnloadFont(labelFont);
    CloseWindow();
    return 0;
}

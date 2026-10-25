#include <stdio.h>
#include <time.h>
#include <string.h>
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
Rectangle taskbar_bounds = {0 ,SCREEN_H-TASKBAR_H,SCREEN_W,TASKBAR_H};
static Font labelFont = { 0 };
static bool labelFontLoaded = false;
typedef struct {
    char name[32];         
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

static const char *const initialIconNames[APP_COUNT] = {
    "Calculator", "Image Editor", "Music Player", "Notepad"
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

static void DrawMenuPanel(Rectangle bounds)
{
    Rectangle shadow = { bounds.x + 3, bounds.y + 4, bounds.width, bounds.height };
    DrawRectangleRounded(shadow, 0.12f, 8, (Color){ 0, 0, 0, 105 });
    DrawRectangleRounded(bounds, 0.12f, 8, (Color){ 29, 35, 47, 250 });
    DrawRectangleLinesEx(bounds, 1.0f, (Color){ 83, 96, 116, 255 });
}

static void DrawMenuItem(Rectangle bounds, const char *label, bool destructive)
{
    bool hovered = CheckCollisionPointRec(GetMousePosition(), bounds);
    if (hovered) {
        Color hoverColor = destructive
            ? (Color){ 103, 53, 59, 255 }
            : (Color){ 52, 78, 111, 255 };
        DrawRectangleRounded(bounds, 0.22f, 8, hoverColor);
    }

    Color textColor = destructive && !hovered
        ? (Color){ 242, 137, 137, 255 }
        : (Color){ 238, 242, 248, 255 };
    DrawOutlinedText(label, (Vector2){ bounds.x + 12, bounds.y + 7 },
                     20, 1.0f, textColor);
}

static void appRename(char *buffer, int capacity)
{
    int character;
    while ((character = GetCharPressed()) > 0) {
        int length = (int)strlen(buffer);
        if (character >= 32 && character <= 126 && length < capacity - 1) {
            buffer[length] = (char)character;
            buffer[length + 1] = '\0';
        }
    }

    if (IsKeyPressed(KEY_BACKSPACE)) {
        int length = (int)strlen(buffer);
        if (length > 0) buffer[length - 1] = '\0';
    }

    Rectangle panel = { (SCREEN_W - 340) / 2, (SCREEN_H - 140) / 2, 340, 140 };
    Rectangle input = { panel.x + 20, panel.y + 52, panel.width - 40, 34 };
    DrawRectangleRec(panel, (Color){ 35, 45, 65, 255 });
    DrawRectangleLinesEx(panel, 2, WHITE);
    DrawText("Enter a new name:", (int)panel.x + 20, (int)panel.y + 18, 20, WHITE);
    DrawRectangleRec(input, RAYWHITE);
    DrawRectangleLinesEx(input, 1, DARKGRAY);
    DrawText(buffer, (int)input.x + 8, (int)input.y + 8, 18, BLACK);
    if ((int)(GetTime() * 2.0) % 2 == 0) {
        int cursorX = (int)input.x + 8 + MeasureText(buffer, 18);
        int cursorMaxX = (int)(input.x + input.width - 8);
        if (cursorX > cursorMaxX) cursorX = cursorMaxX;
        DrawLine(cursorX, (int)input.y + 7, cursorX, (int)input.y + 27, BLACK);
    }
    DrawText("Enter: save    Esc: cancel", (int)panel.x + 20,
             (int)panel.y + 104, 16, LIGHTGRAY);
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
    char date[16];
    strftime(clock, sizeof(clock), "%I:%M %p", lt);
    strftime(date, sizeof(date), "%d-%m-%Y", lt);

    const float clockFontSize = 19.0f;
    const float dateFontSize = 13.0f;
    Vector2 clockSize = { 0 };
    Vector2 dateSize = { 0 };
    if (labelFontLoaded) {
        clockSize = MeasureTextEx(labelFont, clock, clockFontSize, 1.0f);
        dateSize = MeasureTextEx(labelFont, date, dateFontSize, 1.0f);
    } else {
        clockSize = (Vector2){ (float)MeasureText(clock, (int)clockFontSize), clockFontSize };
        dateSize = (Vector2){ (float)MeasureText(date, (int)dateFontSize), dateFontSize };
    }
    float textWidth = clockSize.x > dateSize.x ? clockSize.x : dateSize.x;
    int boxW = (int)textWidth + 28;
    DrawRectangle(SCREEN_W - boxW, y + 2, boxW, TASKBAR_H - 2, (Color){ 15, 22, 38, 255 });
    Vector2 clockPosition = {
        SCREEN_W - boxW + (boxW - clockSize.x) / 2.0f,
        (float)y + 3
    };
    Vector2 datePosition = {
        SCREEN_W - boxW + (boxW - dateSize.x) / 2.0f,
        (float)y + 24
    };
    if (labelFontLoaded) {
        DrawTextEx(labelFont, clock, clockPosition, clockFontSize, 1.0f, WHITE);
        DrawTextEx(labelFont, date, datePosition, dateFontSize, 1.0f,
                   (Color){ 190, 202, 220, 255 });
    } else {
        DrawText(clock, (int)clockPosition.x, (int)clockPosition.y,
                 (int)clockFontSize, WHITE);
        DrawText(date, (int)datePosition.x, (int)datePosition.y,
                 (int)dateFontSize, (Color){ 190, 202, 220, 255 });
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
    SetExitKey(KEY_NULL);
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
    bool renaming = false;
    int selectedIcon = -1;
    char renameBuffer[32] = "";
    AppId activeApp = APP_CALCULATOR;
    Rectangle box;
    Rectangle refresh;
    Rectangle openBounds;
    Rectangle renameBounds;
    Rectangle deleteBounds ;
    bool refresh_bar = false;
    bool iconDeleted[APP_COUNT] = {false};


    while (!WindowShouldClose())
    {
        if (renaming) {
            if (IsKeyPressed(KEY_ENTER)) {
                if (selectedIcon >= 0) strcpy(icons[selectedIcon].name, renameBuffer);
                renaming = false;
            } else if (IsKeyPressed(KEY_ESCAPE)) {
                renaming = false;
            }
        } else if (appOpen) {
            if (IsKeyPressed(KEY_ESCAPE)) appOpen = false;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
                CheckCollisionPointRec(GetMousePosition(), AppCloseButtonBounds())) {
                appOpen = false;
            }
        } else if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mouse = GetMousePosition();
            if (refresh_bar && CheckCollisionPointRec(mouse, refresh)) {
                for (int i = 0; i < APP_COUNT; i++) {
                    strcpy(icons[i].name, initialIconNames[i]);
                    iconDeleted[i] = false;
                }

                appOpen = false;
                context_menu_open = false;
                renaming = false;
                selectedIcon = -1;
                activeApp = APP_CALCULATOR;
                renameBuffer[0] = '\0';
                refresh_bar = false;
            } else {
                refresh_bar = false;
                if (context_menu_open && CheckCollisionPointRec(mouse, openBounds)) {
                    if (selectedIcon >= 0 && !iconDeleted[selectedIcon]) {
                        activeApp = icons[selectedIcon].app;
                        appOpen = true;
                    }
                    context_menu_open = false;
                } else if (context_menu_open && CheckCollisionPointRec(mouse, renameBounds)) {
                    if (selectedIcon >= 0 && !iconDeleted[selectedIcon]) {
                        strcpy(renameBuffer, icons[selectedIcon].name);
                        renaming = true;
                    }
                    context_menu_open = false;
                } else if (context_menu_open && CheckCollisionPointRec(mouse, deleteBounds)) {
                    if (selectedIcon >= 0) iconDeleted[selectedIcon] = true;
                    context_menu_open = false;
                } else {
                    context_menu_open = false;
                    for (int i = 0; i < APP_COUNT; i++) {
                        if (CheckCollisionPointRec(mouse, icons[i].bounds) && !iconDeleted[i]) {
                            activeApp = icons[i].app;
                            appOpen = true;
                            break;
                        }
                    }
                }
            }
        }
        else if(IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)){
            Vector2 mouse_right = GetMousePosition();
            int clickedIcon = -1;

            for (int i = 0; i < APP_COUNT; i++) {
                if (!iconDeleted[i] &&
                    CheckCollisionPointRec(mouse_right, icons[i].bounds)) {
                    clickedIcon = i;
                    break;
                }
            }

            if (clickedIcon >= 0) {
                box = (Rectangle){ mouse_right.x, mouse_right.y, 190, 124 };
                if (box.x + box.width > SCREEN_W) box.x = SCREEN_W - box.width;
                if (box.y + box.height > SCREEN_H - TASKBAR_H)
                    box.y = SCREEN_H - TASKBAR_H - box.height;
                context_menu_open = true;
                refresh_bar = false;
                activeApp = icons[clickedIcon].app;
                selectedIcon = clickedIcon;
            } else if (!CheckCollisionPointRec(mouse_right, taskbar_bounds)) {
                refresh = (Rectangle){ mouse_right.x, mouse_right.y, 190, 46 };
                if (refresh.x + refresh.width > SCREEN_W)
                    refresh.x = SCREEN_W - refresh.width;
                if (refresh.y + refresh.height > SCREEN_H - TASKBAR_H)
                    refresh.y = SCREEN_H - TASKBAR_H - refresh.height;
                refresh_bar = true;
                context_menu_open = false;
            } else {
                context_menu_open = false;
                refresh_bar = false;
            }
        }

         
        BeginDrawing();
            ClearBackground(BLACK);
            DrawWallpaper(wallpaper, hasWallpaper);
            if (appOpen) {
                DrawAppScreen(activeApp);
            } else {
                for (int i = 0; i < APP_COUNT; i++) {if(!iconDeleted[i])DrawIcon(&icons[i]);}
            }
            if(context_menu_open){
                DrawMenuPanel(box);
                openBounds   = (Rectangle){ box.x + 5, box.y + 5, box.width - 10, 36 };
                renameBounds = (Rectangle){ box.x + 5, box.y + 44, box.width - 10, 36 };
                deleteBounds = (Rectangle){ box.x + 5, box.y + 83, box.width - 10, 36 };
                DrawMenuItem(openBounds, "Open", false);
                DrawMenuItem(renameBounds, "Rename", false);
                DrawMenuItem(deleteBounds, "Delete", true);
                DrawLine(box.x + 12, box.y + 42, box.x + box.width - 12,
                         box.y + 42, (Color){ 70, 79, 94, 255 });
                DrawLine(box.x + 12, box.y + 81, box.x + box.width - 12,
                         box.y + 81, (Color){ 70, 79, 94, 255 });
            }
            if(refresh_bar)
            {
               DrawMenuPanel(refresh);
               Rectangle refreshItem = {
                   refresh.x + 5, refresh.y + 5, refresh.width - 10, refresh.height - 10
               };
               DrawMenuItem(refreshItem, "Refresh", false);

            }
            if (renaming) appRename(renameBuffer, sizeof(renameBuffer));
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

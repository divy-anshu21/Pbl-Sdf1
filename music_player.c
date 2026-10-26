#include "apps.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Music song = { 0 };
static FilePathList songFiles = { 0 };
static Font playerFont = { 0 };
static bool initializationAttempted = false;
static bool audioDeviceReady = false;
static bool songLoaded = false;
static bool playerFontLoaded = false;
static bool songPaused = false;
static bool songStopped = true;
static int selectedSong = -1;
static float songListScroll = 0.0f;
static const char *statusMessage = "Choose a song from the list.";

static void DrawPlayerText(const char *text, int x, int y,
                           float fontSize, Color color)
{
    if (playerFontLoaded) {
        DrawTextEx(playerFont, text, (Vector2){ (float)x, (float)y },
                   fontSize, 1.0f, color);
    } else {
        DrawText(text, x, y, (int)fontSize, color);
    }
}

static float MeasurePlayerText(const char *text, float fontSize)
{
    if (playerFontLoaded) {
        return MeasureTextEx(playerFont, text, fontSize, 1.0f).x;
    }

    return (float)MeasureText(text, (int)fontSize);
}

static void MusicPlayer_Cleanup(void)
{
    if (songLoaded) {
        StopMusicStream(song);
        UnloadMusicStream(song);
        songLoaded = false;
    }

    if (songFiles.paths != NULL) {
        UnloadDirectoryFiles(songFiles);
        songFiles = (FilePathList){ 0 };
    }

    if (audioDeviceReady) {
        CloseAudioDevice();
        audioDeviceReady = false;
    }

    if (playerFontLoaded && IsWindowReady()) {
        UnloadFont(playerFont);
    }
    playerFontLoaded = false;
}

static void MusicPlayer_Initialize(void)
{
    if (initializationAttempted) return;
    initializationAttempted = true;
    atexit(MusicPlayer_Cleanup);

    playerFont = LoadFontEx("assets/SEGUISB.TTF", 32, NULL, 0);
    playerFontLoaded = playerFont.texture.id != 0;

    songFiles = LoadDirectoryFilesEx("assets/music", ".mp3", false);

    if (songFiles.count == 0) {
        statusMessage = "No MP3 files found in assets/music.";
        return;
    }

    InitAudioDevice();

    if (!IsAudioDeviceReady()) {
        statusMessage = "Couldn't start the audio device.";
        return;
    }

    audioDeviceReady = true;
    statusMessage = "Choose a song from the list.";
}

static void LoadSelectedSong(int index)
{
    if (index < 0 || (unsigned int)index >= songFiles.count) return;

    if (songLoaded) {
        StopMusicStream(song);
        UnloadMusicStream(song);
        songLoaded = false;
    }

    selectedSong = index;
    songPaused = false;
    songStopped = true;

    if (!audioDeviceReady) {
        statusMessage = "Audio is unavailable.";
        return;
    }

    song = LoadMusicStream(songFiles.paths[index]);

    if (!IsMusicValid(song)) {
        statusMessage = "Couldn't load the selected MP3.";
        return;
    }

    songLoaded = true;
    statusMessage = "Ready to play.";
}

static bool DrawPlayerButton(Rectangle bounds, const char *label)
{
    bool hovered = CheckCollisionPointRec(GetMousePosition(), bounds);

    Color buttonColor = hovered
        ? (Color){ 63, 112, 174, 255 }
        : (Color){ 42, 79, 126, 255 };

    DrawRectangleRounded(bounds, 0.25f, 8, buttonColor);
    DrawRectangleRoundedLinesEx(bounds, 0.25f, 8, 2.0f, WHITE);

    int textWidth = (int)MeasurePlayerText(label, 20);
    int textX = (int)(bounds.x + (bounds.width - textWidth) / 2);
    int textY = (int)(bounds.y + (bounds.height - 20) / 2);

    DrawPlayerText(label, textX, textY, 20, WHITE);

    return hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

static void FormatTime(float seconds, char *buffer, int bufferSize)
{
    if (seconds < 0) seconds = 0;

    int wholeSeconds = (int)seconds;
    int minutes = wholeSeconds / 60;
    int remainingSeconds = wholeSeconds % 60;

    snprintf(buffer, bufferSize, "%d:%02d", minutes, remainingSeconds);
}

static void DrawSongName(Rectangle row, const char *name, Color color)
{
    char label[256];
    snprintf(label, sizeof(label), "%s", name);

    int maxWidth = (int)row.width - 28;
    size_t length = strlen(label);

    while (length > 0 && MeasurePlayerText(label, 18) > maxWidth) {
        if (length >= 3) {
            label[length - 3] = '.';
            label[length - 2] = '.';
            label[length - 1] = '.';
            label[length] = '\0';
        } else {
            label[length - 1] = '\0';
        }
        length--;
    }

    DrawPlayerText(label, (int)row.x + 14, (int)row.y + 11, 18, color);
}

static void DrawSongList(Rectangle sidebar, Rectangle listArea)
{
    Vector2 mouse = GetMousePosition();
    float wheel = GetMouseWheelMove();

    if (CheckCollisionPointRec(mouse, listArea)) {
        songListScroll -= wheel * 44.0f;
    }

    float rowHeight = 44.0f;
    float maxScroll = (float)songFiles.count * rowHeight - listArea.height;
    if (maxScroll < 0.0f) maxScroll = 0.0f;
    if (songListScroll < 0.0f) songListScroll = 0.0f;
    if (songListScroll > maxScroll) songListScroll = maxScroll;

    BeginScissorMode((int)listArea.x, (int)listArea.y,
                     (int)listArea.width, (int)listArea.height);

    for (unsigned int i = 0; i < songFiles.count; i++) {
        Rectangle row = {
            listArea.x + 2,
            listArea.y + (float)i * rowHeight - songListScroll,
            listArea.width - 4,
            rowHeight - 4
        };

        if (row.y + row.height < listArea.y || row.y > listArea.y + listArea.height) {
            continue;
        }

        bool hovered = CheckCollisionPointRec(mouse, row);
        bool selected = selectedSong == (int)i;

        if (selected) {
            DrawRectangleRounded(row, 0.18f, 6, (Color){ 49, 103, 160, 255 });
        } else if (hovered) {
            DrawRectangleRounded(row, 0.18f, 6, (Color){ 58, 78, 101, 255 });
        }

        Color textColor = selected ? WHITE : (Color){ 225, 234, 245, 255 };
        DrawSongName(row, GetFileName(songFiles.paths[i]), textColor);

        if (hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            LoadSelectedSong((int)i);
        }
    }

    EndScissorMode();
}

static void DrawNowPlaying(Rectangle panel)
{
    DrawPlayerText("NOW PLAYING", (int)panel.x + 28, (int)panel.y + 22,
                   18, (Color){ 75, 96, 120, 255 });

    if (selectedSong < 0 || (unsigned int)selectedSong >= songFiles.count) {
        DrawPlayerText(statusMessage, (int)panel.x + 28, (int)panel.y + 92,
                       22, DARKGRAY);
        return;
    }

    const char *songName = GetFileName(songFiles.paths[selectedSong]);
    DrawPlayerText(songName, (int)panel.x + 28, (int)panel.y + 78,
                   26, (Color){ 30, 58, 90, 255 });

    if (!songLoaded) {
        DrawPlayerText(statusMessage, (int)panel.x + 28, (int)panel.y + 126,
                       18, MAROON);
        return;
    }

    float songLength = GetMusicTimeLength(song);
    float songPosition = songStopped ? 0.0f : GetMusicTimePlayed(song);

    float progress = 0.0f;
    if (songLength > 0.0f) {
        progress = songPosition / songLength;
        if (progress < 0.0f) progress = 0.0f;
        if (progress > 1.0f) progress = 1.0f;
    }

    Rectangle progressBar = {
        panel.x + 28,
        panel.y + 238,
        panel.width - 56,
        14
    };

    DrawRectangleRounded(progressBar, 0.5f, 8,
                         (Color){ 190, 205, 222, 255 });

    if (progress > 0.0f) {
        Rectangle filledBar = progressBar;
        filledBar.width *= progress;
        DrawRectangleRounded(filledBar, 0.5f, 8,
                             (Color){ 40, 112, 190, 255 });
    }

    char positionText[16];
    char lengthText[16];
    FormatTime(songPosition, positionText, sizeof(positionText));
    FormatTime(songLength, lengthText, sizeof(lengthText));

    DrawPlayerText(positionText, (int)progressBar.x,
                   (int)progressBar.y + 22, 18, DARKGRAY);
    DrawPlayerText(lengthText,
                   (int)(progressBar.x + progressBar.width -
                         MeasurePlayerText(lengthText, 18)),
                   (int)progressBar.y + 22, 18, DARKGRAY);

    Rectangle playButton = {
        panel.x + panel.width / 2 - 120,
        panel.y + 330,
        110,
        48
    };

    Rectangle stopButton = {
        panel.x + panel.width / 2 + 10,
        panel.y + 330,
        110,
        48
    };

    bool currentlyPlaying = IsMusicStreamPlaying(song);
    const char *playLabel = currentlyPlaying
        ? "Pause"
        : (songPaused ? "Resume" : "Play");

    bool playClicked = DrawPlayerButton(playButton, playLabel);
    bool stopClicked = DrawPlayerButton(stopButton, "Stop");

    if (playClicked) {
        if (IsMusicStreamPlaying(song)) {
            PauseMusicStream(song);
            songPaused = true;
        } else if (songPaused) {
            ResumeMusicStream(song);
            songPaused = false;
        } else {
            PlayMusicStream(song);
            songStopped = false;
        }
    }

    if (stopClicked) {
        StopMusicStream(song);
        SeekMusicStream(song, 0.0f);
        songPaused = false;
        songStopped = true;
    }

    const char *playbackStatus = IsMusicStreamPlaying(song)
        ? "Playing"
        : (songPaused ? "Paused" : (songStopped ? "Stopped" : "Ready"));
    Color statusColor = IsMusicStreamPlaying(song) ? DARKGREEN : DARKGRAY;

    DrawPlayerText(playbackStatus, (int)panel.x + 28, (int)panel.y + 416,
                   18, statusColor);
}

void MusicPlayer_Draw(Rectangle area)
{
    MusicPlayer_Initialize();

    if (songLoaded) {
        UpdateMusicStream(song);
    }

    DrawRectangleRec(area, (Color){ 232, 240, 250, 255 });
    DrawRectangleLinesEx(area, 1.0f, (Color){ 150, 170, 195, 255 });
    DrawPlayerText("MUSIC PLAYER", (int)area.x + 24, (int)area.y + 18,
                   26, (Color){ 30, 58, 90, 255 });

    Rectangle sidebar = {
        area.x + 20,
        area.y + 62,
        300,
        area.height - 82
    };

    Rectangle panel = {
        sidebar.x + sidebar.width + 18,
        sidebar.y,
        area.x + area.width - (sidebar.x + sidebar.width + 38),
        sidebar.height
    };

    DrawRectangleRounded(sidebar, 0.04f, 8, (Color){ 35, 48, 65, 255 });
    DrawRectangleRounded(panel, 0.04f, 8, WHITE);
    DrawRectangleRoundedLinesEx(panel, 0.04f, 8, 1.0f,
                                (Color){ 185, 198, 214, 255 });

    DrawPlayerText("SONGS", (int)sidebar.x + 16, (int)sidebar.y + 14,
                   20, WHITE);
    DrawPlayerText("MP3 files in assets/music", (int)sidebar.x + 16,
                   (int)sidebar.y + 40, 14, (Color){ 185, 198, 214, 255 });

    Rectangle listArea = {
        sidebar.x + 8,
        sidebar.y + 68,
        sidebar.width - 16,
        sidebar.height - 78
    };

    if (songFiles.count > 0) {
        DrawSongList(sidebar, listArea);
    } else {
        DrawPlayerText(statusMessage, (int)sidebar.x + 16,
                       (int)sidebar.y + 88, 16, (Color){ 235, 190, 190, 255 });
    }

    DrawNowPlaying(panel);
}

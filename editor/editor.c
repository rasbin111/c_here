/* ============================================================
   editor.c - A minimal code-editor widget built with raylib
   ============================================================

   Features:
     - Multi-line text buffer with dynamic growth
     - Cursor movement (arrows, Home/End, PageUp/PageDown)
     - Insert / Backspace / Delete / Enter
     - Mouse click to place cursor
     - Vertical scrolling with mouse wheel or keys
     - Line numbers gutter
     - Blinking caret
     - Ctrl+S to save, loads a file passed as argv[1] (or "untitled.txt")

   Build (Linux):
     gcc editor.c -o editor -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

   Build (macOS, raylib installed via brew):
     clang editor.c -o editor -lraylib -framework OpenGL -framework Cocoa \
           -framework IOKit -framework CoreVideo

   Build (Windows, MinGW):
     gcc editor.c -o editor.exe -lraylib -lopengl32 -lgdi32 -lwinmm

   Run:
     ./editor myfile.c
   ============================================================ */

#include "raylib.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---------------- Configuration ---------------- */
#define MAX_LINES 4096
#define MAX_LINE_LEN 1024
#define FONT_SIZE 20
#define LINE_HEIGHT 24
#define GUTTER_WIDTH 60
#define TEXT_PAD_X 10
#define CARET_BLINK_TIME 0.5f

/* ---------------- Editor state ---------------- */
typedef struct {
    char lines[MAX_LINES][MAX_LINE_LEN];
    int lineCount;

    int cursorRow; /* which line the cursor is on          */
    int cursorCol; /* character index within that line     */

    int scrollRow; /* first visible line (for vertical scroll) */

    float charWidth; /* width of one monospace glyph, cached  */
    float caretTimer;
    bool caretVisible;

    char filename[256];
    bool dirty;
} Editor;

/* ---------------- Helpers ---------------- */

static void EditorLoadFile(Editor* ed, const char* path)
{
    strncpy(ed->filename, path, sizeof(ed->filename) - 1);
    ed->lineCount = 0;

    FILE* f = fopen(path, "r");
    if (!f) {
        /* start with a single empty line if the file doesn't exist */
        ed->lines[0][0] = '\0';
        ed->lineCount = 1;
        return;
    }

    char buffer[MAX_LINE_LEN];
    while (fgets(buffer, sizeof(buffer), f) && ed->lineCount < MAX_LINES) {
        size_t len = strlen(buffer);
        while (len > 0 && (buffer[len - 1] == '\n' || buffer[len - 1] == '\r')) {
            buffer[--len] = '\0';
        }
        strncpy(ed->lines[ed->lineCount], buffer, MAX_LINE_LEN - 1);
        ed->lines[ed->lineCount][MAX_LINE_LEN - 1] = '\0';
        ed->lineCount++;
    }
    fclose(f);

    if (ed->lineCount == 0) {
        ed->lines[0][0] = '\0';
        ed->lineCount = 1;
    }
}

static void EditorSaveFile(Editor* ed)
{
    FILE* f = fopen(ed->filename, "w");
    if (!f)
        return;
    for (int i = 0; i < ed->lineCount; i++) {
        fputs(ed->lines[i], f);
        fputc('\n', f);
    }
    fclose(f);
    ed->dirty = false;
}

static void ClampCursor(Editor* ed)
{
    if (ed->cursorRow < 0)
        ed->cursorRow = 0;
    if (ed->cursorRow >= ed->lineCount)
        ed->cursorRow = ed->lineCount - 1;

    int len = (int)strlen(ed->lines[ed->cursorRow]);
    if (ed->cursorCol < 0)
        ed->cursorCol = 0;
    if (ed->cursorCol > len)
        ed->cursorCol = len;
}

/* Insert a single character at the cursor position */
static void EditorInsertChar(Editor* ed, char c)
{
    char* line = ed->lines[ed->cursorRow];
    int len = (int)strlen(line);
    if (len >= MAX_LINE_LEN - 1)
        return; /* line full */

    /* shift everything after the cursor one slot to the right */
    memmove(line + ed->cursorCol + 1, line + ed->cursorCol, len - ed->cursorCol + 1);
    line[ed->cursorCol] = c;
    ed->cursorCol++;
    ed->dirty = true;
}

/* Split the current line into two at the cursor (Enter key) */
static void EditorNewline(Editor* ed)
{
    if (ed->lineCount >= MAX_LINES)
        return;

    /* shift all lines below the cursor down by one */
    for (int i = ed->lineCount; i > ed->cursorRow + 1; i--) {
        strcpy(ed->lines[i], ed->lines[i - 1]);
    }

    char* line = ed->lines[ed->cursorRow];
    char* tail = line + ed->cursorCol;

    strcpy(ed->lines[ed->cursorRow + 1], tail); /* new line gets the tail */
    line[ed->cursorCol] = '\0';                 /* old line keeps the head */

    ed->lineCount++;
    ed->cursorRow++;
    ed->cursorCol = 0;
    ed->dirty = true;
}

/* Backspace: delete character before the cursor, merging lines if needed */
static void EditorBackspace(Editor* ed)
{
    if (ed->cursorCol > 0) {
        char* line = ed->lines[ed->cursorRow];
        int len = (int)strlen(line);
        memmove(line + ed->cursorCol - 1, line + ed->cursorCol, len - ed->cursorCol + 1);
        ed->cursorCol--;
        ed->dirty = true;
    } else if (ed->cursorRow > 0) {
        int prevLen = (int)strlen(ed->lines[ed->cursorRow - 1]);
        int curLen = (int)strlen(ed->lines[ed->cursorRow]);

        if (prevLen + curLen < MAX_LINE_LEN) {
            strcat(ed->lines[ed->cursorRow - 1], ed->lines[ed->cursorRow]);

            /* shift remaining lines up */
            for (int i = ed->cursorRow; i < ed->lineCount - 1; i++) {
                strcpy(ed->lines[i], ed->lines[i + 1]);
            }
            ed->lineCount--;
            ed->cursorRow--;
            ed->cursorCol = prevLen;
            ed->dirty = true;
        }
    }
}

/* Delete: delete the character under the cursor (forward delete) */
static void EditorDelete(Editor* ed)
{
    char* line = ed->lines[ed->cursorRow];
    int len = (int)strlen(line);

    if (ed->cursorCol < len) {
        memmove(line + ed->cursorCol, line + ed->cursorCol + 1, len - ed->cursorCol);
        ed->dirty = true;
    } else if (ed->cursorRow < ed->lineCount - 1) {
        int curLen = (int)strlen(line);
        int nextLen = (int)strlen(ed->lines[ed->cursorRow + 1]);

        if (curLen + nextLen < MAX_LINE_LEN) {
            strcat(line, ed->lines[ed->cursorRow + 1]);
            for (int i = ed->cursorRow + 1; i < ed->lineCount - 1; i++) {
                strcpy(ed->lines[i], ed->lines[i + 1]);
            }
            ed->lineCount--;
            ed->dirty = true;
        }
    }
}

/* Keep the cursor's row within the visible scroll window */
static void EditorScrollToCursor(Editor* ed, int visibleLines)
{
    if (ed->cursorRow < ed->scrollRow) {
        ed->scrollRow = ed->cursorRow;
    }
    if (ed->cursorRow >= ed->scrollRow + visibleLines) {
        ed->scrollRow = ed->cursorRow - visibleLines + 1;
    }
    if (ed->scrollRow < 0)
        ed->scrollRow = 0;
}

/* ---------------- Input handling ---------------- */

static void HandleKeyboard(Editor* ed, int visibleLines)
{
    bool ctrl = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);

    /* Save */
    if (ctrl && IsKeyPressed(KEY_S)) {
        EditorSaveFile(ed);
    }

    /* Character input (typed text, respects keyboard layout) */
    int key = GetCharPressed();
    while (key > 0) {
        if (key >= 32 && key < 127) {
            EditorInsertChar(ed, (char)key);
        }
        key = GetCharPressed();
    }

    if (IsKeyPressed(KEY_ENTER) || IsKeyPressedRepeat(KEY_ENTER))
        EditorNewline(ed);
    if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE))
        EditorBackspace(ed);
    if (IsKeyPressed(KEY_DELETE) || IsKeyPressedRepeat(KEY_DELETE))
        EditorDelete(ed);
    if (IsKeyPressed(KEY_TAB)) {
        EditorInsertChar(ed, ' ');
        EditorInsertChar(ed, ' ');
    }

    if (IsKeyPressed(KEY_LEFT) || IsKeyPressedRepeat(KEY_LEFT)) {
        if (ed->cursorCol > 0)
            ed->cursorCol--;
        else if (ed->cursorRow > 0) {
            ed->cursorRow--;
            ed->cursorCol = (int)strlen(ed->lines[ed->cursorRow]);
        }
    }
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressedRepeat(KEY_RIGHT)) {
        int len = (int)strlen(ed->lines[ed->cursorRow]);
        if (ed->cursorCol < len)
            ed->cursorCol++;
        else if (ed->cursorRow < ed->lineCount - 1) {
            ed->cursorRow++;
            ed->cursorCol = 0;
        }
    }
    if (IsKeyPressed(KEY_UP) || IsKeyPressedRepeat(KEY_UP))
        ed->cursorRow--;
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressedRepeat(KEY_DOWN))
        ed->cursorRow++;
    if (IsKeyPressed(KEY_PAGE_UP))
        ed->cursorRow -= visibleLines;
    if (IsKeyPressed(KEY_PAGE_DOWN))
        ed->cursorRow += visibleLines;

    if (IsKeyPressed(KEY_HOME))
        ed->cursorCol = 0;
    if (IsKeyPressed(KEY_END))
        ed->cursorCol = (int)strlen(ed->lines[ed->cursorRow]);

    ClampCursor(ed);
    EditorScrollToCursor(ed, visibleLines);
}

static void HandleMouse(Editor* ed, Rectangle textArea, int visibleLines)
{
    float wheel = GetMouseWheelMove();
    if (wheel != 0) {
        ed->scrollRow -= (int)(wheel * 3);
        if (ed->scrollRow < 0)
            ed->scrollRow = 0;
        if (ed->scrollRow > ed->lineCount - 1)
            ed->scrollRow = ed->lineCount - 1;
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), textArea)) {
        Vector2 m = GetMousePosition();

        int row = ed->scrollRow + (int)((m.y - textArea.y) / LINE_HEIGHT);
        if (row >= ed->lineCount)
            row = ed->lineCount - 1;
        if (row < 0)
            row = 0;

        int col = (int)((m.x - textArea.x) / ed->charWidth + 0.5f);
        int len = (int)strlen(ed->lines[row]);
        if (col > len)
            col = len;
        if (col < 0)
            col = 0;

        ed->cursorRow = row;
        ed->cursorCol = col;
        ed->caretVisible = true;
        ed->caretTimer = 0;
    }

    (void)visibleLines;
}

/* ---------------- Rendering ---------------- */

static void DrawEditor(Editor* ed, Font font, Rectangle textArea, int visibleLines)
{
    /* Gutter background */
    DrawRectangle(0, (int)textArea.y - 4, GUTTER_WIDTH, (int)textArea.height + 8, (Color) { 30, 30, 34, 255 });
    DrawRectangle(GUTTER_WIDTH, (int)textArea.y - 4, (int)textArea.width, (int)textArea.height + 8, (Color) { 24, 24, 28, 255 });

    int firstLine = ed->scrollRow;
    int lastLine = firstLine + visibleLines;
    if (lastLine > ed->lineCount)
        lastLine = ed->lineCount;

    for (int i = firstLine; i < lastLine; i++) {
        int screenLine = i - firstLine;
        float y = textArea.y + screenLine * LINE_HEIGHT;

        /* Line number */
        char numBuf[16];
        snprintf(numBuf, sizeof(numBuf), "%4d", i + 1);
        DrawTextEx(font, numBuf, (Vector2) { 10, y }, FONT_SIZE, 0, GRAY);

        /* Line text */
        DrawTextEx(font, ed->lines[i], (Vector2) { textArea.x, y }, FONT_SIZE, 0, RAYWHITE);

        /* Caret */
        if (i == ed->cursorRow && ed->caretVisible) {
            float caretX = textArea.x + ed->cursorCol * ed->charWidth;
            DrawRectangle((int)caretX, (int)y, 2, LINE_HEIGHT - 2, YELLOW);
        }
    }

    /* Status bar */
    char status[320];
    snprintf(status, sizeof(status), "%s%s  |  Ln %d, Col %d  |  Ctrl+S to save",
        ed->filename, ed->dirty ? " *" : "", ed->cursorRow + 1, ed->cursorCol + 1);
    DrawRectangle(0, GetScreenHeight() - 28, GetScreenWidth(), 28, (Color) { 20, 20, 24, 255 });
    DrawTextEx(font, status, (Vector2) { 10, GetScreenHeight() - 22 }, 18, 0, LIGHTGRAY);
}

/* ---------------- Main ---------------- */

int main(int argc, char* argv[])
{
    const int screenWidth = 1000;
    const int screenHeight = 700;

    InitWindow(screenWidth, screenHeight, "raylib code editor");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL); /* don't let ESC close the window; ESC is common in code */

    /* A monospace font is essential for an editor; GetFontDefault works but
       loading a proper monospace TTF (e.g. a bundled "mono.ttf") looks better.
       Falls back to raylib's default font if none is found. */
    Font font = GetFontDefault();
    if (FileExists("mono.ttf")) {
        font = LoadFontEx("mono.ttf", FONT_SIZE, NULL, 0);
    }

    Editor ed = { 0 };
    EditorLoadFile(&ed, argc > 1 ? argv[1] : "untitled.txt");

    /* Cache monospace glyph width using 'M' as reference */
    Vector2 sizeM = MeasureTextEx(font, "M", FONT_SIZE, 0);
    ed.charWidth = sizeM.x;

    while (!WindowShouldClose()) {
        int visibleLines = (int)((GetScreenHeight() - 60) / LINE_HEIGHT);
        Rectangle textArea = { GUTTER_WIDTH + TEXT_PAD_X, 20, (float)(GetScreenWidth() - GUTTER_WIDTH - TEXT_PAD_X), (float)(visibleLines * LINE_HEIGHT) };

        HandleKeyboard(&ed, visibleLines);
        HandleMouse(&ed, textArea, visibleLines);

        /* Blink the caret */
        ed.caretTimer += GetFrameTime();
        if (ed.caretTimer >= CARET_BLINK_TIME) {
            ed.caretTimer = 0;
            ed.caretVisible = !ed.caretVisible;
        }

        BeginDrawing();
        ClearBackground((Color) { 24, 24, 28, 255 });
        DrawEditor(&ed, font, textArea, visibleLines);
        EndDrawing();
    }

    if (font.texture.id != GetFontDefault().texture.id)
        UnloadFont(font);
    CloseWindow();
    return 0;
}

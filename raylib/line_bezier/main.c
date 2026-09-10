#include "raylib.h"

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "Raylib - Line Bezier");

    Vector2 startPoint = { 30, 30 };
    Vector2 endPoint = { (float)screenWidth - 30, (float)screenHeight - 30 };

    bool moveStartPoint = false;
    bool moveEndPoint = false;

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        Vector2 mouse = GetMousePosition();

        if (CheckCollisionPointCircle(mouse, startPoint, 10.0f)
            && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
            moveStartPoint = true;
        if (CheckCollisionPointCircle(mouse, endPoint, 10.0f)
            && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
            moveEndPoint = true;

        if (moveStartPoint) {
            startPoint = mouse;
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
                moveStartPoint = false;
        }

        if (moveEndPoint) {
            endPoint = mouse;
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
                moveEndPoint = false;
        }

        // DRAW
        // .....................
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("MOVE START-END POINTS WITH MOUSE", 15, 20, 20, GRAY);

        DrawLineBezier(startPoint, endPoint, 4.0f, BLUE);

        DrawCircleV(startPoint, CheckCollisionPointCircle(mouse, startPoint, 10.0f) ? 14.0f : 8.0f, moveStartPoint ? RED : BLUE);
        DrawCircleV(endPoint, CheckCollisionPointCircle(mouse, endPoint, 10.0f) ? 14.0f : 8.0f, moveStartPoint ? RED : BLUE);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

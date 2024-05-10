#include <raylib.h>
#include <math.h>

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "Rotated Rectangle with Circles");

    // Rectangle parameters
    Rectangle rect = { screenWidth / 2 - 50, screenHeight / 2 - 50, 100, 100 };
    float rotationAngle = 0; // in degrees

    // Circle parameters
    float circleRadius = 10;
    Vector2 circleTopOffset = { 0, -rect.height / 2 - circleRadius }; // Offset for top of rectangle
    Vector2 circleBottomOffset = { 0, rect.height / 2 + circleRadius }; // Offset for bottom of rectangle

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        // Update
        float deltaTime = GetFrameTime();
        rotationAngle += 30 * deltaTime; // Rotate 30 degrees per second clockwise

        BeginDrawing();

        ClearBackground(RAYWHITE);

        // Draw rotated rectangle
        DrawRectanglePro(rect, (Vector2){ rect.width / 2, rect.height / 2 }, rotationAngle, RED);

        // Calculate rotated circle positions
        Vector2 rectCenter = { rect.x + rect.width / 2, rect.y + rect.height / 2 };
        Vector2 circleTopPosition = Vector2Add(rectCenter, Vector2Rotate(circleTopOffset, rotationAngle));
        Vector2 circleBottomPosition = Vector2Add(rectCenter, Vector2Rotate(circleBottomOffset, rotationAngle));

        // Draw circles
        DrawCircleV(circleTopPosition, circleRadius, BLUE);
        DrawCircleV(circleBottomPosition, circleRadius, BLUE);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
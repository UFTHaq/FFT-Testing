// Sandbox_GenerateImageFromRawData.cpp : This file contains the 'main' function. Program execution begins and ends there.
// Author : Ukhem Fahmi Thoriqul Haq.
// Date : 02 - 06 - 2024
// Purpose : Learn to make image from data array. This is to make spectrogram visualization in Tirakat.
//

#include <memory>
#include "raylib.h"


int main() {
    // Initialize raylib
    const int screenWidth = 1000;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "Draw 2D Data Array as Texture");

    // Example 2D data array (simple gradient)
    const int width = 500;
    const int height = 300;
    //Color data[width * height]; 600120 bytes -> 600,12 kilobytes -> limit stack in windows around 1mb
    auto data = std::make_unique<Color[]>(width * height);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Create a gradient from black to white
            //data[y * width + x] = Color{ (unsigned char)(x * 255 / width), (unsigned char)(y * 255 / height), 0, 255 };
            data[y * width + x] = Color{ (unsigned char)(x * 255 / width), (unsigned char)(y * 255 / height), (unsigned char)(y * 255 / height), 255 };
        }
    }

    //C++14
    // Create an Image from the 2D data array
    //Image image = {
    //    data,
    //    width,
    //    height,
    //    1,
    //    PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    //};

    //C++20
    Image image = {
        .data = data.get(),
        .width = width,
        .height = height,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };


    // Load the Image as a Texture
    Texture2D texture = LoadTextureFromImage(image);

    SetTargetFPS(75);

    float time{};

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground({ 20,20,20,255 });

        float deltaTime = GetFrameTime();
        time += deltaTime;

        //// Shift data to the left
        //for (int y = 0; y < height; y++) {
        //    for (int x = 0; x < width - 1; x++) {
        //        data[y * width + x] = data[y * width + x + 1];
        //    }
        //}

        // Shift data to the left using std::memmove
        for (int y = 0; y < height; y++) {
            std::memmove(&data[y * width], &data[y * width + 1], (width - 1) * sizeof(Color));
        }

        // Add new data to the rightmost column
        for (int y = 0; y < height; y++) {
            unsigned char r = (unsigned char)((time * 40) * 255 / width) % 256;
            unsigned char g = (unsigned char)((y + time * 20) * 255 / height) % 256;
            unsigned char b = (unsigned char)((time * 30) * 255 / (width + height)) % 256;
            data[y * width + (width - 1)] = Color{ r, g, b, 100 };
        }

        // Update the Image with the modified data
        UpdateTexture(texture, data.get());

        // Draw the texture to the screen
        DrawTexture(texture, (screenWidth - width) / 2, (screenHeight - height) / 2, WHITE);

        // Draw the texture to the screen using different DrawTexture functions
        //DrawTexture(texture, 0, 0, RED);
        //DrawTextureV(texture, Vector2{ 600, 100 }, WHITE);
        //DrawTextureEx(texture, Vector2{ 100, 400 }, 1.5f, 45.0f, WHITE);
        //DrawTextureRec(texture, Rectangle{ 0, 0, width / 2, height / 2 }, Vector2{ 300, 200 }, WHITE);
        //DrawTextureRec(texture, Rectangle{ 0, 0, width, height }, Vector2{ 300, 200 }, WHITE);
        //DrawTexturePro(texture, Rectangle{ 0, 0, width / 2, height / 2 }, Rectangle{ 500, 200, 200, 100 }, Vector2{ 50, 25 }, 45.0f, WHITE);
        //DrawTexturePro(texture, Rectangle{ 0, 0, width , height }, Rectangle{ 0, 0, screenWidth , screenHeight }, Vector2{ 0 }, .0f, WHITE); // feels jittering

        //NPatchInfo nPatchInfo = { 0 };
        //nPatchInfo.source = Rectangle{ 0, 0, (float)texture.width, (float)texture.height };
        //nPatchInfo.left = 10;
        //nPatchInfo.top = 10;
        //nPatchInfo.right = 10;
        //nPatchInfo.bottom = 10;
        //nPatchInfo.layout = NPATCH_NINE_PATCH;

        //Rectangle destRec = { 100.0f, 100.0f, 200.0f, 200.0f };
        //DrawTextureNPatch(texture, nPatchInfo, destRec, Vector2{ 0, 0 }, 0.0f, WHITE);


        DrawFPS(10, 10);

        EndDrawing();

        // Check if the user pressed the key to save the screenshot
        if (IsKeyPressed(KEY_S)) {
            // Convert the texture to an image
            Image image = LoadImageFromTexture(texture);

            // Save the image to a PNG file
            ExportImage(image, "screenshot.png");

            // Unload the image to free memory
            UnloadImage(image);
        }
    }

    // Clean up resources
    UnloadTexture(texture);
    CloseWindow();

    return 0;
}


// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file

// FFT-Visualize.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <vector>
#include <fstream>
#include <filesystem>

#include <raylib.h>
#include <fftw3.h>

const int N = (1 << 10);
const int G = (1 << 3);
float in[N];
fftw_complex* out;
int num_groups = N / G;
int group_size = G;

struct Frame {
    float left{};
    float right{};
};

struct Screen {
    float w{};
    float h{};
};

void callback(void* bufferData, unsigned int frames) {
    Frame* fs = reinterpret_cast<Frame*>(bufferData);

    for (size_t i = 0; i < frames; i++) {
        std::rotate(in, in + 1, in + N);
        //memmove(in, in + 1, (N - 1) * sizeof(in[0]));
        in[N - 1] = fs[i].left;
    }
}

void dc_offset(float in[]) {
    float dc_offset = 0.0F;
    for (int i = 0; i < N; i++) {
        dc_offset += in[i];
    }
    dc_offset = dc_offset / (float)N;

    for (int i = 0; i < N; i++) {
        in[i] -= dc_offset;
    }
}

void hann_window(float in[], size_t n) {
    // Hann window function
    for (size_t i = 0; i < n; i++) {
        float w = 0.5F * (1.0F - cosf(2.0 * PI * i / (n - 1)));
        in[i] *= w;
    }
}

void rectangular_window(float in[], size_t n) {
    for (size_t i = 0; i < n; i++) {
        in[i] *= 1.0F; // All ones for rectangular window
    }
}

void blackman_window(float in[], size_t n) {
    const double a0 = 0.748488648864886;
    const double a1 = 0.185552250988131;
    const double a2 = 0.035977731108536;
    const double a3 = 0.030013719715309;

    for (size_t i = 0; i < n; i++) {
        double w = a0 - a1 * cos(2.0 * PI * i / (n - 1)) +
            a2 * cos(4.0 * PI * i / (n - 1)) -
            a3 * cos(6.0 * PI * i / (n - 1));
        in[i] *= w;
    }
}

void fft_calculation(float in[], fftw_complex out[], size_t n) {
    assert(n > 0);

    // Create a plan for forward FFT
    fftw_plan plan;
    plan = fftw_plan_dft_1d((int)n, reinterpret_cast<fftw_complex*>(in), out, FFTW_FORWARD, FFTW_ESTIMATE);

    // Execute the FFT
    fftw_execute(plan);

    // Destroy the plan
    fftw_destroy_plan(plan);
}

std::vector<float> extract_music_data(std::string path_file) {
    std::string cmd = "ffmpeg -i \"";
    cmd += path_file;
    //cmd += " -vn -ar 44100 -ac 2 -f s16le temp_audio_data.pcm";
    cmd += "\" -vn -ar 1500 -ac 2 -f s16le temp_audio_data.pcm";

    // Append a temporary file name to the command
    std::string temp_file = "temp_audio_data.pcm";
    //cmd += temp_file;

    // Execute the command and check for errors
    int ret = std::system(cmd.c_str());
    if (ret != 0) {
        std::cerr << "Error executing FFmpeg command" << std::endl;
        return {}; // Return empty vector on error
    }

    // Open the temporary file for reading
    std::ifstream infile(temp_file, std::ios::binary);
    if (!infile.is_open()) {
        std::cerr << "Error opening temporary file" << std::endl;
        return {}; // Return empty vector on error
    }

    // Create vector to store audio data
    std::vector<int16_t> audio_data;

    // Read data from the file and store it in the vector
    short sample;
    while (infile.read(reinterpret_cast<char*>(&sample), sizeof(short))) {
        audio_data.push_back(sample);
    }

    // Close the file and remove it (optional)
    infile.close();
    std::remove(temp_file.c_str()); // Consider error handling for remove

    // find max value
    size_t size = audio_data.size();
    std::cout << "size : " << size << std::endl;
    int max_amp = 0;
    int min_amp = 0;
    for (size_t i = 0; i < size; i++) {
        max_amp = std::max(max_amp, std::abs(audio_data[i]));
    }
    std::cout << "max amplitude: " << max_amp << std::endl;

    // Create vector to store normalized data
    std::vector<float> process_signal = {};
    for (int i = 0; i < size; i++) {
        float normalized_signal = ((float)audio_data[i] - (float)min_amp) / (float)(max_amp - min_amp);
        normalized_signal = normalized_signal * 0.8F;
        //normalized_signal = normalized_signal * 0.03F * (float)GetScreenHeight();
        process_signal.push_back(normalized_signal);
    }

    // Validation of normalization
    float max_float = 0;
    for (size_t i = 0; i < process_signal.size(); i++) {
        max_float = std::max(max_float, std::abs(process_signal[i]));
    }
    std::cout << "max amplitude float: " << max_float << std::endl;

    return process_signal;
}

int main()
{
    InitWindow(1000, 600, "FFT");
    SetConfigFlags(FLAG_MSAA_4X_HINT);

    //SetTargetFPS(100);
    InitAudioDevice();

    out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * (N));

    //std::string path = "resources/DUDUDU-Max-Verstappen.mp3";
    // TODO: FIX error if the song has whitespace
    //std::string path = "resources/DEEN ASSALAM - Cover by SABYAN.mp3";
    std::string path = "resources/DUDUDU Max Verstappen.mp3";
    //std::string path = "resources/Rocky Balboa - Theme Song (HD).mp3";
    //const char* path = "resources/SUSPENSE SOUND EFFECTS.mp3";
    SetMasterVolume(0.25F);
    Music music = LoadMusicStream(path.c_str());
    if (IsMusicReady(music)) {
        PlayMusicStream(music);
        SetMusicVolume(music, 0.5F);
    }
    AttachAudioStreamProcessor(music.stream, callback);

    std::vector<float> display_signal = extract_music_data(path);

    // Variables for low-pass filter
    float alpha = 0.05F; // Smoothing factor
    float smoothed_groups[N/G] = { 0 }; // Initialized to 0

    bool play = true;

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_SPACE)) {
            play = !play;

            if (play) {
                PlayMusicStream(music);
            }
            else {
                PauseMusicStream(music);
            }
        }

        BeginDrawing();
        ClearBackground({ 30, 30, 30, 255 });
        Screen screen = { (float)GetScreenWidth(), (float)GetScreenHeight() };
        UpdateMusicStream(music);

        DrawFPS(10, 10);

        dc_offset(in);
        //hann_window(in, N);
        //rectangular_window(in, N);
        //blackman_window(in, N);
        fft_calculation(in, out, N);

        float step = 1.06F;

        float min_amp = std::numeric_limits<float>::max();
        float max_amp = std::numeric_limits<float>::min();
        for (int i = 0; i < N; i++) {
            float amplitude = (float)sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]) ;
            min_amp = std::min(min_amp, amplitude); // Update minimum magnitude
            max_amp = std::max(max_amp, amplitude); // Update maximum magnitude
        }

        // DRAW FREQUENCY DOMAIN SIGNAL
        int coef = 2; // untuk membagi 2, untuk membuat hasil mirrored
        float bar_w = screen.w / (N / G) * coef;
        for (int group_index = 0; group_index < num_groups / coef; group_index++) {
            float sum = 0.0f;
            int start_index = group_index * group_size;
            int end_index = start_index + group_size;
            // Min-max scaling: Scale the magnitude values to the range [0, 1]
            for (int i = start_index; i < end_index; i++) {
                float magnitude = (float)sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]);
                float normalized_amp = (magnitude - min_amp) / (max_amp - min_amp);
                sum += normalized_amp;
            }
            // Cara 1
            float group_average = sum / group_size;
            //if (group_index % 100 == 1) std::cout << "data : " << group_average << std::endl;
            // Apply low-pass filter
            smoothed_groups[group_index] = (alpha * group_average) + ((1 - alpha) * smoothed_groups[group_index]);
            // Draw FFT BAR
            float bar_h = smoothed_groups[group_index] * screen.h * 0.8F;
            //float bar_h = group_average * screen.h * 0.8F;

            // Cara 2
            // Logaritmic
            //float log_amp = 20.0f * std::log10(group_average + std::numeric_limits<float>::epsilon());
            //log_amp = std::abs(log_amp);
            //if (group_index % 100 == 1) std::cout << "log : " << log_amp << std::endl;
            //// Apply low-pass filter
            //smoothed_groups[group_index] = (alpha * log_amp) + ((1 - alpha) * smoothed_groups[group_index]);
            // Draw FFT BAR
            //float bar_h = smoothed_groups[group_index] * screen.h * 0.01F;
            //float bar_h = log_amp * screen.h * 0.1F * 0.1F;
            //if (group_index % 100 == 1) std::cout << "bar : " << bar_h << std::endl;
        
            float pad = 4.0F;
            Rectangle bar = {
                (group_index * bar_w) + (pad * 1),
                screen.h - bar_h,
                bar_w - (pad * 2),
                bar_h,
            };
            
            // DRAW FFT TIME DOMAIN IN NORMAL WAY
            float hue = (float)group_index / num_groups * 1.9F;
            float sat = 1.0F;
            float val = 1.0F;
            Color color = ColorFromHSV(hue * 360, sat, val);
            DrawRectangleRec(bar, Fade(color, 0.7F));
            DrawCircle((int)bar.x + ((int)bar.width / 2), (int)bar.y, 9, Fade(RAYWHITE, 0.8F));
            DrawCircle((int)bar.x + ((int)bar.width / 2), (int)bar.y, 7, Fade(color, 0.7F));
            DrawCircle((int)bar.x + ((int)bar.width / 2), (int)bar.y, 5, Fade(color, 0.5F));
            DrawCircle((int)bar.x + ((int)bar.width / 2), (int)bar.y, 3, Fade(WHITE, 0.8F));

            // TRY TO MAKE DRAW FFT TIME DOMAIN IN ROTATION
            Vector2 center = { (screen.w / 2), (screen.h / 2) - 30 };
            float w = 20;
            float h = bar_h * 0.5F;
            Rectangle small_bar_rect = { center.x, center.y, w, h };
            float angle = (360.0F/15) * group_index;
            color = ColorFromHSV(hue * 360 + (angle), sat, val);
            DrawRectanglePro(small_bar_rect, {0,0}, angle, Fade(color, 0.7F));
            //DrawRectanglePro(small_bar_rect, {w/2,0}, angle, Fade(color, 0.7F));
        
        }

        float width = screen.w * 0.775F;
        float height = width * 0.075F;
        Rectangle display_signal_rect = {
            (screen.w - width) / 2,
            0 + screen.h * 0.15F,
            width,
            height
        };
        //DrawRectangleRec(display_signal_rect, RED);
        DrawLine(
            display_signal_rect.x, 
            display_signal_rect.y + (display_signal_rect.height * 0.5F), 
            display_signal_rect.x + display_signal_rect.width, 
            display_signal_rect.y + (display_signal_rect.height * 0.5F), 
            Fade(LIGHTGRAY, 0.8F)
        );
        
        // DRAW TIME DOMAIN SIGNAL
        for (int i = 0; i < display_signal.size() - 1; ++i) {
            //float x1 = (float)i * (screen.w / (float)display_signal.size());
            //float y1 = screen.h / 2.0f - display_signal[i];
            //float x2 = (float)(i + 1) * (screen.w / (float)display_signal.size());
            //float y2 = screen.h / 2.0f - display_signal[i + 1];
            //DrawLine(x1, y1, x2, y2, WHITE); // Adjust line color as needed
            // Calculate half thickness based on your desired line width

            // Draw the waveform line with thickness using DrawLineEx
            //float half_thickness = 0.5f; // Adjust as needed
            //Vector2 start = { x1, y1 };
            //Vector2 end = { x2, y2 };
            //DrawLineEx(start, end, half_thickness, Fade(RAYWHITE, 0.9F)); // Adjust color as needed

            //DrawPixelV({ (display_signal_rect.x + i), display_signal.at(i) }, Fade(RAYWHITE, 0.5F));

            float segments = display_signal_rect.width / (float)display_signal.size();
            float center = display_signal_rect.y + (display_signal_rect.height * 0.5F);

            // Skip 5 data
            if (i % 6 == 1) {
                float x1 = display_signal_rect.x + i * segments;
                float y1 = center - display_signal[i] * display_signal_rect.height/2;
                float x2 = display_signal_rect.x + (i + 1) * segments;
                float y2 = center - display_signal[i+1] * display_signal_rect.height/2;
                DrawLine(x1, y1, x2, y2, Fade(LIGHTGRAY, 0.8F)); // Adjust line color as needed
            }
            // TODO: OR maybe draw using DrawLineEx with more thickness and still using skipping data draw, maybe better, i dont know.


            //DrawLine(display_signal_rect.x, center, display_signal_rect.x + display_signal_rect.width, center, WHITE);
            
        }
        EndDrawing();

    }

    CloseAudioDevice();
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

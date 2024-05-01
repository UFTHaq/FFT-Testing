// FFT_Visualization_V2.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <vector>
#include <fstream>
#include <filesystem>
#include <string>

#include <raylib.h>
#include <fftw3.h>
#include <SFML/Audio.hpp>


const int N = (1 << 10);
fftw_complex* in = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * N);
fftw_complex* out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * N);

struct Frame {
    float left{};
    float right{};
};

const int BUCKETS = 100;
float Spectrum[BUCKETS];
float Freq_Bin[BUCKETS + 1];

// Define the size of the smoothing buffer
const int SMOOTHING_BUFFER_SIZE = 6;
// Declare a 2D array to store the previous amplitudes of each frequency bin
float prevAmplitudes[BUCKETS][SMOOTHING_BUFFER_SIZE] = { 0 };
// Declare an array to store the smoothed amplitudes of each frequency bin
float smoothedAmplitudes[BUCKETS] = { 0 };
float maxAmplitude = 0.0F;

struct PeakInfo {
    int frequency_index{0};
    float amplitude{};
};
PeakInfo Peak[BUCKETS];

struct Screen {
    float w{};
    float h{};
};

void callback(void* bufferData, unsigned int frames) {
    if (frames > N) frames = N;

    Frame* fs = reinterpret_cast<Frame*>(bufferData);

    for (size_t i = 0; i < frames; i++) {
        float left = fs[i].left;
        float right = fs[i].right;

        in[i][0] = left;
        in[i][1] = 0.0F;
    }
}

void cleanup() {
    if (in != nullptr) {
        fftw_free(in);
    }
    if (out != nullptr) {
        fftw_free(out);
    }
}

void dc_offset(fftw_complex in[]) {
    float dc_offset = 0.0F;
    for (int i = 0; i < N; i++) {
        dc_offset += in[i][0];
    }
    dc_offset = dc_offset / (float)N;

    for (int i = 0; i < N; i++) {
        in[i][0] -= dc_offset;
    }
}

void hann_window(fftw_complex in[], size_t n) {
    for (size_t i = 0; i < n; i++) {
        float w = 0.5F * (1.0F - cosf(2.0F * PI * i / (n - 1)));
        in[i][0] *= w;
    }
}

void fft_calculation(fftw_complex in[], fftw_complex out[], size_t n) {
    assert(n > 0);

    fftw_plan plan{};
    plan = fftw_plan_dft_1d((int)n, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

    fftw_execute(plan);

    fftw_destroy_plan(plan);
}


float natural_scale(float amplitude, float Fit_factor) {
    return amplitude * Fit_factor;
}

float exponential_scale(float amplitude, float Fit_factor) {
    return std::log10(amplitude * Fit_factor) * Fit_factor;
}

float multi_peak_scale(float amplitude, int i, float Fit_factor, PeakInfo* Peak) {
    if (Peak[i].frequency_index >= 0) {
        return amplitude / Peak[i].amplitude * Fit_factor;
    }
    else {
        // Handle the case where there's no peak (e.g., set to a default value)
        return amplitude * Fit_factor; // Or another appropriate factor
    }
}

float max_peak_scale(float amplitude, float Global_Peak, float Fit_factor) {
    return amplitude / Global_Peak * Fit_factor;
}

enum MODE {
    NATURAL = 1,
    EXPONENTIAL,
    MULTI_PEAK,
    MAX_PEAK
};

int mode = MULTI_PEAK;

// Function to calculate the moving average of an array
float calculateMovingAverage(float* arr, int size) {
    float sum = 0;
    for (int i = 0; i < size; ++i) {
        sum += arr[i];
    }
    return sum / size;
}

std::vector<float> extract_music_data(const std::string& filename) {
    std::vector<float> audio_data{};

    // Load the entire audio for processing (modify for large files)
    sf::SoundBuffer soundBuffer;
    soundBuffer.loadFromFile(filename);

    sf::Uint64 total_samples = soundBuffer.getSampleCount();
    std::cout << "getsamplecount: " << total_samples << std::endl;
    audio_data.reserve(total_samples);

    // Extract all samples directly from sound buffer
    const sf::Int16* samples = soundBuffer.getSamples();

    for (size_t i = 0; i < total_samples; i++) {
        // Convert and push back all samples (no downsampling)
        float sample = static_cast<float>(samples[i]) / 32767.f; // Assuming 32-bit signed integer
        audio_data.push_back(sample);
    }
    std::cout << "audio_data : " << audio_data.size() << std::endl;

    float max_amp = 0;
    float min_amp = 0;
    for (size_t i = 0; i < audio_data.size(); i++) {
        max_amp = std::max(max_amp, std::abs(audio_data[i]));
    }
    std::cout << "max amplitude: " << max_amp << std::endl;

    // Downsampling
    std::vector<float> final_signal{};
    int downsampling_rate = 600;
    int index = 0;
    for (int i = 0; i < audio_data.size(); i += downsampling_rate) {
        //final_signal.push_back(process_signal.at(i));
        final_signal.push_back(audio_data.at(i));
        index++;
    }
    std::cout << "final_signal : " << final_signal.size() << std::endl;

    //return audio_data;
    return final_signal;

    // Catatan:
    // Jika input berupa file WAV, perlu penguatan pada amplitude, sekitar 3 - 6 kali lipat.
}


int main()
{
    std::cout << "Hello World!\n";

    InitWindow(1000, 600, "MUSIC-Visualization");
    SetTargetFPS(100);

    InitAudioDevice();

    std::string path = "resources/Watching Wirtual.mp3";
    //std::string path = "resources/277270__serylis__gibson-guitar-130bpm.wav";
    //std::string path = "resources/DUDUDU Max Verstappen.mp3";
    //std::string path = "resources/DEEN ASSALAM - Cover by SABYAN.mp3";
    //std::string path = "resources/radja_yakin.mp3";

    std::vector<float> display_signal{ extract_music_data(path) };

    SetMasterVolume(0.5F);
    Music music = LoadMusicStream(path.c_str());

    if (IsMusicReady(music)) {
        PlayMusicStream(music);
        SetMusicVolume(music, 0.5F);
    }
    AttachAudioStreamProcessor(music.stream, callback);

    bool play = true;

    int SampleRate = music.stream.sampleRate;
    std::cout << "SampleRate : " << SampleRate << std::endl;

    float min_frequency = 20.0F;
    float max_frequency = 20000.0F;

    float bin_width = (max_frequency - min_frequency) / BUCKETS; // Frequency range per bin
    for (int i = 0; i <= BUCKETS; i++) {
        Freq_Bin[i] = min_frequency + i * bin_width; // Lower bound for each bin
        std::cout << "Freq[" << i << "] : " << Freq_Bin[i] << std::endl;
    }

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
        else if (IsKeyPressed(KEY_ONE)) {
            mode = NATURAL;
        }
        else if (IsKeyPressed(KEY_TWO)) {
            mode = EXPONENTIAL;
        }
        else if (IsKeyPressed(KEY_THREE)) {
            mode = MULTI_PEAK;
        }
        else if (IsKeyPressed(KEY_FOUR)) {
            mode = MAX_PEAK;
        }

        BeginDrawing();
        ClearBackground({ 30, 30, 30, 255 });
        Screen screen = { (float)GetScreenWidth(), (float)GetScreenHeight() };
        DrawFPS(10, 10);

        UpdateMusicStream(music);

        dc_offset(in);
        hann_window(in, N);
        fft_calculation(in, out, N);
        
        for (int i = 0; i < BUCKETS; i++) {
            Spectrum[i] = 0.0F;
        }

        float min_amp = std::numeric_limits<float>::max();
        float max_amp = std::numeric_limits<float>::min();
        for (int i = 0; i < N; i++) {
            float real_num = out[i][0];
            float imaginer = out[i][1];
        
            float amplitude = std::sqrt(real_num * real_num + imaginer * imaginer);
        
            min_amp = std::min(min_amp, amplitude); // Update minimum magnitude
            max_amp = std::max(max_amp, amplitude); // Update maximum magnitude
        }


        for (int i = 0; i < (N/2); i++) {
            float real_num = out[i][0];
            float imaginer = out[i][1];
        
            float amplitude = std::sqrt(real_num * real_num + imaginer * imaginer);
        
        
            for (int j = 0; j < BUCKETS; j++) {
                float freq = min_frequency + i * bin_width;

                if (freq >= Freq_Bin[j] && freq <= Freq_Bin[j + 1]) {
                    Spectrum[j] = std::max(Spectrum[j], amplitude);
                    //Spectrum[j] = amplitude;
        
                    if (amplitude > Peak[j].amplitude) {
                        Peak[j].amplitude = amplitude;
                        Peak[j].frequency_index = i;
                    }
                }
            }
        }

        for (int i = 0; i < BUCKETS; i++) {
            for (int j = SMOOTHING_BUFFER_SIZE - 1; j > 0; --j) {
                prevAmplitudes[i][j] = prevAmplitudes[i][j - 1];
            }

            float norm_amplitude = Spectrum[i];
            //norm_amplitude = std::log2(Spectrum[i] + 1.0f);

            //norm_amplitude = exponential_scale(norm_amplitude, 0.6F);
            //norm_amplitude = multi_peak_scale(norm_amplitude, i, 0.9F, Peak);
            //norm_amplitude = max_peak_scale(norm_amplitude, float Global_Peak, float Fit_factor);
            //natural_scale(norm_amplitude, 3.0F);

            // Store the current amplitude in the smoothing buffer
            //norm_amplitude = (norm_amplitude - min_amp) / (max_amp - min_amp);
            prevAmplitudes[i][0] = norm_amplitude;

            // Calculate the moving average of the smoothing buffer
            smoothedAmplitudes[i] = calculateMovingAverage(prevAmplitudes[i], SMOOTHING_BUFFER_SIZE);

            
            maxAmplitude = std::max(maxAmplitude, smoothedAmplitudes[i]);

            //const float DECAY_FACTOR = 0.95f;
            //smoothedAmplitudes[i] *= DECAY_FACTOR;
        }

        // Just DRAW
        for (int i = 0; i < BUCKETS; i++) {
            float final_amplitude = smoothedAmplitudes[i];

            float rotation_h_coef{};
            switch (mode)
            {
            case NATURAL:
                final_amplitude = natural_scale(final_amplitude, 0.02F);
                rotation_h_coef = 0.6F;
                break;
            case EXPONENTIAL:
                final_amplitude = exponential_scale(final_amplitude, 0.6F);
                rotation_h_coef = 0.9F;
                break;
            case MULTI_PEAK:
                final_amplitude = multi_peak_scale(final_amplitude, i, 1.1F, Peak);
                rotation_h_coef = 0.9F;
                break;
            case MAX_PEAK:
                final_amplitude = max_peak_scale(final_amplitude, maxAmplitude, 0.9F);
                rotation_h_coef = 0.9F;
                break;
            default:
                break;
            }

            //final_amplitude = 0.4 * log2f(final_amplitude + 2);
        
            float bar_h = final_amplitude * screen.h * 0.5F;
            float bar_w = screen.w / BUCKETS;
        
            float pad = 2.0F;
            float base_h = 5.0F;
            Rectangle bar = {
                (i * bar_w) + (pad * 1),
                screen.h - bar_h - base_h,
                bar_w - (pad * 2),
                bar_h
            };

            Rectangle base = {
                (i * bar_w) + (pad * 1),
                screen.h - base_h,
                bar_w - (pad * 2),
                base_h
            };
            Color color{};
            float hue = (float)i / BUCKETS;
            float sat = 1.0F;
            float val = 1.0F;
            color = ColorFromHSV(hue * 360, sat, val);

            DrawRectangleRec(base, GRAY);
            DrawRectangleRec(bar, color);


            // TRY TO MAKE DRAW FFT TIME DOMAIN IN ROTATION
            Vector2 center = { (screen.w / 2), (screen.h / 2) - 10 };
            float w = bar_w * 0.6F;
            //float h = sqrtf(bar_h * rotation_h_coef) * 10;
            float h = bar_h * rotation_h_coef;
            Rectangle small_bar_rect = { center.x, center.y, w, h };
            float angle = (360.0F / 50.0F) * i;
            color = ColorFromHSV(hue * 360 + (angle), sat, val);
            DrawRectanglePro(small_bar_rect, { 0,0 }, angle, Fade(color, 0.7F));
            //DrawRectanglePro(small_bar_rect, {w/2,0}, angle, Fade(color, 0.7F));

            //DrawCircleV(center, 10, { 30, 30, 30, 255 });

        }
        // Diffuser Center circle
        Vector2 center = { (screen.w / 2), (screen.h / 2) - 10 };
        DrawCircleV(center, 8, Fade(DARKGRAY, 0.5F));
        DrawCircleV(center, 4, Fade(RAYWHITE, 0.8F));

        // DRAW TIME DOMAIN SIGNAL
        float width = screen.w * 0.775F;
        float height = width * 0.04F;
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

        for (int i = 0; i < display_signal.size() - 1; ++i) {
        
            float segments = display_signal_rect.width / (float)display_signal.size();
            float center = display_signal_rect.y + (display_signal_rect.height * 0.5F);
        
            float x1 = display_signal_rect.x + i * segments;
            float y1 = center - display_signal[i] * display_signal_rect.height / 2;
            float x2 = display_signal_rect.x + (i) * segments;
            float y2 = center - (-display_signal[i]) * display_signal_rect.height / 2;
            DrawLine(x1, y1, x2, y2, Fade(LIGHTGRAY, 0.8F)); // Adjust line color as needed
        }

        EndDrawing();
    }

    cleanup();
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

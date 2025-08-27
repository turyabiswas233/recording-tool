#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <limits>
#include <algorithm>
#include <cmath>

// Noise reduction function
sf::SoundBuffer reduceNoise(const sf::SoundBuffer &originalBuffer, float thresholdRatio = 0.02f)
{
    // Get original audio data
    const sf::Int16 *samples = originalBuffer.getSamples();
    size_t sampleCount = originalBuffer.getSampleCount();
    unsigned int sampleRate = originalBuffer.getSampleRate();
    unsigned int channelCount = originalBuffer.getChannelCount();

    // Create a copy of samples to modify
    std::vector<sf::Int16> processedSamples(samples, samples + sampleCount);

    // 1. First pass: analyze noise floor (simple approach)
    float noiseFloor = 0.0f;
    const size_t analysisSamples = std::min<size_t>(sampleRate, sampleCount); // Analyze first second
    for (size_t i = 0; i < analysisSamples; ++i)
    {
        noiseFloor += std::abs(samples[i]);
    }
    noiseFloor /= analysisSamples;
    std::cout << "Noise floor: " << noiseFloor << "\n";
    const sf::Int16 threshold = static_cast<sf::Int16>(noiseFloor * thresholdRatio);

    // 2. Second pass: apply noise reduction
    for (size_t i = 0; i < sampleCount; ++i)
    {
        sf::Int16 sample = samples[i];

        // Simple threshold-based noise gate
        if (std::abs(sample) < threshold)
        {
            processedSamples[i] = 0; // Silence quiet samples
        }
        // Optional: Add soft clipping for samples near threshold
        else if (std::abs(sample) < threshold * 2)
        {
            processedSamples[i] = sample * 5.0f; // Reduce amplitude
        }
    }

    // 3. Optional: Simple low-pass filter (reduces high-frequency noise)
    if (channelCount == 1)
    { // Mono
        for (size_t i = 1; i < sampleCount - 1; ++i)
        {
            processedSamples[i] = (processedSamples[i - 1] + processedSamples[i] + processedSamples[i + 1]) / 3;
        }
    }
    else if (channelCount == 2)
    { // Stereo
        for (size_t i = 2; i < sampleCount - 2; i += 2)
        {
            // Left channel
            processedSamples[i] = (processedSamples[i - 2] + processedSamples[i] + processedSamples[i + 2]) / 3;
            // Right channel
            processedSamples[i + 1] = (processedSamples[i - 1] + processedSamples[i + 1] + processedSamples[i + 3]) / 3;
        }
    }

    // Create new buffer with processed data
    sf::SoundBuffer processedBuffer;
    processedBuffer.loadFromSamples(
        processedSamples.data(),
        sampleCount,
        channelCount,
        sampleRate);

    return processedBuffer;
}

using namespace std;
int main()
{
    // Check if recording is available at all
    if (!sf::SoundBufferRecorder::isAvailable())
    {
        cerr << "Audio recording is not available on this system!\n";
        return -1;
    }

    // Get list of available devices
    vector<string> devices = sf::SoundRecorder::getAvailableDevices();

    if (devices.empty())
    {
        cerr << "No audio recording devices found!\n";
        return -1;
    }

    // List available devices
    cout << "Available recording devices:\n";
    for (size_t i = 0; i < devices.size(); ++i)
    {
        cout << i << ": " << devices[i] << "\n";
    }

    // Let user select a device
    size_t selectedDevice = 0;
    cout << "Select device to use (0-" << devices.size() - 1 << "): ";
    cin >> selectedDevice;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if (selectedDevice >= devices.size())
    {
        cerr << "Invalid device selection!\n";
        return -1;
    }

    // Create recorder and set device
    sf::SoundBufferRecorder recorder;
    if (!recorder.setDevice(devices[selectedDevice]))
    {
        cerr << "Failed to set recording device: " << devices[selectedDevice] << "\n";
        return -1;
    }

    cout << "Using device: " << devices[selectedDevice] << "\n";

    sf::RenderWindow window(sf::VideoMode(500, 400), "Audio Recorder");
    window.setFramerateLimit(60);
    sf::CircleShape circle(50);
    circle.setFillColor(sf::Color::Black);
    circle.setOutlineColor(sf::Color::Red);
    circle.setOutlineThickness(5);
    circle.setPosition(200, 100);

    vector<sf::RectangleShape> rectangles;
    for (size_t i = 0; i < 10; ++i)
    {
        sf::RectangleShape rectangle(sf::Vector2f(20, 150 * (i % 2 == 0 ? 1.0 : 0.5)));
        rectangle.setFillColor(sf::Color::Yellow);
        rectangle.setPosition(i * 22 + 200, 200);
        rectangles.push_back(rectangle);
    }

    bool recording = true;
    // Start recording with default sample rate (44100 Hz)
    if (recording == true)
    {
        recorder.start();
        circle.setFillColor(sf::Color::Green);
        circle.setOutlineColor(sf::Color::Black);
    }
    while (window.isOpen())
    {

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.key.code == sf::Keyboard::Space)
            {
                recording = false;
                // Stop recording on mouse click
                recorder.stop();
                window.clear();
                window.close();
            }
        }
        window.clear(sf::Color(5, 5, 15, 200));
        window.draw(circle);
        for (const auto &rectangle : rectangles)
        {
            window.draw(rectangle);
        }
        window.display();
    }

    // Get the recorded audio buffer
    const sf::SoundBuffer &buffer = recorder.getBuffer();
    // Save to WAV file
    string filename = "original_recording.wav";
    if (!buffer.saveToFile(filename))
    {
        cerr << "Failed to save recording to file!\n";
        return -1;
    }
    const sf::SoundBuffer cleanBuffer = reduceNoise(buffer);
    filename = "cleaned_recording.wav";
    if (!cleanBuffer.saveToFile(filename))
    {
        cerr << "Failed to save cleaned recording to file!\n";
        return -1;
    }

    cout << "Recording saved to " << filename << "\n";
    cout << "Duration: " << cleanBuffer.getDuration().asSeconds() << " seconds\n";
    cout << "Sample rate: " << cleanBuffer.getSampleRate() << " Hz\n";
    cout << "Channels: " << cleanBuffer.getChannelCount() << "\n";

    return 0;
}
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>
#include <sstream>
#include <cmath>

using namespace std;
using namespace sf;

#define WIDTH 1000.0f
#define HEIGHT 600.0f
float amplifier = 1.0f;

void drawWaveform(RenderWindow &window, const SoundBuffer &buffer, Font &font)
{
    // Get the audio samples
    const Int16 *samples = buffer.getSamples();
    size_t sampleCount = buffer.getSampleCount();
    unsigned int channelCount = buffer.getChannelCount();
    unsigned int sampleRate = buffer.getSampleRate();
    const unsigned int amplifier = 5;

    stringstream ss;
    ss << "Sample Rate: " << to_string(sampleRate) << " Hz" << endl;
    ss << "Channel Count: " << channelCount << endl;
    ss << "Sample Count: " << sampleCount << endl;

    Text text("Waveform\n" + ss.str(), font, 20);
    text.setFillColor(Color::White);
    text.setPosition(10, 10);
    window.draw(text);

    // Prepare the samples
    sf::VertexArray sine(sf::LineStrip, sampleCount - 1);
    vector<Vertex> vertices(sampleCount / 100);
    double x_range = static_cast<double>(sampleCount - 1);
    float scaleFactor = static_cast<float>(WIDTH - 20) / sampleCount;

    for (size_t i = 0, k = 0; i < sampleCount && k < sampleCount / 100; i += 100, k++)
    {
        float x = 10 + i * scaleFactor;
        float y = (samples[i] / 32767.0f * 100) + HEIGHT / 2;
        vertices[k] = Vertex(Vector2f(x, y), Color::Yellow);
    }
    // Draw the waveform
    window.draw(&vertices[0], vertices.size(), Lines);
}

void printAudioLength(RenderWindow &window, float duration, float curLen, Font &font)
{

    string fixedTo2Digits1 = to_string(curLen);
    fixedTo2Digits1 = fixedTo2Digits1.substr(0, fixedTo2Digits1.find(".") + 3);

    string fixedTo2Digits2 = to_string(duration);
    fixedTo2Digits2 = fixedTo2Digits2.substr(0, fixedTo2Digits2.find(".") + 3);

    Text text("\nDuration: " + fixedTo2Digits1 + "/" + fixedTo2Digits2 + " seconds", font, 20);
    text.setPosition(10, 5 * 20);
    text.setFillColor(Color::Yellow);
    window.draw(text);
}

void amplifyBuffer(SoundBuffer &buffer, float factor)
{
    const Int16 *samples = buffer.getSamples();
    size_t sampleCount = buffer.getSampleCount();
    Int16 *amplifiedSamples = new Int16[sampleCount];

    for (size_t i = 0; i < sampleCount; ++i)
    {
        amplifiedSamples[i] = static_cast<Int16>(samples[i] * factor);
    }

    buffer.loadFromSamples(amplifiedSamples, sampleCount, buffer.getChannelCount(), buffer.getSampleRate());
    delete[] amplifiedSamples;
}

void drawVolumeBar(RenderWindow &window, float volume)
{
    RectangleShape bar(Vector2f(200, 20));
    bar.setFillColor(Color::Green);
    bar.setPosition(WIDTH - 210, HEIGHT - 30);
    bar.setSize(Vector2f(200 * (volume / 100), 20));
    window.draw(bar);

    Text text("Volume: " + to_string(static_cast<int>(volume)) + "%", Font(), 20);
    text.setFillColor(Color::White);
    text.setPosition(WIDTH - 210, HEIGHT - 60);
    window.draw(text);
}

int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        cout << "Usage: " << argv[0] << " <audio file>" << endl;
        return 1;
    }
    // Load the audio file
    SoundBuffer buffer;
    if (!buffer.loadFromFile(argv[1]))
    {
        cout << "Failed to load audio file: " << argv[1] << endl;
        return 1;
    }

    // Get the audio samples
    const Int16 *samples = buffer.getSamples();
    size_t sampleCount = buffer.getSampleCount();
    unsigned int sampleRate = buffer.getSampleRate();
    unsigned int channelCount = buffer.getChannelCount();
    string selectedDevice = SoundRecorder::getDefaultDevice();
    float duration = buffer.getDuration().asSeconds();

    bool isPlaying = false;

    cout << "Sample Rate: " << sampleRate << " Hz" << endl;
    cout << "Channel Count: " << channelCount << endl;
    cout << "Sample Count: " << sampleCount << endl;
    cout << "Duration: " << duration << " seconds" << endl;
    cout << "Default Device:" << selectedDevice << endl;

    // window
    RenderWindow window(VideoMode(WIDTH, HEIGHT), "Waveform");
    window.setFramerateLimit(60);
    RectangleShape playButton(Vector2f(50, 50));
    Vertex vertices[2] = {
        Vertex(Vector2f(10, HEIGHT - HEIGHT / 4), Color::White),
        Vertex(Vector2f(WIDTH - 10, HEIGHT - HEIGHT / 4), Color::White)};
    CircleShape playBall = CircleShape(10);
    playBall.setFillColor(Color::Green);
    playBall.setPosition(10, HEIGHT - HEIGHT / 4 - playBall.getRadius());
    playButton.setFillColor(Color::Green);
    playButton.setPosition(WIDTH / 2 - playButton.getSize().x / 2, HEIGHT - playButton.getSize().y - 10);
    Font font;

    Sound sound(buffer);

    if (!font.loadFromFile("./firacode.ttf"))
    {
        cerr << "Failed to load font" << endl;
    }
    float currentLength = 0.0f;
    sound.setVolume(100);

    while (window.isOpen())
    {
        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
                window.close();
            if (event.type == Event::MouseButtonPressed || event.type == Event::KeyPressed)
            {
                if ((event.mouseButton.button == Mouse::Left && playButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) || event.key.code == Keyboard::Space)
                    // play audio
                    if (isPlaying)
                    {
                        sound.pause();
                        playButton.setFillColor(Color::Green);
                    }
                    else
                    {
                        sound.play();
                        playButton.setFillColor(Color::Red);
                    }
                isPlaying = !isPlaying;
            }
            if (event.type == Event::MouseWheelScrolled)
            {
                if (event.mouseWheelScroll.wheel == Mouse::VerticalWheel)
                {
                    if (event.mouseWheelScroll.delta > 0)
                        amplifier += 1; // Increase volume
                    else if (event.mouseWheelScroll.delta < 0)
                        amplifier -= 1; // Decrease volume

                    if (amplifier <= 1.0f)
                        amplifier = 1.0f;
                    else if (amplifier >= 5.0f)
                        amplifier = 5.0f;
                    amplifyBuffer(buffer, amplifier);
                    sound.setBuffer(buffer);
                }
            }
        }
        if (sound.getStatus() == Sound::Playing)
        {
            currentLength += 1.0f / 60.0f;
        }
        else if (sound.getStatus() == Sound::Paused)
        {
            currentLength = sound.getPlayingOffset().asSeconds();
        }
        else if (sound.getStatus() == Sound::Stopped)
        {
            currentLength = 0.0f;
            playButton.setFillColor(Color::Green);
            isPlaying = false;
        }

        playBall.setPosition(10 + currentLength / duration * (WIDTH - 20) - playBall.getRadius(), HEIGHT - HEIGHT / 4 - playBall.getRadius());

        window.clear(Color(10, 10, 10));
        drawWaveform(window, buffer, font);
        printAudioLength(window, duration, currentLength, font);
        drawVolumeBar(window, amplifier / 5.0 * 100);
        window.draw(playButton);
        window.draw(vertices, 2, Lines);
        window.draw(playBall);
        window.display();
    }
    return 0;
}

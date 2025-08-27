#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>

int main() {
    // Take input for x-axis range and amplitude
    double x_min, x_max, amplitude;
    std::cout << "Enter x-axis range (min max), e.g. -3.14 3.14: ";
    std::cin >> x_min >> x_max;
    std::cout << "Enter amplitude (a): ";
    std::cin >> amplitude;

    // Window settings
    const int width = 800, height = 600;
    sf::RenderWindow window(sf::VideoMode(width, height), "Sine Wave");

    // Prepare sine wave points
    sf::VertexArray sine(sf::LineStrip, width/2);
    double x_range = x_max - x_min;
    for (int i = 0; i < width/2; ++i) {
        double x = x_min + (x_range * i) / (width/2 - 1);
        double y = amplitude * std::sin(x);
        // Map x to [0, width], y to [0, height] (centered vertically)
        float px = static_cast<float>(i);
        float py = static_cast<float>(height / 2 - y * (height / 2) / (std::abs(amplitude) > 0 ? std::abs(amplitude) : 1));
        sine[i].position = sf::Vector2f(px, py);
        sine[i].color = sf::Color::Red;
    }

    // Main loop
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        window.clear(sf::Color::White);
        window.draw(sine);
        window.display();
    }
    return 0;
}
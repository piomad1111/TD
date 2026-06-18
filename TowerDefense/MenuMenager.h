#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <functional>
#include <string>

// Enum stanu gry
enum class GameState;

struct Button {
    sf::RectangleShape background;
    sf::Text text;
    std::function<void()> onClick;

    Button(const sf::Font& font) : text(font) {}
    // Rule of 5 needed because sf::Text holds a reference to a font
    Button(Button&&) = default;
    Button& operator=(Button&&) = default;
    Button(const Button&) = delete;
    Button& operator=(const Button&) = delete;
};

class MenuManager {
public:
    MenuManager(sf::Font& font);

    // NOWE: Dynamiczne zarz dzanie przyciskami
    void clearButtons();
    void addButton(const std::string& text, sf::Vector2f position, std::function<void()> action);

    void update(sf::Vector2f mousePos);
    void handleClick(sf::Vector2f mousePos);
    void draw(sf::RenderWindow& window);

private:
    std::vector<Button> activeButtons;
    sf::Font& mainFont;
};
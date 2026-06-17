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

    // POPRAWKA DLA SFML 3: Poniewa¿ sf::Text nie ma ju¿ domyœlnego konstruktora,
    // musimy zainicjalizowaæ go czcionk¹ na liœcie inicjalizacyjnej.
    Button(const sf::Font& font) : text(font) {}
};

class MenuManager {
public:
    MenuManager(sf::Font& font);

    void initMenu(GameState state, std::function<void(GameState)> stateChanger, std::function<void()> closeApp);
    void update(sf::Vector2f mousePos);
    void handleClick(sf::Vector2f mousePos);
    void draw(sf::RenderWindow& window);

private:
    std::vector<Button> activeButtons;
    sf::Font& mainFont;

    void createButton(const std::string& text, sf::Vector2f position, std::function<void()> action);
};
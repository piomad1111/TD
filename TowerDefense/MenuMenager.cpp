#include "MenuMenager.h"
#include "Game.h" 

MenuManager::MenuManager(sf::Font& font) : mainFont(font) {}

void MenuManager::createButton(const std::string& btnText, sf::Vector2f position, std::function<void()> action) {
    // POPRAWKA DLA SFML 3: Wywo³ujemy konstruktor Button, podaj¹c czcionkê
    Button btn(mainFont);

    btn.text.setString(btnText);
    btn.text.setCharacterSize(50);
    btn.text.setFillColor(sf::Color::White);
    btn.text.setPosition(position);

    btn.background.setSize(sf::Vector2f(250.f, 60.f));
    btn.background.setFillColor(sf::Color(50, 50, 50, 200));
    btn.background.setOutlineThickness(2.f);
    btn.background.setOutlineColor(sf::Color::White);

    // POPRAWKA DLA SFML 3: Funkcja setPosition wymaga teraz jednego obiektu sf::Vector2f. 
    // Zamykamy dwa argumenty w nawiasy klamrowe {}, aby niejawnie stworzyæ wektor.
    btn.background.setPosition({ position.x - 25.f, position.y - 5.f });

    btn.onClick = action;

    activeButtons.push_back(btn);
}

void MenuManager::initMenu(GameState state, std::function<void(GameState)> stateChanger, std::function<void()> closeApp) {
    activeButtons.clear();

    sf::VideoMode desktopMode = sf::VideoMode::getDesktopMode();
    float centerX = desktopMode.size.x / 2.0f - 100.f;

    // POPRAWKA: Tworzymy przyciski TYLKO dla menu g³ównego (MAIN_MENU), a nie dla ekranu logowania
    if (state == GameState::MAIN_MENU) {
        createButton("Graj", { centerX, 360.f }, [stateChanger]() {
            stateChanger(GameState::GAMEPLAY);
            });

        createButton("Wyjscie", { centerX, 460.f }, closeApp);
    }
}

void MenuManager::update(sf::Vector2f mousePos) {
    for (auto& btn : activeButtons) {
        if (btn.background.getGlobalBounds().contains(mousePos)) {
            btn.text.setFillColor(sf::Color::Green);
            btn.background.setOutlineColor(sf::Color::Green);
        }
        else {
            btn.text.setFillColor(sf::Color::White);
            btn.background.setOutlineColor(sf::Color::White);
        }
    }
}

void MenuManager::handleClick(sf::Vector2f mousePos) {
    for (auto& btn : activeButtons) {
        if (btn.background.getGlobalBounds().contains(mousePos)) {
            if (btn.onClick) {
                btn.onClick();
            }
        }
    }
}

void MenuManager::draw(sf::RenderWindow& window) {
    for (const auto& btn : activeButtons) {
        window.draw(btn.background);
        window.draw(btn.text);
    }
}
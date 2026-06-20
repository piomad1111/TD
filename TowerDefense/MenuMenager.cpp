#include "MenuMenager.h"
#include "Game.h" 

MenuManager::MenuManager(sf::Font& font) : mainFont(font) {}

void MenuManager::clearButtons() {
    activeButtons.clear();
}

void MenuManager::addButton(const std::string& btnText, sf::Vector2f position, std::function<void()> action) {
    Button btn(mainFont);
    btn.text.setString(btnText);
    btn.text.setCharacterSize(40); // Zmniejszone z 50 do 40, aby estetyczniej pasowa³o
    btn.text.setFillColor(sf::Color::White);

    // U¿ywamy position.x i size.x z SFML 3
    sf::FloatRect textBounds = btn.text.getLocalBounds();

    // Dynamiczna szerokoœæ przycisku dopasowuj¹ca siê do wielkoœci tekstu + bufor 60px
    float bgWidth = std::max(350.f, textBounds.size.x + 60.f);

    btn.text.setOrigin({ textBounds.position.x + textBounds.size.x / 2.0f, textBounds.position.y + textBounds.size.y / 2.0f });
    btn.text.setPosition(position);

    btn.background.setSize(sf::Vector2f(bgWidth, 60.f));
    btn.background.setFillColor(sf::Color(50, 50, 50, 200));
    btn.background.setOutlineThickness(2.f);
    btn.background.setOutlineColor(sf::Color::White);

    // U¿ywamy position.x i size.x z SFML 3
    sf::FloatRect bgBounds = btn.background.getLocalBounds();
    btn.background.setOrigin({ bgBounds.position.x + bgBounds.size.x / 2.0f, bgBounds.position.y + bgBounds.size.y / 2.0f });
    btn.background.setPosition(position);

    btn.onClick = action;
    activeButtons.push_back(std::move(btn));
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
                break; // PRZERWANIE PÊTLI - wektor móg³ zostaæ wyczyszczony przez onClick()!
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
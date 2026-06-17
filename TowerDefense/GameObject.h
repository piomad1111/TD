#pragma once
#include <SFML/Graphics.hpp>

// Abstrakcyjna klasa bazowa dla wszystkich obiektów w grze (Wrogowie, Wie¿e, Pociski)
class GameObject {
protected:
    sf::Vector2f position;
    // USUNIÊTO: sf::Sprite sprite; (powodowa³o b³¹d braku domyœlnego konstruktora w SFML 3)

public:
    virtual ~GameObject() = default;

    // Czysto wirtualna funkcja logiki - ka¿dy obiekt musi j¹ zaimplementowaæ
    virtual void update(float dt) = 0;

    // NOWE: Czysto wirtualna funkcja rysowania - ka¿dy obiekt (np. Enemy, Wie¿a) rysuje siê sam swoimi metodami
    virtual void draw(sf::RenderWindow& window) = 0;

    sf::Vector2f getPosition() const { return position; }
};
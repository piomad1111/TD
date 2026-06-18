#pragma once
#include <SFML/Graphics.hpp>

// Abstrakcyjna klasa bazowa dla wszystkich obiektow w grze (Wrogowie, Wieze, Pociski)
class GameObject {
protected:
    sf::Vector2f position;

public:
    virtual ~GameObject() = default;

    // Czysto wirtualna funkcja logiki - kazdy obiekt musi ja zaimplementowac 
    virtual void update(float dt) = 0;

    // Czysto wirtualna funkcja rysowania - kazdy obiekt rysuje sie sam
    virtual void draw(sf::RenderWindow& window) = 0;

    sf::Vector2f getPosition() const { return position; }
};
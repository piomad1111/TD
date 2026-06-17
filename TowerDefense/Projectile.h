#pragma once
#include "GameObject.h"
#include <SFML/Graphics.hpp>

class Projectile : public GameObject {
private:
    sf::Vector2f targetPos;
    float speed;
    int damage;
    float splashRadius;
    bool reachedTarget;
    sf::CircleShape shape;

public:
    Projectile(sf::Vector2f startPos, sf::Vector2f tPos, float spd, int dmg, float splash, sf::Color color);

    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

    bool hasReached() const { return reachedTarget; }
    int getDamage() const { return damage; }
    float getSplashRadius() const { return splashRadius; }
};
#pragma once
#include "GameObject.h"
#include <SFML/Graphics.hpp>
#include "Enemy.h" // DODANE: Wymagane, aby kompilator rozpozna³ DamageType

enum class ProjectileEffect { NONE, SLOW, POISON };

class Projectile : public GameObject {
private:
    sf::Vector2f targetPos;
    float speed;
    int damage;
    float splashRadius;
    bool reachedTarget;
    sf::CircleShape shape;
    ProjectileEffect effect;
    DamageType dmgType; // Typ obra¿eñ zadawanych przez pocisk

public:
    Projectile(sf::Vector2f startPos, sf::Vector2f tPos, float spd, int dmg, float splash, sf::Color color, ProjectileEffect eff = ProjectileEffect::NONE, DamageType dType = DamageType::NORMAL);

    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

    bool hasReached() const { return reachedTarget; }
    int getDamage() const { return damage; }
    float getSplashRadius() const { return splashRadius; }
    ProjectileEffect getEffect() const { return effect; }
    DamageType getDamageType() const { return dmgType; }
};
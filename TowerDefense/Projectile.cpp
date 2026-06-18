#include "Projectile.h"
#include <cmath>

Projectile::Projectile(sf::Vector2f startPos, sf::Vector2f tPos, float spd, int dmg, float splash, sf::Color color, ProjectileEffect eff, DamageType dType)
    : targetPos(tPos), speed(spd), damage(dmg), splashRadius(splash), reachedTarget(false), effect(eff), dmgType(dType)
{
    position = startPos;

    // Dopasowanie rozmiaru w zale ci od tego, czy pocisk ma obra enia obszarowe (np. Armata)
    // TUTEJ BRAKOWA O TEJ LINIJKI:
    float radius = (splash > 20.f) ? 8.f : 4.f;
    shape.setRadius(radius);

    shape.setOrigin({ radius, radius });
    shape.setPosition(position);
    shape.setFillColor(color);
}

void Projectile::update(float dt) {
    if (reachedTarget) return;

    sf::Vector2f dir = targetPos - position;
    float distance = std::sqrt(dir.x * dir.x + dir.y * dir.y);

    if (distance <= speed * dt) {
        position = targetPos; // Osi to cel
        reachedTarget = true;
    }
    else {
        dir /= distance; // Normalizacja wektora kierunkowego
        position += dir * speed * dt;
    }
    shape.setPosition(position);
}

void Projectile::draw(sf::RenderWindow& window) {
    window.draw(shape);
}
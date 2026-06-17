#pragma once
#include "GameObject.h"
#include <vector>
#include <functional>
#include <cmath>

class Enemy : public GameObject {
protected:
    float speed;
    int health;
    int maxHealth; // Przechowuje HP pocz¹tkowe do skali paska ¿ycia
    int pointValue;
    int damageToBase; // Zmienna przechowuj¹ca obra¿enia zadawane bazie

    std::function<void(int)> onDeathCallback;

    // Zmienne œledz¹ce pozycjê na œcie¿ce
    size_t currentTargetPoint = 0;
    bool reachedEnd = false;

    // Kszta³t zastêpczy do testów
    sf::CircleShape shape;

    // Prostok¹ty odrysowuj¹ce pasek zdrowia
    sf::RectangleShape healthBarBg;
    sf::RectangleShape healthBarFill;

public:
    // Konstruktor przyjmuje teraz 4 argumenty (w tym damage)
    Enemy(float startSpeed, int startHealth, int points, int damage);
    virtual ~Enemy() = default;

    void setOnDeathCallback(std::function<void(int)> cb);
    void moveAlongPath(const std::vector<sf::Vector2f>& path, float dt);
    void takeDamage(int damage);

    bool isDead() const;

    // Zmienione, aby przyjmowa³o argument œcie¿ki
    bool hasReachedEnd(const std::vector<sf::Vector2f>& path) const;

    // Gettery wymagane przez WaveManager
    int getDamageToBase() const { return damageToBase; }
    int getPointValue() const { return pointValue; }

    virtual void update(float dt) override;
    virtual void draw(sf::RenderWindow& window) override;
};

// Klasa Goblin dziedzicz¹ca po Enemy
class Goblin : public Enemy {
public:
    Goblin(sf::Vector2f startPos);
    void update(float dt) override;
};
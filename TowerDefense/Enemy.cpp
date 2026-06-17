#include "Enemy.h"
#include <algorithm> // Dla std::max

// --- Implementacja klasy bazowej Enemy ---

Enemy::Enemy(float startSpeed, int startHealth, int points, int damage)
    : speed(startSpeed), health(startHealth), maxHealth(startHealth), pointValue(points), damageToBase(damage)
{
    // Inicjalizacja domyœlnego kszta³tu (np. czerwone ko³o)
    shape.setRadius(15.f);
    shape.setFillColor(sf::Color::Red);
    // SFML 3 wymaga nawiasów klamrowych dla sf::Vector2f
    shape.setOrigin({ 15.f, 15.f });

    // Inicjalizacja paska ¿ycia
    healthBarBg.setSize({ 30.f, 5.f });
    healthBarBg.setFillColor(sf::Color::Red);
    healthBarBg.setOrigin({ 15.f, 25.f }); // Przesuniête wy¿ej, nad wroga

    healthBarFill.setSize({ 30.f, 5.f });
    healthBarFill.setFillColor(sf::Color::Green);
    healthBarFill.setOrigin({ 15.f, 25.f }); // Przesuniête wy¿ej, nad wroga
}

void Enemy::setOnDeathCallback(std::function<void(int)> cb) {
    onDeathCallback = cb;
}

void Enemy::takeDamage(int damage) {
    health -= damage;

    // Zmniejszamy szerokoœæ zielonego paska na podstawie pozosta³ego % zdrowia
    float hpPercent = std::max(0.0f, static_cast<float>(health) / maxHealth);
    healthBarFill.setSize({ 30.f * hpPercent, 5.f });

    if (health <= 0 && onDeathCallback) {
        onDeathCallback(pointValue);
        onDeathCallback = nullptr; // Zabezpieczenie przed podwójnym dodaniem z³ota
    }
}

bool Enemy::isDead() const {
    return health <= 0;
}

bool Enemy::hasReachedEnd(const std::vector<sf::Vector2f>& path) const {
    // Jeœli nie ma œcie¿ki lub dotarliœmy do jej koñca
    if (path.empty()) return true;
    return currentTargetPoint >= path.size();
}

void Enemy::moveAlongPath(const std::vector<sf::Vector2f>& path, float dt) {
    if (currentTargetPoint >= path.size()) return;

    sf::Vector2f target = path[currentTargetPoint];
    sf::Vector2f dir = target - position; // 'position' pochodzi z GameObject
    float distance = std::sqrt(dir.x * dir.x + dir.y * dir.y);

    if (distance <= speed * dt) {
        position = target;
        currentTargetPoint++;
    }
    else {
        dir /= distance; // Normalizacja wektora
        position += dir * speed * dt;
    }

    // Aktualizacja pozycji wyœwietlanego kszta³tu
    shape.setPosition(position);

    // Aktualizacja pozycji pasków ¿ycia
    healthBarBg.setPosition(position);
    healthBarFill.setPosition(position);
}

void Enemy::update(float dt) {
    // Miejsce na bazow¹ logikê aktualizacji
}

void Enemy::draw(sf::RenderWindow& window) {
    window.draw(shape);
    window.draw(healthBarBg);
    window.draw(healthBarFill);
}


// --- Implementacja konkretnego przeciwnika: Goblin ---

Goblin::Goblin(sf::Vector2f startPos)
// Wywo³anie konstruktora bazowego: Prêdkoœæ 100, 50 HP, 10 Punktów/Z³ota, 5 DMG do bazy
    : Enemy(100.f, 50, 10, 5)
{
    position = startPos;
    shape.setPosition(position);
    shape.setFillColor(sf::Color::Green); // Gobliny niech bêd¹ zielone
}

void Goblin::update(float dt) {
    Enemy::update(dt);
    // Tutaj mo¿esz dodaæ specyficzn¹ logikê Goblina w przysz³oœci
}
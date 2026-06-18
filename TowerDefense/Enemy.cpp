#include "Enemy.h"
#include <algorithm>

// --- KLASA BAZOWA ENEMY ---
Enemy::Enemy(float startSpeed, int startHealth, int points, int damage)
    : baseSpeed(startSpeed), speed(startSpeed), health(startHealth), maxHealth(startHealth), pointValue(points), damageToBase(damage)
{
    shape.setRadius(15.f);
    shape.setFillColor(sf::Color::Red);
    originalColor = sf::Color::Red;
    shape.setOrigin({ 15.f, 15.f });

    healthBarBg.setSize({ 30.f, 5.f });
    healthBarBg.setFillColor(sf::Color::Red);
    healthBarBg.setOrigin({ 15.f, 25.f });

    healthBarFill.setSize({ 30.f, 5.f });
    healthBarFill.setFillColor(sf::Color::Green);
    healthBarFill.setOrigin({ 15.f, 25.f });
}

void Enemy::setOnDeathCallback(std::function<void(int)> cb) {
    onDeathCallback = cb;
}

void Enemy::takeDamage(int damage, DamageType type) {
    health -= damage;
    float hpPercent = std::max(0.0f, static_cast<float>(health) / maxHealth);
    healthBarFill.setSize({ 30.f * hpPercent, 5.f });

    if (health <= 0 && onDeathCallback) {
        onDeathCallback(pointValue);
        onDeathCallback = nullptr;
    }
}

void Enemy::applySlow(float factor, float duration) {
    if (slowTimer <= 0.f) {
        speed = baseSpeed * factor;
        shape.setFillColor(sf::Color(100, 100, 255)); // Lodowy kolor
    }
    slowTimer = std::max(slowTimer, duration);
}

void Enemy::applyPoison(int dps, float duration) {
    poisonDps = dps;
    poisonTimer = std::max(poisonTimer, duration);
    shape.setFillColor(sf::Color(150, 50, 150)); // Kolor trucizny
}

void Enemy::update(float dt) {
    // Zarz¹dzanie spowolnieniem
    if (slowTimer > 0.f) {
        slowTimer -= dt;
        if (slowTimer <= 0.f) {
            speed = baseSpeed;
            shape.setFillColor(originalColor); // Powrót do normy
        }
    }

    // Zarz¹dzanie trucizn¹
    if (poisonTimer > 0.f) {
        poisonTimer -= dt;
        poisonTickTimer += dt;
        if (poisonTickTimer >= 1.0f) {
            takeDamage(poisonDps, DamageType::POISON); // Trucizna bije co sekundê
            poisonTickTimer = 0.f;
        }
        if (poisonTimer <= 0.f && slowTimer <= 0.f) {
            shape.setFillColor(originalColor);
        }
    }
}

bool Enemy::isDead() const { return health <= 0; }
bool Enemy::hasReachedEnd(const std::vector<sf::Vector2f>& path) const {
    if (path.empty()) return true;
    return currentTargetPoint >= path.size();
}

void Enemy::moveAlongPath(const std::vector<sf::Vector2f>& path, float dt) {
    if (currentTargetPoint >= path.size()) return;
    sf::Vector2f target = path[currentTargetPoint];
    sf::Vector2f dir = target - position;
    float distance = std::sqrt(dir.x * dir.x + dir.y * dir.y);

    if (distance <= speed * dt) {
        position = target;
        currentTargetPoint++;
    }
    else {
        dir /= distance;
        position += dir * speed * dt;
    }

    shape.setPosition(position);
    healthBarBg.setPosition(position);
    healthBarFill.setPosition(position);
}

void Enemy::draw(sf::RenderWindow& window) {
    window.draw(shape);
    window.draw(healthBarBg);
    window.draw(healthBarFill);
}

// ==========================================
// --- IMPLEMENTACJE KONKRETNYCH WROGÓW ---
// ==========================================

// 1. Goblin (Podstawowy)
Goblin::Goblin(sf::Vector2f startPos) : Enemy(100.f, 50, 10, 5) {
    position = startPos; shape.setPosition(position);
    originalColor = sf::Color::Green; shape.setFillColor(originalColor);
}

// 2. ArmoredEnemy (Zbroja odbijaj¹ca zwyk³e ciosy)
ArmoredEnemy::ArmoredEnemy(sf::Vector2f startPos) : Enemy(70.f, 150, 25, 10) {
    position = startPos; shape.setPosition(position);
    originalColor = sf::Color(128, 128, 128); // Szary (Stal)
    shape.setFillColor(originalColor);
    shape.setOutlineThickness(3.f);
    shape.setOutlineColor(sf::Color::White); // Bia³y pancerz wizualnie
}
void ArmoredEnemy::takeDamage(int damage, DamageType type) {
    if (!armorBroken) {
        // Zbroja ³amie siê tylko od Armaty i Pioruna
        if (type == DamageType::CANNON || type == DamageType::LIGHTNING) {
            armorBroken = true;
            shape.setOutlineThickness(0.f); // Traci wizualny pancerz
            originalColor = sf::Color(200, 100, 0); // Zmienia kolor na "odkryty"
            shape.setFillColor(originalColor);
            Enemy::takeDamage(damage, type);
        }
        else if (type == DamageType::POISON) {
            Enemy::takeDamage(damage, type); // Trucizna prze¿era pancerz
        }
        else {
            // Strza³y i magia obijaj¹ siê od zbroi zadaj¹c tylko 1 dmg
            Enemy::takeDamage(1, type);
        }
    }
    else {
        Enemy::takeDamage(damage, type); // Pancerz zniszczony, pe³ne obra¿enia
    }
}

// 3. MaskedEnemy (Niewidzialny dla wie¿, dopóki Radar/Mag go nie ujawni)
MaskedEnemy::MaskedEnemy(sf::Vector2f startPos) : Enemy(130.f, 60, 20, 5) {
    position = startPos; shape.setPosition(position);
    originalColor = sf::Color(50, 50, 50, 180); // Ciemny, pó³przezroczysty
    shape.setFillColor(originalColor);
}
void MaskedEnemy::takeDamage(int damage, DamageType type) {
    if (type == DamageType::MAGIC && !maskBroken) {
        maskBroken = true; // Magia na sta³e zrywa maskê!
        originalColor = sf::Color(180, 0, 180);
        shape.setFillColor(originalColor);
        shape.setOutlineThickness(0.f);
    }
    Enemy::takeDamage(damage, type);
}
bool MaskedEnemy::isTargetable() const {
    return maskBroken || revealedByRadar;
}
void MaskedEnemy::setRevealed(bool revealed) {
    revealedByRadar = revealed;
    if (!maskBroken) {
        if (revealed) {
            shape.setOutlineThickness(2.f);
            shape.setOutlineColor(sf::Color::Cyan); // B³êkitny obrys radaru
        }
        else {
            shape.setOutlineThickness(0.f);
        }
    }
}
void MaskedEnemy::update(float dt) {
    Enemy::update(dt);
    revealedByRadar = false; // Reset radaru co klatkê (Radar wymusza true co update)
}

// 4. BossEnemy (Wielki i powolny)
BossEnemy::BossEnemy(sf::Vector2f startPos) : Enemy(40.f, 1500, 200, 50) {
    position = startPos; shape.setPosition(position);
    shape.setRadius(25.f);
    shape.setOrigin({ 25.f, 25.f });
    originalColor = sf::Color(139, 0, 0); // Bordowy boss
    shape.setFillColor(originalColor);

    // Przesuniêcie paska zdrowia wy¿ej dla wiêkszego modelu
    healthBarBg.setOrigin({ 15.f, 35.f });
    healthBarFill.setOrigin({ 15.f, 35.f });
}

// 5. FastEnemy (Ma³y i niezwykle szybki)
FastEnemy::FastEnemy(sf::Vector2f startPos) : Enemy(250.f, 30, 10, 2) {
    position = startPos; shape.setPosition(position);
    shape.setRadius(10.f);
    shape.setOrigin({ 10.f, 10.f });
    originalColor = sf::Color::Yellow;
    shape.setFillColor(originalColor);

    healthBarBg.setOrigin({ 15.f, 20.f });
    healthBarFill.setOrigin({ 15.f, 20.f });
}

// 6. TankEnemy (Czo³g, na którego super dzia³a trucizna)
TankEnemy::TankEnemy(sf::Vector2f startPos) : Enemy(55.f, 350, 40, 15) {
    position = startPos; shape.setPosition(position);
    shape.setRadius(20.f);
    shape.setOrigin({ 20.f, 20.f });
    originalColor = sf::Color(139, 69, 19); // Br¹zowy 
    shape.setFillColor(originalColor);

    healthBarBg.setOrigin({ 15.f, 30.f });
    healthBarFill.setOrigin({ 15.f, 30.f });
}
void TankEnemy::applyPoison(int dps, float duration) {
    // Obrywa PODWÓJNIE mocno od trucizny przez d³u¿szy czas!
    Enemy::applyPoison(dps * 2, duration + 2.0f);
}
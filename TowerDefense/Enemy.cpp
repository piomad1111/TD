#include "Enemy.h"
#include <algorithm>
#include <ranges>
#include "TextureManager.h" // POTRZEBNE DO WGRYWANIA TEKSTURY

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
        shape.setFillColor(sf::Color(100, 100, 255)); // Lodowy kolor (kszta³t)
        if (sprite) sprite->setColor(sf::Color(100, 100, 255)); // Lodowy kolor (tekstura)
    }
    slowTimer = std::max(slowTimer, duration);
}

void Enemy::applyPoison(int dps, float duration) {
    poisonDps = dps;
    poisonTimer = std::max(poisonTimer, duration);
    shape.setFillColor(sf::Color(150, 50, 150)); // Kolor trucizny (kszta³t)
    if (sprite) sprite->setColor(sf::Color(150, 50, 150)); // Kolor trucizny (tekstura)
}

void Enemy::update(float dt) {
    // Zarz¹dzanie spowolnieniem
    if (slowTimer > 0.f) {
        slowTimer -= dt;
        if (slowTimer <= 0.f) {
            speed = baseSpeed;
            shape.setFillColor(originalColor);
            if (sprite) sprite->setColor(originalColor); // Powrót do normy
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
            if (sprite) sprite->setColor(originalColor); // Powrót do normy
        }
    }

    // --- LOGIKA ANIMACJI DLA SPRITE'ÓW ---
    if (sprite) {
        animationTimer += dt;
        // Animacja zwalnia, jeœli potwór zosta³ spowolniony wie¿¹ lodow¹!
        float adjustedTimePerFrame = timePerFrame * (baseSpeed / speed);

        if (animationTimer >= adjustedTimePerFrame) {
            animationTimer = 0.f;
            currentFrame = (currentFrame + 1) % numFrames;

            // SFML 3: Jawne u¿ycie sf::Vector2i zamiast list inicjalizacyjnych {}, zapobiega C2440
            sprite->setTextureRect(sf::IntRect(
                sf::Vector2i(currentFrame * frameSize.x, currentDirectionRow * frameSize.y),
                frameSize
            ));
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

    if (distance > 0.f) {
        int newDirectionRow = currentDirectionRow;

        // Okreœlenie, w któr¹ stronê wróg aktualnie pod¹¿a i zmiana wiersza animacji
        if (std::abs(dir.x) > std::abs(dir.y)) {
            // Przewa¿a ruch poziomy
            if (dir.x > 0) newDirectionRow = 1; // W prawo
            else           newDirectionRow = 2; // W lewo
        }
        else {
            // Przewa¿a ruch pionowy
            if (dir.y > 0) newDirectionRow = 0; // W dó³
            else           newDirectionRow = 3; // W górê
        }

        // Natychmiastowa aktualizacja tekstury przy zmianie kierunku
        if (newDirectionRow != currentDirectionRow) {
            currentDirectionRow = newDirectionRow;
            if (sprite) {
                sprite->setTextureRect(sf::IntRect(
                    sf::Vector2i(currentFrame * frameSize.x, currentDirectionRow * frameSize.y),
                    frameSize
                ));
            }
        }
    }

    if (distance <= speed * dt) {
        position = target;
        currentTargetPoint++;
    }
    else {
        dir /= distance;
        position += dir * speed * dt;
    }

    shape.setPosition(position);
    if (sprite) sprite->setPosition(position);
    healthBarBg.setPosition(position);
    healthBarFill.setPosition(position);
}

void Enemy::draw(sf::RenderWindow& window) {
    if (sprite) {
        window.draw(*sprite); // Jeœli potwór ma wgran¹ teksturê, rysujemy j¹
    }
    else {
        window.draw(shape);  // Fallback do kó³ka z geometrii dla starszych potworów
    }
    window.draw(healthBarBg);
    window.draw(healthBarFill);
}

// ==========================================
// --- IMPLEMENTACJE KONKRETNYCH WROGÓW ---
// ==========================================

// 1. Goblin (Podstawowy - ZANIMOWANY)
Goblin::Goblin(sf::Vector2f startPos) : Enemy(100.f, 50, 10, 5) {
    position = startPos;
    shape.setPosition(position);

    originalColor = sf::Color::White;
    shape.setFillColor(originalColor);

    sf::Texture& tex = TextureManager::getInstance().get("goblin2.png");
    sprite.emplace(tex);
    sprite->setColor(originalColor);

    numFrames = 3;
    timePerFrame = 0.15f;
    currentFrame = 0;
    currentDirectionRow = 0;

    sf::Vector2u texSize = tex.getSize();
    frameSize = sf::Vector2i(texSize.x / 3, texSize.y / 4);

    sprite->setOrigin(sf::Vector2f(frameSize.x / 2.f, frameSize.y / 2.f));
    sprite->setPosition(position);
    sprite->setTextureRect(sf::IntRect(sf::Vector2i(0, 0), frameSize));

    float desiredHeight = 45.f;
    sprite->setScale(sf::Vector2f(desiredHeight / frameSize.y, desiredHeight / frameSize.y));
}

// 2. ArmoredEnemy (Zbroja odbijaj¹ca zwyk³e ciosy)
ArmoredEnemy::ArmoredEnemy(sf::Vector2f startPos) : Enemy(70.f, 150, 25, 10) {
    position = startPos;
    shape.setPosition(position);

    // Szary kolor bazowy, imituj¹cy zakuty pancerz
    originalColor = sf::Color(180, 180, 180);
    shape.setFillColor(originalColor);
    shape.setOutlineThickness(3.f);
    shape.setOutlineColor(sf::Color::White);

    sf::Texture& tex = TextureManager::getInstance().get("armor1.png");
    sprite.emplace(tex);
    sprite->setColor(originalColor);

    numFrames = 3;
    timePerFrame = 0.15f;
    currentFrame = 0;
    currentDirectionRow = 0;

    sf::Vector2u texSize = tex.getSize();
    frameSize = sf::Vector2i(texSize.x / 3, texSize.y / 4);

    sprite->setOrigin(sf::Vector2f(frameSize.x / 2.f, frameSize.y / 2.f));
    sprite->setPosition(position);
    sprite->setTextureRect(sf::IntRect(sf::Vector2i(0, 0), frameSize));

    float desiredHeight = 50.f; // Nieco wiêkszy od Goblina
    sprite->setScale(sf::Vector2f(desiredHeight / frameSize.y, desiredHeight / frameSize.y));
}

void ArmoredEnemy::takeDamage(int damage, DamageType type) {
    if (!armorBroken) {
        if (type == DamageType::CANNON || type == DamageType::LIGHTNING) {
            armorBroken = true;
            shape.setOutlineThickness(0.f);
            originalColor = sf::Color::White;
            shape.setFillColor(originalColor);

            if (sprite) {
                sprite->setColor(originalColor);

                // --- DODANA LOGIKA ZMIANY W GOBLINA ---
                sf::Texture& goblinTex = TextureManager::getInstance().get("goblin2.png");
                sprite->setTexture(goblinTex);

                // Przeliczamy ponownie rozmiar klatki dla tekstury Goblina
                sf::Vector2u texSize = goblinTex.getSize();
                frameSize = sf::Vector2i(texSize.x / 3, texSize.y / 4);
                sprite->setOrigin(sf::Vector2f(frameSize.x / 2.f, frameSize.y / 2.f));

                // Zmniejszamy rozmiar wyœwietlanego sprite'a z 50.f (Armored) do 45.f (Goblin)
                float desiredHeight = 45.f;
                sprite->setScale(sf::Vector2f(desiredHeight / frameSize.y, desiredHeight / frameSize.y));
            }

            Enemy::takeDamage(damage, type);
        }
        else if (type == DamageType::POISON) {
            Enemy::takeDamage(damage, type);
        }
        else {
            Enemy::takeDamage(1, type);
        }
    }
    else {
        Enemy::takeDamage(damage, type);
    }
}

// 3. MaskedEnemy (Niewidzialny dla wie¿, dopóki Radar/Mag go nie ujawni)
MaskedEnemy::MaskedEnemy(sf::Vector2f startPos) : Enemy(130.f, 60, 20, 5) {
    position = startPos;
    shape.setPosition(position);

    // Niewidzialny - pó³przezroczysty
    originalColor = sf::Color(255, 255, 255, 120);
    shape.setFillColor(originalColor);

    sf::Texture& tex = TextureManager::getInstance().get("goblin2.png");
    sprite.emplace(tex);
    sprite->setColor(originalColor); // Bêdzie pó³przezroczysty!

    numFrames = 3;
    timePerFrame = 0.12f; // Szybszy krok
    currentFrame = 0;
    currentDirectionRow = 0;

    sf::Vector2u texSize = tex.getSize();
    frameSize = sf::Vector2i(texSize.x / 3, texSize.y / 4);

    sprite->setOrigin(sf::Vector2f(frameSize.x / 2.f, frameSize.y / 2.f));
    sprite->setPosition(position);
    sprite->setTextureRect(sf::IntRect(sf::Vector2i(0, 0), frameSize));

    float desiredHeight = 45.f;
    sprite->setScale(sf::Vector2f(desiredHeight / frameSize.y, desiredHeight / frameSize.y));
}

void MaskedEnemy::takeDamage(int damage, DamageType type) {
    if (type == DamageType::MAGIC && !maskBroken) {
        maskBroken = true;
        originalColor = sf::Color::White; // Ca³kowicie odkryty
        shape.setFillColor(originalColor);
        if (sprite) sprite->setColor(originalColor);
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
            shape.setOutlineColor(sf::Color::Cyan);
            if (sprite) sprite->setColor(sf::Color(150, 255, 255, 200)); // B³êkitne podœwietlenie
        }
        else {
            shape.setOutlineThickness(0.f);
            if (sprite) sprite->setColor(originalColor); // Powrót do bycia duchem
        }
    }
}

void MaskedEnemy::update(float dt) {
    Enemy::update(dt);
    revealedByRadar = false;
}


// 4. BossEnemy (Wielki i powolny)
BossEnemy::BossEnemy(sf::Vector2f startPos) : Enemy(40.f, 1500, 200, 50) {
    position = startPos;
    shape.setPosition(position);
    shape.setRadius(25.f);
    shape.setOrigin({ 25.f, 25.f });

    originalColor = sf::Color::White;
    shape.setFillColor(originalColor);

    sf::Texture& tex = TextureManager::getInstance().get("goblin2.png");
    sprite.emplace(tex);
    sprite->setColor(originalColor);

    numFrames = 3;
    timePerFrame = 0.25f; // Ciê¿ki, wolny krok
    currentFrame = 0;
    currentDirectionRow = 0;

    sf::Vector2u texSize = tex.getSize();
    frameSize = sf::Vector2i(texSize.x / 3, texSize.y / 4);

    sprite->setOrigin(sf::Vector2f(frameSize.x / 2.f, frameSize.y / 2.f));
    sprite->setPosition(position);
    sprite->setTextureRect(sf::IntRect(sf::Vector2i(0, 0), frameSize));

    float desiredHeight = 90.f; // Ogromny model bossa
    sprite->setScale(sf::Vector2f(desiredHeight / frameSize.y, desiredHeight / frameSize.y));

    // Przesuniêcie paska zdrowia wy¿ej dla wiêkszego modelu
    healthBarBg.setOrigin({ 15.f, 45.f });
    healthBarFill.setOrigin({ 15.f, 45.f });
}


// 5. FastEnemy (Ma³y i niezwykle szybki)
FastEnemy::FastEnemy(sf::Vector2f startPos) : Enemy(250.f, 30, 10, 2) {
    position = startPos;
    shape.setPosition(position);
    shape.setRadius(10.f);
    shape.setOrigin({ 10.f, 10.f });

    originalColor = sf::Color::White;
    shape.setFillColor(originalColor);

    sf::Texture& tex = TextureManager::getInstance().get("fast1.png");
    sprite.emplace(tex);
    sprite->setColor(originalColor);

    numFrames = 3;
    timePerFrame = 0.08f; // Szybki bieg
    currentFrame = 0;
    currentDirectionRow = 0;

    sf::Vector2u texSize = tex.getSize();
    frameSize = sf::Vector2i(texSize.x / 3, texSize.y / 4);

    sprite->setOrigin(sf::Vector2f(frameSize.x / 2.f, frameSize.y / 2.f));
    sprite->setPosition(position);
    sprite->setTextureRect(sf::IntRect(sf::Vector2i(0, 0), frameSize));

    float desiredHeight = 35.f; // Ma³y przeciwnik
    sprite->setScale(sf::Vector2f(desiredHeight / frameSize.y, desiredHeight / frameSize.y));

    healthBarBg.setOrigin({ 15.f, 20.f });
    healthBarFill.setOrigin({ 15.f, 20.f });
}

// 6. TankEnemy (Czo³g, na którego super dzia³a trucizna)
TankEnemy::TankEnemy(sf::Vector2f startPos) : Enemy(55.f, 350, 40, 15) {
    position = startPos;
    shape.setPosition(position);
    shape.setRadius(20.f);
    shape.setOrigin({ 20.f, 20.f });

    originalColor = sf::Color::White;
    shape.setFillColor(originalColor);

    sf::Texture& tex = TextureManager::getInstance().get("tank1.png");
    sprite.emplace(tex);
    sprite->setColor(originalColor);

    numFrames = 3;
    timePerFrame = 0.2f; // Powolny chód
    currentFrame = 0;
    currentDirectionRow = 0;

    sf::Vector2u texSize = tex.getSize();
    frameSize = sf::Vector2i(texSize.x / 3, texSize.y / 4);

    sprite->setOrigin(sf::Vector2f(frameSize.x / 2.f, frameSize.y / 2.f));
    sprite->setPosition(position);
    sprite->setTextureRect(sf::IntRect(sf::Vector2i(0, 0), frameSize));

    float desiredHeight = 65.f; // Doœæ du¿y
    sprite->setScale(sf::Vector2f(desiredHeight / frameSize.y, desiredHeight / frameSize.y));

    healthBarBg.setOrigin({ 15.f, 35.f });
    healthBarFill.setOrigin({ 15.f, 35.f });
}

void TankEnemy::applyPoison(int dps, float duration) {
    Enemy::applyPoison(dps * 2, duration + 2.0f);
}
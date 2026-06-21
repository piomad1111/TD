#pragma once
#include "GameObject.h"
#include <vector>
#include <functional>
#include <cmath>
#include <optional>
#include <SFML/Graphics.hpp>

// Typy obra¿eñ pomog¹ rozpoznaæ, czy strzela armata, mag, czy np. trucizna
enum class DamageType {
    NORMAL,    // Np. ³ucznik
    CANNON,    // Armata (przebija zbrojê)
    LIGHTNING, // B³yskawica (przebija zbrojê)
    MAGIC,     // Mag (zdejmuje maskê)
    POISON     // Trucizna
};

class Enemy : public GameObject {
protected:
    float baseSpeed;
    float speed;
    int health;
    int maxHealth;
    int pointValue;
    int damageToBase;

    // Zmienne do statusów (Buffy/Debuffy)
    float slowTimer = 0.f;
    float poisonTimer = 0.f;
    float poisonTickTimer = 0.f;
    int poisonDps = 0;
    sf::Color originalColor; // Zapisujemy kolor wroga, aby po spowolnieniu wróci³ do normy

    std::function<void(int)> onDeathCallback;
    size_t currentTargetPoint = 0;

    // Elementy wizualne
    sf::CircleShape shape;

    // SFML 3: Sprite nie ma domyœlnego konstruktora, u¿ywamy opcjonalnego wariantu
    std::optional<sf::Sprite> sprite;

    // Zmienne obs³uguj¹ce siatkê animacji
    sf::Vector2i frameSize;   // Wymiary pojedynczej klatki na teksturze
    int currentFrame = 0;     // Aktualna kolumna
    int currentDirectionRow = 0; // Aktualny wiersz (kierunek)
    int numFrames = 1;        // Iloœæ klatek animacji

    // Parametry czasowe animacji
    float animationTimer = 0.f;
    float timePerFrame = 0.1f;

    sf::RectangleShape healthBarBg;
    sf::RectangleShape healthBarFill;

public:
    Enemy(float startSpeed, int startHealth, int points, int damage);
    virtual ~Enemy() = default;

    void setOnDeathCallback(std::function<void(int)> cb);
    void moveAlongPath(const std::vector<sf::Vector2f>& path, float dt);

    virtual void takeDamage(int damage, DamageType type = DamageType::NORMAL);

    // Funkcje aplikuj¹ce efekty
    virtual void applySlow(float factor, float duration);
    virtual void applyPoison(int dps, float duration);

    bool isDead() const;
    bool hasReachedEnd(const std::vector<sf::Vector2f>& path) const;

    int getDamageToBase() const { return damageToBase; }
    int getPointValue() const { return pointValue; }

    virtual bool isTargetable() const { return true; }
    virtual void setRevealed(bool revealed) {}

    virtual void update(float dt) override;
    virtual void draw(sf::RenderWindow& window) override;
};

// --- PODSTAWOWY WRÓG (ZANIMOWANY) ---
class Goblin : public Enemy {
public:
    Goblin(sf::Vector2f startPos);
};

// --- POZOSTALI PRZECIWNICY ---

class ArmoredEnemy : public Enemy {
private:
    bool armorBroken = false;
public:
    ArmoredEnemy(sf::Vector2f startPos);
    void takeDamage(int damage, DamageType type = DamageType::NORMAL) override;
};

class MaskedEnemy : public Enemy {
private:
    bool maskBroken = false;
    bool revealedByRadar = false;
public:
    MaskedEnemy(sf::Vector2f startPos);
    void takeDamage(int damage, DamageType type = DamageType::NORMAL) override;
    bool isTargetable() const override;
    void setRevealed(bool revealed) override;
    void update(float dt) override;
};

class BossEnemy : public Enemy {
public:
    BossEnemy(sf::Vector2f startPos);
};

class FastEnemy : public Enemy {
public:
    FastEnemy(sf::Vector2f startPos);
};

class TankEnemy : public Enemy {
public:
    TankEnemy(sf::Vector2f startPos);
    void applyPoison(int dps, float duration) override;
};
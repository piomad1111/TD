#pragma once
#include "GameObject.h"
#include <vector>
#include <functional>
#include <cmath>
#include <SFML/Graphics.hpp>

// NOWE: Typy obra¿eñ pomog¹ rozpoznaæ, czy strzela armata, mag, czy np. trucizna
enum class DamageType {
    NORMAL,    // Np. £ucznik
    CANNON,    // Armata (przebija zbrojê)
    LIGHTNING, // B³yskawica (przebija zbrojê)
    MAGIC,     // Mag (zdejmuje maskê)
    POISON     // Trucizna
};

class Enemy : public GameObject {
protected:
    float baseSpeed;     // Zapisujemy prêdkoœæ bazow¹, by móc do niej wróciæ po spowolnieniu
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

    sf::CircleShape shape;
    sf::RectangleShape healthBarBg;
    sf::RectangleShape healthBarFill;

public:
    Enemy(float startSpeed, int startHealth, int points, int damage);
    virtual ~Enemy() = default;

    void setOnDeathCallback(std::function<void(int)> cb);
    void moveAlongPath(const std::vector<sf::Vector2f>& path, float dt);

    // ZMIANA: takeDamage przyjmuje teraz DamageType (domyœlnie NORMAL)
    virtual void takeDamage(int damage, DamageType type = DamageType::NORMAL);

    // NOWE: Funkcje aplikuj¹ce efekty wywo³ywane w Game.cpp (oznaczone virtual dla Tanka)
    virtual void applySlow(float factor, float duration);
    virtual void applyPoison(int dps, float duration);

    bool isDead() const;
    bool hasReachedEnd(const std::vector<sf::Vector2f>& path) const;

    int getDamageToBase() const { return damageToBase; }
    int getPointValue() const { return pointValue; }

    // NOWE: Mechanika namierzania i ujawniania (dla Zamaskowanego wroga i Radaru)
    virtual bool isTargetable() const { return true; } // Domyœlnie ka¿dy wróg jest namierzalny
    virtual void setRevealed(bool revealed) {}         // Domyœlnie radar nic nie zmienia w zwyk³ych wrogach

    virtual void update(float dt) override;
    virtual void draw(sf::RenderWindow& window) override;
};

// --- PODSTAWOWY WRÓG ---
class Goblin : public Enemy {
public:
    Goblin(sf::Vector2f startPos);
};

// --- NOWI PRZECIWNICY ---

// Pancerz chroni przed obra¿eniami NORMAL dopóki nie zostanie zniszczony przez CANNON lub LIGHTNING
class ArmoredEnemy : public Enemy {
private:
    bool armorBroken = false;
public:
    ArmoredEnemy(sf::Vector2f startPos);
    void takeDamage(int damage, DamageType type = DamageType::NORMAL) override;
};

// Zamaskowany wróg nie mo¿e byæ namierzony (isTargetable = false), chyba ¿e jest w zasiêgu radaru (setRevealed) lub dostanie od maga
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

// Boss: Bardzo du¿o HP, wolny, zadaje potê¿ne obra¿enia bazie
class BossEnemy : public Enemy {
public:
    BossEnemy(sf::Vector2f startPos);
};

// Szybki przeciwnik: Ma³o HP, bardzo du¿a prêdkoœæ
class FastEnemy : public Enemy {
public:
    FastEnemy(sf::Vector2f startPos);
};

// Wolny tank wra¿liwy na truciznê
class TankEnemy : public Enemy {
public:
    TankEnemy(sf::Vector2f startPos);
    // Nadpisujemy nak³adanie trucizny, by zadawa³a mu dodatkowe obra¿enia
    void applyPoison(int dps, float duration) override;
};
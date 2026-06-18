#pragma once
#include "GameObject.h"
#include <vector>
#include <functional>
#include <cmath>
#include <SFML/Graphics.hpp>

// NOWE: Typy obra  pomog  rozpozna , czy strzela armata, mag, czy np. trucizna
enum class DamageType {
    NORMAL,    // Np.  ucznik
    CANNON,    // Armata (przebija zbroj )
    LIGHTNING, // B yskawica (przebija zbroj )
    MAGIC,     // Mag (zdejmuje mask )
    POISON     // Trucizna
};

class Enemy : public GameObject {
protected:
    float baseSpeed;
    // Zapisujemy pr  bazow , by m c do niej wr  po spowolnieniu
    float speed;
    int health;
    int maxHealth;
    int pointValue;
    int damageToBase;

    // Zmienne do status w (Buffy/Debuffy)
    float slowTimer = 0.f;
    float poisonTimer = 0.f;
    float poisonTickTimer = 0.f;
    int poisonDps = 0;
    sf::Color originalColor; // Zapisujemy kolor wroga, aby po spowolnieniu wr  do normy

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

    // ZMIANA: takeDamage przyjmuje teraz DamageType (domy lnie NORMAL)
    virtual void takeDamage(int damage, DamageType type = DamageType::NORMAL);

    // NOWE: Funkcje aplikuj ce efekty wywo ywane w Game.cpp (oznaczone virtual dla Tanka)
    virtual void applySlow(float factor, float duration);
    virtual void applyPoison(int dps, float duration);

    bool isDead() const;
    bool hasReachedEnd(const std::vector<sf::Vector2f>& path) const;

    int getDamageToBase() const { return damageToBase; }
    int getPointValue() const { return pointValue; }

    // NOWE: Mechanika namierzania i ujawniania (dla Zamaskowanego wroga i Radaru)
    virtual bool isTargetable() const { return true; } // Domy lnie ka dy wr g jest namierzalny
    virtual void setRevealed(bool revealed) {}         // Domy lnie radar nic nie zmienia w zwyk ych wrogach

    virtual void update(float dt) override;
    virtual void draw(sf::RenderWindow& window) override;
};

// --- PODSTAWOWY WR G ---
class Goblin : public Enemy {
public:
    Goblin(sf::Vector2f startPos);
};

// --- NOWI PRZECIWNICY ---

// Pancerz chroni przed obra eniami NORMAL dop ki nie zostanie zniszczony przez CANNON lub LIGHTNING
class ArmoredEnemy : public Enemy {
private:
    bool armorBroken = false;
public:
    ArmoredEnemy(sf::Vector2f startPos);
    void takeDamage(int damage, DamageType type = DamageType::NORMAL) override;
};

// Zamaskowany wr g nie mo e by  namierzony (isTargetable = false), chyba  e jest w zasi gu radaru (setRevealed) lub dostanie od maga
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

// Boss: Bardzo du o HP, wolny, zadaje pot ne obra enia bazie
class BossEnemy : public Enemy {
public:
    BossEnemy(sf::Vector2f startPos);
};

// Szybki przeciwnik: Ma o HP, bardzo du a pr  
class FastEnemy : public Enemy {
public:
    FastEnemy(sf::Vector2f startPos);
};

// Wolny tank wra liwy na trucizn 
class TankEnemy : public Enemy {
public:
    TankEnemy(sf::Vector2f startPos);

    // Nadpisujemy nak adanie trucizny, by zadawa a mu dodatkowe obra enia
    void applyPoison(int dps, float duration) override;
};
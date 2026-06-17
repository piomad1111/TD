#pragma once
#include "GameObject.h"
#include "Enemy.h"
#include "Projectile.h" // DODANE: Czyste za³¹czenie nag³ówka pocisków
#include <vector>
#include <memory>
#include <SFML/Graphics.hpp>

// Bazowa klasa dla wszystkich typów wie¿
class Tower : public GameObject {
protected:
    float range;               // Zasiêg strza³u
    float attackCooldown;      // Czas miêdzy atakami (w sekundach)
    float timeSinceLastAttack; // Stoper odmierzaj¹cy czas do kolejnego ataku
    int damage;                // Obra¿enia zadawane przeciwnikom
    int cost;                  // Koszt budowy (do weryfikacji w sklepie)

    // NOWE: Parametry dla wystrzeliwanych pocisków
    float projSpeed;
    float projSplashRadius;
    sf::Color projColor;

    sf::RectangleShape shape;       // Reprezentacja graficzna wie¿y
    sf::CircleShape rangeIndicator; // Pó³przezroczyste ko³o pokazuj¹ce zasiêg (opcjonalne)

    // Funkcja wykorzystuj¹ca C++20 <ranges> do znalezienia najbli¿szego celu
    Enemy* findTarget(const std::vector<std::unique_ptr<Enemy>>& enemies);

public:
    // Konstruktor przyjmuje parametry okreœlaj¹ce pociski
    Tower(sf::Vector2f startPos, float tRange, float tCooldown, int tDamage, int tCost, sf::Color color, float pSpeed, float pSplash, sf::Color pColor);
    virtual ~Tower() = default;

    // Nadpisana funkcja z GameObject (logika niezale¿na od wrogów, np. animacje)
    void update(float dt) override;

    // Nadpisana funkcja rysowania
    void draw(sf::RenderWindow& window) override;

    // G³ówna logika potrzebuje dostêpu do wektora pocisków, by je generowaæ
    void updateTower(float dt, const std::vector<std::unique_ptr<Enemy>>& enemies, std::vector<std::unique_ptr<Projectile>>& projectiles);

    int getCost() const { return cost; }
    float getRange() const { return range; }
};

// --- PRZYK£ADOWE PODKLASY (Zgodne z Twoim HUD) ---

// 1. £ucznik: Du¿y zasiêg, ma³e obra¿enia, szybki atak
class ArcherTower : public Tower {
public:
    ArcherTower(sf::Vector2f startPos);
};

// 2. Mag: Œredni zasiêg, potê¿ne obra¿enia, wolny atak
class MageTower : public Tower {
public:
    MageTower(sf::Vector2f startPos);
};

// 3. Armata: Ma³y zasiêg, ogromne obra¿enia obszarowe (splash) - na razie celuje w jednego
class CannonTower : public Tower {
public:
    CannonTower(sf::Vector2f startPos);
};
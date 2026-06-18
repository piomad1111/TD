#pragma once
#include "GameObject.h"
#include "Enemy.h"
#include "Projectile.h"
#include "PlayerStats.h"
#include <vector>
#include <memory>
#include <SFML/Graphics.hpp>

class Tower : public GameObject {
protected:
    float range;
    float bonusRange; // Dodatkowy zasiêg od pobliskiego Radaru
    float attackCooldown;
    float timeSinceLastAttack;
    int damage;
    int cost;

    float projSpeed;
    float projSplashRadius;
    sf::Color projColor;
    ProjectileEffect projEffect; // Efekt pocisku wystrzeliwanego przez tê wie¿ê
    DamageType projDamageType;   // Typ obra¿eñ broni (B³yskawica, Magia, itp)

    sf::RectangleShape shape;
    sf::CircleShape rangeIndicator;

    Enemy* findTarget(const std::vector<std::unique_ptr<Enemy>>& enemies);

public:
    Tower(sf::Vector2f startPos, float tRange, float tCooldown, int tDamage, int tCost, sf::Color color, float pSpeed, float pSplash, sf::Color pColor, ProjectileEffect pEff = ProjectileEffect::NONE, DamageType dType = DamageType::NORMAL);
    virtual ~Tower() = default;

    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

    // Reset i dodawanie bonusowego zasiêgu
    void resetBonusRange() { bonusRange = 0.f; }
    void addBonusRange(float amount) { bonusRange += amount; }

    // G³ówna zaktualizowana logika aktualizacji wie¿y - Wirtualna, bo specyficzne wie¿e mog¹ dzia³aæ inaczej
    virtual void updateTower(float dt, const std::vector<std::unique_ptr<Enemy>>& enemies, std::vector<std::unique_ptr<Projectile>>& projectiles, PlayerStats& playerStats, const std::vector<std::unique_ptr<Tower>>& activeTowers);

    int getCost() const { return cost; }
    float getRange() const { return range; }
};

// --- PODKLASY WIE¯ ---

class ArcherTower : public Tower {
public: ArcherTower(sf::Vector2f startPos);
};

class MageTower : public Tower {
public: MageTower(sf::Vector2f startPos);
};

class CannonTower : public Tower {
public: CannonTower(sf::Vector2f startPos);
};

class IceTower : public Tower {
public: IceTower(sf::Vector2f startPos);
};

class PoisonTower : public Tower {
public: PoisonTower(sf::Vector2f startPos);
};

// Kopalnia nie strzela, tylko dodaje z³oto do PlayerStats
class MineTower : public Tower {
public:
    MineTower(sf::Vector2f startPos);
    void updateTower(float dt, const std::vector<std::unique_ptr<Enemy>>& enemies, std::vector<std::unique_ptr<Projectile>>& projectiles, PlayerStats& playerStats, const std::vector<std::unique_ptr<Tower>>& activeTowers) override;
};

// Radar dodaje zasiêg innym wie¿om (Nie strzela)
class RadarTower : public Tower {
public:
    RadarTower(sf::Vector2f startPos);
    void updateTower(float dt, const std::vector<std::unique_ptr<Enemy>>& enemies, std::vector<std::unique_ptr<Projectile>>& projectiles, PlayerStats& playerStats, const std::vector<std::unique_ptr<Tower>>& activeTowers) override;
};

// Piorun razi kilku przeciwników na raz (U¿ywa <ranges> do sortowania celów)
class LightningTower : public Tower {
public:
    LightningTower(sf::Vector2f startPos);
    void updateTower(float dt, const std::vector<std::unique_ptr<Enemy>>& enemies, std::vector<std::unique_ptr<Projectile>>& projectiles, PlayerStats& playerStats, const std::vector<std::unique_ptr<Tower>>& activeTowers) override;
};
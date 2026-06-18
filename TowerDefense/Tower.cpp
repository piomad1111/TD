#include "Tower.h"
#include <ranges>
#include <algorithm>
#include <iostream>

// --- KLASA BAZOWA TOWER ---

Tower::Tower(sf::Vector2f startPos, float tRange, float tCooldown, int tDamage, int tCost, sf::Color color, float pSpeed, float pSplash, sf::Color pColor, ProjectileEffect pEff, DamageType dType)
    : range(tRange), bonusRange(0.f), attackCooldown(tCooldown), timeSinceLastAttack(tCooldown), damage(tDamage), cost(tCost),
    projSpeed(pSpeed), projSplashRadius(pSplash), projColor(pColor), projEffect(pEff), projDamageType(dType)
{
    position = startPos;
    shape.setSize({ 40.f, 40.f });
    shape.setOrigin({ 20.f, 20.f });
    shape.setPosition(position);
    shape.setFillColor(color);
    shape.setOutlineThickness(2.f);
    shape.setOutlineColor(sf::Color::Black);

    rangeIndicator.setOrigin({ range, range });
    rangeIndicator.setPosition(position);
    rangeIndicator.setFillColor(sf::Color(255, 255, 255, 30));
    rangeIndicator.setOutlineThickness(1.f);
    rangeIndicator.setOutlineColor(sf::Color(255, 255, 255, 100));
}

Enemy* Tower::findTarget(const std::vector<std::unique_ptr<Enemy>>& enemies) {
    float currentTotalRange = range + bonusRange; // Uwzglêdnia bonus od radaru

    auto isValidTarget = [this, currentTotalRange](const std::unique_ptr<Enemy>& enemy) {
        if (enemy->isDead() || !enemy->isTargetable()) return false; // POMIJA ZAMASKOWANYCH (Chyba ¿e radar ich oœwietli)
        sf::Vector2f ePos = enemy->getPosition();
        float distSq = (ePos.x - position.x) * (ePos.x - position.x) + (ePos.y - position.y) * (ePos.y - position.y);
        return distSq <= (currentTotalRange * currentTotalRange);
        };

    auto enemiesInRange = enemies | std::views::filter(isValidTarget);

    if (std::ranges::empty(enemiesInRange)) return nullptr;

    auto closestEnemyIt = std::ranges::min_element(enemiesInRange, std::ranges::less{}, [this](const auto& enemy) {
        sf::Vector2f ePos = enemy->getPosition();
        return (ePos.x - position.x) * (ePos.x - position.x) + (ePos.y - position.y) * (ePos.y - position.y);
        });

    return closestEnemyIt->get();
}

void Tower::update(float dt) {
    timeSinceLastAttack += dt;
    if (timeSinceLastAttack > 0.1f) shape.setOutlineColor(sf::Color::Black);
}

void Tower::updateTower(float dt, const std::vector<std::unique_ptr<Enemy>>& enemies, std::vector<std::unique_ptr<Projectile>>& projectiles, PlayerStats& playerStats, const std::vector<std::unique_ptr<Tower>>& activeTowers) {
    update(dt);
    if (timeSinceLastAttack >= attackCooldown) {
        Enemy* target = findTarget(enemies);
        if (target) {
            projectiles.push_back(std::make_unique<Projectile>(position, target->getPosition(), projSpeed, damage, projSplashRadius, projColor, projEffect, projDamageType));
            timeSinceLastAttack = 0.f;
            shape.setOutlineColor(sf::Color::Yellow);
        }
    }
}

void Tower::draw(sf::RenderWindow& window) {
    // Dynamicznie dostosuj kó³ko zasiêgu do radaru (tylko wizualnie)
    rangeIndicator.setRadius(range + bonusRange);
    rangeIndicator.setOrigin({ range + bonusRange, range + bonusRange });
    window.draw(shape);
}

// --- KONKRETNE WIE¯E ---

ArcherTower::ArcherTower(sf::Vector2f pos)
    : Tower(pos, 200.f, 0.8f, 10, 100, sf::Color(50, 200, 50), 600.f, 0.f, sf::Color::White, ProjectileEffect::NONE, DamageType::NORMAL) {
}

MageTower::MageTower(sf::Vector2f pos)
    : Tower(pos, 150.f, 1.5f, 30, 150, sf::Color(50, 50, 200), 400.f, 25.f, sf::Color::Cyan, ProjectileEffect::NONE, DamageType::MAGIC) {
}

CannonTower::CannonTower(sf::Vector2f pos)
    : Tower(pos, 100.f, 2.5f, 80, 300, sf::Color(200, 50, 50), 200.f, 80.f, sf::Color::Black, ProjectileEffect::NONE, DamageType::CANNON) {
}

IceTower::IceTower(sf::Vector2f pos) // Nak³ada spowolnienie (SLOW)
    : Tower(pos, 180.f, 1.2f, 5, 200, sf::Color(100, 200, 255), 500.f, 0.f, sf::Color::Blue, ProjectileEffect::SLOW, DamageType::NORMAL) {
}

PoisonTower::PoisonTower(sf::Vector2f pos) // Nak³ada Truciznê (POISON)
    : Tower(pos, 160.f, 1.0f, 2, 250, sf::Color(150, 50, 150), 450.f, 0.f, sf::Color::Magenta, ProjectileEffect::POISON, DamageType::POISON) {
}

// --- WIE¯E SPECJALNE (Nadpisuj¹ UpdateTower) ---

MineTower::MineTower(sf::Vector2f pos)
    : Tower(pos, 0.f, 5.0f, 0, 400, sf::Color(255, 215, 0), 0.f, 0.f, sf::Color::Transparent) {
}

void MineTower::updateTower(float dt, const std::vector<std::unique_ptr<Enemy>>& enemies, std::vector<std::unique_ptr<Projectile>>& projectiles, PlayerStats& playerStats, const std::vector<std::unique_ptr<Tower>>& activeTowers) {
    update(dt);
    // Kopalnia nie strzela. Po na³adowaniu dodaje 25 z³ota bezpoœrednio do skarbnicy.
    if (timeSinceLastAttack >= attackCooldown) {
        playerStats.gold += 25;
        timeSinceLastAttack = 0.f;
        shape.setOutlineColor(sf::Color::Yellow);
    }
}

RadarTower::RadarTower(sf::Vector2f pos)
    : Tower(pos, 150.f, 0.f, 0, 500, sf::Color(128, 128, 128), 0.f, 0.f, sf::Color::Transparent) {
}

void RadarTower::updateTower(float dt, const std::vector<std::unique_ptr<Enemy>>& enemies, std::vector<std::unique_ptr<Projectile>>& projectiles, PlayerStats& playerStats, const std::vector<std::unique_ptr<Tower>>& activeTowers) {
    // Radar pasywnie daje +50 zasiêgu wszystkim wie¿om w swoim polu
    for (const auto& tower : activeTowers) {
        if (tower.get() == this) continue; // Nie buffuje sam siebie

        sf::Vector2f tPos = tower->getPosition();
        float distSq = (tPos.x - position.x) * (tPos.x - position.x) + (tPos.y - position.y) * (tPos.y - position.y);

        if (distSq <= range * range) {
            tower->addBonusRange(50.f);
        }
    }

    // RADAR UJAWNIA ZAMASKOWANYCH PRZECIWNIKÓW!
    for (const auto& enemy : enemies) {
        sf::Vector2f ePos = enemy->getPosition();
        float distSq = (ePos.x - position.x) * (ePos.x - position.x) + (ePos.y - position.y) * (ePos.y - position.y);
        if (distSq <= range * range) {
            enemy->setRevealed(true);
        }
    }
}

LightningTower::LightningTower(sf::Vector2f pos)
    : Tower(pos, 160.f, 2.0f, 15, 600, sf::Color::White, 10000.f /*B. Szybki pocisk*/, 0.f, sf::Color::Yellow, ProjectileEffect::NONE, DamageType::LIGHTNING) {
}

void LightningTower::updateTower(float dt, const std::vector<std::unique_ptr<Enemy>>& enemies, std::vector<std::unique_ptr<Projectile>>& projectiles, PlayerStats& playerStats, const std::vector<std::unique_ptr<Tower>>& activeTowers) {
    update(dt);

    if (timeSinceLastAttack >= attackCooldown) {
        float currentTotalRange = range + bonusRange;
        float rangeSq = currentTotalRange * currentTotalRange;

        // <ranges> - filtrujemy wrogów, którzy ¿yj¹ i s¹ w zasiêgu
        auto inRange = enemies | std::views::filter([this, rangeSq](const auto& e) {
            if (e->isDead()) return false;
            float dSq = (e->getPosition().x - position.x) * (e->getPosition().x - position.x) +
                (e->getPosition().y - position.y) * (e->getPosition().y - position.y);
            return dSq <= rangeSq;
            });

        // Przerzucamy do wektora, ¿eby móc ich posortowaæ wg. dystansu
        std::vector<Enemy*> targets;
        for (const auto& e : inRange) targets.push_back(e.get());

        if (!targets.empty()) {
            std::ranges::sort(targets, [this](Enemy* a, Enemy* b) {
                float dSqA = (a->getPosition().x - position.x) * (a->getPosition().x - position.x) + (a->getPosition().y - position.y) * (a->getPosition().y - position.y);
                float dSqB = (b->getPosition().x - position.x) * (b->getPosition().x - position.x) + (b->getPosition().y - position.y) * (b->getPosition().y - position.y);
                return dSqA < dSqB;
                });

            // Wypuszczamy "b³yskawice" do max 3 najbli¿szych celów jednoczeœnie
            int strikes = std::min(3, static_cast<int>(targets.size()));
            for (int i = 0; i < strikes; ++i) {
                projectiles.push_back(std::make_unique<Projectile>(position, targets[i]->getPosition(), projSpeed, damage, projSplashRadius, projColor, projEffect, projDamageType));
            }

            timeSinceLastAttack = 0.f;
            shape.setOutlineColor(sf::Color::Yellow);
        }
    }
}
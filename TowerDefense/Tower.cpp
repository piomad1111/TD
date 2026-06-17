#include "Tower.h"
#include <ranges>
#include <algorithm>
#include <iostream>
#include "Projectile.h" // DODANE: Nag³ówek pocisków

// --- TOWER (KLASA BAZOWA) ---

Tower::Tower(sf::Vector2f startPos, float tRange, float tCooldown, int tDamage, int tCost, sf::Color color, float pSpeed, float pSplash, sf::Color pColor)
    : range(tRange), attackCooldown(tCooldown), timeSinceLastAttack(tCooldown), damage(tDamage), cost(tCost),
    projSpeed(pSpeed), projSplashRadius(pSplash), projColor(pColor)
{
    position = startPos;

    // SFML 3: Wymaga wektorów w klamrach {}
    shape.setSize({ 40.f, 40.f });
    shape.setOrigin({ 20.f, 20.f }); // Œrodek obiektu to jego origin
    shape.setPosition(position);
    shape.setFillColor(color);
    shape.setOutlineThickness(2.f);
    shape.setOutlineColor(sf::Color::Black);

    // WskaŸnik zasiêgu (przydatny przy budowaniu i debugowaniu)
    rangeIndicator.setRadius(range);
    rangeIndicator.setOrigin({ range, range });
    rangeIndicator.setPosition(position);
    rangeIndicator.setFillColor(sf::Color(255, 255, 255, 30)); // 30 to wartoœæ Alpha (bardzo przezroczysty)
    rangeIndicator.setOutlineThickness(1.f);
    rangeIndicator.setOutlineColor(sf::Color(255, 255, 255, 100));
}

Enemy* Tower::findTarget(const std::vector<std::unique_ptr<Enemy>>& enemies) {
    // Krok 1: Definiujemy lambdê predykatu - odrzucamy martwych i tych poza zasiêgiem
    auto isValidTarget = [this](const std::unique_ptr<Enemy>& enemy) {
        if (enemy->isDead()) return false;

        sf::Vector2f ePos = enemy->getPosition();
        // OPTYMALIZACJA: Porównujemy kwadraty odleg³oœci, aby unikn¹æ std::sqrt
        float distSq = (ePos.x - position.x) * (ePos.x - position.x) +
            (ePos.y - position.y) * (ePos.y - position.y);
        return distSq <= (range * range);
        };

    // Krok 2: Tworzymy "widok" (view) przefiltrowanych wrogów u¿ywaj¹c C++20 Ranges
    auto enemiesInRange = enemies | std::views::filter(isValidTarget);

    // Jeœli nikt nie jest w zasiêgu, zwracamy nullptr
    if (enemiesInRange.empty()) {
        return nullptr;
    }

    // Krok 3: Szukamy najbli¿szego wroga w przefiltrowanym widoku
    // U¿ywamy std::ranges::min_element z lambd¹ rzutuj¹c¹ (projection)
    auto closestEnemyIt = std::ranges::min_element(enemiesInRange, std::ranges::less{}, [this](const auto& enemy) {
        sf::Vector2f ePos = enemy->getPosition();
        // Zwracamy kwadrat odleg³oœci, co pos³u¿y jako kryterium porównania (szukamy minimum)
        return (ePos.x - position.x) * (ePos.x - position.x) + (ePos.y - position.y) * (ePos.y - position.y);
        });

    // Min_element zwraca iterator na const std::unique_ptr<Enemy>, wiêc go dereferencjujemy i pobieramy nagi wskaŸnik
    return closestEnemyIt->get();
}

// USUNIÊTO: void Tower::attack(Enemy* target) { ... }

void Tower::update(float dt) {
    // Logika aktualizacji niezale¿na od wrogów (np. ch³odzenie broni)
    timeSinceLastAttack += dt;

    // Prosty efekt wizualny - po strzale (¿ó³ty obrys) wracamy powoli do czarnego
    if (timeSinceLastAttack > 0.1f) {
        shape.setOutlineColor(sf::Color::Black);
    }
}

// ZMIANA: Generowanie i wysy³anie pocisku w kierunku wroga
void Tower::updateTower(float dt, const std::vector<std::unique_ptr<Enemy>>& enemies, std::vector<std::unique_ptr<Projectile>>& projectiles) {
    update(dt); // Aktualizuj bazowe rzeczy (cooldown)

    // Jeœli wie¿a jest prze³adowana
    if (timeSinceLastAttack >= attackCooldown) {
        Enemy* target = findTarget(enemies);

        if (target) {
            // Tworzymy pocisk skierowany w aktualn¹ pozycjê namierzonego wroga
            projectiles.push_back(std::make_unique<Projectile>(position, target->getPosition(), projSpeed, damage, projSplashRadius, projColor));
            timeSinceLastAttack = 0.f; // Zresetuj stoper ataku
            shape.setOutlineColor(sf::Color::Yellow); // Prosty efekt optyczny wystrza³u
        }
    }
}

void Tower::draw(sf::RenderWindow& window) {
    // Mo¿esz zakomentowaæ rysowanie zasiêgu, by nie zaœmiecaæ ekranu w ostatecznej wersji
    // (lub rysowaæ go tylko, gdy gracz najedzie myszk¹ na wie¿ê)
    // window.draw(rangeIndicator); 

    window.draw(shape);
}

// --- KONKRETNE WIE¯E (Zgodne ze zdefiniowanym HUD) ---

// Archer: Zasiêg 200, Cooldown 0.8s, DMG 10, Koszt 100, Kolor Zielony
// Pocisk: Bardzo szybki (600), brak obszarówek (0), Kolor Bia³y
ArcherTower::ArcherTower(sf::Vector2f startPos)
    : Tower(startPos, 200.f, 0.8f, 10, 100, sf::Color(50, 200, 50), 600.f, 0.f, sf::Color::White) {
}

// Mage: Zasiêg 150, Cooldown 1.5s, DMG 30, Koszt 150, Kolor Niebieski
// Pocisk: Œredni (400), lekki splash (25), Kolor B³êkitny
MageTower::MageTower(sf::Vector2f startPos)
    : Tower(startPos, 150.f, 1.5f, 30, 150, sf::Color(50, 50, 200), 400.f, 25.f, sf::Color::Cyan) {
}

// Cannon: Zasiêg 100, Cooldown 2.5s, DMG 80, Koszt 300, Kolor Czerwony
// Pocisk: Wolny (200), du¿y splash (80), Kolor Czarny
CannonTower::CannonTower(sf::Vector2f startPos)
    : Tower(startPos, 100.f, 2.5f, 80, 300, sf::Color(200, 50, 50), 200.f, 80.f, sf::Color::Black) {
}
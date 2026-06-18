#include "WaveManager.h"
#include <iostream>
#include <thread>
#include <chrono>

WaveManager::WaveManager()
    : currentWave(0), totalWaves(10), difficultyLevel(0), isLoading(false), isWaveRunning(false), spawnTimer(0.f), enemiesToSpawn(0) {
}

WaveManager::~WaveManager() {
    if (waveLoaderFuture.valid()) {
        waveLoaderFuture.wait();
    }
}

void WaveManager::reset() {
    std::lock_guard<std::mutex> lock(waveMutex);
    activeEnemies.clear();
    currentWave = 0;
    isWaveRunning = false;
    enemiesToSpawn = 0;
}

void WaveManager::setMapData(const std::vector<sf::Vector2f>& newPath, int totalWvs, int difficulty) {
    pathPoints = newPath;
    totalWaves = totalWvs;
    difficultyLevel = difficulty;
}

void WaveManager::loadWaveDataAsync() {
    if (isLoading) return;

    isLoading = true;
    std::cout << "Rozpoczynam asynchroniczne ladowanie fali: " << currentWave + 1 << "...\n";

    waveLoaderFuture = std::async(std::launch::async, [this]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(800)); // L¿ejsza symulacja na potrzeby testów
        std::lock_guard<std::mutex> lock(waveMutex);
        std::cout << "Fala " << currentWave + 1 << " zaladowana pomyslnie!\n";
        isLoading = false;
        });
}

void WaveManager::startNextWave() {
    if (isLoading || isWaveRunning || currentWave >= totalWaves) {
        return;
    }

    std::lock_guard<std::mutex> lock(waveMutex);
    isWaveRunning = true;
    currentWave++;
    spawnTimer = 0.f;
    enemiesToSpawn = 10 + (currentWave * 5);
}

void WaveManager::updateEnemies(float dt, PlayerStats& playerStats) {
    if (!isWaveRunning) return;

    // --- SPAWNOWANIE WROGÓW ZE ZMIENIONYM BALANSEM MAP ---
    if (enemiesToSpawn > 0) {
        spawnTimer += dt;
        if (spawnTimer >= 1.0f) {
            std::lock_guard<std::mutex> lock(waveMutex);

            sf::Vector2f startPos = pathPoints.empty() ? sf::Vector2f(0.f, 0.f) : pathPoints.front();
            std::unique_ptr<Enemy> newEnemy;

            if (enemiesToSpawn == 1 && currentWave % 5 == 0) {
                newEnemy = std::make_unique<BossEnemy>(startPos); // Boss zawsze na koniec fali podzielnej przez 5
            }
            else {
                int roll = enemiesToSpawn % 10;

                // SKALOWANIE DLA TRUDNEJ MAPY (Ciê¿cy wrogowie od pocz¹tku)
                if (difficultyLevel == 2) {
                    if (currentWave >= 10) {
                        if (roll <= 2) newEnemy = std::make_unique<BossEnemy>(startPos); // Mini bossy!
                        else if (roll <= 5) newEnemy = std::make_unique<TankEnemy>(startPos);
                        else newEnemy = std::make_unique<MaskedEnemy>(startPos);
                    }
                    else if (currentWave >= 5) {
                        if (roll <= 3) newEnemy = std::make_unique<TankEnemy>(startPos);
                        else if (roll <= 6) newEnemy = std::make_unique<ArmoredEnemy>(startPos);
                        else newEnemy = std::make_unique<FastEnemy>(startPos);
                    }
                    else {
                        if (roll <= 4) newEnemy = std::make_unique<ArmoredEnemy>(startPos);
                        else newEnemy = std::make_unique<FastEnemy>(startPos);
                    }
                }
                // SKALOWANIE DLA ŒREDNIEJ MAPY (Balans standardowy)
                else if (difficultyLevel == 1) {
                    if (currentWave >= 8) {
                        if (roll <= 1) newEnemy = std::make_unique<TankEnemy>(startPos);
                        else if (roll <= 4) newEnemy = std::make_unique<MaskedEnemy>(startPos);
                        else newEnemy = std::make_unique<FastEnemy>(startPos);
                    }
                    else if (currentWave >= 4) {
                        if (roll <= 3) newEnemy = std::make_unique<ArmoredEnemy>(startPos);
                        else newEnemy = std::make_unique<Goblin>(startPos);
                    }
                    else {
                        newEnemy = std::make_unique<Goblin>(startPos);
                    }
                }
                // SKALOWANIE DLA £ATWEJ MAPY (Spokojna rozgrywka)
                else {
                    if (currentWave >= 8) {
                        if (roll <= 2) newEnemy = std::make_unique<ArmoredEnemy>(startPos);
                        else if (roll <= 4) newEnemy = std::make_unique<FastEnemy>(startPos);
                        else newEnemy = std::make_unique<Goblin>(startPos);
                    }
                    else {
                        newEnemy = std::make_unique<Goblin>(startPos); // Tylko gobliny przez d³ugi czas
                    }
                }
            }

            newEnemy->setOnDeathCallback([&playerStats](int points) {
                playerStats.addReward(points);
                playerStats.gold += points;
                });

            activeEnemies.push_back(std::move(newEnemy));
            enemiesToSpawn--;
            spawnTimer = 0.f;
        }
    }

    for (auto& enemy : activeEnemies) {
        enemy->moveAlongPath(pathPoints, dt);
        enemy->update(dt);
        if (enemy->hasReachedEnd(pathPoints)) {
            playerStats.takeDamage(enemy->getDamageToBase());
            enemy->takeDamage(99999, DamageType::CANNON);
        }
    }

    std::erase_if(activeEnemies, [](const std::unique_ptr<Enemy>& enemy) {
        return enemy->isDead();
        });

    if (enemiesToSpawn <= 0 && activeEnemies.empty()) {
        isWaveRunning = false;
    }
}

void WaveManager::drawEnemies(sf::RenderWindow& window) {
    std::lock_guard<std::mutex> lock(waveMutex);
    for (const auto& enemy : activeEnemies) {
        enemy->draw(window);
    }
}

bool WaveManager::isWaveActive() const {
    return isWaveRunning;
}
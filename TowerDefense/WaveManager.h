#pragma once
#include <vector>
#include <memory>
#include <mutex>
#include <future>
#include <SFML/Graphics.hpp>
#include "Enemy.h"
#include "PlayerStats.h"

class WaveManager {
private:
    int currentWave;
    int totalWaves; // NOWE: Ilo  fal potrzebnych do wygranej
    int difficultyLevel; // NOWE: Poziom trudno ci (0 -  atwy, 1 -  redni, 2 - Trudny)

    std::vector<std::unique_ptr<Enemy>> activeEnemies;
    std::vector<sf::Vector2f> pathPoints;

    std::mutex waveMutex;
    std::future<void> waveLoaderFuture;

    bool isLoading;
    bool isWaveRunning;
    float spawnTimer;
    int enemiesToSpawn;

public:
    WaveManager();
    ~WaveManager();

    // NOWE: Metody zarz dzania cyklem gry
    void reset();
    void setMapData(const std::vector<sf::Vector2f>& newPath, int totalWvs, int difficulty);
    void loadWaveDataAsync();
    void startNextWave();

    void updateEnemies(float dt, PlayerStats& playerStats);
    void drawEnemies(sf::RenderWindow& window);

    bool isWaveActive() const;
    int getCurrentWave() const { return currentWave; }
    int getTotalWaves() const { return totalWaves; }

    const std::vector<std::unique_ptr<Enemy>>& getEnemies() const {
        return activeEnemies;
    }
};
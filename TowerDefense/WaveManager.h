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
    std::vector<std::unique_ptr<Enemy>> activeEnemies;
    std::vector<sf::Vector2f> pathPoints; // Œcie¿ka, po której id¹ wrogowie

    std::mutex waveMutex; // Do synchronizacji przy asynchronicznym ³adowaniu
    std::future<void> waveLoaderFuture; // Przechowuje asynchroniczne zadanie
    bool isLoading;
    bool isWaveRunning; // POPRAWKA: Ta deklaracja jest konieczna, by usun¹æ b³êdy C2065 i C2614

    float spawnTimer;
    int enemiesToSpawn;

public:
    WaveManager();
    ~WaveManager();

    // Inicjuje asynchroniczne wczytywanie danych o fali z plików/pamiêci
    void loadWaveDataAsync();

    // Ustawia œcie¿kê mapy
    void setPath(const std::vector<sf::Vector2f>& newPath);

    void startNextWave();

    // G³ówna pêtla fali
    void updateEnemies(float dt, PlayerStats& playerStats);
    void drawEnemies(sf::RenderWindow& window);

    bool isWaveActive() const;
    int getCurrentWave() const { return currentWave; }

    // Getter zwracaj¹cy wrogów, wymagany do namierzania przez wie¿e
    const std::vector<std::unique_ptr<Enemy>>& getEnemies() const {
        return activeEnemies;
    }
};
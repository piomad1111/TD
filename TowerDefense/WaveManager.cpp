#include "WaveManager.h"
#include <iostream>
#include <thread>
#include <chrono>

WaveManager::WaveManager()
    : currentWave(0), isLoading(false), isWaveRunning(false), spawnTimer(0.f), enemiesToSpawn(0) {
}

WaveManager::~WaveManager() {
    // Upewniamy siê, ¿e w¹tek ³adowania zakoñczy pracê przed zniszczeniem obiektu
    if (waveLoaderFuture.valid()) {
        waveLoaderFuture.wait();
    }
}

void WaveManager::setPath(const std::vector<sf::Vector2f>& newPath) {
    pathPoints = newPath;
}

void WaveManager::loadWaveDataAsync() {
    if (isLoading) return;

    isLoading = true;
    std::cout << "Rozpoczynam asynchroniczne ladowanie fali: " << currentWave + 1 << "...\n";

    // Odpalamy zadanie w tle (np. wczytywanie skomplikowanych danych XML/JSON mapy)
    waveLoaderFuture = std::async(std::launch::async, [this]() {
        // Symulacja d³ugiego ³adowania (np. odczyt plików)
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));

        // Blokujemy mutex, by bezpiecznie zmodyfikowaæ zmienne
        std::lock_guard<std::mutex> lock(waveMutex);
        // UWAGA: Nie ustawiamy tutaj wrogów. Zrobi to startNextWave po klikniêciu spacji.

        std::cout << "Fala " << currentWave + 1 << " zaladowana pomyslnie!\n";
        isLoading = false;
        });
}

void WaveManager::startNextWave() {
    if (isLoading) {
        std::cout << "Czekaj, dane fali wciaz sie laduja...\n";
        return;
    }

    if (isWaveRunning) {
        return; // Zabezpieczenie przed wielokrotnym wciskaniem spacji, gdy fala ju¿ trwa
    }

    std::lock_guard<std::mutex> lock(waveMutex);
    isWaveRunning = true;
    currentWave++;
    spawnTimer = 0.f;
    enemiesToSpawn = 10 + (currentWave * 5); // Ustalenie wrogów dopiero w momencie startu po klikniêciu
    std::cout << "FALA " << currentWave << " WYSTARTOWALA!\n";
}

void WaveManager::updateEnemies(float dt, PlayerStats& playerStats) {
    if (!isWaveRunning) return; // Jeœli fala pauzuje (czekamy na spacjê), zatrzymujemy logikê

    // --- 1. SPRAWANIE WROGÓW ---
    if (enemiesToSpawn > 0) {
        spawnTimer += dt;
        if (spawnTimer >= 1.0f) { // Co sekundê pojawia siê nowy Goblin
            std::lock_guard<std::mutex> lock(waveMutex);

            sf::Vector2f startPos = pathPoints.empty() ? sf::Vector2f(0.f, 0.f) : pathPoints.front();
            auto newEnemy = std::make_unique<Goblin>(startPos);

            // Konfiguracja callbacku na œmieræ (dodaje z³oto i punkty)
            newEnemy->setOnDeathCallback([&playerStats](int points) {
                playerStats.addReward(points);
                playerStats.gold += points; // Zak³adamy, ¿e points = gold dla prostoty
                });

            activeEnemies.push_back(std::move(newEnemy));
            enemiesToSpawn--;
            spawnTimer = 0.f;
        }
    }

    // --- 2. AKTUALIZACJA I RUCH ---
    for (auto& enemy : activeEnemies) {
        enemy->moveAlongPath(pathPoints, dt);
        enemy->update(dt);

        // Jeœli wróg doszed³ do koñca œcie¿ki
        if (enemy->hasReachedEnd(pathPoints)) {
            playerStats.takeDamage(enemy->getDamageToBase());
            enemy->takeDamage(9999); // Zabijamy wroga (wpad³ do bazy)
        }
    }

    // --- 3. USUWANIE MARTWYCH WROGÓW (Modern C++ idiom) ---
    // U¿ywamy std::erase_if (C++20), co jest mega zoptymalizowane
    std::erase_if(activeEnemies, [](const std::unique_ptr<Enemy>& enemy) {
        return enemy->isDead();
        });

    // --- 4. SPRAWDZANIE KOÑCA FALI ---
    if (enemiesToSpawn <= 0 && activeEnemies.empty()) {
        isWaveRunning = false; // Fala siê skoñczy³a. Oczekujemy na kolejne klikniêcie spacji.
    }
}

void WaveManager::drawEnemies(sf::RenderWindow& window) {
    // Chronimy dostêp na wypadek asynchronicznoœci
    std::lock_guard<std::mutex> lock(waveMutex);
    for (const auto& enemy : activeEnemies) {
        enemy->draw(window);
    }
}

bool WaveManager::isWaveActive() const {
    return isWaveRunning; // Decyduje czy wyœwietliæ tekst "Wciœnij spacjê" w HUD
}
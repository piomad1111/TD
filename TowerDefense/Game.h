#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <format>
#include "PlayerStats.h"
#include "MenuMenager.h" 
#include "WaveManager.h" 
#include "Tower.h" 
#include "Projectile.h" 

enum class GameState {
    LOGIN,
    MAIN_MENU,
    MAP_SELECTION,
    SCOREBOARD,
    OPTIONS,
    SETTINGS,
    GAMEPLAY,
    PAUSE, // <-- DODANO STAN PAUZY
    GAME_OVER
};

struct LevelMap {
    std::string name;
    std::vector<sf::Vector2f> path;
    std::vector<sf::RectangleShape> obstacles;
    int totalWaves = 0;
    int difficulty = 0;
    sf::Color bgColor;
    sf::Color pathColor;

    // NOWE: Wizualizacja grubej œcie¿ki
    std::vector<sf::RectangleShape> pathShapes;
    std::vector<sf::CircleShape> pathJoints;
};

struct ShopButton {
    sf::RectangleShape rect;
    sf::Text text;
    sf::Text costText;
    int cost = 0;
    std::string towerName;

    ShopButton(const sf::Font& font) : text(font), costText(font) {}
    ShopButton(ShopButton&&) = default;
    ShopButton& operator=(ShopButton&&) = default;
    ShopButton(const ShopButton&) = delete;
    ShopButton& operator=(const ShopButton&) = delete;
};

class Game {
public:
    Game();
    void run();

private:
    void initMaps();
    void handleEvents();
    void update(float dt);
    void render();
    void changeState(GameState newState);
    void startGame(int mapIndex);
    bool canPlaceTower(sf::Vector2f pos);
    void initGameplayHUD();
    void drawGameplayHUD();
    void handleGameplayClicks(sf::Vector2f mousePos, sf::Mouse::Button button);

    sf::RenderWindow window;
    GameState currentState;
    sf::Clock clock;

    PlayerStats playerStats;
    MenuManager menuManager;
    WaveManager waveManager;

    std::vector<LevelMap> availableMaps;
    int currentMapIndex = 0;
    bool hasWon = false;

    sf::VertexArray pathLines;

    sf::Texture backgroundTexture;
    sf::Sprite backgroundSprite;
    sf::Font font;

    sf::Text tytul;
    sf::Text loginPromptText;
    sf::Text inputText;
    sf::Text errorText;
    std::string currentInput;
    sf::RectangleShape inputBox;

    sf::RectangleShape hudSidebar;
    sf::Text statsText;
    sf::Text shopTitleText;
    std::vector<ShopButton> shopButtons;

    bool isPlacingTower = false;
    int selectedTowerIndex = -1;
    sf::RectangleShape ghostTower;

    std::vector<std::unique_ptr<Tower>> activeTowers;
    std::vector<std::unique_ptr<Projectile>> activeProjectiles;

    std::vector<ScoreEntry> topScores; // BUFOR NA NAJLEPSZE WYNIKI

    // Zmienne do autostartu fali
    float autoWaveTimer = 0.f;
    float autoWaveDelay = 5.f; // 5 sekund miedzy falami
};
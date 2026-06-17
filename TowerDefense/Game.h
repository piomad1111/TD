#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <format> // C++20 do wygodnego formatowania stringów
#include "PlayerStats.h"
#include "MenuMenager.h" 
#include "WaveManager.h" 
#include "Tower.h" // DODANE: Nag³ówek wie¿
#include "Projectile.h" // DODANE: Nag³ówek pocisków

enum class GameState {
    LOGIN,
    MAIN_MENU,
    MAP_SELECTION,
    GAMEPLAY,
    GAME_OVER
};

struct ShopButton {
    sf::RectangleShape rect;
    sf::Text text;
    sf::Text costText;
    int cost = 0; // POPRAWKA: Inicjalizacja zmiennej naprawiaj¹ca ostrze¿enie C26495
    std::string towerName;

    // SFML 3: sf::Text wymaga czcionki w konstruktorze
    ShopButton(const sf::Font& font) : text(font), costText(font) {}
};

class Game {
public:
    Game();
    void run();

private:
    void handleEvents();
    void update(float dt);
    void render();
    void changeState(GameState newState);

    // NOWE: Funkcja weryfikuj¹ca czy mo¿na postawiæ wie¿ê na danej pozycji
    bool canPlaceTower(sf::Vector2f pos);

    // Nowe funkcje do obs³ugi interfejsu
    void initGameplayHUD();
    void drawGameplayHUD();
    void handleGameplayClicks(sf::Vector2f mousePos, sf::Mouse::Button button);

    sf::RenderWindow window;
    GameState currentState;
    sf::Clock clock;

    PlayerStats playerStats;
    MenuManager menuManager;
    WaveManager waveManager;

    std::vector<sf::Vector2f> testPath;
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
    sf::Text welcomeText;

    // --- ELEMENTY HUD W GRZE ---
    sf::RectangleShape hudSidebar;       // T³o panelu bocznego
    sf::Text statsText;                  // Tekst wyœwietlaj¹cy HP, Gold, Falê
    sf::Text shopTitleText;              // Napis "MENU BUDOWY"
    std::vector<ShopButton> shopButtons; // Przyciski wie¿

    // --- MECHANIKA BUDOWANIA ---
    bool isPlacingTower = false;         // Czy gracz aktualnie trzyma wie¿ê w kursorze?
    int selectedTowerIndex = -1;
    sf::RectangleShape ghostTower;       // Pó³przezroczysty kwadrat pokazuj¹cy gdzie postawimy wie¿ê

    // Kontener przechowuj¹cy wszystkie postawione na mapie wie¿e
    std::vector<std::unique_ptr<Tower>> activeTowers;

    // DODANE: Kontener przechowuj¹cy wszystkie aktywne pociski w powietrzu
    std::vector<std::unique_ptr<Projectile>> activeProjectiles;
};
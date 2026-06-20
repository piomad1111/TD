#include "Game.h"
#include <regex>

Game::Game()
    : currentState(GameState::LOGIN), menuManager(font), backgroundSprite(backgroundTexture),
    tytul(font), loginPromptText(font), inputText(font), errorText(font),
    statsText(font), shopTitleText(font)
{
    sf::VideoMode desktopMode = sf::VideoMode::getDesktopMode();
    window.create(desktopMode, "Tower Defense", sf::State::Fullscreen);

    if (!backgroundTexture.loadFromFile("bgBlue.jpg")) {
        std::cerr << "Blad wczytywania tekstury bgBlue.jpg!" << std::endl;
    }

    if (!font.openFromFile("Roboto-Black.ttf")) {
        std::cerr << "Blad wczytywania czcionki!" << std::endl;
    }

    initMaps();
    pathLines.setPrimitiveType(sf::PrimitiveType::LineStrip);

    tytul.setString("TOWER DEFENSE");
    tytul.setCharacterSize(90);
    tytul.setFillColor(sf::Color::Red);
    sf::FloatRect textRect = tytul.getLocalBounds();
    tytul.setOrigin({ textRect.position.x + textRect.size.x / 2.0f, textRect.position.y + textRect.size.y / 2.0f });

    loginPromptText.setString("Podaj swoj pseudonim (3-12 znakow):");
    loginPromptText.setCharacterSize(30);

    inputBox.setSize(sf::Vector2f(400.f, 60.f));
    inputBox.setFillColor(sf::Color(0, 0, 0, 150));
    inputBox.setOutlineThickness(2.f);

    inputText.setCharacterSize(40);
    inputText.setFillColor(sf::Color::Yellow);

    errorText.setCharacterSize(25);
    errorText.setFillColor(sf::Color::Red);

    initGameplayHUD();

    changeState(GameState::LOGIN);
}

void Game::initMaps() {
    // MAPA £ATWA
    LevelMap easy;
    easy.name = "Latwa (10 fal) [+0.0x pkt]"; // Zaktualizowana nazwa
    easy.path = { {-50.f, 200.f}, {300.f, 200.f}, {300.f, 600.f}, {800.f, 600.f}, {800.f, 300.f}, {1600.f, 300.f} };
    easy.totalWaves = 10;
    easy.difficulty = 0;
    easy.bgColor = sf::Color(34, 139, 34);
    easy.pathColor = sf::Color(100, 100, 100, 150);

    sf::RectangleShape obs1({ 200.f, 200.f });
    obs1.setPosition({ 400.f, 300.f });
    obs1.setFillColor(sf::Color(105, 105, 105));
    obs1.setOutlineThickness(2.f);
    obs1.setOutlineColor(sf::Color::Black);
    easy.obstacles.push_back(obs1);

    availableMaps.push_back(easy);

    // MAPA ŒREDNIA
    LevelMap medium;
    medium.name = "Srednia (15 fal) [+0.5x pkt]"; // Zaktualizowana nazwa z mno¿nikiem
    medium.path = { {-50.f, 500.f}, {200.f, 500.f}, {200.f, 100.f}, {700.f, 100.f}, {700.f, 700.f}, {1200.f, 700.f}, {1200.f, 400.f}, {1600.f, 400.f} };
    medium.totalWaves = 15;
    medium.difficulty = 1;
    medium.bgColor = sf::Color(139, 115, 85);
    medium.pathColor = sf::Color(60, 40, 20, 150);

    sf::RectangleShape obs2({ 400.f, 150.f });
    obs2.setPosition({ 300.f, 300.f });
    obs2.setFillColor(sf::Color(80, 80, 80));
    obs2.setOutlineThickness(2.f);
    obs2.setOutlineColor(sf::Color::Black);

    sf::RectangleShape obs3({ 200.f, 300.f });
    obs3.setPosition({ 900.f, 200.f });
    obs3.setFillColor(sf::Color(80, 80, 80));
    obs3.setOutlineThickness(2.f);
    obs3.setOutlineColor(sf::Color::Black);

    medium.obstacles.push_back(obs2);
    medium.obstacles.push_back(obs3);

    availableMaps.push_back(medium);

    // MAPA TRUDNA
    LevelMap hard;
    hard.name = "Trudna (20 fal) [+1.0x pkt]"; // Zaktualizowana nazwa z mno¿nikiem
    hard.path = { {800.f, -50.f}, {800.f, 300.f}, {200.f, 300.f}, {200.f, 800.f}, {1300.f, 800.f}, {1300.f, 150.f}, {1600.f, 150.f} };
    hard.totalWaves = 20;
    hard.difficulty = 2;
    hard.bgColor = sf::Color(60, 60, 60);
    hard.pathColor = sf::Color(200, 50, 0, 150);

    sf::RectangleShape obs4({ 600.f, 400.f });
    obs4.setPosition({ 350.f, 350.f });
    obs4.setFillColor(sf::Color(30, 30, 30));
    obs4.setOutlineThickness(2.f);
    obs4.setOutlineColor(sf::Color::Black);
    hard.obstacles.push_back(obs4);

    availableMaps.push_back(hard);
}

void Game::initGameplayHUD() {
    shopButtons.clear();
    float windowWidth = static_cast<float>(window.getSize().x);
    float windowHeight = static_cast<float>(window.getSize().y);
    float sidebarWidth = 350.f;

    hudSidebar.setSize({ sidebarWidth, windowHeight });
    hudSidebar.setPosition({ windowWidth - sidebarWidth, 0.f });
    hudSidebar.setFillColor(sf::Color(40, 40, 50, 240));
    hudSidebar.setOutlineThickness(-3.f);
    hudSidebar.setOutlineColor(sf::Color(100, 100, 100));

    statsText.setCharacterSize(22);
    statsText.setFillColor(sf::Color::White);
    statsText.setPosition({ windowWidth - sidebarWidth + 20.f, 20.f });

    shopTitleText.setString("MENU BUDOWY");
    shopTitleText.setCharacterSize(30);
    shopTitleText.setFillColor(sf::Color::Yellow);
    shopTitleText.setPosition({ windowWidth - sidebarWidth + 60.f, 180.f });

    std::vector<std::pair<std::string, int>> towerTypes = {
        {"1. Lucznik", 100}, {"2. Mag", 150}, {"3. Armata", 300},
        {"4. Lodowa", 200},  {"5. Trucizny", 250}, {"6. Kopalnia", 400},
        {"7. Radar", 500},   {"8. Piorun", 600}
    };

    float startY = 230.f;
    for (size_t i = 0; i < towerTypes.size(); ++i) {
        ShopButton btn(font);
        btn.towerName = towerTypes[i].first;
        btn.cost = towerTypes[i].second;

        btn.rect.setSize({ sidebarWidth - 40.f, 50.f });
        btn.rect.setPosition({ windowWidth - sidebarWidth + 20.f, startY + (i * 65.f) });
        btn.rect.setFillColor(sf::Color(70, 70, 80));
        btn.rect.setOutlineThickness(2.f);

        btn.text.setString(btn.towerName);
        btn.text.setCharacterSize(20);
        btn.text.setPosition({ btn.rect.getPosition().x + 10.f, btn.rect.getPosition().y + 10.f });

        btn.costText.setString(std::to_string(btn.cost) + " G");
        btn.costText.setCharacterSize(20);
        btn.costText.setFillColor(sf::Color::Yellow);
        btn.costText.setPosition({ btn.rect.getPosition().x + 220.f, btn.rect.getPosition().y + 10.f });

        shopButtons.push_back(std::move(btn));
    }

    ghostTower.setSize({ 40.f, 40.f });
    ghostTower.setOrigin({ 20.f, 20.f });
    ghostTower.setFillColor(sf::Color(255, 255, 255, 128));
}


void Game::changeState(GameState newState) {
    // --- INTELIGENTNY SYSTEM POWROTU ---
    // Zmienna statyczna przechowuj¹ca ostatni "g³ówny" ekran przed wejœciem w podmenu
    static GameState savedReturnState = GameState::MAIN_MENU;

    // Jeœli obecnie jesteœmy w G³ównym Menu lub Pauzie, zapisujemy to jako punkt powrotu
    if (currentState == GameState::MAIN_MENU || currentState == GameState::PAUSE) {
        savedReturnState = currentState;
    }

    // Kopiujemy go lokalnie by bezpiecznie przekazaæ do lambd (przycisków)
    GameState targetReturn = savedReturnState;

    currentState = newState;
    menuManager.clearButtons();

    float centerX = window.getSize().x / 2.0f;

    if (currentState == GameState::MAIN_MENU) {
        menuManager.addButton("Graj", { centerX, 250.f }, [this]() { changeState(GameState::MAP_SELECTION); });
        menuManager.addButton("Tablica Wynikow", { centerX, 350.f }, [this]() { changeState(GameState::SCOREBOARD); });
        menuManager.addButton("Modyfikatory", { centerX, 450.f }, [this]() { changeState(GameState::OPTIONS); });
        menuManager.addButton("Ustawienia ", { centerX, 550.f }, [this]() { changeState(GameState::SETTINGS); });
        menuManager.addButton("Wyjscie", { centerX, 650.f }, [this]() { window.close(); });
    }
    else if (currentState == GameState::MAP_SELECTION) {
        menuManager.addButton(availableMaps[0].name, { centerX, 300.f }, [this]() { startGame(0); });
        menuManager.addButton(availableMaps[1].name, { centerX, 400.f }, [this]() { startGame(1); });
        menuManager.addButton(availableMaps[2].name, { centerX, 500.f }, [this]() { startGame(2); });
        menuManager.addButton("Powrot", { centerX, 700.f }, [this]() { changeState(GameState::MAIN_MENU); });
    }
    else if (currentState == GameState::SCOREBOARD) {
        topScores = PlayerStats::loadLeaderboard();
        // ZMIANA: Powrót kieruje do zapisanego targetReturn
        menuManager.addButton("Powrot", { centerX, 750.f }, [this, targetReturn]() { changeState(targetReturn); });
    }
    else if (currentState == GameState::OPTIONS) {
        menuManager.addButton(playerStats.modifierLessGold ? "[X] Mniej zlota (+0.5x mnoznik)" : "[ ] Mniej zlota (+0.5x mnoznik)", { centerX, 300.f }, [this]() {
            playerStats.modifierLessGold = !playerStats.modifierLessGold;
            playerStats.recalculateMultiplier();
            changeState(GameState::OPTIONS);
            });
        menuManager.addButton(playerStats.modifierFragileBase ? "[X] Krucha Baza (+0.5x mnoznik)" : "[ ] Krucha Baza (+0.5x mnoznik)", { centerX, 400.f }, [this]() {
            playerStats.modifierFragileBase = !playerStats.modifierFragileBase;
            playerStats.recalculateMultiplier();
            changeState(GameState::OPTIONS);
            });
        menuManager.addButton(playerStats.autoWaveStart ? "[X] Auto-start fal (Brak mnoznika)" : "[ ] Auto-start fal (Brak mnoznika)", { centerX, 500.f }, [this]() {
            playerStats.autoWaveStart = !playerStats.autoWaveStart;
            changeState(GameState::OPTIONS);
            });
        // ZMIANA: Powrót kieruje do zapisanego targetReturn
        menuManager.addButton("Powrot", { centerX, 650.f }, [this, targetReturn]() { changeState(targetReturn); });
    }
    else if (currentState == GameState::SETTINGS) {
        menuManager.addButton("1280x720 (Okno)", { centerX, 300.f }, [this]() {
            window.create(sf::VideoMode({ 1280, 720 }), "Tower Defense", sf::Style::Close);
            initGameplayHUD();
            changeState(GameState::SETTINGS);
            });
        menuManager.addButton("1920x1080 (Okno)", { centerX, 400.f }, [this]() {
            window.create(sf::VideoMode({ 1920, 1080 }), "Tower Defense", sf::Style::Close);
            initGameplayHUD();
            changeState(GameState::SETTINGS);
            });
        menuManager.addButton("Pelny Ekran", { centerX, 500.f }, [this]() {
            window.create(sf::VideoMode::getDesktopMode(), "Tower Defense", sf::State::Fullscreen);
            initGameplayHUD();
            changeState(GameState::SETTINGS);
            });
        // ZMIANA: Powrót kieruje do zapisanego targetReturn
        menuManager.addButton("Powrot", { centerX, 700.f }, [this, targetReturn]() { changeState(targetReturn); });
    }
    else if (currentState == GameState::PAUSE) {
        menuManager.addButton("Wznow", { centerX, 250.f }, [this]() { changeState(GameState::GAMEPLAY); });
        menuManager.addButton("Tablica Wynikow", { centerX, 350.f }, [this]() {
            topScores = PlayerStats::loadLeaderboard();
            changeState(GameState::SCOREBOARD);
            });
        menuManager.addButton("Ustawienia ", { centerX, 450.f }, [this]() { changeState(GameState::SETTINGS); });
        menuManager.addButton("Wroc do Menu", { centerX, 550.f }, [this]() { changeState(GameState::MAIN_MENU); });
        menuManager.addButton("Wyjscie", { centerX, 650.f }, [this]() { window.close(); });
    }
    else if (currentState == GameState::GAME_OVER) {
        topScores = PlayerStats::loadLeaderboard();
        menuManager.addButton("Zagraj ponownie", { centerX, 520.f }, [this]() { changeState(GameState::MAP_SELECTION); });
        menuManager.addButton("Wroc do Menu", { centerX, 600.f }, [this]() { changeState(GameState::MAIN_MENU); });
    }
}


void Game::startGame(int mapIndex) {
    currentMapIndex = mapIndex;
    auto& map = availableMaps[currentMapIndex];

    // DODANIE MNO¯NIKA TRUDNOŒCI: £atwa +0.0x, Œrednia +0.5x, Trudna +1.0x
    playerStats.mapDifficultyMultiplier = map.difficulty * 0.5f;
    playerStats.baseHealth = playerStats.modifierFragileBase ? 50 : playerStats.maxBaseHealth;
    playerStats.gold = playerStats.modifierLessGold ? 500 : 1000;
    playerStats.score = 0;

    // Obliczamy ostateczny mnoznik po wszystkich bonusach
    playerStats.recalculateMultiplier();

    isPlacingTower = false;
    hasWon = false;

    activeTowers.clear();
    activeProjectiles.clear();
    waveManager.reset();
    waveManager.setMapData(map.path, map.totalWaves, map.difficulty);
    autoWaveTimer = autoWaveDelay;

    pathLines.clear();
    for (const auto& point : map.path) {
        pathLines.append(sf::Vertex(point, map.pathColor));
    }

    changeState(GameState::GAMEPLAY);
    waveManager.loadWaveDataAsync();
}


void Game::run() {
    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        handleEvents();
        update(dt);
        render();
    }
}

void Game::handleEvents() {
    while (const std::optional event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) window.close();

        if (currentState == GameState::LOGIN) {
            if (const auto* textEvent = event->getIf<sf::Event::TextEntered>()) {
                std::uint32_t unicode = textEvent->unicode;
                if (unicode == 8 && !currentInput.empty()) {
                    currentInput.pop_back(); errorText.setString("");
                }
                else if (unicode >= 32 && unicode < 128 && currentInput.size() < 12) {
                    currentInput += static_cast<char>(unicode); errorText.setString("");
                }
            }
            if (const auto* keyEvent = event->getIf<sf::Event::KeyPressed>()) {
                if (keyEvent->scancode == sf::Keyboard::Scancode::Enter) {
                    std::regex nickPattern("^[a-zA-Z0-9]{3,12}$");
                    if (std::regex_match(currentInput, nickPattern)) {
                        playerStats.nickname = currentInput; changeState(GameState::MAIN_MENU);
                    }
                    else errorText.setString("Blad: Nick musi miec 3-12 znakow!");
                }
            }
        }
        else if (currentState == GameState::GAMEPLAY) {
            if (const auto* keyEvent = event->getIf<sf::Event::KeyPressed>()) {
                if (keyEvent->scancode == sf::Keyboard::Scancode::Space) waveManager.startNextWave();
                if (keyEvent->scancode == sf::Keyboard::Scancode::Escape) {
                    if (isPlacingTower) isPlacingTower = false;
                    else changeState(GameState::PAUSE); // Prze³¹cz na PAUZÊ zamiast na MENU
                }
            }
            if (const auto* mouseEvent = event->getIf<sf::Event::MouseButtonPressed>()) {
                sf::Vector2f mousePos(static_cast<float>(mouseEvent->position.x), static_cast<float>(mouseEvent->position.y));
                handleGameplayClicks(mousePos, mouseEvent->button);
            }
        }
        else if (currentState == GameState::PAUSE) {
            if (const auto* keyEvent = event->getIf<sf::Event::KeyPressed>()) {
                // Szybki powrót równie¿ za pomoc¹ klawisza ESC
                if (keyEvent->scancode == sf::Keyboard::Scancode::Escape) {
                    changeState(GameState::GAMEPLAY);
                }
            }
            if (const auto* mouseEvent = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mouseEvent->button == sf::Mouse::Button::Left) {
                    menuManager.handleClick({ static_cast<float>(mouseEvent->position.x), static_cast<float>(mouseEvent->position.y) });
                }
            }
        }
        else {
            if (const auto* mouseEvent = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mouseEvent->button == sf::Mouse::Button::Left) {
                    menuManager.handleClick({ static_cast<float>(mouseEvent->position.x), static_cast<float>(mouseEvent->position.y) });
                }
            }
        }
    }
}

void Game::handleGameplayClicks(sf::Vector2f mousePos, sf::Mouse::Button button) {
    if (button == sf::Mouse::Button::Right) { isPlacingTower = false; return; }

    if (button == sf::Mouse::Button::Left) {
        if (hudSidebar.getGlobalBounds().contains(mousePos)) {
            for (size_t i = 0; i < shopButtons.size(); ++i) {
                if (shopButtons[i].rect.getGlobalBounds().contains(mousePos)) {
                    if (playerStats.getGold() >= shopButtons[i].cost) {
                        isPlacingTower = true;
                        selectedTowerIndex = static_cast<int>(i);

                        if (i == 0) ghostTower.setFillColor(sf::Color(0, 255, 0, 150));
                        else if (i == 1) ghostTower.setFillColor(sf::Color(0, 0, 255, 150));
                        else if (i == 2) ghostTower.setFillColor(sf::Color(255, 0, 0, 150));
                        else if (i == 3) ghostTower.setFillColor(sf::Color(100, 200, 255, 150));
                        else if (i == 4) ghostTower.setFillColor(sf::Color(150, 50, 150, 150));
                        else if (i == 5) ghostTower.setFillColor(sf::Color(255, 215, 0, 150));
                        else if (i == 6) ghostTower.setFillColor(sf::Color(128, 128, 128, 150));
                        else if (i == 7) ghostTower.setFillColor(sf::Color(255, 255, 255, 150));
                    }
                    return;
                }
            }
        }
        else if (isPlacingTower) {
            if (canPlaceTower(mousePos)) {
                if (playerStats.spendGold(shopButtons[selectedTowerIndex].cost)) {
                    if (selectedTowerIndex == 0) activeTowers.push_back(std::make_unique<ArcherTower>(mousePos));
                    else if (selectedTowerIndex == 1) activeTowers.push_back(std::make_unique<MageTower>(mousePos));
                    else if (selectedTowerIndex == 2) activeTowers.push_back(std::make_unique<CannonTower>(mousePos));
                    else if (selectedTowerIndex == 3) activeTowers.push_back(std::make_unique<IceTower>(mousePos));
                    else if (selectedTowerIndex == 4) activeTowers.push_back(std::make_unique<PoisonTower>(mousePos));
                    else if (selectedTowerIndex == 5) activeTowers.push_back(std::make_unique<MineTower>(mousePos));
                    else if (selectedTowerIndex == 6) activeTowers.push_back(std::make_unique<RadarTower>(mousePos));
                    else if (selectedTowerIndex == 7) activeTowers.push_back(std::make_unique<LightningTower>(mousePos));
                }
                isPlacingTower = false;
            }
        }
    }
}

bool Game::canPlaceTower(sf::Vector2f pos) {
    if (pos.x > window.getSize().x - 370.f) return false;

    for (const auto& tower : activeTowers) {
        sf::Vector2f tPos = tower->getPosition();
        float distSq = (pos.x - tPos.x) * (pos.x - tPos.x) + (pos.y - tPos.y) * (pos.y - tPos.y);
        if (distSq < 45.f * 45.f) return false;
    }

    const auto& path = availableMaps[currentMapIndex].path;
    for (size_t i = 0; i < path.size() - 1; ++i) {
        sf::Vector2f a = path[i], b = path[i + 1], ab = b - a, ap = pos - a;
        float proj = ap.x * ab.x + ap.y * ab.y;
        float abLenSq = ab.x * ab.x + ab.y * ab.y;
        float t = std::max(0.0f, std::min(1.0f, proj / abLenSq));
        sf::Vector2f closest = a + ab * t;
        float distToPathSq = (pos.x - closest.x) * (pos.x - closest.x) + (pos.y - closest.y) * (pos.y - closest.y);
        if (distToPathSq < 35.f * 35.f) return false;
    }

    sf::FloatRect towerBounds({ pos.x - 20.f, pos.y - 20.f }, { 40.f, 40.f });
    for (const auto& obs : availableMaps[currentMapIndex].obstacles) {
        if (obs.getGlobalBounds().findIntersection(towerBounds).has_value()) {
            return false;
        }
    }

    return true;
}


void Game::update(float dt) {
    sf::Vector2i mousePosI = sf::Mouse::getPosition(window);
    sf::Vector2f mousePos(static_cast<float>(mousePosI.x), static_cast<float>(mousePosI.y));

    menuManager.update(mousePos);

    // Wykrywanie najechania myszk¹ na wie¿e - dzia³a w GAMEPLAY oraz na PAUZIE
    if (currentState == GameState::GAMEPLAY || currentState == GameState::PAUSE) {
        for (auto& tower : activeTowers) {
            sf::Vector2f tPos = tower->getPosition();
            float distSq = (mousePos.x - tPos.x) * (mousePos.x - tPos.x) + (mousePos.y - tPos.y) * (mousePos.y - tPos.y);
            // Zwiêkszono promieñ z 20.f do 45.f - lepiej odpowiada wymiarom wizualnym wie¿y
            tower->setHovered(distSq <= 45.f * 45.f);
        }
    }

    if (currentState == GameState::LOGIN) {
        inputText.setString(currentInput + "_");
    }
    else if (currentState == GameState::GAMEPLAY) {
        if (playerStats.getHealth() <= 0) {
            hasWon = false;
            PlayerStats::saveScoreToLeaderboard(playerStats.nickname, playerStats.getScore());
            changeState(GameState::GAME_OVER);
            return;
        }
        else if (!waveManager.isWaveActive() && waveManager.getEnemies().empty() && waveManager.getCurrentWave() >= availableMaps[currentMapIndex].totalWaves) {
            hasWon = true;
            playerStats.addReward(playerStats.getGold());
            playerStats.addReward(playerStats.getHealth() * 10);

            PlayerStats::saveScoreToLeaderboard(playerStats.nickname, playerStats.getScore());

            changeState(GameState::GAME_OVER);
            return;
        }

        waveManager.updateEnemies(dt, playerStats);

        // Resetowanie bonusowego zasiêgu (z radaru) i update
        for (auto& tower : activeTowers) {
            tower->resetBonusRange();
        }

        for (auto& tower : activeTowers) tower->updateTower(dt, waveManager.getEnemies(), activeProjectiles, playerStats, activeTowers);

        for (auto& proj : activeProjectiles) {
            proj->update(dt);
            if (proj->hasReached()) {
                float splash = proj->getSplashRadius();
                bool isSplash = splash > 0.f;
                float hitRadiusSq = isSplash ? (splash * splash) : (45.f * 45.f);

                for (auto& enemy : waveManager.getEnemies()) {
                    if (enemy->isDead()) continue;

                    sf::Vector2f ePos = enemy->getPosition();
                    sf::Vector2f pPos = proj->getPosition();
                    float distSq = (ePos.x - pPos.x) * (ePos.x - pPos.x) + (ePos.y - pPos.y) * (ePos.y - pPos.y);

                    if (distSq <= hitRadiusSq) {
                        enemy->takeDamage(proj->getDamage(), proj->getDamageType());

                        if (proj->getEffect() == ProjectileEffect::SLOW) {
                            enemy->applySlow(0.5f, 2.0f);
                        }
                        else if (proj->getEffect() == ProjectileEffect::POISON) {
                            enemy->applyPoison(8, 3.0f);
                        }

                        if (!isSplash) break;
                    }
                }
            }
        }
        std::erase_if(activeProjectiles, [](const std::unique_ptr<Projectile>& p) { return p->hasReached(); });

        for (auto& btn : shopButtons) {
            if (btn.rect.getGlobalBounds().contains(mousePos)) {
                btn.rect.setOutlineColor(playerStats.getGold() >= btn.cost ? sf::Color::Green : sf::Color::Red);
            }
            else btn.rect.setOutlineColor(sf::Color::White);

            if (playerStats.getGold() < btn.cost) {
                btn.text.setFillColor(sf::Color(100, 100, 100)); btn.costText.setFillColor(sf::Color(100, 100, 100));
            }
            else {
                btn.text.setFillColor(sf::Color::White); btn.costText.setFillColor(sf::Color::Yellow);
            }
        }

        if (isPlacingTower) {
            ghostTower.setPosition(mousePos);
            if (canPlaceTower(mousePos)) {
                ghostTower.setOutlineThickness(0.f);
            }
            else {
                ghostTower.setFillColor(sf::Color(100, 100, 100, 150));
                ghostTower.setOutlineThickness(2.f);
                ghostTower.setOutlineColor(sf::Color::Red);
            }
        }

        if (playerStats.autoWaveStart && !waveManager.isWaveActive() && waveManager.getCurrentWave() < availableMaps[currentMapIndex].totalWaves && !hasWon) {
            autoWaveTimer -= dt;
            if (autoWaveTimer <= 0.f) {
                waveManager.startNextWave();
                autoWaveTimer = autoWaveDelay;
            }
        }
        else if (waveManager.isWaveActive()) {
            autoWaveTimer = autoWaveDelay;
        }
    }
}


void Game::drawGameplayHUD() {
    window.draw(hudSidebar);

    statsText.setString(std::format("Dowodca: {}\nZloto: {} G\nPunkty: {}\nMnoznik: {:.1f}x\nBaza: {} / {}\nFala: {} / {}",
        playerStats.nickname, playerStats.getGold(), playerStats.getScore(), playerStats.scoreMultiplier, playerStats.getHealth(), playerStats.maxBaseHealth,
        waveManager.getCurrentWave(), availableMaps[currentMapIndex].totalWaves));
    window.draw(statsText);

    window.draw(shopTitleText);

    for (const auto& btn : shopButtons) {
        window.draw(btn.rect);
        window.draw(btn.text);
        window.draw(btn.costText);
    }

    if (!waveManager.isWaveActive() && waveManager.getCurrentWave() < availableMaps[currentMapIndex].totalWaves) {
        sf::Text info(font);

        if (playerStats.autoWaveStart) {
            info.setString(std::format("Nastepna fala za: {:.1f}s", autoWaveTimer));
            info.setFillColor(sf::Color::Cyan);
        }
        else {
            info.setString("Wcisnij SPACJE, aby rozpoczac fale!");
            info.setFillColor(sf::Color::White);
        }

        info.setCharacterSize(28);
        info.setPosition({ window.getSize().x / 2.f - 250.f, 20.f });
        info.setOutlineThickness(2.f);
        info.setOutlineColor(sf::Color::Black);
        window.draw(info);
    }
}

void Game::render() {
    window.clear(sf::Color(20, 20, 30));

    float centerX = window.getSize().x / 2.0f;

    if (currentState == GameState::LOGIN) {
        tytul.setPosition({ centerX, 150.f });
        loginPromptText.setPosition({ centerX - 250.f, 350.f });
        inputBox.setPosition({ centerX - 200.f, 400.f });
        inputText.setPosition({ centerX - 180.f, 405.f });
        errorText.setPosition({ centerX - 200.f, 480.f });

        window.draw(backgroundSprite); window.draw(tytul); window.draw(loginPromptText);
        window.draw(inputBox); window.draw(inputText); window.draw(errorText);
    }
    else if (currentState == GameState::MAIN_MENU) {
        tytul.setPosition({ centerX, 150.f });
        window.draw(backgroundSprite); window.draw(tytul);
    }
    else if (currentState == GameState::MAP_SELECTION) {
        window.draw(backgroundSprite);
        sf::Text titleText(font);
        titleText.setString("WYBIERZ MAPE DO OBRONY");
        titleText.setCharacterSize(60);
        titleText.setFillColor(sf::Color::Yellow);
        sf::FloatRect bounds = titleText.getLocalBounds();
        titleText.setOrigin({ bounds.position.x + bounds.size.x / 2.0f, bounds.position.y + bounds.size.y / 2.0f });
        titleText.setPosition({ centerX, 150.f });
        window.draw(titleText);
    }
    else if (currentState == GameState::SCOREBOARD) {
        window.draw(backgroundSprite);

        sf::Text titleText(font);
        titleText.setString("TABLICA WYNIKOW");
        titleText.setCharacterSize(60);
        titleText.setFillColor(sf::Color::Yellow);
        sf::FloatRect bounds = titleText.getLocalBounds();
        titleText.setOrigin({ bounds.position.x + bounds.size.x / 2.0f, bounds.position.y + bounds.size.y / 2.0f });
        titleText.setPosition({ centerX, 100.f });
        window.draw(titleText);

        float startY = 200.f;
        for (size_t i = 0; i < topScores.size(); ++i) {
            sf::Text scoreText(font);
            scoreText.setString(std::format("{}. {} - {} pkt", i + 1, topScores[i].name, topScores[i].score));
            scoreText.setCharacterSize(45);

            if (i == 0) scoreText.setFillColor(sf::Color(255, 215, 0));
            else if (i == 1) scoreText.setFillColor(sf::Color(192, 192, 192));
            else if (i == 2) scoreText.setFillColor(sf::Color(205, 127, 50));
            else scoreText.setFillColor(sf::Color::White);

            sf::FloatRect sb = scoreText.getLocalBounds();
            scoreText.setOrigin({ sb.position.x + sb.size.x / 2.0f, sb.position.y + sb.size.y / 2.0f });
            scoreText.setPosition({ centerX, startY + (i * 50.f) });
            window.draw(scoreText);
        }
    }
    else if (currentState == GameState::OPTIONS) {
        window.draw(backgroundSprite);
        sf::Text titleText(font);
        titleText.setString(std::format("MODYFIKATORY (Ostatni mnoznik: {:.1f}x)", playerStats.scoreMultiplier));
        titleText.setCharacterSize(45);
        titleText.setFillColor(sf::Color::Cyan);
        sf::FloatRect bounds = titleText.getLocalBounds();
        titleText.setOrigin({ bounds.position.x + bounds.size.x / 2.0f, bounds.position.y + bounds.size.y / 2.0f });
        titleText.setPosition({ centerX, 150.f });
        window.draw(titleText);
    }
    else if (currentState == GameState::SETTINGS) {
        window.draw(backgroundSprite);
        sf::Text titleText(font);
        titleText.setString("USTAWIENIA EKRANU");
        titleText.setCharacterSize(60);
        titleText.setFillColor(sf::Color::Yellow);
        sf::FloatRect bounds = titleText.getLocalBounds();
        titleText.setOrigin({ bounds.position.x + bounds.size.x / 2.0f, bounds.position.y + bounds.size.y / 2.0f });
        titleText.setPosition({ centerX, 150.f });
        window.draw(titleText);
    }
    else if (currentState == GameState::GAME_OVER) {
        window.draw(backgroundSprite);

        sf::Text endText(font);
        endText.setString(hasWon ? "WYGRANA! OBRONIONO BAZE!" : "PRZEGRANA! BAZA ZNISZCZONA");
        endText.setCharacterSize(60);
        endText.setFillColor(hasWon ? sf::Color::Green : sf::Color::Red);
        endText.setOutlineThickness(4.f);
        endText.setOutlineColor(sf::Color::Black);
        sf::FloatRect bounds = endText.getLocalBounds();
        endText.setOrigin({ bounds.position.x + bounds.size.x / 2.0f, bounds.position.y + bounds.size.y / 2.0f });
        endText.setPosition({ centerX, 100.f });
        window.draw(endText);

        sf::Text scoreText(font);
        scoreText.setString(std::format("Tvoj Wynik: {}", playerStats.getScore()));
        scoreText.setCharacterSize(40);
        scoreText.setFillColor(sf::Color::Yellow);
        sf::FloatRect sb = scoreText.getLocalBounds();
        scoreText.setOrigin({ sb.position.x + sb.size.x / 2.0f, sb.position.y + sb.size.y / 2.0f });
        scoreText.setPosition({ centerX, 160.f });
        window.draw(scoreText);

        float startY = 230.f;
        for (size_t i = 0; i < std::min(topScores.size(), static_cast<size_t>(5)); ++i) {
            sf::Text st(font);
            st.setString(std::format("{}. {} - {} pkt", i + 1, topScores[i].name, topScores[i].score));
            st.setCharacterSize(35);

            if (i == 0) st.setFillColor(sf::Color(255, 215, 0));
            else if (i == 1) st.setFillColor(sf::Color(192, 192, 192));
            else if (i == 2) st.setFillColor(sf::Color(205, 127, 50));
            else st.setFillColor(sf::Color::White);

            sf::FloatRect sb2 = st.getLocalBounds();
            st.setOrigin({ sb2.position.x + sb2.size.x / 2.0f, sb2.position.y + sb2.size.y / 2.0f });
            st.setPosition({ centerX, startY + (i * 45.f) });
            window.draw(st);
        }
    }
    else if (currentState == GameState::GAMEPLAY || currentState == GameState::PAUSE) {
        window.clear(availableMaps[currentMapIndex].bgColor);

        for (const auto& obs : availableMaps[currentMapIndex].obstacles) {
            window.draw(obs);
        }

        window.draw(pathLines);

        waveManager.drawEnemies(window);
        for (const auto& tower : activeTowers) tower->draw(window);
        for (const auto& proj : activeProjectiles) proj->draw(window);

        if (isPlacingTower) window.draw(ghostTower);

        drawGameplayHUD();

        if (currentState == GameState::PAUSE) {
            sf::RectangleShape overlay(sf::Vector2f(window.getSize().x, window.getSize().y));
            overlay.setFillColor(sf::Color(0, 0, 0, 150));
            window.draw(overlay);

            sf::Text pauseText(font);
            pauseText.setString("PAUZA");
            pauseText.setCharacterSize(80);
            pauseText.setFillColor(sf::Color::Yellow);
            sf::FloatRect bounds = pauseText.getLocalBounds();
            pauseText.setOrigin({ bounds.position.x + bounds.size.x / 2.0f, bounds.position.y + bounds.size.y / 2.0f });
            pauseText.setPosition({ centerX, 150.f });
            window.draw(pauseText);
        }
    }

    if (currentState != GameState::GAMEPLAY) menuManager.draw(window);
    window.display();
}
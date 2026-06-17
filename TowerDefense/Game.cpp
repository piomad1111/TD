#include "Game.h"
#include <regex> 

Game::Game()
    : currentState(GameState::LOGIN),
    menuManager(font),
    backgroundSprite(backgroundTexture),
    tytul(font),
    loginPromptText(font),
    inputText(font),
    errorText(font),
    welcomeText(font),
    statsText(font),
    shopTitleText(font)
{
    sf::VideoMode desktopMode = sf::VideoMode::getDesktopMode();
    window.create(desktopMode, "Tower Defense", sf::State::Fullscreen);

    if (!backgroundTexture.loadFromFile("bgBlue.jpg")) {
        std::cerr << "Blad wczytywania tekstury bgBlue.jpg! Tlo bedzie czarne." << std::endl;
    }

    if (!font.openFromFile("Roboto-Black.ttf")) {
        std::cerr << "Blad wczytywania czcionki!" << std::endl;
    }

    // --- TESTOWA ŒCIE¯KA DLA WROGÓW ---
    testPath = {
        {-50.f, 200.f}, {400.f, 200.f}, {400.f, 600.f}, {1000.f, 600.f}, {1000.f, 300.f}, {1600.f, 300.f}
    };
    waveManager.setPath(testPath);

    pathLines.setPrimitiveType(sf::PrimitiveType::LineStrip);
    for (const auto& point : testPath) {
        pathLines.append(sf::Vertex(point, sf::Color(100, 100, 100, 150)));
    }

    // --- INICJALIZACJA MENU LOGOWANIA ---
    tytul.setString("TOWER DEFENSE");
    tytul.setCharacterSize(90);
    tytul.setFillColor(sf::Color::Red);
    sf::FloatRect textRect = tytul.getLocalBounds();
    tytul.setOrigin({ textRect.position.x + textRect.size.x / 2.0f,
                      textRect.position.y + textRect.size.y / 2.0f });
    tytul.setPosition({ desktopMode.size.x / 2.0f, 150.f });

    loginPromptText.setString("Podaj swoj pseudonim (3-12 znakow):");
    loginPromptText.setCharacterSize(30);
    loginPromptText.setPosition({ desktopMode.size.x / 2.0f - 250.f, 350.f });

    inputBox.setSize(sf::Vector2f(400.f, 60.f));
    inputBox.setFillColor(sf::Color(0, 0, 0, 150));
    inputBox.setOutlineThickness(2.f);
    inputBox.setPosition({ desktopMode.size.x / 2.0f - 200.f, 400.f });

    inputText.setCharacterSize(40);
    inputText.setFillColor(sf::Color::Yellow);
    inputText.setPosition({ desktopMode.size.x / 2.0f - 180.f, 405.f });

    errorText.setCharacterSize(25);
    errorText.setFillColor(sf::Color::Red);
    errorText.setPosition({ desktopMode.size.x / 2.0f - 200.f, 480.f });

    // --- INICJALIZACJA INTERFEJSU GRY (HUD) ---
    initGameplayHUD();

    menuManager.initMenu(currentState,
        [this](GameState s) { changeState(s); },
        [this]() { window.close(); }
    );
}

void Game::initGameplayHUD() {
    float windowWidth = static_cast<float>(window.getSize().x);
    float windowHeight = static_cast<float>(window.getSize().y);
    float sidebarWidth = 350.f;

    hudSidebar.setSize({ sidebarWidth, windowHeight });
    hudSidebar.setPosition({ windowWidth - sidebarWidth, 0.f });
    hudSidebar.setFillColor(sf::Color(40, 40, 50, 240));
    hudSidebar.setOutlineThickness(-3.f);
    hudSidebar.setOutlineColor(sf::Color(100, 100, 100));

    statsText.setCharacterSize(24);
    statsText.setFillColor(sf::Color::White);
    statsText.setPosition({ windowWidth - sidebarWidth + 20.f, 20.f });

    shopTitleText.setString("MENU BUDOWY");
    shopTitleText.setCharacterSize(30);
    shopTitleText.setFillColor(sf::Color::Yellow);
    shopTitleText.setPosition({ windowWidth - sidebarWidth + 60.f, 200.f });

    std::vector<std::pair<std::string, int>> towerTypes = {
        {"1. Lucznik", 100},
        {"2. Mag", 150},
        {"3. Armata", 300}
    };

    float startY = 260.f;
    for (size_t i = 0; i < towerTypes.size(); ++i) {
        ShopButton btn(font);
        btn.towerName = towerTypes[i].first;
        btn.cost = towerTypes[i].second;

        btn.rect.setSize({ sidebarWidth - 40.f, 70.f });
        btn.rect.setPosition({ windowWidth - sidebarWidth + 20.f, startY + (i * 90.f) });
        btn.rect.setFillColor(sf::Color(70, 70, 80));
        btn.rect.setOutlineThickness(2.f);

        btn.text.setString(btn.towerName);
        btn.text.setCharacterSize(24);
        btn.text.setPosition({ btn.rect.getPosition().x + 10.f, btn.rect.getPosition().y + 10.f });

        btn.costText.setString("Koszt: " + std::to_string(btn.cost) + " G");
        btn.costText.setCharacterSize(20);
        btn.costText.setFillColor(sf::Color::Yellow);
        btn.costText.setPosition({ btn.rect.getPosition().x + 10.f, btn.rect.getPosition().y + 40.f });

        shopButtons.push_back(btn);
    }

    ghostTower.setSize({ 40.f, 40.f }); // Dopasowano rozmiar ducha do rozmiaru wie¿y
    ghostTower.setOrigin({ 20.f, 20.f });
    ghostTower.setFillColor(sf::Color(255, 255, 255, 128));
}

void Game::changeState(GameState newState) {
    currentState = newState;

    if (currentState == GameState::GAMEPLAY) {
        playerStats.baseHealth = playerStats.maxBaseHealth;
        playerStats.gold = 500;
        isPlacingTower = false;
        activeTowers.clear(); // Czyœcimy wie¿e przy restarcie/nowej grze
        activeProjectiles.clear(); // DODANE: Czyœcimy pociski przy nowej grze
        waveManager.loadWaveDataAsync();
    }

    menuManager.initMenu(currentState,
        [this](GameState s) { changeState(s); },
        [this]() { window.close(); }
    );
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
        if (event->is<sf::Event::Closed>()) {
            window.close();
        }

        if (currentState == GameState::LOGIN) {
            if (const auto* textEvent = event->getIf<sf::Event::TextEntered>()) {
                std::uint32_t unicode = textEvent->unicode;
                if (unicode == 8 && !currentInput.empty()) {
                    currentInput.pop_back();
                    errorText.setString("");
                }
                else if (unicode >= 32 && unicode < 128 && currentInput.size() < 12) {
                    currentInput += static_cast<char>(unicode);
                    errorText.setString("");
                }
            }

            if (const auto* keyEvent = event->getIf<sf::Event::KeyPressed>()) {
                if (keyEvent->scancode == sf::Keyboard::Scancode::Enter) {
                    std::regex nickPattern("^[a-zA-Z0-9]{3,12}$");
                    if (std::regex_match(currentInput, nickPattern)) {
                        playerStats.nickname = currentInput;
                        changeState(GameState::MAIN_MENU);
                    }
                    else {
                        errorText.setString("Blad: Nick musi miec 3-12 znakow!");
                    }
                }
            }
        }
        else if (currentState == GameState::GAMEPLAY) {
            if (const auto* keyEvent = event->getIf<sf::Event::KeyPressed>()) {
                if (keyEvent->scancode == sf::Keyboard::Scancode::Space) {
                    waveManager.startNextWave();
                }
                if (keyEvent->scancode == sf::Keyboard::Scancode::Escape) {
                    if (isPlacingTower) {
                        isPlacingTower = false;
                    }
                    else {
                        changeState(GameState::MAIN_MENU);
                    }
                }
            }

            if (const auto* mouseEvent = event->getIf<sf::Event::MouseButtonPressed>()) {
                sf::Vector2f mousePos(static_cast<float>(mouseEvent->position.x),
                    static_cast<float>(mouseEvent->position.y));
                handleGameplayClicks(mousePos, mouseEvent->button);
            }
        }
        else {
            if (const auto* mouseEvent = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mouseEvent->button == sf::Mouse::Button::Left) {
                    sf::Vector2f mousePos(static_cast<float>(mouseEvent->position.x),
                        static_cast<float>(mouseEvent->position.y));
                    menuManager.handleClick(mousePos);
                }
            }
        }
    }
}

void Game::handleGameplayClicks(sf::Vector2f mousePos, sf::Mouse::Button button) {
    if (button == sf::Mouse::Button::Right) {
        isPlacingTower = false;
        return;
    }

    if (button == sf::Mouse::Button::Left) {
        if (hudSidebar.getGlobalBounds().contains(mousePos)) {
            for (size_t i = 0; i < shopButtons.size(); ++i) {
                if (shopButtons[i].rect.getGlobalBounds().contains(mousePos)) {
                    if (playerStats.getGold() >= shopButtons[i].cost) {
                        isPlacingTower = true;
                        selectedTowerIndex = static_cast<int>(i);

                        if (i == 0) ghostTower.setFillColor(sf::Color(0, 255, 0, 150));
                        if (i == 1) ghostTower.setFillColor(sf::Color(0, 0, 255, 150));
                        if (i == 2) ghostTower.setFillColor(sf::Color(255, 0, 0, 150));
                    }
                    return;
                }
            }
        }
        else if (isPlacingTower) {
            // ZMIANA: Najpierw sprawdzamy rygorystycznie czy miejsce jest dozwolone
            if (canPlaceTower(mousePos)) {
                if (playerStats.spendGold(shopButtons[selectedTowerIndex].cost)) {
                    std::cout << "Zbudowano: " << shopButtons[selectedTowerIndex].towerName
                        << " na x:" << mousePos.x << " y:" << mousePos.y << "\n";

                    if (selectedTowerIndex == 0) {
                        activeTowers.push_back(std::make_unique<ArcherTower>(mousePos));
                    }
                    else if (selectedTowerIndex == 1) {
                        activeTowers.push_back(std::make_unique<MageTower>(mousePos));
                    }
                    else if (selectedTowerIndex == 2) {
                        activeTowers.push_back(std::make_unique<CannonTower>(mousePos));
                    }
                }
                isPlacingTower = false;
            }
        }
    }
}

// NOWE: Algorytmy matematyczne sprawdzaj¹ce mo¿liwoœæ zbudowania wie¿y
bool Game::canPlaceTower(sf::Vector2f pos) {
    // 1. Sprawdzanie kolizji z innymi wie¿ami (Odleg³oœæ œrodków > 40px)
    for (const auto& tower : activeTowers) {
        sf::Vector2f tPos = tower->getPosition();
        float distSq = (pos.x - tPos.x) * (pos.x - tPos.x) + (pos.y - tPos.y) * (pos.y - tPos.y);
        if (distSq < 45.f * 45.f) return false;
    }

    // 2. Sprawdzanie kolizji ze œcie¿k¹ (Najkrótszy dystans od punktu do odcinka)
    for (size_t i = 0; i < testPath.size() - 1; ++i) {
        sf::Vector2f a = testPath[i];
        sf::Vector2f b = testPath[i + 1];

        sf::Vector2f ab = b - a;
        sf::Vector2f ap = pos - a;

        float proj = ap.x * ab.x + ap.y * ab.y;
        float abLenSq = ab.x * ab.x + ab.y * ab.y;

        float t = proj / abLenSq;
        t = std::max(0.0f, std::min(1.0f, t)); // Przyciêcie t do zakresu 0.0 - 1.0

        sf::Vector2f closest = a + ab * t;
        sf::Vector2f d = pos - closest;
        float distToPathSq = d.x * d.x + d.y * d.y;

        if (distToPathSq < 30.f * 30.f) return false; // Odleg³oœæ 30px od osi œcie¿ki to zakazana strefa
    }

    // 3. Sprawdzanie czy nie nachodzi na HUD po prawej (350px szerokoœci + ma³y margines)
    if (pos.x > window.getSize().x - 370.f) return false;

    return true;
}

void Game::update(float dt) {
    sf::Vector2i mousePosI = sf::Mouse::getPosition(window);
    sf::Vector2f mousePos(static_cast<float>(mousePosI.x), static_cast<float>(mousePosI.y));

    menuManager.update(mousePos);

    if (currentState == GameState::LOGIN) {
        inputText.setString(currentInput + "_");
    }
    else if (currentState == GameState::GAMEPLAY) {
        waveManager.updateEnemies(dt, playerStats);

        // ZMIANA: Wie¿e dostaj¹ referencjê do pocisków
        for (auto& tower : activeTowers) {
            tower->updateTower(dt, waveManager.getEnemies(), activeProjectiles);
        }

        // NOWE: Aktualizacja pocisków i wykrywanie ich uderzeñ (Hitbox)
        for (auto& proj : activeProjectiles) {
            proj->update(dt);
            if (proj->hasReached()) {
                float splash = proj->getSplashRadius();
                bool isSplash = splash > 0.f;
                // Jeœli splash jest 0 (np. £ucznik), u¿ywamy sta³ego, ma³ego promienia uderzenia
                float hitRadiusSq = isSplash ? (splash * splash) : (45.f * 45.f);

                for (auto& enemy : waveManager.getEnemies()) {
                    if (enemy->isDead()) continue;
                    sf::Vector2f ePos = enemy->getPosition();
                    sf::Vector2f pPos = proj->getPosition();
                    float distSq = (ePos.x - pPos.x) * (ePos.x - pPos.x) + (ePos.y - pPos.y) * (ePos.y - pPos.y);

                    if (distSq <= hitRadiusSq) {
                        enemy->takeDamage(proj->getDamage());
                        if (!isSplash) break; // Zwyk³y pocisk razi tylko pierwszy napotkany cel
                    }
                }
            }
        }

        // C++20 Erase-If - usuwamy pociski, które dotar³y do celu
        std::erase_if(activeProjectiles, [](const std::unique_ptr<Projectile>& p) { return p->hasReached(); });

        for (auto& btn : shopButtons) {
            if (btn.rect.getGlobalBounds().contains(mousePos)) {
                if (playerStats.getGold() >= btn.cost) {
                    btn.rect.setOutlineColor(sf::Color::Green);
                }
                else {
                    btn.rect.setOutlineColor(sf::Color::Red);
                }
            }
            else {
                btn.rect.setOutlineColor(sf::Color::White);
            }

            if (playerStats.getGold() < btn.cost) {
                btn.text.setFillColor(sf::Color(100, 100, 100));
                btn.costText.setFillColor(sf::Color(100, 100, 100));
            }
            else {
                btn.text.setFillColor(sf::Color::White);
                btn.costText.setFillColor(sf::Color::Yellow);
            }
        }

        if (isPlacingTower) {
            ghostTower.setPosition(mousePos);

            // ZMIANA: Kolor zale¿y od tego, czy w danym miejscu legalnie wolno postawiæ obiekt
            if (canPlaceTower(mousePos)) {
                ghostTower.setOutlineThickness(0.f);
                if (selectedTowerIndex == 0) ghostTower.setFillColor(sf::Color(0, 255, 0, 150));
                else if (selectedTowerIndex == 1) ghostTower.setFillColor(sf::Color(0, 0, 255, 150));
                else if (selectedTowerIndex == 2) ghostTower.setFillColor(sf::Color(255, 0, 0, 150));
            }
            else {
                ghostTower.setFillColor(sf::Color(100, 100, 100, 150)); // Szary wskaŸnik blokady
                ghostTower.setOutlineThickness(2.f);
                ghostTower.setOutlineColor(sf::Color::Red); // WyraŸny czerwony obrys b³êdu
            }
        }

        if (playerStats.getHealth() <= 0) {
            changeState(GameState::GAME_OVER);
        }
    }
}

void Game::drawGameplayHUD() {
    window.draw(hudSidebar);

    std::string statsStr = std::format(
        "Dowodca: {}\n\nZloto: {} G\nZdrowie Bazy: {} / {}\nFala: {}",
        playerStats.nickname,
        playerStats.getGold(),
        playerStats.getHealth(),
        playerStats.maxBaseHealth,
        waveManager.getCurrentWave()
    );
    statsText.setString(statsStr);
    window.draw(statsText);

    window.draw(shopTitleText);
    for (const auto& btn : shopButtons) {
        window.draw(btn.rect);
        window.draw(btn.text);
        window.draw(btn.costText);
    }

    if (!waveManager.isWaveActive()) {
        sf::Text info(font);
        info.setString("Wcisnij SPACJE, aby rozpoczac fale!");
        info.setCharacterSize(28);
        info.setFillColor(sf::Color::White);
        info.setPosition({ window.getSize().x / 2.f - 250.f, 20.f });
        info.setOutlineThickness(2.f);
        info.setOutlineColor(sf::Color::Black);
        window.draw(info);
    }
}

void Game::render() {
    window.clear(sf::Color(20, 20, 30));

    if (currentState == GameState::LOGIN) {
        window.draw(backgroundSprite);
        window.draw(tytul);
        window.draw(loginPromptText);
        window.draw(inputBox);
        window.draw(inputText);
        window.draw(errorText);
    }
    else if (currentState == GameState::MAIN_MENU) {
        window.draw(backgroundSprite);
        window.draw(tytul);
    }
    else if (currentState == GameState::GAMEPLAY) {
        window.clear(sf::Color(34, 139, 34));

        window.draw(pathLines);

        waveManager.drawEnemies(window);

        for (const auto& tower : activeTowers) {
            tower->draw(window);
        }

        // DODANE: Rysowanie lec¹cych pocisków
        for (const auto& proj : activeProjectiles) {
            proj->draw(window);
        }

        if (isPlacingTower) {
            window.draw(ghostTower);
        }

        drawGameplayHUD();
    }

    if (currentState != GameState::GAMEPLAY) {
        menuManager.draw(window);
    }

    window.display();
}
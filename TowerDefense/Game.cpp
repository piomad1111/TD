#include "Game.h"
#include <regex>
#include <cmath>
#include "TextureManager.h"

// Anonimowa przestrzeñ nazw dla zmiennych globalnych tego pliku (strona tutoriala)
namespace {
    int tutorialPage = 0;
}

Game::Game()
    : currentState(GameState::LOGIN), menuManager(font), backgroundSprite(backgroundTexture),
    tytul(font), loginPromptText(font), inputText(font), errorText(font),
    statsText(font), shopTitleText(font)
{
    sf::VideoMode desktopMode = sf::VideoMode::getDesktopMode();
    window.create(desktopMode, "Tower Defense", sf::State::Fullscreen);

    // SFML 3: Wymuszamy logiczny widok gry (Kamerê) 1920x1080 niezale¿nie od wymiarów fizycznych okna
    window.setView(sf::View(sf::Vector2f(1920.f / 2.f, 1080.f / 2.f), sf::Vector2f(1920.f, 1080.f)));

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
    availableMaps.clear();

    // --- FUNKCJE POMOCNICZE DO KAFELKOWANIA TEKSTUR ---

    // Konfiguracja t³a z obs³ug¹ kafelkowania
    auto setupBackground = [](LevelMap& map, const std::string& texName = "") {
        map.bgShape.setSize(sf::Vector2f(1920.f, 1080.f));
        if (!texName.empty()) {
            sf::Texture& tex = TextureManager::getInstance().get(texName);
            tex.setRepeated(true);
            map.bgShape.setTexture(&tex);
            map.bgShape.setTextureRect(sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(1920, 1080)));
            map.bgShape.setFillColor(sf::Color::White); // Biel odzyskuje oryginalne kolory tekstury
        }
        else {
            map.bgShape.setFillColor(map.bgColor);
        }
        };

    // Konfiguracja tekstury na przeszkodê
    auto setupObstacle = [](sf::RectangleShape& obs, const std::string& texName = "") {
        if (!texName.empty()) {
            sf::Texture& tex = TextureManager::getInstance().get(texName);
            tex.setRepeated(true);
            obs.setTexture(&tex);
            obs.setTextureRect(sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(static_cast<int>(obs.getSize().x), static_cast<int>(obs.getSize().y))));
            obs.setFillColor(sf::Color::White);
        }
        };

    // Funkcja buduj¹ca grub¹ œcie¿kê z okr¹g³ymi ³¹czeniami i mo¿liwoœci¹ otexturowania
    auto buildThickPath = [](LevelMap& map, const std::string& texName = "") {
        float pathThickness = 80.f; // Grubosc sciezki
        sf::Texture* tex = nullptr;

        if (!texName.empty()) {
            tex = &TextureManager::getInstance().get(texName);
            tex->setRepeated(true);
        }

        for (size_t i = 0; i < map.path.size() - 1; ++i) {
            sf::Vector2f p1 = map.path[i];
            sf::Vector2f p2 = map.path[i + 1];
            sf::Vector2f diff = p2 - p1;
            float length = std::sqrt(diff.x * diff.x + diff.y * diff.y);

            float angleRads = std::atan2(diff.y, diff.x);

            sf::RectangleShape segment(sf::Vector2f(length, pathThickness));
            segment.setOrigin(sf::Vector2f(0.f, pathThickness / 2.0f));
            segment.setPosition(p1);
            segment.setRotation(sf::radians(angleRads));

            if (tex) {
                segment.setTexture(tex);
                segment.setTextureRect(sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(static_cast<int>(length), static_cast<int>(pathThickness))));
                segment.setFillColor(sf::Color::White);
            }
            else {
                segment.setFillColor(map.pathColor);
            }
            map.pathShapes.push_back(segment);

            // Okr¹g³e z³¹cze
            sf::CircleShape joint(pathThickness / 2.0f);
            joint.setOrigin(sf::Vector2f(pathThickness / 2.0f, pathThickness / 2.0f));
            joint.setPosition(p1);
            if (tex) {
                joint.setTexture(tex);
                joint.setTextureRect(sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(static_cast<int>(pathThickness), static_cast<int>(pathThickness))));
                joint.setFillColor(sf::Color::White);
            }
            else {
                joint.setFillColor(map.pathColor);
            }
            map.pathJoints.push_back(joint);
        }
        // Zamkniêcie ostatniego punktu okrêgiem
        if (!map.path.empty()) {
            sf::CircleShape joint(pathThickness / 2.0f);
            joint.setOrigin(sf::Vector2f(pathThickness / 2.0f, pathThickness / 2.0f));
            joint.setPosition(map.path.back());
            if (tex) {
                joint.setTexture(tex);
                joint.setTextureRect(sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(static_cast<int>(pathThickness), static_cast<int>(pathThickness))));
                joint.setFillColor(sf::Color::White);
            }
            else {
                joint.setFillColor(map.pathColor);
            }
            map.pathJoints.push_back(joint);
        }
        };


    // ===============================================
    // MAPA £ATWA
    // ===============================================
    // Zmienne do ³adowania tekstur (Wpisz nazwy plików z folderu projektu, np. "grass.jpg")
    std::string easyBgTex = "";
    std::string easyPathTex = "";
    std::string easyObsTex = "";

    LevelMap easy;
    easy.name = "Latwa (10 fal) [+0.0x pkt]";
    easy.path = { {-50.f, 200.f}, {300.f, 200.f}, {300.f, 600.f}, {800.f, 600.f}, {800.f, 300.f}, {1600.f, 300.f} };
    easy.totalWaves = 10;
    easy.difficulty = 0;
    easy.bgColor = sf::Color(34, 139, 34);
    easy.pathColor = sf::Color(168, 122, 81);

    setupBackground(easy, easyBgTex);

    sf::RectangleShape obs1(sf::Vector2f(200.f, 200.f));
    obs1.setPosition({ 400.f, 300.f });
    obs1.setFillColor(sf::Color(105, 105, 105));
    obs1.setOutlineThickness(2.f);
    obs1.setOutlineColor(sf::Color::Black);

    setupObstacle(obs1, easyObsTex);
    easy.obstacles.push_back(obs1);

    buildThickPath(easy, easyPathTex);
    availableMaps.push_back(easy);

    // ===============================================
    // MAPA ŒREDNIA
    // ===============================================
    // Zmienne do ³adowania tekstur (Wpisz nazwy plików z folderu projektu, np. "sand.jpg")
    std::string mediumBgTex = "";
    std::string mediumPathTex = "";
    std::string mediumObsTex = "";

    LevelMap medium;
    medium.name = "Srednia (15 fal) [+0.5x pkt]";
    medium.path = { {-50.f, 500.f}, {200.f, 500.f}, {200.f, 100.f}, {700.f, 100.f}, {700.f, 700.f}, {1200.f, 700.f}, {1200.f, 400.f}, {1600.f, 400.f} };
    medium.totalWaves = 15;
    medium.difficulty = 1;
    medium.bgColor = sf::Color(139, 115, 85);
    medium.pathColor = sf::Color(138, 142, 145);

    setupBackground(medium, mediumBgTex);

    sf::RectangleShape obs2(sf::Vector2f(400.f, 150.f));
    obs2.setPosition({ 250.f, 300.f });
    obs2.setFillColor(sf::Color(80, 80, 80));
    obs2.setOutlineThickness(2.f);
    obs2.setOutlineColor(sf::Color::Black);
    setupObstacle(obs2, mediumObsTex);

    sf::RectangleShape obs3(sf::Vector2f(200.f, 300.f));
    obs3.setPosition({ 800.f, 200.f });
    obs3.setFillColor(sf::Color(80, 80, 80));
    obs3.setOutlineThickness(2.f);
    obs3.setOutlineColor(sf::Color::Black);
    setupObstacle(obs3, mediumObsTex);

    medium.obstacles.push_back(obs2);
    medium.obstacles.push_back(obs3);

    buildThickPath(medium, mediumPathTex);
    availableMaps.push_back(medium);

    // ===============================================
    // MAPA TRUDNA
    // ===============================================
    // Zmienne do ³adowania tekstur (Wpisz nazwy plików z folderu projektu, np. "lava_bg.jpg")
    std::string hardBgTex = "";
    std::string hardPathTex = "";
    std::string hardObsTex = "";

    LevelMap hard;
    hard.name = "Trudna (20 fal) [+1.0x pkt]";
    hard.path = { {800.f, -50.f}, {800.f, 300.f}, {200.f, 300.f}, {200.f, 800.f}, {1300.f, 800.f}, {1300.f, 150.f}, {1600.f, 150.f} };
    hard.totalWaves = 20;
    hard.difficulty = 2;
    hard.bgColor = sf::Color(60, 60, 60);
    hard.pathColor = sf::Color(180, 50, 20);

    setupBackground(hard, hardBgTex);

    sf::RectangleShape obs4(sf::Vector2f(600.f, 400.f));
    obs4.setPosition({ 350.f, 350.f });
    obs4.setFillColor(sf::Color(30, 30, 30));
    obs4.setOutlineThickness(2.f);
    obs4.setOutlineColor(sf::Color::Black);
    setupObstacle(obs4, hardObsTex);
    hard.obstacles.push_back(obs4);

    buildThickPath(hard, hardPathTex);
    availableMaps.push_back(hard);
}

void Game::initGameplayHUD() {
    shopButtons.clear();

    // Sztywno ustalamy ROZDZIELCZOŒÆ LOGICZN¥, bez pytania okna o jego fizyczny wymiar
    float windowWidth = 1920.f;
    float windowHeight = 1080.f;
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

    // Powiêkszony duch wie¿y dopasowany do wizualnego rozmiaru wie¿ (100x100)
    ghostTower.setSize({ 50.f, 70.f });
    ghostTower.setOrigin({ 20.f, 20.f });
    ghostTower.setFillColor(sf::Color(255, 255, 255, 128));
}


void Game::changeState(GameState newState) {
    // --- INTELIGENTNY SYSTEM POWROTU ---
    static GameState savedReturnState = GameState::MAIN_MENU;

    if (currentState == GameState::MAIN_MENU || currentState == GameState::PAUSE) {
        savedReturnState = currentState;
    }

    GameState targetReturn = savedReturnState;

    currentState = newState;
    menuManager.clearButtons();

    // CENTROWANIE OPARTE NA STA£EJ ROZDZIELCZOŒCI LOGICZNEJ
    float centerX = 1920.f / 2.0f;

    if (currentState == GameState::MAIN_MENU) {
        menuManager.addButton("Graj", { centerX, 250.f }, [this]() { changeState(GameState::MAP_SELECTION); });
        menuManager.addButton("Tablica Wynikow", { centerX, 350.f }, [this]() { changeState(GameState::SCOREBOARD); });
        menuManager.addButton("Modyfikatory", { centerX, 450.f }, [this]() { changeState(GameState::OPTIONS); });
        menuManager.addButton("Ustawienia ", { centerX, 550.f }, [this]() { changeState(GameState::SETTINGS); });
        menuManager.addButton("Tutorial", { centerX, 650.f }, [this]() { changeState(GameState::TUTORIAL); });
        menuManager.addButton("Wyjscie", { centerX, 750.f }, [this]() { window.close(); });
    }
    else if (currentState == GameState::MAP_SELECTION) {
        menuManager.addButton(availableMaps[0].name, { centerX, 300.f }, [this]() { startGame(0); });
        menuManager.addButton(availableMaps[1].name, { centerX, 400.f }, [this]() { startGame(1); });
        menuManager.addButton(availableMaps[2].name, { centerX, 500.f }, [this]() { startGame(2); });
        menuManager.addButton("Powrot", { centerX, 700.f }, [this]() { changeState(GameState::MAIN_MENU); });
    }
    else if (currentState == GameState::SCOREBOARD) {
        topScores = PlayerStats::loadLeaderboard();
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
        menuManager.addButton("Powrot", { centerX, 650.f }, [this, targetReturn]() { changeState(targetReturn); });
    }
    else if (currentState == GameState::SETTINGS) {
        menuManager.addButton("1280x720 (Okno)", { centerX, 300.f }, [this]() {
            window.create(sf::VideoMode({ 1280, 720 }), "Tower Defense", sf::Style::Close);
            window.setView(sf::View(sf::Vector2f(1920.f / 2.f, 1080.f / 2.f), sf::Vector2f(1920.f, 1080.f)));
            changeState(GameState::SETTINGS);
            });
        menuManager.addButton("1920x1080 (Okno)", { centerX, 400.f }, [this]() {
            window.create(sf::VideoMode({ 1920, 1080 }), "Tower Defense", sf::Style::Close);
            window.setView(sf::View(sf::Vector2f(1920.f / 2.f, 1080.f / 2.f), sf::Vector2f(1920.f, 1080.f)));
            changeState(GameState::SETTINGS);
            });
        menuManager.addButton("Pelny Ekran", { centerX, 500.f }, [this]() {
            window.create(sf::VideoMode::getDesktopMode(), "Tower Defense", sf::State::Fullscreen);
            window.setView(sf::View(sf::Vector2f(1920.f / 2.f, 1080.f / 2.f), sf::Vector2f(1920.f, 1080.f)));
            changeState(GameState::SETTINGS);
            });
        menuManager.addButton("Powrot", { centerX, 700.f }, [this, targetReturn]() { changeState(targetReturn); });
    }
    else if (currentState == GameState::TUTORIAL) {
        // Szerzej rozstawione przyciski nawigacyjne
        menuManager.addButton("Poprzednia", { centerX - 450.f, 950.f }, [this]() {
            tutorialPage = 0; changeState(GameState::TUTORIAL);
            });
        menuManager.addButton("Powrot", { centerX, 950.f }, [this, targetReturn]() {
            changeState(targetReturn);
            });
        menuManager.addButton("Nastepna", { centerX + 450.f, 950.f }, [this]() {
            tutorialPage = 1; changeState(GameState::TUTORIAL);
            });
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

    playerStats.mapDifficultyMultiplier = map.difficulty * 0.5f;
    playerStats.baseHealth = playerStats.modifierFragileBase ? 50 : playerStats.maxBaseHealth;
    playerStats.gold = playerStats.modifierLessGold ? 500 : 1000;
    playerStats.score = 0;

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
                    else changeState(GameState::PAUSE);
                }
            }
            if (const auto* mouseEvent = event->getIf<sf::Event::MouseButtonPressed>()) {
                // TRANSLACJA MYSZKI - T³umaczy fizyczne klikniêcia w oknie na nasz¹ wirtualn¹ mapê 1920x1080
                sf::Vector2f mousePos = window.mapPixelToCoords(mouseEvent->position);
                handleGameplayClicks(mousePos, mouseEvent->button);
            }
        }
        else if (currentState == GameState::PAUSE) {
            if (const auto* keyEvent = event->getIf<sf::Event::KeyPressed>()) {
                if (keyEvent->scancode == sf::Keyboard::Scancode::Escape) {
                    changeState(GameState::GAMEPLAY);
                }
            }
            if (const auto* mouseEvent = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mouseEvent->button == sf::Mouse::Button::Left) {
                    sf::Vector2f mousePos = window.mapPixelToCoords(mouseEvent->position);
                    menuManager.handleClick(mousePos);
                }
            }
        }
        else {
            if (const auto* mouseEvent = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mouseEvent->button == sf::Mouse::Button::Left) {
                    sf::Vector2f mousePos = window.mapPixelToCoords(mouseEvent->position);
                    menuManager.handleClick(mousePos);
                }
            }
        }
    }
}

void Game::handleGameplayClicks(sf::Vector2f mousePos, sf::Mouse::Button button) {
    if (button == sf::Mouse::Button::Right) {
        isPlacingTower = false;

        // --- LOGIKA SPRZEDA¯Y WIE¯Y (PPM) ---
        int currentWv = waveManager.getCurrentWave();
        int totalWvs = availableMaps[currentMapIndex].totalWaves;

        // Zablokuj mo¿liwoœæ sprzeda¿y wie¿y na 5 rund przed koñcem gry
        if (currentWv <= totalWvs - 5) {
            for (auto it = activeTowers.begin(); it != activeTowers.end(); ++it) {
                sf::Vector2f tPos = (*it)->getPosition();
                float distSq = (mousePos.x - tPos.x) * (mousePos.x - tPos.x) + (mousePos.y - tPos.y) * (mousePos.y - tPos.y);

                // Sprawdzamy czy trafiliœmy myszk¹ w hitbox wie¿y
                if (distSq <= 45.f * 45.f) {
                    playerStats.gold += (*it)->getCost() / 2; // Zwrot po³owy kosztów
                    activeTowers.erase(it); // Usuniêcie wie¿y
                    break;
                }
            }
        }
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
    // Opieramy limit ekranu na sztywnej logice - ekran ZAWSZE ma wymiar wirtualny 1920x1080
    if (pos.x > 1920.f - 370.f) return false;

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

        // Zmieniony margines dla grubszej œcie¿ki (40px po³owa œcie¿ki + 20px wie¿y = 60px)
        if (distToPathSq < 60.f * 60.f) return false;
    }

    // Dostosowany rozmiar kolizji (100x100)
    sf::FloatRect towerBounds(sf::Vector2f(pos.x - 50.f, pos.y - 50.f), sf::Vector2f(100.f, 100.f));
    for (const auto& obs : availableMaps[currentMapIndex].obstacles) {
        if (obs.getGlobalBounds().findIntersection(towerBounds).has_value()) {
            return false;
        }
    }

    return true;
}


void Game::update(float dt) {
    sf::Vector2i mousePosI = sf::Mouse::getPosition(window);
    // TRANSLACJA DLA BIE¯¥CEGO RUCHU MYSZKI
    sf::Vector2f mousePos = window.mapPixelToCoords(mousePosI);

    menuManager.update(mousePos);

    if (currentState == GameState::GAMEPLAY || currentState == GameState::PAUSE) {
        for (auto& tower : activeTowers) {
            sf::Vector2f tPos = tower->getPosition();
            float distSq = (mousePos.x - tPos.x) * (mousePos.x - tPos.x) + (mousePos.y - tPos.y) * (mousePos.y - tPos.y);
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

        for (auto& tower : activeTowers) {
            tower->resetBonusRange();
        }

        for (auto& tower : activeTowers) tower->updateTower(dt, waveManager.getEnemies(), activeProjectiles, playerStats, activeTowers);

        for (auto& proj : activeProjectiles) {

            // Logika naprowadzania pocisku dla Maga
            if (Enemy* target = proj->getHomingTarget()) {
                bool isAlive = false;
                for (const auto& e : waveManager.getEnemies()) {
                    if (e.get() == target && !e->isDead()) {
                        isAlive = true;
                        break;
                    }
                }

                if (isAlive) {
                    proj->setTargetPos(target->getPosition());
                }
                else {
                    proj->clearHomingTarget();
                }
            }

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

    // --- INFORMACJA O SPRZEDA¯Y WIE¯ ---
    sf::Text sellInfo(font);
    sellInfo.setCharacterSize(22);
    sellInfo.setPosition(sf::Vector2f(1920.f - 330.f, 850.f));

    if (waveManager.getCurrentWave() <= availableMaps[currentMapIndex].totalWaves - 5) {
        sellInfo.setString("PPM na wieze:\nSprzedaj (50%)");
        sellInfo.setFillColor(sf::Color(100, 255, 100)); // Zielonkawy
    }
    else {
        sellInfo.setString("Sprzedaz zablokowana\n(Ostatnie 5 fal!)");
        sellInfo.setFillColor(sf::Color(255, 100, 100)); // Czerwonawy
    }
    window.draw(sellInfo);

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
        // Œrodek w oparciu o 1920
        info.setPosition({ 1920.f / 2.f - 250.f, 20.f });
        info.setOutlineThickness(2.f);
        info.setOutlineColor(sf::Color::Black);
        window.draw(info);
    }
}

void Game::render() {
    window.clear(sf::Color(20, 20, 30));

    float centerX = 1920.f / 2.0f;

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
    else if (currentState == GameState::TUTORIAL) {
        window.draw(backgroundSprite);

        // TYTU£ ENCYKLOPEDII
        sf::Text titleText(font);
        titleText.setString("LEKSYKON DOWODCY");
        titleText.setCharacterSize(60);
        titleText.setFillColor(sf::Color::Yellow);
        sf::FloatRect bounds = titleText.getLocalBounds();
        titleText.setOrigin({ bounds.position.x + bounds.size.x / 2.0f, bounds.position.y + bounds.size.y / 2.0f });
        titleText.setPosition({ centerX, 80.f });
        window.draw(titleText);

        if (tutorialPage == 0) {
            // STRONA 1: WROGOWIE
            struct TutEnemy { std::string tex; int cols; int rows; std::string name; std::string desc; };
            std::vector<TutEnemy> enemies = {
                {"goblin2.png", 3, 4, "Goblin", "Podstawowy wrog.\nPrzecietne zdrowie i predkosc.\nBrak specjalnych umiejetnosci."},
                {"armor1.png", 3, 4, "Opancerzony", "Posiada gruby pancerz odbijajacy\nzwykle strzaly. Uzyj Armaty lub Pioruna!"},
                {"masked.png", 3, 4, "Zamaskowany", "Niewidzialny dla zwyklych wiez.\nTylko Mag lub Radar moga go wykryc!"},
                {"boss1.png", 3, 4, "Boss", "Ogromny i niezwykle powolny.\nPosiada gigantyczna ilosc punktow zycia!"},
                {"fast1.png", 3, 4, "Szybki", "Maly i bardzo zwinny przeciwnik.\nWieza Lodowa z latwoscia go spowolni."},
                {"tank1.png", 3, 4, "Tank", "Powolny gigant pochlaniajacy obrazenia.\nNiezwykle wrazliwy na trucizne!"}
            };

            for (size_t i = 0; i < enemies.size(); ++i) {
                float xPos = (i % 2 == 0) ? 500.f : 1420.f;
                // Bezpieczne u¿ycie static_cast zapobiega ostrze¿eniom C4244
                float yOffset = static_cast<float>(static_cast<int>(i) / 2) * 220.f;
                float yPos = 230.f + yOffset;

                sf::Texture& t = TextureManager::getInstance().get(enemies[i].tex);
                sf::Sprite spr(t);

                sf::Vector2u tSize = t.getSize();
                if (tSize.x > 0 && tSize.y > 0) {
                    sf::Vector2i fSize(tSize.x / enemies[i].cols, tSize.y / enemies[i].rows);
                    spr.setTextureRect(sf::IntRect(sf::Vector2i(0, 0), fSize));
                    spr.setOrigin(sf::Vector2f(fSize.x / 2.f, fSize.y / 2.f));
                    spr.setScale(sf::Vector2f(120.f / fSize.y, 120.f / fSize.y));
                    spr.setPosition(sf::Vector2f(xPos - 200.f, yPos + 30.f));
                    window.draw(spr);
                }

                sf::Text nText(font);
                nText.setString(enemies[i].name + "\n" + enemies[i].desc);
                nText.setCharacterSize(24);
                nText.setFillColor(sf::Color::White);
                nText.setPosition(sf::Vector2f(xPos - 100.f, yPos));
                window.draw(nText);
            }
        }
        else {
            // STRONA 2: WIE¯E
            struct TutTower { std::string tex; std::string name; std::string desc; };
            std::vector<TutTower> towers = {
                {"tower_archer.png", "Wieza Lucznicza", "Tania i szybka. Dobra na poczatek,\nale slaba przeciwko pancerzom."},
                {"tower_mage.png", "Wieza Maga", "Strzela magicznymi samonaprowadzajacymi\npociskami. Demaskuje wrogow!"},
                {"tower_cannon.png", "Armata", "Wystrzeliwuje kule armatnie zadajace\nobrazenia obszarowe. Niszczy pancerze."},
                {"tower_ice.png", "Wieza Lodowa", "Zadaje znikome obrazenia, ale\nznaczaco spowalnia przeciwnikow."},
                {"tower_poison.png", "Wieza Trucizn", "Zatruwa wrogow, zadajac obrazenia\nw czasie. Zabojcza dla Tankow."},
                {"tower_mine.png", "Kopalnia", "Nie atakuje. Generuje zloto co kilka\nsekund. Kazda kolejna wydobywa o 50% mniej."},
                {"tower_radar.png", "Radar", "Nie atakuje. Zwieksza zasieg\nwiez obok i wykrywa zamaskowanych."},
                {"tower_lightning.png", "Wieza Piorunow", "Razi piorunami do 3 wrogow naraz.\nBlyskawice niszcza pancerze!"}
            };

            for (size_t i = 0; i < towers.size(); ++i) {
                float xPos = (i % 2 == 0) ? 500.f : 1420.f;
                // Bezpieczne u¿ycie static_cast zapobiega ostrze¿eniom C4244
                float yOffset = static_cast<float>(static_cast<int>(i) / 2) * 175.f;
                float yPos = 200.f + yOffset;

                sf::Texture& t = TextureManager::getInstance().get(towers[i].tex);
                sf::Sprite spr(t);

                sf::Vector2u tSize = t.getSize();
                if (tSize.x > 0 && tSize.y > 0) {
                    spr.setOrigin(sf::Vector2f(tSize.x / 2.f, tSize.y / 2.f));
                    spr.setScale(sf::Vector2f(80.f / tSize.y, 80.f / tSize.y));
                    spr.setPosition(sf::Vector2f(xPos - 200.f, yPos + 30.f));
                    window.draw(spr);
                }

                sf::Text nText(font);
                nText.setString(towers[i].name + "\n" + towers[i].desc);
                nText.setCharacterSize(24);
                nText.setFillColor(sf::Color::White);
                nText.setPosition(sf::Vector2f(xPos - 100.f, yPos));
                window.draw(nText);
            }
        }
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
        // T£O KAFELKOWE B¥D G£ADKI KOLOR:
        window.clear(sf::Color::Black);
        window.draw(availableMaps[currentMapIndex].bgShape);

        // Rysowanie z³¹cz i segmentów grubej œcie¿ki
        for (const auto& joint : availableMaps[currentMapIndex].pathJoints) {
            window.draw(joint);
        }
        for (const auto& shape : availableMaps[currentMapIndex].pathShapes) {
            window.draw(shape);
        }

        for (const auto& obs : availableMaps[currentMapIndex].obstacles) {
            window.draw(obs);
        }

        waveManager.drawEnemies(window);
        for (const auto& tower : activeTowers) tower->draw(window);
        for (const auto& proj : activeProjectiles) proj->draw(window);

        if (isPlacingTower) window.draw(ghostTower);

        drawGameplayHUD();

        if (currentState == GameState::PAUSE) {
            // SZTYWNY ROZMIAR ZACIEMNIENIA EKRANU W OPARCIU O LOGICZNE 1920x1080
            sf::RectangleShape overlay(sf::Vector2f(1920.f, 1080.f));
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
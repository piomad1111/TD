#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <filesystem>
#include <iostream>

// Struktura reprezentuj¹ca pojedynczy wpis w tablicy wyników
struct ScoreEntry {
    std::string name;
    int score;

    // Przeci¹¿enie operatora potrzebne do ³atwego sortowania wyników (malej¹co)
    bool operator<(const ScoreEntry& other) const {
        return score > other.score;
    }
};

// Klasa zgodna z Twoim diagramem UML z pliku PDF.
// Bêdzie przechowywaæ dane gracza pomiêdzy ró¿nymi stanami gry.
class PlayerStats {
public:
    std::string nickname = "";
    int gold = 800;
    int score = 0;
    int baseHealth = 100;
    int maxBaseHealth = 100;

    // Modyfikatory
    float scoreMultiplier = 1.0f;
    float mapDifficultyMultiplier = 0.0f; // NOWE: Dodatkowa premia za trudniejsz¹ mapê
    bool modifierLessGold = false;
    bool modifierFragileBase = false;

    // Opcja (Ulatwienie Interfejsu)
    bool autoWaveStart = false;

    void recalculateMultiplier() {
        // Podstawowy mno¿nik + nagroda za trudnoœæ mapy
        scoreMultiplier = 1.0f + mapDifficultyMultiplier;
        // + Nagrody za w³¹czone utrudnienia w Opcjach
        if (modifierLessGold) scoreMultiplier += 0.5f;
        if (modifierFragileBase) scoreMultiplier += 0.5f;
    }

    void addReward(int points) {
        // Punkty mno¿ymy przez ca³y skumulowany modyfikator
        score += static_cast<int>(points * scoreMultiplier);
    }

    bool spendGold(int amount) {
        if (gold >= amount) {
            gold -= amount;
            return true;
        }
        return false;
    }

    void takeDamage(int damage) { baseHealth -= damage; }

    int getScore() const { return score; }
    int getGold() const { return gold; }
    int getHealth() const { return baseHealth; }

    // --- OBS£UGA TABLICY WYNIKÓW (Zapis i odczyt z pliku txt) ---
    static std::vector<ScoreEntry> loadLeaderboard() {
        std::vector<ScoreEntry> scores;
        std::filesystem::path filePath = "scoreboard.txt";

        if (!std::filesystem::exists(filePath)) {
            return scores;
        }

        std::ifstream inFile(filePath);
        if (inFile.is_open()) {
            std::string name;
            int score;
            while (inFile >> name >> score) {
                scores.push_back({ name, score });
            }
        }

        std::sort(scores.begin(), scores.end());
        return scores;
    }

    static void saveScoreToLeaderboard(const std::string& name, int finalScore) {
        std::filesystem::path filePath = "scoreboard.txt";

        std::vector<ScoreEntry> scores = loadLeaderboard();

        std::string safeName = name.empty() ? "Nieznany" : name;
        scores.push_back({ safeName, finalScore });

        std::sort(scores.begin(), scores.end());

        std::ofstream outFile(filePath);
        if (outFile.is_open()) {
            for (size_t i = 0; i < std::min(scores.size(), static_cast<size_t>(10)); ++i) {
                outFile << scores[i].name << " " << scores[i].score << "\n";
            }
        }
        else {
            std::cerr << "Nie udalo sie otworzyc pliku scoreboard.txt do zapisu!\n";
        }
    }
};
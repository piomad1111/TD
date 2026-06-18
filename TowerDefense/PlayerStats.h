#pragma once
#include <string>

// Klasa zgodna z Twoim diagramem UML z pliku PDF.
// Bêdzie przechowywaæ dane gracza pomiêdzy ró¿nymi stanami gry.
class PlayerStats {
public:
    std::string nickname = "";
    int gold = 900;
    int score = 0;
    int baseHealth = 100;
    int maxBaseHealth = 100;

    // Te metody zaimplementujemy w pe³ni póŸniej, gdy zajmiemy siê mechanik¹
    void addReward(int points) { score += points; }
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
};
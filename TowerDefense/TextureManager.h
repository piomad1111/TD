#pragma once
#include <SFML/Graphics.hpp>
#include <map>
#include <string>
#include <iostream>

class TextureManager {
public:
    static TextureManager& getInstance() {
        static TextureManager instance;
        return instance;
    }

    // £aduje teksturê jeœli nie by³a wczeœniej za³adowana, zwraca referencjê
    sf::Texture& get(const std::string& filename) {
        auto it = textures.find(filename);
        if (it != textures.end()) return it->second;

        sf::Texture tex;
        if (!tex.loadFromFile(filename)) {
            std::cerr << "Blad wczytywania tekstury: " << filename << "\n";
        }
        textures[filename] = std::move(tex);
        return textures[filename];
    }

private:
    TextureManager() = default;
    std::map<std::string, sf::Texture> textures;
};
#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

using namespace std;
using namespace sf;

#define SQUARE 'O'

struct Slagalica {
    int brojPogodjanja;
    int maksimalniBrojPogadjanja;
    Texture znakoviTexture[6];
    Sprite znakoviSprite[6];
    string znakovi[6];
    string generisaniZnakovi[4];
    Slagalica() {
        znakovi[0] = "skocko";
        znakovi[1] = "tref";
        znakovi[2] = "pik";
        znakovi[3] = "srce";
        znakovi[4] = "karo";
        znakovi[5] = "zvezda";
        maksimalniBrojPogadjanja = 10;
        brojPogodjanja = 0;
    }
};

int contains(int i, int indexRed[], int redCount) {
    for (int j = 0; j < redCount; j++) {
        if (indexRed[j] == i) {
            return 1;
        }
    }
    return 0;
}

void printColors(int guess[], int comb[], int &yellowCount, int &redCount) {
    redCount = 0;
    yellowCount = 0;
    int i, j;
    int tempComb[4], tempGuess[4];

    // Create temporary copies of the combination and guess
    for (i = 0; i < 4; i++) {
        tempComb[i] = comb[i];
        tempGuess[i] = guess[i];
    }

    // First pass to count reds
    for (i = 0; i < 4; i++) {
        if (tempGuess[i] == tempComb[i]) {
            redCount++;
            tempGuess[i] = tempComb[i] = -1; // Mark as used
        }
    }

    // Second pass to count yellows
    for (i = 0; i < 4; i++) {
        if (tempGuess[i] != -1) {
            for (j = 0; j < 4; j++) {
                if (tempGuess[i] == tempComb[j]) {
                    yellowCount++;
                    tempComb[j] = -1; // Mark as used
                    break;
                }
            }
        }
    }

    // Printing for debug purposes
    cout << "Red: " << redCount << " Yellow: " << yellowCount << endl;
}

int isEqual(int guess[], int comb[]) {
    for (int i = 0; i < 4; i++) {
        if (guess[i] != comb[i]) {
            return 0;
        }
    }
    return 1;
}

void load(Slagalica& slagalicaInstance) {
    for (int i = 0; i < 6; i++) {
        if (!slagalicaInstance.znakoviTexture[i].loadFromFile("znakovi/" + slagalicaInstance.znakovi[i] + ".jpg")) {
            cerr << "Error loading texture: " << slagalicaInstance.znakovi[i] << ".jpg" << endl;
            exit(-1);
        }
        slagalicaInstance.znakoviSprite[i].setTexture(slagalicaInstance.znakoviTexture[i]);
        Vector2u textureSize = slagalicaInstance.znakoviTexture[i].getSize();
        slagalicaInstance.znakoviSprite[i].setScale(100.f / textureSize.x, 100.f / textureSize.y);
    }
}

void postaviZnakove(Slagalica& slagalicaInstance) {
    for (int i = 0; i < 6; i++) {
        slagalicaInstance.znakoviSprite[i].setPosition(Vector2f(450, 100 * i));
    }
}

void generisiZnakove(Slagalica& slagalicaInstance, int comb[]) {
    srand(static_cast<unsigned>(time(0)));
    for (int i = 0; i < 4; i++) {
        int znakIndex = rand() % 6;
        comb[i] = znakIndex;
        slagalicaInstance.generisaniZnakovi[i] = slagalicaInstance.znakovi[znakIndex];
    }
}

void generisiKrugove(vector<CircleShape>& krugovi, int yellowCount, int redCount, float startX, float startY, float circleRadius) {
    for (int i = 0; i < yellowCount + redCount; i++) {
        CircleShape circle(circleRadius);
        circle.setPosition(550 + i * (circleRadius * 2 + 5), startY); // Adding a small gap between circles
        if (i < redCount) {
            circle.setFillColor(Color::Red);
        } else {
            circle.setFillColor(Color::Yellow);
        }
        krugovi.push_back(circle);
    }
}

int main() {
    RenderWindow window(VideoMode(1000, 600), "Skocko", Style::Default);

    int guess[4], comb[4];
    int currGuessCountRow = 0;
    int currGuessCount = 0;
    Slagalica slagalicaInstance;

    load(slagalicaInstance);
    postaviZnakove(slagalicaInstance);
    generisiZnakove(slagalicaInstance, comb);

    vector<vector<CircleShape>> redYellowCircles;
    vector<Sprite> drawnSprites;
    float spriteSize = 100.f;
    float startX = 10.f;
    float startY = 10.f;
    float circleRadius = 40.f; // Circle radius

    int spritesInRow = 0;
    float nextX = startX;
    float nextY = startY;

    int yellowCount = 0;
    int redCount = 0;

    while (window.isOpen()) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) {
                window.close();
            }
            else if (event.type == Event::MouseButtonPressed) {
                if (event.mouseButton.button == Mouse::Left) {
                    Vector2i mousePos = Mouse::getPosition(window);
                    Vector2f mousePosF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));

                    for (int i = 0; i < 6; i++) {
                        if (slagalicaInstance.znakoviSprite[i].getGlobalBounds().contains(mousePosF)) {
                            Sprite newSprite;
                            newSprite.setTexture(slagalicaInstance.znakoviTexture[i]);
                            newSprite.setScale(spriteSize / slagalicaInstance.znakoviTexture[i].getSize().x,
                                                spriteSize / slagalicaInstance.znakoviTexture[i].getSize().y);
                            newSprite.setPosition(nextX, nextY);

                            drawnSprites.push_back(newSprite);

                            guess[currGuessCountRow] = i;
                            currGuessCountRow = (currGuessCountRow + 1) % 4;
                            currGuessCount++;
                            spritesInRow++;

                            if (spritesInRow >= 4) {
                                printColors(guess, comb, yellowCount, redCount);

                                vector<CircleShape> newCircles;
                                generisiKrugove(newCircles, yellowCount, redCount, nextX + spriteSize + 10, nextY, circleRadius);
                                redYellowCircles.push_back(newCircles);

                                if (isEqual(guess, comb)) {
                                    cout << "Congratulations! You've guessed the combination." << endl;
                                }

                                currGuessCountRow = 0;
                                spritesInRow = 0;
                                nextX = startX;
                                nextY += spriteSize;
                            } else {
                                nextX += spriteSize;
                            }
                        }
                    }
                }
            }
        }

        window.clear(Color::White);
        for (int i = 0; i < 6; i++) {
            window.draw(slagalicaInstance.znakoviSprite[i]);
        }
        for (const auto& sprite : drawnSprites) {
            window.draw(sprite);
        }
        for (const auto& circles : redYellowCircles) {
            for (const auto& circle : circles) {
                window.draw(circle);
            }
        }
        window.display();
    }

    return 0;
}

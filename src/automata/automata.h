#ifndef CELLULAR_AUTOMATA
#define CELLULAR_AUTOMATA

#include <iostream>
#include <random>
#include <vector>

class CellularAutomata
{
public:
    CellularAutomata(int width, int height);
    bool checkCell(int cellX, int cellY, int cellType);
    unsigned char* getStateAsImage();
    void update();
    void updateInputs(float mouseXin, float mouseYin, bool lmbDown, bool rmbDown);
private:
    std::vector<std::vector<int>> grid;
    std::vector<std::vector<int>> newGrid;
    int width;
    int height;
    bool leftMouseDown;
    bool rightMouseDown;
    float mouseX;
    float mouseY;
};

#endif

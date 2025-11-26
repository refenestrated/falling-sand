#ifndef CELLULAR_AUTOMATA
#define CELLULAR_AUTOMATA

#include <iostream>

class CellularAutomata
{
public:
    CellularAutomata(int width, int height);
    ~CellularAutomata();
    int getCell(int cellX, int cellY);
    unsigned char* getStateAsImage();
    void update();
    void updateInputs(float mouseXin, float mouseYin, bool lmbDown, bool rmbDown);
private:
    int** grid;
    int width;
    int height;
    bool leftMouseDown;
    bool rightMouseDown;
    float mouseX;
    float mouseY;
};

#endif

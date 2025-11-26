#include "automata.h"

CellularAutomata::CellularAutomata(int width, int height)
{
    this->width = width;
    this->height = height;

    grid = new int*[width];
    for (int i = 0; i < width; i++)
    {
        grid[i] = new int[height];
        for (int j = 0; j < height; j++)
        {
            grid[i][j] = 0;
        }
    }
}

CellularAutomata::~CellularAutomata()
{
    for (int i = 0; i < width; i++)
    {
        delete[] grid[i];
    }
    delete[] grid;
}

//utility function
int CellularAutomata::getCell(int cellX, int cellY)
{
    return grid[cellX][cellY];
}

unsigned char* CellularAutomata::getStateAsImage()
{
    unsigned char* pixels = new unsigned char[width * height * 4];

    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            int cellType = grid[i][j];

            int pixelIndex = (i + j * width) * 4;

            switch (cellType)
            {
                case 0:
                    pixels[pixelIndex] = 0;
                    pixels[pixelIndex + 1] = 0;
                    pixels[pixelIndex + 2] = 0;
                    pixels[pixelIndex + 3] = 255;
                    break;
                case 1:
                    pixels[pixelIndex] = 255;
                    pixels[pixelIndex + 1] = 255;
                    pixels[pixelIndex + 2] = 0;
                    pixels[pixelIndex + 3] = 255;
                    break;
                case 2:
                    pixels[pixelIndex] = 0;
                    pixels[pixelIndex + 1] = 0;
                    pixels[pixelIndex + 2] = 255;
                    pixels[pixelIndex + 3] = 255;
                    break;
            }
        }
    }

    return pixels;
}

//automata rules are applied
void CellularAutomata::update()
{
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            int cellType = grid[i][j];
            switch (cellType)
            {
                //cell is air
                case 0:
                    break;

                //cell is sand
                case 1:
                    if (j + 1 < height && grid[i][j - 1] == 0) {  // move down
                        grid[i][j - 1] = 1;
                        grid[i][j] = 0;
                    }
                    else if (i + 1 < width && j - 1 < height && grid[i + 1][j - 1] == 0) {  // move diagonal right
                        grid[i + 1][j - 1] = 1;
                        grid[i][j] = 0;
                    }
                    else if (i - 1 >= 0 && j - 1 < height && grid[i - 1][j - 1] == 0) {  // move diagonal left
                        grid[i - 1][j - 1] = 1;
                        grid[i][j] = 0;
                    }
                    break;

                case 2:
                    if (j + 1 < height && grid[i][j - 1] == 0) {  // move down
                        grid[i][j - 1] = 2;
                        grid[i][j] = 0;
                    }
                    else if (i + 1 < width && grid[i + 1][j] == 0) {  // move diagonal right
                        grid[i + 1][j] = 2;
                        grid[i][j] = 0;
                    }
                    else if (i - 1 >= 0 && grid[i - 1][j] == 0) {  // move diagonal left
                        grid[i - 1][j] = 2;
                        grid[i][j] = 0;
                    }
                    break;
            }
        }
    }
}

//inputs take effect
void CellularAutomata::updateInputs(float mouseXin, float mouseYin, bool lmbDown, bool rmbDown)
{
    mouseX = mouseXin;
    mouseY = height - mouseYin;

    leftMouseDown = lmbDown;
    rightMouseDown = rmbDown;

    if (mouseX < 0 || mouseX >= width || mouseY < 0 || mouseY >= height)
    {
        leftMouseDown = false;
        rightMouseDown = false;
    }

    if (leftMouseDown)
    {
        for (int i = 0; i < 10; i++)
        {
            for (int j = 0; j < 10; j++)
            {
                int x {(int)mouseX + 1};
                int y {(int)mouseY + j};
                if (x >= 0 && x < width && y >= 0 && y < height)
                {
                    grid[x][y] = 1;
                }
            }
        }
    }
    else if (rightMouseDown)
    {
        for (int i = 0; i < 10; i++)
        {
            for (int j = 0; j < 10; j++)
            {
                int x {(int)mouseX + 1};
                int y {(int)mouseY + j};
                if (x >= 0 && x < width && y >= 0 && y < height)
                {
                    grid[x][y] = 2;
                }
            }
        }
    }
}

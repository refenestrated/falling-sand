#include "automata.h"

CellularAutomata::CellularAutomata(int width, int height)
{
    this->width = width;
    this->height = height;

    grid.resize(width);
    newGrid.resize(width);

    for (int i = 0; i < width; i++)
    {
        grid[i].resize(height, 0);
        newGrid[i].resize(height, 0);
    }
}

bool CellularAutomata::checkCell(int cellX, int cellY, int cellType)
{
    //ensure cell is valid
    if (cellX < 0 || cellX >= width || cellY < 0 || cellY >= height)
    {
        return false;
    }

    //compare cell types
    if (grid[cellX][cellY] == cellType)
    {
        return true;
    }
    return false;
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
    std::random_device rd;
    std::mt19937 g(rd());
    std::uniform_int_distribution<std::mt19937::result_type> dist1(0, 1);

    // i is cell x position and j is cell y position
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            int cellType {grid[i][j]};
            int up {j + 1};
            int down {j - 1};
            int left {i - 1};
            int right {i + 1};

            switch (cellType)
            {
                //cell is air
                case 0:
                    break;

                //cell is sand
                case 1:
                    if (checkCell(i, down, 0))
                    {
                        newGrid[i][down] = 1;
                        newGrid[i][j] = 0;
                    }
                    else if (checkCell(right, down, 0))
                    {
                        newGrid[right][down] = 1;
                        newGrid[i][j] = 0;
                    }
                    else if (checkCell(left, down, 0))
                    {
                        newGrid[left][down] = 1;
                        newGrid[i][j] = 0;
                    }
                    break;

                //cell is water
                case 2:
                    if (checkCell(i, down, 0))
                    {
                        newGrid[i][down] = 2;
                        newGrid[i][j] = 0;
                    }
                    else if (dist1(g) == 1)
                    {
                        if (checkCell(right, j, 0))
                        {
                            newGrid[right][j] = 2;
                            newGrid[i][j] = 0;
                        }
                        else if (checkCell(left, j, 0))
                        {
                            newGrid[left][j] = 2;
                            newGrid[i][j] = 0;
                        }
                    }
                    else
                    {
                        if (checkCell(left, j, 0))
                        {
                            newGrid[left][j] = 2;
                            newGrid[i][j] = 0;
                        }
                        else if (checkCell(right, j, 0))
                        {
                            newGrid[right][j] = 2;
                            newGrid[i][j] = 0;
                        }
                    }

                    break;
            }
        }
    }

    grid = newGrid;
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
                int x {(int)mouseX + i};
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
                int x {(int)mouseX + i};
                int y {(int)mouseY + j};
                if (x >= 0 && x < width && y >= 0 && y < height)
                {
                    grid[x][y] = 2;
                }
            }
        }
    }
}

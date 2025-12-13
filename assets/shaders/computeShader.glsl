#version 430 core

struct Cell
{
    vec4 colour;
    int type;
    int justMoved;
    int density;
    int inertia;
};

layout(std430, binding = 0) buffer GridBuffer
{
    Cell grid[];
};

layout(std430, binding = 1) buffer nextGridBuffer
{
    Cell nextGrid[];
};

layout(std430, binding = 2) buffer claimBuffer
{
    int claim[];
};

layout(std430, binding = 3) buffer movedBuffer
{
    int moved[];
};

layout (local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

uniform int pass;

uniform float time;

uniform int gridWidth;
uniform int gridHeight;

uniform int mouseX;
uniform int mouseY;

uniform bool leftMouseDown;
uniform bool rightMouseDown;

Cell Air = Cell(vec4(0.0, 0.0, 0.0, 0.0), 0, 0, 0, 0);
Cell Sand = Cell(vec4(1.0, 1.0, 0.0, 1.0), 1, 0, 10, 0);
Cell Water = Cell(vec4(0.0, 0.0, 1.0, 1.0), 2, 0, 5, 0);

uint hash(uint x)
{
    x ^= x >> 16;
    x *= 0x7feb352d;
    x ^= x >> 15;
    x *= 0x846ca68b;
    x ^= x >> 16;
    return x;
}

float random(uint seed)
{
    return float(hash(seed)) / float(0xffffffff);
}

bool inBounds(uint IDx)
{
    return IDx < uint(gridWidth * gridHeight);
}

bool tryClaimAndMove(uint source, uint destination, Cell cell)
{
    if (!inBounds(destination) || destination == source)
    {
        return false;
    }

    Cell destCell = grid[destination];

    if (destCell.density >= cell.density)
    {
        return false;
    }

    int expected = -1;
    int result = atomicCompSwap(claim[destination], expected, int(source));
    if (result == expected)
    {
        nextGrid[destination] = cell;
        nextGrid[destination].justMoved = 1;

        nextGrid[source] = destCell;
        nextGrid[source].justMoved = 0;

        moved[source] = 1;
        moved[destination] = 1;
        return true;
    }
    return false;
}

void main()
{
    ivec2 gID = ivec2(gl_GlobalInvocationID.xy);
    uint IDx = gID.y * gridWidth + gID.x;

    if (!inBounds(IDx))
    {
        return;
    }

    Cell currentCell = grid[IDx];

    //user painting
    ivec2 clampedMouse = ivec2(clamp(mouseX, 0, gridWidth - 1), clamp(mouseY, 0, gridHeight - 1));

    if (pass == 0)
    {
        if (IDx < uint((gridWidth * gridHeight)))
        {
            claim[IDx] = -1;
            moved[IDx] = 0;
        }
    }

    //paint pass
    else if (pass == 1)
    {
        int radius = 4;

        nextGrid[IDx] = currentCell;
        nextGrid[IDx].justMoved = 0;

        if (distance(vec2(gID), vec2(clampedMouse)) < float(radius))
        {
            if (leftMouseDown)
            {
                nextGrid[IDx] = Sand;
            }
            else if (rightMouseDown)
            {
                nextGrid[IDx] = Water;
            }
        }
    }

    else if (currentCell.justMoved == 1)
    {
        return;
    }

    else if (pass >= 2 && moved[IDx] == 1)
    {
        return;
    }

    //gravity pass
    else if (pass == 2 && currentCell.type != 0)
    {
        uint down = IDx;
        if (gID.y > 0)
        {
            down = IDx - gridWidth;
            tryClaimAndMove(IDx, down, currentCell);
        }
    }

    //diagonal movement pass
    else if (pass == 3 && currentCell.type == 1)
    {
        uint downLeft = IDx;
        uint downRight = IDx;

        if (gID.x > 0 && gID.y > 0)
        {
            downLeft = IDx - gridWidth - 1;
        }
        if (gID.x < gridWidth - 1 && gID.y > 0)
        {
            downRight = IDx - gridWidth + 1;
        }

        bool preferRight = ((hash(IDx) & 1u) == 0u);
        if (preferRight)
        {
            if (!tryClaimAndMove(IDx, downRight, currentCell))
            {
                tryClaimAndMove(IDx, downLeft, currentCell);
            }
        }
        else
        {
            if (!tryClaimAndMove(IDx, downLeft, currentCell))
            {
                tryClaimAndMove(IDx, downRight, currentCell);
            }
        }
    }

    //horizontal movement pass
    else if (pass == 4 && currentCell.type == 2)
    {
        uint left = IDx;
        uint right = IDx;

        if (gID.x > 0 && grid[IDx - 1].type == 0)
        {
            left = IDx - 1;
        }
        if (gID.x < gridWidth - 1 && grid[IDx + 1].type == 0)
        {
            right = IDx + 1;
        }

        if (currentCell.inertia == 1)
        {
            if (!tryClaimAndMove(IDx, right, currentCell))
            {
                tryClaimAndMove(IDx, left, currentCell);
            }
        }
        else if (currentCell.inertia == -1)
        {
            if (!tryClaimAndMove(IDx, left, currentCell))
            {
                tryClaimAndMove(IDx, right, currentCell);
            }
        }
        else
        {
            bool preferRight = ((hash(IDx + uint(pass) + uint(time * 997.0)) & 1u) == 0u);
            if (preferRight)
            {
                if (!tryClaimAndMove(IDx, right, currentCell))
                {
                    tryClaimAndMove(IDx, left, currentCell);
                }
            }
            else
            {
                if (!tryClaimAndMove(IDx, left, currentCell))
                {
                    tryClaimAndMove(IDx, right, currentCell);
                }
            }
        }
    }
}


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

layout(std430, binding = 3) buffer movedBuffer
{
    int moved[];
};

uniform bool debug;

uniform int gridWidth;
uniform int gridHeight;
uniform int screenWidth;
uniform int screenHeight;

out vec4 FragColour;

void main()
{
    vec2 uv = gl_FragCoord.xy / vec2(screenWidth, screenHeight);
    ivec2 gID = ivec2(uv * vec2(gridWidth, gridHeight));

    if (gID.x >= gridWidth || gID.y >= gridHeight)
    {
        discard;
    }

    uint IDx = gID.y * gridWidth + gID.x;

    Cell currentCell = grid[IDx];

    //debug view which shows which cells have moved in the last frame
    if (debug)
    {
        if (moved[IDx] == 1)
        {
            FragColour = vec4(1.0, 0.0, 0.0, 1.0);
        }
        else
        {
            FragColour = vec4(0.0);
        }
    }
    //default view
    else
    {
        FragColour = vec4(currentCell.colour);
    }
}

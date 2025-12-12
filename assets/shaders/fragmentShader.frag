#version 430 core

struct Cell
{
    vec4 colour;
    int type;
    int justMoved;
    int density;
};

layout(std430, binding = 0) buffer GridBuffer
{
    Cell grid[];
};

uniform int gridWidth;
uniform int gridHeight;
uniform int screenWidth;
uniform int screenHeight;

out vec4 FragColour;

void main()
{
    vec2 uv = gl_FragCoord.xy / vec2(screenWidth, screenHeight);
    ivec2 gID = ivec2(uv * vec2(gridWidth, gridHeight));

    //gID.x = int(float(gID.x) / float(gridWidth) * gridWidth);
    //gID.y = int(float(gID.y) / float(gridHeight) * gridHeight);

    if (gID.x >= gridWidth || gID.y >= gridHeight)
    {
        discard;
    }

    uint IDx = gID.y * gridWidth + gID.x;

    Cell currentCell = grid[IDx];

    /*
    if (currentCell.justMoved == 0)
    {
        FragColour = vec4(1.0, 0.0, 0.0, 1.0);
    }
    else
    {
        FragColour = vec4(0.0);
    }
    */
    FragColour = vec4(currentCell.colour);
}

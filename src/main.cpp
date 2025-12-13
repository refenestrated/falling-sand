#include <iostream>
#include <SDL3/SDL.h>
#include <glad.h>
#include "shader/Shader.h"

int constexpr SCR_WIDTH {1920};
int constexpr SCR_HEIGHT {1080};
int constexpr GRID_WIDTH {SCR_WIDTH / 8};
int constexpr GRID_HEIGHT {SCR_HEIGHT / 8};

struct Cell
{
    float colour[4];
    int type;
    int justMoved;
    int density;
    int inertia;
};

void updateGridBuffers(GLuint& gridBuffer, GLuint& nextGridBuffer);
void checkOpenGLError();

int main(int argc, char **argv)
{
    //initialise SDL3
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "SDL video failed to initialise: " << SDL_GetError() << std::endl;
        return -1;
    }
    else
    {
        std::cout << "SDL video successfully initialised" << std::endl;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    //create window
    SDL_Window* window = SDL_CreateWindow("falling sand experiment", SCR_WIDTH, SCR_HEIGHT, SDL_WINDOW_OPENGL);

    if (!window)
    {
        std::cerr << "window failed to initialise: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }
    else
    {
        std::cout << "window successfully created"<< std::endl;
    }

    //create OpenGL context
    SDL_GLContext glContext = SDL_GL_CreateContext(window);

    if (!glContext)
    {
        std::cerr << "OpenGL context failed to initialise: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    //initialise GLAD
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    {
        std::cerr << "GLAD failed to initialise: " << SDL_GetError() << std::endl;
        SDL_GL_DestroyContext(glContext);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }
    else
    {
        std::cout << "GLAD successfully initialised" << std::endl;
    }

    //face data
    float vertices[] = {
        // vertex positions
        1.0f,  1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f
    };
    unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    //std::cout << "Max work groups: " << GL_MAX_COMPUTE_WORK_GROUP_COUNT << std::endl;

    //OpenGL buffers + arrays
    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //create shader program
    Shader automataShader("../assets/shaders/vertexShader.vert", "../assets/shaders/fragmentShader.frag");
    Shader automataCompute("../assets/shaders/computeShader.glsl");

    GLuint gridBuffer, nextGridBuffer, claimBuffer, movedBuffer;
    glGenBuffers(1, &gridBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, gridBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, GRID_WIDTH * GRID_HEIGHT * sizeof(Cell), nullptr, GL_DYNAMIC_DRAW);

    Cell* data = (Cell*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, GRID_WIDTH * GRID_HEIGHT * sizeof(Cell), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

    glGenBuffers(1, &nextGridBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, nextGridBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, GRID_WIDTH * GRID_HEIGHT * sizeof(Cell), nullptr, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &claimBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, claimBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, GRID_WIDTH * GRID_HEIGHT * sizeof(int), nullptr, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &movedBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, movedBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, GRID_WIDTH * GRID_HEIGHT * sizeof(int), nullptr, GL_DYNAMIC_DRAW);

    for (int i = 0; i < GRID_WIDTH * GRID_HEIGHT; i++)
    {
        data[i].type = 0;
        data[i].colour[0] = 0.0f;
        data[i].colour[1] = 0.0f;
        data[i].colour[2] = 0.0f;
        data[i].colour[3] = 0.0f;
        data[i].justMoved = 0;
        data[i].density = 0;
        data[i].inertia = 0;
    }

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, gridBuffer);

    int numWorkGroupsX = (GRID_WIDTH + 15) / 16;
    int numWorkGroupsY = (GRID_HEIGHT + 15) / 16;

    //main loop
    std::cout << "Starting main loop" << std::endl;

    SDL_Event e;

    const int targetFPS {1000};
    const float frameDelay {1000 / targetFPS};

    int numPasses {5};
    bool debugView {false};

    bool running {true};
    while (running)
    {
        Uint32 frameStart = SDL_GetTicks();

        //mouse position and held state
        float mouseX, mouseY;
        uint32_t mouseState = SDL_GetMouseState(&mouseX, &mouseY);

        float mouseXNormal {mouseX / (float)SCR_WIDTH * GRID_WIDTH};
        float mouseYNormal {GRID_HEIGHT - (mouseY / (float)SCR_HEIGHT * GRID_HEIGHT)};

        bool leftMouseDown = (mouseState & SDL_BUTTON_MASK(SDL_BUTTON_LEFT)) != 0;
        bool rightMouseDown = (mouseState & SDL_BUTTON_MASK(SDL_BUTTON_RIGHT)) != 0;

        //SDL events - keyboard inputs etc...
        while(SDL_PollEvent(&e))
        {
            if (e.type == SDL_EVENT_QUIT)
            {
                running = false;
            }
            else if (e.type == SDL_EVENT_KEY_DOWN)
            {
                if (e.key.key == SDLK_SPACE)
                {
                    debugView = true;
                }
            }
            else if (e.type == SDL_EVENT_KEY_UP)
            {
                if (e.key.key == SDLK_SPACE)
                {
                    debugView = false;
                }
            }
        }

        automataCompute.use();
        automataCompute.setFloat("time", SDL_GetTicks());
        automataCompute.setInt("gridWidth", GRID_WIDTH);
        automataCompute.setInt("gridHeight", GRID_HEIGHT);
        automataCompute.setInt("mouseX", (int)mouseXNormal);
        automataCompute.setInt("mouseY", (int)mouseYNormal);
        automataCompute.setBool("leftMouseDown", leftMouseDown);
        automataCompute.setBool("rightMouseDown", rightMouseDown);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, gridBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, nextGridBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, claimBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, movedBuffer);

        /*
        passes:
            0 - reset claimBuffer
            1 - user painting
            2 - gravity
            3 - diagonal movement (sand)
            4 - horizontal movement (water)
        */
        for (int i = 0; i < numPasses; i++)
        {
            automataCompute.setInt("pass", i);
            automataCompute.dispatch(numWorkGroupsX, numWorkGroupsY, 1);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        }

        updateGridBuffers(gridBuffer, nextGridBuffer);

        //graphics
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.3f, 0.4f, 0.5f, 1.0f); //debug colour in case quad doesn't render

        automataShader.use();

        automataShader.setInt("gridWidth", GRID_WIDTH);
        automataShader.setInt("gridHeight", GRID_HEIGHT);
        automataShader.setInt("screenWidth", SCR_WIDTH);
        automataShader.setInt("screenHeight", SCR_HEIGHT);
        automataShader.setBool("debug", debugView);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        SDL_GL_SwapWindow(window);

        //framerate delay
        Uint32 frameTime = SDL_GetTicks() - frameStart;
        //std::cout << frameTime << std::endl;
        if (frameDelay > frameTime)
        {
            SDL_Delay(frameDelay - frameTime);
        }
    }

    std::cout << "Ended main loop" << std::endl;

    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);

    SDL_GL_DestroyContext(glContext);

    SDL_DestroyWindow(window);

    SDL_Quit();

    std::cout << "SDL successfully quit" << std::endl;

    return 0;
}

void updateGridBuffers(GLuint& gridBuffer, GLuint& nextGridBuffer)
{
    GLuint temp = gridBuffer;
    gridBuffer = nextGridBuffer;
    nextGridBuffer = temp;

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, gridBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, nextGridBuffer);
}

void checkOpenGLError()
{
    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR)
    {
        std::cerr << "OpenGL error: " << error << std::endl;
    }
}

#include <iostream>
#include <SDL3/SDL.h>
#include "glad/include/glad/glad.h"
#include "automata/automata.h"
#include "shader/Shader.h"

int constexpr SCR_WIDTH {800};
int constexpr SCR_HEIGHT {600};

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
        // vertex positions      // texture coordinates
        1.0f,  1.0f, 0.0f,       1.0f, 1.0f, // top right
        1.0f, -1.0f, 0.0f,       1.0f, 0.0f, // bottom right
        -1.0f, -1.0f, 0.0f,      0.0f, 0.0f, // bottom left
        -1.0f,  1.0f, 0.0f,      0.0f, 1.0f  // top left
    };
    unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    //create texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

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

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    //create shader program
    Shader automataShader("../assets/shaders/vertexShader.vert", "../assets/shaders/fragmentShader.frag");

    //create cellular automata
    CellularAutomata fallingSand(SCR_WIDTH, SCR_HEIGHT);


    automataShader.use();

    //main loop
    std::cout << "Starting main loop" << std::endl;

    SDL_Event e;

    const int targetFPS {60};
    const int frameDelay {1000 / targetFPS};

    bool running {true};
    while (running)
    {
        Uint32 frameStart = SDL_GetTicks();

        //mouse position and held state
        float mouseX, mouseY;
        uint32_t mouseState = SDL_GetMouseState(&mouseX, &mouseY);

        bool leftMouseDown = (mouseState & SDL_BUTTON_MASK(SDL_BUTTON_LEFT)) != 0;
        bool rightMouseDown = (mouseState & SDL_BUTTON_MASK(SDL_BUTTON_RIGHT)) != 0;


        //SDL events - keyboard inputs etc...
        while(SDL_PollEvent(&e))
        {
            if (e.type == SDL_EVENT_QUIT)
            {
                running = false;
            }
        }

        //cellular automata process
        fallingSand.updateInputs(mouseX, mouseY, leftMouseDown, rightMouseDown);
        fallingSand.update();

        //graphics
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.3f, 0.4f, 0.5f, 1.0f);

        //create texture to show automata state
        unsigned char* pixelData = fallingSand.getStateAsImage();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, pixelData);

        delete[] pixelData;

        automataShader.use();

        automataShader.setInt("stateTexture", 0);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


        SDL_GL_SwapWindow(window);

        //framerate delay
        Uint32 frameTime = SDL_GetTicks() - frameStart;
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

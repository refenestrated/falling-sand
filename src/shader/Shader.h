#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glad.h>

class Shader
{
public:
    GLuint ID;

    Shader(const char* vertexPath, const char* fragmentPath)
    {
        std::string vertexCode, fragmentCode;
        std::ifstream vShaderFile, fShaderFile;

        vShaderFile.exceptions(std::ifstream::failbit);
        fShaderFile.exceptions(std::ifstream::failbit);
        try
        {
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);
            std::stringstream vShaderStream, fShaderStream;

            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();

            vShaderFile.close();
            fShaderFile.close();

            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();
        }
        catch(std::ifstream::failure& e)
        {
            std::cerr << "ERROR: Shader file failed to read" << e.what() << std::endl;
        }
        const char* vShaderCode = vertexCode.c_str();
        const char* fShaderCode = fragmentCode.c_str();

        unsigned int vertex, fragment;
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, nullptr);
        glCompileShader(vertex);
        checkCompileError(vertex, "VERTEX");

        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, nullptr);
        glCompileShader(fragment);
        checkCompileError(fragment, "FRAGMENT");

        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);
        checkCompileError(ID, "PROGRAM");

        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }

    Shader(const char* computePath)
    {
        std::string computeCode;
        std::ifstream cShaderFile;

        cShaderFile.exceptions(std::ifstream::failbit);
        try
        {
            cShaderFile.open(computePath);
            std::stringstream cShaderStream;
            cShaderStream << cShaderFile.rdbuf();
            cShaderFile.close();
            computeCode = cShaderStream.str();
        }
        catch(std::ifstream::failure& e)
        {
            std::cerr << "ERROR: Compute shader file failed to read: " << e.what() << std::endl;
        }

        const char* cShaderCode = computeCode.c_str();

        unsigned int compute = glCreateShader(GL_COMPUTE_SHADER);
        glShaderSource(compute, 1, &cShaderCode, nullptr);
        glCompileShader(compute);
        checkCompileError(compute, "COMPUTE");

        ID = glCreateProgram();
        glAttachShader(ID, compute);
        glLinkProgram(ID);
        checkCompileError(ID, "PROGRAM");

        glDeleteShader(compute);
    }

    void use()
    {
        glUseProgram(ID);
    }

    void dispatch(int x, int y, int z)
    {
        glDispatchCompute(x, y, z);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }

    void setBool(const std::string &name, bool value)
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }
    void setInt(const std::string &name, int value)
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }
    void setFloat(const std::string &name, float value)
    {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }
    void setImage2D(const std::string &name, GLuint textureID, GLenum access = GL_READ_WRITE)
    {
        GLint loc = glGetUniformLocation(ID, name.c_str());
        glUniform1i(loc, 0);
        glBindImageTexture(0, textureID, 0, GL_FALSE, 0, access, GL_RGBA8);
    }

private:
    void checkCompileError(unsigned int shader, std::string type)
    {
        int success;
        char infolog[1024];

        if (type == "PROGRAM")
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, 1024, nullptr, infolog);
                std::cerr << "ERROR: Program linking of type: " << type << "\n" << infolog << std::endl;
            }
        }
        else
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, 1024, nullptr, infolog);
                std::cerr << "ERROR: Shader compilation of type: " << type << "\n" << infolog << std::endl;
            }
        }
    }
};

#endif

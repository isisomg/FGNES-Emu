#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

const int GRID_SIZE = 10; // Tamanho do grid (10x10)
float grid[GRID_SIZE][GRID_SIZE][3]; // Array de cores para cada quadrado (RGB)

float vertices[] = {
    // Posições dos vértices para um quadrado
    -0.1f,  0.1f, 0.0f,  // topo esquerdo
    -0.1f, -0.1f, 0.0f,  // baixo esquerdo
     0.1f,  0.1f, 0.0f,  // topo direito
     0.1f, -0.1f, 0.0f   // baixo direito
};

// Índices para desenhar os dois triângulos que formam o quadrado
GLuint indices[] = {
    0, 1, 2, // Triângulo superior
    2, 1, 3  // Triângulo inferior
};

// Shader de vértice (simples, apenas posições)
const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;
out vec3 color;
void main() {
    gl_Position = vec4(aPos, 1.0);
    color = aColor;
}
)";

// Shader de fragmento (simples, apenas cor)
const char* fragmentShaderSource = R"(
#version 330 core
in vec3 color;
out vec4 FragColor;
void main() {
    FragColor = vec4(color, 1.0);
}
)";

// Função para compilar shaders
GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    // Checando por erros
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Erro de compilação de shader: " << infoLog << std::endl;
    }
    return shader;
}

// Função para criar o programa de shader
GLuint createShaderProgram() {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Checando por erros
    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Erro de link de programa: " << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

void initGrid() {
    // Inicializando o grid com cores aleatórias
    for (int i = 0; i < GRID_SIZE; ++i) {
        for (int j = 0; j < GRID_SIZE; ++j) {
            grid[i][j][0] = static_cast<float>(rand()) / RAND_MAX; // Vermelho
            grid[i][j][1] = static_cast<float>(rand()) / RAND_MAX; // Verde
            grid[i][j][2] = static_cast<float>(rand()) / RAND_MAX; // Azul
        }
    }
}

void renderGrid(GLuint shaderProgram, GLuint VAO, GLuint VBO, GLuint EBO) {
    glUseProgram(shaderProgram);

    // Definir a projeção ortográfica
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_TRUE, &projection[0][0]);

    for (int i = 0; i < GRID_SIZE; ++i) {
        for (int j = 0; j < GRID_SIZE; ++j) {
            // Modificando a cor de cada quadrado
            GLfloat color[3] = { grid[i][j][0], grid[i][j][1], grid[i][j][2] };

            // Bind de buffers
            glBindVertexArray(VAO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

            // Setando a cor do quadrado
            GLuint colorLoc = glGetUniformLocation(shaderProgram, "aColor");
            glUniform3fv(colorLoc, 1, color);

            // Desenhando o quadrado
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
    }
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Falha ao inicializar GLFW" << std::endl;
        return -1;
    }

    // Criando a janela
    GLFWwindow* window = glfwCreateWindow(800, 600, "Grid 2D com OpenGL", nullptr, nullptr);
    if (!window) {
        std::cerr << "Falha ao criar a janela GLFW" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Inicializando o GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Falha ao inicializar o GLAD" << std::endl;
        return -1;
    }

    GLuint shaderProgram = createShaderProgram();

    // Configurando os buffers para os quadrados
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    initGrid();

    // Projeção ortográfica para 2D
    glm::mat4 projection = glm::ortho(0.0f, GRID_SIZE, GRID_SIZE, 0.0f, -1.0f, 1.0f);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        renderGrid(shaderProgram, VAO, VBO, EBO);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>
#include <sstream>  // Necessário para std::ostringstream
#include <fstream>
#include <windows.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "CPU.cpp"
#include "Memoria.cpp"

#define JANELA_LARGURA 800
#define JANELA_ALTURA 600

#define PIXELS_LARGURA 32
#define PIXELS_ALTURA 32

// Codigos de erros
#define ERRO_CRIAR_JANELA -1 // Falha ao criar janela com glfwCreateWindow.
#define ERRO_INIT_GLAD -2 // Erro ao inicializar GLAD
#define ERRO_COMPILAR_VERTEX_SHADER -3 // Erro ao compilar Vertex Shader
#define ERRO_COMPILAR_FRAGMENT_SHADER -4 // Erro ao compilar Fragment Shader
#define ERRO_COMPILAR_SHADER_PROGRAM -5 // Erro compilar Shader Program

struct Cor
{
	int r, g, b;
};

Cor cores[] = {
	{0x7C,0x7C,0x7C},
	{0x00,0x00,0xFC},
	{0x00,0x00,0xBC},
	{0x44,0x28,0xBC},
	{0x94,0x00,0x84},
	{0xA8,0x00,0x20},
	{0xA8,0x10,0x00},
	{0x88,0x14,0x00},
	{0x50,0x30,0x00},
	{0x00,0x78,0x00},
	{0x00,0x68,0x00},
	{0x00,0x58,0x00},
	{0x00,0x40,0x58},
	{0x00,0x00,0x00},
	{0x00,0x00,0x00},
	{0x00,0x00,0x00},
	{0xBC,0xBC,0xBC},
	{0x00,0x78,0xF8},
	{0x00,0x58,0xF8},
	{0x68,0x44,0xFC},
	{0xD8,0x00,0xCC},
	{0xE4,0x00,0x58},
	{0xF8,0x38,0x00},
	{0xE4,0x5C,0x10},
	{0xAC,0x7C,0x00},
	{0x00,0xB8,0x00},
	{0x00,0xA8,0x00},
	{0x00,0xA8,0x44},
	{0x00,0x88,0x88},
	{0x00,0x00,0x00},
	{0x00,0x00,0x00},
	{0x00,0x00,0x00},
	{0xF8,0xF8,0xF8},
	{0x3C,0xBC,0xFC},
	{0x68,0x88,0xFC},
	{0x98,0x78,0xF8},
	{0xF8,0x78,0xF8},
	{0xF8,0x58,0x98},
	{0xF8,0x78,0x58},
	{0xFC,0xA0,0x44},
	{0xF8,0xB8,0x00},
	{0xB8,0xF8,0x18},
	{0x58,0xD8,0x54},
	{0x58,0xF8,0x98},
	{0x00,0xE8,0xD8},
	{0x78,0x78,0x78},
	{0x00,0x00,0x00},
	{0x00,0x00,0x00},
	{0xFC,0xFC,0xFC},
	{0xA4,0xE4,0xFC},
	{0xB8,0xB8,0xF8},
	{0xD8,0xB8,0xF8},
	{0xF8,0xB8,0xF8},
	{0xF8,0xA4,0xC0},
	{0xF0,0xD0,0xB0},
	{0xFC,0xE0,0xA8},
	{0xF8,0xD8,0x78},
	{0xD8,0xF8,0x78},
	{0xB8,0xF8,0xB8},
	{0xB8,0xF8,0xD8},
	{0x00,0xFC,0xFC},
	{0xF8,0xD8,0xF8},
	{0x00,0x00,0x00},
	{0x00,0x00,0x00}
};


void reajustarViewport_callback(GLFWwindow* janela, int largura, int altura); // Ajusta o tamanho do viewport quando o tamanho da janela for alterado
void processarEntrada(GLFWwindow* janela, Memoria& mem); // Processamento de eventos de entrada.
unsigned int criarShaderProgram(); // Cria o programa de Shader a ser utilizado.
GLFWwindow* inicializarJanela(); // Inicia GLFW, GLAD e retorna a janela criada configurada com o ajuste viewport callback.
unsigned int criarGrid(); // Gera o grid com o tamanho definido.
void obterCores(Memoria& mem, glm::vec4* vetorCores);

int main() {

	Memoria mem;
	CPU cpu;
	cpu.inicializar();

	// Carrega os dados na memoria

	std::string path = "snake.hex";
	std::ifstream input(path, std::ios::in | std::ios::binary);
	if (input.is_open() == false) {
		return -1;
	}

	input.seekg(0, input.end);
	int tamanho = input.tellg();
	input.seekg(0, input.beg);

	for (int i = 0; i < tamanho; i++) {
		input.read((char*)&mem[cpu.PC + i], sizeof(char));
	}

	//for (int i = 0; i < tamanho; i++) {
	//	printf("%x ", mem[cpu.PC + i]);
	//}
	printf("\n");

	// Dados carregados na memoria com sucesso

	GLFWwindow* janela = inicializarJanela();

	GLint maxUniforms;
	glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &maxUniforms);
	std::cout << "Max uniform size: " << maxUniforms << std::endl;

	const unsigned int SHADER_PROGRAM = criarShaderProgram();
	unsigned int VAO = criarGrid();

	glm::vec4 coresPixels[PIXELS_LARGURA * PIXELS_ALTURA];

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Modo Wireframe

	// Loop de renderização
	while (!glfwWindowShouldClose(janela)) {

		processarEntrada(janela, mem);

		// Renderização
		if (cpu.PC == 0xFFFF) {
			break;
		}

		cpu.executar(mem);
		if (cpu.atualizarGrafico == false) {
			continue;
		}
		cpu.atualizarGrafico = false;
		glClearColor(0.0f, 0.0f, 0.2f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		obterCores(mem, coresPixels);

		int corUniform = glGetUniformLocation(SHADER_PROGRAM, "cor");
		glUniform4fv(corUniform, PIXELS_LARGURA * PIXELS_ALTURA, glm::value_ptr(coresPixels[0]));

		glUseProgram(SHADER_PROGRAM);
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, PIXELS_ALTURA * PIXELS_LARGURA * 6, GL_UNSIGNED_INT, 0);

		glfwSwapBuffers(janela); // Troca os buffers de imagem
		glfwPollEvents(); // Checa eventos como clicks, teclado etc.

		Sleep(100);
		std::cout << std::hex << (int)mem[0x00FF] << "\n";
		//std::cout << "X: " << (int)cpu.X << " ";
		//std::cout << "Z: " << (int)cpu.Z << std::endl;
	}

	for (int i = 0; i < 16; i++) {
		std::cout << "0x020" << i << ": " << std::hex << (short)mem[0x0200 + i] << std::endl;
	}

	// Finaliza
	glfwTerminate();

	return 0;
}
void obterCores(Memoria& mem, glm::vec4* vetorCores) {
	for (int i = 0; i < PIXELS_ALTURA * PIXELS_LARGURA; i++) {
		int indice = mem[0x0200 + i];
		vetorCores[i] = glm::vec4(cores[indice].r / 255.0f, cores[indice].g / 255.0f, cores[indice].b / 255.0f, 1.0f);
	}
}

unsigned int criarGrid() {

	float distanciaPadrao;

	if (PIXELS_LARGURA > PIXELS_ALTURA) {
		distanciaPadrao = 2.0f / PIXELS_ALTURA;
	}
	else {
		distanciaPadrao = 2.0f / PIXELS_LARGURA;
	}

	float distanciaX = distanciaPadrao;
	float distanciaY = distanciaPadrao;

	const int numRetangulos = PIXELS_ALTURA * PIXELS_LARGURA;
	const int numVertices = numRetangulos * 4;
	const int numIndices = numRetangulos * 6;

	float* vertices = new float[numVertices * 3];
	unsigned int* indices = new unsigned int[numIndices];

	float larguraTotal = distanciaPadrao * PIXELS_LARGURA;
	float alturaTotal = distanciaPadrao * PIXELS_ALTURA;

	float inicioX = -larguraTotal / 2.0f;
	float inicioY = alturaTotal / 2.0f;

	int indice = 0;
	for (int linha = 0; linha < PIXELS_ALTURA; linha++) {
		for (int coluna = 0; coluna < PIXELS_LARGURA; coluna++) {
			float x0 = inicioX + coluna * distanciaPadrao;
			float y0 = inicioY - linha * distanciaPadrao;
			float x1 = x0 + distanciaPadrao;
			float y1 = y0 - distanciaPadrao;

			// Vértices em ordem: superior esquerdo, inferior esquerdo, inferior direito, superior direito
			vertices[indice++] = x0; vertices[indice++] = y0; vertices[indice++] = 0.0f; // top-left
			vertices[indice++] = x0; vertices[indice++] = y1; vertices[indice++] = 0.0f; // bottom-left
			vertices[indice++] = x1; vertices[indice++] = y1; vertices[indice++] = 0.0f; // bottom-right
			vertices[indice++] = x1; vertices[indice++] = y0; vertices[indice++] = 0.0f; // top-right
		}
	}

	indice = 0;
	int indiceOffset = 0;
	for (int linha = 0; linha < PIXELS_ALTURA; linha++) {
		for (int coluna = 0; coluna < PIXELS_LARGURA; coluna++) {

			indices[indice++] = indiceOffset + 0;
			indices[indice++] = indiceOffset + 1;
			indices[indice++] = indiceOffset + 3;

			indices[indice++] = indiceOffset + 1;
			indices[indice++] = indiceOffset + 2;
			indices[indice++] = indiceOffset + 3;

			indiceOffset += 4;
		}
	}

	unsigned int VAO;
	glGenVertexArrays(1, &VAO);

	unsigned int VBO;
	glGenBuffers(1, &VBO);

	unsigned int EBO;
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * numVertices * 3, vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * numIndices, indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return VAO;
}

void reajustarViewport_callback(GLFWwindow* janela, int largura, int altura) {
	glViewport(0, 0, largura, altura);
}

GLFWwindow* inicializarJanela() {
	// Inicialialização
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Cria a janela
	GLFWwindow* janela = glfwCreateWindow(JANELA_LARGURA, JANELA_ALTURA, "Emulador", NULL, NULL);
	if (janela == NULL) {
		std::cout << "Falha ao criar janela." << std::endl;
		glfwTerminate();
		exit(ERRO_CRIAR_JANELA);
	}

	// Usa a janela que foi criada
	glfwMakeContextCurrent(janela);

	// Inicializar GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Falha ao iniciar GLAD." << std::endl;
		exit(ERRO_INIT_GLAD);
	}

	// Configura Viewport.
	glViewport(0, 0, JANELA_LARGURA, JANELA_ALTURA);
	glfwSetFramebufferSizeCallback(janela, reajustarViewport_callback);

	return janela;
}

void processarEntrada(GLFWwindow* janela, Memoria& mem) {
	if (glfwGetKey(janela, GLFW_KEY_ESCAPE) == GLFW_PRESS) { // Encerra ao clicar esc.
		glfwSetWindowShouldClose(janela, true);
	}
	if (glfwGetKey(janela, GLFW_KEY_W) == GLFW_PRESS) { 
		mem[0x00FF] = 0x77;
		std::cout << "andou" << std::endl;
	}
}

unsigned int criarShaderProgram() {
	// Criando um programa de shader.
	const char* vertexShaderSource = "#version 330 core\n"
		"layout (location = 0) in vec3 aPos;\n"
		"flat out int triIndex;\n"
		"void main()\n"
		"{\n"
		" gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
		" triIndex = gl_VertexID / 3;\n"
		"}\0";
	constexpr int numCor = PIXELS_ALTURA * PIXELS_LARGURA;

	std::ostringstream oss;
	oss << "#version 330 core\n"
		<< "out vec4 FragColor;\n"
		<< "uniform vec4 cor[" << numCor << "];\n"
		<< "flat in int triIndex;\n"
		<< "void main()\n"
		<< "{\n"
		<< " FragColor = cor[triIndex / 2];\n"
		<< "}\0";

	std::string fragmentShaderSourceStr = oss.str();
	const char* fragmentShaderSource = fragmentShaderSourceStr.c_str();

	// Criando o Vertex Shader
	unsigned int vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	int sucesso;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &sucesso);
	if (!sucesso) {
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "Falha ao compilar Vertex Shader.\n" << infoLog << std::endl;
		glfwTerminate();
		exit(ERRO_COMPILAR_VERTEX_SHADER);
	}

	// Criando o Fragment 

	unsigned int fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &sucesso);
	if (!sucesso) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "Falha ao compilar Fragment Shader.\n" << infoLog << std::endl;
		glfwTerminate();
		exit(ERRO_COMPILAR_FRAGMENT_SHADER);
	}

	// Criar Shader Program
	unsigned int SHADER_PROGRAM = glCreateProgram();
	glAttachShader(SHADER_PROGRAM, vertexShader);
	glAttachShader(SHADER_PROGRAM, fragmentShader);
	glLinkProgram(SHADER_PROGRAM);
	glGetShaderiv(SHADER_PROGRAM, GL_LINK_STATUS, &sucesso);
	if (!sucesso) {
		glGetShaderInfoLog(SHADER_PROGRAM, 512, NULL, infoLog);
		std::cout << "Falha ao linkar programa de Shader.\n" << infoLog << std::endl;
		glfwTerminate();
		exit(ERRO_COMPILAR_SHADER_PROGRAM);
	}

	glUseProgram(SHADER_PROGRAM);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return SHADER_PROGRAM;
}


//#include <GL/glut.h>
//#include <GL/gl.h>
//#include <iostream>
//#include "Memoria.cpp"
//#include <fstream>
//#include "CPU.cpp"
//#include <vector>
//#include <chrono>
//
//#undef main
//
//struct Pixel {
//	GLubyte r, g, b;
//};
//
//Pixel cores[] = {
//	{0x7C,0x7C,0x7C},
//	{0x00,0x00,0xFC},
//	{0x00,0x00,0xBC},
//	{0x44,0x28,0xBC},
//	{0x94,0x00,0x84},
//	{0xA8,0x00,0x20},
//	{0xA8,0x10,0x00},
//	{0x88,0x14,0x00},
//	{0x50,0x30,0x00},
//	{0x00,0x78,0x00},
//	{0x00,0x68,0x00},
//	{0x00,0x58,0x00},
//	{0x00,0x40,0x58},
//	{0x00,0x00,0x00},
//	{0x00,0x00,0x00},
//	{0x00,0x00,0x00},
//	{0xBC,0xBC,0xBC},
//	{0x00,0x78,0xF8},
//	{0x00,0x58,0xF8},
//	{0x68,0x44,0xFC},
//	{0xD8,0x00,0xCC},
//	{0xE4,0x00,0x58},
//	{0xF8,0x38,0x00},
//	{0xE4,0x5C,0x10},
//	{0xAC,0x7C,0x00},
//	{0x00,0xB8,0x00},
//	{0x00,0xA8,0x00},
//	{0x00,0xA8,0x44},
//	{0x00,0x88,0x88},
//	{0x00,0x00,0x00},
//	{0x00,0x00,0x00},
//	{0x00,0x00,0x00},
//	{0xF8,0xF8,0xF8},
//	{0x3C,0xBC,0xFC},
//	{0x68,0x88,0xFC},
//	{0x98,0x78,0xF8},
//	{0xF8,0x78,0xF8},
//	{0xF8,0x58,0x98},
//	{0xF8,0x78,0x58},
//	{0xFC,0xA0,0x44},
//	{0xF8,0xB8,0x00},
//	{0xB8,0xF8,0x18},
//	{0x58,0xD8,0x54},
//	{0x58,0xF8,0x98},
//	{0x00,0xE8,0xD8},
//	{0x78,0x78,0x78},
//	{0x00,0x00,0x00},
//	{0x00,0x00,0x00},
//	{0xFC,0xFC,0xFC},
//	{0xA4,0xE4,0xFC},
//	{0xB8,0xB8,0xF8},
//	{0xD8,0xB8,0xF8},
//	{0xF8,0xB8,0xF8},
//	{0xF8,0xA4,0xC0},
//	{0xF0,0xD0,0xB0},
//	{0xFC,0xE0,0xA8},
//	{0xF8,0xD8,0x78},
//	{0xD8,0xF8,0x78},
//	{0xB8,0xF8,0xB8},
//	{0xB8,0xF8,0xD8},
//	{0x00,0xFC,0xFC},
//	{0xF8,0xD8,0xF8},
//	{0x00,0x00,0x00},
//	{0x00,0x00,0x00}
//};
//
//int FPS = 60;
//
//int WIDTH = 32;
//int HEIGHT = 32;
//
//std::vector<Pixel> screenBuffer(WIDTH*HEIGHT);
//GLuint textureID;
//bool needsUpdate = true;
//
//CPU cpu;
//Memoria mem;
//
//void timer_callback(int ms);
//void display_callback();
//void init();
//void reshape_callback(int x, int y);
//void drawGrid();
//void quadrado(int x, int y, int cont);
//void updateScreenBuffer();
//
//int main(int argc, char **argv) {
//
//	cpu.inicializar();
//	
//	// Carrega os dados na memoria
//	
//	std::string path = "snake.hex";
//	std::ifstream input(path, std::ios::in | std::ios::binary);
//	if (input.is_open() == false) {
//		return -1;
//	}
//		
//	input.seekg(0, input.end);
//	int tamanho = input.tellg();
//	input.seekg(0, input.beg);
//
//	for (int i = 0; i < tamanho; i++) {
//		input.read((char*)&mem[cpu.PC + i], sizeof(char));
//	}
//	
//	for (int i = 0; i < tamanho; i++) {
//		printf("%x ", mem[cpu.PC + i]);
//	}
//	printf("\n");
//	
//	// Dados carregados na memoria com sucesso
//
//	glutInit(&argc, argv);
//	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
//	glutInitWindowSize(500,500);
//	glutCreateWindow("F&GNES");
//	glutDisplayFunc(display_callback);
//	glutReshapeFunc(reshape_callback);
//	glutTimerFunc(0, timer_callback, 0);
//	init();
//	glutMainLoop();
//
//	return 0;
//}
//
//void timer_callback(int ms) {
//	glutPostRedisplay();
//	glutTimerFunc(1000/FPS, timer_callback, 0);
//}
//
//void drawGrid() {
//	int contador = 0;
//	for (int x = 0; x < WIDTH; x++) {
//		for (int y = 0; y < HEIGHT; y++) {
//			quadrado(x, y, contador++);
//		}
//	}
//}
//void quadrado(int x, int y, int cont) {
//	glLineWidth(1.0);
//	
//	int indice = mem[0x200 + cont];
//
//	glBegin(GL_QUADS);
//	glColor3ub(cores[indice].r, cores[indice].g, cores[indice].b);
//	glVertex2f(x, y);
//	glVertex2f(x+1, y);
//	glVertex2f(x+1, y+1);
//	glVertex2f(x, y+1);
//
//	glEnd();
//}
//
//// Modifique o reshape_callback()
//void reshape_callback(int w, int h) {
//	glViewport(0, 0, w, h);
//	glMatrixMode(GL_PROJECTION);
//	glLoadIdentity();
//	gluOrtho2D(0, WIDTH, HEIGHT, 0);
//	glMatrixMode(GL_MODELVIEW);
//}
//
//void init() {
//	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
//
//	// Configura textura
//	glGenTextures(1, &textureID);
//	glBindTexture(GL_TEXTURE_2D, textureID);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0,
//		GL_RGB, GL_UNSIGNED_BYTE, NULL);
//}
//
//void updateScreenBuffer() {
//	for (int y = 0; y < HEIGHT; y++) {
//		for (int x = 0; x < WIDTH; x++) {
//			int index = y * WIDTH + x;
//			int colorIndex = mem[0x200 + index];
//			screenBuffer[index] = {
//				(GLubyte)cores[colorIndex].r,
//				(GLubyte)cores[colorIndex].g,
//				(GLubyte)cores[colorIndex].b
//			};
//		}
//	}
//	needsUpdate = true;
//}
//
//// Modifique a display_callback()
//void display_callback() {
//	static auto lastTime = std::chrono::high_resolution_clock::now();
//
//	// Executa a CPU em velocidade real
//	auto now = std::chrono::high_resolution_clock::now();
//	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTime).count();
//	if (elapsed >= 16) { // ~60 FPS
//		if (cpu.PC != 0xFFFF) {
//			cpu.executar(mem);
//			if (cpu.atualizarGrafico) {
//				updateScreenBuffer();
//				cpu.atualizarGrafico = false;
//			}
//			else {
//				return;
//			}
//		}
//		else {
//			exit(0);
//		}
//		lastTime = now;
//	}
//
//	glClear(GL_COLOR_BUFFER_BIT);
//
//	// Renderiza a textura
//	if (needsUpdate) {
//		glBindTexture(GL_TEXTURE_2D, textureID);
//		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, WIDTH, HEIGHT,
//			GL_RGB, GL_UNSIGNED_BYTE, screenBuffer.data());
//		needsUpdate = false;
//	}
//
//	// Desenha a textura em um quadrado
//	glEnable(GL_TEXTURE_2D);
//	glBindTexture(GL_TEXTURE_2D, textureID);
//	glBegin(GL_QUADS);
//	glTexCoord2f(0, 1); glVertex2f(0, 0);
//	glTexCoord2f(1, 1); glVertex2f(WIDTH, 0);
//	glTexCoord2f(1, 0); glVertex2f(WIDTH, HEIGHT);
//	glTexCoord2f(0, 0); glVertex2f(0, HEIGHT);
//	glEnd();
//	glDisable(GL_TEXTURE_2D);
//
//	glutSwapBuffers();
//	glutPostRedisplay(); // Mantém a animação
//}

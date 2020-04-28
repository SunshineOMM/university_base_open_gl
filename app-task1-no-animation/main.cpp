﻿#include <iostream>       // cout
#include <stdexcept>      // runtime_error
#include <vector>
#include <core.h>
#include <time.h>
#include <cstdlib>

using namespace std;


// Функция, которая будет обрабатывать ошибки GLFW.
void ErrorCallback(const int code, const char* const error) {
	throw runtime_error(error);
}



//=====================================================================================================================
//															ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ
//=====================================================================================================================
	// модель точки:
	// позиция vec2 p
	// цвет vec3 rgb

const size_t N = 100; // Количество точек
const auto STRIDE = 6 * sizeof(GLfloat); // С учётом выравнивания по модели std430
const GLfloat POINT_SIZE = 3;            // Размер отрисовываемых точек




int main() {
	// Основной код приложения пишите здесь
		// Кидайте runtime_error("Exception message") в случае какой-либо ошибки
		// Инициализация GLFW с проверкой

	// Устанавливаем функцию обработки ошибок GLFW.
	glfwSetErrorCallback(ErrorCallback);
	try {
		if (glfwInit() == GLFW_FALSE) throw runtime_error("Failed to init GLFW!");

		// Создание окна
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		GLFWwindow* window;
		{
			auto monitor = glfwGetPrimaryMonitor();
			auto mode = glfwGetVideoMode(monitor);
			window = glfwCreateWindow(mode->width, mode->height, "TITLE", nullptr, nullptr); //mode->width, mode->height,monitor
			if (window == nullptr)
				throw runtime_error("Failed to create window!");
			glfwSetKeyCallback(window, KeyCallback);
			glfwMakeContextCurrent(window);

			int w, h;
			glfwGetFramebufferSize(window, &w, &h);

			glViewport(0, 0, w, h);
			glEnable(GL_PROGRAM_POINT_SIZE);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}

		// Устанавливаем функции обработки событий клавиатуры  
		glfwSetKeyCallback(window, KeyCallback);
		

		// Делаем созданное окно текущим.
		// Все дальнейшие вызываемые функции OpenGL будут относиться к данному окну.
		glfwMakeContextCurrent(window);

		// Инициализируем GLEW.
		// Происходит инициализация функций OpenGL.
		{
			auto error = glewInit();
			if (error != GLEW_OK) {

				auto message = reinterpret_cast<const char*>(glewGetErrorString(error)); throw runtime_error(string("Failed to initialize GLEW: ") + message);

			}

		}
		glEnable(GL_MULTISAMPLE);
		glEnable(GL_PROGRAM_POINT_SIZE);

		//место для кода отрисовки точек

		

		// =========================
		// СБОРКА ШЕЙДЕРНЫХ ПРОГРАММ
		// =========================

		ShaderProgram renderProgram({ {"shaders/vertex.glsl",   GL_VERTEX_SHADER},{"shaders/fragment.glsl", GL_FRAGMENT_SHADER} });



		const auto renderPointSizeLocation = glGetUniformLocation(renderProgram.id, "pointSize");
		// =================================
		// СОЗДАНИЕ VBO НЕОБХОДИМОГО РАЗМЕРА
		// =================================

		GLuint vbo;
		{
			glGenBuffers(1, &vbo);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);                 // Привязка VBO к вершинному шейдеру
			vector<GLfloat> data(N * 6);
			glBufferData(GL_ARRAY_BUFFER, N * STRIDE, data.data(), GL_DYNAMIC_COPY);
			for (int i = 0; i < data.size(); ++i) {
				switch (i % 6) {
				case 0: {
					data[i] = rand() % 50;
					break;
				}
				case 1: {
					data[i] = rand() % 50;
					break;
				}
				case 3: {
					data[i] = rand() % 255;
					break;
				}
				case 4: {
					data[i] = rand() % 255;
					break;
				}
				case 5: {
					data[i] = rand() % 255;
					break;
				}
				}
			}
		}
		// ============================
		// СОЗДАНИЕ И ИНИЦИАЛИЗАЦИЯ VAO
		// ============================

		GLuint vao;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		{
			const auto locationP = glGetAttribLocation(renderProgram.id, "p");
			const auto locationRGB = glGetAttribLocation(renderProgram.id, "rgb");
			if (locationP == -1 || locationRGB == -1 )
				throw runtime_error("Failed to locate render attribute(s)!");

			glVertexAttribPointer(locationP, 2, GL_FLOAT, GL_FALSE, STRIDE, nullptr);
			glVertexAttribPointer(locationRGB, 3, GL_FLOAT, GL_FALSE, STRIDE, reinterpret_cast<GLvoid*>(3 * sizeof(GLfloat)));

			glEnableVertexAttribArray(locationP);
			glEnableVertexAttribArray(locationRGB);
		}

		// ===========================
		// ПРЕДСТАРТОВАЯ ПОДГОТОВКА...
		// ===========================
		glUseProgram(renderProgram.id);

		glUniform1f(renderPointSizeLocation, POINT_SIZE);





		// ========
		// ПОЕХАЛИ!
		// ========
		while (!glfwWindowShouldClose(window)) {

			// Устанавливаем цвет очистки экрана в зависимости от позиции курсора мыши.
			// Первые три параметра обозначают цвет RGB.
			// Подробнее про RGB: https://ru.wikipedia.org/w/index.php?title=RGB&stable=1.
			// Последний параметр означает непрозрачность.
			glClearColor(255,255,255, 1);//   0,191,255,

			// Очищаем экран.
			glClear(GL_COLOR_BUFFER_BIT);


			glDrawArrays(GL_POINTS, 0, N);


			glfwSwapBuffers(window);
			// Блокируем выполнение текущего потока, пока не произойдёт какого-либо события.
			// Если необходимо отрисовывать следующий кадр, не дожидаясь событий,
			// используйте glfwPollEvents() вместо glfwWaitEvents().
			glfwWaitEvents();

		}

	}
	catch (const exception& e) {
		glfwTerminate();
		cout << e.what() << endl;		//ЗАПОМНИТЬ!!!
		return EXIT_FAILURE;
	}
}
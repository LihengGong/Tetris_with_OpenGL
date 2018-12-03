// This example is heavily based on the tutorial at https://open.gl

// OpenGL Helpers to reduce the clutter
#include "Helpers.h"

// GLFW is necessary to handle the OpenGL context
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <vector>
// Linear Algebra Library
#include <Eigen/Core>
#include <Eigen/Dense>

// Timer
#include <chrono>

#include <unistd.h>
#include <spawn.h>
#include <sys/wait.h>

const int TOTAL_SQUARE_NUM = 400;
const int TOTAL_ROWS = 20;
const int TOTAL_COLS = 20;
// Contains the vertex for Bezier curves
const double bc_step = 0.001;
const int BC_VERTICE_NUM = (int)((double)1.0 / (double)bc_step);

const double EPSILON = 0.00000001;
const double PI  =3.141592653589793238463;

bool board_grid[TOTAL_ROWS][TOTAL_COLS];
bool board_grid_backup[TOTAL_ROWS][TOTAL_COLS];
TetrisShape *pTshape = NULL;
bool is_ending = false;
long long int total_smashed = 1;

void mouse_button_callback4(GLFWwindow* window, int button, int action, int mods);

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS) {
    // Update the position of the first vertex if the keys 1,2, or 3 are pressed
        switch (key)
        {
            case GLFW_KEY_LEFT:
                if (pTshape != NULL && pTshape->can_move_left()) {
                    pTshape->move_left();
                }
                break;
            case GLFW_KEY_RIGHT:
                if (pTshape != NULL && pTshape->can_move_right()) {
                    pTshape->move_right();
                }
                break;
            case GLFW_KEY_DOWN:
                if (pTshape != NULL && pTshape->can_move_down()) {
                    pTshape->move_down();
                }
                break;
            case GLFW_KEY_UP:
                if (pTshape != NULL && pTshape->can_morph()) {
                    pTshape->morph();
                }
                break;
            case GLFW_KEY_SPACE:
                if (pTshape != NULL) {
                    pTshape->move_to_bottom();
                }
                break;
            default:
                break;
        }
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
}

void check_grid() {
    for (int row = 0; row < TOTAL_ROWS; ++row) {
        for (int col = 0; col < TOTAL_COLS; ++col) {
            board_grid_backup[row][col] = board_grid[row][col];
        }
    }

    std::vector<int> ind_vec;
    for (int row = 0; row < TOTAL_ROWS; ++row) {
        int sq_set_num = 0;
        for (int col = 0; col < TOTAL_COLS; ++col) {
            if (board_grid[row][col]) {
                ++sq_set_num;
            }
        }

        if (sq_set_num == TOTAL_COLS) {
            std::cout << "Shift down one row";
            ++total_smashed;
            ind_vec.push_back(row);
        }
    }

    int real_row = TOTAL_ROWS - 1;
    for (int row = TOTAL_ROWS - 1; row >=0 ; --row) {
        size_t cursize = ind_vec.size();
        if (cursize > 0 && ind_vec[cursize - 1] == row) {
            ind_vec.pop_back();
            continue;
        }

        for (int col = 0; col < TOTAL_COLS; ++col) {
            board_grid[real_row][col] = board_grid_backup[row][col];
        }
        real_row--;
    }

    for (int row = real_row; row >= 0; --row) {
        for (int col = 0; col < TOTAL_COLS; ++col) {
            board_grid[row][col] = false;
        }
    }
}

void check_game_ending() {
    if (pTshape != NULL) {
        for (int i = 0; i < SQUARE_PER_SHAPE; ++i) {
            if (board_grid[pTshape->cdnt[i].x][pTshape->cdnt[i].y]) {
                is_ending = true;
            }
        }
    }
}

void run_game() {
    if (pTshape != NULL && pTshape->can_move_down()) {
        pTshape->move_down();
    } else if (pTshape != NULL) {
        pTshape->persist();
        delete pTshape;
        pTshape = NULL;
    } else {
        std::cout << "need new Tetris shape\n";
        SHAPE_TYPE rnd_type = static_cast<SHAPE_TYPE>(rand() % TETRIS_TOTALSHAPE);
        pTshape = new TetrisShape(rnd_type);
        // pTshape = new TetrisShape(TETRIS_RIGHTNSHAPE);
    }
    check_game_ending();
    check_grid();
}

void render_game(OglRect *pRects[TOTAL_SQUARE_NUM]) {
    for (int r = 0; r < TOTAL_ROWS; ++r) {
        for (int c = 0; c < TOTAL_COLS; ++c) {
            if (board_grid[r][c] || 
                (pTshape != NULL && pTshape->is_display(r, c))) {
                pRects[r * TOTAL_ROWS + c]->render();
            }
        }
    }
}

void free_game_memory(OglRect *pRects[TOTAL_SQUARE_NUM]) {
    if (pTshape != NULL) {
        delete pTshape;
        pTshape = NULL;
    }

    for (int col = 0; col < TOTAL_ROWS; ++col) {
        for (int row = 0; row < TOTAL_COLS; ++row) {
            delete pRects[col * TOTAL_ROWS + row];
        }
    }
}

int task_4() {
    for (int r = 0; r < TOTAL_ROWS; ++r) {
        for (int c = 0; c < TOTAL_COLS; ++c) {
            board_grid[r][c] = false;
        }
    }
    std::cout << "size of board grid:" << sizeof(board_grid) << "\n";

    for (int k = 0; k < 10; ++k) {
        board_grid[18][k] = true;
    }
    for (int k = 14; k < 20; ++k) {
        board_grid[18][k] = true;
    }

    srand(time(0));

    GLFWwindow* window;
    
    Program program;
    printf("task 4\n");

    double cur_scale = 1.0;
    double cur_shift_right = 0.;
    double cur_shift_up = 0.;

    // Initialize the library
    if (!glfwInit())
        return -1;

    // Activate supersampling
    glfwWindowHint(GLFW_SAMPLES, 8);

    // Ensure that we get at least a 3.2 context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

    // On apple we have to load a core profile with forward compatibility
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create a windowed mode window and its OpenGL context
    window = glfwCreateWindow(800, 800, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    int major, minor, rev;
    major = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MAJOR);
    minor = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MINOR);
    rev = glfwGetWindowAttrib(window, GLFW_CONTEXT_REVISION);
    printf("OpenGL version recieved: %d.%d.%d\n", major, minor, rev);
    printf("Supported OpenGL is %s\n", (const char*)glGetString(GL_VERSION));
    printf("Supported GLSL is %s\n", (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));

    // Register the keyboard callback
    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    OglRect::init();

    // Save the current time --- it will be used to dynamically change the triangle color
    auto t_start = std::chrono::high_resolution_clock::now();
    // Register the mouse callback
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    long long int nbFrames = 0;
    long long int newFrames = 0;

    double lastTime = glfwGetTime();
    double newLastTime = glfwGetTime();

    const double maxFPS = 60.0;
    const double maxPeriod = 1.0 / maxFPS;
    double drop_speed = 1.6;

    OglRect *pRects[TOTAL_SQUARE_NUM];
    for (int col = 0; col < TOTAL_ROWS; ++col) {
        for (int row = 0; row < TOTAL_COLS; ++row) {
            pRects[col * TOTAL_ROWS + row] = new OglRect(row, col);
        }
    }

    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window))
    {
        // Set the uniform value depending on the time difference
        auto t_now = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();
        // glUniform3f(program.uniform("triangleColor"), (float)(sin(time * 4.0f) + 1.0f) / 2.0f, 0.0f, 0.0f);

        double newCurrentTime = glfwGetTime();

        double deltaTime = newCurrentTime - newLastTime;
        if (deltaTime >= maxPeriod) {
            newLastTime = newCurrentTime;
            double currentTime = glfwGetTime();
            nbFrames++;
            if ( (currentTime - lastTime) >= (1.0 / drop_speed) ){ // If last prinf() was more than 1 sec ago
                // printf and reset timer

                printf("%f ms/frame\n", 1000.0/double(nbFrames));
                nbFrames = 0;
                lastTime += 1.0 / drop_speed;
                run_game();
                if (total_smashed % 12 == 0 && drop_speed < 10.0) {
                    printf("toal smashed: %lld\n", total_smashed);
                    total_smashed = 1;
                    drop_speed += 0.1;
                }
            }
            // Get size of the window
            int width, height;
            glfwGetWindowSize(window, &width, &height);

            // Clear the framebuffer
            glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            render_game(pRects);

            // Swap front and back buffers
            glfwSwapBuffers(window);
        }
        if (is_ending) {
            break;
        }
        // Poll for and process events
        glfwPollEvents();
    }

    glfwTerminate();
    OglRect::teardown();

    free_game_memory(pRects);
    exit(0);
    return 0;
}


int main(void) {
    int status;
    task_4();

    return 0;
}
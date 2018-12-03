#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <vector>
#include <Eigen/Core>
#include <Eigen/Dense>

#define GL_SILENCE_DEPRECATION

#include <chrono>

#include <unistd.h>
#include <spawn.h>
#include <sys/wait.h>



#ifdef _WIN32
#  include <windows.h>
#  undef max
#  undef min
#  undef DrawText
#endif

#ifndef __APPLE__
#  define GLEW_STATIC
#  include <GL/glew.h>
#endif

#ifdef __APPLE__
#   include <OpenGL/gl3.h>
#   define __gl_h_ /* Prevent inclusion of the old gl.h */
#else
#   ifdef _WIN32
#       include <windows.h>
#   endif
#   include <GL/gl.h>
#endif

enum Op_mode {
    INIT_MODE,
    INSERTION_MODE,
    DRAG_MODE,
    TRANSLATION_MODE,
    COLOR_MODE,
    DELETE_MODE,
    VIEW_CONTROL_MODE
};

enum TRANSLATION_SUBMODE {
    INIT_SUB_MODE,
    ROTATE_CLOCKWISE_MODE,
    ROTATE_COUNTERCLOCKWISE_MODE,
    SCALE_UP_MODE,
    SCALE_DOWN_MODE,
    PAINT_MODE
};

enum MOV_DIRECTION {
    MOV_LEFT,
    MOV_RIGHT,
    MOV_DOWN,
    MOV_UP
};

enum VIEW_SUBMODE {
    VIEW_INIT_MODE,
    VIEW_PLUS_MODE,
    VIEW_MINUS_MODE,
    VIEW_PAN_RIGHT_MODE,
    VIEW_PAN_LEFT_MODE,
    VIEW_PAN_UP_MODE,
    VIEW_PAN_DOWN_MODE
};


class VertexArrayObject
{
public:
    unsigned int id;

    VertexArrayObject() : id(0) {}

    // Create a new VAO
    void init();

    // Select this VAO for subsequent draw calls
    void bind();

    // Release the id
    void free();
};

class VertexBufferObject
{
public:
    typedef unsigned int GLuint;
    typedef int GLint;

    GLuint id;
    GLuint rows;
    GLuint cols;

    VertexBufferObject() : id(0), rows(0), cols(0) {}

    // Create a new empty VBO
    void init();

    // Updates the VBO with a matrix M
    void update(const Eigen::MatrixXf& M);

    // Select this VBO for subsequent draw calls
    void bind();

    // Release the id
    void free();
};

// This class wraps an OpenGL program composed of two shaders
class Program
{
public:
  typedef unsigned int GLuint;
  typedef int GLint;

  GLuint vertex_shader;
  GLuint fragment_shader;
  GLuint program_shader;

  Program() : vertex_shader(0), fragment_shader(0), program_shader(0) { }

  // Create a new shader from the specified source strings
  bool init(const std::string &vertex_shader_string,
  const std::string &fragment_shader_string,
  const std::string &fragment_data_name);

  // Select this shader for subsequent draw calls
  void bind();

  // Release all OpenGL objects
  void free();

  // Return the OpenGL handle of a named shader attribute (-1 if it does not exist)
  GLint attrib(const std::string &name) const;

  // Return the OpenGL handle of a uniform attribute (-1 if it does not exist)
  GLint uniform(const std::string &name) const;

  // Bind a per-vertex array attribute
  GLint bindVertexAttribArray(const std::string &name, VertexBufferObject& VBO) const;

  GLuint create_shader_helper(GLint type, const std::string &shader_string);

};

// From: https://blog.nobel-joergensen.com/2013/01/29/debugging-opengl-using-glgeterror/
void _check_gl_error(const char *file, int line);

///
/// Usage
/// [... some opengl calls]
/// glCheckError();
///
#define check_gl_error() _check_gl_error(__FILE__,__LINE__)



class OglRect {
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    Eigen::Matrix4f model;
    static const GLchar* vertex_shader;
    static const GLchar* fragment_shader;
    static Program program;
    bool is_visible;
    int grid_x, grid_y;

    static const int LEFT_MOST;
    static const int RIGHT_MOST;
    static const float GRID_WIDTH;
    static const int TOTAL_TRIANGLES;
    static int SQUARE_TRIANGLE_NUM;

    static Eigen::MatrixXf V;
    static Eigen::MatrixXf C;

    static VertexArrayObject VAO;
    static VertexBufferObject VBO;
    static VertexBufferObject VBO_C;

    OglRect(int x, int y) {
        model = Eigen::Matrix4f::Identity();
        is_visible = true;
        grid_x = x + LEFT_MOST;
        grid_y = RIGHT_MOST - 1 - y;
        scale(GRID_WIDTH);
        translate(grid_x * (GRID_WIDTH / 2.0), grid_y * (GRID_WIDTH / 2.0));
    }

    OglRect(int x, int y, bool is_vis) {
        OglRect(x, y);
        is_visible = is_vis;
    }

    static void init();
    static void teardown();
    void render();
    void scale(float fac);
    void translate(float dist_x, float dist_y);
};

enum SHAPE_TYPE {
    TETRIS_LSHAPE,
    TETRIS_GAMMASHAPE,
    TETRIS_STRIPSHAPE,
    TETRIS_TSHAPE,
    TETRIS_SQUARESHAPE,
    TETRIS_LEFTNSHAPE,
    TETRIS_RIGHTNSHAPE,
    TETRIS_TOTALSHAPE
};

enum SHAPE_SUBTYPE {
    LSHAPE_DOWN,
    LSHAPE_LEFT,
    LSHAPE_UP,
    LSHAPE_RIGHT,
    GAMMASHAPE_DOWN,
    GAMMASHAPE_RIGHT,
    GAMMASHAPE_UP,
    GAMMASHAPE_LEFT,
    STRIPSHAPE_LANDSCAPE,
    STRIPSHAPE_PORTRAIT,
    TSHAPE_DOWN,
    TSHAPE_LEFT,
    TSHAPE_UP,
    TSHAPE_RIGHT,
    LEFTNSHAPE_VERTICAL,
    LEFTNSHAPE_HORIZONTAL,
    RIGHTNSHAPE_VERTICAL,
    RIGHTNSHAPE_HORIZONTAL
};

struct coordinate {
    int x;
    int y;
    coordinate(): x(0), y(0) {}
    coordinate(int _x, int _y): x(_x), y(_y) {}

    void move_down() {
        ++x;
    }

    void move_left() {
        --y;
    }

    void move_right() {
        ++y;
    }
};

const GLuint SQUARE_PER_SHAPE = 4;

class TetrisShape {
public:
    SHAPE_TYPE stype;
    SHAPE_SUBTYPE shsubtype;
    coordinate cdnt[SQUARE_PER_SHAPE];
    TetrisShape(SHAPE_TYPE t);

    int leftmost();

    int rightmost();

    int upmost();

    int downmost();

    void move_left();

    void move_right();

    void move_down();

    void move_to_bottom();

    bool can_move_left();

    bool can_move_right();

    bool can_move_down();

    bool can_morph_stripe();

    bool can_morph_lshape();
    bool can_lshape_down_to_left();
    bool can_lshape_left_to_up();
    bool can_lshape_up_to_right();
    bool can_lshape_right_to_down();

    bool can_morph_gammashape();
    bool can_gammashape_down_to_right();
    bool can_gammashape_right_to_up();
    bool can_gammashape_up_to_left();
    bool can_gammashape_left_to_down();

    bool can_morph_leftnshape();

    bool can_morph_rightnshape();

    bool can_morph_tshape();
    bool can_tshape_down_to_left();

    bool can_morph();

    void morph();
    void morph_stripshape();
    void morph_lshape();
    void morph_gammashape();
    void morph_tshape();
    void morph_leftnshape();
    void morph_rightnshape();

    bool is_display(int _x, int _y);

    void persist();
};

#endif

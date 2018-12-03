#include "Helpers.h"

#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>


const double EPSILON = 0.00000001;
const double PI  =3.141592653589793238463;

int TOTAL_SQUARE_NUM = 400;

void VertexArrayObject::init()
{
  glGenVertexArrays(1, &id);
  check_gl_error();
}

void VertexArrayObject::bind()
{
  glBindVertexArray(id);
  check_gl_error();
}

void VertexArrayObject::free()
{
  glDeleteVertexArrays(1, &id);
  check_gl_error();
}

void VertexBufferObject::init()
{
  glGenBuffers(1,&id);
  check_gl_error();
}

void VertexBufferObject::bind()
{
  glBindBuffer(GL_ARRAY_BUFFER,id);
  check_gl_error();
}

void VertexBufferObject::free()
{
  glDeleteBuffers(1,&id);
  check_gl_error();
}

void VertexBufferObject::update(const Eigen::MatrixXf& M)
{
  assert(id != 0);
  glBindBuffer(GL_ARRAY_BUFFER, id);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float)*M.size(), M.data(), GL_DYNAMIC_DRAW);
  rows = M.rows();
  cols = M.cols();
  check_gl_error();
}

bool Program::init(
  const std::string &vertex_shader_string,
  const std::string &fragment_shader_string,
  const std::string &fragment_data_name)
{
  using namespace std;
  vertex_shader = create_shader_helper(GL_VERTEX_SHADER, vertex_shader_string);
  fragment_shader = create_shader_helper(GL_FRAGMENT_SHADER, fragment_shader_string);

  if (!vertex_shader || !fragment_shader)
    return false;

  program_shader = glCreateProgram();

  glAttachShader(program_shader, vertex_shader);
  glAttachShader(program_shader, fragment_shader);

  glBindFragDataLocation(program_shader, 0, fragment_data_name.c_str());
  glLinkProgram(program_shader);

  GLint status;
  glGetProgramiv(program_shader, GL_LINK_STATUS, &status);

  if (status != GL_TRUE)
  {
    char buffer[512];
    glGetProgramInfoLog(program_shader, 512, NULL, buffer);
    cerr << "Linker error: " << endl << buffer << endl;
    program_shader = 0;
    return false;
  }

  check_gl_error();
  return true;
}

void Program::bind()
{
  glUseProgram(program_shader);
  check_gl_error();
}

GLint Program::attrib(const std::string &name) const
{
  return glGetAttribLocation(program_shader, name.c_str());
}

GLint Program::uniform(const std::string &name) const
{
  return glGetUniformLocation(program_shader, name.c_str());
}

GLint Program::bindVertexAttribArray(
        const std::string &name, VertexBufferObject& VBO) const
{
  GLint id = attrib(name);
  if (id < 0)
    return id;
  if (VBO.id == 0)
  {
    glDisableVertexAttribArray(id);
    return id;
  }
  VBO.bind();
  glEnableVertexAttribArray(id);
  glVertexAttribPointer(id, VBO.rows, GL_FLOAT, GL_FALSE, 0, 0);
  check_gl_error();

  return id;
}

void Program::free()
{
  if (program_shader)
  {
    glDeleteProgram(program_shader);
    program_shader = 0;
  }
  if (vertex_shader)
  {
    glDeleteShader(vertex_shader);
    vertex_shader = 0;
  }
  if (fragment_shader)
  {
    glDeleteShader(fragment_shader);
    fragment_shader = 0;
  }
  check_gl_error();
}

GLuint Program::create_shader_helper(GLint type, const std::string &shader_string)
{
  using namespace std;
  if (shader_string.empty())
    return (GLuint) 0;

  GLuint id = glCreateShader(type);
  const char *shader_string_const = shader_string.c_str();
  glShaderSource(id, 1, &shader_string_const, NULL);
  glCompileShader(id);

  GLint status;
  glGetShaderiv(id, GL_COMPILE_STATUS, &status);

  if (status != GL_TRUE)
  {
    char buffer[512];
    if (type == GL_VERTEX_SHADER)
      cerr << "Vertex shader:" << endl;
    else if (type == GL_FRAGMENT_SHADER)
      cerr << "Fragment shader:" << endl;
    else if (type == GL_GEOMETRY_SHADER)
      cerr << "Geometry shader:" << endl;
    cerr << shader_string << endl << endl;
    glGetShaderInfoLog(id, 512, NULL, buffer);
    cerr << "Error: " << endl << buffer << endl;
    return (GLuint) 0;
  }
  check_gl_error();

  return id;
}

void _check_gl_error(const char *file, int line)
{
  GLenum err (glGetError());

  while(err!=GL_NO_ERROR)
  {
    std::string error;

    switch(err)
    {
      case GL_INVALID_OPERATION:      error="INVALID_OPERATION";      break;
      case GL_INVALID_ENUM:           error="INVALID_ENUM";           break;
      case GL_INVALID_VALUE:          error="INVALID_VALUE";          break;
      case GL_OUT_OF_MEMORY:          error="OUT_OF_MEMORY";          break;
      case GL_INVALID_FRAMEBUFFER_OPERATION:  error="INVALID_FRAMEBUFFER_OPERATION";  break;
    }

    std::cerr << "GL_" << error.c_str() << " - " << file << ":" << line << std::endl;
    err = glGetError();
  }
}

const int TOTAL_ROWS = 20;
const int TOTAL_COLS = 20;

extern bool board_grid[TOTAL_ROWS][TOTAL_COLS];

Program OglRect::program;
const GLchar* OglRect::vertex_shader =
            "#version 150 core\n"
                    "in vec2 position;"
                    "in vec3 color;"
                    "uniform mat4 model;"
                    "uniform float visible;"
                    "out vec3 f_color;"
                    "void main()"
                    "{"
                    "    gl_Position = model * vec4(position, visible, 1.0);"
                    "    f_color = color;"
                    "}";

const GLchar* OglRect::fragment_shader =
        "#version 150 core\n"
                "in vec3 f_color;"
                "out vec4 outColor;"
                "void main()"
                "{"
                "    outColor = vec4(f_color, 1.0);"
                "}";

int OglRect::SQUARE_TRIANGLE_NUM = 2;


const int OglRect::LEFT_MOST = -9;
const int OglRect::RIGHT_MOST = 10;
const float OglRect::GRID_WIDTH = 0.2;
const int OglRect::TOTAL_TRIANGLES = 6;

Eigen::MatrixXf OglRect::V;
Eigen::MatrixXf OglRect::C;

VertexArrayObject OglRect::VAO;
VertexBufferObject OglRect::VBO;
VertexBufferObject OglRect::VBO_C;

void OglRect::init() {
    
    V.resize(2, 3 * SQUARE_TRIANGLE_NUM * TOTAL_SQUARE_NUM);
    Eigen::MatrixXf onesquare(2, 3 * SQUARE_TRIANGLE_NUM);
    onesquare << -0.5, -0.5, 0.,  -0.5, 0.,  0., 
                0,    0.5, 0.5, 0.,   0.,  0.5;
    for (int ind = 0; ind < TOTAL_SQUARE_NUM; ++ind) {
        for (int col = 0; col < 3 * SQUARE_TRIANGLE_NUM; ++col) {
            V.col(ind * 3 * SQUARE_TRIANGLE_NUM + col) = onesquare.col(col);
        }
    }

    C.resize(3, 3 * SQUARE_TRIANGLE_NUM * TOTAL_SQUARE_NUM);
    Eigen::MatrixXf C1(3, 3);
    C1 <<
    1,  0, 0,
    0,  1, 0,
    0,  0, 1;
    for (int ind = 0; ind < SQUARE_TRIANGLE_NUM * TOTAL_SQUARE_NUM; ind++) {
        C.col(ind*3 + 0) = C1.col(0);
        C.col(ind*3 + 1) = C1.col(1);
        C.col(ind*3 + 2) = C1.col(2);
    }
    program.init(vertex_shader,fragment_shader,"outColor");
    program.bind();

    VAO.init();
    VAO.bind();

    VBO.init();
    VBO_C.init();

    VBO.update(V);
    VBO_C.update(C);

    program.bindVertexAttribArray("position",VBO);
    program.bindVertexAttribArray("color", VBO_C);

    
}

void OglRect::teardown() {
    program.free();
    VAO.free();
    VBO.free();
    VBO_C.free();
}

void OglRect::render() {
    VAO.bind();

    // Bind your program
    program.bind();

    glUniformMatrix4fv(program.uniform("model"), 1, GL_FALSE, model.data());
    if (is_visible) {
      glUniform1f(program.uniform("visible"), 0.);
    } else {
      glUniform1f(program.uniform("visible"), -10.0);
    }
    glDrawArrays(GL_TRIANGLES, 0, TOTAL_TRIANGLES);
}

void OglRect::scale(float fac) {
    Eigen::Matrix4f scl = Eigen::Matrix4f::Identity();
    scl(0, 0) *= fac;
    scl(1, 1) *= fac;
    model = scl * model;
}

void OglRect::translate(float dist_x, float dist_y) {
    Eigen::Matrix4f mov = Eigen::Matrix4f::Identity();
    mov.col(3) << dist_x, dist_y, 0., 1.0;
    model = mov * model;
}


void update_game_logic() {
    
}

TetrisShape::TetrisShape(SHAPE_TYPE t): stype(t) {
    cdnt[0] = coordinate(0, 10); 
    switch (stype) {
        case TETRIS_LSHAPE:
            cdnt[1] = coordinate(0, 11);
            cdnt[2] = coordinate(0, 12);
            cdnt[3] = coordinate(1, 12);
            shsubtype = LSHAPE_DOWN;
            break;
        case TETRIS_GAMMASHAPE:
            cdnt[1] = coordinate(0, 9);
            cdnt[2] = coordinate(0, 8);
            cdnt[3] = coordinate(1, 8);
            shsubtype = GAMMASHAPE_DOWN;
            break;
        case TETRIS_STRIPSHAPE:
            cdnt[1] = coordinate(0, 11);
            cdnt[2] = coordinate(0, 12);
            cdnt[3] = coordinate(0, 13);
            shsubtype = STRIPSHAPE_LANDSCAPE;
            break;
        case TETRIS_TSHAPE:
            cdnt[1] = coordinate(0, 11);
            cdnt[2] = coordinate(0, 12);
            cdnt[3] = coordinate(1, 11);
            shsubtype = TSHAPE_DOWN;
            break;
        case TETRIS_SQUARESHAPE:
            cdnt[1] = coordinate(0, 11);
            cdnt[2] = coordinate(1, 10);
            cdnt[3] = coordinate(1, 11);
            break;
        case TETRIS_LEFTNSHAPE:
            cdnt[1] = coordinate(1, 10);
            cdnt[2] = coordinate(1, 11);
            cdnt[3] = coordinate(2, 11);
            shsubtype = LEFTNSHAPE_VERTICAL;
            break;
        case TETRIS_RIGHTNSHAPE:
            cdnt[1] = coordinate(1, 10);
            cdnt[2] = coordinate(1, 9);
            cdnt[3] = coordinate(2, 9);
            shsubtype = RIGHTNSHAPE_VERTICAL;
            break;
        default:
            break;
    }
}

int TetrisShape::leftmost() {
    int res = cdnt[0].y;
    for (int i = 1; i < SQUARE_PER_SHAPE; ++i) {
        res = std::min(res, cdnt[i].y);
    }
    return res;
}

int TetrisShape::rightmost() {
    int res = cdnt[0].y;
    for (int i = 1; i < SQUARE_PER_SHAPE; ++i) {
        res = std::max(res, cdnt[i].y);
    }
    return res;
}

int TetrisShape::upmost() {
    int res = cdnt[0].x;
    for (int i = 1; i < SQUARE_PER_SHAPE; ++i) {
        res = std::min(res, cdnt[i].x);
    }
    return res;
}

int TetrisShape::downmost() {
    int res = cdnt[0].x;
    for (int i = 1; i < SQUARE_PER_SHAPE; ++i) {
        res = std::max(res, cdnt[i].x);
    }
    return res;
}

void TetrisShape::move_left() {
    for (int i = 0; i < SQUARE_PER_SHAPE; ++i) {
        cdnt[i].move_left();
    }
}

void TetrisShape::move_right() {
    for (int i = 0; i < SQUARE_PER_SHAPE; ++i) {
        cdnt[i].move_right();
    }
}

void TetrisShape::move_down() {
    for (int i = 0; i < SQUARE_PER_SHAPE; ++i) {
        cdnt[i].move_down();
    }
}

bool TetrisShape::can_move_left() {
    for (int i = 0; i < SQUARE_PER_SHAPE; ++i) {
      if (board_grid[cdnt[i].x][cdnt[i].y - 1]) {
        return false;
      }
    }
    return (leftmost() > 0);
}

bool TetrisShape::can_move_right() {
    for (int i = 0; i < SQUARE_PER_SHAPE; ++i) {
      if (board_grid[cdnt[i].x][cdnt[i].y + 1]) {
        return false;
      }
    }
    return (rightmost() < 19);
}

bool TetrisShape::can_move_down() {
    for (int i = 0; i < SQUARE_PER_SHAPE; ++i) {
      if (board_grid[cdnt[i].x + 1][cdnt[i].y]) {
        return false;
      }
    }
    return (downmost() < 19);
}

bool TetrisShape::is_display(int _x, int _y) {
    for (int i = 0; i < SQUARE_PER_SHAPE; ++i) {
        if (_x == cdnt[i].x && _y == cdnt[i].y) {
            return true;
        }
    }

    return false;
}

void TetrisShape::persist() {
    for (int i = 0; i < SQUARE_PER_SHAPE; ++i) {
        board_grid[cdnt[i].x][cdnt[i].y] = true;
    }
}

bool TetrisShape::can_morph_stripe() {
    switch (shsubtype) {
      case STRIPSHAPE_LANDSCAPE:
          for (int i = 0; i < SQUARE_PER_SHAPE; ++i) {
            if (board_grid[cdnt[0].x + i][cdnt[0].y]) {
                return false;
            }
          }
          return (cdnt[0].x + 3) < 20;
          break;
      case STRIPSHAPE_PORTRAIT:
          for (int i = 0; i < SQUARE_PER_SHAPE; ++i) {
            if (board_grid[cdnt[0].x][cdnt[0].y + i]) {
                return false;
            }
          }
          return (cdnt[0].y + 3) < 20;
          break;
      default:
          return false;
          break;
    }
    return false;
}

bool TetrisShape::can_lshape_down_to_left() {
    for (int i = 0; i < SQUARE_PER_SHAPE - 1; ++i) {
        if ( (cdnt[2].x + i > 19) ||
              board_grid[cdnt[2].x + i][cdnt[2].y]) {
            std::cout << "i is %d\n";
            return false;
        }
    }
    if ((cdnt[1].x + 2 > 19) ||
          board_grid[cdnt[1].x + 2][cdnt[2].y]) {
        std::cout << "last i false\n";
        return false;
    }
    std::cout << "is less than:" << (downmost() < 19) << "\n";
    return downmost() < 19;
}

bool TetrisShape::can_lshape_left_to_up() {
    for (int i = 0; i < SQUARE_PER_SHAPE - 1; ++i) {
        if (cdnt[2].y - i < 0) {
            return false;
        }
        if (board_grid[cdnt[2].x][cdnt[2].y - i]) {
            return false;
        }
    }
    if ((cdnt[1].y - 2 < 0) || board_grid[cdnt[1].x][cdnt[1].y - 2]) {
        return false;
    }
    return leftmost() > 1;
}

bool TetrisShape::can_lshape_up_to_right() {
  for (int i = 0; i < SQUARE_PER_SHAPE - 1; ++i) {
      if ((cdnt[2].x - i < 0) ||
          board_grid[cdnt[2].x - i][cdnt[2].y]) {
          return false;
      }
  }
  if ( (cdnt[1].x - 2) < 0 ||
        board_grid[cdnt[1].x - 2][cdnt[1].y]) {
      return false;
  }
  return upmost() > 1;
}

bool TetrisShape::can_lshape_right_to_down() {
    for (int i = 0; i < SQUARE_PER_SHAPE - 1; ++i) {
        if ( (cdnt[2].y + i > 19) || 
              board_grid[cdnt[2].x][cdnt[2].y + i]) {
            return false;
        }
    }
    if ( (cdnt[1].y + 2 > 19) ||
          board_grid[cdnt[1].x][cdnt[1].y + 2]) {
        return false;
    }
    return rightmost() < 19;
}

bool TetrisShape::can_morph_lshape() {
    std::cout << "enter can_morph_lshape \n";
    switch (shsubtype) {
        case LSHAPE_DOWN:
            // shsubtype = LSHAPE_LEFT;
            return can_lshape_down_to_left();
            break;
        case LSHAPE_LEFT:
            return can_lshape_left_to_up();
            // shsubtype = LSHAPE_UP;
            break;
        case LSHAPE_UP:
            return can_lshape_up_to_right();
            // shsubtype = LSHAPE_RIGHT;
            break;
        case LSHAPE_RIGHT:
            return can_lshape_right_to_down();
            // shsubtype = LSHAPE_DOWN;
            break;
        default:
            break;
    }
    std::cout << "can_morph_lshape returns false";
    return false;
}

void TetrisShape::morph_stripshape() {
    switch (shsubtype) {
        case STRIPSHAPE_LANDSCAPE:
            cdnt[1].x = cdnt[0].x + 1;
            cdnt[1].y = cdnt[0].y;
            cdnt[2].x = cdnt[0].x + 2;
            cdnt[2].y = cdnt[0].y;
            cdnt[3].x = cdnt[0].x + 3;
            cdnt[3].y = cdnt[0].y;
            shsubtype = STRIPSHAPE_PORTRAIT;
            break;
        case STRIPSHAPE_PORTRAIT:
            cdnt[1].x = cdnt[0].x;
            cdnt[1].y = cdnt[0].y + 1;
            cdnt[2].x = cdnt[0].x;
            cdnt[2].y = cdnt[0].y + 2;
            cdnt[3].x = cdnt[0].x;
            cdnt[3].y = cdnt[0].y + 3;
            shsubtype = STRIPSHAPE_LANDSCAPE;
            break;
        default:
            break;
    }
}

void TetrisShape::morph_lshape() {
    switch (shsubtype) {
        case LSHAPE_DOWN:
            cdnt[0].x = cdnt[2].x;
            cdnt[0].y = cdnt[2].y;
            cdnt[1].x = cdnt[3].x;
            cdnt[1].y = cdnt[3].y;
            cdnt[2].x = cdnt[3].x + 1;
            // cdnt[2].y = cdnt[2].y;
            cdnt[3].x = cdnt[2].x;
            cdnt[3].y = cdnt[2].y - 1;
            shsubtype = LSHAPE_LEFT;
            break;
        case LSHAPE_LEFT:
            cdnt[0].x = cdnt[2].x;
            cdnt[0].y = cdnt[2].y;
            cdnt[1].x = cdnt[3].x;
            cdnt[1].y = cdnt[3].y;
            // cdnt[2].x = cdnt[2].x;
            cdnt[2].y = cdnt[3].y - 1;
            cdnt[3].x = cdnt[3].x - 1;
            cdnt[3].y = cdnt[2].y;
            shsubtype = LSHAPE_UP;
            break;
        case LSHAPE_UP:
            cdnt[0].x = cdnt[2].x;
            cdnt[0].y = cdnt[2].y;
            cdnt[1].x = cdnt[3].x;
            cdnt[1].y = cdnt[3].y;
            cdnt[2].x = cdnt[3].x - 1;
            // cdnt[2].y = cdnt[2].y;
            cdnt[3].x = cdnt[2].x;
            cdnt[3].y = cdnt[2].y + 1;
            shsubtype = LSHAPE_RIGHT;
            break;
        case LSHAPE_RIGHT:
            cdnt[0].x = cdnt[2].x;
            cdnt[0].y = cdnt[2].y;
            cdnt[1].x = cdnt[3].x;
            cdnt[1].y = cdnt[3].y;
            // cdnt[2].x = cdnt[2].x;
            cdnt[2].y = cdnt[3].y + 1;
            cdnt[3].x = cdnt[2].x + 1;
            cdnt[3].y = cdnt[2].y;
            shsubtype = LSHAPE_DOWN;
            break;
        default:
            break;
    }
}

bool TetrisShape::can_morph_gammashape() {
    switch (shsubtype) {
        case GAMMASHAPE_DOWN:
            // shsubtype = LSHAPE_LEFT;
            return can_gammashape_down_to_right();
            break;
        case GAMMASHAPE_RIGHT:
            return can_gammashape_right_to_up();
            // shsubtype = LSHAPE_UP;
            break;
        case GAMMASHAPE_UP:
            return can_gammashape_up_to_left();
            // shsubtype = LSHAPE_RIGHT;
            break;
        case GAMMASHAPE_LEFT:
            return can_gammashape_left_to_down();
            // shsubtype = LSHAPE_DOWN;
            break;
        default:
            break;
    }
    std::cout << "can_morph_gammashape returns false";
    return false;
}

bool TetrisShape::can_gammashape_down_to_right() {
    for (int i = 0; i < SQUARE_PER_SHAPE - 1; ++i) {
        if ( (cdnt[2].x + i > 19) ||
              board_grid[cdnt[2].x + i][cdnt[2].y]) {
            std::cout << "i is %d\n";
            return false;
        }
    }
    if ((cdnt[1].x + 2 > 19) ||
          board_grid[cdnt[1].x + 2][cdnt[2].y]) {
        std::cout << "last i false\n";
        return false;
    }
    std::cout << "is less than:" << (downmost() < 19) << "\n";
    return downmost() < 19;
}

bool TetrisShape::can_gammashape_right_to_up() {
    for (int i = 0; i < SQUARE_PER_SHAPE - 1; ++i) {
        if (cdnt[2].y + i > 19) {
            return false;
        }
        if (board_grid[cdnt[2].x][cdnt[2].y + i]) {
            return false;
        }
    }
    if ((cdnt[2].y + 2 > 19) || board_grid[cdnt[1].x][cdnt[2].y + 2]) {
        return false;
    }
    return rightmost() < 19;
}

bool TetrisShape::can_gammashape_up_to_left() {
    for (int i = 0; i < SQUARE_PER_SHAPE - 1; ++i) {
        if ((cdnt[2].x - i < 0) ||
            board_grid[cdnt[2].x - i][cdnt[2].y]) {
            return false;
        }
    }
    if ( (cdnt[1].x - 2) < 0 ||
            board_grid[cdnt[1].x - 2][cdnt[1].y]) {
        return false;
    }
    return upmost() > 1;
}

bool TetrisShape::can_gammashape_left_to_down() {
    for (int i = 0; i < SQUARE_PER_SHAPE - 1; ++i) {
        if ( (cdnt[2].y - i < 0) || 
            board_grid[cdnt[2].x][cdnt[2].y - i]) {
            return false;
        }
    }
    if ( (cdnt[1].y - 2 < 0) ||
        board_grid[cdnt[1].x][cdnt[1].y - 2]) {
        return false;
    }
    return leftmost() > 1;
}

void TetrisShape::morph_gammashape() {
    switch (shsubtype) {
        case GAMMASHAPE_DOWN:
            cdnt[0].x = cdnt[2].x;
            cdnt[0].y = cdnt[2].y;
            cdnt[1].x = cdnt[3].x;
            cdnt[1].y = cdnt[3].y;
            cdnt[2].x = cdnt[3].x + 1;
            // cdnt[2].y = cdnt[2].y;
            cdnt[3].x = cdnt[2].x;
            cdnt[3].y = cdnt[2].y + 1;
            shsubtype = GAMMASHAPE_RIGHT;
            break;
        case GAMMASHAPE_RIGHT:
            cdnt[0].x = cdnt[2].x;
            cdnt[0].y = cdnt[2].y;
            cdnt[1].x = cdnt[3].x;
            cdnt[1].y = cdnt[3].y;
            // cdnt[2].x = cdnt[2].x;
            cdnt[2].y = cdnt[3].y + 1;
            cdnt[3].x = cdnt[3].x - 1;
            cdnt[3].y = cdnt[2].y;
            shsubtype = GAMMASHAPE_UP;
            break;
        case GAMMASHAPE_UP:
            cdnt[0].x = cdnt[2].x;
            cdnt[0].y = cdnt[2].y;
            cdnt[1].x = cdnt[3].x;
            cdnt[1].y = cdnt[3].y;
            cdnt[2].x = cdnt[3].x - 1;
            // cdnt[2].y = cdnt[2].y;
            cdnt[3].x = cdnt[2].x;
            cdnt[3].y = cdnt[2].y - 1;
            shsubtype = GAMMASHAPE_LEFT;
            break;
        case GAMMASHAPE_LEFT:
            cdnt[0].x = cdnt[2].x;
            cdnt[0].y = cdnt[2].y;
            cdnt[1].x = cdnt[3].x;
            cdnt[1].y = cdnt[3].y;
            // cdnt[2].x = cdnt[2].x;
            cdnt[2].y = cdnt[3].y - 1;
            cdnt[3].x = cdnt[2].x + 1;
            cdnt[3].y = cdnt[2].y;
            shsubtype = GAMMASHAPE_DOWN;
            break;
        default:
            break;
    }
}

bool TetrisShape::can_morph_tshape() {
    switch (shsubtype) {
        case TSHAPE_DOWN:
            if ((cdnt[1].x - 1) < 0 ||
                 board_grid[cdnt[1].x - 1][cdnt[1].y]) {
                return false;
            }
            return upmost() > 0;
            break;
        case TSHAPE_LEFT:
            if ((cdnt[1].y + 1) > 19 ||
                 board_grid[cdnt[1].x][cdnt[1].y + 1]) {
                return false;
            }
            return rightmost() < 19;
            break;
        case TSHAPE_UP:
            if ((cdnt[1].x + 1) > 19 ||
                 board_grid[cdnt[1].x + 1][cdnt[1].y]) {
                return false;
            }
            return downmost() < 19;
            break;
        case TSHAPE_RIGHT:
            if ((cdnt[1].y - 1) < 0 ||
                 board_grid[cdnt[1].x][cdnt[1].y - 1]) {
                return false;
            }
            return leftmost() > 0;
            break;
        default:
            break;
    }

    return false;
}

void TetrisShape::morph_tshape() {
    switch (shsubtype) {
        case TSHAPE_DOWN:
            cdnt[0].x = cdnt[1].x - 1;
            cdnt[0].y = cdnt[1].y;
            cdnt[2].x = cdnt[1].x + 1;
            cdnt[2].y = cdnt[1].y;
            cdnt[3].x = cdnt[1].x;
            cdnt[3].y = cdnt[1].y - 1;
            shsubtype = TSHAPE_LEFT;
            break;
        case TSHAPE_LEFT:
            cdnt[0].x = cdnt[1].x;
            cdnt[0].y = cdnt[1].y - 1;
            cdnt[2].x = cdnt[1].x;
            cdnt[2].y = cdnt[1].y + 1;
            cdnt[3].x = cdnt[1].x - 1;
            cdnt[3].y = cdnt[1].y;
            shsubtype = TSHAPE_UP;
            break;
        case TSHAPE_UP:
            cdnt[0].x = cdnt[1].x + 1;
            cdnt[0].y = cdnt[1].y;
            cdnt[2].x = cdnt[1].x - 1;
            cdnt[2].y = cdnt[1].y;
            cdnt[3].x = cdnt[1].x;
            cdnt[3].y = cdnt[1].y + 1;
            shsubtype = TSHAPE_RIGHT;
            break;
        case TSHAPE_RIGHT:
            cdnt[0].x = cdnt[1].x;
            cdnt[0].y = cdnt[1].y - 1;
            cdnt[2].x = cdnt[1].x;
            cdnt[2].y = cdnt[1].y + 1;
            cdnt[3].x = cdnt[1].x + 1;
            cdnt[3].y = cdnt[1].y;
            shsubtype = TSHAPE_DOWN;
            break;
        default:
            break;
    }
}

bool TetrisShape::can_morph_leftnshape() {
    switch (shsubtype) {
        case LEFTNSHAPE_VERTICAL:
            if ( (cdnt[2].y + 1 > 19) ||
                  board_grid[cdnt[2].x + 1][cdnt[2].y - 1] ||
                  board_grid[cdnt[2].x][cdnt[2].y+1]) {
                return false;
            }
            return true;
            break;
        case LEFTNSHAPE_HORIZONTAL:
            if ((cdnt[2].x - 1 < 0) || (cdnt[2].y - 1 < 0) ||
                 board_grid[cdnt[2].x - 1][cdnt[2].y - 1] ||
                 board_grid[cdnt[2].x + 1][cdnt[2].y]) {
                return false;
            }
            return true;
            break;
        default:
            break;
    }
    return false;
}

void TetrisShape::morph_leftnshape() {
    switch (shsubtype) {
        case LEFTNSHAPE_VERTICAL:
            cdnt[0].x = cdnt[2].x + 1;
            cdnt[0].y = cdnt[2].y - 1;
            cdnt[1].x = cdnt[2].x + 1;
            cdnt[1].y = cdnt[2].y;
            cdnt[3].x = cdnt[2].x;
            cdnt[3].y = cdnt[2].y + 1;
            shsubtype = LEFTNSHAPE_HORIZONTAL;
            break;
        case LEFTNSHAPE_HORIZONTAL:
            cdnt[0].x = cdnt[2].x - 1;
            cdnt[0].y = cdnt[2].y - 1;
            cdnt[1].x = cdnt[2].x;
            cdnt[1].y = cdnt[2].y - 1;
            cdnt[3].x = cdnt[2].x + 1;
            cdnt[3].y = cdnt[2].y;
            shsubtype = LEFTNSHAPE_VERTICAL;
            break;
        default:
            break;
    }
}

bool TetrisShape::can_morph_rightnshape(){
    switch (shsubtype) {
        case RIGHTNSHAPE_VERTICAL:
            if ((cdnt[1].y + 1 > 19) ||
                 board_grid[cdnt[1].x + 1][cdnt[1].y] ||
                 board_grid[cdnt[1].x + 1][cdnt[1].y + 1]) {
                return false;
            }
            return true;
            break;
        case RIGHTNSHAPE_HORIZONTAL:
            if ((cdnt[1].x - 1 < 0) ||
                 board_grid[cdnt[1].x - 1][cdnt[1].y] ||
                 board_grid[cdnt[1].x + 1][cdnt[1].y - 1]) {
                return false;
            }
            return true;
            break;
        default:
            break;
    }
    return false;
}

void TetrisShape::morph_rightnshape() {
    switch (shsubtype) {
        case RIGHTNSHAPE_VERTICAL:
            cdnt[0].x = cdnt[1].x;
            cdnt[0].y = cdnt[1].y - 1;
            cdnt[2].x = cdnt[1].x + 1;
            cdnt[2].y = cdnt[1].y;
            cdnt[3].x = cdnt[1].x + 1;
            cdnt[3].y = cdnt[1].y + 1;
            shsubtype = RIGHTNSHAPE_HORIZONTAL;
            break;
        case RIGHTNSHAPE_HORIZONTAL:
            cdnt[0].x = cdnt[1].x - 1;
            cdnt[0].y = cdnt[1].y;
            cdnt[2].x = cdnt[1].x;
            cdnt[2].y = cdnt[1].y - 1;
            cdnt[3].x = cdnt[1].x + 1;
            cdnt[3].y = cdnt[1].y - 1;
            shsubtype = RIGHTNSHAPE_VERTICAL;
            break;
        default:
            break;
    }
}

bool TetrisShape::can_morph() {
    switch (stype) {
        case TETRIS_STRIPSHAPE:
            return can_morph_stripe();
            break;
        case TETRIS_LSHAPE:
            std::cout << "can morph:" << can_morph_lshape() << "\n";
            return can_morph_lshape();
            break;
        case TETRIS_GAMMASHAPE:
            return can_morph_gammashape();
            break;
        case TETRIS_SQUARESHAPE:
            return true;
            break;
        case TETRIS_TSHAPE:
            return can_morph_tshape();
            break;
        case TETRIS_LEFTNSHAPE:
            return can_morph_leftnshape();
            break;
        case TETRIS_RIGHTNSHAPE:
            return can_morph_rightnshape();
            break;
        default:
            return false;
            break;
    }

    std::cout << "return false in can morph\n";
    return false;
}

void TetrisShape::morph() {
    switch (stype) {
        case TETRIS_STRIPSHAPE:
            morph_stripshape();
            break;
        case TETRIS_LSHAPE:
            morph_lshape();
            break;
        case TETRIS_GAMMASHAPE:
            morph_gammashape();
            break;
        case TETRIS_TSHAPE:
            morph_tshape();
            break;
        case TETRIS_SQUARESHAPE:
            break;
        case TETRIS_LEFTNSHAPE:
            morph_leftnshape();
            break;
        case TETRIS_RIGHTNSHAPE:
            morph_rightnshape();
            break;
        default:
            break;
    }
}

void TetrisShape::move_to_bottom() {
    while (can_move_down()) {
        move_down();
    }
}
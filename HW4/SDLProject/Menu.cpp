/**
* Author: James Zhang
* Assignment: Rise of the AI
* Date due: 2025-04-05, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/
#include "Menu.h"
#include "Utility.h"
constexpr char FONT_FILEPATH[] = "assets/font.png";
GLuint font_texture_id;

void Menu::initialise()
{
    font_texture_id  = Utility::load_texture(FONT_FILEPATH);
}

void Menu::update(float delta_time){}


void Menu::render(ShaderProgram *g_shader_program)
{
    std::string text = "Press ENTER to start";
//    draw_text(ShaderProgram *program, GLuint font_texture_id, std::string text, float screen_size, float spacing, glm::vec3 position)
    Utility::draw_text(g_shader_program,
                       font_texture_id,
                       text,
                       0.4f,    // screen_size
                       0.02f,   // spacing
                       glm::vec3(-4.0f, 0.0f, 0.0f) // position
                       );
}



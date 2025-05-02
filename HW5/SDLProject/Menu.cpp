/**
* Author: James Zhang
* Assignment: Student's Choice
* Date due: May 2, 2:00pm
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
    /**
     BGM and SFX
     */
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    
    m_game_state.bgm = Mix_LoadMUS("assets/inevitable.mp3");
    Mix_PlayMusic(m_game_state.bgm, -1);
    
    Mix_VolumeMusic(MIX_MAX_VOLUME/5);
}

void Menu::update(float delta_time){}


void Menu::render(ShaderProgram *g_shader_program)
{
    std::string text = "Press ENTER to start";
    Utility::draw_text(g_shader_program,
                       font_texture_id,
                       text,
                       0.4f,    // screen_size
                       0.02f,   // spacing
                       glm::vec3(-4.0f, 0.0f, 0.0f) // position
                       );
}



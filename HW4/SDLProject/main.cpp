#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define LEVEL1_WIDTH 14
#define LEVEL1_HEIGHT 8
#define LEVEL1_LEFT_EDGE 5.0f
#define LEVEL1_HEIGHT_EDGE 4

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_mixer.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "cmath"
#include <ctime>
#include <vector>
#include "Entity.h"
#include "Map.h"
#include "Utility.h"
#include "Scene.h"
#include "LevelA.h"
#include "LevelB.h"
#include "LevelC.h"
#include "Menu.h"

// ————— CONSTANTS ————— //
constexpr int WINDOW_WIDTH  = 640,
          WINDOW_HEIGHT = 480;

constexpr float BG_RED     = 0.529f,
            BG_GREEN   = 0.808f,
            BG_BLUE    = 0.922f,
            BG_OPACITY = 1.0f;

constexpr int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH  = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;


constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
                F_SHADER_PATH[] = "shaders/fragment_textured.glsl",
                FONT_FILEPATH[] = "assets/font.png";
        

constexpr float MILLISECONDS_IN_SECOND = 1000.0;

enum AppStatus { RUNNING, TERMINATED };

bool GAME_OVER = false;

// ————— GLOBAL VARIABLES ————— //
Scene *g_current_scene;
Menu   *g_menu;
LevelA *g_level_a;
LevelB *g_level_b;
LevelC *g_level_c;
Scene* g_levels[4];

SDL_Window* g_display_window;
GLuint font_tex_id;
AppStatus g_app_status = RUNNING;
ShaderProgram g_shader_program;
glm::mat4 g_view_matrix, g_projection_matrix;

float g_previous_ticks = 0.0f;
float g_accumulator = 0.0f;
int g_lives;

void switch_to_scene(Scene *scene, Entity* shared_player = nullptr)
{
    g_current_scene = scene;

    if (shared_player != nullptr) {
        g_current_scene->set_player(shared_player);
    }

    g_current_scene->initialise();
}
void initialise();
void process_input();
void update();
void render();
void shutdown();


void initialise()
{
    // ————— VIDEO ————— //
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    g_display_window = SDL_CreateWindow("Don't Be Afraid!",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_OPENGL);
    
    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    
    SDL_GL_MakeCurrent(g_display_window, context);
    if (context == nullptr)
    {
        shutdown();
    }
    
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    // ————— GENERAL ————— //
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);
    
    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());
    
    glClearColor(BG_RED, BG_GREEN, BG_BLUE, BG_OPACITY);
    font_tex_id  = Utility::load_texture(FONT_FILEPATH);
    
    g_menu = new Menu();
    
    // ————— LEVEL A SETUP ————— //
    g_level_a = new LevelA();
    
    // ————— LEVEL B SETUP ————— //
    g_level_b = new LevelB();
    
    // ————— LEVEL C SETUP ————— //
    g_level_c = new LevelC();
    
    g_levels[0] = g_menu;
    g_levels[1] = g_level_a;
    g_levels[2] = g_level_b;
    g_levels[3] = g_level_c;
    
    switch_to_scene(g_menu);
    
    
    // ————— BLENDING ————— //
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    std::cout << "Initialization complete. Current scene: "
              << (g_current_scene == g_menu ? "Menu" : "Other")
              << std::endl;


}

void process_input()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                g_app_status = TERMINATED;
                return;

            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_q) {
                    g_app_status = TERMINATED;
                    return;
                }
                // for menu page
                if (g_current_scene == g_menu && event.key.keysym.sym == SDLK_RETURN) {
                    switch_to_scene(g_levels[1]);
                    g_lives = g_current_scene->get_state().player->get_lives();
                    return;
                }
                // for other levels
                if (g_current_scene->get_state().player != nullptr &&
                    event.key.keysym.sym == SDLK_SPACE &&
                    g_current_scene->get_state().player->get_collided_bottom()) {

                    g_current_scene->get_state().player->jump();
                    Mix_PlayChannel(-1, g_current_scene->get_state().jump_sfx, 0);
                }
                break;
        }
    }

    // if menu page, skip
    if (g_current_scene->get_state().player == nullptr) return;

    Entity* player = g_current_scene->get_state().player;
    player->set_movement(glm::vec3(0.0f));

    const Uint8 *key_state = SDL_GetKeyboardState(NULL);

    if (key_state[SDL_SCANCODE_LEFT] && player->get_lives() > 0)
        player->move_left();
    else if (key_state[SDL_SCANCODE_RIGHT] && player->get_lives() > 0)
        player->move_right();

    if (glm::length(player->get_movement()) > 1.0f)
        player->normalise_movement();
}


void update()
{
    if (g_current_scene == g_levels[0]) return;
    
    if (g_current_scene->get_state().player->get_lives()!= g_lives) Mix_PlayChannel(-1, g_current_scene->get_state().death_sfx, 0);
    g_lives = g_current_scene->get_state().player->get_lives();

    // ————— DELTA TIME / FIXED TIME STEP CALCULATION ————— //
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;
    
    delta_time += g_accumulator;
    
    if (delta_time < FIXED_TIMESTEP)
    {
        g_accumulator = delta_time;
        return;
    }
    
    while (delta_time >= FIXED_TIMESTEP) {
        // ————— UPDATING THE SCENE (i.e. map, character, enemies...) ————— //
        g_current_scene->update(FIXED_TIMESTEP);
        
        delta_time -= FIXED_TIMESTEP;
    }
    
    g_accumulator = delta_time;
    
    
    // ————— PLAYER CAMERA ————— //
    if (g_current_scene->get_state().player == nullptr) return;
    g_view_matrix = glm::mat4(1.0f);
    if (g_current_scene->get_state().player->get_position().x > LEVEL1_LEFT_EDGE &&
        g_current_scene->get_state().player->get_position().y > -3.75f) {
        g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-g_current_scene->get_state().player->get_position().x, -g_current_scene->get_state().player->get_position().y, 0));
    }
    else if (g_current_scene->get_state().player->get_position().x > LEVEL1_LEFT_EDGE) {
        g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-g_current_scene->get_state().player->get_position().x, 3.75, 0));
    }
    else if (g_current_scene->get_state().player->get_position().y > -3.75f) {
        g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-5, -g_current_scene->get_state().player->get_position().y, 0));
    }
    else {
        g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-5, 3.75, 0));
    }
}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT);

    // ————— RENDERING THE SCENE (i.e. map, character, enemies...) ————— //
    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);
    g_current_scene->render(&g_shader_program);
    
    
    // ————— RENDERING THE STATIC TEXTS ————— //
    glm::mat4 ui_view_matrix = glm::mat4(1.0f);
    glm::mat4 ui_projection_matrix = glm::ortho(0.0f, (float)WINDOW_WIDTH, 0.0f, (float)WINDOW_HEIGHT, -1.0f, 1.0f);
    
    g_shader_program.set_view_matrix(ui_view_matrix);
    g_shader_program.set_projection_matrix(ui_projection_matrix);
    
    if (g_current_scene != g_levels[0]) {
        std::string lives_text = "Lives Remaining: " + std::to_string(g_current_scene->get_state().player->get_lives());
        Utility::draw_text(&g_shader_program, font_tex_id, lives_text, 20.0f, 2.0f, glm::vec3(20.0f, WINDOW_HEIGHT - 40.0f, 0.0f));
        
        if (g_current_scene->get_state().player->get_lives()==0) {
            Utility::draw_text(&g_shader_program, font_tex_id, "YOU LOST!", 30.0f, 2.0f, glm::vec3(WINDOW_WIDTH/3, (float)WINDOW_HEIGHT/2+80.0f, 0.0f));
            g_current_scene->get_state().enemies->deactivate();
        }
        if (g_current_scene->get_state().player->get_active()==false) {
            Utility::draw_text(&g_shader_program, font_tex_id, "YOU WIN!", 30.0f, 2.0f, glm::vec3(WINDOW_WIDTH/3, (float)WINDOW_HEIGHT/2+80.0f, 0.0f));
        }
        if (g_current_scene->get_state().player->get_active()==false && GAME_OVER==false) {
            Mix_PlayChannel(-1, g_current_scene->get_state().win_sfx, 0);
            GAME_OVER = true;
        }
    }
    
    
    SDL_GL_SwapWindow(g_display_window);
}


void shutdown()
{    
    SDL_Quit();
    
    // ————— DELETING LEVEL A DATA (i.e. map, character, enemies...) ————— //
    delete g_level_a;
}

// ————— GAME LOOP ————— //
int main(int argc, char* argv[])
{
    initialise();
    
    while (g_app_status == RUNNING)
    {
        process_input();
        update();
        
        if (g_current_scene->get_state().next_scene_id > 0){
            Entity* shared_player = g_current_scene->get_state().player;
            int next_id = g_current_scene->get_state().next_scene_id;
            switch_to_scene(g_levels[next_id], shared_player);
            
        }
        
        render();
    }
    
    shutdown();
    return 0;
}

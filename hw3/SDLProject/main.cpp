#define LOG(argument) std::cout << argument << '\n'
#define STB_IMAGE_IMPLEMENTATION
#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1

#ifdef _WINDOWS
    #include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "Entity.h"
#include <vector>
#include <ctime>
#include "cmath"

// ————— CONSTANTS ————— //
constexpr int WINDOW_WIDTH  = 960,
              WINDOW_HEIGHT = 680;

constexpr float BG_RED     = 0.1765625f,
                BG_GREEN   = 0.17265625f,
                BG_BLUE    = 0.1609375f,
                BG_OPACITY = 1.0f;

constexpr int VIEWPORT_X = 0,
              VIEWPORT_Y = 0,
              VIEWPORT_WIDTH  = WINDOW_WIDTH,
              VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
               F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

constexpr char  hill1_FILEPATH[]    = "hill1.png",
                hill2_FILEPATH[]    = "hill2.png",
                platform_FILEPATH[] = "platform.png",
                rokt_idle_FILEPATH[] = "cohete_off.png",
                rokt_fire_FILEPATH[] = "cohete_on_wf.png",
                FONT_FILEPATH[] = "font.png";

constexpr float MILLISECONDS_IN_SECOND = 1000.0;

constexpr GLint NUMBER_OF_TEXTURES = 1,
                LEVEL_OF_DETAIL    = 0,
                TEXTURE_BORDER     = 0;
bool GAME_STARTED = false;
bool GAME_OVER = false;
bool GAME_WIN = false;

// ————— STRUCTS AND ENUMS —————//
enum AppStatus  { RUNNING, TERMINATED };
enum FilterType { NEAREST, LINEAR     };

struct GameState {
    Entity* hill[4];
    Entity* platform[2];
    Entity* rocket;
    Entity* text[5];
};

// ————— VARIABLES ————— //
GameState g_game_state;

SDL_Window* g_display_window;
AppStatus g_app_status = RUNNING;

ShaderProgram g_shader_program;
glm::mat4 g_view_matrix, g_projection_matrix, g_terrain_matrix;

constexpr float FIXED_TIMESTEP = 1.0f / 60.0f;
float g_time_accumulator = 0.0f;

float g_previous_ticks = 0.0f;

void initialise();
void process_input();
void update();
void render();
void shutdown();

GLuint load_texture(const char* filepath);

// ———— GENERAL FUNCTIONS ———— //
GLuint load_texture(const char* filepath, FilterType filterType)
{
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components,
                                     STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }

    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER,
                 GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    filterType == NEAREST ? GL_NEAREST : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                    filterType == NEAREST ? GL_NEAREST : GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    stbi_image_free(image);

    return textureID;
}
GLuint load_texture(const char* filepath)
{
    // STEP 1: Loading the image file
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }

    // STEP 2: Generating and binding a texture ID to our image
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    // STEP 3: Setting our texture filter parameters

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // STEP 4: Releasing our file from memory and returning our texture id
    stbi_image_free(image);

    return textureID;
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("Lunar Lander",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

    if (g_display_window == nullptr)
    {
        std::cerr << "Error: SDL window could not be created.\n";
        shutdown();
    }

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_view_matrix       = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    // Load texture
    GLuint hill1_tex_id = load_texture(hill1_FILEPATH);
    GLuint hill2_tex_id = load_texture(hill2_FILEPATH);
    GLuint platform_tex_id = load_texture(platform_FILEPATH);
    std::vector<GLuint> rokt_textures_ids = {
        load_texture(rokt_idle_FILEPATH),
        load_texture(rokt_fire_FILEPATH)
    };
    GLuint font_tex_id  = load_texture(FONT_FILEPATH);

// Initialize rocket
    g_game_state.rocket = new Entity(
         rokt_textures_ids,           // texture id vector
         glm::vec3(-3.0f, 2.5f, 0.0f) // position
    );
    
//  Initialize objects
    g_game_state.hill[0] = new Entity(hill1_tex_id, glm::vec3(-4.0f, -2.75f,0.0f), glm::vec3(2.0f, 2.0f, 0.0f));
    g_game_state.hill[1] = new Entity(hill2_tex_id, glm::vec3(0.0f, -2.75f,0.0f), glm::vec3(2.0f, 2.0f, 0.0f));
    g_game_state.hill[2] = new Entity(hill1_tex_id, glm::vec3(4.0f, -2.75f,0.0f), glm::vec3(3.0f, 2.0f, 0.0f));
    g_game_state.hill[3] = new Entity(hill1_tex_id, glm::vec3(-2.0f, -2.75f,0.0f), glm::vec3(3.0f, 4.0f, 0.0f));
    g_game_state.platform[0] = new Entity(platform_tex_id, glm::vec3(1.7f, -3.4f, 0.0f),glm::vec3(1.0f, 1.0f, 0.0f));
    g_game_state.platform[1] = new Entity(platform_tex_id, glm::vec3(-4.2f, -0.3f, 0.0f),glm::vec3(1.0f, 1.0f, 0.0f));
    
    g_game_state.text[0] = new Entity(font_tex_id, glm::vec3(-3.2f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f), "Press SPACE to start");
    g_game_state.text[1] = new Entity(font_tex_id, glm::vec3(-2.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f), "Mission Failed");
    g_game_state.text[2] = new Entity(font_tex_id, glm::vec3(-2.5f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f), "Mission Accomplished");
    g_game_state.text[3] = new Entity(font_tex_id, glm::vec3(3.3f, 3.5f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f), "Fuel:");
    g_game_state.text[4] = new Entity(font_tex_id, glm::vec3(4.0f, 3.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f), std::to_string(g_game_state.rocket->get_fuel()));
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    if (GAME_STARTED &&  !GAME_OVER && !GAME_WIN) {
        g_game_state.rocket->set_acceleration_y(-0.5f);
        g_game_state.rocket->set_acceleration_x(0.0f);
        g_game_state.rocket->set_status(IDLE);
    }
    
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                g_app_status = TERMINATED;
                break;

            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                    case SDLK_q: g_app_status = TERMINATED;
                    break;
                    case SDLK_SPACE:
                        GAME_STARTED = true; // start the game
                        g_game_state.rocket->set_acceleration_y(-0.5f);
                    break;
                    default:    break;
                }

            default:
                break;
        }
    }

    const Uint8 *key_state = SDL_GetKeyboardState(NULL);
    if(GAME_STARTED &&  !GAME_OVER && !GAME_WIN && g_game_state.rocket->get_fuel() > 0) {
        if (key_state[SDL_SCANCODE_W]) {
            g_game_state.rocket->set_acceleration_y(0.6f);
            g_game_state.rocket->set_status(IGNITE);
            g_game_state.rocket->set_fuel();
        }
        if (key_state[SDL_SCANCODE_A]) {
            g_game_state.rocket->set_acceleration_x(-0.4f);
            g_game_state.rocket->set_status(IGNITE);
            g_game_state.rocket->set_fuel();
        }
        if (key_state[SDL_SCANCODE_D]) {
            g_game_state.rocket->set_acceleration_x(0.4f);
            g_game_state.rocket->set_status(IGNITE);
            g_game_state.rocket->set_fuel();
        }
    }
    
}


void update()
{
// ————— DELTA TIME ————— //
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND; // get the current number of ticks
    float delta_time = ticks - g_previous_ticks; // the delta time is the difference from the last frame
    g_previous_ticks = ticks;
    
    for (int i=0; i<4; ++i) {
        g_game_state.hill[i]->update(delta_time);
    }
    g_game_state.platform[0]->update(delta_time);
    

    // ————— FIXED TIMESTEP ————— //
    delta_time += g_time_accumulator;

    if (delta_time < FIXED_TIMESTEP)
    {
        g_time_accumulator = delta_time;
        return;
    }

    while (delta_time >= FIXED_TIMESTEP)
    {
        g_game_state.rocket->update(FIXED_TIMESTEP);
        g_game_state.platform[1]->update_platform(FIXED_TIMESTEP);
        for (int i=0; i<4; ++i) {
            g_game_state.rocket->collision_detector(g_game_state.hill[i]);
        }
        g_game_state.rocket->collision_detector(g_game_state.platform[0]);
        g_game_state.rocket->collision_detector_with_moving(g_game_state.platform[1]);
        g_game_state.text[4]->set_text(std::to_string(g_game_state.rocket->get_fuel()));
        if (GAME_WIN || GAME_OVER) {
            g_game_state.rocket->set_status(IDLE);
            g_game_state.rocket->set_velocity_x(0);
            g_game_state.rocket->set_velocity_y(0);
            g_game_state.rocket->set_acceleration_x(0);
            g_game_state.rocket->set_acceleration_y(0);
        }
        delta_time -= FIXED_TIMESTEP;
    }

    g_time_accumulator = delta_time;
    
}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT);
 
    for (int i=0; i<4; ++i) {
        g_game_state.hill[i]->render(&g_shader_program);
    }
    g_game_state.platform[0]->render(&g_shader_program);
    g_game_state.platform[1]->render(&g_shader_program);
    g_game_state.rocket->render(&g_shader_program);
    g_game_state.text[3]->render(&g_shader_program);
    g_game_state.text[4]->render(&g_shader_program);
    if (!GAME_STARTED) {
        g_game_state.text[0]->render(&g_shader_program);
    }
    if (GAME_OVER) {
        g_game_state.text[1]->render(&g_shader_program);
    }
    else if (GAME_WIN) {
        g_game_state.text[2]->render(&g_shader_program);
    }
    
    SDL_GL_SwapWindow(g_display_window);
    
}


void shutdown()
{
    SDL_Quit();
    delete   g_game_state.platform[0];
}


int main(int argc, char* argv[])
{
    initialise();

    while (g_app_status == RUNNING)
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}

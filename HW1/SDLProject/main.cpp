/**
* Author: James Zhang   hz2852@nyu.edu
* Assignment: Simple 2D Scene
* Date due: 2025-02-15, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/
#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define LOG(argument) std::cout << argument << '\n'
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"                // 4x4 Matrix
#include "glm/gtc/matrix_transform.hpp"  // Matrix transformation methods
#include "ShaderProgram.h"
#include "stb_image.h"

enum AppStatus { RUNNING, TERMINATED };

// Our window dimensions
constexpr int WINDOW_WIDTH  = 1280,
              WINDOW_HEIGHT = 960;

// Background color components
constexpr float BG_RED     = 0.255f,
                BG_BLUE    = 0.231f,
                BG_GREEN   = 0.248f,
                BG_OPACITY = 1.0f;

// Our viewport—or our "camera"'s—position and dimensions
constexpr int VIEWPORT_X      = 0,
              VIEWPORT_Y      = 0,
              VIEWPORT_WIDTH  = WINDOW_WIDTH,
              VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
               F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

constexpr GLint NUMBER_OF_TEXTURES = 1,
                LEVEL_OF_DETAIL    = 0,
                TEXTURE_BORDER     = 0;

//constexpr char RED_FILEPATH[]    = "/Users/tutu/Desktop/CS3113/HW1/SDLProject/red.png",
//                BLUE_FILEPATH[] = "/Users/tutu/Desktop/CS3113/HW1/SDLProject/blue.png";
constexpr char RED_FILEPATH[]    = "red.png",
                BLUE_FILEPATH[] = "blue.png";

// Parameters for scaling
constexpr float GROWTH_FACTOR = 1.2f;  // growth rate of 1.0% per frame
constexpr float SHRINK_FACTOR = 0.8f;  // growth rate of -1.0% per frame
constexpr int MAX_FRAME = 30;
int g_frame_counter = 0;
bool g_is_growing = true;

constexpr float ROT_INCREMENT = 2.5f;

AppStatus g_app_status = RUNNING;
SDL_Window* g_display_window;

ShaderProgram g_shader_program;

glm::mat4 g_view_matrix,
          g_red_matrix,
          g_blue_matrix,
          g_red_center_matrix,
          g_blue_center_matrix,
          g_projection_matrix;


glm::vec3 g_rotation_red    = glm::vec3(0.0f, 0.0f, 0.0f),
          g_rotation_blue = glm::vec3(0.0f, 0.0f, 0.0f),
          g_translation_red  = glm::vec3(0.0f, 0.0f, 0.0f),
          g_translation_blue  = glm::vec3(0.0f, 0.0f, 0.0f);

GLuint g_red_texture_id,
       g_blue_texture_id;

float g_previous_ticks = 0.0f;

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

    // STEP 4: Releasing our file from memory and returning our texture id
    stbi_image_free(image);

    return textureID;
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("CS3113_Project 1_James Zhang",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_OPENGL);
    
    if (g_display_window == nullptr)
    {
        std::cerr << "ERROR: SDL Window could not be created.\n";
        g_app_status = TERMINATED;
        
        SDL_Quit();
        exit(1);
    }
    
    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    // Initialise camera
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    // Load up shaders
    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);
    
    // Initialise our view, model, and projection matrices
    g_view_matrix       = glm::mat4(1.0f);
    g_red_matrix      = glm::mat4(1.0f);
    g_blue_matrix     = glm::mat4(1.0f);
    g_red_center_matrix = glm::mat4(1.0f);
    g_red_center_matrix = glm::translate(g_red_center_matrix, glm::vec3(2.0f, 0.0f, 0.0f));
    g_blue_center_matrix = glm::mat4(1.0f);
    g_blue_center_matrix = glm::translate(g_blue_center_matrix, glm::vec3(-2.0f, 0.0f, 0.0f));
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);
    
    glUseProgram(g_shader_program.get_program_id());
    
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    g_red_texture_id    = load_texture(RED_FILEPATH);
    g_blue_texture_id = load_texture(BLUE_FILEPATH);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE)
        {
            g_app_status = TERMINATED;
        }
    }
}

void update() {
    
    float ticks = (float) SDL_GetTicks() / 1000.0;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;
    
    //for scaling
    glm::vec3 scale_vector;
    g_frame_counter += 1;
    
    if (g_frame_counter >= MAX_FRAME)
    {
        g_is_growing = !g_is_growing;
        g_frame_counter = 0;
    }
    
    scale_vector = glm::vec3(g_is_growing ? GROWTH_FACTOR : SHRINK_FACTOR,
                             g_is_growing ? GROWTH_FACTOR : SHRINK_FACTOR,
                             1.0f);

        /* Game logic */
    g_translation_red.x += 1.0f * delta_time;
    g_translation_red.y += 1.0f * delta_time;
    g_translation_blue.x += 1.0f * delta_time;
    g_translation_blue.y += 1.0f * delta_time;
    g_rotation_red.z += ROT_INCREMENT * delta_time;
    g_rotation_blue.z += -1 * ROT_INCREMENT * delta_time; // rotate along the opposite direction
    
    // reset
    g_red_matrix    = glm::mat4(1.0f);
    g_red_matrix    = glm::translate(g_red_matrix, glm::vec3(-2.0f, 0.0f, 0.0f));
    g_blue_matrix    = glm::mat4(1.0f);
    g_blue_matrix    = glm::translate(g_blue_matrix, glm::vec3(2.0f, 0.0f, 0.0f));
    
    // Transformation
    g_red_matrix = glm::translate(g_red_matrix, glm::vec3(3*cos(g_translation_red.x), 3*sin(g_translation_red.y), 0.0f));
    g_red_matrix = glm::rotate(g_red_matrix, g_rotation_red.z, glm::vec3(0.0f, 0.0f, 1.0f));
    g_blue_matrix = glm::translate(g_blue_matrix, glm::vec3(-3*sin(g_translation_blue.x), 3*cos(g_translation_blue.y), 0.0f));
    g_blue_matrix = glm::rotate(g_blue_matrix, g_rotation_blue.z, glm::vec3(0.0f, 0.0f, 1.0f));
    g_blue_matrix = glm::scale(g_blue_matrix, scale_vector);
    g_red_matrix = glm::scale(g_red_matrix, scale_vector);
}

void draw_object(glm::mat4 &object_g_model_matrix, GLuint &object_texture_id)
{
    g_shader_program.set_model_matrix(object_g_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6); // we are now drawing 2 triangles, so use 6, not 3
}


void render() {
    glClear(GL_COLOR_BUFFER_BIT);
    
    float vertices[] = {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  // triangle 1
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f   // triangle 2
    };

    float texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     // triangle 1
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
    };
    
    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());
    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT,
                          false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    
    draw_object(g_red_matrix, g_red_texture_id);
    draw_object(g_blue_matrix, g_blue_texture_id);
    draw_object(g_red_center_matrix, g_red_texture_id);
    draw_object(g_blue_center_matrix, g_blue_texture_id);
    
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    
    SDL_GL_SwapWindow(g_display_window);
}

void shutdown() { SDL_Quit(); }

/**
 Start here—we can see the general structure of a game loop without worrying too much about the details yet.
 */
int main(int argc, char* argv[])
{
    // Initialise our program—whatever that means
    initialise();
    
    while (g_app_status == RUNNING)
    {
        process_input();  // If the player did anything—press a button, move the joystick—process it
        update();         // Using the game's previous state, and whatever new input we have, update the game's state
        render();         // Once updated, render those changes onto the screen
    }
    
    shutdown();  // The game is over, so let's perform any shutdown protocols
    return 0;
}

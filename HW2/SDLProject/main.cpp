/**
* Author: James Zhang   hz2852@nyu.edu
* Assignment: Pong Clone
* Date due: 2025-03-01, 11:59pm
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
#include <random>

enum AppStatus { RUNNING, TERMINATED };

// Our window dimensions
constexpr int WINDOW_WIDTH  = 960,
              WINDOW_HEIGHT = 680;

// Background color components
constexpr float BG_RED     = 0.0f,
                BG_BLUE    = 0.105f,
                BG_GREEN   = 0.148f,
                BG_OPACITY = 1.0f;

// Our viewport—or our "camera"'s—position and dimensions
constexpr int VIEWPORT_X      = 0,
              VIEWPORT_Y      = 0,
              VIEWPORT_WIDTH  = WINDOW_WIDTH,
              VIEWPORT_HEIGHT = WINDOW_HEIGHT,
              FONTBANK_SIZE = 16;

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
               F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

constexpr GLint NUMBER_OF_TEXTURES = 1,
                LEVEL_OF_DETAIL    = 0,
                TEXTURE_BORDER     = 0;

constexpr char  bubble_FILEPATH[]    = "bubble.png",
                octoL_FILEPATH[] = "octoL.png",
                octoR_FILEPATH[] = "octoR.png",
                wall_FILEPATH[] = "wall.png",
                paddleL_FILEPATH[] = "octoL.png",
                paddleR_FILEPATH[] = "octoR.png",
                font_FILEPATH[] = "font.png";

AppStatus g_app_status = RUNNING;
bool g_game_started = false;
bool g_single_mode = false;
bool two_ball = false;
bool three_ball = false;
bool game_over = false;
char g_winner;
float g_left_paddle_direction = 1.0f;
SDL_Window* g_display_window;

ShaderProgram g_shader_program;

// global matrix for all objects
glm::mat4 g_view_matrix,
          g_upper_wall_matrix,
          g_lower_wall_matrix,
          g_projection_matrix,
          g_left_paddle_matrix,
          g_right_paddle_matrix,
          g_ball_matrix,
          g_ball2_matrix,
          g_ball3_matrix;

glm::vec3 g_paddleL_movement = glm::vec3(0.0f, 0.0f, 0.0f),
          g_paddleL_init_position = glm::vec3(-4.2f, 0.0f, 0.0f),
          g_paddleL_position = glm::vec3(0.0f, 0.0f, 0.0f),
          g_paddleR_init_position = glm::vec3(4.2f, 0.0f, 0.0f),
          g_paddleR_position = glm::vec3(0.0f, 0.0f, 0.0f),
          g_paddleR_movement = glm::vec3(0.0f, 0.0f, 0.0f),
          g_paddle_scale = glm::vec3(1.5f, 1.5f, 0.0f),
          g_ball_movement    = glm::vec3(0.0f, 0.0f, 0.0f),
          g_ball_position    = glm::vec3(0.0f, 0.0f, 0.0f),
          g_ball2_movement   = glm::vec3(0.0f, 0.0f, 0.0f),
          g_ball2_position   = glm::vec3(0.0f, 0.0f, 0.0f),
          g_ball3_movement   = glm::vec3(0.0f, 0.0f, 0.0f),
          g_ball3_position   = glm::vec3(0.0f, 0.0f, 0.0f),
          g_ball_scale       = glm::vec3(0.5f,0.5f,0.0f),
          g_wall_scale       = glm::vec3(1.0f, 1.0f, 0.0f),
          g_upper_wall_position = glm::vec3(0.0f, 3.5f, 0.0f),
          g_lower_wall_position = glm::vec3(0.0f, -3.5f, 0.0f);
          
        

GLuint g_wall_texture_id,
       g_blue_texture_id,
       g_paddleL_texture_id,
       g_paddleR_texture_id,
       g_font_texture_id,
       g_ball_texture_id;

constexpr float PADDLE_SPEED = 3.0f; // speed of paddles
constexpr float BALL_SPEED = 1.5f; // speed of the ball
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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // STEP 4: Releasing our file from memory and returning our texture id
    stbi_image_free(image);

    return textureID;
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("Pong by James Zhang - Project 2",
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
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    g_upper_wall_matrix      = glm::mat4(1.0f);
    g_upper_wall_matrix     = glm::translate(g_upper_wall_matrix, g_upper_wall_position);
    g_upper_wall_matrix     = glm::scale(g_upper_wall_matrix, g_wall_scale);
    
    g_lower_wall_matrix      = glm::mat4(1.0f);
    g_lower_wall_matrix     = glm::translate(g_lower_wall_matrix, g_lower_wall_position);
    g_lower_wall_matrix     = glm::scale(g_lower_wall_matrix, g_wall_scale);

    g_left_paddle_matrix    = glm::mat4(1.0f);
    g_left_paddle_matrix    = glm::translate(g_left_paddle_matrix, g_paddleL_init_position);
    g_left_paddle_matrix    = glm::scale(g_left_paddle_matrix, g_paddle_scale);
    
    g_right_paddle_matrix    = glm::mat4(1.0f);
    g_right_paddle_matrix    = glm::translate(g_right_paddle_matrix, g_paddleR_init_position);
    g_right_paddle_matrix    = glm::scale(g_right_paddle_matrix, g_paddle_scale);
    
    g_ball_matrix           = glm::mat4(1.0f);
    g_ball_matrix           = glm::scale(g_ball_matrix, g_ball_scale);
    
    g_ball2_matrix           = glm::mat4(1.0f);
    g_ball2_matrix           = glm::scale(g_ball2_matrix, g_ball_scale);
    
    g_ball3_matrix           = glm::mat4(1.0f);
    g_ball3_matrix           = glm::scale(g_ball3_matrix, g_ball_scale);
    
    
    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);
    
    glUseProgram(g_shader_program.get_program_id());
    
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    g_wall_texture_id    = load_texture(wall_FILEPATH);
    g_paddleL_texture_id = load_texture(paddleL_FILEPATH);
    g_paddleR_texture_id = load_texture(paddleR_FILEPATH);
    g_ball_texture_id    = load_texture(bubble_FILEPATH);
    g_font_texture_id    = load_texture(font_FILEPATH);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void set_init_ball_move(glm::vec3& ball_movement) {
    float value_x;
    float value_y;
    std::random_device rd_num_generator;
    value_y = rd_num_generator() % 4;
    
    // randomly choose a start player (i.e. ball goes to left or right)
    if (rd_num_generator() % 101 < 50) {
        value_x = 3.5f;
    }
    else { value_x = -3.5f;}
    
    // randomly launch up or down
    if (rd_num_generator() % 101 < 50) {
        value_y = -value_y;
    }

    ball_movement = glm::vec3(value_x, value_y, 0.0f);
}

void process_input()
{
    // reset to prevent it keeping moving
    g_paddleL_movement.y = 0.0f;
    g_paddleR_movement.y = 0.0f;
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            // End game
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                g_app_status = TERMINATED;
                break;

            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                    case SDLK_q: g_app_status = TERMINATED; break;
                  
                    case SDLK_SPACE:
                        if (g_game_started == false) {
                            g_game_started = true;
                            set_init_ball_move(g_ball_movement);
                        }
                        break;
                        
                    case SDLK_t:
                        g_single_mode = !g_single_mode;
                        break;
                    case SDLK_1:
                        two_ball = false;
                        three_ball = false;
                        break;
                    case SDLK_2:
                        two_ball = true;
                        three_ball = false;
                        set_init_ball_move(g_ball2_movement);
                        break;
                    case SDLK_3:
                        two_ball = true;
                        three_ball = true;
                        set_init_ball_move(g_ball2_movement);
                        set_init_ball_move(g_ball3_movement);
                        break;
                }
            

            default:
                break;
        }
    }
    const Uint8 *key_state = SDL_GetKeyboardState(NULL);
    if (key_state[SDL_SCANCODE_W])
    {
        if (g_single_mode == 0) g_paddleL_movement.y = 1.00f;
    }
    if (key_state[SDL_SCANCODE_S])
    {
        if (g_single_mode == 0) g_paddleL_movement.y = -1.00f;
    }
    if (key_state[SDL_SCANCODE_UP])
    {
        g_paddleR_movement.y = 1.00f;
    }
    if (key_state[SDL_SCANCODE_DOWN])
    {
        g_paddleR_movement.y = -1.00f;
    }
}

void collision_detector(glm::vec3& ball_position, glm::vec3& ball_movement) {
    // --- COLLISION LOGIC --- //
    float x_distance_lp = fabs(g_paddleL_init_position.x+g_paddleL_position.x-ball_position.x) - ((g_paddle_scale.x + g_ball_scale.x) / 2.0f);
    float y_distance_lp = fabs(g_paddleL_init_position.y+g_paddleL_position.y-ball_position.y) - ((g_paddle_scale.y + g_ball_scale.y) / 2.0f);
    float x_distance_rp = fabs(g_paddleR_init_position.x+g_paddleR_position.x-ball_position.x) - ((g_paddle_scale.x + g_ball_scale.x) / 2.0f);
    float y_distance_rp = fabs(g_paddleR_init_position.y+g_paddleR_position.y-ball_position.y) - ((g_paddle_scale.y + g_ball_scale.y) / 2.0f);
    float x_distance_upper_wall = fabs(ball_position.x - g_upper_wall_position.x) - ((10.0f + g_ball_scale.x) / 2.0f);
    float y_distance_upper_wall = fabs(ball_position.y - g_upper_wall_position.y) - ((g_wall_scale.y + g_ball_scale.y) / 2.0f);
    float x_distance_lower_wall = fabs(ball_position.x - g_lower_wall_position.x) - ((10.0f + g_ball_scale.x) / 2.0f);
    float y_distance_lower_wall = fabs(ball_position.y - g_lower_wall_position.y) - ((g_wall_scale.y + g_ball_scale.y) / 2.0f);
    
    if (x_distance_lp <= 0 && y_distance_lp <= 0) {
        std::cout << "Collision detected!" << std::endl;
        ball_position.x += 0.2f;
        ball_movement.x = -ball_movement.x;
        std::random_device rd_num_generator;
        ball_movement.y = rd_num_generator() % 5;
        
    }
    
    if (x_distance_rp <= 0 && y_distance_rp <= 0) {
        std::cout << "Collision detected!" << std::endl;
        ball_position.x -= 0.2f;
        ball_movement.x = -ball_movement.x;
        std::random_device rd_num_generator;
        ball_movement.y = rd_num_generator() % 5;
    }
    if (x_distance_upper_wall <= 0 && y_distance_upper_wall <= 0)  {
        std::cout << "Wall Collision detected!" << std::endl;
        ball_movement.y = -ball_movement.y;
        ball_position.y -= 0.2f;
    }
    if (x_distance_lower_wall <= 0 && y_distance_lower_wall <= 0) {
        std::cout << "Wall Collision detected!" << std::endl;
        ball_movement.y = -ball_movement.y;
        ball_position.y += 0.2f;
    }
    if (ball_position.x < -5.3f && game_over == false ) {
        g_winner = 'R';
        game_over = true;
    }
    
    else if (ball_position.x > 5.3f && game_over == false) {
        g_winner = 'L';
        game_over = true;
    }
}

void update() {
    // Delta Time
    float ticks = (float) SDL_GetTicks() / 1000;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;
    
    if (g_single_mode) {
        g_paddleL_position.y += g_left_paddle_direction * delta_time * PADDLE_SPEED;
        if (g_paddleL_position.y >= 2.4f) {
            g_paddleL_position.y -= 0.2f;
            g_left_paddle_direction = -g_left_paddle_direction;
            
        }
        else if (g_paddleL_position.y <= -2.4f) {
            g_paddleL_position.y += 0.2f;
            g_left_paddle_direction = -g_left_paddle_direction;
        }
    }
    else {
        // Update the new positions for paddles
        // limit the paddles so they move between the walls
        if (g_paddleL_position.y + g_paddleL_movement.y * delta_time * PADDLE_SPEED  <= 2.4f &&
            g_paddleL_position.y + g_paddleL_movement.y * delta_time * PADDLE_SPEED  >= -2.4f ) {
            g_paddleL_position += g_paddleL_movement * delta_time * PADDLE_SPEED;}
    }
    if (g_paddleR_position.y + g_paddleR_movement.y * delta_time * PADDLE_SPEED  <= 2.4f &&
        g_paddleR_position.y + g_paddleR_movement.y * delta_time * PADDLE_SPEED  >= -2.4f ) {
        g_paddleR_position += g_paddleR_movement * delta_time * PADDLE_SPEED;}
    
    // ball moving logic
    g_ball_position += g_ball_movement * delta_time * BALL_SPEED;
    
    // Translation
    g_left_paddle_matrix    = glm::mat4(1.0f);
    g_left_paddle_matrix    = glm::translate(g_left_paddle_matrix, g_paddleL_init_position);
    g_left_paddle_matrix    = glm::translate(g_left_paddle_matrix, g_paddleL_position);
    g_left_paddle_matrix    = glm::scale(g_left_paddle_matrix, g_paddle_scale);
    
    g_right_paddle_matrix    = glm::mat4(1.0f);
    g_right_paddle_matrix    = glm::translate(g_right_paddle_matrix, g_paddleR_init_position);
    g_right_paddle_matrix    = glm::translate(g_right_paddle_matrix, g_paddleR_position);
    g_right_paddle_matrix    = glm::scale(g_right_paddle_matrix, g_paddle_scale);
    
    g_ball_matrix            = glm::mat4(1.0f);
    g_ball_matrix            = glm::translate(g_ball_matrix, g_ball_position);
    g_ball_matrix           = glm::scale(g_ball_matrix, g_ball_scale);
    
    // --- COLLISION LOGIC --- //
    collision_detector(g_ball_position, g_ball_movement);
    
    if (two_ball) { // For the second ball
        g_ball2_position += g_ball2_movement * delta_time * BALL_SPEED;
        g_ball2_matrix = glm::mat4(1.0f);
        g_ball2_matrix = glm::translate(g_ball2_matrix, g_ball2_position);
        g_ball2_matrix = glm::scale(g_ball2_matrix, g_ball_scale);
        collision_detector(g_ball2_position, g_ball2_movement);
    }
    if (three_ball) { // For the three ball
        g_ball2_position += g_ball2_movement * delta_time * BALL_SPEED;
        g_ball2_matrix = glm::mat4(1.0f);
        g_ball2_matrix = glm::translate(g_ball2_matrix, g_ball2_position);
        g_ball2_matrix = glm::scale(g_ball2_matrix, g_ball_scale);
        collision_detector(g_ball2_position, g_ball2_movement);
        
        g_ball3_position += g_ball3_movement * delta_time * BALL_SPEED;
        g_ball3_matrix = glm::mat4(1.0f);
        g_ball3_matrix = glm::translate(g_ball3_matrix, g_ball3_position);
        g_ball3_matrix = glm::scale(g_ball3_matrix, g_ball_scale);
        collision_detector(g_ball3_position, g_ball3_movement);
    }
}

void draw_wall(glm::mat4 &object_g_model_matrix, GLuint &object_texture_id)
{
    g_shader_program.set_model_matrix(object_g_model_matrix);
    float vertices[] = {
        -5.0f, -0.5f, 5.0f, -0.5f, 5.0f, 0.5f,  // triangle 1
        -5.0f, -0.5f, 5.0f, 0.5f, -5.0f, 0.5f   // triangle 2
    };

    float texture_coordinates[] = {
        0.0,  1.0, 10.0,  1.0, 10.0, 0.0,
        0.0,  1.0, 10.0, 0.0,  0.0, 0.0
    };

    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());
    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT,
                          false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    
    g_shader_program.set_model_matrix(object_g_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6); // we are now drawing 2 triangles, so use 6, not 3
}
void draw_object(glm::mat4 &object_g_model_matrix, GLuint &object_texture_id)
{
    g_shader_program.set_model_matrix(object_g_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void draw_text(ShaderProgram *program, GLuint font_texture_id, std::string text,
               float font_size, float spacing, glm::vec3 position)
{
    // Scale the size of the fontbank in the UV-plane
    // We will use this for spacing and positioning
    float width = 1.0f / FONTBANK_SIZE;
    float height = 1.0f / FONTBANK_SIZE;

    // Instead of having a single pair of arrays, we'll have a series of pairs—one for
    // each character. Don't forget to include <vector>!
    std::vector<float> vertices;
    std::vector<float> texture_coordinates;

    // For every character...
    for (int i = 0; i < text.size(); i++) {
        // 1. Get their index in the spritesheet, as well as their offset (i.e. their
        //    position relative to the whole sentence)
        int spritesheet_index = (int) text[i];  // ascii value of character
        float offset = (font_size + spacing) * i;
        
        // 2. Using the spritesheet index, we can calculate our U- and V-coordinates
        float u_coordinate = (float) (spritesheet_index % FONTBANK_SIZE) / FONTBANK_SIZE;
        float v_coordinate = (float) (spritesheet_index / FONTBANK_SIZE) / FONTBANK_SIZE;

        // 3. Inset the current pair in both vectors
        vertices.insert(vertices.end(), {
            offset + (-0.5f * font_size), 0.5f * font_size,
            offset + (-0.5f * font_size), -0.5f * font_size,
            offset + (0.5f * font_size), 0.5f * font_size,
            offset + (0.5f * font_size), -0.5f * font_size,
            offset + (0.5f * font_size), 0.5f * font_size,
            offset + (-0.5f * font_size), -0.5f * font_size,
        });

        texture_coordinates.insert(texture_coordinates.end(), {
            u_coordinate, v_coordinate,
            u_coordinate, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate + width, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate, v_coordinate + height,
        });
    }

    // 4. And render all of them using the pairs
    glm::mat4 model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, position);
    
    program->set_model_matrix(model_matrix);
    glUseProgram(program->get_program_id());
    
    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0,
                          vertices.data());
    glEnableVertexAttribArray(program->get_position_attribute());
    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0,
                          texture_coordinates.data());
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());
    
    glBindTexture(GL_TEXTURE_2D, font_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, (int) (text.size() * 6));
    
    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);
    draw_wall(g_upper_wall_matrix, g_wall_texture_id);
    draw_wall(g_lower_wall_matrix, g_wall_texture_id);
    
    if (g_winner == 'L') {
        draw_text(&g_shader_program, g_font_texture_id, "The LEFT player WON!", 0.3f, 0.02f, glm::vec3(-3.2f, 0.0f, 0.0f));
    }
    else if (g_winner == 'R') {
        draw_text(&g_shader_program, g_font_texture_id, "The RIGHT player WON!", 0.3f, 0.02f, glm::vec3(-3.2f, 0.0f, 0.0f));
    }
    float vertices[] = {
        -0.5, -0.5, 0.5, -0.5, 0.5, 0.5,
        -0.5, -0.5, 0.5, 0.5, -0.5, 0.5
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
    
    draw_object(g_left_paddle_matrix, g_paddleL_texture_id);
    draw_object(g_right_paddle_matrix, g_paddleR_texture_id);
    draw_object(g_ball_matrix, g_ball_texture_id);
    
    if (g_game_started == false) {
        draw_text(&g_shader_program, g_font_texture_id, "Press SPACE to start", 0.3f, 0.02f, glm::vec3(-3.2f, 0.0f, 0.0f));
    }
    
    if (two_ball) {
        draw_object(g_ball2_matrix, g_ball_texture_id);
    }
    if (three_ball) {
        draw_object(g_ball2_matrix, g_ball_texture_id);
        draw_object(g_ball3_matrix, g_ball_texture_id);
    }
    
    
    
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

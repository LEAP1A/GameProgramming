/**
* Author: James Zhang
* Assignment: Rise of the AI
* Date due: 2025-04-05, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/
#include "LevelA.h"
#include "Utility.h"

#define LEVEL_WIDTH 17
#define LEVEL_HEIGHT 8

constexpr char SPRITESHEET_FILEPATH[] = "assets/hero.png",
           ENEMY_FILEPATH[]       = "assets/chick.png",
            target_FILEPATH[]     = "assets/target.png";


unsigned int LEVEL_DATA[] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 37, 38, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 37, 38, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 26, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 26, 0, 0, 0, 0, 0, 0, 0,
    26, 26, 26, 26, 26, 26, 0, 0, 0, 0, 4, 4, 4, 26, 26, 26, 26,
    4, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

LevelA::~LevelA()
{
    delete [] m_game_state.enemies;
    delete    m_game_state.player;
    delete    m_game_state.map;
    delete    m_game_state.target;
    Mix_FreeChunk(m_game_state.jump_sfx);
    Mix_FreeMusic(m_game_state.bgm);
}

void LevelA::initialise()
{
    
    
    GLuint map_texture_id = Utility::load_texture("assets/tile_sheet.png");
    m_game_state.map = new Map(LEVEL_WIDTH,   // width
                               LEVEL_HEIGHT, // height
                               LEVEL_DATA,  // level_data
                               map_texture_id,  // texture_id
                               1.0f,            // tile_size
                               8,               // tile_count_x
                               8);              // tile_count_y
    
    GLuint player_texture_id = Utility::load_texture(SPRITESHEET_FILEPATH);

    int player_walking_animation[4][4] =
    {
        { 8, 9, 10, 11 },  // to move to the left,
        { 12, 13, 14, 15 }, // to move to the right,
        { 0, 1, 2, 3 }, // to move upwards,
        { 4, 5, 6, 7 }   // to move downwards
    };

    glm::vec3 acceleration = glm::vec3(0.0f, -4.81f, 0.0f);

    m_game_state.player = new Entity(
        player_texture_id,         // texture id
        5.0f,                      // speed
        acceleration,              // acceleration
        4.5f,                      // jumping power
        player_walking_animation,  // animation index sets
        0.0f,                      // animation time
        4,                         // animation frame amount
        0,                         // current animation index
        8,                         // animation column amount
        3,                         // animation row amount
        1.0f,                      // width
        1.0f,                       // height
        PLAYER,
        3
    );
    
    m_game_state.player->set_position(glm::vec3(2.0f, 0.0f, 0.0f));

    /**
     Enemies' stuff */
    GLuint enemy_texture_id = Utility::load_texture(ENEMY_FILEPATH);
    int enemy_walking_animation[4][4] =
    {
        { 9, 10, 11, 9 },   // left (row 3)
        { 3, 4, 5, 3 },     // right (row 2)
        { 0, 1, 2, 0 },     // up (row 1)
        { 6, 7, 8, 6 }
    };

    m_game_state.enemies = new Entity[ENEMY_COUNT];

    for (int i = 0; i < ENEMY_COUNT; i++)
    {
        m_game_state.enemies[i] =  Entity(
                                          enemy_texture_id ,         // texture id
                                          1.5f,                      // speed
                                          acceleration,              // acceleration
                                          4.5f,                      // jumping power
                                          enemy_walking_animation,  // animation index sets
                                          0.0f,                      // animation time
                                          4,                         // animation frame amount
                                          0,                         // current animation index
                                          3,                         // animation column amount
                                          4,                         // animation row amount
                                          1.0f,                      // width
                                          1.0f,                       // height
                                          ENEMY,
                                          1
                                      );
    }
    
    m_game_state.enemies[0].set_ai_type(WALKER);
//    m_game_state.enemies[0].set_ai_state(IDLE);
    m_game_state.enemies[0].set_position(glm::vec3(12.0f, 0.0f, 0.0f));
    m_game_state.enemies[0].set_movement(glm::vec3(0.0f));
    m_game_state.enemies[0].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    
    m_game_state.enemies[0].set_right_bound(16.0f);
    m_game_state.enemies[0].set_left_bound(10.0f);

    /**
    Target
     */
    GLuint target_texture = Utility::load_texture(target_FILEPATH);
    m_game_state.target = new Entity(target_texture, 0.0f, 1.0f, 1.0f, PLATFORM);
    m_game_state.target -> set_position(glm::vec3(16.0f, -3.0f ,0.0f));

    /**
     BGM and SFX
     */
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    
    m_game_state.bgm = Mix_LoadMUS("assets/8bit.mp3");
    if (!m_game_state.bgm) {
        printf("Failed to load music! SDL_mixer Error: %s\n", Mix_GetError());
    }
    
    Mix_PlayMusic(m_game_state.bgm, -1);
    Mix_VolumeMusic(MIX_MAX_VOLUME);
    
    m_game_state.jump_sfx = Mix_LoadWAV("assets/jump.wav");
    m_game_state.death_sfx = Mix_LoadWAV("assets/death.wav");

}

void LevelA::update(float delta_time)
{
    
    
    m_game_state.player->update(delta_time, m_game_state.player, m_game_state.target, 1, m_game_state.map);
    
    for (int i = 0; i < ENEMY_COUNT; i++)
    {
        m_game_state.enemies[i].update(delta_time, m_game_state.player, NULL, 0, m_game_state.map);
    }
    
    m_game_state.target->update(delta_time, m_game_state.player, m_game_state.player, 1, m_game_state.map);
    
    if (m_game_state.player->get_collided_top_target())  m_game_state.next_scene_id = 2;
    
}


void LevelA::render(ShaderProgram *g_shader_program)
{
    m_game_state.map->render(g_shader_program);
    m_game_state.player->render(g_shader_program);
    for (int i = 0; i < ENEMY_COUNT; i++) m_game_state.enemies[i].render(g_shader_program);
    
    m_game_state.target->render(g_shader_program);

}



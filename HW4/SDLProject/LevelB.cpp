#include "LevelB.h"
#include "Utility.h"

#define LEVEL_WIDTH 21
#define LEVEL_HEIGHT 8

constexpr char ZOMBIE_FILEPATH[]       = "assets/zombie.png",
            tar_FILEPATH[]     = "assets/target.png";


unsigned int LVL_DATA[] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0,0,0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0,0,0,
    0, 26, 26, 0, 0, 0, 0, 0, 0, 0, 37, 38, 0, 4, 0, 0, 0, 0,0,0,0,
    0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 26, 26, 0, 0,0,0,0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 26, 26,0,0,0,
    0, 0, 0, 37, 38, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0,0,0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 26, 26, 26, 26, 0, 0,26,26,26,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 0,0,0
};

LevelB::~LevelB()
{
    delete [] m_game_state.enemies;
    delete    m_game_state.player;
    delete    m_game_state.map;
    delete    m_game_state.target;
    Mix_FreeChunk(m_game_state.jump_sfx);
    Mix_FreeMusic(m_game_state.bgm);
}

void LevelB::initialise()
{
    m_game_state.player->set_velocity(glm::vec3(0.0f));
    m_game_state.player->set_acceleration(glm::vec3(0.0f, -4.81f, 0.0f));
    m_game_state.player->set_movement(glm::vec3(0.0f));

    GLuint map_texture_id = Utility::load_texture("assets/tile_sheet.png");
    m_game_state.map = new Map(LEVEL_WIDTH,   // width
                               LEVEL_HEIGHT, // height
                               LVL_DATA,  // LVL_DATA
                               map_texture_id,  // texture_id
                               1.0f,            // tile_size
                               8,               // tile_count_x
                               8);              // tile_count_y
    
    m_game_state.player->set_position(glm::vec3(2.0f, 0.0f, 0.0f));
    

    glm::vec3 acceleration = glm::vec3(0.0f, -4.81f, 0.0f);
    
    

    /**
     Enemies' stuff */
    GLuint enemy_texture_id = Utility::load_texture(ZOMBIE_FILEPATH);
    int enemy_walking_animation[4][4] =
    {
        {0, 3, 6, 9},
        {0, 3, 6, 9},
        {0, 3, 6, 9},
        {0, 3, 6, 9}
    };
    
    ENEMY_COUNT = 2;
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
                                          10,                         // animation column amount
                                          1,                         // animation row amount
                                          1.0f,                      // width
                                          1.0f,                       // height
                                          ENEMY,
                                          1
                                      );
        m_game_state.enemies[i].set_ai_type(GUARD);
        m_game_state.enemies[i].set_ai_state(IDLE);
        m_game_state.enemies[i].set_position(glm::vec3(14.0f, 0.0f, 0.0f));
        m_game_state.enemies[i].set_movement(glm::vec3(0.0f));
        m_game_state.enemies[i].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    }
    

    m_game_state.enemies[0].set_position(glm::vec3(14.0f, 0.0f, 0.0f));
    m_game_state.enemies[1].set_position(glm::vec3(14.0f, -4.0f, 0.0f));

    /**
    Target
     */
    GLuint target_texture = Utility::load_texture(tar_FILEPATH);
    m_game_state.target = new Entity(target_texture, 0.0f, 1.0f, 1.0f, PLATFORM);
    m_game_state.target -> set_position(glm::vec3(16.0f, -1.0f ,0.0f));

    /**
     BGM and SFX
     */
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    
    m_game_state.bgm = Mix_LoadMUS("assets/8bit.mp3");
    if (!m_game_state.bgm) {
        printf("Failed to load music! SDL_mixer Error: %s\n", Mix_GetError());
    }
    
    Mix_PlayMusic(m_game_state.bgm, -1);
    Mix_VolumeMusic(MIX_MAX_VOLUME / 2);

    m_game_state.jump_sfx = Mix_LoadWAV("assets/jump.wav");
    m_game_state.death_sfx = Mix_LoadWAV("assets/death.wav");

}

void LevelB::update(float delta_time)
{

    m_game_state.player->update(delta_time, m_game_state.player, m_game_state.target, 1, m_game_state.map);
    
    for (int i = 0; i < ENEMY_COUNT; i++)
    {
        m_game_state.enemies[i].update(delta_time, m_game_state.player, NULL, 0, m_game_state.map);
    }
    m_game_state.target->update(delta_time, m_game_state.player, m_game_state.player, 1, m_game_state.map);
    if (m_game_state.player->get_collided_top_target()) m_game_state.next_scene_id = 3;
}


void LevelB::render(ShaderProgram *g_shader_program)
{
    m_game_state.map->render(g_shader_program);
    m_game_state.player->render(g_shader_program);
    for (int i = 0; i < ENEMY_COUNT; i++) m_game_state.enemies[i].render(g_shader_program);
    
    m_game_state.target->render(g_shader_program);
    
}

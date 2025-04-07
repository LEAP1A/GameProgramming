#include "LevelC.h"
#include "Utility.h"

#define LEVEL_WIDTH 21
#define LEVEL_HEIGHT 8

constexpr char ZOM_FILEPATH[]  = "assets/orc_piratess.png",
            BIRD_FILEPATH[]    = "assets/bat.png",
            BOX_FILEPATH[]     = "assets/target.png";


unsigned int MAP_DATA[] =
{
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0, 26, 26, 26, 26,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 26, 26,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  4,  4,  4,  4,  4,  4,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    26, 26, 26, 0,  0,  0,  0,  0,  0, 26,  0,  0, 26, 26, 26, 26, 26, 26, 26,  0,  0
};

LevelC::~LevelC()
{
    delete [] m_game_state.enemies;
    delete    m_game_state.player;
    delete    m_game_state.map;
    delete    m_game_state.target;
    Mix_FreeChunk(m_game_state.jump_sfx);
    Mix_FreeMusic(m_game_state.bgm);
}

void LevelC::initialise()
{
    m_game_state.player->set_velocity(glm::vec3(0.0f));
    m_game_state.player->set_acceleration(glm::vec3(0.0f, -4.81f, 0.0f));
    m_game_state.player->set_movement(glm::vec3(0.0f));

    GLuint map_texture_id = Utility::load_texture("assets/tile_sheet.png");
    m_game_state.map = new Map(LEVEL_WIDTH,   // width
                               LEVEL_HEIGHT, // height
                               MAP_DATA,  // MAP_DATA
                               map_texture_id,  // texture_id
                               1.0f,            // tile_size
                               8,               // tile_count_x
                               8);              // tile_count_y
    
    m_game_state.player->set_position(glm::vec3(2.0f, -5.0f, 0.0f));
    

    glm::vec3 acceleration = glm::vec3(0.0f, -4.81f, 0.0f);
    
    /**
     Enemies' stuff */
    GLuint enemy_texture_id = Utility::load_texture(ZOM_FILEPATH);
    GLuint bird_texture_id = Utility::load_texture(BIRD_FILEPATH);
    int enemy_walking_animation[4][4] =
    {
        {10, 12, 14, 16}, // left
        {28, 30, 32, 34}, // right
        {0, 3, 6, 9}, // up
        {0, 3, 6, 9}  // down
    };
    int bird_flying_animation[4][4] =
    {
        {12, 13, 14, 15}, // left
        {4, 5, 6, 7}, // right
        {0, 2, 4, 6}, // up
        {0, 2, 4, 6}  // down
    };
    
    ENEMY_COUNT = 3;
    m_game_state.enemies = new Entity[ENEMY_COUNT];

    for (int i = 0; i < 2; i++)
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
                                          9,                         // animation column amount
                                          4,                         // animation row amount
                                          1.0f,                      // width
                                          1.0f,                       // height
                                          ENEMY,
                                          1
                                      );
        m_game_state.enemies[i].set_ai_type(GUARD);
        m_game_state.enemies[i].set_ai_state(IDLE);
        m_game_state.enemies[i].set_movement(glm::vec3(0.0f));
        m_game_state.enemies[i].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    }
    

    m_game_state.enemies[0].set_position(glm::vec3(14.0f, 0.0f, 0.0f));
    m_game_state.enemies[1].set_position(glm::vec3(14.0f, -4.0f, 0.0f));
    
    m_game_state.enemies[2] =  Entity(
                                      bird_texture_id,         // texture id
                                      1.5f,                      // speed
                                      glm::vec3(0.0f, 0.0f, 0.0f),              // acceleration
                                      4.5f,                      // jumping power
                                      bird_flying_animation,  // animation index sets
                                      0.0f,                      // animation time
                                      4,                         // animation frame amount
                                      0,                         // current animation index
                                      4,                         // animation column amount
                                      4,                         // animation row amount
                                      1.0f,                      // width
                                      1.0f,                       // height
                                      ENEMY,
                                      1
                                  );
    m_game_state.enemies[2].set_ai_type(FLYER);
    m_game_state.enemies[2].face_right();
    m_game_state.enemies[2].set_position(glm::vec3(5.0f, -6.0f, 0.0f));
    m_game_state.enemies[2].set_movement(glm::vec3(0.0f));
    m_game_state.enemies[2].set_jumping_power(0.0f);

    m_game_state.enemies[2].set_right_bound(9.0f);
    m_game_state.enemies[2].set_left_bound(3.0f);


    /**
    Target
     */
    GLuint target_texture = Utility::load_texture(BOX_FILEPATH);
    m_game_state.target = new Entity(target_texture, 0.0f, 1.0f, 1.0f, PLATFORM);
    m_game_state.target -> set_position(glm::vec3(8.0f, 2.0f ,0.0f));

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
    m_game_state.win_sfx = Mix_LoadWAV("assets/win.wav");

}

void LevelC::update(float delta_time)
{
    
    m_game_state.player->update(delta_time, m_game_state.player, m_game_state.target, 1, m_game_state.map);
    
    for (int i = 0; i < ENEMY_COUNT; i++)
    {
        m_game_state.enemies[i].update(delta_time, m_game_state.player, NULL, 0, m_game_state.map);
    }
    m_game_state.target->update(delta_time, m_game_state.player, m_game_state.player, 1, m_game_state.map);
    if (m_game_state.player->get_collided_top_target()) m_game_state.player->deactivate();
    
}


void LevelC::render(ShaderProgram *g_shader_program)
{
    m_game_state.map->render(g_shader_program);
    m_game_state.player->render(g_shader_program);
    for (int i = 0; i < ENEMY_COUNT; i++) m_game_state.enemies[i].render(g_shader_program);
    
    m_game_state.target->render(g_shader_program);
    
}

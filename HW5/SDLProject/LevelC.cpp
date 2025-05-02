/**
* Author: James Zhang
* Assignment: Student's Choice
* Date due: May 2, 2:00pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/
#include "LevelC.h"
#include "Utility.h"

#define LEVEL_WIDTH 17
#define LEVEL_HEIGHT 8

constexpr char SPRITESHEET_FILEPATH[] = "assets/plane.png";
            

LevelC::~LevelC()
{
    delete    m_game_state.player;
    Mix_FreeChunk(m_game_state.fire_sfx);
    Mix_FreeMusic(m_game_state.bgm);
}

void LevelC::initialise()
{
    /**
     BGM and SFX
     */
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    
    m_game_state.fire_sfx = Mix_LoadWAV("assets/laser1.wav");
    m_game_state.boom_sfx = Mix_LoadWAV("assets/boom6.wav");
    m_game_state.win_sfx = Mix_LoadWAV("assets/win.wav");
    
    background_texture_id = Utility::load_texture("assets/starfield_alpha.png");
    background_matrix = glm::translate(background_matrix, glm::vec3(0.0f, 0.0f, 0.0f));
    background_matrix = glm::scale(background_matrix, glm::vec3(1.0f, 1.0f, 0.0f));
    
    GLuint player_texture_id = Utility::load_texture(SPRITESHEET_FILEPATH);
    GLuint bullet_texture_id = Utility::load_texture("assets/bullet.png");
    GLuint little_bullet_tex_id = Utility::load_texture("assets/enemy_bullet2.png");
    GLuint explosion_tex_id = Utility::load_texture("assets/explo.png");
    
    m_game_state.player = new Entity(player_texture_id, 1.0f, 1.0f, PLAYER);
    
    m_game_state.player->set_position(glm::vec3(0.0f, -3.0f, 0.0f));
    m_game_state.player->set_speed(4.0f);
    m_game_state.player->set_velocity(glm::vec3(0.0f, 2.0f, 0.0f));
    m_game_state.player->set_lives(3);
    
    // player's bullets
    int counter=0;
    for (int i= 0; i<32; ++i) {
        m_game_state.bullets[i] = new Entity(bullet_texture_id, counter%5, 4, 1, BULLET);
        m_game_state.bullets[i]->set_position(glm::vec3(7.0f, 0.0f, 0.0f)); // out of the screen waiting to be used
        m_game_state.bullets[i]->set_velocity(glm::vec3(0.0f, 0.0f, 0.0f));
        
        counter++;
    }
    
    
    /**
     Enemies' stuff */
    GLuint boss_texture_id = Utility::load_texture("assets/boss.png");
    for (int i=0;i<ENEMY_COUNT;++i) {
        m_game_state.enemies[i] = new Entity(boss_texture_id, 3.0f, 3.0f, ENEMY);
        m_game_state.enemies[i]->set_cooldown(1.0f);
        m_game_state.enemies[i]->set_velocity(glm::vec3(0.0f, 1.0f, 0.0f));
        m_game_state.enemies[i]->set_ai_type(BOSS);
        m_game_state.enemies[i]->deactivate();
        // enemies' bullet
        for (int j=0; j<ENEMY_BULLET_COUNT; ++j) {
            m_game_state.enemies[i]->bullets[j] = new Entity(little_bullet_tex_id, 1, 1, 1, BULLET_ROUND);
            m_game_state.enemies[i]->bullets[j]->set_width(0.1f);
            m_game_state.enemies[i]->bullets[j]->set_height(0.1f);
            m_game_state.enemies[i]->bullets[j]->set_position(glm::vec3(7.0f, 0.0f, 0.0f)); // out of the screen waiting to be used
            
        }
    }
    // Wave 1
    float wave1_loc = 7.0f;
    m_game_state.enemies[0]->set_position(glm::vec3(0.0f, wave1_loc, 0.0f));


    /**
    Explosion
     */
    int* explosion = new int[16];
    for (int f = 0; f < 16; ++f) explosion[f] = f;
    for (int i = 0; i < 10; ++i) {
        Entity* e = new Entity(explosion_tex_id, 0, 4, 4, EXP);
        e->set_animation_indice(explosion);
        e->deactivate();
        explosion_pool.push_back(e);
    }

    

}

void LevelC::update(float delta_time)
{
    printf("Player's location is %f\n", m_game_state.player->get_position().y);
    m_game_state.player->update(delta_time, m_game_state.player, NULL, 0);
    // update player's bullets
    for (int i=0; i<32; i++) {
        if (m_game_state.bullets[i]==nullptr || !m_game_state.bullets[i]->get_active()) continue;
        m_game_state.bullets[i]->update(delta_time, m_game_state.player, NULL, 0);
        // bullets collides with enemies
        for (int j = 0; j<ENEMY_COUNT; ++j) {
            if (!m_game_state.enemies[j]->get_active()) continue;
            if (m_game_state.bullets[i]->check_collision(m_game_state.enemies[j])) {
                spawnExplosion(m_game_state.enemies[j]->get_position());
                m_game_state.bullets[i]->deactivate();
                m_game_state.enemies[j]->enemy_hit();
                if (m_game_state.enemies[j]->get_health()<=0) {
                    m_game_state.enemies[j]->set_health(0);
                    spawnExplosion(m_game_state.enemies[j]->get_position());
                    m_game_state.enemies[j]->deactivate();
                    m_game_state.enemies[j]->is_dead();
                    m_game_state.enemies[j]->set_position(glm::vec3(-5.0f, 0.0f, 0.0f));
                    m_game_state.player->deactivate();
                    
                }
            }
        }
    }
    // update enemies
    for (int i=0;i<ENEMY_COUNT;++i) {
        if (!m_game_state.player->get_active()) break;
        m_game_state.enemies[i]->update(delta_time, m_game_state.player, NULL, 0);
        // if player collides with an enemy, they are both dead
        if(m_game_state.enemies[i]->check_collision(m_game_state.player)) {
            m_game_state.player->get_attacked();
            m_game_state.enemies[i]->deactivate();
            m_game_state.enemies[i]->is_dead();
            spawnExplosion(m_game_state.player->get_position());
            spawnExplosion(m_game_state.enemies[i]->get_position());
        }
        // update enemies' bullets
        for (int j=0;j<ENEMY_BULLET_COUNT;++j) {
            m_game_state.enemies[i]->bullets[j]->update(delta_time, m_game_state.player, NULL, 0);
            // if the player is killed by enemies' bullets
            if(m_game_state.enemies[i]->bullets[j]->check_collision(m_game_state.player)) {
                spawnExplosion(m_game_state.player->get_position());
                m_game_state.player->get_attacked();
            }
        }
    }
    // update explosions
    for (auto& e : explosion_pool) {
        if (e->get_active()) {
            e->update(delta_time, nullptr, nullptr, 0);
        }
    }
}


void LevelC::render(ShaderProgram *g_shader_program)
{
    Utility::draw_background(g_shader_program, background_matrix, background_texture_id);
    m_game_state.player->render(g_shader_program);
    for (int i=0; i<32; i++) {
        if (m_game_state.bullets[i]==nullptr) continue;
        m_game_state.bullets[i]->render(g_shader_program);
    }
    for (int i=0;i<ENEMY_COUNT;++i) {
        m_game_state.enemies[i]->render(g_shader_program);
        for (int j=0;j<ENEMY_BULLET_COUNT;++j) {
            m_game_state.enemies[i]->bullets[j]->render(g_shader_program);
        }
    }
    for (auto& e : explosion_pool) {
        if (e->get_active()) {
            e->render(g_shader_program);
        }
    }
}

void LevelC::spawnExplosion(const glm::vec3& pos) {
    Entity* e = explosion_pool[next_explosion];
    e->activate();
    e->set_position(pos);
    e->set_animation_index(0);
    e->set_animation_time(0.0f);
    next_explosion = (next_explosion + 1) % explosion_pool.size();
    Mix_PlayChannel(-1, m_game_state.boom_sfx, 0);
}



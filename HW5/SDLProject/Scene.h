#pragma once
#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "Utility.h"
#include "Entity.h"

/**
    Notice that the game's state is now part of the Scene class, not the main file.
*/
struct GameState
{
    // ————— GAME OBJECTS ————— //
    Entity *player;
    
    Entity *enemies[22];
    
    Entity* bullets[32] = {};
    
    // ————— AUDIO ————— //
    Mix_Music *bgm;
    Mix_Chunk *fire_sfx;
    Mix_Chunk *boom_sfx;
    Mix_Chunk *win_sfx;
    
    // ————— POINTERS TO OTHER SCENES ————— //
    int next_scene_id = 0;
};

class Scene {

    
    
public:
    GameState m_game_state;
    std::vector<Entity*> explosion_pool;
    int next_explosion =0;
    // ————— ATTRIBUTES ————— //
    int ENEMY_COUNT = 1;
    
    // ————— METHODS ————— //
    void spawnExplosion(const glm::vec3& pos);
    virtual void initialise() = 0;
    virtual void update(float delta_time) = 0;
    virtual void render(ShaderProgram *program) = 0;
    
    // ————— GETTERS ————— //
    GameState const get_state() const { return m_game_state;             }
    int const get_number_of_enemies() const { return ENEMY_COUNT; }

    void set_scene_id(int new_id) {
        m_game_state.next_scene_id = new_id;
    }
    void set_player(Entity* player) {
        m_game_state.player = player;
    }
};

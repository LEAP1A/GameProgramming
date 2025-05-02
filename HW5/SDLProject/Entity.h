#ifndef ENTITY_H
#define ENTITY_H

//#include <SDL_mixer.h>
#include "glm/glm.hpp"
#include "ShaderProgram.h"

enum EntityType { PLAYER, ENEMY, BULLET, BULLET_ROUND, EXP};
enum AIType     { ATTACKER, DEFENDER, BOSS     };
enum AIState    { WALKING, IDLE, ATTACKING };

//Mix_Chunk *death_sfx = Mix_LoadWAV("assets/death.wav");


class Entity
{
private:
    bool m_is_active = true;
    bool m_enemy_alive = true;
    int m_lives = 1;
    int m_speed;
    int m_attackPhase = 0;
    int m_boss_health = 500;
    
    //Enemy bullet
    int   m_nextBullet    = 0;      // next bullet index
    float m_fireCooldown  = 1.0f;   // cool down time in second
    float m_fireTimer     = 0.0f;
    
    EntityType m_entity_type;
    AIType     m_ai_type;
    AIState    m_ai_state;
    float ai_right_bound;
    float ai_left_bound;
    
    
    // ————— TRANSFORMATIONS ————— //
    glm::vec3 m_movement;
    glm::vec3 m_position;
    glm::vec3 m_scale;
    glm::vec3 m_velocity;
    glm::vec3 m_acceleration;

    glm::mat4 m_model_matrix;

    // ————— TEXTURES ————— //
    GLuint    m_texture_id;

    // ————— ANIMATION ————— //
    int m_animation_cols;
    int m_animation_frames,
        m_animation_index,
        m_animation_rows;

    int* m_animation_indices = nullptr;
    float m_animation_time = 0.0f;

    float m_width = 1.0f,
          m_height = 1.0f;
    // ————— COLLISIONS ————— //
    bool m_collided_top    = false;
    bool m_collided_top_target = false;
    bool m_collided_bottom = false;
    bool m_collided_bottom_target = false;
    bool m_collided_left   = false;
    bool m_collided_right  = false;

public:
    Entity* bullets[32]; // bullets for enemy
    // ————— STATIC VARIABLES ————— //
    static constexpr int SECONDS_PER_FRAME = 8;

    // ————— METHODS ————— //
    Entity();
    Entity(GLuint texture_id, int animation_index, int animation_cols, int animation_rows, EntityType EntityType);
    Entity(GLuint texture_id, float width, float height, EntityType EntityType); // Simpler constructor
    Entity(GLuint texture_id, float width, float height, EntityType EntityType, AIType AIType, AIState AIState); // AI constructor
    ~Entity();
    
    void move_left() {
        if (m_position.x <= -4.5f) return;
        m_movement.x = -1.0f;
    }
    void move_right() {
        if (m_position.x >= 4.5f) return;
        m_movement.x = 1.0f;
    }
    
    void draw_sprite_from_texture_atlas(ShaderProgram* program, GLuint texture_id, int index);
    void draw_bullets_from_texture_atlas(ShaderProgram* program, GLuint texture_id, int index);
    void draw_enemy(ShaderProgram* program, GLuint texture_id);
    bool const check_collision(Entity* other) const;
    
    void const check_collision_y(Entity* collidable_entities, int collidable_entity_count);
    void const check_collision_x(Entity* collidable_entities, int collidable_entity_count);
    
//    // Overloading our methods to check for only the map
//    void const check_collision_y(Map *map);
//    void const check_collision_x(Map *map);
    
    void update(float delta_time, Entity *player, Entity *collidable_entities, int collidable_entity_count);
    void render(ShaderProgram* program);

    void ai_activate(Entity *player);
    
    void ai_fireOneBullet();
    void ai_fireDoubleBullet();
    void ai_fireThreeBullet();
    void ai_fireWideFan();
    
    void ai_attacker(Entity *player);
    void ai_defender(Entity *player);
    void ai_boss(Entity *player);
    
    void get_attacked() {
        if (m_lives<1) return;
        m_lives -= 1;
        if (m_lives==0) deactivate();
        else {
            float cur_pos = m_position.y;
            m_position = glm::vec3(0.0f, cur_pos, 0.0f);
        }
        
        
    }
    
    void normalise_movement() { m_movement = glm::normalize(m_movement); }

    // ————— GETTERS ————— //
    EntityType const get_entity_type()    const { return m_entity_type;   };
    AIType     const get_ai_type()        const { return m_ai_type;       };
    AIState    const get_ai_state()       const { return m_ai_state;      };
    glm::vec3 const get_position()     const { return m_position; }
    glm::vec3 const get_velocity()     const { return m_velocity; }
    glm::vec3 const get_acceleration() const { return m_acceleration; }
    glm::vec3 const get_movement()     const { return m_movement; }
    glm::vec3 const get_scale()        const { return m_scale; }
    GLuint    const get_texture_id()   const { return m_texture_id; }
    
    bool      const get_collided_top() const { return m_collided_top; }
    bool      const get_collided_top_target() const { return m_collided_top_target; }
    bool      const get_collided_bottom() const { return m_collided_bottom; }
    bool      const get_collided_bottom_target() const { return m_collided_bottom_target; }
    bool      const get_collided_right() const { return m_collided_right; }
    bool      const get_collided_left() const { return m_collided_left; }
    int       const get_lives()         const { return m_lives;}
    bool      const get_active()        const { return m_is_active;}
    bool      const enemy_alive()         const {return m_enemy_alive;}
    int       const get_health()        const {return m_boss_health;}

    void activate()   { m_is_active = true;  };
    void deactivate() { m_is_active = false; };
    void is_dead() {m_enemy_alive = false;}
    // ————— SETTERS ————— //
    void const set_entity_type(EntityType new_entity_type)  { m_entity_type = new_entity_type;};
    void const set_ai_type(AIType new_ai_type){ m_ai_type = new_ai_type;};
    void const set_ai_state(AIState new_state){ m_ai_state = new_state;};
    void const set_position(glm::vec3 new_position) { m_position = new_position; }
    void const set_velocity(glm::vec3 new_velocity) { m_velocity = new_velocity; }
    void const set_acceleration(glm::vec3 new_acceleration) { m_acceleration = new_acceleration; }
    void const set_speed(float new_speed) { m_speed = new_speed;}
    void const set_movement(glm::vec3 new_movement) { m_movement = new_movement; }
    void const set_scale(glm::vec3 new_scale) { m_scale = new_scale; }
    void const set_texture_id(GLuint new_texture_id) { m_texture_id = new_texture_id; }
    void const set_animation_cols(int new_cols) { m_animation_cols = new_cols; }
    void const set_animation_rows(int new_rows) { m_animation_rows = new_rows; }
    void const set_animation_frames(int new_frames) { m_animation_frames = new_frames; }
    void const set_animation_index(int new_index) { m_animation_index = new_index; }
    void const set_animation_time(float new_time) { m_animation_time = new_time; }
    void const set_animation_indice(int* animation_indices) {m_animation_indices = animation_indices;}
    void const set_width(float new_width) {m_width = new_width; }
    void const set_height(float new_height) {m_height = new_height; }
    void const set_right_bound(float bound) {ai_right_bound = bound;}
    void const set_left_bound(float bound) {ai_left_bound = bound;}
    void const set_cooldown(float time) {m_fireCooldown = time;}
    void const set_lives(int new_lives) {m_lives = new_lives;}
    void const enemy_hit() {m_boss_health -= 2;}
    void const set_health(int n) {m_boss_health = n;}
    
};

#endif // ENTITY_H

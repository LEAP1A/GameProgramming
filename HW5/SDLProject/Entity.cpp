/**
* Author: James Zhang
* Assignment: Student's Choice
* Date due: May 2, 2:00pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/
#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "Entity.h"
#include <random>

void Entity::ai_activate(Entity *player)
{
    switch (m_ai_type)
    {
        case ATTACKER:
            ai_attacker(player);
            break;
            
        case DEFENDER:
            ai_defender(player);
            break;
        
        case BOSS:
            ai_boss(player);
        default:
            break;
    }
}

void Entity::ai_fireOneBullet()
{
    Entity* b = bullets[m_nextBullet];

    glm::vec3 spawnPos = m_position + glm::vec3(0.0f, -1.0f, 0.0f);
    b->set_position(spawnPos);


    glm::vec3 vel = glm::vec3(0, -1.0f, 0.0f);
    b->set_velocity(vel);

    m_nextBullet = (m_nextBullet + 1) % 16;
}
void Entity::ai_fireDoubleBullet()
{
    const float x_offset = 0.3f;

    for (int i = 0; i < 2; ++i) {
        Entity* b = bullets[m_nextBullet];

        float x_pos = (i == 0) ? -x_offset : x_offset;
        glm::vec3 spawnPos = m_position + glm::vec3(x_pos, -1.0f, 0.0f);
        glm::vec3 vel = glm::vec3(0.0f, -1.0f, 0.0f);

        b->set_position(spawnPos);
        b->set_velocity(vel);

        m_nextBullet = (m_nextBullet + 1) % 32;
    }
}

void Entity::ai_fireThreeBullet()
{
    
    const float speed = 1.0f;
    const float angles_deg[3] = { 210.0f, 270.0f, 330.0f };  // left,mid,right
    const float PI = 3.1415926f;

    for (int i = 0; i < 3; ++i)
    {
        Entity* b = bullets[m_nextBullet];

        // init pos
        float x_offset = (i - 1) * 0.3f;  // -0.3, 0, 0.3
        glm::vec3 spawnPos = m_position + glm::vec3(x_offset, -1.0f, 0.0f);
        b->set_position(spawnPos);

        // conver to radiant
        float angle_rad = angles_deg[i] * PI / 180.0f;

        // velocity
        float vx = speed * cos(angle_rad) * 1.5;
        float vy = speed * sin(angle_rad);
        glm::vec3 vel = glm::vec3(vx, vy, 0.0f);
        b->set_velocity(vel);

        m_nextBullet = (m_nextBullet + 1) % 16;
    }
}
void Entity::ai_fireWideFan()
{
    const int num_bullets = 6;
    const float start_angle_deg = 180.0f;
    const float end_angle_deg = 360.0f;
    const float angle_step = (end_angle_deg - start_angle_deg) / (num_bullets - 1);
    const float speed = 1.5f;
    const float PI = 3.1415926f;

    for (int i = 0; i < num_bullets; ++i)
    {
        Entity* b = bullets[m_nextBullet];

        float angle_deg = start_angle_deg + i * angle_step;
        float angle_rad = angle_deg * PI / 180.0f;

        float vx = speed * cos(angle_rad) * 3.0f;
        float vy = speed * sin(angle_rad) * 1.5f;
        glm::vec3 vel = glm::vec3(vx, vy, 0.0f);

        float x_offset = (i - (num_bullets - 1) / 2.0f) * 0.25f;
        glm::vec3 spawnPos = m_position + glm::vec3(x_offset, -1.0f, 0.0f);

        b->set_position(spawnPos);
        b->set_velocity(vel);

        m_nextBullet = (m_nextBullet + 1) % 16;
    }
}

void Entity::ai_attacker(Entity *player)
{
    if (m_fireTimer >= m_fireCooldown) {
        ai_fireOneBullet();
        m_fireTimer = 0.0f;
    }
    
    if (check_collision(player)) {
        player->get_attacked();
    }
}
//
void Entity::ai_defender(Entity *player)
{
    if (m_fireTimer >= m_fireCooldown) {
        ai_fireThreeBullet();
        m_fireTimer = 0.0f;
    }
    
    if (check_collision(player)) {
        player->get_attacked();
    }
}
//
void Entity::ai_boss(Entity *player)
{
    if (check_collision(player)) {
        player->get_attacked();
    }
    
    if (m_fireTimer >= m_fireCooldown) {
        // single shoot
        if (m_attackPhase < 3) {
            ai_fireDoubleBullet();
            m_attackPhase++;
        }
        //three bullet shoot
        else if (m_attackPhase < 6) {
            ai_fireThreeBullet();
            m_attackPhase++;
        }
        
        else if (m_attackPhase < 10) {
            ai_fireWideFan();
            m_attackPhase++;
        }
        // mixed
        else if (m_attackPhase < 12) {
            ai_fireDoubleBullet();
            ai_fireWideFan();
            m_attackPhase++;
        }
        // reset
        if (m_attackPhase >= 12) {
            m_attackPhase = 0;
        }
        // Chase the player
        float boss_x = m_position.x;
        float player_x = player->get_position().x;
        float vx = 0.0f;
        float move_speed = 0.3f;

        if (fabs(player_x - boss_x) > 0.05f) {
            if (player_x > boss_x) {
                vx = move_speed;
            } else {
                vx = -move_speed;
            }
        }

        m_velocity.x = vx;

        m_fireTimer = 0.0f;
    }
}
// Default constructor
Entity::Entity()
    : m_position(0.0f), m_movement(0.0f), m_scale(1.0f, 1.0f, 0.0f), m_model_matrix(1.0f), m_animation_cols(0), m_animation_frames(0), m_animation_index(0),
    m_animation_rows(0), m_animation_indices(nullptr), m_animation_time(0.0f),
    m_texture_id(0), m_velocity(0.0f), m_acceleration(0.0f), m_width(0.0f), m_height(0.0f) {}

Entity::Entity(GLuint texture_id, int animation_index, int animation_cols, int animation_rows, EntityType EntityType) :
    m_position(0.0f),
    m_movement(0.0f),
    m_scale(1.0f,1.0f,0.0f),
    m_model_matrix(1.0f),
    m_texture_id(texture_id),
    m_animation_index(animation_index),
    m_animation_cols(animation_cols),
    m_animation_rows(animation_rows),
    m_animation_frames(animation_cols * animation_rows),
    m_animation_indices(nullptr),
    m_animation_time(0.0f),
    m_velocity(0.0f),
    m_acceleration(0.0f),
    m_width(1.0f),
    m_height(1.0f),
    m_entity_type(EntityType),
    m_lives(1),
    m_is_active(true)
{}

Entity::Entity(GLuint texture_id, float width, float height, EntityType type)
    :   m_position(0.0f),
        m_movement(0.0f),
        m_scale(1.0f, 1.0f, 0.0f),
        m_model_matrix(1.0f),
        m_animation_cols(1),
        m_animation_frames(1),
        m_animation_index(0),
        m_animation_rows(1),
        m_animation_indices(nullptr),
        m_animation_time(0.0f),
        m_texture_id(texture_id),
        m_velocity(0.0f),
        m_acceleration(0.0f),
        m_width(width),
        m_height(height),
        m_entity_type(type),
        m_lives(1),
        m_is_active(true) {}


Entity::~Entity() { }

void Entity::draw_sprite_from_texture_atlas(ShaderProgram* program, GLuint texture_id, int index)
{
    // Step 1: Calculate the UV location of the indexed frame
    float u_coord = (float)(index % m_animation_cols) / (float)m_animation_cols;
    float v_coord = (float)(index / m_animation_cols) / (float)m_animation_rows;

    // Step 2: Calculate its UV size
    float width = 1.0f / (float)m_animation_cols;
    float height = 1.0f / (float)m_animation_rows;

    // Step 3: Just as we have done before, match the texture coordinates to the vertices
    float tex_coords[] =
    {
        u_coord, v_coord + height, u_coord + width, v_coord + height, u_coord + width, v_coord,
        u_coord, v_coord + height, u_coord + width, v_coord, u_coord, v_coord
    };
    float vertices[12] = {
        -0.5, -0.5, 0.5, -0.5,  0.5, 0.5,
        -0.5, -0.5, 0.5,  0.5, -0.5, 0.5
    };

    // Step 4: And render
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->get_position_attribute());

    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}

void Entity::draw_bullets_from_texture_atlas(ShaderProgram* program, GLuint texture_id, int index)
{
    // Step 1: Calculate the UV location of the indexed frame
    float u_coord = (float)(index % m_animation_cols) / (float)m_animation_cols;
    float v_coord = (float)(index / m_animation_cols) / (float)m_animation_rows;

    // Step 2: Calculate its UV size
    float width = 1.0f / (float)m_animation_cols;
    float height = 1.0f / (float)m_animation_rows;

    // Step 3: Just as we have done before, match the texture coordinates to the vertices
    float tex_coords[] =
    {
        u_coord, v_coord + height, u_coord + width, v_coord + height, u_coord + width, v_coord,
        u_coord, v_coord + height, u_coord + width, v_coord, u_coord, v_coord
    };
    
    // for rectangle
    float vertices[12] = {
        -0.09375f, -0.25f,   0.09375f, -0.25f,   0.09375f,  0.25f,
        -0.09375f, -0.25f,   0.09375f,  0.25f,  -0.09375f,  0.25f
    };
    // for round
    if (m_entity_type==BULLET_ROUND) {
        vertices[0] = -0.07941f;
        vertices[1] = -0.075f;

        vertices[2] =  0.07941f;
        vertices[3] = -0.075f;

        vertices[4] =  0.07941f;
        vertices[5] =  0.075f;

        vertices[6] = -0.07941f;
        vertices[7] = -0.075f;

        vertices[8] =  0.07941f;
        vertices[9] =  0.075f;

        vertices[10] = -0.07941f;
        vertices[11] =  0.075f;
    }
    
    

    // Step 4: And render
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->get_position_attribute());

    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}

void Entity::draw_enemy(ShaderProgram* program, GLuint texture_id) {
    float scale = 0.8f;
    if (m_ai_type==BOSS) scale = 3.0f;
    
   float vertices[] = {
       -0.5f * scale, -0.5f * scale,
        0.5f * scale, -0.5f * scale,
        0.5f * scale,  0.5f * scale,
       -0.5f * scale, -0.5f * scale,
        0.5f * scale,  0.5f * scale,
       -0.5f * scale,  0.5f * scale
   };

   float tex_coords[] = {
       0.0f,  0.0f,
       1.0f,  0.0f,
       1.0f,  1.0f,
       0.0f,  0.0f,
       1.0f,  1.0f,
       0.0f,  1.0f
   };

    glBindTexture(GL_TEXTURE_2D, m_texture_id);

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->get_position_attribute());
    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}


bool const Entity::check_collision(Entity* other) const
{
    float x_distance = fabs(m_position.x - other->m_position.x) - ((m_width + other->m_width) / 2.0f);
    float y_distance = fabs(m_position.y - other->m_position.y) - ((m_height + other->m_height) / 2.0f);

    return x_distance < 0.0f && y_distance < 0.0f;
}



void Entity::update(float delta_time, Entity *player, Entity *collidable_entities, int collidable_entity_count)
{
    if (m_entity_type==PLAYER && !m_is_active) return;
    if (!m_is_active && !m_enemy_alive) return;
    if (m_lives==0 && m_entity_type!=EXP) return;
 
    m_collided_top    = false;
    m_collided_bottom = false;
    m_collided_left   = false;
    m_collided_right  = false;
    m_collided_top_target = false;
    m_collided_bottom_target = false;
    
    // --- Animation --- //
    if (m_animation_indices != NULL)
    {
        m_animation_time += delta_time;
        float frames_per_second = (float) 1 / SECONDS_PER_FRAME;
        
        if (m_animation_time >= frames_per_second)
        {
            m_animation_time = 0.0f;
            m_animation_index++;
            
            if (m_animation_index >= m_animation_frames)
            {
                deactivate();
            }
        }
    }
    
    /*if (m_entity_type != BULLET && m_entity_type != BULLET_ROUND)*/
    if (m_entity_type == PLAYER) {
        m_velocity.x = m_movement.x * m_speed;
    }
    
    m_position.y += m_velocity.y * delta_time;
    m_position.x += m_velocity.x * delta_time;
    
    m_model_matrix = glm::mat4(1.0f);
    m_model_matrix = glm::translate(m_model_matrix, m_position);
        
    if (m_entity_type==ENEMY) {
//        printf("Enemy's position is %f\n",m_position.y);
        float diff = m_position.y - player->get_position().y;
        
        if (!m_is_active && diff <= 7.5f ) {
            activate();
        }
        if (m_ai_type==BOSS && diff <= 5.0f) m_velocity.y = 2.0f;
        if (diff <= 8.0f) {
            m_fireTimer += delta_time;
            ai_activate(player);
        }
    }
}


void Entity::render(ShaderProgram* program)
{
    if (!m_is_active && m_entity_type!=EXP) return;
    
    program->set_model_matrix(m_model_matrix);

    if (m_animation_indices != NULL)
    {
        draw_sprite_from_texture_atlas(program, m_texture_id, m_animation_indices[m_animation_index]);
        return;
    }
    if (m_entity_type==BULLET || m_entity_type==BULLET_ROUND ) {
        draw_bullets_from_texture_atlas(program, m_texture_id, m_animation_index);
        return;
    }
    if (m_entity_type==ENEMY) {
        draw_enemy(program, m_texture_id);
        return;
    }
    float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    float tex_coords[] = { 0.0,  1.0, 1.0,  1.0, 1.0, 0.0,  0.0,  1.0, 1.0, 0.0,  0.0, 0.0 };

    glBindTexture(GL_TEXTURE_2D, m_texture_id);

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->get_position_attribute());
    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
    
}

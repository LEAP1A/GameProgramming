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

constexpr int FONTBANK_SIZE = 16;
float platform_direction = 1.0f;

// Rocket constructor
Entity::Entity(std::vector<GLuint> texture_ids, glm::vec3 position)
        : m_texture_ids(texture_ids), m_position(position), m_velocity(0.0f), m_scale(0.8f, 0.8f, 0.0f),
        m_model_matrix(1.0f), current_status(IDLE), m_acceleration(glm::vec3(0.0f, 0.0f, 0.0f)) {}

// Constructor for static objects
Entity::Entity(GLuint texture_id, glm::vec3 position, glm::vec3 scale)
        : m_position(position), m_velocity(0.0f), m_scale(scale), m_model_matrix(1.0f), m_texture_id(texture_id) {}

// Constructor for texts
Entity::Entity(GLuint texture_id, glm::vec3 position, glm::vec3 scale, std::string txt)
        : m_position(position), m_velocity(0.0f), m_scale(scale), m_model_matrix(1.0f), m_texture_id(texture_id), text(txt) {}

Entity::~Entity() { }

void Entity::collision_detector(Entity* another) {
    
    float tolerance_x = 0.15f;
    float tolerance_y = 0.15f;
    
    float x_distance = fabs(m_position.x - another->get_position().x) - ((m_scale.x + another->get_scale().x) / 2.0f) + tolerance_x;
    float y_distance = fabs(m_position.y - another->get_position().y) - ((m_scale.y + another->get_scale().y) / 2.0f) + tolerance_y;

    if (x_distance < 0 && y_distance < 0)
    {
        
        if (another->m_position == glm::vec3(1.7f, -3.4f, 0.0f)) {
            // Make sure the velocity is small enough to land safely
            glm::vec3 velocity = get_velocity();
            float landing_speed = fabs(velocity.y);
            float horizontal_speed = fabs(velocity.x);
            
            if (landing_speed > 0.8f || horizontal_speed > 0.5f) {
                GAME_OVER = true;
            } else {
                GAME_WIN = true;
            }
        }
        else GAME_OVER = true;
    }
}

void Entity::collision_detector_with_moving(Entity* another) {
    
    float tolerance_x = 0.15f;
    float tolerance_y = 0.15f;
    
    float x_distance = fabs(m_position.x - another->get_position().x) - ((m_scale.x + another->get_scale().x) / 2.0f) + tolerance_x;
    float y_distance = fabs(m_position.y - another->get_position().y) - ((m_scale.y + another->get_scale().y) / 2.0f) + tolerance_y;

    if (x_distance < 0 && y_distance < 0)
    {
        glm::vec3 velocity = get_velocity();
    
        bool approaching_from_top = (m_position.y > another->get_position().y) && (velocity.y < 0);
        
        if (!approaching_from_top) {
            GAME_OVER = true;
            return;
        }
        
        float landing_speed = fabs(velocity.y);
        float horizontal_speed = fabs(velocity.x);
        
        if (landing_speed > 0.8f || horizontal_speed > 0.5f) {
            GAME_OVER = true;
        } else {
            GAME_WIN = true;
        }
    }
}

void Entity::draw(ShaderProgram* program)
{

    float vertices[12];
    float texture_coordinates[12] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f
    };
    
    GLuint current_texture;
    if (m_texture_ids.empty()) {
        current_texture = m_texture_id;
    } else {
        current_texture = (current_status == IDLE) ? m_texture_ids[0] : m_texture_ids[1];
    }
    
    glBindTexture(GL_TEXTURE_2D, current_texture);
    
    // calculate the offset so that the igniting rocket will not be "bounced up"
    float idle_height = 177.0f; // pic height of idle rocket
    float ignite_height = 246.0f; // pic height of fire rocket
    float width = 110.0f; // pic width for both rocket
    
    float aspect_ratio;
    float y_offset = 0.0f; // offset that will be used to adjust the location of the rocket after ignition
    
    if (!m_texture_ids.empty()) {
        if (current_status == IDLE) {
            aspect_ratio = idle_height / width;
        } else {
            aspect_ratio = ignite_height / width;
            // calculate the offset and standardlize
            float height_diff = (ignite_height - idle_height) / ignite_height;
            y_offset = -height_diff * aspect_ratio * 0.5f;
        }
    } else {
        aspect_ratio = 1.0f;
    }
    
    // standardlize the height
    float half_height = 0.5f * aspect_ratio;
    
    vertices[0] = -0.5f; vertices[1] = -half_height + y_offset;
    vertices[2] = 0.5f;  vertices[3] = -half_height + y_offset;
    vertices[4] = 0.5f;  vertices[5] = half_height + y_offset;
    
    vertices[6] = -0.5f; vertices[7] = -half_height + y_offset;
    vertices[8] = 0.5f;  vertices[9] = half_height + y_offset;
    vertices[10] = -0.5f; vertices[11] = half_height + y_offset;
    

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0,
                          vertices);
    glEnableVertexAttribArray(program->get_position_attribute());

    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0,
                          texture_coordinates);
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}

void Entity::draw_text(ShaderProgram* program, GLuint font_texture_id, std::string text,
               float font_size, float spacing, glm::vec3 position)
{
    // Scale the size of the fontbank in the UV-plane
    // We will use this for spacing and positioning
    float width = 1.0f / FONTBANK_SIZE;
    float height = 1.0f / FONTBANK_SIZE;

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

void Entity::update(float delta_time)
{
    m_velocity += m_acceleration * delta_time;
    m_position += m_velocity * delta_time;
    
    m_model_matrix = glm::mat4(1.0f);
    m_model_matrix = glm::translate(m_model_matrix, m_position);
    m_model_matrix = glm::scale(m_model_matrix, m_scale);
    
}
void Entity::update_platform(float delta_time)
{
    int speed = 1.0f;
    if (m_position.x >= 4.5f) {
        m_position.x -= 0.1f;
        platform_direction = -platform_direction;
    }
    else if (m_position.x <= -4.5f) {
        m_position.x += 0.1f;
        platform_direction = -platform_direction;
    }
    m_position.x += platform_direction * speed * delta_time;
    
    m_model_matrix = glm::mat4(1.0f);
    m_model_matrix = glm::translate(m_model_matrix, m_position);
    m_model_matrix = glm::scale(m_model_matrix, m_scale);
    
}

void Entity::render(ShaderProgram* program)
{
    program->set_model_matrix(m_model_matrix);
    if (text == "") draw(program);
    else draw_text(program, m_texture_id, text, 0.3f, 0.02f, m_position);
    
}

extern bool GAME_OVER;
extern bool GAME_WIN;

enum Animation { IDLE, IGNITE };

class Entity
{
private:
    // ————— TEXTURES ————— //
    std::vector<GLuint> m_texture_ids = {};  // Vector of texture IDs for different animations
    GLuint m_texture_id;

    // ————— ANIMATIONS ————— //
    Animation current_status;
    
    glm::vec3 m_acceleration = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 m_velocity = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 m_position;
    glm::vec3 m_scale;
    std::string text = "";

    glm::mat4 m_model_matrix;
    int fuel = 2000;
    
public:
    static constexpr int SECONDS_PER_FRAME = 6;

    // ————— CONSTRUCTORS ————— //
    Entity(std::vector<GLuint> texture_ids, glm::vec3 position);
    Entity(GLuint texture_id, glm::vec3 position, glm::vec3 scale);
    Entity(GLuint texture_id, glm::vec3 position, glm::vec3 scale, std::string txt);
    ~Entity();

    // ————— METHODS ————— //
    void draw(ShaderProgram* program);
    void draw_text(ShaderProgram* program, GLuint font_texture_id, std::string text,
                   float font_size, float spacing, glm::vec3 position);
    void update(float delta_time);
    void update_platform(float delta_time);
    void render(ShaderProgram* program);
    void collision_detector(Entity* another);
    void collision_detector_with_moving(Entity* another);


    // Getters and Setters
    glm::vec3 const get_position() const { return m_position; }
    glm::vec3 const get_velocity() const { return m_velocity; }
    glm::vec3 const get_scale() const { return m_scale; }
    int const get_acceleration_x() const {return m_acceleration.x;}
    int const get_fuel() const {return fuel;}
 
    void const set_position(glm::vec3 new_position) { m_position = new_position; }
    void const set_velocity_x(float new_mov) { m_velocity.x = new_mov; }
    void const set_velocity_y(float new_mov) { m_velocity.y = new_mov; }
    void const set_scale(glm::vec3 new_scale) { m_scale = new_scale; }
    void const set_acceleration_y(float new_acc) { m_acceleration.y = new_acc; }
    void const set_acceleration_x(float new_acc) { m_acceleration.x = new_acc; }
    void const set_status(Animation new_status) { current_status = new_status; }
    void const set_fuel() { fuel -= 1; }
    void const set_text(std::string new_text) { text = new_text; }
};

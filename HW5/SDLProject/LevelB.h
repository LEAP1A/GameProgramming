#include "Scene.h"

class LevelB : public Scene {
    GLuint background_texture_id;
    glm::mat4 background_matrix = glm::mat4(1.0f);
    
public:
    // ————— STATIC ATTRIBUTES ————— //
    int ENEMY_COUNT = 22;
    
    // ————— DESTRUCTOR ————— //
    ~LevelB();
    
    // ————— METHODS ————— //
    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
    void spawnExplosion(const glm::vec3& pos);
};

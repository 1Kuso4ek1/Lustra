#pragma once
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <vector>

namespace dev {

class Matrices
{
public:
    struct Binding
    {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 projection;
    };

    Matrices();

    void PushMatrix();
    void PopMatrix();

    void Scale(glm::vec3 size);
    void Translate(glm::vec3 pos);
    void Rotate(float angle, glm::vec3 axis);

    glm::mat4& GetModel();
    glm::mat4& GetView();
    glm::mat4& GetProjection();

    Binding GetBinding() const;

private:

    std::vector<glm::mat4> model, view, projection;
};

}

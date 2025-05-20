#include <Matrices.hpp>

namespace lustra
{

Matrices::Matrices()
{
    // I guess it can optimize everything a little bit
    model.reserve(3);
    view.reserve(3);
    projection.reserve(3);

    model.push_back(glm::mat4(1.0f));
    view.push_back(glm::mat4(1.0f));
    projection.push_back(glm::mat4(1.0f));
}

void Matrices::PushMatrix()
{
    model.push_back(model.back());
    view.push_back(view.back());
}

void Matrices::PopMatrix()
{
    if(model.size() > 1)
    {
        model.pop_back();
        view.pop_back();
    }
}

void Matrices::Scale(const glm::vec3 size)
{
    model.back() = glm::scale(model.back(), size);
}

void Matrices::Translate(const glm::vec3 pos)
{
    model.back() = glm::translate(model.back(), pos);
}

void Matrices::Rotate(const float angle, const glm::vec3 axis)
{
    model.back() = glm::rotate(model.back(), angle, axis);
}

glm::mat4& Matrices::GetModel()
{
    return model.back();
}

glm::mat4& Matrices::GetView()
{
    return view.back();
}

glm::mat4& Matrices::GetProjection()
{
    return projection.back();
}

Matrices::Binding Matrices::GetBinding() const
{
    return { model.back(), view.back(), projection.back() };
}

}

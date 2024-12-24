#pragma once
#include <LLGL/Types.h>
#include <entt/fwd.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace dev
{

struct CameraComponent;

class Camera
{
public:
    Camera() = default;

    void SetPerspective();
    void SetOrthographic(float left, float right, float bottom, float top);
    void SetViewport(const LLGL::Extent2D& resolution);

    void SetFov(float fov);
    void SetNear(float near);
    void SetFar(float far);

    void SetFirstPerson(bool firstPerson);

    glm::mat4 GetProjectionMatrix() const;
    glm::vec2 GetViewportSize() const;
    
    float GetFov() const;
    float GetNear() const;
    float GetFar() const;

    float GetAspect() const;

    bool IsFirstPerson() const;
    
private:
    float fov = 90.0f;
    float near = 0.1f;
    float far = 100.0f;

    float aspect;

    bool firstPerson = true;

private:
    glm::mat4 projectionMatrix;

    glm::vec2 viewportSize;

private:
    friend void DrawComponentUI(CameraComponent& component, entt::entity entity);
};

}

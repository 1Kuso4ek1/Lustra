#pragma once
#include <EventManager.hpp>

#include <LLGL/Types.h>
#include <entt/fwd.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace dev
{

struct CameraComponent;

class Camera : public EventListener
{
public:
    Camera();
    ~Camera();

    void SetPerspective();
    void SetOrthographic(float left, float right, float bottom, float top);
    void SetViewport(const LLGL::Extent2D& resolution);

    void SetFov(float fov);
    void SetNear(float near);
    void SetFar(float far);

    void SetFirstPerson(bool firstPerson);

    void SetUp(const glm::vec3& up);

    void SetLookAt(const glm::vec3& lookAtPos);
    void SetViewMatrix(const glm::mat4& viewMatrix);

    void OnEvent(Event& event) override;

    glm::vec2 WorldToScreen(const glm::vec3& pos) const;
    glm::vec3 ScreenToWorld(const glm::vec2& pos) const;

    glm::vec3 GetUp() const;
    glm::vec3 GetLookAt() const;
    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectionMatrix() const;
    glm::vec2 GetViewport() const;
    
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
    glm::vec3 up{ 0.0f, 1.0f, 0.0f };

    glm::vec3 lookAtPos;

    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;

    glm::vec2 viewportSize;

private:
    friend void DrawComponentUI(CameraComponent& component, entt::entity entity);
};

}

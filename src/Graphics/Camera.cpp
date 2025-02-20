#include <Camera.hpp>

namespace dev
{

Camera::Camera()
{
    EventManager::Get().AddListener(Event::Type::WindowResize, this);
}

Camera::Camera(Camera&& other)
    : fov(other.fov), near(other.near), far(other.far), aspect(other.aspect),
      firstPerson(other.firstPerson), up(other.up), lookAtPos(other.lookAtPos),
      viewMatrix(other.viewMatrix), projectionMatrix(other.projectionMatrix), viewportSize(other.viewportSize)
{
    EventManager::Get().AddListener(Event::Type::WindowResize, this);
}

Camera::~Camera()
{
    EventManager::Get().RemoveListener(Event::Type::WindowResize, this);
}

void Camera::SetPerspective()
{
    projectionMatrix = glm::perspective(glm::radians(fov), aspect, near, far);
}

void Camera::SetOrthographic(float left, float right, float bottom, float top)
{
    projectionMatrix = glm::ortho(left, right, bottom, top, near, far);
}

void Camera::SetViewport(const LLGL::Extent2D& resolution)
{
    viewportSize = { resolution.width, resolution.height };
    aspect = (float)resolution.width / resolution.height;

    SetPerspective();
}

void Camera::SetFov(float fov)
{
    this->fov = fov;
    SetPerspective();
}

void Camera::SetNear(float near)
{
    this->near = near;
    SetPerspective();
}

void Camera::SetFar(float far)
{
    this->far = far;
    SetPerspective();
}

void Camera::SetFirstPerson(bool firstPerson)
{
    this->firstPerson = firstPerson;
}

void Camera::SetUp(const glm::vec3& up)
{
    this->up = up;
}

void Camera::SetLookAt(const glm::vec3& lookAtPos)
{
    this->lookAtPos = lookAtPos;
}

void Camera::SetViewMatrix(const glm::mat4& viewMatrix)
{
    this->viewMatrix = viewMatrix;
}

void Camera::OnEvent(Event& event)
{
    if(event.GetType() == Event::Type::WindowResize)
    {
        auto resizeEvent = dynamic_cast<WindowResizeEvent*>(&event);

        SetViewport(resizeEvent->GetSize());
    }
}

glm::vec2 Camera::WorldToScreen(const glm::vec3& pos) const
{
    auto viewPos = viewMatrix * glm::vec4(pos, 1.0f);

    // Not in front of the camera
    if(viewPos.z > 0.0)
        return { NAN, NAN };

    auto clipPos = projectionMatrix * viewPos;

    clipPos /= clipPos.w;

    glm::vec2 res = {
        (clipPos.x + 1.0f) * viewportSize.x / 2.0f,
        (1.0f - clipPos.y) * viewportSize.y / 2.0f
    };
    
    return (
        glm::all(
            glm::lessThanEqual(glm::abs(res), viewportSize)
        ) ? res : glm::vec2(NAN, NAN)
    );
}

glm::vec3 Camera::ScreenToWorld(const glm::vec2& pos) const
{
    auto clipPos = glm::vec4(
        (pos.x / viewportSize.x) * 2.0f - 1.0f,
        1.0f - (pos.y / viewportSize.y) * 2.0f,
        0.0f,
        1.0f
    );

    auto inv = glm::inverse(projectionMatrix * viewMatrix);

    auto worldPos = inv * clipPos;

    return worldPos / worldPos.w;
}

glm::vec3 Camera::GetUp() const
{
    return up;
}

glm::vec3 Camera::GetLookAt() const
{
    return lookAtPos;
}

glm::mat4 Camera::GetViewMatrix() const
{
    return viewMatrix;
}

glm::mat4 Camera::GetProjectionMatrix() const
{
    return projectionMatrix;
}

glm::vec2 Camera::GetViewport() const
{
    return viewportSize;
}

float Camera::GetFov() const
{
    return fov;
}

float Camera::GetNear() const
{
    return near;
}

float Camera::GetFar() const
{
    return far;
}

float Camera::GetAspect() const
{
    return aspect;
}

bool Camera::IsFirstPerson() const
{
    return firstPerson;
}

}

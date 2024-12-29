#pragma once
#include <Renderer.hpp>

namespace dev
{

class RendererBase
{
public:
    RendererBase() = default;
    virtual ~RendererBase() = default;

    virtual void Draw(const std::unordered_map<uint32_t, LLGL::Resource*>& resources,
                      std::function<void(LLGL::CommandBuffer*)> setUniforms,
                      LLGL::RenderTarget* renderTarget = Renderer::Get().GetSwapChain()) {};

    virtual LLGL::RenderTarget* GetPrimaryRenderTarget();
};

}

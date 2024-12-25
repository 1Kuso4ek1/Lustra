#pragma once
#include <Renderer.hpp>

namespace dev
{

class RendererBase
{
public:
    RendererBase() = default;
    virtual ~RendererBase() = default;

    virtual void Draw(LLGL::RenderTarget* renderTarget = Renderer::Get().GetSwapChain()) {};

    virtual LLGL::RenderTarget* GetPrimaryRenderTarget();
};

}

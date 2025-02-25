#include <RendererBase.hpp>

namespace lustra
{

LLGL::RenderTarget* RendererBase::GetPrimaryRenderTarget()
{
    return Renderer::Get().GetSwapChain();
}

LLGL::Texture* RendererBase::GetDepth()
{
    return nullptr;
}

}

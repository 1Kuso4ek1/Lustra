#include <RendererBase.hpp>

namespace dev
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

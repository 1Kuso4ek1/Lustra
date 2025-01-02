#include <RendererBase.hpp>

namespace dev
{

LLGL::RenderTarget* RendererBase::GetPrimaryRenderTarget()
{
    return Renderer::Get().GetSwapChain();
}

}

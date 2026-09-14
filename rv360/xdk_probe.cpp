#include <xtl.h>
#include <xgraphics.h>
#include <d3d9.h>
#include <xaudio2.h>

static IDirect3DDevice9 *g_device;

void rv360_xdk_probe(void)
{
    D3DVIEWPORT9 viewport;
    ZeroMemory(&viewport, sizeof(viewport));
    g_device->SetViewport(&viewport);
    g_device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
}

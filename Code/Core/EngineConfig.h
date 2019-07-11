#pragma once

// Set "RENDERER_STATISTICS" as preprocessor definition in order to enable the gathering of statistics(tiny binary size and tiny negative performance impact)
#define RENDERER_STATISTICS 1

// Set "RENDERER_DEBUG" as preprocessor definition in order to enable e.g.Direct3D 9 PIX functions(D3DPERF_ * functions, also works directly within VisualStudio 2017 out - of - the - box) debug features(disabling support just reduces the binary size slightly but makes debugging more difficult)
//#define RENDERER_DEBUG 1

#define RENDERER_NULL
#define RENDERER_DIRECT3D11
#define RENDERER_DIRECT3D12
#define RENDERER_OPENGLES3
#define RENDERER_OPENGL
#define RENDERER_OPENGL_GLSLTOSPIRV
#define RENDERER_VULKAN
#define RENDERER_VULKAN_GLSLTOSPIRV
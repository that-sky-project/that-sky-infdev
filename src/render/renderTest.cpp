#include <Windows.h>
#include <includes/htmodloader.h>
#include <Utils/Types.h>
#include <Utils/StlAllocator.hpp>
#include <Memory/Heap.hpp>
#include "utils/htmodloader.hpp"
#include "render/renderTest.hpp"
#include "sky/skyGame.hpp"
#include "sky/skyScene.hpp"
#include "sky/skyMaterialDefBarn.hpp"
#include "sky/skyMetaHelper.hpp"
#include "mod/base/override.hpp"
#include "mod/scriptEngine/luacall.hpp"

typedef void (*PFN_NetModule_Initialize)(
  void *, void *, void *, Game *, void *, void *);
typedef void (*PFN_NetModule_Update)(
  void *, Game *, void *, void *, void *, void *);

struct GrassShVertex {
  static constexpr u32 kNumAttrs = 5;
  static constexpr GfxType kTypes[kNumAttrs] = {kGfxType_FLOAT3, kGfxType_BYTE4, kGfxType_UBYTE4, kGfxType_UBYTE4, kGfxType_UBYTE4};
  static constexpr GfxAttr kAttrs[kNumAttrs] = {kGfxAttr_Position, kGfxAttr_Normal, kGfxAttr_Light0, kGfxAttr_Light1, kGfxAttr_Light2};

  GrassShVertex(
    f32 x,
    f32 y,
    f32 z
  ) {
    a_position[0] = x;
    a_position[1] = y;
    a_position[2] = z;
  }

  f32 a_position[3] = {0};
  u32 a_normal = 0x000000FF;
  u32 a_light0 = 0x0FF0F0F0;
  u32 a_light1 = 0x7F7F7F7F;
  u32 a_light2 = 0x66666666;
};

struct TerrainDepthVertex {
  static constexpr u32 kNumAttrs = 1;
  static constexpr GfxType kTypes[kNumAttrs] = {kGfxType_FLOAT3};
  static constexpr GfxAttr kAttrs[kNumAttrs] = {kGfxAttr_Position};

  TerrainDepthVertex(
    f32 x,
    f32 y,
    f32 z
  ) {
    a_position[0] = x;
    a_position[1] = y;
    a_position[2] = z;
  }

  f32 a_position[3] = {0};
};

static const TerrainDepthVertex sVerticesT[3] = {{0, 1, 0}, {0, 1, 100}, {100, 1, 100}};
static const GrassShVertex sVerticesG[3] = {{0, 1, 0}, {0, 1, 100}, {100, 1, 100}};
static const u16 sIndicesT[6] = {0, 1, 2, 0, 2, 1};
static const u16 sIndicesG[6] = {0, 1, 2, 0, 2, 1};

void RenderTest::Initialize(
  Game *game
) {
  /*lua_debugdostring(
    GetOverride()->GetLua(),
    "local shaders = {\"EndPortal\", \"SimpleColorTest\"}\n"
    "\n"
    "for i = 1, #shaders do\n"
    "  local shaderName = shaders[i]\n"
    "  if game:resources():GetResource(\"Shader\", shaderName) == nil then\n"
    "    local res = Shader.new(game:resourceHeap())\n"
    "    res:name(shaderName)\n"
    "    res:heap(game:resourceHeap())\n"
    "\n"
    "    -- Resource parameters.\n"
    "    res:group(\"Opaque\")\n"
    "    res:vs(shaderName .. \".vert\")\n"
    "    res:fs(shaderName .. \".frag\")\n"
    "\n"
    "    res:IncLoadCount()\n"
    "    res:IncLoadCount()\n"
    "    game:resources():LoadImmediate(res)\n"
    "  end\n"
    "end\n"
  );*/

  materialDefBarn = game->resolveMember<MaterialDefBarn *>("materialDefBarn");
  resourceManager = game->resolveMember<ResourceManager *>("resources");
  heap = game->resolveMember<Heap *>("levelHeap");
  scene = game->resolveMember<Scene *>("scene");

  HTTellText("§c[ThatSkyInfdev] game.materialDefBarn = %p", materialDefBarn);
  HTTellText("§c[ThatSkyInfdev] game.resourceManager = %p", resourceManager);
  HTTellText("§c[ThatSkyInfdev] game.levelHeap = %p", heap);
  HTTellText("§c[ThatSkyInfdev] game.scene = %p", scene);

  m_InitializeTerrain();
  m_InitializeEndPortal();
}

void RenderTest::Terminate() {
  m_TerminateTerrain();
  m_TerminateEndPortal();
}

void RenderTest::Update() {
  m_UpdateTerrain();
  m_UpdateEndPortal();
}

void RenderTest::m_InitializeTerrain() {
  ;
}

void RenderTest::m_InitializeEndPortal() {
  // Load shader.
  lua_debugdostring(
    GetOverride()->GetLua(),
    "local res = game:resources():GetResource(\"Shader\", \"EndPortal\")\n"
    "if res == nil then\n"\
    "  res = Shader.new(game:resourceHeap())"
    "  res:name(\"EndPortal\")\n"
    "  res:heap(game:resourceHeap())\n"
    "\n"
    "  -- Resource parameters.\n"
    "  res:group(\"Opaque\")\n"
    "  res:vs(\"EndPortal.vert\")\n"
    "  res:fs(\"EndPortal.frag\")\n"
    "\n"
    "  game:resources():LoadImmediate(res)\n"
    "end\n"
    "\n"
    "res:IncLoadCount()\n"
  );

  const GfxType t[3] = {kGfxType_FLOAT3, kGfxType_FLOAT2, kGfxType_FLOAT4};
  const GfxAttr a[3] = {kGfxAttr_Position, kGfxAttr_TexCoord0, kGfxAttr_Color};

  const u16 indices[3 * 4] = {
    0, 1, 2, 2, 3, 0,
    4, 5, 6, 6, 7, 4,
  };

  endportalD.BeginDefinition("EndPortalTest", 8);
  endportalD.AddVertexBuffer(
    0,
    t,
    a,
    3,
    kGfxBind_UploadTriple,
    0,
    nullptr);
  endportalD.AddIndexBuffer(
    0, kGfxType_SHORT, kGfxBind_UploadSingle, 12, indices);
  endportalD.EndDefinition();

  RenderList *rl = scene->GetRenderListByName("Opaque");

  endportalR.Initialize(&endportalD, resourceManager, "EndPortal", rl, 0, nullptr);
  endportalR.SetPrimitiveCapacity(0x6);
  //endportalR.AllocVertexSparse(0, nullptr, 0x400);

  MaterialDefBarn::SetMaterialShaderUniforms(
    endportalR.GetPipelineInstance(),
    materialDefBarn->GetDef(kMaterial_None),
    resourceManager);
}

void RenderTest::m_TerminateTerrain() {
  ;
}

void RenderTest::m_TerminateEndPortal() {
  // Unload shader.
  lua_debugdostring(
    GetOverride()->GetLua(),
    "local res = game:resources():GetResource(\"Shader\", \"EndPortal\")\n"
    "if res ~= nil then\n"
    "  res:DecLoadCount()\n"
    "  if res:GetLoadCount() == 0 then\n"
    "    game:resources():UnloadImmediate(res)\n"
    "    Shader.delete(res, game:resourceHeap())\n"
    "  end\n"
    "end\n"
  );

  //endportalR.Terminate();
  endportalD.Release();
}

void RenderTest::m_UpdateTerrain() {
  ;
}

void RenderTest::m_UpdateEndPortal() {
  GpuBuffer &gpuBuffer = endportalD.GetVertexBuffer(0);
  void *mem = gpuBuffer.MapBuffer();
  if (mem) {
    float vao[9 * 4] = {
      0,  1,  0, 0, 0, 0.5, 1, 0.5, 0.5,
      0,  1, 10, 0, 1, 0.5, 1, 0.5, 0.5,
      10, 1, 10, 1, 0, 0.5, 1, 0.5, 0.5,
      10, 1,  0, 1, 0, 0.5, 1, 0.5, 0.5,
    };
    memset(mem, 0, 9 * 8);
    memcpy(mem, vao, sizeof(vao));
    gpuBuffer.UnmapBuffer();
  }

  endportalR.Queue();
}

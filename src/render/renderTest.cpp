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

static RenderTest *gRenderTest = nullptr;
//static u08 gBuffer[sizeof(RenderTest)] = {0};

static HTAsmFunction sfn_NetModule_Initialize = {
  "NetModule::Initialize()",
  nullptr,
  nullptr,
  nullptr
};

static HTAsmFunction sfn_NetModule_Update = {
  "NetModule::Update()",
  nullptr,
  nullptr,
  nullptr
};

static void hook_NetModule_Initialize(
  void *a1,
  void *a2,
  void *a3,
  Game *a4,
  void *a5,
  void *a6,
  void *a7
) {
  ((PFN_NetModule_Initialize)sfn_NetModule_Initialize.origin)(
    a1, a2, a3, a4, a5, a6);
}

static void hook_NetModule_Update(
  void *a1,
  Game *a2,
  void *a3,
  void *a4,
  void *a5,
  void *a6,
  void *a7
) {
  ((PFN_NetModule_Update)sfn_NetModule_Update.origin)(
    a1, a2, a3, a4, a5, a6);

  if (!gRenderTest) {
    //gRenderTest = new RenderTest();
    // Allocate the RenderTest object in a single memory page to set memory
    // R/W breakpoints.
    //void *buffer = VirtualAlloc((void *)0x0000001145140000ull, 0x1000, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    void *buffer = VirtualAlloc(nullptr, 0x2000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    Assert(buffer);
    new (buffer) RenderTest();
    gRenderTest = (RenderTest *)buffer;
    gRenderTest->initialize(a2);
  } else {
    gRenderTest->update();
  }
}

// We attatch the render module to NetModule.
void RenderTest::installHook() {
  uintptr_t baseAddr = (uintptr_t)GetModuleHandleA(nullptr);

  sfn_NetModule_Initialize.fn = (void *)(baseAddr + 0x00CF96F0);
  sfn_NetModule_Initialize.detour= (void *)hook_NetModule_Initialize;
  HTModLoader::createHookAndEnable(nullptr, &sfn_NetModule_Initialize);

  sfn_NetModule_Update.fn = (void *)(baseAddr + 0x00CFA5B0);
  sfn_NetModule_Update.detour= (void *)hook_NetModule_Update;
  HTModLoader::createHookAndEnable(nullptr, &sfn_NetModule_Update);
}

static const TerrainDepthVertex sVerticesT[3] = {{0, 1, 0}, {0, 1, 100}, {100, 1, 100}};
static const GrassShVertex sVerticesG[3] = {{0, 1, 0}, {0, 1, 100}, {100, 1, 100}};
static const u16 sIndicesT[6] = {0, 1, 2, 0, 2, 1};
static const u16 sIndicesG[6] = {0, 1, 2, 0, 2, 1};

void RenderTest::initialize(
  Game *game
) {
  materialDefBarn = game->resolveMember<MaterialDefBarn *>("materialDefBarn");
  resourceManager = game->resolveMember<ResourceManager *>("resources");
  heap = game->resolveMember<Heap *>("levelHeap");
  scene = game->resolveMember<Scene *>("scene");

  HTTellText("§c[ThatSkyInfdev] game.materialDefBarn = %p", materialDefBarn);
  HTTellText("§c[ThatSkyInfdev] game.resourceManager = %p", resourceManager);
  HTTellText("§c[ThatSkyInfdev] game.levelHeap = %p", heap);
  HTTellText("§c[ThatSkyInfdev] game.scene = %p", scene);

  m_initializeTerrain();
  m_initializeEndPortal();
}

void RenderTest::update() {
  m_updateTerrain();
  m_updateEndPortal();
}

void RenderTest::m_initializeTerrain() {
  ;
}

void RenderTest::m_initializeEndPortal() {
  const GfxType t[3] = {kGfxType_FLOAT3, kGfxType_FLOAT2, kGfxType_FLOAT4};
  const GfxAttr a[3] = {kGfxAttr_Position, kGfxAttr_TexCoord0, kGfxAttr_Color};

  const u16 indices[3 * 4] = {
    0, 1, 2, 1, 3, 2,
    0, 2, 1, 1, 2, 3,
  };

  endportalD.BeginDefinition("EndPortalTest", 4);
  endportalD.AddVertexBuffer(
    0,
    t,
    a,
    3,
    kGfxBind_UploadTriple,
    0,
    nullptr);
  endportalD.AddIndexBuffer(
    0, kGfxType_SHORT, kGfxBind_UploadSingle, 0x6, indices);
  endportalD.EndDefinition();

  RenderList *rl = scene->GetRenderListByName("Opaque");

  endportalR.Initialize(&endportalD, resourceManager, "EndPortal", rl, 0, nullptr);
  endportalR.AllocVertexSparse(0, nullptr, 0x400);

  MaterialDefBarn::SetMaterialShaderUniforms(
    endportalR.GetPipelineInstance(),
    materialDefBarn->GetDef(kMaterial_None),
    resourceManager);
}

void RenderTest::m_updateTerrain() {
  ;
}

void RenderTest::m_updateEndPortal() {
  GpuBuffer &gpuBuffer = endportalD.GetVertexBuffer(0);
  void *mem = gpuBuffer.MapBuffer();
  if (mem) {
    float vao[9 * 4] = {
      0,  1,  0, 0, 0, 0.5, 1, 0.5, 0.5,
      0,  1, 10, 0, 1, 0.5, 1, 0.5, 0.5,
      10, 1,  0, 1, 0, 0.5, 1, 0.5, 0.5,
      10, 1, 10, 1, 0, 0.5, 1, 0.5, 0.5,
    };
    memcpy(mem, vao, sizeof(vao));
    gpuBuffer.UnmapBuffer();
  }

  endportalR.Queue();
}

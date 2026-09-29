#include <Windows.h>
#include <includes/htmodloader.h>
#include <Utils/Types.h>
#include <Utils/StlAllocator.hpp>
#include <Memory/Heap.hpp>
#include "utils/htmodloader.hpp"
#include "render/renderTest.hpp"
#include "render/vertexArrayElements.hpp"
#include "sky/skyGame.hpp"
#include "sky/skyScene.hpp"
#include "sky/skyMaterialDefBarn.hpp"
#include "sky/skyMetaHelper.hpp"
#include "mod/base/override.hpp"
#include "mod/scriptEngine/luacall.hpp"

static const TerrainDepthVertex sVerticesT[3] = {{0, 1, 0}, {0, 1, 100}, {100, 1, 100}};
static const GrassShVertex sVerticesG[3] = {{0, 1, 0}, {0, 1, 100}, {100, 1, 100}};
static const u16 sIndicesT[6] = {0, 1, 2, 0, 2, 1};
static const u16 sIndicesG[6] = {0, 1, 2, 0, 2, 1};

static void s_LuaLoadShader(
  cstring name
) {
  static constexpr cstring s_loader = 
    "local name = \"%s\""
    "local res = game:resources():GetResource(\"Shader\", name)\n"
    "if res == nil then\n"\
    "  res = Shader.new(game:resourceHeap())"
    "  res:name(name)\n"
    "  res:heap(game:resourceHeap())\n"
    "\n"
    "  -- Resource parameters.\n"
    "  res:group(\"Opaque\")\n"
    "  res:vs(name .. \".vert\")\n"
    "  res:fs(name .. \".frag\")\n"
    "\n"
    "  game:resources():LoadImmediate(res)\n"
    "end\n"
    "\n"
    "res:IncLoadCount()\n";

  char script[2048];

  snprintf(script, sizeof(script), s_loader, name);
  lua_debugdostring(GetOverride()->GetLua(), script);
}

static void s_LuaUnloadShader(
  cstring name
) {
  static constexpr cstring s_loader =
    "local res = game:resources():GetResource(\"Shader\", \"%s\")\n"
    "if res ~= nil then\n"
    "  res:DecLoadCount()\n"
    "  if res:GetLoadCount() == 0 then\n"
    "    game:resources():UnloadImmediate(res)\n"
    "    Shader.delete(res, game:resourceHeap())\n"
    "  end\n"
    "end\n";

  char script[2048];

  snprintf(script, sizeof(script), s_loader, name);
  lua_debugdostring(GetOverride()->GetLua(), script);
}

namespace RenderTest {

// ----------------------------------------------------------------------------
// [SECTION] RenderTest/ShaderTest
// ----------------------------------------------------------------------------

void ShaderTest::Initialize(
  Game *game
) {
  m_Initialize(
    game->resolveMember<Scene *>("scene"),
    game->resolveMember<ResourceManager *>("resources"),
    game->resolveMember<MaterialDefBarn *>("materialDefBarn"),
    game->resolveMember<CollisionGeoBarn *>("collisionGeoBarn")
  );
}

void ShaderTest::m_Initialize(
  Scene *scene,
  ResourceManager *resources,
  MaterialDefBarn *materialDefBarn,
  CollisionGeoBarn *collisionGeoBarn
) {
  m_collisionGeoBarn = collisionGeoBarn;

  // Load shader.
  s_LuaLoadShader("EndPortal");

  const GfxType t[3] = {kGfxType_FLOAT3, kGfxType_FLOAT2, kGfxType_FLOAT4};
  const GfxAttr a[3] = {kGfxAttr_Position, kGfxAttr_TexCoord0, kGfxAttr_Color};

  const u16 indices[3 * 4] = {
    0, 1, 2, 2, 3, 0,
    4, 5, 6, 6, 7, 4,
  };

  m_endportalD.BeginDefinition("EndPortalTest", 8);
  m_endportalD.AddVertexBuffer(0, t, a, 3, kGfxBind_UploadTriple, 0, nullptr);
  m_endportalD.AddIndexBuffer(0, kGfxType_SHORT, kGfxBind_UploadSingle, 12, indices);
  m_endportalD.EndDefinition();

  RenderList *rl = scene->GetRenderListByName("Opaque");

  m_endportalR.Initialize(&m_endportalD, resources, "EndPortal", rl, 0, nullptr);
  m_endportalR.SetPrimitiveCapacity(0x6);

  MaterialDefBarn::SetMaterialShaderUniforms(
    m_endportalR.GetPipelineInstance(),
    materialDefBarn->GetDef(kMaterial_None),
    resources);

  HTTellText("§a[ThatSkyInfdev] RenderTest: Initialized ShaderTest");

  static const TerrainDepthVertex s_vertexData[4] = {{0, 1, 0}, {0, 1, 10}, {10, 1, 10}, {10, 1, 0}};
  static const u16 s_indexData[6] = {0, 1, 2, 2, 3, 0};
  static const Material s_mtrlData[4] = {kMaterial_Cliff, kMaterial_Cliff, kMaterial_Cliff, kMaterial_Cliff};
  static const u32 s_colorData = 0xFFFFFFFF;
  static const u32 s_lightData = 0xFFFFFFFF;

  Matrix4 transform = Matrix4(1);

  CollisionGeoMeshData meshData;
  meshData.tag = "Test";
  meshData.idxBuffer = s_indexData;
  meshData.idxCount = 6;
  meshData.idxStride = sizeof(u16);
  meshData.vtxBuffer = s_vertexData;
  meshData.vtxCount = 4;
  meshData.vtxStride = sizeof(TerrainDepthVertex);
  meshData.min = Vector4(-0.1f, 0.9f, -0.1f, 0);
  meshData.max = Vector4(10.1f, 1.1f, 10.1f, 0);
  m_geoIndex = m_collisionGeoBarn->AddGeo(meshData);

  HTTellText("§a[ThatSkyInfdev] RenderTest: geoindex = %d", m_geoIndex);

  CollisionGeoInstanceData instData;
  instData.mtrlData = s_mtrlData;
  instData.mtrlType = kGfxType_UBYTE;
  instData.mtrlStride = 0;//sizeof(Material);
  instData.colorData = &s_colorData;
  instData.colorType = kGfxType_UBYTE4;
  instData.colorStride = 0;
  /*instData.lightData = &s_lightData;
  instData.lightType = kGfxType_UBYTE4;
  instData.lightStride = 0;*/
  instData.mask = 0x40;
  instData.unk_1 = 1000.0f;
  m_geoInst = m_collisionGeoBarn->AddInstance(m_geoIndex, transform, instData, nullptr);

  HTTellText("§c[ThatSkyInfdev] RenderTest: geoInst = %p", m_geoInst);
}

void ShaderTest::Terminate() {
  // Remove collision.
  m_collisionGeoBarn->RemoveInstance(m_geoInst);
  m_collisionGeoBarn->RemoveGeo(m_geoIndex);

  m_geoInst = nullptr;
  m_geoIndex = 0;

  // Unload shader.
  s_LuaUnloadShader("EndPortal");

  // Release renderer.
  m_endportalR.Release();
  m_endportalD.Release();
}

void ShaderTest::BuildScene() {
  GpuBuffer &gpuBuffer = m_endportalD.GetVertexBuffer(0);
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

  m_endportalR.Queue();
}

// ----------------------------------------------------------------------------
// [SECTION] RenderTest/TextureTest
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// [SECTION] RenderTest/Render
// ----------------------------------------------------------------------------

void Render::Initialize(
  Game *game
) {
  tests = new Test *[3];
  tests[0] = new TerrainTest();
  tests[1] = new ShaderTest();
  tests[2] = new TextureTest();

  for (i32 i = 0; i < 3; i++)
    tests[i]->Initialize(game);
}

void Render::Terminate() {
  for (i32 i = 0; i < 3; i++) {
    tests[i]->Terminate();
    delete tests[i];
  }

  delete[] tests;
}

void Render::BuildScene() {
  for (i32 i = 0; i < 3; i++)
    tests[i]->BuildScene();
}

}

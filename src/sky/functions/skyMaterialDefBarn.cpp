#include "utils/rva.hpp"
#include "sky/skyMaterialDefBarn.hpp"

void MaterialDefBarn::SetMaterialShaderUniforms(
  PipelineInstance *pipelineInstance,
  const MaterialDef &materialDef,
  ResourceManager *resources
) {
  using PFN = void (*)(PipelineInstance *, const MaterialDef &, ResourceManager *);
  static const Rva s_MaterialDefBarn_SetMaterialShaderUniforms = 0x00DDD6A0;
  return ((PFN)s_MaterialDefBarn_SetMaterialShaderUniforms())(
    pipelineInstance, materialDef, resources);
}

const MaterialDef &MaterialDefBarn::GetDef(
  Material mat
) {
  using PFN = const MaterialDef &(*)(MaterialDefBarn *, Material);
  static const Rva s_MaterialDefBarn_GetDef = 0x00DDD200;
  return ((PFN)s_MaterialDefBarn_GetDef())(this, mat);
}

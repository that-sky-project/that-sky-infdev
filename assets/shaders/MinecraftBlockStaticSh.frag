#version 460

#ifdef VULKAN
  #define UNIFORM(_Set, _Binding) layout (set = _Set, binding = _Binding) uniform
  #define UNIFORM_140(_Set, _Binding) layout (set = _Set, binding = _Binding, std140) uniform
#else
  #define UNIFORM(_Set, _Binding) layout (binding = _Binding) uniform
  #define UNIFORM_140(_Set, _Binding) layout (binding = _Binding, std140) uniform
#endif

// - Uniforms.

UNIFORM_140(1, 0) GlobalShaderUniforms {
  vec4 u_texSizeFull;
  vec4 u_texSizeHalf;
  mediump vec4 u_postParams1;
  mediump vec4 u_postParams2;
  mediump vec4 u_lensDistortion;
  mediump vec4 u_lensDistortion2;
  mat4 u_view;
  mat4 u_proj;
  mat4 u_viewProj;
  mat4 u_viewProjInv;
  mat4 u_viewProjOld;
  mat4 u_viewProjCull;
  mat4 u_camMat;
  vec4 u_camNearPlane;
  vec4 u_lightShadowConsts;
  vec4 u_aaCornerUvs0;
  vec4 u_aaCornerUvs1;
  vec3 u_eyePos;
  float u_unitHeightFrac;
  float u_dt;
  float u_time;
  float u_levelTime;
  float u_averagedt;
  float u_averageLum;
  float u_zNear;
  float u_zFar;
  uint u_cameraCut;
  vec4 u_oceanOffsetHeight;
  mediump vec3 u_grassSpecularTint;
  float u_underwater;
  vec3 u_windVec;
  float u_hudToSceneRatio;
  vec4 u_jitterFullHalf;
  vec4 u_jitterFullHalfOld;
  vec3 u_projPlaneU;
  vec3 u_projPlaneUOld;
  vec3 u_projPlaneV;
  vec3 u_projPlaneVOld;
  vec3 u_projPlaneOrigin;
  vec3 u_projPlaneOriginOld;
  vec4 u_cheapFog;
  vec2 u_cheapFog2;
  mediump vec2 u_sunAccess;
  float u_probeExposure;
  mediump vec3 u_sunDir;
  mediump float u_oceanBlendPow;
  mediump vec4 u_sunSizeIntMoonPhase;
  mediump vec3 u_sunColor;
  float u_atmosphereDensity;
  vec3 u_fogDensityHeightDist;
  mediump float u_averageSkyOcclusion;
  mediump vec3 u_averageSkyColor;
  float u_atmosphereSample;
  mediump vec3 u_averageSkyboxColor;
  vec3 u_atmosphereSunColor;
  vec4 u_averageProbeColor;
  vec4 u_earthPosRad;
  float u_cinemaBrightness;
  float u_galleryChromaTest;
  float u_displayShoulderCutoff;
  vec4 u_beaconPosEnable;
  vec3 u_beaconColor;
  float u_disableMeshMotion;
  vec4 u_portalConst;
  vec4 u_waterSimTerms;
  vec4 u_waterSimTerms2;
  vec4 u_waterSimTerms3;
  mediump vec3 u_windScroll0;
  mediump vec3 u_windScroll1;
  mediump vec3 u_windScroll2;
  mediump vec3 u_pointLight;
  mediump vec4 u_probeShR0;
  mediump vec4 u_probeShR1;
  mediump vec4 u_probeShG0;
  mediump vec4 u_probeShG1;
  mediump vec4 u_probeShB0;
  mediump vec4 u_probeShB1;
  mediump vec3 u_probeShRGB2;
  vec3 u_probeCenter;
  vec4 u_cloudWindMoveNoise;
  vec4 u_cloudWindScrollSpace;
  mediump vec4 u_cloudMat;
  vec4 u_cloudVoxelScales;
  vec3 u_localAvatarPos;
  float u_cloudPackedBinDimInv;
  vec3 u_randomOffset;
  mediump float u_cloudUndulationScale;
  vec3 u_cloudBinMinWorld;
  vec3 u_cloudBinRangeWorld;
  vec3 u_rainMovement;
  vec3 u_minLevelBounds;
  vec3 u_maxLevelBounds;
  vec3 u_levelBounds;
  float u_reflectionPlaneHeight;
  vec4 u_debugTweaks;
} gsu;

UNIFORM(1, 1) mediump sampler2D u_texture;

// - Inputs.

// UV location.
layout (location = 0) in highp vec2 v_uv0;

// - Outputs.

// Output color.
layout (location = 0) out mediump vec4 o_fragColor0;

void main() {
  o_fragColor0 = texture(u_texture, v_uv0);
}

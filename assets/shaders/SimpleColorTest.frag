#version 460

precision mediump int;
precision highp float;

#ifdef VULKAN
  #define UNIFORM(_Set, _Binding) layout (set = _Set, binding = _Binding) uniform
  #define UNIFORM_140(_Set, _Binding) layout (set = _Set, binding = _Binding, std140) uniform
#else
  #define UNIFORM(_Set, _Binding) layout (binding = _Binding) uniform
  #define UNIFORM_140(_Set, _Binding) layout (binding = _Binding, std140) uniform
#endif

// - Uniforms.

// - Inputs.

// - Outputs.

// Output color.
layout (location = 0) out mediump vec4 o_fragColor0;

void main() {
  o_fragColor0 = vec4(0.5, 0.5, 0.5, 1.0);
}

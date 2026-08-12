#!/usr/bin/env node
/*
 * cvt.js — Convert SPIRV-Cross JSON reflection (.refl) to engine binary reflection (.ref, magic=916)
 *
 * Usage:
 *   node cvt.js <stage1.refl> [stage2.refl ...] [-o out.ref] [--no-program]
 *
 * Example:
 *   node cvt.js GrassSh.vk.vert.refl GrassSh.vk.frag.refl -o GrassSh.ref
 *
 * Notes:
 *   - A single .ref file can contain multiple stages (vert/frag/comp); this tool accepts
 *     multiple .refl files, treats each as one stage, and merges them into one .ref.
 *   - Binary format derived from reverse-engineering GrassSh-13f.win.ref
 *     (see ida/result.md, ida/require.md).
 *
 * Precision notes (which fields are confirmed vs. inferred):
 *   [Confirmed] File header, 6 array counts in 12B stage header,
 *               entryPoint/inputs/outputs/textures records, uniform block header+members
 *               (type/arrayCount/offset), ubo record block_size(u16@+32).
 *   [Inferred]  ubo record binding at +30 (0/1/2 in samples matches block order);
 *               set and +31/+35 flag positions unknown, set to 0.
 *               Stage header +7..+11 and per-stage 8B tail cannot be derived from JSON, set to 0.
 *   [Heuristic] Program-level binding table (uniformDescs/samplerDescs) has no direct
 *               counterpart in the “raw” JSON; reconstructed as “all textures + material
 *               uniforms whose name starts with u_mat”; disable with --no-program.
 */

'use strict';
const fs = require('fs');
const path = require('path');

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------
const MAGIC = 916; // 0x0394

// JSON "mode" -> shaderType
const SHADER_TYPE = { vert: 0, frag: 3, pixel: 3, comp: 4, compute: 4 };

// JSON value type -> binary type code (reuse GfxType: Float=1,Float2=2,Float3=3,Float4=4, mat4=35)
const TYPE_CODE = {
  float: 1, vec2: 2, vec3: 3, vec4: 4,
  int: 5, ivec2: 6, ivec3: 7, ivec4: 8,
  uint: 9, uvec2: 10, uvec3: 11, uvec4: 12,
  mat2: 33, mat3: 34, mat4: 35,
};
// Sampler type code (texture record at +30; sample: sampler2D=1)
const SAMPLER_CODE = { sampler2D: 1, sampler3D: 2, samplerCube: 3, sampler2DArray: 4 };

// GfxAttr enum values (see src/sky/skyGfx.hpp). inputs record stores (GfxAttr + 1) at +29.
const GFXATTR = {
  position: 0, color: 1, normal: 2, tangent: 3,
  texcoord0: 4, texcoord1: 5, texcoord2: 6, texcoord3: 7,
  texcoord4: 8, texcoord5: 9, texcoord6: 10, texcoord7: 11,
  uv0: 4, uv1: 5, uv2: 6, uv3: 7, uv4: 8, uv5: 9, uv6: 10, uv7: 11,
  light0: 12, light1: 13, light2: 14, light3: 15,
  noise0: 16, pointsize: 17, center: 18, rotx: 19, roty: 20, rotz: 21, scale: 22,
  userdata0: 23, userdata1: 24, userdata2: 25, userdata3: 26,
  railinfo: 27, railbindend: 28, railbindup: 29, normdepth: 30,
  material0: 31, material1: 32,
};

const warnings = [];
function warn(msg) { warnings.push(msg); }

function typeCode(t) {
  if (t in TYPE_CODE) return TYPE_CODE[t];
  warn(`Unknown type "${t}", set as vec4(4)`);
  return 4;
}
function samplerCode(t) {
  if (t in SAMPLER_CODE) return SAMPLER_CODE[t];
  warn(`Unknown sampler "${t}", set as sampler2D(1)`);
  return 1;
}
// Infer GfxAttr from attribute name (strip a_ prefix, match by convention). Returns -1 for undefined.
function attrOfName(name) {
  let k = String(name).toLowerCase();
  if (k.startsWith('a_')) k = k.slice(2);
  if (k in GFXATTR) return GFXATTR[k];
  // Compatible with a_texcoordN / a_lightN / a_materialN etc.
  const m = k.match(/^(texcoord|uv|light|userdata|material)(\d+)$/);
  if (m) {
    const base = { texcoord: 'texcoord', uv: 'texcoord', light: 'light', userdata: 'userdata', material: 'material' }[m[1]];
    const key = base + m[2];
    if (key in GFXATTR) return GFXATTR[key];
  }
  warn(`Attribute "${name}" cannot map to GfxAttr, attrSemantic set as 0(Undefined)`);
  return -1; // +1 results in 0 = Undefined
}
function arrayCountOf(member) {
  if (Array.isArray(member.array) && member.array.length)
    return member.array.reduce((a, b) => a * (b || 1), 1);
  return 1;
}

// ---------------------------------------------------------------------------
// Byte writer (all little-endian)
// ---------------------------------------------------------------------------
class Writer {
  constructor() { this.b = []; }
  u8(v) { this.b.push(v & 0xff); return this; }
  u16(v) { return this.u8(v).u8(v >>> 8); }
  u32(v) { return this.u16(v & 0xffff).u16(v >>> 16); }
  zeros(n) { for (let i = 0; i < n; i++) this.b.push(0); return this; }
  raw(arr) { for (const x of arr) this.b.push(x & 0xff); return this; }
  // Write name into size-byte zero-padded fixed-length field (null-terminated)
  name(str, size) {
    const buf = Buffer.alloc(size);
    const s = String(str == null ? '' : str);
    const n = buf.write(s, 0, size - 1, 'utf8');
    if (Buffer.byteLength(s, 'utf8') > size - 1)
      warn(`Name "${s}" exceeds the ${size - 1} byte field and has been truncated`);
    for (const x of buf) this.b.push(x);
    return this;
  }
  get length() { return this.b.length; }
  toBuffer() { return Buffer.from(this.b); }
}

// ---------------------------------------------------------------------------
// Single stage serialization
// ---------------------------------------------------------------------------
function encodeStage(refl) {
  const w = new Writer();

  const mode = (refl.entryPoints && refl.entryPoints[0] && refl.entryPoints[0].mode) || 'vert';
  const entryName = (refl.entryPoints && refl.entryPoints[0] && refl.entryPoints[0].name) || 'main';
  if (!(mode in SHADER_TYPE)) warn(`Unknown mode "${mode}", shaderType set to 0(vert)`);
  const shaderType = SHADER_TYPE[mode] ?? 0;

  const types = refl.types || {};          // Array 1: uniformBlocks (including gl_PerVertex)
  const inputs = refl.inputs || [];        // Array 2
  const outputs = refl.outputs || [];      // Array 3
  const textures = refl.textures || [];    // Array 4
  const ubos = refl.ubos || [];            // Array 5: blockBindings

  const typeKeys = Object.keys(types);

  // --- Stage header (12B): type + 6 counts + 5 padding (zeroed) ---
  w.u8(shaderType);
  w.u8(1);                 // cnt1 entryPoint always 1
  w.u8(typeKeys.length);   // cnt2 uniformBlocks
  w.u8(inputs.length);     // cnt3 inputs
  w.u8(outputs.length);    // cnt4 outputs
  w.u8(textures.length);   // cnt5 textures
  w.u8(ubos.length);       // cnt6 blockBindings (ubos)
  w.u8(shaderType == SHADER_TYPE.vert ? 8 : 0); // cnt7 maxBindings
  w.zeros(4);              // +8..+11 (cannot be derived from JSON)

  // --- Array 0 entryPoint (32B x 1) ---
  w.name(entryName, 32);

  // --- Array 1 uniformBlocks: each block = 32B header (name30 + memberCount u16) + 36B per member ---
  for (const key of typeKeys) {
    const blk = types[key];
    const members = blk.members || [];
    w.name(blk.name || key, 30);
    w.u16(members.length);
    for (const m of members) {
      w.name(m.name, 28);
      w.u16(typeCode(m.type));       // +28
      w.u16(arrayCountOf(m));        // +30
      w.u16((m.offset | 0) & 0xffff);// +32
      w.u8(0);                       // +34
      w.u8(0xff);                    // +35 sentinel constant
    }
  }

  // --- Array 2 inputs (32B): name28 + type + (GfxAttr+1) + location + 0 ---
  for (const v of inputs) {
    w.name(v.name, 28);
    w.u8(typeCode(v.type));          // +28
    w.u8((attrOfName(v.name) + 1) & 0xff); // +29 = GfxAttr+1
    w.u8((v.location | 0) & 0xff);   // +30
    w.u8(0);                         // +31
  }

  // --- Array 3 outputs (32B): name28 + 0 + 0 + type + location ---
  for (const v of outputs) {
    w.name(v.name, 28);
    w.u8(0);                         // +28
    w.u8(0);                         // +29
    w.u8(typeCode(v.type));          // +30
    w.u8((v.location | 0) & 0xff);   // +31
  }

  // --- Array 4 textures (32B): name30 + samplerType + binding ---
  for (const t of textures) {
    w.name(t.name, 30);
    w.u8(samplerCode(t.type));       // +30
    w.u8((t.binding | 0) & 0xff);    // +31 (set position unknown, not encoded)
  }

  // --- Array 5 blockBindings/ubos (36B): name30 + binding + 0 + block_size(u16) + 0 + 0 ---
  for (const u of ubos) {
    var flags = 0
      , blockSize = (u.block_size | 0) & 0xfffffff;

    if (u.name === "PerFrameUniforms")
      flags |= 0x40000000;
    else if (u.name === "PerPassUniforms")
      flags |= 0x80000000;

    w.name(u.name, 30);
    w.u8(typeKeys.indexOf(u.type));  // +30 (type index)
    w.u8((u.binding | 0) & 0xff);    // +31 (binding index)
    w.u32(blockSize | flags);        // +32 (block size & flags)

    // Actually only bit [31:28] of the above DWORD is considered as flags. Lower bits are
    // considered as size of the uniform buffer.
    // 0x10000000: isStorageBuffer, marked as VK_DESCRIPTOR_TYPE_STORAGE_BUFFER if true,
    //             or VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER if false.
    // 0x20000000: isReadonly, marked as read-only if true.
    // 0x40000000: isGlobalUniformBuffer, look up the buffer in global shader uniform buffer
    //             list if true.
    // 0x80000000: isExternalBuffer, marked as external (manually) bound if true.
  }

  // The remapping table from vertex buffer slot index (used in `vkCmdBindVertexBuffers`)
  // to Vulkan binding number (`VkVertexInputAttributeDescription.binding`).
  // Usually set to 00 01 03 04 05 06 07 08
  w.u8(0); w.u8(1); w.u8(3); w.u8(4);
  w.u8(5); w.u8(6); w.u8(7); w.u8(8);

  return { buffer: w.toBuffer(), shaderType, mode, textures, ubos, types };
}

// ---------------------------------------------------------------------------
// Program-level binding table (heuristic reconstruction; use --no-program to disable)
//   samplerDescs = deduplicated textures across all stages
//   uniformDescs = deduplicated material uniforms whose names start with u_mat
// ---------------------------------------------------------------------------
function encodeProgramTable(stages) {
  const uni = [];   // {name, type, binding}
  const samp = [];  // {name, binding}
  const uSeen = new Set(), sSeen = new Set();

  for (const st of stages) {
    for (const t of st.textures) {
      if (sSeen.has(t.name)) continue;
      sSeen.add(t.name);
      samp.push({ name: t.name, binding: t.binding | 0 });
    }
    // Material uniforms: iterate type members referenced by each UBO in this stage
    for (const u of st.ubos) {
      const blk = st.types[u.type] || Object.values(st.types).find(b => b.name === u.name);
      if (!blk) continue;
      for (const m of (blk.members || [])) {
        if (!/^u_mat/i.test(m.name) || uSeen.has(m.name)) continue;
        uSeen.add(m.name);
        // type 0x10=vertex-visible / 0x13=fragment-visible (rough split by first-occurring stage)
        uni.push({ name: m.name, type: st.shaderType === 0 ? 0x10 : 0x13, binding: u.binding | 0 });
      }
    }
  }

  const w = new Writer();
  // Each 48B: name32 + type + index + slot(u16, =binding<<4) + 6B0 + arrayCount(u32)=1
  uni.forEach((u, i) => {
    w.name(u.name, 32);
    w.u8(u.type); w.u8(i); w.u16((u.binding << 4) & 0xffff);
    w.zeros(6); w.u32(1);
  });
  samp.forEach((s, i) => {
    w.name(s.name, 32);
    w.u8(0x13); w.u8(i); w.u16((s.binding << 4) & 0xffff);
    w.zeros(6); w.u32(1);
  });

  return { buffer: w.toBuffer(), uniformCount: uni.length, samplerCount: samp.length };
}

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------
function main(argv) {
  const inputs = [];
  let out = null, withProgram = true;
  for (let i = 0; i < argv.length; i++) {
    const a = argv[i];
    if (a === '-o' || a === '--out') out = argv[++i];
    else if (a === '--no-program') withProgram = false;
    else if (a === '-h' || a === '--help') { printUsage(); return 0; }
    else inputs.push(a);
  }
  if (!inputs.length) { printUsage(); return 1; }

  // Read and sort by shaderType (vert, frag, comp)
  const stages = inputs.map(f => {
    const json = JSON.parse(fs.readFileSync(f, 'utf8'));
    return encodeStage(json);
  }).sort((a, b) => a.shaderType - b.shaderType);

  const prog = withProgram ? encodeProgramTable(stages)
    : { buffer: Buffer.alloc(0), uniformCount: 0, samplerCount: 0 };

  // File header (16B)
  const head = new Writer();
  head.u32(MAGIC);
  head.u32(stages.length);
  head.u32(prog.uniformCount);
  head.u32(prog.samplerCount);

  const parts = [head.toBuffer(), ...stages.map(s => s.buffer), prog.buffer];
  const outBuf = Buffer.concat(parts);

  if (!out) {
    const base = path.basename(inputs[0]).split('.')[0] || 'out';
    out = path.join(path.dirname(inputs[0]), base + '.ref');
  }
  fs.writeFileSync(out, outBuf);

  // Summary
  console.log(`Write for ${out} (${outBuf.length}) Bytes`);
  stages.forEach(s => console.log(`  stage: ${s.mode.padEnd(5)} shaderType=${s.shaderType}  ${s.buffer.length}B`));
  console.log(`  Programs: uniform=${prog.uniformCount} sampler=${prog.samplerCount}  ${prog.buffer.length}B`);
  if (warnings.length) {
    console.log(`[WARN] (${warnings.length}):`);
    for (const wmsg of [...new Set(warnings)]) console.log('  - ' + wmsg);
  }
  return 0;
}

function printUsage() {
  console.log('Usage: node cvt.js <stage1.refl> [stage2.refl ...] [-o out.ref] [--no-program]');
}

process.exit(main(process.argv.slice(2)));

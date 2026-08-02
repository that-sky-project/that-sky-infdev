const fs = require("fs")
  , pl = require("path")
  , cp = require("child_process");

const DIST_DIR = "./dist";

const ASSETS_DIR = "./assets";
const ASSETS_DIST = pl.join(DIST_DIR, "assets");

const SHADER_FLAGS = "-V -g";
const SHADER_DIR = pl.join(ASSETS_DIR, "shaders");
const SHADER_DIST = pl.join(ASSETS_DIST, "shaders");

const PLATFORM = "win";

// Vulkan target configs: each produces a separate output variant
const VK_TARGETS = [
  { env: "vulkan1.3", ver: "13f" },
  { env: "vulkan1.2", ver: "12f" },
];

// Map GLSL file extension -> SPIR-V shader stage suffix
const STAGE_MAP = {
  ".vert": "vs",
  ".frag": "fs",
  ".comp": "cs"
};

const CVT_SCRIPT = pl.join("scripts", "shader-refl.js");

// ---------------------------------------------------------------------------
// Phase 1: GLSL → SPIR-V + reflection JSON
// ---------------------------------------------------------------------------

/**
 * Compile a single shader source for a given Vulkan target, then generate
 * reflection JSON via spirv-cross.
 *
 * Returns { spvPath, reflPath, baseName, vkVer } so downstream steps can
 * group stages by (baseName, vkVer) and produce binary .ref files.
 */
function buildShader(srcPath, vkTarget) {
  const fileName = pl.basename(srcPath);
  const ext = pl.extname(fileName);
  const baseName = pl.basename(fileName, ext);
  const stage = STAGE_MAP[ext];

  if (!stage) {
    console.warn(`[WARN] Unknown shader stage "${ext}" — skipping ${fileName}`);
    return null;
  }

  const outBase = `${baseName}-${vkTarget.ver}.${PLATFORM}.${stage}`;
  const spvPath = pl.join(SHADER_DIST, `${outBase}.spv`);
  const reflPath = pl.join(SHADER_DIST, `${outBase}.refl.json`);

  // GLSL → SPIR-V
  const compileCmd =
    `glslangValidator ${SHADER_FLAGS} --target-env ${vkTarget.env} "${srcPath}" -o "${spvPath}"`;
  console.log(`[BUILD] ${fileName}  ->  ${outBase}.spv  (${vkTarget.env})`);
  cp.execSync(compileCmd, { stdio: "inherit" });

  // SPIR-V → reflection JSON
  const reflCmd = `spirv-cross "${spvPath}" --reflect`;
  console.log(`[REFL]  ${outBase}.spv  ->  ${outBase}.refl.json`);
  const reflJson = cp.execSync(reflCmd, { encoding: "utf-8" });
  fs.writeFileSync(reflPath, reflJson, "utf-8");

  return { spvPath, reflPath, baseName, vkVer: vkTarget.ver };
}

// ---------------------------------------------------------------------------
// Phase 2: reflection JSON → binary .ref
// ---------------------------------------------------------------------------

/**
 * Group refl.json paths by (baseName, vkVer) and invoke scripts/shader-refl.js
 * for each group to produce a single merged binary .ref file.
 */
function buildBinaryReflections(reflEntries) {
  // Group: key = "baseName|vkVer"
  const groups = new Map();
  for (const entry of reflEntries) {
    if (!entry) continue;
    const key = `${entry.baseName}|${entry.vkVer}`;
    if (!groups.has(key)) groups.set(key, []);
    groups.get(key).push(entry.reflPath);
  }

  if (groups.size === 0) {
    console.log("No reflection JSON files to convert.");
    return;
  }

  console.log(`\n--- Binary reflection (.ref) ---`);
  for (const [key, reflPaths] of groups) {
    const [baseName, vkVer] = key.split("|");
    const refName = `${baseName}-${vkVer}.${PLATFORM}.ref`;
    const refPath = pl.join(SHADER_DIST, refName);

    // Sort so vert comes before frag (matching shaderType order in CVT)
    reflPaths.sort();

    const args = [...reflPaths, "-o", refPath];
    console.log(`[CVT]   ${reflPaths.map(p => pl.basename(p)).join(" + ")}  ->  ${refName}`);
    cp.execSync(`node "${CVT_SCRIPT}" ${args.map(a => `"${a}"`).join(" ")}`, { stdio: "inherit" });
  }
}

// ---------------------------------------------------------------------------
// Phase 3: cleanup intermediate JSON reflections
// ---------------------------------------------------------------------------

function cleanReflJson(reflEntries) {
  console.log(`\n--- Cleaning up .refl.json ---`);
  for (const entry of reflEntries) {
    if (!entry) continue;
    try {
      fs.unlinkSync(entry.reflPath);
      console.log(`[RM]    ${pl.basename(entry.reflPath)}`);
    } catch (_) { /* already gone */ }
  }
}

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------

function buildAllShaders() {
  fs.mkdirSync(SHADER_DIST, { recursive: true });

  const entries = fs.readdirSync(SHADER_DIR, { withFileTypes: true });
  const shaderFiles = entries
    .filter(e => e.isFile() && STAGE_MAP[pl.extname(e.name)])
    .map(e => pl.join(SHADER_DIR, e.name));

  if (shaderFiles.length === 0) {
    console.log("No shader files found under", SHADER_DIR);
    return;
  }

  console.log(`Found ${shaderFiles.length} shader file(s):`);
  shaderFiles.forEach(f => console.log(`  ${pl.relative(ASSETS_DIR, f)}`));
  console.log(`Targets: ${VK_TARGETS.map(v => v.env).join(", ")}\n`);

  // Phase 1: compile & reflect
  const reflEntries = [];
  for (const src of shaderFiles) {
    for (const vk of VK_TARGETS) {
      reflEntries.push(buildShader(src, vk));
    }
  }

  // Phase 2: JSON → binary .ref
  buildBinaryReflections(reflEntries);

  // Phase 3: delete intermediate .refl.json
  cleanReflJson(reflEntries);

  console.log("\nAll shaders built.");
}

buildAllShaders();

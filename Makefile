# Requires GNU Make with a POSIX-compatible shell (Git Bash / MSYS2).

DIST       := dist/Data
SHADER_SRC := assets/shaders
SHADER_BIN := $(DIST)/Shaders/Bin
LEVEL_SRC  := assets/levels/Infdev_INF
LEVEL_OUT  := $(DIST)/Levels/Infdev_INF

SHADERS    := EndPortal SimpleColorTest

.PHONY: all level shaders clean

all: level shaders

# ---------------------------------------------------------------------------
# Level
# ---------------------------------------------------------------------------

level: $(LEVEL_OUT)/Objects.level.bin $(LEVEL_OUT)/BstBaked.meshes $(LEVEL_OUT)/Resources.lua

$(LEVEL_OUT)/Objects.level.bin: \
		$(LEVEL_SRC)/Infdev_INF.level.json \
		$(LEVEL_SRC)/Infdev_INF.decl.json \
		| $(LEVEL_OUT)
	node scripts/level.js $^ $@

$(LEVEL_OUT)/BstBaked.meshes: $(LEVEL_SRC)/Infdev_INF.obj | $(LEVEL_OUT)
	node node_modules/@that-sky-project/that-sky-level/dist/cli.js -C -i $< -o $@

$(LEVEL_OUT)/Resources.lua: $(LEVEL_SRC)/Resources.lua | $(LEVEL_OUT)
	cp $< $@

# ---------------------------------------------------------------------------
# Shaders
# ---------------------------------------------------------------------------

SHADER_REFS := $(foreach s,$(SHADERS), \
  $(SHADER_BIN)/$(s)-12f.win.ref \
  $(SHADER_BIN)/$(s)-13f.win.ref \
  $(SHADER_BIN)/$(s)-13h.win.ref)

shaders: $(SHADER_REFS)

# Mark reflection JSONs as intermediate so Make removes them after use.
.INTERMEDIATE: $(foreach s,$(SHADERS), \
  $(SHADER_BIN)/$(s)-12f.win.vs.refl.json $(SHADER_BIN)/$(s)-12f.win.fs.refl.json \
  $(SHADER_BIN)/$(s)-13f.win.vs.refl.json $(SHADER_BIN)/$(s)-13f.win.fs.refl.json \
  $(SHADER_BIN)/$(s)-13h.win.vs.refl.json $(SHADER_BIN)/$(s)-13h.win.fs.refl.json)

# Macro: emit rules for one shader × one variant.
# $1 = shader name   $2 = variant suffix (12f / 13f / 13h)   $3 = vulkan target
define SHADER_RULES =
$(SHADER_BIN)/$1-$2.win.vs.spv: $(SHADER_SRC)/$1.vert | $(SHADER_BIN)
	glslc -g --target-env=$3 $$< -o $$@

$(SHADER_BIN)/$1-$2.win.fs.spv: $(SHADER_SRC)/$1.frag | $(SHADER_BIN)
	glslc -g --target-env=$3 $$< -o $$@

$(SHADER_BIN)/$1-$2.win.vs.refl.json: $(SHADER_BIN)/$1-$2.win.vs.spv
	spirv-cross $$< --reflect > $$@

$(SHADER_BIN)/$1-$2.win.fs.refl.json: $(SHADER_BIN)/$1-$2.win.fs.spv
	spirv-cross $$< --reflect > $$@

$(SHADER_BIN)/$1-$2.win.ref: \
		$(SHADER_BIN)/$1-$2.win.fs.refl.json \
		$(SHADER_BIN)/$1-$2.win.vs.refl.json
	node scripts/shader-refl.js $$^ -o $$@
endef

$(foreach s,$(SHADERS),$(eval $(call SHADER_RULES,$(s),12f,vulkan1.2)))
$(foreach s,$(SHADERS),$(eval $(call SHADER_RULES,$(s),13f,vulkan1.3)))
$(foreach s,$(SHADERS),$(eval $(call SHADER_RULES,$(s),13h,vulkan1.3)))

# ---------------------------------------------------------------------------
# Directories (order-only prerequisites)
# ---------------------------------------------------------------------------

$(LEVEL_OUT) $(SHADER_BIN):
	mkdir -p $@

# ---------------------------------------------------------------------------
# Clean
# ---------------------------------------------------------------------------

clean:
	rm -rf $(DIST)

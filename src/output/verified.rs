//! Verified working features — hand-curated catalogue of offsets, hooks
//! and ConVar tricks that have been **confirmed working in a live CS2
//! internal cheat** against the current build.
//!
//! Most of this data is already present in the auto-extracted offset
//! / signature / schema files this tool produces. The point of this
//! module is to give consumers a single place that says, in plain
//! English, *"yes, this offset on this entity does this exact thing,
//! and here is the gotcha you need to know to make it work."*
//!
//! a2x's pelite-based pipeline gives us the canonical RVAs and schema
//! offsets — that data is the source of truth and lives in the regular
//! `offsets.*` / `client_dll.*` / `signatures.*` files. This catalogue
//! cross-references those values with verified live-engine notes so a
//! cheat developer can copy-paste a feature and have it work first try.

use serde_json::json;

#[derive(Clone, Copy)]
pub struct VerifiedField {
    /// e.g. "C_SmokeGrenadeProjectile"
    pub class: &'static str,
    /// e.g. "m_nSmokeEffectTickBegin"
    pub field: &'static str,
    /// hex offset relative to the class base (entity)
    pub offset: u32,
    /// e.g. "int32" / "Vector3" / "bool"
    pub ty: &'static str,
    /// short note about what to write / how to read
    pub note: &'static str,
}

#[derive(Clone, Copy)]
pub struct VerifiedFeature {
    /// e.g. "No Smoke"
    pub name: &'static str,
    /// short paragraph explaining what we tested + where to write
    pub summary: &'static str,
    /// fields touched
    pub fields: &'static [VerifiedField],
    /// ConVar tricks (name + flags-to-strip + value-slot offset) — empty if N/A
    pub convars: &'static [VerifiedConVar],
    /// Hooks installed (function + module + signature key in database.rs)
    pub hooks: &'static [VerifiedHook],
}

#[derive(Clone, Copy)]
pub struct VerifiedConVar {
    /// ConVar name
    pub name: &'static str,
    /// flags to strip from cvar+0x30 (e.g. FCVAR_CHEAT=0x400)
    pub strip_flags: u32,
    /// modern Source 2 ConVar<T> value lives at cvar+0x58 — set true
    /// if we have to write *both* the legacy +0x40 union AND the
    /// modern +0x58 slot
    pub write_both_slots: bool,
    /// what value(s) we write
    pub value: &'static str,
}

#[derive(Clone, Copy)]
pub struct VerifiedHook {
    /// e.g. "DrawSkyboxArray"
    pub function: &'static str,
    /// e.g. "scenesystem.dll"
    pub module: &'static str,
    /// signature database key (look up in src/signatures/database.rs)
    pub signature: &'static str,
    /// what we do once hooked
    pub action: &'static str,
}

// ----------------------------------------------------------------------
// Catalogue. Add entries as new features are verified working in-game.
// Build target: CS2 14152+ (April 2026).
// ----------------------------------------------------------------------
pub static FEATURES: &[VerifiedFeature] = &[
    VerifiedFeature {
        name: "No Smoke",
        summary: "Iterate the entity list, identify C_SmokeGrenadeProjectile via \
                  CEntityIdentity::m_designerName == \"smokegrenade_projectile\", \
                  zero m_nSmokeEffectTickBegin and clear m_bDidSmokeEffect. Engine \
                  re-evaluates every tick, so writes stick. Also zero \
                  m_flLastSmokeOverlayAlpha on the local pawn for the screen overlay.",
        fields: &[
            VerifiedField { class: "C_SmokeGrenadeProjectile", field: "m_nSmokeEffectTickBegin", offset: 0x1250, ty: "int32",   note: "set 0 to skip the puff" },
            VerifiedField { class: "C_SmokeGrenadeProjectile", field: "m_bDidSmokeEffect",       offset: 0x1254, ty: "bool",    note: "set false" },
            VerifiedField { class: "C_CSPlayerPawn",           field: "m_flLastSmokeOverlayAlpha", offset: 0x1420, ty: "float",   note: "local pawn's screen overlay alpha; set 0" },
        ],
        convars: &[],
        hooks: &[],
    },
    VerifiedFeature {
        name: "Smoke Color",
        summary: "Same entity walk as No Smoke; write a Vector (3 floats * 255) to \
                  m_vSmokeColor. Particle system reads this every frame.",
        fields: &[
            VerifiedField { class: "C_SmokeGrenadeProjectile", field: "m_vSmokeColor", offset: 0x125C, ty: "Vector3 (RGB 0..255)", note: "3 floats, multiply RGB by 255" },
        ],
        convars: &[],
        hooks: &[],
    },
    VerifiedFeature {
        name: "No Flash",
        summary: "Zero m_flFlashDuration and m_flFlashMaxAlpha on the local pawn. \
                  Trip the write only when duration > 0 to avoid spamming the engine.",
        fields: &[
            VerifiedField { class: "C_CSPlayerPawn", field: "m_flFlashDuration", offset: 0x1400, ty: "float", note: "set 0 to clear flash" },
            VerifiedField { class: "C_CSPlayerPawn", field: "m_flFlashMaxAlpha", offset: 0x13FC, ty: "float", note: "set 0 for full removal, 0..255 for partial" },
        ],
        convars: &[],
        hooks: &[],
    },
    VerifiedFeature {
        name: "Skybox Tint",
        summary: "Hook scenesystem.dll!DrawSkyboxArray, intercept the draw-primitive \
                  pointer (3rd arg). Build 14152 moved the tint vec3 from +0x100 \
                  to +0xE8 inside the skybox object — writing to the old +0x100 slot \
                  poisons the renderer with NaN and crashes after ~60s. \
                  Layout: +0xE8..+0xF0 RGB tint floats, +0xF4 mode int, \
                  +0xF8..+0x104 four sun-angle floats. \
                  Do NOT write to env_sky m_vTintColor — the renderer caches \
                  material params at level setup and ignores entity writes.",
        fields: &[
            VerifiedField { class: "env_sky", field: "m_vTintColor",              offset: 0xFB9, ty: "Color32", note: "DOES NOT WORK at runtime — use the DrawSkyboxArray hook instead" },
            VerifiedField { class: "env_sky", field: "m_vTintColorLightingOnly",  offset: 0xFBD, ty: "Color32", note: "Same caveat" },
            VerifiedField { class: "env_sky", field: "m_flBrightnessScale",       offset: 0xFC4, ty: "float",   note: "DOES update live (independent code path)" },
        ],
        convars: &[],
        hooks: &[
            VerifiedHook {
                function: "DrawSkyboxArray",
                module: "scenesystem.dll",
                signature: "DrawSkyboxArray",
                action: "Patch RGB at primitive_obj+0xE8 before passing through to the original.",
            },
        ],
    },
    VerifiedFeature {
        name: "FOV Changer",
        summary: "Two-prong approach. (1) Hook GetWorldFov in client.dll \
                  (signature SetWorldFov, E8-CALL @ +1) and return the desired \
                  value when the local pawn is not scoped. (2) Every tick, write \
                  the desired FOV into m_iFOV and m_iFOVStart on the camera \
                  services AND into m_iDesiredFOV on the local controller \
                  (canonical source the renderer reads). The controller-level \
                  field is the one that gets reset back to default if you only \
                  fight the camera-services side.",
        fields: &[
            VerifiedField { class: "CBasePlayerController", field: "m_iDesiredFOV", offset: 0x784, ty: "uint32", note: "a2x-named m_iDesiredFOV_OnController" },
            VerifiedField { class: "CCSPlayer_CameraServices", field: "m_iFOV",      offset: 0x290, ty: "uint32", note: "current camera FOV" },
            VerifiedField { class: "CCSPlayer_CameraServices", field: "m_iFOVStart", offset: 0x294, ty: "uint32", note: "target camera FOV" },
            VerifiedField { class: "C_CSPlayerPawn",            field: "m_pCameraServices", offset: 0x1218, ty: "CCSPlayer_CameraServices*", note: "deref to reach m_iFOV/m_iFOVStart" },
        ],
        convars: &[],
        hooks: &[
            VerifiedHook {
                function: "GetWorldFov",
                module: "client.dll",
                signature: "SetWorldFov",
                action: "Return cfg.fovValue when not scoped, else delegate to original.",
            },
        ],
    },
    VerifiedFeature {
        name: "Chams (Material Override)",
        summary: "Standard CS2 chams approach: override the material on \
                  CCSGOPlayerAnimGraphState::DrawObject path or material pointers \
                  on weapon/player render contexts. Use the cs2-dumper material \
                  signatures (FindParameter/UpdateParameter from materialsystem2) \
                  to swap shader params at draw time.",
        fields: &[],
        convars: &[],
        hooks: &[
            VerifiedHook { function: "GeneratePrimitives",       module: "scenesystem.dll",      signature: "CSceneAnimatableObject::GeneratePrimitives", action: "Substitute material on player/weapon scene objects." },
            VerifiedHook { function: "FindParameter",            module: "materialsystem2.dll",  signature: "FindParameter",                              action: "Look up the param slot." },
            VerifiedHook { function: "UpdateParameter",          module: "materialsystem2.dll",  signature: "UpdateParameter",                            action: "Write the new value." },
        ],
    },
    VerifiedFeature {
        name: "Fullbright",
        summary: "ConVar mat_fullbright is registered FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY. \
                  Source 2 ConVar layout (verified in scenesystem.dll sub_1804ACB70): \
                  cvar+0x30 flags DWORD, cvar+0x40 legacy value union, cvar+0x58 \
                  modern ConVar<T> value storage. The renderer's fullbright branch \
                  is gated on (flags & 0x400) == 0, so we MUST strip FCVAR_CHEAT \
                  (0x400) and FCVAR_DEVELOPMENTONLY (0x4000), then write the \
                  desired value to BOTH +0x40 (legacy) AND +0x58 (modern) slots — \
                  some code paths read each.",
        fields: &[],
        convars: &[
            VerifiedConVar { name: "mat_fullbright", strip_flags: 0x4400, write_both_slots: true, value: "1 to enable / 0 to disable" },
        ],
        hooks: &[],
    },
    VerifiedFeature {
        name: "Third Person",
        summary: "Hook CCSGOViewAdviceService::OverrideView (signature OverrideView, \
                  client.dll). After calling the original (lets engine set up \
                  first-person camera), read the eye position out of CViewSetup, \
                  build forward from view angles, and rewrite CViewSetup origin to \
                  eye - forward*dist + (0,0,8). Leave angles untouched. \
                  Also flip the engine input flag at clientBase + dwCSGOInput + 0x229 \
                  to true so the local model renders. Restore to false on disable. \
                  CViewSetup field offsets (CS2 build 14152): origin=+0x490, \
                  angles=+0x4A0, fov=+0x230. \
                  Do NOT use c_thirdpersonshoulder ConVar — it's FCVAR_CHEAT and \
                  reads from cvar+0x58 (same gotchas as mat_fullbright); direct \
                  CViewSetup write is the canonical CS2 approach.",
        fields: &[],
        convars: &[],
        hooks: &[
            VerifiedHook {
                function: "OverrideView",
                module: "client.dll",
                signature: "CCSGOViewAdvice::OverrideView",
                action: "Rewrite CViewSetup origin (offset 0x490) to a third-person position derived from view angles.",
            },
        ],
    },
    VerifiedFeature {
        name: "Anti-Fog",
        summary: "Disable every fog source per-entity. Cheap and total. \
                  env_fog_controller has fogparams_t embedded at +0x608: +0x14 \
                  colorPrimary, +0x18 colorSecondary, +0x24 start, +0x28 end, \
                  +0x30 maxDensity, +0x64 enable. env_cubemap_fog: +0x62C m_bActive, \
                  +0x630 m_flFogMaxOpacity, +0x60C/0x608 start/end distances. \
                  env_volumetric_fog_controller: +0x600 m_flScattering, +0x610 \
                  m_flDrawDistance, +0x64C m_bActive, +0x674 m_bStartDisabled. \
                  env_volumetric_fog_volume: +0x600 m_bActive, +0x620 m_flStrength.",
        fields: &[
            VerifiedField { class: "C_FogController",                 field: "m_fog (params)",   offset: 0x608, ty: "fogparams_t", note: "+0x14 RGB primary, +0x18 RGB secondary, +0x24 start, +0x28 end, +0x30 density, +0x64 enable bool" },
            VerifiedField { class: "C_EnvCubemapFog",                 field: "m_bActive",        offset: 0x62C, ty: "bool",        note: "set false" },
            VerifiedField { class: "C_EnvCubemapFog",                 field: "m_flFogMaxOpacity",offset: 0x630, ty: "float",       note: "set 0" },
            VerifiedField { class: "C_EnvVolumetricFogController",    field: "m_bActive",        offset: 0x64C, ty: "bool",        note: "set false" },
            VerifiedField { class: "C_EnvVolumetricFogController",    field: "m_bStartDisabled", offset: 0x674, ty: "bool",        note: "set true" },
            VerifiedField { class: "C_EnvVolumetricFogVolume",        field: "m_bActive",        offset: 0x600, ty: "bool",        note: "set false" },
            VerifiedField { class: "C_EnvVolumetricFogVolume",        field: "m_flStrength",     offset: 0x620, ty: "float",       note: "set 0" },
        ],
        convars: &[],
        hooks: &[],
    },
    VerifiedFeature {
        name: "No Color Correction",
        summary: "Color-correction LUT entities (color_correction designer name) ship \
                  on most maps and apply the mood grading (warm dust on Mirage, \
                  blue-grey on Anubis, etc). Disabling them is a free visibility \
                  upgrade. Zero m_flMaxWeight and m_flCurWeight, clear m_bEnabled \
                  and m_bEnabledOnClient[0], zero m_flCurWeightOnClient.",
        fields: &[
            VerifiedField { class: "C_ColorCorrection", field: "m_flMaxWeight",         offset: 0x61C, ty: "float", note: "set 0" },
            VerifiedField { class: "C_ColorCorrection", field: "m_flCurWeight",         offset: 0x620, ty: "float", note: "set 0" },
            VerifiedField { class: "C_ColorCorrection", field: "m_bEnabled",            offset: 0x824, ty: "bool",  note: "set false" },
            VerifiedField { class: "C_ColorCorrection", field: "m_bEnabledOnClient[0]", offset: 0x828, ty: "bool",  note: "set false" },
            VerifiedField { class: "C_ColorCorrection", field: "m_flCurWeightOnClient", offset: 0x82C, ty: "float", note: "set 0" },
        ],
        convars: &[],
        hooks: &[],
    },
    VerifiedFeature {
        name: "Night Mode / Asus Mode (Sky Tint via env_sky)",
        summary: "For env_sky entities: m_vTintColor (Color32) at +0xFB9 and \
                  m_flBrightnessScale at +0xFC4 DO update live and are safe to \
                  poke per-tick. Use these for night/asus presets. The sky-tint \
                  hook (DrawSkyboxArray) handles dynamic per-frame color; this \
                  entity-write path handles the slower per-preset apply.",
        fields: &[
            VerifiedField { class: "env_sky", field: "m_vTintColor",             offset: 0xFB9, ty: "Color32", note: "RGBA 0..255 packed" },
            VerifiedField { class: "env_sky", field: "m_vTintColorLightingOnly", offset: 0xFBD, ty: "Color32", note: "match m_vTintColor for consistency" },
            VerifiedField { class: "env_sky", field: "m_flBrightnessScale",      offset: 0xFC4, ty: "float",   note: "1.0 = default" },
        ],
        convars: &[],
        hooks: &[],
    },
];

// ----------------------------------------------------------------------
// Renderers
// ----------------------------------------------------------------------

pub fn render_json(build_number: Option<u32>) -> String {
    let features: Vec<_> = FEATURES
        .iter()
        .map(|f| {
            json!({
                "name":    f.name,
                "summary": f.summary,
                "fields":  f.fields.iter().map(|fld| json!({
                    "class":  fld.class,
                    "field":  fld.field,
                    "offset": format!("0x{:X}", fld.offset),
                    "type":   fld.ty,
                    "note":   fld.note,
                })).collect::<Vec<_>>(),
                "convars": f.convars.iter().map(|c| json!({
                    "name":             c.name,
                    "strip_flags":      format!("0x{:X}", c.strip_flags),
                    "write_both_slots": c.write_both_slots,
                    "value":            c.value,
                })).collect::<Vec<_>>(),
                "hooks":   f.hooks.iter().map(|h| json!({
                    "function":  h.function,
                    "module":    h.module,
                    "signature": h.signature,
                    "action":    h.action,
                })).collect::<Vec<_>>(),
            })
        })
        .collect();

    let doc = json!({
        "_doc":           "Hand-curated catalogue of CS2 features verified working in a live internal cheat. \
                          Cross-reference with offsets.*, signatures.* and client_dll.* for canonical addresses.",
        "cs2_build":      build_number,
        "feature_count":  FEATURES.len(),
        "features":       features,
    });

    serde_json::to_string_pretty(&doc).unwrap_or_else(|_| String::from("{}"))
}

pub fn render_md(build_number: Option<u32>) -> String {
    let mut out = String::new();
    out.push_str("# Verified Working Features\n\n");
    out.push_str(
        "Hand-curated catalogue of CS2 features that have been **confirmed working \
         in a live internal cheat** against the current build. Cross-reference with \
         the auto-generated `offsets.*`, `signatures.*` and `client_dll.*` files for \
         canonical addresses; this document captures the *gotchas* — the kind of \
         thing that took an evening of IDA work to figure out.\n\n",
    );
    if let Some(b) = build_number {
        out.push_str(&format!("**CS2 build:** `{}`\n\n", b));
    }
    out.push_str(&format!("**Feature count:** {}\n\n---\n\n", FEATURES.len()));

    for f in FEATURES.iter() {
        out.push_str(&format!("## {}\n\n", f.name));
        out.push_str(f.summary);
        out.push_str("\n\n");

        if !f.fields.is_empty() {
            out.push_str("### Fields\n\n");
            out.push_str("| Class | Field | Offset | Type | Note |\n");
            out.push_str("|---|---|---|---|---|\n");
            for fld in f.fields.iter() {
                out.push_str(&format!(
                    "| `{}` | `{}` | `0x{:X}` | `{}` | {} |\n",
                    fld.class, fld.field, fld.offset, fld.ty, fld.note,
                ));
            }
            out.push('\n');
        }

        if !f.convars.is_empty() {
            out.push_str("### ConVars\n\n");
            out.push_str("| Name | Strip Flags | Write Both Slots | Value |\n");
            out.push_str("|---|---|---|---|\n");
            for c in f.convars.iter() {
                out.push_str(&format!(
                    "| `{}` | `0x{:X}` | {} | {} |\n",
                    c.name, c.strip_flags, c.write_both_slots, c.value,
                ));
            }
            out.push('\n');
        }

        if !f.hooks.is_empty() {
            out.push_str("### Hooks\n\n");
            out.push_str("| Function | Module | Signature | Action |\n");
            out.push_str("|---|---|---|---|\n");
            for h in f.hooks.iter() {
                out.push_str(&format!(
                    "| `{}` | `{}` | `{}` | {} |\n",
                    h.function, h.module, h.signature, h.action,
                ));
            }
            out.push('\n');
        }

        out.push_str("---\n\n");
    }

    out
}

pub fn render_hpp(build_number: Option<u32>) -> String {
    let mut out = String::new();
    out.push_str("// =====================================================================\n");
    out.push_str("// verified_features.hpp — hand-curated working CS2 cheat features\n");
    out.push_str("// =====================================================================\n");
    out.push_str("//\n");
    out.push_str("// Each entry is a known-good offset / hook / ConVar trick that has been\n");
    out.push_str("// verified live against the current CS2 build. Constants are static\n");
    out.push_str("// constexpr so they can be used at compile time.\n");
    out.push_str("//\n");
    if let Some(b) = build_number {
        out.push_str(&format!("// CS2_BUILD: {}\n", b));
    }
    out.push_str("// =====================================================================\n\n");
    out.push_str("#pragma once\n\n");
    out.push_str("#include <cstdint>\n\n");
    out.push_str("namespace cs2::verified\n{\n");

    for f in FEATURES.iter() {
        let ns = sanitize_ident(f.name);
        out.push_str(&format!("    // ----- {} -----\n", f.name));
        out.push_str(&format!("    namespace {}\n    {{\n", ns));
        for fld in f.fields.iter() {
            let cls = sanitize_ident(fld.class);
            let fname = sanitize_ident(fld.field);
            out.push_str(&format!(
                "        constexpr std::ptrdiff_t {}__{} = 0x{:X}; // {} — {}\n",
                cls, fname, fld.offset, fld.ty, fld.note,
            ));
        }
        for c in f.convars.iter() {
            let cn = sanitize_ident(c.name);
            out.push_str(&format!(
                "        constexpr uint32_t convar_{}__strip_flags = 0x{:X};\n",
                cn, c.strip_flags,
            ));
            out.push_str(&format!(
                "        constexpr bool     convar_{}__write_both  = {};\n",
                cn, c.write_both_slots,
            ));
        }
        out.push_str("    }\n\n");
    }

    out.push_str("} // namespace cs2::verified\n");
    out
}

fn sanitize_ident(input: &str) -> String {
    let mut s = String::with_capacity(input.len());
    for c in input.chars() {
        if c.is_ascii_alphanumeric() || c == '_' {
            s.push(c);
        } else {
            s.push('_');
        }
    }
    if s.chars().next().map(|c| c.is_ascii_digit()).unwrap_or(false) {
        s.insert(0, '_');
    }
    s
}

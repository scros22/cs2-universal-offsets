// Generated using https://github.com/a2x/cs2-dumper
// 2026-04-21 22:21:30.322409500 UTC

pub const cs2_dumper = struct {
    pub const schemas = struct {
        // Module: materialsystem2.dll
        // Class count: 14
        // Enum count: 5
        pub const materialsystem2_dll = struct {
            // Alignment: 4
            // Member count: 4
            pub const VertJustification_e = enum(u32) {
                VERT_JUSTIFICATION_TOP = 0x0,
                VERT_JUSTIFICATION_CENTER = 0x1,
                VERT_JUSTIFICATION_BOTTOM = 0x2,
                VERT_JUSTIFICATION_NONE = 0x3
            };
            // Alignment: 4
            // Member count: 3
            pub const LayoutPositionType_e = enum(u32) {
                LAYOUTPOSITIONTYPE_VIEWPORT_RELATIVE = 0x0,
                LAYOUTPOSITIONTYPE_FRACTIONAL = 0x1,
                LAYOUTPOSITIONTYPE_NONE = 0x2
            };
            // Alignment: 4
            // Member count: 3
            pub const ViewFadeMode_t = enum(u32) {
                VIEW_FADE_CONSTANT_COLOR = 0x0,
                VIEW_FADE_MODULATE = 0x1,
                VIEW_FADE_MOD2X = 0x2
            };
            // Alignment: 4
            // Member count: 3
            pub const BloomBlendMode_t = enum(u32) {
                BLOOM_BLEND_ADD = 0x0,
                BLOOM_BLEND_SCREEN = 0x1,
                BLOOM_BLEND_BLUR = 0x2
            };
            // Alignment: 4
            // Member count: 4
            pub const HorizJustification_e = enum(u32) {
                HORIZ_JUSTIFICATION_LEFT = 0x0,
                HORIZ_JUSTIFICATION_CENTER = 0x1,
                HORIZ_JUSTIFICATION_RIGHT = 0x2,
                HORIZ_JUSTIFICATION_NONE = 0x3
            };
            // Parent: None
            // Field count: 1
            pub const MaterialParam_t = struct {
                pub const @"": usize = 0x0; // 
            };
            // Parent: materialsystem2
            // Field count: 0
            pub const MaterialParamVector_t = struct {
            };
            // Parent: materialsystem2
            // Field count: 0
            pub const MaterialParamString_t = struct {
            };
            // Parent: None
            // Field count: 1
            pub const PostProcessingResource_t = struct {
                pub const @"": usize = 0x0; // 
            };
            // Parent: None
            // Field count: 0
            pub const MaterialParamInt_t = struct {
            };
            // Parent: None
            // Field count: 1
            pub const PostProcessingVignetteParameters_t = struct {
                pub const @"": usize = 0x0; // 
            };
            // Parent: None
            // Field count: 1
            pub const PostProcessingLocalContrastParameters_t = struct {
                pub const @"": usize = 0x0; // 
            };
            // Parent: None
            // Field count: 1
            pub const PostProcessingTonemapParameters_t = struct {
                pub const @"": usize = 0x0; // 
            };
            // Parent: None
            // Field count: 1
            pub const PostProcessingFogScatteringParameters_t = struct {
                pub const @"": usize = 0x0; // 
            };
            // Parent: xL___
            // Field count: 0
            pub const MaterialParamBuffer_t = struct {
            };
            // Parent: None
            // Field count: 1
            pub const MaterialResourceData_t = struct {
                pub const @"": usize = 0x0; // 
            };
            // Parent: None
            // Field count: 1
            pub const PostProcessingBloomParameters_t = struct {
                pub const @"": usize = 0x0; // 
            };
            // Parent: None
            // Field count: 0
            pub const MaterialParamFloat_t = struct {
            };
            // Parent: None
            // Field count: 0
            pub const MaterialParamTexture_t = struct {
            };
        };
    };
};

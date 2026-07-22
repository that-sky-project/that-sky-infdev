#ifndef __SKY_SKYGFX_HPP__
#define __SKY_SKYGFX_HPP__

#include <Utils/Types.h>

enum GfxType: u08 {
  kGfxType_UNKNOWN = 0,
  kGfxType_FLOAT,
  kGfxType_FLOAT2,
  kGfxType_FLOAT3,
  kGfxType_FLOAT4,
  kGfxType_INT,
  kGfxType_INT2,
  kGfxType_INT3,
  kGfxType_INT4,
  kGfxType_UINT,
  kGfxType_UINT2,
  kGfxType_UINT3,
  kGfxType_UINT4,
  kGfxType_HALF,
  kGfxType_HALF2,
  kGfxType_HALF3,
  kGfxType_HALF4,
  kGfxType_SHORT,
  kGfxType_SHORT2,
  kGfxType_SHORT3,
  kGfxType_SHORT4,
  kGfxType_BYTE,
  kGfxType_BYTE2,
  kGfxType_BYTE3,
  kGfxType_BYTE4,
  kGfxType_UBYTE,
  kGfxType_UBYTE2,
  kGfxType_UBYTE3,
  kGfxType_UBYTE4,
  kGfxType_USHORT,
  kGfxType_USHORT2,
  kGfxType_USHORT3,
  kGfxType_USHORT4,
  kGfxType_FLOAT3x3,
  kGfxType_FLOAT4x3,
  kGfxType_FLOAT4x4,
  kGfxType_HALF3x3,
  kGfxType_HALF4x3,
  kGfxType_HALF4x4,
};

enum GfxAttr: u08 {
  kGfxAttr_Undefined = 0,
  kGfxAttr_Position,
  kGfxAttr_Color,
  kGfxAttr_Normal,
  kGfxAttr_Tangent,
  kGfxAttr_TexCoord0,
  kGfxAttr_TexCoord1,
  kGfxAttr_TexCoord2,
  kGfxAttr_TexCoord3,
  kGfxAttr_TexCoord4,
  kGfxAttr_TexCoord5,
  kGfxAttr_TexCoord6,
  kGfxAttr_TexCoord7,
  kGfxAttr_Light0,
  kGfxAttr_Light1,
  kGfxAttr_Light2,
  kGfxAttr_Light3,
  kGfxAttr_Noise0,
  kGfxAttr_PointSize,
  kGfxAttr_Center,
  kGfxAttr_RotX,
  kGfxAttr_RotY,
  kGfxAttr_RotZ,
  kGfxAttr_Scale,
  kGfxAttr_UserData0,
  kGfxAttr_UserData1,
  kGfxAttr_UserData2,
  kGfxAttr_UserData3,
  kGfxAttr_RailInfo,
  kGfxAttr_RailBindEnd,
  kGfxAttr_RailBindUp,
  kGfxAttr_NormDepth,
  kGfxAttr_Material0,
  kGfxAttr_Material1,
};

enum GfxTexelFormat: u08 {
  kGfxTexelFormat_Undefined = 0,
  kGfxTexelFormat_U565,
  kGfxTexelFormat_U8,
  kGfxTexelFormat_I8,
  kGfxTexelFormat_U8_sRGB,
  kGfxTexelFormat_U8x2,
  kGfxTexelFormat_U8x2_int,
  kGfxTexelFormat_U8x2_sRGB,
  kGfxTexelFormat_U8x4,
  kGfxTexelFormat_U8x4_int,
  kGfxTexelFormat_U8x4_sRGB,
  kGfxTexelFormat_U8x4_BGRA,
  kGfxTexelFormat_U8x4_BGRA_sRGB,
  kGfxTexelFormat_U10_10_10_2,
  kGfxTexelFormat_U10_10_10_2_BGRA,
  kGfxTexelFormat_U10_10_10_XR,
  kGfxTexelFormat_U10_10_10_XR_sRGB,
  kGfxTexelFormat_U16_int,
  kGfxTexelFormat_U16x2,
  kGfxTexelFormat_U16x4,
  kGfxTexelFormat_F11_11_10,
  kGfxTexelFormat_F16,
  kGfxTexelFormat_F16x2,
  kGfxTexelFormat_F16x4,
  kGfxTexelFormat_F32,
  kGfxTexelFormat_F32x2,
  kGfxTexelFormat_F32x4,
  kGfxTexelFormat_Depth32,
  kGfxTexelFormat_Depth24Stencil8,
  kGfxTexelFormat_ETC2_RGB,
  kGfxTexelFormat_ETC2_RGBA,
  kGfxTexelFormat_ETC2_RGB_sRGB,
  kGfxTexelFormat_ETC2_RGBA_sRGB,
  kGfxTexelFormat_EAC_R11_UNORM,
  kGfxTexelFormat_EAC_R11_SNORM,
  kGfxTexelFormat_EAC_RG11_UNORM,
  kGfxTexelFormat_EAC_RG11_SNORM,
  kGfxTexelFormat_ASTC_RGBA_4x4,
  kGfxTexelFormat_ASTC_RGBA_5x5,
  kGfxTexelFormat_ASTC_RGBA_6x6,
  kGfxTexelFormat_ASTC_RGBA_8x8,
  kGfxTexelFormat_ASTC_RGBA_4x4_sRGB,
  kGfxTexelFormat_ASTC_RGBA_5x5_sRGB,
  kGfxTexelFormat_ASTC_RGBA_6x6_sRGB,
  kGfxTexelFormat_ASTC_RGBA_8x8_sRGB,
  kGfxTexelFormat_BC1_RGB,
  kGfxTexelFormat_BC1_RGB_sRGB,
  kGfxTexelFormat_BC3_RGBA,
  kGfxTexelFormat_BC3_RGBA_sRGB,
  kGfxTexelFormat_BC4_R_UNORM,
  kGfxTexelFormat_BC4_R_SNORM,
  kGfxTexelFormat_BC5_RG_UNORM,
  kGfxTexelFormat_BC5_RG_SNORM,
  kGfxTexelFormat_BC6_RGBF_UNORM,
  kGfxTexelFormat_BC6_RGBF_SNORM,
  kGfxTexelFormat_BC7_RGBA,
  kGfxTexelFormat_BC7_RGBA_sRGB,
};

/*
// ---- by AI, 0.15.1 texel formats.
// GPU texel (pixel) formats.
//
// Reconstructed from VulkanRenderer's GfxTexelFormat -> VkFormat lookup table
// (sub_7EB8A0 / dword_1395CB0). The trailing comment on each entry is the raw
// VkFormat value the Vulkan backend maps it to. Entries marked "unsupported"
// map to VK_FORMAT_UNDEFINED (0) in that table and are rejected by
// VulkanRenderer::IsTexelFormatSupported (support mask 0x7FFFFFFE01FFDFFF).
enum GfxTexelFormat: u08 {
  kGfxTexelFormat_UNKNOWN = 0,        //  0  VK_FORMAT_UNDEFINED

  // -- packed low-bit color --
  kGfxTexelFormat_RGB565,             //  1  VK_FORMAT_B5G6R5_UNORM_PACK16

  // -- 8-bit unorm / uint / sRGB --
  kGfxTexelFormat_R8,                 //  2  VK_FORMAT_R8_UNORM
  kGfxTexelFormat_R8_SRGB,            //  3  VK_FORMAT_R8_SRGB
  kGfxTexelFormat_RG8,                //  4  VK_FORMAT_R8G8_UNORM
  kGfxTexelFormat_RG8UI,              //  5  VK_FORMAT_R8G8_UINT
  kGfxTexelFormat_RG8_SRGB,           //  6  VK_FORMAT_R8G8_SRGB
  kGfxTexelFormat_RGBA8,              //  7  VK_FORMAT_R8G8B8A8_UNORM
  kGfxTexelFormat_RGBA8UI,            //  8  VK_FORMAT_R8G8B8A8_UINT
  kGfxTexelFormat_RGBA8_SRGB,         //  9  VK_FORMAT_R8G8B8A8_SRGB
  kGfxTexelFormat_BGRA8,              // 10  VK_FORMAT_B8G8R8A8_UNORM
  kGfxTexelFormat_BGRA8_SRGB,         // 11  VK_FORMAT_B8G8R8A8_SRGB

  // -- 10/11-bit packed --
  kGfxTexelFormat_RGB10A2,            // 12  VK_FORMAT_A2B10G10R10_UNORM_PACK32
  kGfxTexelFormat_RGB10A2UI,          // 13  unsupported (VK_FORMAT_UNDEFINED)

  // -- 16-bit integer --
  kGfxTexelFormat_R16UI,              // 14  VK_FORMAT_R16_UINT
  kGfxTexelFormat_RGBA16UI,           // 15  VK_FORMAT_R16G16B16A16_UINT

  // -- floating point --
  kGfxTexelFormat_RG11B10F,           // 16  VK_FORMAT_B10G11R11_UFLOAT_PACK32
  kGfxTexelFormat_R16F,               // 17  VK_FORMAT_R16_SFLOAT
  kGfxTexelFormat_RG16F,              // 18  VK_FORMAT_R16G16_SFLOAT
  kGfxTexelFormat_RGBA16F,            // 19  VK_FORMAT_R16G16B16A16_SFLOAT
  kGfxTexelFormat_R32F,               // 20  VK_FORMAT_R32_SFLOAT
  kGfxTexelFormat_RG32F,              // 21  VK_FORMAT_R32G32_SFLOAT
  kGfxTexelFormat_RGBA32F,            // 22  VK_FORMAT_R32G32B32A32_SFLOAT

  // -- depth / stencil --
  kGfxTexelFormat_Depth32F,           // 23  VK_FORMAT_D32_SFLOAT
  kGfxTexelFormat_Depth32F_Stencil8,  // 24  VK_FORMAT_D32_SFLOAT_S8_UINT

  // -- reserved gap: all unsupported by VulkanRenderer (map to UNDEFINED) --
  kGfxTexelFormat_Reserved25,         // 25  unsupported
  kGfxTexelFormat_Reserved26,         // 26  unsupported
  kGfxTexelFormat_Reserved27,         // 27  unsupported
  kGfxTexelFormat_Reserved28,         // 28  unsupported
  kGfxTexelFormat_Reserved29,         // 29  unsupported
  kGfxTexelFormat_Reserved30,         // 30  unsupported
  kGfxTexelFormat_Reserved31,         // 31  unsupported
  kGfxTexelFormat_Reserved32,         // 32  unsupported

  // -- ETC2 (mobile) --
  kGfxTexelFormat_ETC2_RGB8,          // 33  VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK
  kGfxTexelFormat_ETC2_RGBA8,         // 34  VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK
  kGfxTexelFormat_ETC2_RGB8_SRGB,     // 35  VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK
  kGfxTexelFormat_ETC2_RGBA8_SRGB,    // 36  VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK

  // -- ASTC (mobile) --
  kGfxTexelFormat_ASTC_4x4,           // 37  VK_FORMAT_ASTC_4x4_UNORM_BLOCK
  kGfxTexelFormat_ASTC_6x6,           // 38  VK_FORMAT_ASTC_6x6_UNORM_BLOCK
  kGfxTexelFormat_ASTC_8x8,           // 39  VK_FORMAT_ASTC_8x8_UNORM_BLOCK
  kGfxTexelFormat_ASTC_10x10,         // 40  VK_FORMAT_ASTC_10x10_UNORM_BLOCK
  kGfxTexelFormat_ASTC_12x12,         // 41  VK_FORMAT_ASTC_12x12_UNORM_BLOCK
  kGfxTexelFormat_ASTC_4x4_SRGB,      // 42  VK_FORMAT_ASTC_4x4_SRGB_BLOCK
  kGfxTexelFormat_ASTC_6x6_SRGB,      // 43  VK_FORMAT_ASTC_6x6_SRGB_BLOCK
  kGfxTexelFormat_ASTC_8x8_SRGB,      // 44  VK_FORMAT_ASTC_8x8_SRGB_BLOCK
  kGfxTexelFormat_ASTC_10x10_SRGB,    // 45  VK_FORMAT_ASTC_10x10_SRGB_BLOCK
  kGfxTexelFormat_ASTC_12x12_SRGB,    // 46  VK_FORMAT_ASTC_12x12_SRGB_BLOCK

  // -- BC / DXT (desktop) --
  kGfxTexelFormat_BC1,                // 47  VK_FORMAT_BC1_RGB_UNORM_BLOCK
  kGfxTexelFormat_BC1_SRGB,           // 48  VK_FORMAT_BC1_RGB_SRGB_BLOCK
  kGfxTexelFormat_BC3,                // 49  VK_FORMAT_BC3_UNORM_BLOCK
  kGfxTexelFormat_BC3_SRGB,           // 50  VK_FORMAT_BC3_SRGB_BLOCK
  kGfxTexelFormat_BC4,                // 51  VK_FORMAT_BC4_UNORM_BLOCK
  kGfxTexelFormat_BC4_SNORM,          // 52  VK_FORMAT_BC4_SNORM_BLOCK
  kGfxTexelFormat_BC5,                // 53  VK_FORMAT_BC5_UNORM_BLOCK
  kGfxTexelFormat_BC5_SNORM,          // 54  VK_FORMAT_BC5_SNORM_BLOCK
  kGfxTexelFormat_BC6H_UF,            // 55  VK_FORMAT_BC6H_UFLOAT_BLOCK
  kGfxTexelFormat_BC6H_SF,            // 56  VK_FORMAT_BC6H_SFLOAT_BLOCK
  kGfxTexelFormat_BC7,                // 57  VK_FORMAT_BC7_UNORM_BLOCK
  kGfxTexelFormat_BC7_SRGB,           // 58  VK_FORMAT_BC7_SRGB_BLOCK

  // -- EAC single / dual channel --
  kGfxTexelFormat_EAC_R11,            // 59  VK_FORMAT_EAC_R11_UNORM_BLOCK
  kGfxTexelFormat_EAC_R11_SNORM,      // 60  VK_FORMAT_EAC_R11_SNORM_BLOCK
  kGfxTexelFormat_EAC_RG11,           // 61  VK_FORMAT_EAC_R11G11_UNORM_BLOCK
  kGfxTexelFormat_EAC_RG11_SNORM,     // 62  VK_FORMAT_EAC_R11G11_SNORM_BLOCK

  kGfxTexelFormat_NUM,                // 63 (0x3F) upper bound checked in sub_7EB8A0
};
*/

#endif

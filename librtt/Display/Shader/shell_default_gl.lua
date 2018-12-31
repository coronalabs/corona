local shell = {}

shell.language = "glsl"

shell.category = "default"

shell.name = "default"

shell.vertex =
[[
attribute vec2 a_Position;
attribute vec3 a_TexCoord;
attribute vec4 a_ColorScale;
attribute vec4 a_UserData;

uniform P_DEFAULT float u_TotalTime;
uniform P_DEFAULT float u_DeltaTime;
uniform P_UV vec4 u_TexelSize;
uniform P_POSITION vec2 u_ContentScale;

uniform P_POSITION mat4 u_ViewProjectionMatrix;

#define CoronaVertexUserData a_UserData
#define CoronaTexCoord a_TexCoord.xy

#define CoronaTotalTime u_TotalTime
#define CoronaDeltaTime u_DeltaTime
#define CoronaTexelSize u_TexelSize
#define CoronaContentScale u_ContentScale

#if MASK_COUNT > 0
    uniform P_POSITION mat3 u_MaskMatrix0;
#endif

#if MASK_COUNT > 1
    uniform P_POSITION mat3 u_MaskMatrix1;
#endif

#if MASK_COUNT > 2
    uniform P_POSITION mat3 u_MaskMatrix2;
#endif

varying P_POSITION vec2 v_Position;
varying P_UV vec2 v_TexCoord;
#ifdef TEX_COORD_Z
	varying P_UV float v_TexCoordZ;
#endif

varying P_COLOR vec4 v_ColorScale;
varying P_DEFAULT vec4 v_UserData;

#if MASK_COUNT > 0
    varying P_UV vec2 v_MaskUV0;
#endif

#if MASK_COUNT > 1
    varying P_UV vec2 v_MaskUV1;
#endif

#if MASK_COUNT > 2
    varying P_UV vec2 v_MaskUV2;
#endif

#ifdef Rtt_WEBGL_ENV
%s
#else
P_POSITION vec2 VertexKernel( P_POSITION vec2 position );
#endif

void main()
{
	// "varying" are only meant as OUTPUT variables. ie: Write-only variables
	// meant to provide to a fragment shader, values computed in a vertex
	// shader.
	//
	// Certain devices, like the "Samsung Galaxy Tab 2", DON'T allow you to
	// use "varying" variable like any other local variables.

	v_TexCoord = a_TexCoord.xy;
#ifdef TEX_COORD_Z
	v_TexCoordZ = a_TexCoord.z;
#endif
	v_ColorScale = a_ColorScale;
	v_UserData = a_UserData;

	P_POSITION vec2 position = VertexKernel( a_Position );

    #if MASK_COUNT > 0
        v_MaskUV0 = ( u_MaskMatrix0 * vec3( position, 1.0 ) ).xy;
    #endif

    #if MASK_COUNT > 1
        v_MaskUV1 = ( u_MaskMatrix1 * vec3( position, 1.0 ) ).xy;
    #endif

    #if MASK_COUNT > 2
        v_MaskUV2 = ( u_MaskMatrix2 * vec3( position, 1.0 ) ).xy;
    #endif

    gl_Position = u_ViewProjectionMatrix * vec4( position, 0.0, 1.0 );
}
]]

shell.fragment =
[[
uniform sampler2D u_FillSampler0;
uniform sampler2D u_FillSampler1;
uniform P_DEFAULT float u_TotalTime;
uniform P_DEFAULT float u_DeltaTime;
uniform P_UV vec4 u_TexelSize;
uniform P_POSITION vec2 u_ContentScale;

varying P_POSITION vec2 v_Position;
varying P_UV vec2 v_TexCoord;
#ifdef TEX_COORD_Z
	varying P_UV float v_TexCoordZ;
#endif

varying P_COLOR vec4 v_ColorScale;
varying P_DEFAULT vec4 v_UserData;

#define CoronaColorScale( color ) (v_ColorScale*(color))
#define CoronaVertexUserData v_UserData

#define CoronaTotalTime u_TotalTime
#define CoronaDeltaTime u_DeltaTime
#define CoronaTexelSize u_TexelSize
#define CoronaContentScale u_ContentScale
#define CoronaSampler0 u_FillSampler0
#define CoronaSampler1 u_FillSampler1

#if MASK_COUNT > 0
    uniform sampler2D u_MaskSampler0;
    varying P_UV vec2 v_MaskUV0;
#endif

#if MASK_COUNT > 1
    uniform sampler2D u_MaskSampler1;
    varying P_UV vec2 v_MaskUV1;
#endif

#if MASK_COUNT > 2
    uniform sampler2D u_MaskSampler2;
    varying P_UV vec2 v_MaskUV2;
#endif

#ifdef Rtt_WEBGL_ENV
%s
#else
P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord );
#endif

void main()
{
#ifdef TEX_COORD_Z
    P_COLOR vec4 result = FragmentKernel( v_TexCoord.xy / v_TexCoordZ );
#else
    P_COLOR vec4 result = FragmentKernel( v_TexCoord );
#endif
    
    #if MASK_COUNT > 0
        result *= texture2D( u_MaskSampler0, v_MaskUV0 ).r;
    #endif

    #if MASK_COUNT > 1
        result *= texture2D( u_MaskSampler1, v_MaskUV1 ).r;
    #endif

    #if MASK_COUNT > 2
        result *= texture2D( u_MaskSampler2, v_MaskUV2 ).r;
    #endif

    gl_FragColor = result;
}
]]

return shell

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

--
--
--

-- The code above is the canonical GL shell. What follows closely adheres to it in form, but with a few
-- choice bits meant to be "tweaked" with appropriate subsitutions. (To keep them synced, the replacement
-- logic is run with all defaults and must reproduces the "true" shell exactly.)

--
--
--

local vform =
[[
attribute VSHELL_APOS a_Position;
attribute vec3 a_TexCoord;
attribute vec4 a_ColorScale;
attribute vec4 a_UserData;

uniform P_DEFAULT float u_TotalTime;
uniform P_DEFAULT float u_DeltaTime;
uniform P_UV vec4 u_TexelSize;
uniform P_POSITION vec2 u_ContentScale;VSHELL_DECL_USERDATA

uniform P_POSITION mat4 u_ViewProjectionMatrix;

#define CoronaVertexUserData a_UserData
#define CoronaTexCoord a_TexCoord.xy
VSHELL_DEFINE_APOS_VAR
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
P_POSITION VSHELL_KRET VertexKernel( VSHELL_KARGDECL );
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

	P_POSITION VSHELL_KRET position = VSHELL_KCALL;

    #if MASK_COUNT > 0
        v_MaskUV0 = ( u_MaskMatrix0 * vec3( VSHELL_MASK_POS ) ).xy;
    #endif

    #if MASK_COUNT > 1
        v_MaskUV1 = ( u_MaskMatrix1 * vec3( VSHELL_MASK_POS ) ).xy;
    #endif

    #if MASK_COUNT > 2
        v_MaskUV2 = ( u_MaskMatrix2 * vec3( VSHELL_MASK_POS ) ).xy;
    #endif

    gl_Position = VSHELL_LHS_MATRIX * vec4( VSHELL_RHS_VEC3, 1.0 );VSHELL_SET_POSITION_Z
}
]]

--
--
--

local fform =
[[
uniform FSHELL_SAMPLER0_TYPE u_FillSampler0;
uniform FSHELL_SAMPLER1_TYPE u_FillSampler1;
uniform P_DEFAULT float u_TotalTime;
uniform P_DEFAULT float u_DeltaTime;
uniform P_UV vec4 u_TexelSize;
uniform P_POSITION vec2 u_ContentScale;FSHELL_DECL_USERDATA

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

--
--
--

local Replacements = 
{
  VSHELL_APOS = "vec2",
  VSHELL_KRET = "vec2",
  VSHELL_MASK_POS = "position, 1.0",
  VSHELL_KARGDECL = "P_POSITION vec2 position",
  VSHELL_KCALL = "VertexKernel( a_Position )",
  VSHELL_DEFINE_APOS_VAR = "",
  VSHELL_LHS_MATRIX = "u_ViewProjectionMatrix",
  VSHELL_RHS_VEC3 = "position, 0.0",
  VSHELL_SET_POSITION_Z = "",
  VSHELL_DECL_USERDATA = "",
  
  FSHELL_SAMPLER0_TYPE = "sampler2D",
  FSHELL_SAMPLER1_TYPE = "sampler2D",
  FSHELL_DECL_USERDATA = ""
}

--
--
--

local assert = assert
local concat = table.concat
local ipairs = ipairs
local pairs = pairs
local type = type

local function NilOrString( options, key )
  local v = options[key]

  assert( v == nil or type( v ) == "string" )
  
  return v
end

local function ConfigureTweaks( options, replacements, uniforms )
  local zAsVar = NilOrString( options, "varFromPosZ" )
  local lhm = NilOrString( options, "lhsMatrix" )
  local rhz = NilOrString( options, "rhsZcoord" )
  local spz = NilOrString( options, "setPositionZ" )
  local s0t = NilOrString( options, "sampler0Type" )
  local s1t = NilOrString( options, "sampler1Type" )
  local zAsExtraArg = options.extraKernelArgumentFromPosZ

  if zAsExtraArg or zAsVar then
    if zAsExtraArg then
      replacements.VSHELL_KARGDECL = "P_POSITION vec2 position, P_POSITION float extra"
      replacements.VSHELL_KCALL = "VertexKernel( a_Position.xy, a_Position.z )"
    else
      replacements.VSHELL_KCALL = "VertexKernel( a_Position.xy )"
    end
    
    if zAsVar then
      replacements.VSHELL_DEFINE_APOS_VAR = "#define " .. zAsVar .. " a_Position.z\n\n"
    end

    replacements.VSHELL_APOS = "vec3"
    replacements.VSHELL_RHS_VEC3 = "position, " .. ( rhz or "0.0" )
  elseif options.posAttributeHasZ then
    replacements.VSHELL_APOS = "vec3"
    replacements.VSHELL_KRET = "vec3"
    replacements.VSHELL_KARGDECL = "P_POSITION vec3 position"
    replacements.VSHELL_RHS_VEC3 = "position"
    replacements.VSHELL_MASK_POS = "position.xy, 1.0"
    
    rhz = nil
  elseif rhz then
    replacements.VSHELL_RHS_VEC3 = "position, " .. rhz
  end

  if uniforms and options.declareUniforms then
    local decl_uniforms = { "", "" }

    for _, userData in ipairs( uniforms ) do
      if type( userData.type ) == "string" and type( userData.index ) == "number" then
        decl_uniforms[#decl_uniforms + 1] = "uniform " .. userData.type .. " u_UserData" .. userData.index .. ";"
      end
    end

    decl_uniforms = concat( decl_uniforms, "\n" )

    if decl_uniforms == "vertex" then
      replacements.VSHELL_DECL_USERDATA = decl_uniforms
    elseif decl_uniforms == "fragment" then
      replacements.FSHELL_DECL_USERDATA = decl_uniforms
    else
      replacements.VSHELL_DECL_USERDATA = decl_uniforms
      replacements.FSHELL_DECL_USERDATA = decl_uniforms
    end
  end

  if spz then
    replacements.VSHELL_SET_POSITION_Z = "\n    gl_Position.z = " .. spz .. ";"
  end

  for k, v in pairs( {
    VSHELL_LHS_MATRIX = lhm,
    FSHELL_SAMPLER0_TYPE = s0t,
    FSHELL_SAMPLER1_TYPE = s1t
  } ) do
    replacements[k] = v
  end
end

local function Replace( options, uniforms )
  assert( options == nil or type( options ) == "table" )
  assert( uniforms == nil or type( uniforms ) == "table" )
  
  options = options or {}

  local replacements = {}

  for k, v in pairs( Replacements ) do
    replacements[k] = v
  end

  ConfigureTweaks( options, replacements, uniforms )

  local vert, frag = vform, fform

  for k, v in pairs( replacements ) do
    vert = vert:gsub( k, v )
    frag = frag:gsub( k, v )
  end

  local hasZ = "vec3" == replacements.VSHELL_APOS
  local modifiedSampler = "sampler2D" ~= replacements.FSHELL_SAMPLER0_TYPE or "sampler2D" ~= replacements.FSHELL_SAMPLER1_TYPE

  return hasZ, modifiedSampler, vert, frag
end

shell.replace = Replace

--
--
--

local _, _, defVert, defFrag = Replace( nil )

assert( defVert == shell.vertex, "Default vertex shell and replacement form have diverged" )
assert( defFrag == shell.fragment, "Default fragment shell and replacement form have diverged" )

--
--
--

return shell

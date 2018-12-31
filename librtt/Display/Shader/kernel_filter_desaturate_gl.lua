local kernel = {}

kernel.language = "glsl"

kernel.category = "filter"

kernel.name = "desaturate"

kernel.vertexData =
{
    {
        name = "intensity",
        default = 0.5,
        min = 0,
        max = 1,
        index = 0, -- v_UserData.x
    },
}

kernel.fragment =
[[
const P_COLOR vec3 kWeights = vec3( 0.2125, 0.7154, 0.0721 );

P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
    P_COLOR vec4 texColor = texture2D( u_FillSampler0, texCoord ) * v_ColorScale;
    P_COLOR float luminance = dot( texColor.rgb, kWeights );

    P_COLOR vec3 result = mix( vec3( texColor.rgb ), vec3( luminance ), v_UserData.x );

    return vec4( result, texColor.a );
}
]]

return kernel

local kernel = {}

kernel.language = "glsl"

kernel.category = "composite"

kernel.name = "screen"

kernel.vertexData =
{
    {
        name = "alpha",
        default = 1,
        min = 0,
        max = 1,
        index = 0, -- v_UserData.x
    },
}

kernel.fragment =
[[
P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
    P_COLOR vec4 base = texture2D( u_FillSampler0, texCoord );
    P_COLOR vec4 blend = texture2D( u_FillSampler1, texCoord );

    P_COLOR vec4 result = 1.0 - ( 1.0 - base ) * ( 1.0 - blend );

    return mix( base, result, v_UserData.x ) * v_ColorScale;
}
]]

return kernel

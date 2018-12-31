local kernel = {}

kernel.language = "glsl"

kernel.category = "composite"

kernel.name = "average"

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

    P_COLOR vec4 result = 0.5 * (base + blend);

    return mix( base, result, v_UserData.x ) * v_ColorScale;
}
]]

return kernel

local kernel = {}

kernel.language = "glsl"

kernel.category = "composite"

kernel.name = "color"

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
const P_COLOR vec4 kWeights = vec4( 0.2125, 0.7154, 0.0721, 1.0 );

P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
    P_COLOR vec4 base = texture2D( u_FillSampler0, texCoord );
    P_COLOR vec4 blend = texture2D( u_FillSampler1, texCoord );

    // Luminance of blend
    P_COLOR float luminance = dot( blend, kWeights );

    // TODO
    P_COLOR vec4 result;

    return mix( base, result, v_UserData.x ) * v_ColorScale;
}
]]

return kernel

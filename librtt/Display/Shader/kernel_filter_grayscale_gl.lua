local kernel = {}

kernel.language = "glsl"

kernel.category = "filter"

kernel.name = "grayscale"

kernel.fragment =
[[
const P_COLOR vec3 kWeights = vec3( 0.2125, 0.7154, 0.0721 );

P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
    P_COLOR vec4 texColor = texture2D( u_FillSampler0, texCoord ) * v_ColorScale;
    P_COLOR float luminance = dot( texColor.rgb, kWeights );

    return vec4( vec3( luminance ), texColor.a );
}
]]

return kernel

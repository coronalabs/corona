local kernel = {}

kernel.language = "glsl"

kernel.category = "filter"

kernel.name = "blur"

kernel.fragment =
[[
P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
    P_UV vec4 result = vec4( 0.0 );
    P_UV float offset = 2.5;
    result += texture2D( u_FillSampler0, texCoord + vec2( -offset, -offset ) * u_TexelSize.xy );
    result += texture2D( u_FillSampler0, texCoord + vec2(  0.0, -offset ) * u_TexelSize.xy ) * 2.0;
    result += texture2D( u_FillSampler0, texCoord + vec2(  offset, -offset ) * u_TexelSize.xy );
    result += texture2D( u_FillSampler0, texCoord + vec2( -offset,  0.0 ) * u_TexelSize.xy ) * 2.0;
    result += texture2D( u_FillSampler0, texCoord + vec2(  0.0,  0.0 ) * u_TexelSize.xy );
    result += texture2D( u_FillSampler0, texCoord + vec2(  offset,  0.0 ) * u_TexelSize.xy ) * 2.0;
    result += texture2D( u_FillSampler0, texCoord + vec2( -offset,  offset ) * u_TexelSize.xy );
    result += texture2D( u_FillSampler0, texCoord + vec2(  0.0,  offset ) * u_TexelSize.xy ) * 2.0;
    result += texture2D( u_FillSampler0, texCoord + vec2(  offset,  offset ) * u_TexelSize.xy );
    result = result * v_ColorScale;
    return result * 0.076923; // 0.076923 = (1 / 13)
}
]]

return kernel

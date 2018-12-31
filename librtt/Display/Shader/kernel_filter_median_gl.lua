local kernel = {}

kernel.language = "glsl"

kernel.category = "filter"

kernel.name = "median"

kernel.fragment =
[[
#define s2(a, b)				temp = a; a = min(a, b); b = max(temp, b);
#define mn3(a, b, c)			s2(a, b); s2(a, c);
#define mx3(a, b, c)			s2(b, c); s2(a, c);

#define mnmx3(a, b, c)			mx3(a, b, c); s2(a, b);                                   // 3 exchanges
#define mnmx4(a, b, c, d)		s2(a, b); s2(c, d); s2(a, c); s2(b, d);                   // 4 exchanges
#define mnmx5(a, b, c, d, e)	s2(a, b); s2(c, d); mn3(a, c, e); mx3(b, d, e);           // 6 exchanges
#define mnmx6(a, b, c, d, e, f) s2(a, d); s2(b, e); s2(c, f); mn3(a, b, c); mx3(d, e, f); // 7 exchanges

P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
	P_UV vec2 leftTextureCoordinate = texCoord + vec2( - u_TexelSize.x, 0.0 );
	P_UV vec2 rightTextureCoordinate = texCoord + vec2( u_TexelSize.x, 0.0 );

	P_UV vec2 topTextureCoordinate = texCoord + vec2( 0.0, - u_TexelSize.y );
	P_UV vec2 topLeftTextureCoordinate = texCoord + vec2( - u_TexelSize.x, - u_TexelSize.y );
	P_UV vec2 topRightTextureCoordinate = texCoord + vec2( u_TexelSize.x, - u_TexelSize.y );

	P_UV vec2 bottomTextureCoordinate = texCoord + vec2( 0.0, u_TexelSize.y );
	P_UV vec2 bottomLeftTextureCoordinate = texCoord + vec2( - u_TexelSize.x, u_TexelSize.y );
	P_UV vec2 bottomRightTextureCoordinate = texCoord + vec2( u_TexelSize.x, u_TexelSize.y );

	P_COLOR vec3 v[6];

	v[0] = texture2D(u_FillSampler0, bottomLeftTextureCoordinate).rgb;
	v[1] = texture2D(u_FillSampler0, topRightTextureCoordinate).rgb;
	v[2] = texture2D(u_FillSampler0, topLeftTextureCoordinate).rgb;
	v[3] = texture2D(u_FillSampler0, bottomRightTextureCoordinate).rgb;
	v[4] = texture2D(u_FillSampler0, leftTextureCoordinate).rgb;
	v[5] = texture2D(u_FillSampler0, rightTextureCoordinate).rgb;

	P_COLOR vec3 temp;

	mnmx6(v[0], v[1], v[2], v[3], v[4], v[5]);

	v[5] = texture2D(u_FillSampler0, bottomTextureCoordinate).rgb;
	mnmx5(v[1], v[2], v[3], v[4], v[5]);

	v[5] = texture2D(u_FillSampler0, topTextureCoordinate).rgb;
	mnmx4(v[2], v[3], v[4], v[5]);

	P_COLOR vec4 middle_sample;
	middle_sample = texture2D(u_FillSampler0, texCoord);

	v[5] = middle_sample.rgb;
	mnmx3(v[3], v[4], v[5]);

	return vec4(v[4], middle_sample.a);
}
]]

return kernel

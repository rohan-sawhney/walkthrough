#version 400 core
in vec2 texCoords;

uniform sampler2D lastMip;
uniform ivec2 lastMipSize;

void main(void)
{
	vec4 texels;
	texels.x = texture(lastMip, texCoords).x;
	texels.y = textureOffset(lastMip, texCoords, ivec2(-1, 0)).x;
	texels.z = textureOffset(lastMip, texCoords, ivec2(-1,-1)).x;
	texels.w = textureOffset(lastMip, texCoords, ivec2( 0,-1)).x;
	
	float maxZ = max(max(texels.x, texels.y), max(texels.z, texels.w));

	vec3 extra;
	if ((2*int(gl_FragCoord.x) + 1) == lastMipSize.x-1) {
        // if we are reducing an odd-width texture then the edge fragments have to fetch additional texels
		// if both edges are odd, fetch the top-left corner texel
		if ((2*int(gl_FragCoord.y) + 1) == lastMipSize.y-1) {
			extra.z = textureOffset(lastMip, texCoords, ivec2(1, 1)).x;
			maxZ = max(maxZ, extra.z);
		}
		extra.x = textureOffset(lastMip, texCoords, ivec2(1, 0)).x;
		extra.y = textureOffset(lastMip, texCoords, ivec2(1,-1)).x;
		maxZ = max(maxZ, max(extra.x, extra.y));
	
    } else if ((2*int(gl_FragCoord.y) + 1) == lastMipSize.y-1) {
        // if we are reducing an odd-height texture then the edge fragments have to fetch additional texels
		extra.x = textureOffset(lastMip, texCoords, ivec2( 0, 1)).x;
		extra.y = textureOffset(lastMip, texCoords, ivec2(-1, 1)).x;
		maxZ = max(maxZ, max(extra.x, extra.y));
	}
	
	gl_FragDepth = maxZ;
}

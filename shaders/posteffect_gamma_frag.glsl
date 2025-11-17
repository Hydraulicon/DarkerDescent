////////////////////////////////////////////////////////
// PostEffect Gamma - Fragment Shader
//
// Applies gamma correction to the final image
// Replaces SDL3's removed hardware gamma functionality
////////////////////////////////////////////////////////

#version 120
#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect diffuseMap;
@define sampler_diffuseMap 0

uniform float afGamma;

void main()
{
	vec2 vUV = gl_FragCoord.xy;
	vec3 vColor = texture2DRect(diffuseMap, vUV).xyz;

	// Apply gamma correction: output = input^(1/gamma)
	// Clamp to avoid negative values from pow()
	vColor = max(vColor, vec3(0.0));
	vColor = pow(vColor, vec3(1.0 / afGamma));

	gl_FragColor.xyz = vColor;
	gl_FragColor.w = 1.0;
}

#version 420 core

uniform sampler2D depthMap;
uniform sampler2D noiseMap;
uniform sampler2D normalMap;

uniform vec4 cameraRange;	// x= near, y= far, z= near*far, w= far-near
uniform float radiusSSAO;

const float totStrength = 1.38;
const float strength = 0.07;
const float offset = 18.0;
const float falloff = 0.000002;
//const float rad = 0.04; //0.006;

// these are the random vectors inside a unit sphere
const vec3 pSphere[10] = vec3[10](vec3(-0.010735935, 0.01647018, 0.0062425877),vec3(-0.06533369, 0.3647007, -0.13746321),vec3(-0.6539235, -0.016726388, -0.53000957),vec3(0.40958285, 0.0052428036, -0.5591124),vec3(-0.1465366, 0.09899267, 0.15571679),vec3(-0.44122112, -0.5458797, 0.04912532),vec3(0.03755566, -0.10961345, -0.33040273),vec3(0.019100213, 0.29652783, 0.066237666),vec3(0.8765323, 0.011236004, 0.28265962),vec3(0.29264435, -0.40794238, 0.15964167));


in vec2 v_texcoord;
out vec4 color;


float readDepth(in vec2 uv)
{
  float n = cameraRange.x; // camera z near
  float f = cameraRange.y; // camera z far
  float z = texture2D(depthMap, uv).x;
  return (2.0 * n) / (f + n - z * cameraRange.w);
}

#define SAMPLES 10
const float invSamples = -1.38/10.0;

void main(void)
{
	// grab a normal for reflecting the sample rays later on
	vec3 fres = normalize((texture2D(noiseMap,v_texcoord*offset).xyz*2.0) - vec3(1.0));

	vec4 currentPixelSample = texture2D(normalMap,v_texcoord);
	float currentPixelDepth = readDepth(v_texcoord);

	// current fragment coords in screen space
	vec3 ep = vec3(v_texcoord,currentPixelDepth);
	// get the normal of current fragment
	vec3 norm = normalize(currentPixelSample.xyz*2.0-1.0);

	float bl = 0.0;
	// adjust for the depth ( not sure if this is good..)
	float radD = radiusSSAO;///currentPixelDepth;

	//vec3 ray, se, occNorm;
	float occluderDepth, depthDifference;
	vec4 occluderFragment;
	vec3 ray;
	for(int i=0; i < SAMPLES; ++i)
	{
		// get a vector (randomized inside of a sphere with radius 1.0) from a texture and reflect it
		ray = radD*reflect(pSphere[i],fres);

		// get the depth of the occluder fragment
		vec2 ocuv = ep.xy + sign(dot(ray,norm) )*ray.xy;
		occluderFragment.xyz = normalize(texture2D(normalMap, ocuv).xyz*2.0-1.0);
		occluderFragment.a = readDepth(ocuv);

		// if depthDifference is negative = occluder is behind current fragment
		depthDifference = currentPixelDepth-occluderFragment.a;

		// calculate the difference between the normals as a weight
		// the falloff equation, starts at falloff and is kind of 1/x^2 falling
		bl += step(falloff,depthDifference)*(1.0-dot(occluderFragment.xyz,norm))*(1.0-smoothstep(falloff,strength,depthDifference));
	}
	float ao1 = 1.0+bl*invSamples;


	bl = 0.0;
	radD *= 1.5;

	for(int i=0; i < SAMPLES; ++i)
	{
		// get a vector (randomized inside of a sphere with radius 1.0) from a texture and reflect it
		ray = radD*reflect(pSphere[i],fres);

		// get the depth of the occluder fragment
		vec2 ocuv = ep.xy + sign(dot(ray,norm) )*ray.xy;
		occluderFragment.xyz = normalize(texture2D(normalMap, ocuv).xyz*2.0-1.0);
		occluderFragment.a = readDepth(ocuv);

		// if depthDifference is negative = occluder is behind current fragment
		depthDifference = currentPixelDepth-occluderFragment.a;

		// calculate the difference between the normals as a weight
		// the falloff equation, starts at falloff and is kind of 1/x^2 falling
		bl += step(falloff,depthDifference)*(1.0-dot(occluderFragment.xyz,norm))*(1.0-smoothstep(falloff,strength,depthDifference));
	}
	float ao2 = 1.0+bl*invSamples;

	float ao = ao1*ao2;
	color = vec4(ao, ao, ao, 1.0);
}


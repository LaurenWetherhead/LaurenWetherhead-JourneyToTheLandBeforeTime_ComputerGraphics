#version 330 core

layout (std140) uniform camera
{
	mat4 projection;
	mat4 view;
	mat4 pvm;
	mat4 ortho;
	vec4 position;
};

/* set light ubo. do not modify.*/
struct light
{
	ivec4 att; 
	vec4 pos; // position
	vec4 dir;
	vec4 amb; // ambient intensity
	vec4 dif; // diffuse intensity
	vec4 spec; // specular intensity
	vec4 atten;
	vec4 r;
};
layout(std140) uniform lights
{
	vec4 amb;
	ivec4 lt_att; // lt_att[0] = number of lights
	light lt[4];
};

uniform float iTime;
uniform mat4 model;		/*model matrix*/

in vec3 vtx_pos;
// My vtx_uv
in vec2 vtx_uv;

out vec4 frag_color;


uniform vec3 ka;            /* object material ambient */
uniform vec3 kd;            /* object material diffuse */
uniform vec3 ks;            /* object material specular */
uniform float shininess;    /* object material shininess */


// My tex_color
uniform sampler2D tex_color;    /* texture sampler for color */

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// HASH 
vec2 hash2(vec2 v)
{
	vec2 rand = vec2(0,0);
	
	rand  = 50.0 * 1.05 * fract(v * 0.3183099 + vec2(0.71, 0.113));
    rand = -1.0 + 2 * 1.05 * fract(rand.x * rand.y * (rand.x + rand.y) * rand);
	return rand;
}

////////////////////////////////////////////////////////////////////////////////
// T E C H N I C A L   I M P L E M E N T A T I O N // 

// ADD 2D WORLEY COMPONENT --> 
float h(float v) //r
{
 	return fract(cos(v*89.42)*343.42);
}

vec2 h(vec2 v) //n
{
 	return vec2(h(v.x*23.62-300.0+v.y*34.35),h(v.x*45.13+256.0+v.y*38.89)); 
}

// 2D Worley
float worley(in vec2 v)
{
    float distance = 2.0;
	//float dist = 0.0;
    for (int i = -1; i <= 1; i++) 
    {
        for (int j = -1; j <= 1; j++) 
        {
            // Neighbor place in the grid
            vec2 k = floor(v) + vec2(i,j); // p

            float dist = length(h(k) + vec2(i, j) - fract(v));
            if (distance > dist)
            {
             	distance = dist;   
            }
        }
    }
    
    return 1.0 - distance;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PERLIN NOISE 
float perlin_noise(vec2 v) 
{
    float noise = 0;
	vec2 i = floor(v);
    vec2 f = fract(v);
    vec2 m = f*f*(3.0-2.0*f);
	
	noise = mix( mix( dot( hash2(i + vec2(0.0, 0.0)), f - vec2(0.0,0.0)),
					 dot( hash2(i + vec2(1.0, 0.0)), f - vec2(1.0,0.0)), m.x),
				mix( dot( hash2(i + vec2(0.0, 1.0)), f - vec2(0.0,1.0)),
					 dot( hash2(i + vec2(1.0, 1.0)), f - vec2(1.0,1.0)), m.x), m.y);
	return noise;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NOISE OCTAVE
float noiseOctave(vec2 v, int num)
{
	float sum = 0;
	for(int i =0; i<num; i++){
		sum += pow(2,-1*i) *worley(pow(2,i) *v);// + 
//              (0.5 * voronoi2D(v * 2.)) + 
 //             (0.25 * voronoi2D(v * 4.)));//pow(2,-1*i) * perlin_noise(pow(2,i) * v);
//;
	}
	return sum;
}

float height(vec2 v){
    float h = 0;
	h = 0.75 * noiseOctave(v, 5);
	if(h<0) h *= .5;
	return h * 2./20;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COMPUTE NORMAL
vec3 compute_normal(vec2 v, float d)
{	
	vec3 normal_vector = vec3(0,0,0);
	vec3 v1 = vec3(v.x + d, v.y, height(vec2(v.x + d, v.y)));
	vec3 v2 = vec3(v.x - d, v.y, height(vec2(v.x - d, v.y)));
	vec3 v3 = vec3(v.x, v.y + d, height(vec2(v.x, v.y + d)));
	vec3 v4 = vec3(v.x, v.y - d, height(vec2(v.x, v.y - d)));
	
	normal_vector = normalize(cross(v1-v2, v3-v4));
	return normal_vector;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SHADING PHONG
uniform samplerCube skybox;
vec4 shading_phong(light li, vec3 e, vec3 p, vec3 s, vec3 n) 
{
    vec3 v = normalize(e - p);
    vec3 l = normalize(s - p);
    vec3 r = normalize(reflect(-l, n));

    vec3 ambColor = ka * li.amb.rgb;
    vec3 difColor = kd * li.dif.rgb * max(0., dot(n, l));
    vec3 specColor = ks * li.spec.rgb * pow(max(dot(v, r), 0.), shininess);

	vec3 R = reflect(v, normalize(n));
    vec3 color = texture(skybox, vec3(R.x, -R.y, -R.z)).rgb;
    frag_color = vec4(color, 1.0);


    return vec4(0.2*color+ambColor + difColor + specColor, 1);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Draw the terrain
vec3 shading_terrain(vec3 pos) {
//	pos.z=height(vec2(pos.x, pos.y));
	vec3 n = compute_normal(pos.xy, 0.01);
	vec3 e = position.xyz;
	vec3 p = pos.xyz;
	vec3 s = lt[0].pos.xyz;

    n = normalize((model * vec4(n, 0)).xyz);
    p = (model * vec4(p, 1)).xyz;

    vec3 color = shading_phong(lt[0], e, p, s, n).xyz;

	float h = abs(pos.z) ;
	h = h*7;
	vec3 emissiveColor = mix(vec3(0.5,0.5,1.0), vec3(0.7,1.0,1.0),h);

	return color * emissiveColor;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

vec4 shading_texture_with_color() 
{

    vec4 color = vec4(0.0);     //// we set the default color to be black, update its value in your implementation below
    vec2 uv = vtx_uv;           //// the uv coordinates you need to read texture values

    /* your implementation starts */

    color = texture(tex_color, uv); // Read texture value from the texture sampler named tex_color with uv //vec2(0,0)
	//color = vec4(1.,0.,0.,1.);

    /* your implementation ends */

    return color;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void main()
{
    //frag_color = vec4(shading_terrain(vtx_pos), 1.0);
	vec3 tcolor=shading_terrain(vtx_pos);
	vec3 wcolor=(shading_texture_with_color().xyz) * 1;

	vec4 color = vec4(tcolor * wcolor,1.);
	frag_color = color;

}

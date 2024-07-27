#version 330 core

layout(std140) uniform camera
{
    mat4 projection;
    mat4 view;
    mat4 pvm;
    mat4 ortho;
    vec4 position;
};

vec2 hash2(vec2 v)
{
    vec2 rand = vec2(0, 0);

	// Your implementation starts here

	// example hash function
    rand = 50.0 * 1.05 * fract(v * 0.3183099 + vec2(0.71, 0.113));
    rand = -1.0 + 2 * 1.05 * fract(rand.x * rand.y * (rand.x + rand.y) * rand);

	// Your implementation ends here

    return rand;
}

float perlin_noise(vec2 v)
{
	// Your implementation starts here

    float noise = 0;
    vec2 i = floor(v);
    vec2 f = fract(v);
    vec2 m = f * f * (3.0 - 2.0 * f);

    noise = mix(mix(dot(hash2(i + vec2(0.0, 0.0)), f - vec2(0.0, 0.0)), dot(hash2(i + vec2(1.0, 0.0)), f - vec2(1.0, 0.0)), m.x), mix(dot(hash2(i + vec2(0.0, 1.0)), f - vec2(0.0, 1.0)), dot(hash2(i + vec2(1.0, 1.0)), f - vec2(1.0, 1.0)), m.x), m.y);
	// Your implementation ends here
    return noise;
}

////////////////////////////////////////////////////////////////////////////////
// T E C H N I C A L   I M P L E M E N T A T I O N // 

// ADD 2D WORLEY COMPONENT --> 
float h(float v)
{
 	return fract(cos(v*89.42)*343.42);
}

vec2 h(vec2 v)
{
 	return vec2(h(v.x*23.62-300.0+v.y*34.35),h(v.x*45.13+256.0+v.y*38.89)); 
}
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

uniform mat4 model;		/*model matrix*/

/*input variables*/
layout(location = 0) in vec4 pos;

/*output variables*/
out vec3 vtx_pos;		////vertex position in the world space
out vec2 vtx_uv;

void main()
{
    vtx_pos = pos.xyz;
    vtx_pos.z = height(pos.xy);
    vtx_uv = vec2(vtx_pos.x,vtx_pos.y);
    
    gl_Position = pvm * model * vec4(vtx_pos, 1.);
}
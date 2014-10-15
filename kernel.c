constant int MAX_STEPS = 10;
constant float MIN_DIST = .1f;
constant float SCALE = 2;
constant float BAILOUT = 1000;

//float DE(float3);
float raymarch(float3, float3);
/*
float DE(float3 p) {
    float r = p.x*p.x + p.y*p.y + p.z*p.z;
    
    float t;
    int i;
    
    for(i = 0; i < 10 && r < BAILOUT; i++) {
        if(p.x+p.y < 0) {
            t = -p.y;
            p.y = -p.x;
            p.x = t;
        }
        if(p.x+p.z < 0) {
            t = -p.z;
            p.z = -p.x;
            p.x = t;
        }
        if(p.y+p.z < 0) {
            t = -p.z;
            p.z = -p.y;
            p.y = t;
        }
        
        p.x = SCALE*p.x - (SCALE-1);
        p.y = SCALE*p.y - (SCALE-1);
        p.z = SCALE*p.z - (SCALE-1);
        r = p.x*p.x + p.y*p.y + p.z*p.z;
    }
    
    return (sqrt(r)-2) * pow(SCALE,-i);
}*/

float raymarch(float3 origin, float3 dir) {
    float totalDistance = 0.0f;
	int steps;
	for (steps=0; steps < MAX_STEPS; steps++) {
		float3 p = origin + totalDistance * dir;
		//float distance = DE(p);
        float distance = 1;
		totalDistance += distance;
		if (distance < MIN_DIST) break;
	}
	return 1.0f-float(steps)/float(MAX_STEPS);
}

__kernel void kern( __global float4 *dst, uint vW, uint vH, __global float* viewMat ) {
    float aspect = (float)(vW) / (float)(vH);
    
    float x = (float)(get_global_id(0) % vW) / (float)(vW);
	float y = (float)(get_global_id(0) / vW) / (float)(vH);
    
    x = (x -0.5f)*aspect;
    y = y -0.5f;
    
	float3 origin = float3(0, 0, -1);
    float3 dir	= normalize(float3(x, y, 0) - origin);
    
    float g = raymarch(origin, dir);
    
    dst[get_global_id(0)] = float4(g,g,g,1);
}



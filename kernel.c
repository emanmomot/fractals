
constant int MAX_STEPS = 100;
constant float MIN_DIST = .0001f;
constant float SCALE = 1.3f;
constant float BAILOUT = 1000;

constant float CX = 1;
constant float CY = 1;
constant float CZ = 1;

constant float rot = 1.0f/32.0f;
constant float rot2 = 1.0f/16.0f;

void rotate1(float3* p) {
	p->x = p->z*sinpi(rot) + p->x*cospi(rot);
	p->z = p->z*cospi(rot) - p->x*sinpi(rot);
}

void rotate2(float3* p) {
	p->y = p->y*cospi(rot2) - p->z*sinpi(rot2);
	p->z = p->y*sinpi(rot2) - p->z*cospi(rot2);
}

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
        
        p.x = SCALE*p.x - CX*(SCALE-1.0f);
        p.y = SCALE*p.y - CY*(SCALE-1.0f);
        p.z = SCALE*p.z - CZ*(SCALE-1.0f);
        r = p.x*p.x + p.y*p.y + p.z*p.z;
    }
    
    return (sqrt(r)-2.0f) * pow(SCALE,-i);
}

float raymarch(float3 origin, float3 dir) {
    float totalDistance = 0.0f;
	int steps;
	for (steps=0; steps < MAX_STEPS; steps++) {
		float3 p = origin + totalDistance * dir;
		float distance = DE(p);
		totalDistance += distance;
		if (distance < MIN_DIST) break;
	}
	return 1.0f-(float)(steps)/(float)(MAX_STEPS);
}

float3 matVec(__global float* matrix, float3* vector){
	float3 result;
	result.x = matrix[0]*((*vector).x)+matrix[4]*((*vector).y)+matrix[8]*((*vector).z)+matrix[12];
	result.y = matrix[1]*((*vector).x)+matrix[5]*((*vector).y)+matrix[9]*((*vector).z)+matrix[13];
	result.z = matrix[2]*((*vector).x)+matrix[6]*((*vector).y)+matrix[10]*((*vector).z)+matrix[14];
	return result;
}

__kernel void kern( __global float4 *dst, uint vW, uint vH, __global float* viewMat ) {
    float aspect = (float)(vW) / (float)(vH);
    
    float x = (float)(get_global_id(0) % vW) / (float)(vW);
	float y = (float)(get_global_id(0) / vW) / (float)(vH);
    
    x = (x -0.5f)*aspect;
    y = y -0.5f;
    
    float3 o = (float3)(0, 0, -5);
	float3 origin = matVec(viewMat, &o);
	
	float3 d = (float3)(x, y, 0);
    float3 dir	= normalize(matVec(viewMat, &d) - origin);
    
    float g = raymarch(origin, dir);
    
    dst[get_global_id(0)] = (float4)(g,g,g,1);
}



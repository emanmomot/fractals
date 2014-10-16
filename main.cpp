
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#include <GLUT/glut.h>
#else
#include <CL/cl.h>
#include <stdlib.h>
#include <GLFW/glfw3.h>
#endif

#include <iostream>
#include <fstream>
#include <cstring>

#include "Camera.h"
#include "Input.h"

int vW;
int vH;

size_t global_work_size;

float viewMat[16];

cl_kernel kernel;
cl_context context;
cl_command_queue queue;

cl_mem screenBuffer;
cl_mem viewTransform;

GLuint screenTex;

Camera* playerCam;
Input* input;

void
initScene() {
	playerCam = new Camera(vec3(0,0,20),.1f);
	input = new Input(vW, vH);
}

void
initOpenGL() {
    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &screenTex);
}

void
checkErr(cl_int err) {
    if(err != CL_SUCCESS)
		std::cout<<"err   :  " << err << std::endl;
}

void
initOpenCL() {
    // 1. Get a platform.
	cl_platform_id platform[2];
	cl_int err;
    
	cl_uint numplats;
	err = clGetPlatformIDs( 2, platform, &numplats );

	// 2. Find a gpu device.
	cl_device_id device;
    
	err = clGetDeviceIDs( platform[1], CL_DEVICE_TYPE_GPU,
                   1,
                   &device,
                   NULL);

	// 3. Create a context and command queue on that device.
	context = clCreateContext( NULL,
                                         1,
                                         &device,
                                         NULL, NULL, &err);
                                         
	queue = clCreateCommandQueue( context,
                                 device,
                                 0, NULL );
	// 4. Perform runtime source compilation, and obtain kernel entry point.
	std::ifstream file("kernel.c");
	std::string source;
	while(!file.eof()){
		char line[256];
		file.getline(line,255);
		source += line;
	}
    
	cl_ulong maxSize;
	clGetDeviceInfo(device, CL_DEVICE_MAX_MEM_ALLOC_SIZE , sizeof(cl_ulong), &maxSize, 0);
    
	const char* str = source.c_str();
	cl_program program = clCreateProgramWithSource( context,
                                                   1,
                                                   &str,
                                                   NULL, NULL );
    
    const char options[] = "-Werror -cl-std=CL1.1";
	err = clBuildProgram( program, 1, &device, options, NULL, NULL );
    
	if ( err != CL_SUCCESS ) {
        char* programLog;
        size_t logSize;
        cl_build_status status;
        
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_STATUS,
                              sizeof(cl_build_status), &status, NULL);
        
		// check build log
        clGetProgramBuildInfo(program, device,
                              CL_PROGRAM_BUILD_LOG, 0, NULL, &logSize);
        programLog = (char*) calloc (logSize+1, sizeof(char));
        clGetProgramBuildInfo(program, device,
                              CL_PROGRAM_BUILD_LOG, logSize+1, programLog, NULL);
        
        printf("Build failed; error=%d, programLog:nn%s",
               err, programLog);
        free(programLog);
	}
    
    
	kernel = clCreateKernel( program, "kern", NULL );
	// 5. Create a data buffer.
	
	viewTransform = clCreateBuffer( context,
                                   CL_MEM_READ_WRITE,
                                   16 *sizeof(cl_float),
                                   NULL, &err );
    
	clSetKernelArg(kernel, 3, sizeof(viewTransform), (void*) &viewTransform);
    	
	clFinish(queue);
}

void
update() {

	input->update(playerCam);
	playerCam->setMatrices();
	glGetFloatv(GL_MODELVIEW_MATRIX, viewMat);

}

void
draw() {
	update();
	 
    // 7. Execute the kernel
	cl_int err = clEnqueueNDRangeKernel( queue,
                           kernel,
                           1,
                           NULL,
                           &global_work_size,
                           NULL, 0, NULL, NULL);
                           
    
	// 8. Look at the results via synchronous buffer map.
	cl_float4 *ptr = (cl_float4 *) clEnqueueMapBuffer( queue,
														screenBuffer,
														CL_TRUE,
														CL_MAP_READ,
														0,
														vW * vH * sizeof(cl_float4),
														0, NULL, NULL, &err );

	checkErr(err);
																											
	cl_float* viewMatPtr = (cl_float *) clEnqueueMapBuffer( queue,
														   viewTransform,
														   CL_TRUE,
														   CL_MAP_WRITE,
														   0,
														   16 * sizeof(cl_float),
														   0, NULL, NULL, &err );
														   

												
	memcpy(viewMatPtr, viewMat, sizeof(float)*16);
	
    unsigned char* pixels = new unsigned char[vW*vH*4];
	for(int i=0; i <  vW * vH; i++){
		pixels[i*4] = ptr[i].s[0]*255;
		pixels[i*4+1] = ptr[i].s[1]*255;
		pixels[i*4+2] = ptr[i].s[2]*255;
		pixels[i*4+3] = 1;
	}

	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, vW, vH, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	delete [] pixels;

	clEnqueueUnmapMemObject(queue, viewTransform, viewMatPtr, 0, 0, 0);
	clEnqueueUnmapMemObject(queue, screenBuffer, ptr, 0, 0, 0);
    
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1,1,1,-1,1,100);
	glMatrixMode(GL_MODELVIEW);
    
    glPushMatrix();
	glLoadIdentity();
	glBegin(GL_QUADS);
	glTexCoord2f(0,1);
	glVertex3f(-1,-1,-1);
	glTexCoord2f(0,0);
	glVertex3f(-1,1,-1);
	glTexCoord2f(1,0);
	glVertex3f(1,1,-1);
	glTexCoord2f(1,1);
	glVertex3f(1,-1,-1);
	glEnd();
	glPopMatrix();
	
    glutSwapBuffers();
    glutPostRedisplay();
    
    clFinish(queue);
}

void
reshape(int w, int h) {
	vW = w;
	vH = h;
	
	screenBuffer = clCreateBuffer( context,
							   CL_MEM_WRITE_ONLY,
							   vW * vH *sizeof(cl_float4),
							   NULL, NULL );

	clSetKernelArg(kernel, 0, sizeof(screenBuffer), (void*) &screenBuffer);
	clSetKernelArg(kernel, 1, sizeof(cl_uint), (void*) &vW);
	clSetKernelArg(kernel, 2, sizeof(cl_uint), (void*) &vH);
	
	global_work_size = w * h;
	
	glViewport(0, 0, w, h);
	input->setViewport(w, h);

	unsigned char* pixels = new unsigned char[vW*vH*4];
	glBindTexture(GL_TEXTURE_2D, screenTex);
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexImage2D(GL_TEXTURE_2D, 0, 4, vW, vH, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	delete [] pixels;
}

void
keyboard(unsigned char key, int x, int y) {
	input->keyboard(key,x,y);
}

void 
keyboardUp(unsigned char key, int x, int y) {
	input->keyboardUp(key,x,y);
}

void
passivemotion( int x, int y ) {
	input->passivemotion(x,y);
}

int
main(int argc, char *argv[]) {

	initOpenCL();

	initOpenGL();

    initScene();

	 GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;

}


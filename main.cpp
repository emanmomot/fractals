
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#include <GLUT/glut.h>
#else
#include <CL/cl.h>
#include <GL/glut.h>
#endif

#include <iostream>
#include <fstream>

const int vW = 500;
const int vH = 500;

size_t global_work_size = vW * vH;

cl_kernel kernel;
cl_context context;
cl_command_queue queue;

cl_mem screenBuffer;

void
initOpenGL() {
    glEnable(GL_TEXTURE_2D);
}

void
initOpenCL() {
    // 1. Get a platform.
	cl_platform_id platform;
    
	clGetPlatformIDs( 1, &platform, NULL );
	// 2. Find a gpu device.
	cl_device_id device;
    
	clGetDeviceIDs( platform, CL_DEVICE_TYPE_GPU,
                   1,
                   &device,
                   NULL);
	// 3. Create a context and command queue on that device.
	cl_context context = clCreateContext( NULL,
                                         1,
                                         &device,
                                         NULL, NULL, NULL);
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
	cl_int err = clBuildProgram( program, 1, &device, options, NULL, NULL );
    
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
	screenBuffer        = clCreateBuffer( context,
                                   CL_MEM_WRITE_ONLY,
                                   vW * vH *sizeof(cl_float4),
                                   NULL, 0 );
	cl_mem viewTransform = clCreateBuffer( context,
                                   CL_MEM_READ_WRITE,
                                   16 *sizeof(cl_float),
                                   NULL, 0 );
    
	clSetKernelArg(kernel, 0, sizeof(screenBuffer), (void*) &screenBuffer);
	clSetKernelArg(kernel, 1, sizeof(cl_uint), (void*) &vW);
	clSetKernelArg(kernel, 2, sizeof(cl_uint), (void*) &vH);
	clSetKernelArg(kernel, 3, sizeof(viewTransform), (void*) &viewTransform);
    
    // 6. Fill input data buffers
	cl_float* viewMatPtr = (cl_float *) clEnqueueMapBuffer( queue,
                                                           viewTransform,
                                                           CL_TRUE,
                                                           CL_MAP_WRITE,
                                                           0,
                                                           16 * sizeof(cl_float),
                                                           0, NULL, NULL, NULL );
    
	viewMatPtr[0] = viewMatPtr[5] = viewMatPtr[10] = viewMatPtr[15] = 1;
    
	clEnqueueUnmapMemObject(queue, viewTransform, viewMatPtr, 0, 0, 0);
}

void
update() {
    
}

void
draw() {
    
    // 7. Execute the kernel
	clEnqueueNDRangeKernel( queue,
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
                                                            0, NULL, NULL, NULL );
    
    
    unsigned char* pixels = new unsigned char[vW*vH*4];
	for(int i=0; i <  vW * vH; i++){
		pixels[i*4] = ptr[i].s[0]*255;
		pixels[i*4+1] = ptr[i].s[1]*255;
		pixels[i*4+2] = ptr[i].s[2]*255;
		pixels[i*4+3] = 1;
	}
    
	glBindTexture(GL_TEXTURE_2D, 1);
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexImage2D(GL_TEXTURE_2D, 0, 4, vW, vH, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	delete [] pixels;
    
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1,1,1,-1,1,100);
	glMatrixMode(GL_MODELVIEW);
    
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
	
    glutSwapBuffers();
    glutPostRedisplay();
    
    clFinish(queue);
}

int
main(int argc, char *argv[]) {
	glutInit(&argc, argv);
    
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(vW, vH);
	glutCreateWindow("Raytrace");
    
	glutDisplayFunc(draw);
    //glutKeyboardFunc(keyboard);
    
	//glClearColor(ambient[0], ambient[1], ambient[2], ambient[3]);
    
    initOpenGL();
    initOpenCL();
    
	glutMainLoop();
    
    return 0;
}


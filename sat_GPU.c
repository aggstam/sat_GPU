// -----------------------------------------------------------------------
//
// This program solves the Propositional (Boolean) Satisfiability problem
// using Depth-First Search algorithm and validates each vector using OpenCl.
// Problems are read from an input file, while solution is written to screen
// and an output file.
//
// Author: Aggelos Stamatiou, April 2017
//
// -----------------------------------------------------------------------

#include <CL/cl.h>

// Common code file
#include "utils.c"

// OpenCl global variables
int WI;			// Work items.
cl_int status;
cl_context context;
cl_command_queue cmdQueue;
cl_kernel kernel;
size_t globalWorkSize[1];
cl_mem d_problem;
cl_mem d_finish;
int d_step;		// Used to define starting index in Problem vector for each work item.

// Extra timer.
float communication_time;

// GPU timers.
cl_event myEvent;
cl_ulong startTimeNs, endTimeNs;
float GPU_run_time_sum;

// Auxiliary function that displays a message in case of wrong input parameters.
void syntax_error(char **argv) {
	printf("Wrong syntax. Use the following:\n\n");
	printf("%s <work items> <inputfile>\n\n", argv[0]);
	printf("where:\n");
	printf("<work items> = number of computing units of the graphics card\n");
	printf("<inputfile> = name of the file with the problem description\n");
	printf("Program terminates.\n");
}

// Reading the kernel file.
char* readSource(const char *sourceFilename) {
	FILE *fp;
	int err;
	char *source;
	int size;

	fp = fopen(sourceFilename, "rb");
	if (fp == NULL) {
		printf("Could not open kernel file: %s\n", sourceFilename);
		exit(-1);
	}
	err = fseek(fp, 0, SEEK_END);
	if (err != 0) {
		printf("Error seeking to end of file.\n");
		exit(-1);
	}
	size = ftell(fp);
	if (size < 0) {
		printf("Error getting file position.\n");
		exit(-1);
	}
	err = fseek(fp, 0, SEEK_SET);
	if (err != 0) {
		printf("Error seeking to start of file.\n");
		exit(-1);
	}
	source = (char*)malloc(size + 1);
	if (source == NULL) {
		printf("Error allocating %d bytes for the program source.\n", size + 1);
		exit(-1);
	}
	err = fread(source, 1, size, fp);
	if (err != size) {
		printf("only read %d bytes.\n", err);
		exit(0);
	}
	source[size] = '\0';
	return source;
}

// This function checks whether a current partial assignment is already invalid using the GPU. 
// In order for a partial assignment to be invalid, there should exist a clause such that
// all propositions in the clause have already value and their values are such that 
// the clause is false. We validate the vector by counting how many clauses are valid.
// In order for the vector to be invalid, count is less than K (number of clauses).
int valid(struct frontier_node *node) {
	// Pass the vector to GPU.
	cl_mem d_vector;
	d_vector = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, N * sizeof(int), node->vector, &status);
	if (status != CL_SUCCESS || d_vector == NULL) {
		printf("clCreateBuffer failed\n");
		exit(-1);
	}

	// Create a partial sums table for the GPU.
	int *partial_sums = (int*)malloc(WI * sizeof(int));
	for (int i = 0; i < WI; i++) {
		partial_sums[i] = 0;
	}
	cl_mem d_partial_sums;
	d_partial_sums = clCreateBuffer(context, CL_MEM_WRITE_ONLY, WI * sizeof(int), NULL, &status);
	if (status != CL_SUCCESS || d_partial_sums == NULL) {
		printf("clCreateBuffer failed\n");
		exit(-1);
	}

	// Set kernel arguments.
	status = clSetKernelArg(kernel, 1, sizeof(cl_mem), &d_vector);
	status |= clSetKernelArg(kernel, 3, sizeof(cl_mem), &d_partial_sums);
	if (status != CL_SUCCESS) {
		printf("clSetKernelArg failed\n");
		exit(-1);
	}

	clock_t S_idle_timer = clock();

	// Run kernel.
	status = clEnqueueNDRangeKernel(cmdQueue, kernel, 1, NULL, globalWorkSize, NULL, 0, NULL, &myEvent);
	if (status != CL_SUCCESS) {
		printf("clEnqueueNDRangeKernel failed\n");
		exit(-1);
	}

	// Copy back the memory to the host.
	status = clEnqueueReadBuffer(cmdQueue, d_partial_sums, CL_TRUE, 0, WI * sizeof(int), partial_sums, 0, NULL, NULL);
	if (status != CL_SUCCESS) {
		printf("clEnqueueReadBuffer failed\n");
		exit(-1);
	}

	clock_t E_idle_timer = clock();

	float idle_time = ((float)(E_idle_timer - S_idle_timer) / CLOCKS_PER_SEC);

	// Release OpenCL objects.
	clReleaseMemObject(d_partial_sums);
	clReleaseMemObject(d_vector);

	// Get timers values.
	clWaitForEvents(1, &myEvent);
	clGetEventProfilingInfo(myEvent, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &startTimeNs, NULL);
	clGetEventProfilingInfo(myEvent, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &endTimeNs, NULL);
	clReleaseEvent(myEvent);
	float GPU_run_time = ((endTimeNs - startTimeNs) / 1000000000.0);
	GPU_run_time_sum += GPU_run_time;

	communication_time += idle_time - GPU_run_time;

	// Reduce the partial sums.
	int sum = 0;
	for (int i = 0; i < WI; i++) {
		sum += partial_sums[i];
	}

	free(partial_sums);

	// Check validation.

	if (sum < K) {
		return 0;
	}

	return 1;
}

// Depth-First Search functions
#include "dfs.c"

int main(int argc, char **argv) {
	int err;

	srand((unsigned)time(NULL));

	if (argc != 3) {
		syntax_error(argv);
		exit(-1);
	}

	WI = atoi(argv[1]);
	
	err = readfile(argv[2]);
	if (err < 0) {
		exit(-1);
	}

	printf("\nThis OpenCL programm solves the Propositional (Boolean) Satisfiability Problem \n");
	printf("written in file %s, using Depth First Search Algorithm.\n", argv[2]);
	printf("Number of work items: %s\n\n", argv[1]);
	
	printf("Device info:\n\n");

	cl_uint numPlatforms = 0;
	cl_platform_id *platforms;
	// Query for the number of recongnized platforms.
	status = clGetPlatformIDs(0, NULL, &numPlatforms);
	if (status != CL_SUCCESS) {
		printf("clGetPlatformIDs failed. Program terminates.\n");
		exit(-1);
	}

	// Make sure some platforms were found.
	if (numPlatforms == 0) {
		printf("No platforms detected. Program terminates.\n");
		exit(-1);
	}

	// Allocate enough space for each platform.
	platforms = (cl_platform_id*)malloc(numPlatforms * sizeof(cl_platform_id));
	if (platforms == NULL) {
		perror("malloc");
		exit(-1);
	}

	// Fill in platforms.
	clGetPlatformIDs(numPlatforms, platforms, NULL);
	if (status != CL_SUCCESS) {
		printf("clGetPlatformIDs failed. Program terminates.\n");
		exit(-1);
	}

	// Print out some basic information about each platform.
	
	printf("%u platforms detected\n", numPlatforms);
	for (int i = 0; i < numPlatforms; i++) {
		char buf[100];
		printf("Platform %u: \n", i);
		status = clGetPlatformInfo(platforms[i], CL_PLATFORM_VENDOR, sizeof(buf), buf, NULL);
		printf("\tVendor: %s\n", buf);
		status |= clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, sizeof(buf), buf, NULL);
		printf("\tName: %s\n", buf);
		if (status != CL_SUCCESS) {
			printf("clGetPlatformInfo failed. Program terminates.\n");
			exit(-1);
		}
	}
	printf("\n");

	// Retrive the number of devices present.
	cl_uint numDevices = 0;
	cl_device_id *devices;
	status = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU, 0, NULL, &numDevices);
	if (status != CL_SUCCESS) {
		printf("clGetDeviceIDs failed. Program terminates.\n");
		exit(-1);
	}

	// Make sure some devices were found.
	if (numDevices == 0) {
		printf("No devices detected. Program terminates.\n");
		exit(-1);
	}

	// Allocate enough space for each device.
	devices = (cl_device_id*)malloc(numDevices * sizeof(cl_device_id));
	if (devices == NULL) {
		perror("malloc");
		exit(-1);
	}

	// Fill in devices.
	status = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU, numDevices, devices, NULL);
	if (status != CL_SUCCESS) {
		printf("clGetDeviceIDs failed. Program terminates.\n");
		exit(-1);
	}

	// Print out some basic information about each device.
	printf("%u devices detected\n", numDevices);
	for (int i = 0; i < numDevices; i++) {
		char buf[100];
		printf("Device %u: \n", i);
		status = clGetDeviceInfo(devices[i], CL_DEVICE_VENDOR, sizeof(buf), buf, NULL);
		printf("\tDevice: %s\n", buf);
		status |= clGetDeviceInfo(devices[i], CL_DEVICE_NAME, sizeof(buf), buf, NULL);
		printf("\tName: %s\n", buf);
		if (status != CL_SUCCESS) {
			printf("clGetDeviceInfo failed. Program terminates.\n");
			exit(-1);
		}
	}

	printf("\n");

	// Create a context and associate it with the devices.
	context = clCreateContext(NULL, numDevices, devices, NULL, NULL, &status);
	if (status != CL_SUCCESS || context == NULL) {
		printf("clCreateContext failed. Program terminates.\n");
		exit(-1);
	}

	// Create a command queue and associate it with the device you want to execute on.
	cmdQueue = clCreateCommandQueue(context, devices[0], CL_QUEUE_PROFILING_ENABLE, &status);
	if (status != CL_SUCCESS || cmdQueue == NULL) {
		printf("clCreateCommandQueue failed. Program terminates.\n");
		exit(-1);
	}

	cl_program program;
	char *source;
	const char *sourceFile = "cl_valid.cl";
	// This function reads in the source code of the program.
	source = readSource(sourceFile);
	// Create a program. The 'source' string is the code from the cl_valid.cl file.
	program = clCreateProgramWithSource(context, 1, (const char**)&source, NULL, &status);
	if (status != CL_SUCCESS) {
		printf("clCreateProgramWithSource failed. Program terminates.\n");
		exit(-1);
	}

	cl_int buildErr;
	// Build (compile & link) the program for the devices.
	// Save the return value in 'buildErr' (the following
	// code will print any compilation errors to the screen).
	buildErr = clBuildProgram(program, numDevices, devices, NULL, NULL, NULL);
	// If there are build errors, print them to the screen.
	if (buildErr != CL_SUCCESS) {
		printf("Program failed to build.\n");
		cl_build_status buildStatus;
		for (int i = 0; i < numDevices; i++) {
			clGetProgramBuildInfo(program, devices[i], CL_PROGRAM_BUILD_STATUS,
				sizeof(cl_build_status), &buildStatus, NULL);
			if (buildStatus == CL_SUCCESS) {
				continue;
			}
			char *buildLog;
			size_t buildLogSize;
			clGetProgramBuildInfo(program, devices[i], CL_PROGRAM_BUILD_LOG, 0,
				NULL, &buildLogSize);
			buildLog = (char*)malloc(buildLogSize);
			if (buildLog == NULL) {
				perror("malloc");
				exit(-1);
			}
			clGetProgramBuildInfo(program, devices[i], CL_PROGRAM_BUILD_LOG,
				buildLogSize, buildLog, NULL);
			buildLog[buildLogSize - 1] = '\0';
			printf("Device %u Build Log:\n%s\n", i, buildLog);
			free(buildLog);
		}
		exit(0);
	}

	// Create a kernel from the vector validation function (named "clvalid").
	kernel = clCreateKernel(program, "clvalid", &status);
	if (status != CL_SUCCESS) {
		printf("clCreateKernel failed. Program terminates.\n");
		exit(-1);
	}

	// Define an index space (global work size) of threads for execution.
	// A workgroup size (local work size) is not required, but can be used.
	// There are WI threads.
	if (K <= WI) {
		WI = K; // If K is less than threads, we use K threads.
	}
	globalWorkSize[0] = WI;

	// Define the step and finishing index for each thread.
	d_step = K / WI;

	int *finish = (int*)malloc(WI * sizeof(int)); // Finishing index of each work item.
	if (finish == NULL) {
		printf("Memory exhausted. Program terminates.\n");
		exit(-1);
	}
	for (int i = 0; i < WI - 1; i++) {
		finish[i] = d_step*(i + 1);
	}
	// Last thread gets the extra work if K mod WI != 0.
	finish[WI - 1] = K; 

	// Pass data to GPU.
	d_finish = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, WI * sizeof(int), finish, &status);
	if (status != CL_SUCCESS || d_finish == NULL) {
		printf("clCreateBuffer failed. Program terminates.\n");
		exit(-1);
	}

	d_problem = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, K*M * sizeof(int), Problem, &status);
	if (status != CL_SUCCESS || d_problem == NULL) {
		printf("clCreateBuffer failed. Program terminates.\n");
		exit(-1);
	}

	// Set kernel arguments.
	status = clSetKernelArg(kernel, 0, sizeof(cl_mem), &d_problem);
	status |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &d_finish);
	status |= clSetKernelArg(kernel, 4, sizeof(int), &d_step);
	status |= clSetKernelArg(kernel, 5, sizeof(int), &M);
	if (status != CL_SUCCESS) {
		printf("clSetKernelArg failed. Program terminates.\n");
		exit(-1);
	}

	printf("No build errors, starting solving the problem...\n");

	//display_problem();

	struct frontier_node *solution_node = search(); // The main call.

	if (solution_node != NULL) {

		printf("\nSolution found with depth-first!\n");
		printf("\nSolution vector propositions values:\n");
		display(solution_node->vector);

	}
	else {
		if (mem_error == -1) {
			printf("Memory exhausted. Program terminates.\n");
		}
		else {
			printf("\nNO SOLUTION EXISTS. Proved by depth-first!");
		}
	}

	printf("\n\nTime spent = %0.3f\n", ((float)t2 - t1) / CLOCKS_PER_SEC);
	printf("GPU execution time = %0.3f\n", GPU_run_time_sum);
	printf("Communication time = %0.3f\n", communication_time);
	
	// Cleanup OpenCL structures.
	clReleaseProgram(program);
	clReleaseKernel(kernel);
	clReleaseCommandQueue(cmdQueue);
	clReleaseMemObject(d_problem);
	clReleaseMemObject(d_finish);
	clReleaseContext(context);

	return 0;
}

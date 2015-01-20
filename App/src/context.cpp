#include "context.h"

#include <algorithm>
#include <stdexcept>
#include <sstream>

using namespace std;

Context::Context(unsigned int platform, unsigned int device, cl_device_type deviceType)
{
	cl_uint pCount = platform + 1;
	cl_platform_id* platforms = new cl_platform_id[pCount];

	clGetPlatformIDs(pCount, platforms, &pCount);
	if (platform >= pCount)
	{
		throw runtime_error("Platform ID too high.");
	}

	pid = platforms[platform];

	cl_uint dCount = device + 1;
	cl_device_id* devices = new cl_device_id[dCount];

	clGetDeviceIDs(pid, deviceType, dCount, devices, &dCount);
	if (device >= dCount)
	{
		throw runtime_error("Device ID too high.");
	}

	cl_int err;
	context = clCreateContext(nullptr, 1, devices + device, nullptr, nullptr, &err);
	if (err != CL_SUCCESS)
	{
		stringstream ss;
		ss << "clCreateContext returned with error code " << err << ".";
		throw runtime_error(ss.str());
	}

	did = devices[device];
}

Context::~Context()
{
	clReleaseContext(context);
}


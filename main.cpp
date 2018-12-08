#define __CL_ENABLE_EXCEPTIONS

#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/cl.hpp>
#else

#include <cl.hpp>

#endif

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <random>
#include <fstream>

//std::string helloStr = "__kernel void "
//                      "hello(void) "
//                      "{ "
//                      "  "
//                      "} ";

int main() {
    cl_int err = CL_SUCCESS;
    const size_t matrix_size = 1024;
    auto* matrix = new double[matrix_size];
    double min = 0.0;
    double max = 0.0;

    std::mt19937 rng;
    rng.seed(std::random_device()());
    std::uniform_int_distribution<std::mt19937::result_type> dist6(1,matrix_size);

    for (int i = 0; i < matrix_size; ++i) {
        matrix[i] = dist6(rng);
    }

    std::ifstream file("program.cl", std::ios::binary | std::ios::ate);
    std::string program((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    std::cout << program << std::endl;

    try {

        std::vector<cl::Platform> platforms;
        cl::Platform::get(&platforms);
        if (platforms.empty()) {
            std::cout << "Platform size 0\n";
            return -1;
        }

        cl_context_properties properties[] = {CL_CONTEXT_PLATFORM, (cl_context_properties) (platforms[0])(), 0};
        cl::Context context(CL_DEVICE_TYPE_GPU, properties);

        std::vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();

        cl::Program::Sources source(1,std::make_pair(program.c_str(), program.length()));
        cl::Program program_ = cl::Program(context, source);
        program_.build(devices);

        cl::Kernel kernel(program_, "normalize", &err);
        kernel.setArg(0, sizeof(double) * matrix_size, matrix);
        kernel.setArg(1, sizeof(double), &max);
        kernel.setArg(2, sizeof(double), &min);

        cl::Event event;
        cl::CommandQueue queue(context, devices[0], 0, &err);
        queue.enqueueNDRangeKernel(
                kernel,
                cl::NullRange,
                cl::NDRange(4, 4),
                cl::NullRange,
                nullptr,
                &event);

        event.wait();
    }
    catch (cl::Error &err) {
        std::cerr
                << "ERROR: "
                << err.what()
                << "("
                << err.err()
                << ")"
                << std::endl;
    }

    delete[] matrix;

    return EXIT_SUCCESS;
}
#define __CL_ENABLE_EXCEPTIONS

#include <CL/opencl.h>
#include <iostream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <random>


int main(int argc, char const* argv[]) {
    size_t pocet_prvku = 1024;
    double* matice = new double[pocet_prvku];
    std::string zdrojovy_kod = "__kernel void moje_normalizace(\n"
                               "__global double* mat, const int pocet_prvku) {\n"
                               "    const int idx = get_global_id(0);\n"
                               "    if (idx < pocet_prvku) {\n"
                               //"        mat[idx] = mat[idx] + 1;\n"
                               "        int j = 5;"
                               "    }\n"
                               "}";
    const char* p_zdrojovy_kod = zdrojovy_kod.c_str();

    cl_int error = 0;
    cl_context context;
    cl_command_queue queue;
    cl_device_id device;
    cl_mem matrix;
    cl_program program;
    cl_kernel kernel;
    size_t matrix_size = sizeof(double) * pocet_prvku;

    std::cout << "Kernel kod:\n" << zdrojovy_kod << std::endl;

    std::cout << "Hledam dostupna zarizeni..." << std::endl;
    error = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
    /* pokud ID nebylo nalezeno nebo nastala jina chyba */
    if (error != CL_SUCCESS) {
        std::cout << "Nenalezeno ID zarizeni" << std::endl;
        return -1;
    }
    std::cout << "Vytvarim kontext..." << std::endl;
    context = clCreateContext(0, 1, &device, NULL, NULL, &error);
    /* pokud se vytvoreni kontextu nezdarilo */
    if (error != CL_SUCCESS) {
        std::cout << "Kontext nevytvoren: " << error << std::endl;
        goto cleanup_context;
    }
    std::cout << "Vytvarim frontu prikazu..." << std::endl;
    queue = clCreateCommandQueueWithProperties(context, device, 0, &error);
    /* pokud se vytvoreni fronty nezdarilo */
    if (error != CL_SUCCESS) {
        std::cout << "Fronta prikazu nebyla vytvorena: " << error << std::endl;
        goto cleanup_queue;
    }

    std::cout << "Vytvarim prostor pro matici" << std::endl;
    matrix = clCreateBuffer(context, CL_MEM_READ_WRITE, matrix_size, NULL, &error);
    std::cout << "Predavam obsah matice do bufferu" << std::endl;
    error |= clEnqueueWriteBuffer(queue, matrix, CL_TRUE, 0, matrix_size, matice, 0, NULL, NULL);
    /* pokud se vytvoreni bufferu nepovedlo */
    if (error != CL_SUCCESS) {
        std::cout << "Nepodarilo se vytvorit buffer pro matici" << error << std::endl;
        goto cleanup_matrix;
    }

    std::cout << "Vytvarim program dle zdrojoveho kodu..." << std::endl;
    program = clCreateProgramWithSource(context, 1, &p_zdrojovy_kod, NULL, &error);
    /* jestlize nebylo mozne program vytvorit */
    if (error != CL_SUCCESS) {
        std::cout << "Program se nepodarilo vytvorit: " << error << std::endl;
        goto cleanup_matrix;
    }
    std::cout << "Sestavuji program..." << std::endl;
    error = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    /* jestlize nebylo mozne program sestavit */
    if (error != CL_SUCCESS) {
        std::cout << "Program se nepodarilo zkompilovat" << std::endl;
        char compiler_log[4096];
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, sizeof(compiler_log), compiler_log, NULL);
        std::cout << "OpenCL compiler failed:\n" << compiler_log << std::endl;
        goto cleanup_matrix;
    }
    std::cout << "Vytvarim kernel..." << std::endl;
    kernel = clCreateKernel(program, "moje_normalizace", &error);
    /* v pripade, ze se vytvoreni kernelu nepovedlo */
    if (error != CL_SUCCESS) {
        std::cout << "Kernel se nepodarilo vytvorit: " << error << std::endl;
        goto cleanup_kernel;
    }

    std::cout << "Vkladam parametry do kernelu..." << std::endl;
    error = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*) &matrix);
    error |= clSetKernelArg(kernel, 1, sizeof(size_t), (void*) &pocet_prvku);
    if (error != CL_SUCCESS) {
        std::cout << "Parametry nebyly predany " << error << std::endl;
        goto cleanup_kernel;
    }

    std::cout << "Spoustim kod na GPU..." << std::endl;
    error = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &pocet_prvku, NULL, 0, NULL, NULL);
    /* pokud se spusteni neprovedlo */
    if (error != CL_SUCCESS) {
        std::cout << "Nepodarilo se spustit vypocet na GPU: " << error << std::endl;
        goto cleanup_kernel;
    }
    /* cekani na vysledek operace */
    std::cout << "Cekam na dokonceni kodu na GPU..." << std::endl;
    clFinish(queue);

    std::cout << "Ctu vysledek z GPU..." << std::endl;
    error = clEnqueueReadBuffer(queue, matrix, CL_TRUE, 0, matrix_size, NULL, 0, NULL, NULL);
    /* pokud nebylo mozne vystup precist */
    if (error != CL_SUCCESS) {
        std::cout << "Vystup se nezdarilo precist: " << error << std::endl;
        goto cleanup_kernel;
    }

    std::cout << "Vse se uspesne provedlo, jdu vycistit pamet..." << std::endl;
    cleanup_kernel:
    clReleaseKernel(kernel);
    /* uvolneni pameti programu */
    cleanup_program:
    clReleaseProgram(program);
    /* uvolneni pameti 1. vstupni matice */
    cleanup_matrix:
    clReleaseMemObject(matrix);
    /* uvolneni pameti fronty prikazu */
    cleanup_queue:
    clReleaseCommandQueue(queue);
    /* uvolneni pameti OpenCL kontextu */
    cleanup_context:
    clReleaseContext(context);
    delete[] matice;
}
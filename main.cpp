#define __CL_ENABLE_EXCEPTIONS

#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/cl.hpp>
#else

#include <cl.hpp>

#endif

#include <iostream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <random>

const char* program = "__kernel void normalize() {}";

int main(int argc, char const *argv[])
{
    size_t pocet_prvku = 1024;
    double* matice = new double[pocet_prvku];

    cl_int error = 0;
    cl_context context;
    cl_command_queue queue;
    cl_device_id device;
    cl_mem matrix;
    cl_program program;
    cl_kernel kernel;
    size_t matrix_size = sizeof(double) * pocet_prvku;

    /* ziskani ID GPU zarizeni */
    error = clGetDeviceIDs(NULL,CL_DEVICE_TYPE_GPU,1,&device,NULL);
    /* pokud ID nebylo nalezeno nebo nastala jina chyba */
    if(error != CL_SUCCESS){
        std::cout << "Nenalezeno ID zarizeni" << std::endl;
        return;
    }
    /* vytvoreni OpenCL kontextu */
    context = clCreateContext(0,1,&device,NULL,NULL,&error);
    /* pokud se vytvoreni kontextu nezdarilo */
    if(error != CL_SUCCESS){
        std::cout << "KOntext nevytvoren" << std::endl;
        goto cleanup_context;
    }
    /* vytvoreni fronty prikazu pro dane zarizeni */
    queue = clCreateCommandQueueWithProperties(context,device,0,&error);
    /* pokud se vytvoreni fronty nezdarilo */
    if(error != CL_SUCCESS){
        std::cout << "Fronta prikazu nebyla vytvorena" << std::endl;
        goto cleanup_queue;
    }

    matrix = clCreateBuffer(context,CL_MEM_READ_ONLY,matrix_size,NULL,&error);
    /* predani pointeru 1. matice do OpenCL bufferu */
    error |= clEnqueueWriteBuffer(queue,matrix,CL_TRUE,0,matrix_size,matice,0,NULL,NULL);
    /* pokud se vytvoreni bufferu nepovedlo */
    if(error != CL_SUCCESS){
        std::cout << "Nepodarilo se vytvorit buffer pro matici" << std::endl;
        goto cleanup_matrix;
    }

    /* vytvoreni programu dle zdrojoveho textu v promenne "zdrojovy_kod" */
    program = clCreateProgramWithSource(context,1,&program,NULL,&error);
    /* jestlize nebylo mozne program vytvorit */
    if(error != CL_SUCCESS){
        std::cout << "Program se nepodarilo vytvorit" << std::endl;
        goto cleanup_program;
    }
    error = clBuildProgram(program,0,NULL,NULL,NULL,NULL);
    /* jestlize nebylo mozne program sestavit */
    if(error != CL_SUCCESS){
        std::cout << "Program se nepodarilo zkompilovat" << std::endl;
        goto cleanup_program;
    }
    kernel = clCreateKernel(program,"normalize",&error);
    /* v pripade, ze se vytvoreni kernelu nepovedlo */
    if(error != CL_SUCCESS){
        std::cout << "Kernel se nepodarilo vytvorit" << std::endl;
        goto cleanup_kernel;
    }

    error  = clSetKernelArg(kernel,0,sizeof(cl_mem),(void*)&matrix);
    if(error != CL_SUCCESS){
        std::cout << "Parametry nebyly predany" << std::endl;
        goto cleanup_kernel;
    }

    /* spusteni vypoctu na GPU */
    error = clEnqueueNDRangeKernel(queue,kernel,1,NULL,&pocet_prvku,NULL,0,NULL,NULL);
    /* pokud se spusteni neprovedlo */
    if(error != CL_SUCCESS){
        std::cout << "Nepodarilo se spustit vypocet na GPU" << std::endl;
        goto cleanup_kernel;
    }
    /* cekani na vysledek operace */
    clFinish(queue);
    /* precteni vysledku z GPU vypoctu */
    error = clEnqueueReadBuffer(queue,matrix3,CL_TRUE,0,bytes,res,0,NULL,NULL);
    /* pokud nebylo mozne vystup precist */
    if(error != CL_SUCCESS){
        std::cout << "Vystup se nezdarilo precist" << std::endl;
        goto cleanup_kernel;
    }

    /* uvolneni pameti jadra */
    cleanup_kernel:  clReleaseKernel(kernel);
    /* uvolneni pameti programu */
    cleanup_program: clReleaseProgram(program);
    /* uvolneni pameti 1. vstupni matice */
    cleanup_matrix: clReleaseMemObject(matrix);
    /* uvolneni pameti fronty prikazu */
    cleanup_queue:   clReleaseCommandQueue(queue);
    /* uvolneni pameti OpenCL kontextu */
    cleanup_context: clReleaseContext(context);
    delete[] matice;
}
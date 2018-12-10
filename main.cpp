#define __CL_ENABLE_EXCEPTIONS

#include <iostream>
#include <cmath>
#include <vector>
#include <random>
#include <fstream>

#include "cl.hpp"

void napln_matici(std::vector<double> &matice, int velikost) {
    std::mt19937 rng;
    rng.seed(std::random_device()());
    std::uniform_int_distribution<std::mt19937::result_type> dist(1, velikost);

    std::cout << "Vytvarim matici o velikosti: " << velikost << std::endl;
    for (auto i = 0; i < velikost; ++i) {
        auto val = dist(rng);
        //matice.push_back(val);
        matice[i] = val;
        std::cout << val << " | ";
    }
    std::cout << std::endl;
}

int main(int argc, const char *argv[]) {
    int pocet_prvku;
    std::cout << "Zadejte velikost matice (mocniny čísla 2): ";
    std::cin >> pocet_prvku;
    size_t velikost_matice = sizeof(double) * pocet_prvku;

    std::vector<double> vstupni_matice(pocet_prvku);
    std::vector<double> vystupni_matice(pocet_prvku);

    napln_matici(vstupni_matice, pocet_prvku);

    try {
        std::cout << "Hledam dostupne platformy..." << std::endl;
        std::vector<cl::Platform> platforms;
        cl::Platform::get(&platforms);
        if (platforms.empty()) {
            std::cout << "Platform size 0" << std::endl;
            return -1;
        }
        std::cout << "Nalezl jsem nejakou platformu." << std::endl;

        cl_context_properties properties[] = {CL_CONTEXT_PLATFORM, (cl_context_properties) (platforms[0])(), 0};
        cl::Context context(CL_DEVICE_TYPE_GPU, properties);
        std::cout << "Mam vytvoreny kontext." << std::endl;

        std::cout << "Hledam dostupna zarizeni..." << std::endl;
        std::vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();
        std::cout << "Nasel jsem celkem: " << devices.size() << " zarizeni." << std::endl;

        std::cout << "Nacitam kernel kod..." << std::endl;
        std::ifstream file("../program.cl", std::ifstream::in);
        std::string zdrojovy_kod((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        //std::cout << zdrojovy_kod << std::endl;

        cl::Program::Sources source(1, std::make_pair(zdrojovy_kod.c_str(), zdrojovy_kod.length()));
        std::cout << "Nadefinoval jsem zdrojovy kod." << std::endl;
        cl::Program program = cl::Program(context, source);
        program.build(devices);
        std::cout << "Uspesne jsem sestavil program." << std::endl;

        cl::CommandQueue queue(context, devices[0], 0);

        cl::Buffer prostor_vstupni_matice(context, CL_MEM_READ_ONLY, velikost_matice);
        cl::Buffer prostor_vystupni_matice(context, CL_MEM_WRITE_ONLY, velikost_matice);

        queue.enqueueWriteBuffer(prostor_vstupni_matice, CL_TRUE, 0, velikost_matice, vstupni_matice.data());

        cl::Kernel kernel(program, "moje_hledani_extremu");
        kernel.setArg(0, prostor_vstupni_matice);
        kernel.setArg(1, prostor_vystupni_matice);
        kernel.setArg(2, sizeof(int), (void *) &pocet_prvku);
        kernel.setArg(3, sizeof(double) * pocet_prvku, nullptr); // Prechodova pamet pro MIN
        kernel.setArg(4, sizeof(double) * pocet_prvku, nullptr); // Prechodova pamet pro MAX

        queue.enqueueNDRangeKernel(kernel, 1, pocet_prvku, pocet_prvku);

        queue.finish();

        queue.enqueueReadBuffer(prostor_vystupni_matice, CL_TRUE, 0, velikost_matice, vystupni_matice.data());


        std::cout << "Vysledek normalizace:" << std::endl;
        for (const auto &val : vystupni_matice) {
            std::cout << val << " | ";
        }
        std::cout << std::endl;

    } catch (std::exception &ex) {
        std::cout << ex.what() << std::endl;
    }

}
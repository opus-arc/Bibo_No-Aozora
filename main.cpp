#include "SSP_MMC/SSP_MMC.h"
#include<iostream>

int main() {
    try {

        double cost = SSP_MMC::read_SSPMMC_Result("cost", "12", 234);
        double ivl = SSP_MMC::read_SSPMMC_Result("ivl", "8", 123);
        double recall = SSP_MMC::read_SSPMMC_Result("recall", "1", 4);

        std::cout << cost << std::endl;
        std::cout << ivl << std::endl;
        std::cout << recall << std::endl;

    }catch(const std::exception& e) {
        std::cout << e.what() << '\n';
    }

    return 0;
}

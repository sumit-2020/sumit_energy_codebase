#include "test_routine.h"

int main(int argc, char **argv) {
    //Begin Setup of FPGA
    fpga_t* fpga;
    fpga_info_list info;

    // Populate the fpga_info_list struct
    if (fpga_list(&info) != 0) {
        printf(RED "Error populating fpga_info_list\n" RESET);
        exit(EXIT_FAILURE);
    }
    fpga = fpga_open(0);
    if(!fpga){
        printf(RED "Problem on opening the fpga \n" RESET);
        exit(EXIT_FAILURE);
    }
    fpga_reset(fpga); //keep this

    // Draining buffered items...
    cout << endl << GREEN "Draining FPGA command buffers...." RESET << endl;
    uint rbuf[32];
    int num_recv = 0 ;
    do {
        num_recv = fpga_recv(fpga, 0, (void*)rbuf, 32, 1000);
        printf("\r %3d", num_recv);
        fflush(stdout);
        usleep(50*1000);
    } while (num_recv);
    cout << endl;
    fpga_close(fpga);
    return 0;
}

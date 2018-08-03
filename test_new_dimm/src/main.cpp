/**
 * @brief Test if we can read and write data using the spec. timing parameters
 *
 * Use this whenever:
 * 1. We have a new DIMM. Test it to make sure it's not faulty.
 * 2. We are not sure if the FPGA board is in the correct operating state.
 *
 * @author Kevin Chang <kevincha@cmu.edu>
 **/

#include "timer.h"
#include "test_routine.h"

int main(int argc, char **argv) {
    // Begin Setup of FPGA
    fpga_info_list info;
    if (fpga_list(&info) != 0) {
        printf("Error populating fpga_info_list\n");
        exit(EXIT_FAILURE);
    }
    printf("Number of devices: %d\n", info.num_fpgas);
    for (int i = 0; i < info.num_fpgas; i++) {
        printf("%d: id:%d\n", i, info.id[i]);
        printf("%d: num_chnls:%d\n", i, info.num_chnls[i]);
        printf("%d: name:%s\n", i, info.name[i]);
        printf("%d: vendor id:%04X\n", i, info.vendor_id[i]);
        printf("%d: device id:%04X\n", i, info.device_id[i]);
    }
    fpga_t* fpga = fpga_open(0);
    if(!fpga){
        printf("Problem on opening the fpga \n");
        exit(EXIT_FAILURE);
    }
    printf("The FPGA has been opened successfully! \n");
    fpga_reset(fpga); //keep this
    //////

    CmdQueue* cq = nullptr ;
    cout << "Total tested number of banks=" << NUM_BANKS <<
        " rows=" << NUM_ROWS << endl;

    uint8_t patt = 0xaa;
    for (uint bank=0; bank < NUM_BANKS; bank++) {
        cout << "Testng Bank " << bank << endl;
        for (uint r=0; r < NUM_ROWS; r++) {
            // Invert the pattern every other row
            patt = ~patt;
            turnBus(fpga, BUSDIR::WRITE, cq);
            writeRow(fpga, r, bank, patt, cq, 1024);
            turnBus(fpga, BUSDIR::READ, cq);
            int error = 0 ;
            error = readAndCompareRow(fpga, r, bank, patt, cq, 1024);
            if (error > 0)
                cout << error << " errors at Bank: " << bank << ", Row : " << r << endl;
        }
    }

    usleep(500 * 1000); // takes microseconds
    fpga_close(fpga);
    return 0;
}

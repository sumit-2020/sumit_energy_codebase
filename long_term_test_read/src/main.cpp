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

    ofstream error_file ;
    error_file.open("long_test_"+ fmtTime()) ;

    time_t start_time, now;
    const time_t ONE_DAY = 24 * 60 * 60;
    time(&start_time);
    time(&now);

    // Worst-case pattern
    const uint8_t patt_base = 0xaa;
    uint8_t patt_inv = (uint8_t)~patt_base;

    // 30-day test
    cout << "Day: " << ((now - start_time) / ONE_DAY) << endl;
    while ((now - start_time) < (30 * ONE_DAY)) {
        // Some filler iterations until we check the time again
        for (int i = 0; i < 256; i++) {
            for (int bank = 0; bank < NUM_BANKS; bank++) {
                for (uint r = 0; r < NUM_ROWS; r++) {
                    uint8_t patt = (r & (uint8_t)0x1) ? patt_base : patt_inv;
                    turnBus(fpga, BUSDIR::WRITE, cq) ;
                    writeRow(fpga, r, bank, patt, cq, 1024) ;
                    turnBus(fpga, BUSDIR::READ, cq) ;
                    int error = 0 ;
                    error = readAndCompareRow(fpga, r, bank, patt, cq, 1024) ;
                    if (error) {
                        cout << endl << "ERROR at Row : " << r << " count " << error << endl;
                        error_file << fmtTime() << " " << r << " " << error << endl;
                    }
                }
            }
            sleep(1); // stop for a second
        }

        // Time
        time(&now);
        cout << "\rDay: " << ((now - start_time) / ONE_DAY);
        fflush(stdout);
    }
    cout << endl;

    // DONE
    error_file.close();
    usleep(500 * 1000); // takes microseconds

    fpga_close(fpga);
}

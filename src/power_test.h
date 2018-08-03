#ifndef TEST_POWER_INFINITELOOP
#define TEST_POWER_INFINITELOOP

#include "test.h"
#include <map>

#define IDD4_COL1 0
#define IDD4_COL2 1 //NUM_COLS/2

typedef std::map<std::string,int> latency_list;

class PowerTest: public Test {
	public:
		static const int LOOP_BUFFER_SIZE = 2048;

    static const uint col0 = 0;
    static const uint col1 = 120; // 0x0078 : (F0)8 refer to hynix datasheet

		PowerTest(int sandbox) : Test(sandbox)
		{
		}

    void idd0_loop(latency_list, DramAddr *);
    void idd1_loop(latency_list, DramAddr *);

    void idd4r_init(int, uint8_t);
    void idd4r_loop(int);
    void idd4w_loop(int, uint8_t);

    void idd2p0_loop();
    void idd2p1_loop();
    void idd2p_loop();
    void idd2q_loop();
    void idd3p_loop();
    void idd5b_init();
    void idd5b_stop();
    void idd5b_loop(uint);
    void idd6_loop();
    void idd6et_loop();
    void idd7_init();
    void idd7_loop();
    void idd8_loop();

    void sumitfcfs_loop();
    void sumitfrfcfs_loop();
    void ahb_init(int, uint8_t);
    void sumitahb_loop();
    void sumitfrfcfspriorhit_loop();

    void act_rdn_pre(DramAddr *,uint);
    void act_wrn_pre(DramAddr *,uint, uint8_t);
    void act_rdn_pre_fix_time (DramAddr *, uint);
    void act_wrn_pre_fix_time(DramAddr *, uint , uint8_t );

    void col_init(int bank, int row, int columbn, uint8_t pattern);
    void col_loop(int bank, int row, int columbn, uint8_t pattern);

    void idd0_datasheet_loop(latency_list);
    void idd1_datasheet_loop(latency_list);
    void idd2n_datasheet_loop();
    void idd3n_datasheet_loop();

};

#endif

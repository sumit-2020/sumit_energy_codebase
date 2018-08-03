#ifndef TEST_H
#define TEST_H
#include <string>
#include <fstream>
#include "softmc_api.h"
#include "node_state.h"

#define BUFF_SIZE 16
#define warn_not_impl std::cout << "This function is not implemented yet" << std::endl;

typedef uint read_buffer [BUFF_SIZE];
typedef std::vector<uint8_t> read_flex_buffer;

class Test {

public:
    NodeState * ns;
	fpga_t * fpga;
	CmdQueue * cq;
	bool sandbox;
    static const int ch = 0;
    read_flex_buffer rflexbuf;

	Test(int);
	~Test();

    void turn_bus(BUSDIR);
    void write_entire_row   (DramAddr *, uint8_t);
    void read_entire_row    (DramAddr * );
    void act_pre            (DramAddr *);
    void act                (DramAddr *);
    void pre                (DramAddr *);
    void act_rd_pre         (DramAddr *);
    void act_rd_rd_pre      (DramAddr *);
    void act_rd_rd_rd_pre   (DramAddr *);
    void act_wr_pre         (DramAddr *, uint8_t);
    void act_wr_wr_pre      (DramAddr *, uint8_t);
    void wait_for (int);

    void write_column         (DramAddr *, uint8_t);
    void write_column_turnbus (DramAddr *, uint8_t);
    void read_column          (DramAddr *);

    void buff_wait(uint n);
    void send_commands(bool add_startTr = true);
    void read_back();
    void generate_loop_trace(std::string);
    bool close_open_banks();

    //Loop Related
    bool end_of_init();
    bool end_of_loop(uint);
    void stop_looping();
};
#endif

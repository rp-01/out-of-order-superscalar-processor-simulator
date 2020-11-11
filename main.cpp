#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <math.h>
#include <bitset>
#include <algorithm>
#include <math.h>
#include <iomanip>

struct SIM_INPUT{
    unsigned int peak_fd_rate = 0;
    unsigned int sched_queue_size = 0;
    std::string trace_file = "";
}
unsigned int cycle_count = 0;

struct ROB{
    std::string state = ""; // IF, ID, IS, EX, WB
    unsigned int tag = 0;
    src_reg_1 = 0;
    src_reg_2 = 0;
    dest_reg = 0;
    op_type = 0;
    unsigned int IF_duartion = 0;
    unsigned int ID_duration = 0;
    unsigned int IS_duration = 0;
    unsigned int EX_duration = 0;
    unsigned int WB_duration = 0;
    unsigned int IF_cycle = 0;
    unsigned int ID_cycle = 0;
    unsigned int IS_cycle = 0;
    unsigned int EX_cycle = 0;
    unsigned int WB_cycle = 0;
}

std::vector<ROB> details;
int main(int argc, char *argv[]){
     char *pCh;
}
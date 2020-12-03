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
#include <numeric>

struct ROB
{
    int op_type = 0;
    int dest_reg = 0;
    int src_reg1 = 0;
    int src_reg2 = 0;
    std::string src1_flag = "ready"; // defaut val ready
    std::string src2_flag = "ready"; // default val ready
    std::string state = "if"; // IF, ID, IS, EX, WB
    unsigned int tag = 0;
    unsigned int src1_tag = 0;
    unsigned int src2_tag = 0;
    unsigned int IF_duration = 0;
    unsigned int ID_duration = 0;
    unsigned int IS_duration = 0;
    unsigned int EX_duration = 1;
    unsigned int WB_duration = 0;
    unsigned int IF_cycle = 0;
    unsigned int ID_cycle = 0;
    unsigned int IS_cycle = 0;
    unsigned int EX_cycle = 0;
    unsigned int WB_cycle = 0;
    unsigned int ex_stall = 0;
};

std::vector<ROB> instruction_data;
std::vector<ROB> dispatch_queue;
std::vector<ROB> schedule_queue;
std::vector<ROB> ex_queue;
std::vector<ROB> wb_queue;
std::vector<unsigned int> ipc_total;
unsigned int ipc = 0;
double ipc_avg = 0.0;
unsigned int cycle_count = 0;
unsigned int schedule_size = 0;
unsigned int peak_rate = 0;
std::vector<std::string> file_content;
std::string trace_file = "";

void execute(unsigned int n_size, unsigned int s_size);
void issue(unsigned int n_size, unsigned int s_size);
void dispatch(unsigned int n_size, unsigned int s_size);
void fetch(unsigned int n_size, unsigned int s_size);
bool sort_tag(const ROB &x, const ROB &y);

int main(int argc, char *argv[])
{
    char *pCh;
    schedule_size = strtoul(argv[1], &pCh, 10);
    peak_rate = strtoul(argv[2], &pCh, 10);
    trace_file = argv[argc - 1];
    std::string token = "";
    std::vector<std::string> input;
    unsigned int hexAd = 0;
    std::fstream myfile(trace_file);
    std::string line = "";
    unsigned int idx_bits = 0;
    unsigned int fileLen = 1;

    if (myfile.is_open())
    {
        while (getline(myfile, line))
        {
            if (line.empty())
            {
                continue;
            }
            file_content.push_back(line);
        }
        myfile.close();
    }
    else
    {
        std::cout << "**" << trace_file << " doesn't exist.**" << std::endl;
        std::cout << "**Please make sure that the trace files are in the same directory as sim executable.**" << std::endl;
    }

    instruction_data.resize(file_content.size());

    for (int i = 0; i < file_content.size(); i++)
    {
        std::istringstream stm(file_content[i]);
        input.clear();
        while (stm >> token)
        {
            input.push_back(token);
        }
        instruction_data[i].tag = i;
        std::stringstream(input[1]) >> instruction_data[i].op_type;
        std::stringstream(input[2]) >> instruction_data[i].dest_reg;
        std::stringstream(input[3]) >> instruction_data[i].src_reg1;
        std::stringstream(input[4]) >> instruction_data[i].src_reg2;
        if (instruction_data[i].op_type == 0)
        {
            instruction_data[i].ex_stall = 1; // record ex stall
        }
        else if (instruction_data[i].op_type == 1)
        {

            instruction_data[i].ex_stall = 2; // record ex stall
        }
        else if (instruction_data[i].op_type == 2)
        {

            instruction_data[i].ex_stall = 5; // record ex stall
        }
    }
    int total_instruction = instruction_data.size();

    while (wb_queue.size() < total_instruction)
    {
        execute(peak_rate, schedule_size);
        issue(peak_rate, schedule_size);
        dispatch(peak_rate, schedule_size);
        fetch(peak_rate, schedule_size);
        ipc_total.push_back(ipc); // record instructions completed
        ipc = 0;                  // reset ipc var
        cycle_count++;
    }

    std::sort(wb_queue.begin(), wb_queue.end(), sort_tag);

    // print results
    for (int i = 0; i < wb_queue.size(); i++)
    {

        std::cout << wb_queue[i].tag << "fu{" << wb_queue[i].op_type << "} "
                  << "src{" << wb_queue[i].src_reg1 << "," << wb_queue[i].src_reg2 << "} "
                  << "dst{" << wb_queue[i].dest_reg << "} "
                  << "IF{" << wb_queue[i].IF_cycle << "," << wb_queue[i].IF_duration << "} "
                  << "ID{" << wb_queue[i].ID_cycle << "," << wb_queue[i].ID_duration << "} "
                  << "IS{" << wb_queue[i].IS_cycle << "," << wb_queue[i].IS_duration << "} "
                  << "EX{" << wb_queue[i].EX_cycle << "," << wb_queue[i].EX_duration << "} "
                  << "WB{" << wb_queue[i].WB_cycle << "," << wb_queue[i].WB_duration << "} "
                  << std::endl;
    }

    // calc avg IPC
    ipc_avg = accumulate(ipc_total.begin(), ipc_total.end(), 0.0) / cycle_count;
    ipc_avg = std::round(ipc_avg * 100000) / 100000;

    std::cout << "number of instructions = " << wb_queue.size() << std::endl;
    std::cout << "number of cycles       = " << cycle_count << std::endl;
    std::cout << std::fixed << std::setprecision(5) << "IPC                   = " << ipc_avg << std::endl;
}

// execute
void execute(unsigned int n_size, unsigned int s_size)
{
    for (int i = 0; i < ex_queue.size(); i++)
    {

        if (ex_queue[i].state == "ex")
        {
            if (ex_queue[i].EX_duration == ex_queue[i].ex_stall)
            {
                ipc++; // record # of completed instructions
                ex_queue[i].state = "wb";
                ex_queue[i].WB_cycle = cycle_count;
                ex_queue[i].WB_duration++;
                // store instruction details for output
                wb_queue.push_back(ex_queue[i]);
            }
            else
            {
                ex_queue[i].EX_duration++;
            }
        }
    }

    for (unsigned int i = 0; i < ex_queue.size();)
    {
        if (ex_queue[i].state == "wb")
        {
            ex_queue.erase(ex_queue.begin() + i);
        }

        else
            i++;
    }
    std::sort(wb_queue.begin(), wb_queue.end(), sort_tag);

    for (int i = 0; i < schedule_queue.size(); i++)
    {
        for (int j = 0; j < wb_queue.size(); j++)
        {
            if (wb_queue[j].tag == schedule_queue[i].src1_tag)
            {
                schedule_queue[i].src1_flag = "ready";
            }
            if (wb_queue[j].tag == schedule_queue[i].src2_tag)
            {
                schedule_queue[i].src2_flag = "ready";
            }
        }
    }
}

// issue
void issue(unsigned int n_size, unsigned int s_size)
{
    unsigned int fn_count = 0;
    for (int i = 0; i < schedule_queue.size(); i++)
    {
        if (schedule_queue[i].state == "is")
        {
            schedule_queue[i].IS_duration++;
            //check if both regs are ready
            if (schedule_queue[i].src1_flag == "ready" & schedule_queue[i].src2_flag == "ready")
            {

                if (fn_count < (n_size + 1))
                {
                    schedule_queue[i].state = "ex";
                    schedule_queue[i].EX_cycle = cycle_count;
                    ex_queue.push_back(schedule_queue[i]);
                    fn_count++;
                }
            }
        }
    }

    for (unsigned int i = 0; i < schedule_queue.size();)
    {
        if (schedule_queue[i].state == "ex")
            schedule_queue.erase(schedule_queue.begin() + i);
        else
            i++;
    }
}

// dispatch
void dispatch(unsigned int n_size, unsigned int s_size)
{
    for (int i = 0; i < dispatch_queue.size(); i++)
    {
        if (dispatch_queue[i].state == "id")
        {
            if (schedule_queue.size() < s_size)
            {
                dispatch_queue[i].state = "is"; // change state to IS
                dispatch_queue[i].IS_cycle = cycle_count;

                if (!schedule_queue.empty())
                {
                    for (int j = 0; j < schedule_queue.size(); j++)
                    {
                        if (dispatch_queue[i].src_reg1 == schedule_queue[j].dest_reg && schedule_queue[j].dest_reg != -1)
                        {
                            if (dispatch_queue[i].src1_tag == 0)
                            {
                                dispatch_queue[i].src1_tag = schedule_queue[j].tag;
                                dispatch_queue[i].src1_flag = "not";
                            }
                            else if (dispatch_queue[i].src1_tag != 0 && dispatch_queue[i].src1_tag < schedule_queue[j].tag)
                            {
                                dispatch_queue[i].src1_tag = schedule_queue[j].tag;
                                dispatch_queue[i].src1_flag = "not";
                            }
                        }
                        if (dispatch_queue[i].src_reg2 == schedule_queue[j].dest_reg && schedule_queue[j].dest_reg != -1)
                        {
                            if (dispatch_queue[i].src2_tag == 0)
                            {
                                dispatch_queue[i].src2_tag = schedule_queue[j].tag;
                                dispatch_queue[i].src2_flag = "not";
                            }
                            else if (dispatch_queue[i].src2_tag != 0 && dispatch_queue[i].src2_tag < schedule_queue[j].tag)
                            {
                                dispatch_queue[i].src2_tag = schedule_queue[j].tag;
                                dispatch_queue[i].src2_flag = "not";
                            }
                        }
                    }
                }
                if (!ex_queue.empty())
                {
                    std::sort(ex_queue.begin(), ex_queue.end(), sort_tag);
                    for (int j = 0; j < ex_queue.size(); j++)
                    {
                        if (ex_queue[j].dest_reg == dispatch_queue[i].src_reg1 && ex_queue[j].dest_reg != -1)
                        {

                            if (dispatch_queue[i].src1_tag == 0)
                            {
                                dispatch_queue[i].src1_tag = ex_queue[j].tag;
                                dispatch_queue[i].src1_flag = "not";
                            }
                            else if (dispatch_queue[i].src1_tag < ex_queue[j].tag)
                            {
                                dispatch_queue[i].src1_tag = ex_queue[j].tag;
                                dispatch_queue[i].src1_flag = "not";
                            }
                        }
                        if (ex_queue[j].dest_reg == dispatch_queue[i].src_reg2 && ex_queue[j].dest_reg != -1)
                        {
                            if (dispatch_queue[i].src2_tag == 0)
                            {
                                dispatch_queue[i].src2_tag = ex_queue[j].tag;
                                dispatch_queue[i].src2_flag = "not";
                            }
                            else if (dispatch_queue[i].src2_tag < ex_queue[j].tag)
                            {
                                dispatch_queue[i].src2_tag = ex_queue[j].tag;
                                dispatch_queue[i].src2_flag = "not";
                            }
                        }
                    }
                }
                std::sort(wb_queue.begin(), wb_queue.end(), sort_tag);
                for (int j = 0; j < wb_queue.size(); j++)
                {
                    if (wb_queue[j].dest_reg == dispatch_queue[i].src_reg1 && wb_queue[j].dest_reg != -1)
                    {

                        if (dispatch_queue[i].src1_tag == 0)
                        {
                            dispatch_queue[i].src1_tag = wb_queue[j].tag;
                            dispatch_queue[i].src1_flag = "ready";
                        }
                        else if (dispatch_queue[i].src1_tag < wb_queue[j].tag)
                        {
                            dispatch_queue[i].src1_tag = wb_queue[j].tag;
                            dispatch_queue[i].src1_flag = "ready";
                        }
                    }
                    if (wb_queue[j].dest_reg == dispatch_queue[i].src_reg2 && wb_queue[j].dest_reg != -1)
                    {
                        if (dispatch_queue[i].src2_tag == 0)
                        {
                            dispatch_queue[i].src2_tag = wb_queue[j].tag;
                            dispatch_queue[i].src2_flag = "ready";
                        }
                        else if (dispatch_queue[i].src2_tag < wb_queue[j].tag)
                        {
                            dispatch_queue[i].src2_tag = wb_queue[j].tag;
                            dispatch_queue[i].src2_flag = "ready";
                        }
                    }
                }
                // fill scheduling queue
                schedule_queue.push_back(dispatch_queue[i]);
            }
            else if (schedule_queue.size() == s_size) // if schedue queue full
            {
                //increment stall in ID state
                dispatch_queue[i].ID_duration++;
            }
        }
        else if (dispatch_queue[i].state == "if") // unconditional IF to ID transition
        {
            //ipc++;
            dispatch_queue[i].state = "id";
            dispatch_queue[i].ID_duration++;
            dispatch_queue[i].ID_cycle = cycle_count; // capture cycle count at state change to ID
        }
    }

    //remove instructions from dispatch queue
    for (unsigned int i = 0; i < dispatch_queue.size();)
    {
        if (dispatch_queue[i].state == "is")
            dispatch_queue.erase(dispatch_queue.begin() + i);
        else
            i++;
    }
}

// fetch instruction to dispatch queue and change state to IF
void fetch(unsigned int n_size, unsigned int s_size)
{
    for (int i = 0; i < n_size; i++)
    {
        if (dispatch_queue.size() < (n_size * 2))
        {
            if (instruction_data.empty())
            {
                break;
            }
            // push to dispatch queue
            instruction_data[0].IF_cycle = cycle_count; // capture cycle count at state change to IF
            instruction_data[0].IF_duration++;

            dispatch_queue.push_back(instruction_data[0]);
            instruction_data.erase(instruction_data.begin());
        }
    }
}
bool sort_tag(const ROB &x, const ROB &y)
{
    return x.tag < y.tag;
}
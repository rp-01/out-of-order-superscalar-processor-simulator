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

struct ROB
{
    int op_type = 0;
    int dest_reg = 0;
    int src_reg1 = 0;
    int src_reg2 = 0;
    std::string dest_flag = "";
    std::string src1_flag = "ready";
    std::string src2_flag = "ready";
    std::string state = ""; // IF, ID, IS, EX, WB
    unsigned int tag = 0;
    unsigned int IF_duration = 0;
    unsigned int ID_duration = 0;
    unsigned int IS_duration = 0;
    unsigned int EX_duration = 0;
    //unsigned int WB_duration = 0; // constant
    unsigned int IF_cycle = 0;
    unsigned int ID_cycle = 0;
    unsigned int IS_cycle = 0;
    unsigned int EX_cycle = 0;
    unsigned int WB_cycle = 0;

    unsigned int ex_stall = 0;
};

std::vector<ROB> instuction_data;
std::vector<ROB> dispatch_queue;
std::vector<ROB> schedule_queue;
std::vector<ROB> ex_queue;
std::vector<ROB> wb_queue;
unsigned int cycle_count = 0;

unsigned int schedule_size = 0;
unsigned int peak_rate = 0;

std::vector<std::string> file_content;
std::string trace_file = "";

void execute(unsigned int n_size, unsigned int s_size);
void issue(unsigned int n_size, unsigned int s_size);
void dispatch(unsigned int n_size, unsigned int s_size);
void fetch(unsigned int n_size, unsigned int s_size);

int main(int argc, char *argv[])
{
    char *pCh;
    schedule_size = strtoul(argv[1], &pCh, 10);
    peak_rate = strtoul(argv[2], &pCh, 10);
    trace_file = argv[argc - 1];

    // resize lists
    //dispatch_queue.resize(peak_rate * 2);
    //schedule_queue.resize(schedule_size);
    //ex_queue.resize(peak_rate + 1);

    //std::cout << dispatch_queue.size() << std::endl;
    //std::cout << ex_queue.size() << std::endl;
    //std::cout << schedule_queue.size() << std::endl;

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

    instuction_data.resize(file_content.size());

    for (int i = 0; i < file_content.size(); i++)
    {
        std::istringstream stm(file_content[i]);
        input.clear();
        while (stm >> token)
        {

            input.push_back(token);
        }
        std::stringstream(input[1]) >> instuction_data[i].op_type;
        std::stringstream(input[2]) >> instuction_data[i].dest_reg;
        std::stringstream(input[3]) >> instuction_data[i].src_reg1;
        std::stringstream(input[4]) >> instuction_data[i].src_reg2;
        if (instuction_data[i].op_type == 0)
        {
            instuction_data[i].ex_stall = 1;
        }
        else if (instuction_data[i].op_type == 1)
        {
            instuction_data[i].ex_stall = 1;
        }
        else if (instuction_data[i].op_type == 2)
        {
            instuction_data[i].ex_stall = 1;
        }
        instuction_data[i].tag = i;
    }

    /*for (int i = 0; i < instuction_data.size(); i++)
    {
        std::cout << instuction_data[i].op_type << std::endl;
        std::cout << instuction_data[i].dest_reg << std::endl;
        std::cout << instuction_data[i].src_reg1 << std::endl;
        std::cout << instuction_data[i].src_reg2 << std::endl;
        std::cout << instuction_data[i].tag << std::endl;

    }*/
    while (cycle_count < file_content.size())
    {
        execute(peak_rate, schedule_size);
        
        issue(peak_rate, schedule_size);
        dispatch(peak_rate, schedule_size);
        fetch(peak_rate, schedule_size);
        std::cout << cycle_count << std::endl;
        cycle_count++;
    }
}

void execute(unsigned int n_size, unsigned int s_size)
{
    for (int i = 0; i < ex_queue.size(); i++)
    {
        if (ex_queue[i].state == "ex")
        {
            if (ex_queue[i].EX_duration == ex_queue[i].ex_stall)
            {
                ex_queue[i].state = "wb";
                ex_queue[i].WB_cycle = cycle_count;

                // update sr

                for (int j = 1; j < schedule_queue.size(); j++)
                {
                    if (schedule_queue[i].dest_reg == schedule_queue[j].src_reg1)
                    {
                        schedule_queue[j].src1_flag = "ready";
                    }
                    if (schedule_queue[i].dest_reg == schedule_queue[j].src_reg2)
                    {
                        schedule_queue[j].src2_flag = "ready";
                    }
                }
                // store instruction details for output
                wb_queue.push_back(ex_queue[i]);
                // ex_queue space
                ex_queue.erase(ex_queue.begin() + (i - 1));
            }
            else
            {
                ex_queue[i].EX_duration++;
            }
        }
        if (ex_queue.size() == 0)
        {
            break;
        }
    }
}

void issue(unsigned int n_size, unsigned int s_size)
{
    for (int i = 0; i < schedule_queue.size(); i++)
    {
        for (int j = 1; j < schedule_queue.size(); j++)
        {
            if (schedule_queue[i].dest_reg == schedule_queue[j].src_reg1)
            {
                schedule_queue[j].src1_flag = "not";
            }
            if (schedule_queue[i].dest_reg == schedule_queue[j].src_reg2)
            {
                schedule_queue[j].src2_flag = "not";
            }
        }
    }
    for (int i = 0; i < schedule_queue.size(); i++)
    {
        if (schedule_queue[i].state == "is")
        {
            //check if both regs are ready
            if (schedule_queue[i].src1_flag == "ready" & schedule_queue[i].src2_flag == "ready")
            {
                // update state since both src reg are ready
                schedule_queue[i].state = "ex";
                schedule_queue[i].EX_cycle = cycle_count;

                // push the updated instruction to ex queue till (n_size+1)
                if (ex_queue.size() <= (n_size + 1))
                {
                    ex_queue.push_back(schedule_queue[i]);
                    //free schedule_queue space
                    schedule_queue.erase(schedule_queue.begin() + (i - 1));
                }
            }
            else if (schedule_queue[i].src1_flag != "ready" | schedule_queue[i].src2_flag != "ready")
            {
                schedule_queue[i].IS_duration++; // increment stall in IS state
            }
        }
        if (schedule_queue.size() == 0)
        {
            break;
        }
    }
}

// fetch instruction to dispatch queue and change state to IF
void fetch(unsigned int n_size, unsigned int s_size)
{
    for (int i = 0; i < n_size; i++)
    {
        if (dispatch_queue.size() <= (n_size * 2))
        {
            // push to dispatch queue
            dispatch_queue.push_back(instuction_data[i]);
        }
        else
        {
            for (int i = 0; i < dispatch_queue.size(); i++)
            {
                if (dispatch_queue[i].state == "")
                {
                    // change empty state to IF state
                    dispatch_queue[i].state = "if";
                    dispatch_queue[i].IF_cycle = cycle_count; // capture cycle count at state change to IF
                }
            }
        }
    }
}
void dispatch(unsigned int n_size, unsigned int s_size)
{
    for (int i = 0; i < dispatch_queue.size(); i++)
    {
        if (dispatch_queue[i].state == "id")
        {
            if (schedule_queue.size() < s_size)
            {
                dispatch_queue[i].state = "is";           // change state to IS
                dispatch_queue[i].IS_cycle = cycle_count; // capture cycle count at state change

                schedule_queue.push_back(dispatch_queue[i]);
                dispatch_queue.erase(dispatch_queue.begin() + (i - 1)); // free dispatch queue space
            }
            else if (schedule_queue.size() == s_size) // if schedue queue full
            {
                //increment stall in ID state
                dispatch_queue[i].ID_duration++;
            }
        }
        else if (dispatch_queue[i].state == "if") // unconditional IF to ID transition
        {
            dispatch_queue[i].state = "id";
            dispatch_queue[i].IF_duration++;

            dispatch_queue[i].ID_cycle = cycle_count; // capture cycle count at state change to ID
        }
        if (dispatch_queue.size() == 0)
        {
            break;
        }
    }
}
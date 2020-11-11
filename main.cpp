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
    std::string op_type = "";
     int dest_reg = 0;
     int src_reg1 = 0;
     int src_reg2 = 0;
    unsigned int dest_tag = 0;
    unsigned int src1_tag = 0;
    unsigned int src2_tag  = 0;
    std::string state = ""; // IF, ID, IS, EX, WB
    unsigned int IF_duration = 0;
    unsigned int ID_duration = 0;
    unsigned int IS_duration = 0;
    unsigned int EX_duration = 0;
    unsigned int WB_duration = 0;

    unsigned int IF_cycle = 0;
    unsigned int ID_cycle = 0;
    unsigned int IS_cycle = 0;
    unsigned int EX_cycle = 0;
    unsigned int WB_cycle = 0;
    
    unsigned int ex_stall = 0;
};

std::vector<int> register_file;
std::vector<ROB> instuction_data;
std::vector<ROB> dispatch_queue;
std::vector<ROB> schedule_queue;
std::vector<ROB> ex_queue;
unsigned int cycle_count = 0;

unsigned int schedule_size = 0;
unsigned int peak_rate = 0;

std::vector<std::string> file_content;
std::string trace_file = "";

void execute();
void issue();
void dispatch();
void fetch();

int main(int argc, char *argv[])
{
    char *pCh;
    schedule_size = strtoul(argv[1], &pCh, 10);
    peak_rate = strtoul(argv[2], &pCh, 10);
    trace_file = argv[argc - 1];


    // resize lists
    dispatch_queue.resize(peak_rate * 2);
    schedule_queue.resize(schedule_size);
    ex_queue.resize(peak_rate+1);
    register_file.resize(schedule_queue);

    std::cout << dispatch_queue.size() << std::endl;
    std::cout << ex_queue.size() << std::endl;
    std::cout << schedule_queue.size() << std::endl;
    
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
        if(instuction_data[i].op_type == 0){
            instuction_data[i].ex_stall = 1;
        }
        else if(instuction_data[i].op_type == 1){
            instuction_data[i].ex_stall = 1;

        }
        else if(instuction_data[i].op_type == 2){
            instuction_data[i].ex_stall = 1;
            
        }
        instuction_data[i].dest_tag = i;
        
        
    }
    /*for (int i = 0; i < instuction_data.size(); i++)
    {
        std::cout << instuction_data[i].op_type << std::endl;
        std::cout << instuction_data[i].dest_reg << std::endl;
        std::cout << instuction_data[i].src_reg1 << std::endl;
        std::cout << instuction_data[i].src_reg2 << std::endl;
        std::cout << instuction_data[i].tag << std::endl;

    }*/
    while(cycle_count < file_content.size()){
        execute();
        issue();
        dispatch();
        fetch();

        cycle_count++;
    }

}

void execute(){
    for(int i = 0; i < ex_queue.size(); i++){
        if(ex_queue[i].state == "ex"){
            if(ex_queue[i].EX_duration == ex_queue[i].ex_stall){
                ex_queue[i].state == "wb";
            }
        else{
            ex_queue[i].EX_duration++;
        }
        }
    }
}

void fetch{
    for(int i = 0; i < dispatch_queue.size(); i++){

    }
}
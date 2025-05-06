#include<iostream>
#include<string>
#include<fstream>
#include<sstream>
#include<vector>
#include<cmath>
#include "helpers.h"

int main(){
    std::vector<int> registers(32, 0); // [reg, val] 0 to 32
    std::vector<int> memory(997, 0); // [address, value] 0 to 996
    std::vector<instruction> instr; // code string
    
    // file opening
    std::cout << "enter filename: ";
    std::string file_name;
    std::cin >> file_name;
    
    std::ifstream input(file_name);
    if(!input){
        std::cerr << "file does not exist!";
        return 1;
    }
    // input file parsing
    pars(input, registers, memory, instr);
    input.close();
    
    // pipeline simulation
    std::string user_input = "yes";
    std::vector<std::vector<std::string>> pipeline_log;
    std::vector<std::string> pipeline_stages = {"IF",  "ID", "EX", "MEM", "WB"};    // IF, ID, EX, MEM, WB

    // initiating the pipeline (5 stage)
    for(int i = 0; i < instr.size(); ++i){
        std::vector<std::string> temp;
        for (int j = 0; j < i; ++j) {
            temp.push_back("");
        }
        for (int j = 0; j < pipeline_stages.size(); ++j){
            temp.push_back(pipeline_stages[j]);
        }
        pipeline_log.push_back(temp);
    }

    int instr_index = 0;
    int instruction_rounds = 1; // how many times do we loop our simulation
    int instr_count = instr.size(); // how many instructions we need to keep track of
    std::vector<int>flushes(20, 0); // indexes where they occur, num of flushes

    while(instr_count > 0 && user_input == "yes"){
        instr_index++;
        instruction_rounds++;
        // hazard checks (data and control)
        // data hazard // lw , sw, add, addi, sub, slt, beq, bne
        if(instr_count != instr.size()){

            int curr_index = instr_index - 1; // current row
            int prev_index = curr_index - 1; // previous row
            //std::cout << "index: " << curr_index << std::endl;

            // data hazards
            if(instr[prev_index].name == "add" || instr[prev_index].name == "sub" || instr[prev_index].name == "slt"){

                if(instr[prev_index].rd == instr[curr_index].rs || instr[prev_index].rd == instr[curr_index].rt)
                {
                    for(int i = curr_index + 1; i < curr_index + 4; i++){
                        if(pipeline_log[prev_index][i + 1] == "WB"){
                            break;
                        }
                        else if(pipeline_log[prev_index][i] != "WB"){
                            pipeline_log[curr_index].insert(pipeline_log[curr_index].begin() + curr_index + 2, "stall");
                        }
                    }
                }
            }

            else if(instr[prev_index].name == "addi"){ // addi $t1, $t2, 10 rt, rs, imm

                if(instr[prev_index].rt == instr[curr_index].rs || instr[prev_index].rt == instr[curr_index].rt)
                {
                    for(int i = curr_index + 1; i < curr_index + 4; i++){
                        if(pipeline_log[prev_index][i + 1] == "WB"){
                            break;
                        }
                        else if(pipeline_log[prev_index][i] != "WB"){ 
                            pipeline_log[curr_index].insert(pipeline_log[curr_index].begin() + curr_index + 2, "stall");
                        }
                    }
                }
            }

            if(instr[prev_index].name == "lw"){ // lw $t0, 4($s3) lw rt, imm(rs)
                if(instr[prev_index].rt == instr[curr_index].rs || instr[prev_index].rt == instr[curr_index].rt)
                {
                    for(int i = curr_index + 1; i < curr_index + 4; i++){
                        if(pipeline_log[prev_index][i + 1] == "WB"){
                            break;
                        }
                        else if(pipeline_log[prev_index][i] != "WB"){
                            pipeline_log[curr_index].insert(pipeline_log[curr_index].begin() + curr_index + 2, "stall");
                        }
                    }
                }
            }
            
            // control hazards beq, bne
            if(instr[prev_index].name == "beq"){
                int possible_flushes = 0;
                for(int i = curr_index + 1; i < curr_index + 4; i++){
                    if(pipeline_log[prev_index][i] != "ID"){possible_flushes++;}
                    if(pipeline_log[prev_index][i + 1] == "ID"){ break;}
                }
            
                // if beq is true
                if(instr[prev_index].rs == instr[prev_index].rt){
                    pipeline_log[curr_index].insert(pipeline_log[curr_index].begin() + curr_index, "s"); // key for skip instruction since PC + 4
                    //std::cout << "skip!" << std::endl;
                    instr_index += instr[prev_index].address_imm;
                    instr_count--;
                }
                // if beq is false
                if(instr[curr_index].rs != instr[curr_index].rt){
                    // if time allows, iterate through how many instructions it takes for branch to decide
                    flushes[curr_index] = 2;
                }
            }

            if(instr[prev_index].name == "bne"){
                int possible_flushes = 0;
                for(int i = curr_index + 1; i < curr_index + 4; i++){
                    if(pipeline_log[prev_index][i] != "ID"){possible_flushes++;}
                    if(pipeline_log[prev_index][i + 1] == "ID"){ break;}
                }
                // if bne is true
                if(instr[prev_index].rs != instr[prev_index].rt){
                    pipeline_log[curr_index].insert(pipeline_log[curr_index].begin() + curr_index, "s"); // key for skip instruction since PC + 4
                    //std::cout << "skip!" << std::endl;
                    instr_index += instr[prev_index].address_imm;
                    instr_count--;
                }
                // if bne is false
                if(instr[curr_index].rs == instr[curr_index].rt){
                    // if time allows, iterate through how many instructions it takes for branch to decide
                    flushes[curr_index] = possible_flushes;
                }
            }
        }

        //std::cout << "instr_index: " << instr_index-1 << std::endl;
        // execute instruction after reaching WB stage
        if(instr[instr_index - 1].name == "add"){
            registers[instr[instr_index - 1].rd] = registers[instr[instr_index-1].rs] + registers[instr[instr_index-1].rt];
        }

        else if(instr[instr_index - 1].name == "addi"){
            registers[instr[instr_index - 1].rt] = registers[instr[instr_index-1].rs] + instr[instr_index-1].address_imm;
        }

        else if(instr[instr_index-1].name == "sub"){
            registers[instr[instr_index-1].rd] = registers[instr[instr_index-1].rs] - registers[instr[instr_index-1].rt];
        }

        else if(instr[instr_index-1].name == "lw"){
            registers[instr[instr_index-1].rt] = memory[registers[instr[instr_index-1].rs] + instr[instr_index-1].address_imm] ;
        }

        else if(instr[instr_index-1].name == "sw"){
            memory[registers[instr[instr_index-1].rs]] = registers[instr[instr_index-1].rt + instr[instr_index-1].address_imm];
        }

        instr_count--;
        if(instr_count <= 0){
            std::cout << "round complete!" << std::endl;
            std::cout << "no instructions left to simulate!" << std:: endl;
            break;
        }
        std::cout << "round complete!";
        std::cout << " Go through another round? (yes/no): ";
        std::cin >> user_input;
        while(user_input != "yes" && user_input != "no"){
            std::cout << "you must enter \"yes\" or \"no\" to proceed!: ";
            std::cin >> user_input;
            std::cin.ignore();
        }
    }
    
    std::ofstream output("output.txt");
    
    if (pipeline_log.size() > instruction_rounds) {
        pipeline_log.resize(instruction_rounds);
    }

    // longest instruction to print everything out
    int longest_instruction = 0;
    for (int i = 1; i < instruction_rounds; ++i) {
        if (pipeline_log[i].size() > pipeline_log[longest_instruction].size()) {
            longest_instruction = i;
        }
    }
    
    // Identify which instructions should be skipped (those with "s" in any column)
    std::vector<bool> instructions_to_skip(pipeline_log.size(), false);
    for (int i = 0; i < pipeline_log.size(); ++i) {
        for (int j = 0; j < pipeline_log[i].size(); ++j) {
            if (pipeline_log[i][j] == "s") {
                instructions_to_skip[i] = true;
                break; // No need to check further columns for this instruction
            }
        }
    }

    // main output
    int cc_counter = 1; // counter for CC# labels
    for (int j = 0; j < pipeline_log[longest_instruction].size(); ++j) {
        output << "CC#" << cc_counter++ << ": ";

        // iterate over each instruction
        for (int i = 0; i < pipeline_log.size(); ++i) {
            // insnstruction marked for skipping, skip it
            if (instructions_to_skip[i]) {
                continue;
            }

            // print instruction
            if (j < pipeline_log[i].size() && pipeline_log[i][j] != "") {
                output << "I" << i + 1 << "-" << pipeline_log[i][j] << " ";
            }
        }

        output << std::endl;
    }

    // registers output
    output << "REGISTERS" << std::endl;
    for(int i = 0; i < registers.size(); i++){
        if(registers[i] != 0){
            output << "R" << i << " " << registers[i] << std::endl;
        }
    }

    // memory output
    output << "MEMORY" << std::endl;
    for(int i = 0; i < memory.size(); i++){
        if(memory[i] != 0){
            output << i << " " << memory[i] << std::endl;
        }
    }

    input.close();
    output.close();

    return 0;
}
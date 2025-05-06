#ifndef HELPERS_H
#define HELPERS_H

#include<iostream>
#include<string>
#include<fstream>
#include<sstream>
#include<vector>
#include<string>
#include<algorithm>

struct instruction{
    std::string type; // "R" or "I"
    std::string name; // add, lw, sw, etc.
    std::string opcode;
    int rs;
    int rt;
    int rd;
    int shamt;
    std::string funct;
    int address_imm;
    std::string h;

    std::string current_stage;
};

int parseSignedBinary(std::string& bin) {
    int value = std::stoi(bin, nullptr, 2);
    // msb == 1 check
    if (bin[0] == '1') {
        value -= (1 << bin.length()); // subtract 2^16 since it's 16 bits
    }
    return value;
}

void pars(std::ifstream&input, std::vector<int>& registers, std::vector<int>& memory, std::vector<instruction>& instr){
    std::string line;
    while(getline(input, line)){
       
        line.erase(remove(line.begin(), line.end(), '\r'), line.end());
        line.erase(remove(line.begin(), line.end(), '\n'), line.end());

        if(line == "REGISTERS"){
            while(true){
                getline(input, line);
                if(line == "MEMORY" || line == "CODE" ){break;}

                int index = stoi(line.substr(1, line.find(' ') - 1));
                std::stringstream ss(line.substr(line.find(' ') + 1));
                int value;
                ss >> value;

                if(index == 0){continue;} // $zero is always zero
                registers[index] = value;
            }
        }

        if(line == "MEMORY"){
            while(true){
                getline(input, line);
                if(line == "REGISTERS" || line == "CODE" ){break;}
                std::stringstream ss(line);
                std::string memory_location, memory_contents;

                ss >> memory_location;
                ss >> memory_contents;

                memory[stoi(memory_location)] = stoi(memory_contents);
            }
        }

        if(line == "CODE"){
            while(getline(input, line)){
                if(line.empty()) {continue;}
                std::string s = line; // binary string from input file
                // parsing the binary string
                instruction obj;
                std::string opcode = s.substr(0, 6);
                obj.opcode = s.substr(0, 6); // Add this
                // r-type
                if(opcode == "000000"){
                    obj.type = "R";
                    obj.rs = stoi(s.substr(6, 5), nullptr, 2);
                    obj.rt = stoi(s.substr(11, 5), nullptr, 2);
                    obj.rd = stoi(s.substr(16, 5), nullptr, 2);
                    obj.shamt = stoi(s.substr(21, 5), nullptr, 2);
                    obj.funct = s.substr(26, 6);
                    if(obj.funct == "100000") {obj.name = "add";}
                    else if(obj.funct == "100010") {obj.name = "sub";}
                    else if (obj.funct == "101010") {obj.name = "slt";}
                    obj.address_imm = 0;
                }
                // i-type
                else{
                    obj.type = "I";
                    if(obj.opcode == "100011") {obj.name = "lw";}
                    else if(obj.opcode == "101011") {obj.name = "sw";}
                    else if (obj.opcode == "001000") {obj.name = "addi";}
                    else if (obj.opcode == "000100") {obj.name = "beq";}
                    else if (obj.opcode == "000101") {obj.name = "bne";}
                    obj.rs = stoi(s.substr(6, 5), nullptr, 2);
                    obj.rt = stoi(s.substr(11, 5), nullptr, 2);
                    obj.h=  s.substr(16, 16);
                    obj.address_imm = parseSignedBinary(obj.h); 
                    obj.shamt = 0;
                    obj.rd = 0;
                }
                instr.push_back(obj);
            }
        }
    }
}

#endif
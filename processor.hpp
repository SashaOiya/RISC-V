#pragma once

#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <list>

enum format_t {
    ER_FORMAT = 0,
    R = 1,
    I = 2,
    S = 3,
    B = 4,
    U = 5,
    J = 6
};

enum command_t {
    ER_COMMAND = 0,
    XORI = 1,
    SLLI = 2,
    SRLI = 3,
    ADDI = 4,
    SRAI = 5,
    ADD  = 6,
    SLTI = 7,
    AND  = 8,
    ANDI = 9,
    AUIPC = 10,
    BEQ  = 11
};

template <typename Bit_t>
class processor_c {
    struct request_t {
        void (*func)(request_t &request) = nullptr;
        std::string name = {}; // for debug
        size_t rd = -1;
        int rs1 = -1;
        int rs2 = -1;
        Bit_t immediate = 0;
    };

    struct node_t {
        Bit_t opcode = 0;
        Bit_t funct3 = 0;
        format_t format = ER_FORMAT;
        command_t name2 = ER_COMMAND; 
        std::string name = {}; // +?
        void (*func)(request_t &request) {}; // ++
        Bit_t funct7 = 0;
    };  
                                                                    
    typename std::vector<Bit_t> data = {};
    std::list<request_t> opcodes_storage = {};
    std::vector<int> x = {};

    std::string input_file_name_ = {}; // remove

    static const int command_n = 11;
    const node_t command[command_n] = {{0b0010011, 0b100, I, XORI, "XORI", xori_}, 
                                       {0b0010011, 0b101, R, SRLI, "SRLI", srli_, 0b0000000}, 
                                       {0b0010011, 0b101, R, SRAI, "SRAI", 0b0100000}, 
                                       {0b0010011, 0b000, I, ADDI, "ADDI"}, {0b0110011, 0b000, R, ADD , "ADD" , 0b0000000},
                                       {0b0010011, 0b010, I, SLTI, "SLTI"}, {0b0110011, 0b111, R, AND , "AND" , 0b0000000}, 
                                       {0b0010011, 0b111, I, ANDI, "ANDI"}, {0b0010111, 0b000, U, AUIPC, "AUIPC" },
                                       {0b1100011, 0b000, B, BEQ,  "BEQ" }/*, {0b0010011, 0b001, I, SLLI, "SLLI", slli_}*/ };
public:
    processor_c ( const char *input_file_name ) : input_file_name_(input_file_name) {
        x.reserve ( 31 );
        std::ifstream in (input_file_name_);  // remove
        if (in.is_open())
        {
            size_t size = in.tellg(); 
            std::cout << size << '\n';
            std::string str(size, '\0'); 
            in.seekg(0);
            in.read ( &str[0], size );
            data = str;
        }
        in.close();
        // reserve
        // processor
    }

    void Processor ()
    {
        size_t size = data.size();
        for ( size_t i = 0; i < size; ++i ) {
            request_t node = {};
            for ( int j = 0; j < command_n; ++j ) {      // baaaaaaaaaaad
                if ( ((data[i] & 0b1111111)  == command[j].opcode) ) {
                    bool flag = false;
                    switch (command[j].format) {
                        case I:{
                            if (((data[i]>>12) & 0b111)  == command[j].funct3) {
                                node.func = command[j].func; // move
                                node.name = command[j].name;
                                node.rd  = (data[i] >> 7) & 0b11111;
                                node.rs1 = (data[i] >> 15)& 0b11111;
                                node.immediate = data[i] >> 20;
                                flag = true;
                            }
                        } 
                        break;
                        case U:{
                            node.func = command[j].func; // move
                            node.name = command[j].name;
                            node.immediate = data[i] >> 12; // strange 
                            flag = true;
                        }
                        break;
                        case R:{
                            if ((((data[i]>>12) & 0b111)  == command[j].funct3 ) && 
                                ((data[i] >> 25) == command[j].funct7)) {
                                node.func = command[j].func; // move
                                node.name = command[j].name;
                                node.rd  = (data[i] >> 7) & 0b11111;
                                node.rs1 = (data[i] >> 15)& 0b11111;
                                node.rs2 = (data[i] >> 20)& 0b11111;
                                flag = true;
                            }
                        }
                        break;
                        case S: {
                            if (((data[i]>>12) & 0b111)  == command[j].funct3) {
                                node.func = command[j].func; // move
                                node.name = command[j].name;
                                node.rs1 = (data[i] >> 15)& 0b11111;
                                node.rs2 = (data[i] >> 20)& 0b11111;
                                node.immediate = ((data[i] >> 7)&  0b11111) | ((data[i] >> 25)<< 5);
                                flag = true;
                            }
                        }
                        break;
                        case B: {
                            if (((data[i]>>12) & 0b111)  == command[j].funct3) {
                                node.func = command[j].func; // move
                                node.name = command[j].name;
                                node.rs1 = (data[i] >> 15)& 0b11111;
                                node.rs2 = (data[i] >> 20)& 0b11111;
                                node.immediate = ((data[i] >> 8)&  0b11110) | (((data[i] >> 25)& 0b111111)<< 5) | (((data[i] >> 7)& 0b1)<< 11) | ((data[i] >> 31)<< 12);
                                flag = true;
                            }
                        }
                        case J: {
                            node.func = command[j].func; // move
                            node.name = command[j].name;
                            node.rd  = (data[i] >> 7) & 0b11111;
                            node.immediate = ((data[i] >> 20)&  0b11111111110) | (((data[i] >> 20)& 0b1)<< 11) | (((data[i] >> 12)& 0b11111111)<< 12) | ((data[i] >> 31)<< 20);
                            flag = true;
                        }
                        default :{
                            ;//errors;
                        }
                        break;
                    }
                    if ( flag ) {
                        opcodes_storage.push_back(node);
                        break;
                    }

                }
            } 
            assert ( opcodes_storage.size() == i );
        }
    }

private:

    void processing () {
        request_t fetch = opcodes_storage.pop_front();
        // end check
        return fetch.func(fetch);
    }

    void xori_ (request_t &request) {
        x[request.rd] = x[request.rs1] ^ request.immediate;
        /*request_t fetch = opcodes_storage.pop_front();
        return fetch.func(fetch);*/
    }

    void srli_ (request_t &request) {
        x[request.rd] = x[request.rs1] >> request.rs2;
        //request_t fetch = opcodes_storage.pop_front();
        //return fetch.func(fetch);
    }

    void srai_ (request_t &request) { // noooooooooooooooooo
        const int sign = x[request.rs1]>>31;
        x[request.rd] = x[request.rs1] >> request.rs2;
        //request_t fetch = opcodes_storage.pop_front();
        //return fetch.func(fetch);
    }

    void addi_ (request_t &request) {
        x[request.rd] = x[request.rs1] + request.immediate;
        //request_t fetch = opcodes_storage.pop_front();
        //return fetch.func(fetch);
    }

    void add_ (request_t &request) {
        x[request.rd] = x[request.rs1] + x[request.rs2];
        auto &fetch_itt =  opcodes_storage.begin();
        // end
        //return fetch.func(fetch);
    }

    void andi_ (request_t &request) {
        x[request.rd] = x[request.rs1] & request.immediate;
    }

    void and_ (request_t &request) {
        x[request.rd] = x[request.rs1] & x[request.rs2];
    } 

};
/*

Error_t Processor ( struct Vm_t *vm_spu )
{
    assert ( vm_spu != nullptr );

    int max_ip = vm_spu->file_size * sizeof ( elem_t );

    Stack_t ret_stack  = {};
    Stack_Ctor ( &ret_stack );

    for ( int ip = 0; ip < max_ip; ++ip ) {
        if ( Processing ( vm_spu, &ip, &ret_stack ) == ARG_END ) {

            break;
        }
        // else
        Stack_Dump ( &(vm_spu->stack), INFORMATION );
    }
$
    Stack_Dtor ( &ret_stack );

    return NO_ERR;
}

Arg_Indicator_t Processing ( struct Vm_t *vm_spu, int *ip, Stack_t *ret_stack )
{
    int opcode = vm_spu->data[*ip] & 0b1111111;

    int register_flag = vm_spu->data[*ip] & 0b100000;
    int const_flag    = vm_spu->data[*ip] & 0b1000000;
    int ram_flag      = vm_spu->data[*ip] & 0b10000000;
    int value = 0;


    return ARG_NO_ERROR;
}

*/
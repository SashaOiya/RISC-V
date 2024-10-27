#pragma once

#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <list>
#include "AutoExpandvector.hpp"
#include <elfio/elfio.hpp>

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
    ADDI = 1,
    ADD = 2,
    AND = 3,
    ANDI = 4,
    BEQ = 5,
    BGE = 6,
    BGEU = 7,
    BLT = 8,
    BLTU = 9,
    BNE = 10,
    JAL,
    JALR,
    LB,
    LBU,
    LD,
    LH,
    LHU,
    LUI,
    LW,
    OR,
    ORI,
    SB,
    SH,
    SLL,
    SLLI,
    SLT,
    SLTIU,
    SLTU,
    SRA,
    SRL,
    SRLI,
    SRAI, //
    SLTI,
    SUB,
    SW,
    XOR,
    XORI
};

struct SectionData {
    std::string name;
    std::vector<uint8_t> data;
};

std::vector<SectionData> readElfSections(const std::string& filename) {
    ELFIO::elfio reader;
    std::vector<SectionData> sections;

    // Открываем ELF-файл
    if (!reader.load(filename)) {
        std::cerr << "Error: Could not open ELF file " << filename << std::endl;
        return sections;
    }

    // Проходим по всем секциям и сохраняем их содержимое в векторах
    for (int i = 0; i < reader.sections.size(); ++i) {
        const ELFIO::section* sec = reader.sections[i];

        // Считываем данные секции в вектор
        std::vector<uint8_t> data(sec->get_data(), sec->get_data() + sec->get_size());

        // Добавляем секцию в наш контейнер
        sections.push_back({ sec->get_name(), data });
    }

    return sections;
}

template <typename Bit_t> // int32_t
struct request_t {
    void (*func)(request_t &request) = nullptr;
    std::string name = {}; // for debug
    size_t rd = -1;
    int rs1 = -1;
    int rs2 = -1;
    Bit_t immediate = 0;
};

template <typename Bit_t>
class processor {
    struct node_t {
        Bit_t opcode = 0;
        Bit_t funct3 = 0;
        format_t format = ER_FORMAT;
        command_t name2 = ER_COMMAND; 
        std::string name = {}; // +?
        void (processor<Bit_t>::*Metric)(request_t<Bit_t> &request);
        Bit_t funct7 = 0;
    };  
                                                                    
    int program_counter = 0; 
    size_t all_command_n = 0;
    std::vector<request_t<Bit_t>> opcodes_storage = {};
    std::vector<int> registers = {}; // name every registers serch in Internet
    // chaeck x0 on 0
    AutoExpandVector<Bit_t> memory{};

    std::string input_file_name_ = {}; // remove

    static const int command_n = 39;
    const node_t command[command_n] = {{0b0010011, 0b000, I, ADDI, "ADDI", &processor::addi_}, 
                                       {0b0110011, 0b000, R, ADD , "ADD" , &processor::add_, 0b0000000},
                                       {0b0110011, 0b111, R, AND , "AND" , &processor::and_,  0b0000000}, 
                                       {0b0010011, 0b111, I, ANDI, "ANDI", &processor::andi_}, 
                                       //{0b0010111, 0b000, U, AUIPC, "AUIPC", &processor::auipc_ },
                                       {0b1100011, 0b000, B, BEQ , "BEQ" , &processor::beq_ }, 
                                       {0b1100011, 0b101, B, BGE , "BGE" , &processor::bge_ },
                                       {0b1100011, 0b111, B, BGEU, "BGEU", &processor::bgeu_},
                                       {0b1100011, 0b100, B, BLT , "BLT" , &processor::blt_ },
                                       {0b1100011, 0b110, B, BLTU, "BLTU", &processor::bltu_},
                                       {0b1100011, 0b001, B, BNE , "BNE" , &processor::bne_ },
                                       {0b1101111, 0b000, J, JAL , "JAL" , &processor::jal_ },
                                       {0b1100111, 0b000, I, JALR, "JALR", &processor::jalr_},
                                       {0b0000011, 0b000, I, LB  , "LB"  , &processor::lb_  },
                                       {0b0000011, 0b100, I, LBU , "LBU" , &processor::lbu_ },
                                       {0b0000011, 0b011, I, LD  , "LD"  , &processor::ld_  },
                                       {0b0000011, 0b001, I, LH  , "LH"  , &processor::lh_  },
                                       {0b0000011, 0b101, I, LHU , "LHU" , &processor::lhu_ },
                                       {0b0110111, 0b000, U, LUI , "LUI" , &processor::lui_ },
                                       {0b0000011, 0b010, I, LW  , "LW"  , &processor::lw_  },
                                       {0b0110011, 0b110, R, OR  , "OR"  , &processor::or_ , 0b0000000 },
                                       {0b0010011, 0b110, I, ORI , "ORI" , &processor::ori_ },
                                       {0b0100011, 0b011, S, SB  , "SB"  , &processor::sb_  },
                                       {0b0100011, 0b001, S, SH  , "SH"  , &processor::sh_  },
                                       {0b0110011, 0b001, R, SLL , "SLL" , &processor::sll_, 0b0000000 },
                                       {0b0010011, 0b001, I, SLLI, "SLLI", &processor::slli_},
                                       {0b0110011, 0b010, R, SLT , "SLT" , &processor::slt_, 0b0000000 },
                                       {0b0010011, 0b010, I, SLTI, "SLTI", &processor::slti_},
                                       {0b0010011, 0b011, I, SLTIU,"SLTIU",&processor::sltiu_},
                                       {0b0110011, 0b011, R, SLTU, "SLTU", &processor::sltu_, 0b0000000},
                                       {0b0110011, 0b101, R, SRA , "SRA" , &processor::sra_ },
                                       {0b0110011, 0b101, S, SRL , "SRL" , &processor::srl_ },
                                       {0b0010011, 0b101, R, SRLI, "SRLI", &processor::srli_, 0b0000000}, 
                                       {0b0010011, 0b101, R, SRAI, "SRAI", &processor::srai_, 0b0100000}, 
                                       {0b0110011, 0b000, R, SUB , "SUB" , &processor::sub_ },
                                       {0b0100011, 0b010, S, SW  , "SW"  , &processor::sw_  },
                                       {0b0110011, 0b100, R, XOR , "XOR" , &processor::xor_, 0b0000000},
                                       {0b0010011, 0b100, I, XORI, "XORI", &processor::xori_} };
public:
    processor ( const char *input_file_name ) : input_file_name_(input_file_name) {
        typename std::vector<Bit_t> data = {};
        /*registers.reserve ( 32 );
        std::ifstream in (input_file_name_);  // remove
        if (in.is_open())
        {
        }
        in.close();
        // reserve
        // processor*/
        // all_command_n
    }

    void next_fetch () {
        assert ( program_counter <= all_command_n) 
        if (program_counter < all_command_n ) {
            auto fetch = opcodes_storage[program_counter];
            ++program_counter; // may be error with jump
            return fetch.fun(fetch); // strange
        }
        return ;
    }

private:
    Bit_t sign_extend(int32_t value, int bits) {
        int32_t shift = 32 - bits;

        return (value << shift) >> shift; // Знаковое расширение
    }

    void Processor () // cpu look like calculator
    {
        size_t size = data.size();
        for ( size_t i = 0; i < size; ++i ) {
            request_t<Bit_t> node = {};
            for ( int j = 0; j < command_n; ++j ) {      // baaaaaaaaaaad
                if ( (data[i] & 0b1111111)  == command[j].opcode ) {
                    bool flag = true;
                    switch (command[j].format) {
                        case I:{
                            if (((data[i]>>12) & 0b111)  == command[j].funct3) {
                                node.rd  = (data[i] >> 7) & 0b11111;
                                node.rs1 = (data[i] >> 15)& 0b11111;
                                node.immediate = sign_extend ( data[i] >> 20, 12 );
                            }
                        } 
                        break;
                        case U:{
                            node.immediate = data[i] & 0xFFFFF000;  
                        }
                        break;
                        case R:{
                            if ((((data[i]>>12) & 0b111)  == command[j].funct3 ) && 
                                ((data[i] >> 25) == command[j].funct7)) {
                                node.rd  = (data[i] >> 7) & 0b11111;
                                node.rs1 = (data[i] >> 15)& 0b11111;
                                node.rs2 = (data[i] >> 20)& 0b11111;
                            }
                        }
                        break;
                        case S: {
                            if (((data[i]>>12) & 0b111)  == command[j].funct3) {
                                node.rs1 = (data[i] >> 15)& 0b11111;
                                node.rs2 = (data[i] >> 20)& 0b11111;
                                node.immediate = (data[i] >> 7)&  0b11111) | ((data[i] >> 25)<< 5);
                                node.immediate = sign_extend ( node.immediate, 12);
                            }
                        }
                        break;
                        case B: {
                            if (((data[i]>>12) & 0b111)  == command[j].funct3) {
                                node.rs1 = (data[i] >> 15)& 0b11111;
                                node.rs2 = (data[i] >> 20)& 0b11111;
                                node.immediate = (((data[i] >> 8)&  0b11110) << 1 )   | (((data[i] >> 25)& 0b111111)<< 5) | 
                                                 (((data[i] >> 7)& 0b1)<< 11) | ((data[i] >> 31)<< 12);
                                node.immediate = sign_extend ( node.immediate, 13 );
                            }
                        }
                        break;
                        case J: {
                            node.rd  = (data[i] >> 7) & 0b11111;
                            node.immediate = ((data[i] >> 20)&  0b11111111110)    | 
                                             (((data[i] >> 20)& 0b1)<< 11)        | 
                                             (((data[i] >> 12)& 0b11111111)<< 12) | 
                                             ((data[i] >> 31)<< 20);
                            node.immediate = sign_extend ( node.immediate, 21 );
                        }
                        break;
                        default :{
                            flag = false;
                        }
                        break;
                    }
                    if ( flag ) {
                        node.func = command[j].func; // move // strange
                        node.name = command[j].name;
                        opcodes_storage.push_back(node);
                        break;
                    }

                }
            } 
            //assert ( opcodes_storage.size() == i );
        }
    }

void addi_ (request_t<Bit_t> &request) {
    registers[request.rd] = registers[request.rs1] + request.immediate;
    return next_fetch();
}

void add_ (request_t<Bit_t> &request) {
    registers[request.rd] = registers[request.rs1] + registers[request.rs2];
    return next_fetch();
}

void andi_ (request_t<Bit_t> &request) {
    registers[request.rd] = registers[request.rs1] & request.immediate;
    return next_fetch();
}

void and_ (request_t<Bit_t> &request) {
    registers[request.rd] = registers[request.rs1] & registers[request.rs2];
    return next_fetch();
} 

void beq_ (request_t<Bit_t> &request) { // noooooooooo
    if ( registers[request.rs1] == registers[request.rs2] ) { // check sign
        program_counter += request.immediate / 4;
    }
    return next_fetch();
} 

void bge_ (request_t<Bit_t> &request) { // noooooooooo
    if ( registers[request.rs1] >= registers[request.rs2] ) { // check sign
        program_counter += request.immediate / 4;  
    }
    return next_fetch();
} 

void bgeu_ (request_t<Bit_t> &request) { // noooooooooo
    if ( registers[request.rs1] >= registers[request.rs2] ) {  // check sign
        program_counter += request.immediate / 4;
    }
    return next_fetch();
}

void blt_ (request_t<Bit_t> &request) { // noooooooooo
    if ( registers[request.rs1] < registers[request.rs2] ) { //  signed rs2
        program_counter += request.immediate / 4;
    }
    return next_fetch();
}

void bltu_ (request_t<Bit_t> &request) { // noooooooooo
    if ( registers[request.rs1] < registers[request.rs2] ) { //  unsigned rs2
        program_counter += request.immediate / 4; 
    }
    return next_fetch();
}

void bne_ (request_t<Bit_t> &request) { // noooooooooo
    if ( registers[request.rs1] != registers[request.rs2] ) { 
        program_counter += request.immediate / 4; // because imm in byte 
    }
    return next_fetch();
}

void jal_ (request_t<Bit_t> &request) { // noooooooooo
    registers[request.rd] = program_counter + 1; // strange
    program_counter += request.immediate / 4;
    return next_fetch();
}

void jalr_ (request_t<Bit_t> &request) { // noooooooooo
    size_t t = program_counter + 1; // check behavior
    program_counter = ( registers [request.rs1] + request.immediate ) / 4) & ∼1
    registers[request.rd] = t;
    return next_fetch();
}

void lb_ (request_t<Bit_t> &request) { // noooooooooo
    registers[request.rd] = sign_extend ( memory[registers[request.rs1] + request.immediate] & 0xFF, 8 ); 
    //Загрузить байт. Скопировать один байт из памяти по адресу imm+rs1 в регистр rd. Sign
    return next_fetch();
}

void lbu_ (request_t<Bit_t> &request) { // noooooooooo
    registers[request.rd] = memory[registers[request.rs1] + request.immediate]  & 0xFF;
    //Загрузить байт без знака. 
    //Скопировать один байт из памяти по адресу imm+rs1 в регистр rd.
    return next_fetch();
}

void ld_ (request_t<Bit_t> &request) { // noooooooooo
    //x[rd] = memory[registers[request.rs1] + request.immediate][63:0];
    //Загрузить двойное слово знаковое.  Sext
    //Скопировать двойное слово из памяти по адресу imm+rs1 в регистр rd.
    return next_fetch();
}

void lh_ (request_t<Bit_t> &request) { // noooooooooo
    registers[request.rd] = sign_extend ( memory[registers[request.rs1] + request.immediate]  & 0xFFFF, 16);
    //Загрузить половину слова. Скопировать половину слова из памяти по адресу imm+rs1 в регистр rd. Sext
    return next_fetch();
}

void lhu_ (request_t<Bit_t> &request) { // noooooooooo
    registers[request.rd] = memory[registers[request.rs1] + request.immediate]  & 0xFFFF;
    //Загрузить полуслово без знака. Скопировать полуслово из памяти по адресу imm+rs1 в регистр rd.
    return next_fetch();
}

void lui_ (request_t<Bit_t> &request) { // noooooooooo
    //x[rd] = sext(imm[31:12] << 12)
    //Поместите верхние 20 бит imm в rd и заполните нижние 12 бит нулями.
    return next_fetch();
}

void lw_ (request_t<Bit_t> &request) { // noooooooooo
    registers[request.rd] = memory[registers[request.rs1] + request.immediate];
    //Загрузить слово. Скопировать слово из памяти по адресу imm+rs1 в регистр rd. Sign
    return next_fetch();
}

void or_ (request_t<Bit_t> &request) { // noooooooooo
    registers[request.rd] = registers[request.rs1] | registers[request.rs2];
    return next_fetch();
}

void ori_ (request_t<Bit_t> &request) { // noooooooooo
    registers[request.rd] = registers[request.rs1] | registers[request.immediate];
    return next_fetch();
}

void sb_ (request_t<Bit_t> &request) { // noooooooooo
    memory[registers[request.rs1] + request.immediate] = registers[request.rs2] & 0xFF;
    return next_fetch();
}

void sd_ (request_t<Bit_t> &request) { // noooooooooo
    //memory[registers[request.rs1] + request.immediate] = registers[request.rs2] [63:0];
    //Сохраните двойное слово. 
    return next_fetch();
}

void sh_ (request_t<Bit_t> &request) { // noooooooooo
    memory[registers[request.rs1] + request.immediate] = registers[request.rs2] & 0xFFFF;
    //Сохраните двойное слово. 
    return next_fetch();
}

void sll_ (request_t<Bit_t> &request) { // noooooooooo
    registers[request.rd] = registers[request.rs1] << registers[request.rs2];
    //Логический сдвиг влево. Используются только младшие 5 битов в rs2.
    return next_fetch();
}

/////////

void xori_ (request_t<Bit_t> &request) {
    registers[request.rd] = registers[request.rs1] ^ request.immediate;
    return next_fetch();
}

void srli_ (request_t<Bit_t> &request) {
    registers[request.rd] = registers[request.rs1] >> request.rs2;
    return next_fetch();
}

void srai_ (request_t<Bit_t> &request) { // noooooooooooooooooo
    const int sign = registers[request.rs1]>>31;
    registers[request.rd] = registers[request.rs1] >> request.rs2;
    return next_fetch();
}


void slti_ (request_t<Bit_t> &request) { // noooooooooo
    registers[request.rd] = (registers[request.rs1] < registers[request.immediate]) ? 1 : 0;
    return next_fetch();
} 



};

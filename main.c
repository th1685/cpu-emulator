#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
/*#include <time.h>*/

#include "graphics.h"

typedef struct _registers {
    uint8_t* reg_A;
    uint8_t* reg_B;
    uint8_t* reg_C;
    
    uint8_t* program_counter;
    uint8_t* status_register;
    uint8_t* stack_pointer;
    uint8_t* result;
} REGISTERS;

/*void delay(int mill) {
	clock_t start_time = 1000 * clock() / CLOCKS_PER_SEC;
	while ((1000 * clock() / CLOCKS_PER_SEC) < start_time + mill);
}*/

unsigned int int_to_int(unsigned int k) {
    return (k == 0 || k == 1 ? k : ((k % 2) + 10 * int_to_int(k / 2)));
}

// void ripple_carry_add(uint8_t* register_in, uint8_t value, REGISTERS* registers) {
//     uint8_t overflow_test = *register_in;
//     *register_in += value;
//     if(*register_in<overflow_test) *registers->status_register |= (1<<6); /*overflow/zero flag*/
//     return;
// }

// void ripple_carry_subtract(uint8_t* register_in, uint8_t value, REGISTERS* registers) {
//     uint8_t overflow_test = *register_in;
//     *register_in -= value;
//     if(*register_in>overflow_test) *registers->status_register |= (1<<6); /*overflow/zero flag*/
//     return;
// }

uint8_t ripple_carry_add(uint8_t* register_in, uint8_t value, REGISTERS* registers) {
    /*real ripple carry adder -> redirect output to first input register, send overflow to overflow status flag*/
    /*see above commented-out function*/
    uint8_t output = *register_in;
    output += value;
    if(output<*register_in) *registers->status_register |= (1<<6); /*set overflow flag*/
    else *registers->status_register &= ~(1<<6); /*clear overflow flag*/
    return output;
}

uint8_t ripple_carry_subtract(uint8_t* register_in, uint8_t value, REGISTERS* registers) {
    /*real ripple carry subtractor -> redirect output to first input register, send overflow to overflow status flag*/
    /*see above commented-out function*/
    uint8_t output = *register_in;
    output -= value;
    if(output>*register_in) *registers->status_register |= (1<<6); /*set overflow flag*/
    else *registers->status_register &= ~(1<<6); /*clear overflow flag*/
    return output;
}

void compare_greater_than(uint8_t* register_in1, uint8_t* register_in2, REGISTERS* registers) {
    /*THIS BEHAVES AS A GREATER THAN OR EQUAL TO*/
    /*a>b => a-b>0*/
    uint8_t check = ripple_carry_subtract(register_in1, *register_in2, registers);
    /*check overflow flag -> if subtraction caused overflow, b>a => a<b else a>b*/
    uint8_t overflow_test = (*registers->status_register >> 6) & 1; /*isolate overflow flag*/
    
    if(!overflow_test) *registers->status_register |= (1<<7); /*set compare flag*/
    else *registers->status_register &= ~(1<<7); /*clear compare flag*/
}

void compare_less_than(uint8_t* register_in1, uint8_t* register_in2, REGISTERS* registers) {
    /*a<b => a-b<0 => b-a>0*/
    uint8_t check = ripple_carry_subtract(register_in1, *register_in2, registers);
    /*check overflow flag -> if subtraction caused overflow, b>a => a<b else a>b*/
    uint8_t overflow_test = (*registers->status_register >> 6) & 1; /*isolate overflow flag*/

    if(overflow_test) *registers->status_register |= (1<<7); /*set compare flag*/
    else *registers->status_register &= ~(1<<7); /*clear compare flag*/
}

void compare_equal_to(uint8_t* register_in1, uint8_t* register_in2, REGISTERS* registers) {
    /*real equality check -> a=b => a-b=0 therefore if any bit is on in the result, the result is non-zero*/
    uint8_t check = ripple_carry_subtract(register_in1, *register_in2, registers);
    uint8_t result = (check & 1) | (check & 2) >> 1 | (check & 4) >> 2 | (check & 8) >> 3 | (check & 16) >> 4 | (check & 32) >> 5 | (check & 64) >> 6 | (check & 128) >> 7;
    *registers->status_register |= (!result<<7); /*compare flag*/
}

void branch(uint8_t address, REGISTERS* registers) {
    /*in real branch, given address is 'released' to program counter depending on signal coming from compare flag*/
    uint8_t compare_test = (*registers->status_register >> 7) & 1; /*isolate compare flag*/
    if(compare_test) registers->program_counter[0] = address;

    return;
}

void exec_ins(uint16_t word, REGISTERS* registers, uint8_t* ram, uint8_t* stack, PANEL* panel) {
    /*print current instruction*/
    wprintw(panel->instruction_window, "%x:\t%x\n", *registers->program_counter-2, word);

    /*split into 4 byte chunks*/
    uint8_t word0 = word >> 8;
    uint8_t word1 = word & 0xff;

    uint8_t chunk0 = (word0 & 0xf0) >> 4;
    uint8_t chunk1 = word0 & 0x0f;
    uint8_t chunk2 = (word1 & 0xf0) >> 4;
    uint8_t chunk3 = word1 & 0x0f;

    switch(chunk0) { /*instruction*/
        case 0x00: /*no operation*/
        wprintw(panel->debug_window, "[%2x]chunk0: %x\tno operation\n", *registers->program_counter-2, chunk0);
        break;

        case 0x01: /*store register in memory*/
        wprintw(panel->debug_window, "[%2x]chunk0: %x\tstore register in memory\n", *registers->program_counter-2, chunk0);
        switch(chunk1) {
            case 0x00: /*register A*/
            wprintw(panel->debug_window, "[%2x]\tchunk1: %x\tregister A\n", *registers->program_counter-2, chunk1);
            ram[((chunk2 << 4) + chunk3)] = *registers->reg_A;
            wprintw(panel->debug_window, "[%2x]\taddress (chunk2+chunk3): %x\n", *registers->program_counter-2, ((chunk2 << 4) + chunk3));
            break;

            case 0x01: /*register B*/
            wprintw(panel->debug_window, "[%2x]\tchunk1: %x\tregister B\n", *registers->program_counter-2, chunk1);
            ram[((chunk2 << 4) + chunk3)] = *registers->reg_B;
            wprintw(panel->debug_window, "[%2x]\taddress (chunk2+chunk3): %x\n", *registers->program_counter-2, ((chunk2 << 4) + chunk3));
            break;

            case 0x02: /*register C*/
            wprintw(panel->debug_window, "[%2x]\tchunk1: %x\tregister C\n", *registers->program_counter-2, chunk1);
            ram[((chunk2 << 4) + chunk3)] = *registers->reg_C;
            wprintw(panel->debug_window, "[%2x]\taddress (chunk2+chunk3): %x\n", *registers->program_counter-2, ((chunk2 << 4) + chunk3));
            break;

            case 0x03: /*register R*/
            wprintw(panel->debug_window, "[%2x]\tchunk1: %x\tregister R\n", *registers->program_counter-2, chunk1);
            ram[((chunk2 << 4) + chunk3)] = *registers->result;
            wprintw(panel->debug_window, "[%2x]\taddress (chunk2+chunk3): %x\n", *registers->program_counter-2, ((chunk2 << 4) + chunk3));
            break;

            default:
            wprintw(panel->debug_window, "[%2x]\tchunk1: %x\tillegal instruction\n", *registers->program_counter-2, chunk1);
            break;
        }
        break;

        case 0x02: /*set register to value*/
        wprintw(panel->debug_window, "[%2x]chunk0: %x\tset register to value\n", *registers->program_counter-2, chunk0);
        switch(chunk1) {
            case 0x00: /*register A - immediate mode*/
            wprintw(panel->debug_window, "[%2x]\tchunk1: %x\tregister A - immediate mode\n", *registers->program_counter-2, chunk1);
            *registers->reg_A = (chunk2 << 4) + chunk3;
            wprintw(panel->debug_window, "[%2x]\tvalue (chunk2+chunk3): %x\n", *registers->program_counter-2, *registers->reg_A);
            break;

            case 0x01: /*register B - immediate mode*/
            wprintw(panel->debug_window, "[%2x]\tchunk1: %x\tregister B - immediate mode\n", *registers->program_counter-2, chunk1);
            *registers->reg_B = (chunk2 << 4) + chunk3;
            wprintw(panel->debug_window, "[%2x]\tvalue (chunk2+chunk3): %x\n", *registers->program_counter-2, *registers->reg_B);
            break;

            case 0x02: /*register C - immediate mode*/
            wprintw(panel->debug_window, "[%2x]\tchunk1: %x\tregister C - immediate mode\n", *registers->program_counter-2, chunk1);
            *registers->reg_C = (chunk2 << 4) + chunk3;
            wprintw(panel->debug_window, "[%2x]\tvalue (chunk2+chunk3): %x\n", *registers->program_counter-2, *registers->reg_C);
            break;

            /*addressing mode - load register from memory*/
            case 0x03: /*register A - addressing mode*/
            wprintw(panel->debug_window, "[%2x]\tchunk1: %x\tregister A - addressing mode\n", *registers->program_counter-2, chunk1);
            *registers->reg_A = ram[((chunk2 << 4) + chunk3)];
            wprintw(panel->debug_window, "[%2x]\taddress (chunk2+chunk3): %x\n", *registers->program_counter-2, ((chunk2 << 4) + chunk3));
            break;

            case 0x04: /*register B - addressing mode*/
            wprintw(panel->debug_window, "[%2x]\tchunk1: %x\tregister B - addressing mode\n", *registers->program_counter-2, chunk1);
            *registers->reg_B = ram[((chunk2 << 4) + chunk3)];
            wprintw(panel->debug_window, "[%2x]\taddress (chunk2+chunk3): %x\n", *registers->program_counter-2, ((chunk2 << 4) + chunk3));
            break;

            case 0x05: /*register C - addressing mode*/
            wprintw(panel->debug_window, "[%2x]\tchunk1: %x\tregister C - addressing mode\n", *registers->program_counter-2, chunk1);
            *registers->reg_C = ram[((chunk2 << 4) + chunk3)];
            wprintw(panel->debug_window, "[%2x]\taddress (chunk2+chunk3): %x\n", *registers->program_counter-2, ((chunk2 << 4) + chunk3));
            break;

            default:
            wprintw(panel->debug_window, "[%2x]\tchunk1: %x\tillegal instruction\n", *registers->program_counter-2, chunk1);
            break;
        }
        break;

        case 0x03: /*jump*/
        wprintw(panel->debug_window, "[%2x]chunk0: %x\tjump\n", *registers->program_counter-2, chunk0);
        switch(chunk1){
            case 0x00: /*jump to subroutine*/
            wprintw(panel->debug_window, "[%2x]\tchunk1: %x\tto subroutine at: %x\n", *registers->program_counter-2, chunk1, ((chunk2 << 4) + chunk3));
            registers->stack_pointer[0]++;
            stack[*registers->stack_pointer] = *registers->program_counter;
            *registers->program_counter = (chunk2 << 4) + chunk3;
            break;

            case 0x01: /*unconditional jump*/
            wprintw(panel->debug_window, "[%2x]\tchunk1: %x\tunconditionally to address: %x\n", *registers->program_counter-2, chunk1, ((chunk2 << 4) + chunk3));
            *registers->program_counter = (chunk2 << 4) + chunk3;
            break;

            case 0x02: /*return from subroutine*/
            wprintw(panel->debug_window, "[%2x]\tchunk1: %x\treturn from subroutine to address: %x\n", *registers->program_counter-2, chunk1, stack[*registers->stack_pointer]);
            *registers->program_counter = stack[*registers->stack_pointer];
            registers->stack_pointer[0]--;
            break;

            default:
            wprintw(panel->debug_window, "[%2x]\tchunk1: %x\tillegal instruction\n", *registers->program_counter-2, chunk1);
            break;
        }
        break;

        case 0x04: /*increment/decrement*/
        wprintw(panel->debug_window, "[%2x]chunk0: %x\tincrement/decrement: \n", *registers->program_counter-2, chunk0);
        switch(chunk1) {
            case 0x00: /*increment*/
            wprintw(panel->debug_window, "[%2x]\tchunk1: %x\tincrement\n", *registers->program_counter-2, chunk1);
            switch(chunk2) {
                case 0x00: /*register A*/
                wprintw(panel->debug_window, "[%2x]\t\tchunk2: %x\tregister A\n", *registers->program_counter-2, chunk2);
                registers->result[0] = ripple_carry_add(registers->reg_A, (uint8_t)0x01, registers);
                break;

                case 0x01: /*register B*/
                wprintw(panel->debug_window, "[%2x]\t\tchunk2: %x\tregister B\n", *registers->program_counter-2, chunk2);
                registers->result[0] = ripple_carry_add(registers->reg_B, (uint8_t)0x01, registers);
                break;

                case 0x02: /*register C*/
                wprintw(panel->debug_window, "[%2x]\t\tchunk2: %x\tregister C\n", *registers->program_counter-2, chunk2);
                registers->result[0] = ripple_carry_add(registers->reg_C, (uint8_t)0x01, registers);
                break;

                default:
                wprintw(panel->debug_window, "[%2x]\t\tchunk2: %x\tillegal instruction\n", *registers->program_counter-2, chunk2);
                break;
            }
            break;

            case 0x01: /*decrement*/
            wprintw(panel->debug_window, "[%2x]\tchunk1: %x\tdecrement\n", *registers->program_counter-2, chunk1);
            switch(chunk2) {
                case 0x00: /*register A*/
                wprintw(panel->debug_window, "[%2x]\t\tchunk2: %x\tregister A\n", *registers->program_counter-2, chunk2);
                registers->result[0] = ripple_carry_subtract(registers->reg_A, (uint8_t)0x01, registers);
                break;

                case 0x01: /*register B*/
                wprintw(panel->debug_window, "[%2x]\t\tchunk2: %x\tregister B\n", *registers->program_counter-2, chunk2);
                registers->result[0] = ripple_carry_subtract(registers->reg_B, (uint8_t)0x01, registers);
                break;

                case 0x02: /*register C*/
                wprintw(panel->debug_window, "[%2x]\t\tchunk2: %x\tregister C\n", *registers->program_counter-2, chunk2);
                registers->result[0] = ripple_carry_subtract(registers->reg_C, (uint8_t)0x01, registers);
                break;

                default:
                wprintw(panel->debug_window, "[%2x]\t\tchunk2: %x\tillegal instruction\n", *registers->program_counter-2, chunk2);
                break;
            }
            break;

            default:
            wprintw(panel->debug_window, "[%2x]\tchunk1: %x\tillegal instruction\n", *registers->program_counter-2, chunk1);
            break;
        }
        break;

        case 0x05: /*add*/
        wprintw(panel->debug_window, "[%2x]chunk0: %x\tadd\n", *registers->program_counter-2, chunk0);
        switch(chunk1) {
            case 0x00: /*register A - immediate mode*/
            wprintw(panel->debug_window, "[%2x]\tchunk1: %x\tregister A - immediate mode\n", *registers->program_counter-2, chunk1);
            wprintw(panel->debug_window, "[%2x]\tvalue: %x\n", *registers->program_counter-2, ((chunk2 << 4) + chunk3));
            registers->result[0] = ripple_carry_add(registers->reg_A, ((chunk2 << 4) + chunk3), registers);
            break;

            case 0x01: /*register B - immediate mode*/
            wprintw(panel->debug_window, "[%2x]\tchunk1: %x\tregister B - immediate mode\n", *registers->program_counter-2, chunk1);
            wprintw(panel->debug_window, "[%2x]\tvalue: %x\n", *registers->program_counter-2, ((chunk2 << 4) + chunk3));
            registers->result[0] = ripple_carry_add(registers->reg_B, ((chunk2 << 4) + chunk3), registers);
            break;

            case 0x02: /*register C - immediate mode*/
            wprintw(panel->debug_window, "[%2x]\tchunk1: %x\tregister C - immediate mode\n", *registers->program_counter-2, chunk1);
            wprintw(panel->debug_window, "[%2x]\tvalue: %x\n", *registers->program_counter-2, ((chunk2 << 4) + chunk3));
            registers->result[0] = ripple_carry_add(registers->reg_C, ((chunk2 << 4) + chunk3), registers);
            break;

            case 0x03: /*register A - addressing mode*/
            wprintw(panel->debug_window, "[%2x]\tchunk1: %x\tregister A - addressing mode\n", *registers->program_counter-2, chunk1);
            wprintw(panel->debug_window, "[%2x]\taddress: %x\n", *registers->program_counter-2, ((chunk2 << 4) + chunk3));
            registers->result[0] = ripple_carry_add(registers->reg_A, ram[((chunk2 << 4) + chunk3)], registers);
            break;

            case 0x04: /*register B - addressing mode*/
            wprintw(panel->debug_window, "[%2x]\tchunk1: %x\tregister B - addressing mode\n", *registers->program_counter-2, chunk1);
            wprintw(panel->debug_window, "[%2x]\taddress: %x\n", *registers->program_counter-2, ((chunk2 << 4) + chunk3));
            registers->result[0] = ripple_carry_add(registers->reg_B, ram[((chunk2 << 4) + chunk3)], registers);
            break;

            case 0x05: /*register C - addressing mode*/
            wprintw(panel->debug_window, "[%2x]\tchunk1: %x\tregister C - addressing mode\n", *registers->program_counter-2, chunk1);
            wprintw(panel->debug_window, "[%2x]\taddress: %x\n", *registers->program_counter-2, ((chunk2 << 4) + chunk3));
            registers->result[0] = ripple_carry_add(registers->reg_C, ram[((chunk2 << 4) + chunk3)], registers);
            break;

            default:
            wprintw(panel->debug_window, "[%2x]\tchunk1: %x\tillegal instruction\n", *registers->program_counter-2, chunk1);
            break;
        }
        break;

        case 0x06: /*subtract*/
        wprintw(panel->debug_window, "[%2x]chunk0: %x\tsubtract\n", *registers->program_counter-2, chunk0);
        switch(chunk1) {
            case 0x00: /*register A - immediate mode*/
            wprintw(panel->debug_window, "[%2x]\tchunk1: %x\tregister A - immediate mode\n", *registers->program_counter-2, chunk1);
            wprintw(panel->debug_window, "[%2x]\tvalue: %x\n", *registers->program_counter-2, ((chunk2 << 4) + chunk3));
            registers->result[0] = ripple_carry_subtract(registers->reg_A, ((chunk2 << 4) + chunk3), registers);
            break;

            case 0x01: /*register B - immediate mode*/
            wprintw(panel->debug_window, "[%2x]\tchunk1: %x\tregister B - immediate mode\n", *registers->program_counter-2, chunk1);
            wprintw(panel->debug_window, "[%2x]\tvalue: %x\n", *registers->program_counter-2, ((chunk2 << 4) + chunk3));
            registers->result[0] = ripple_carry_subtract(registers->reg_B, ((chunk2 << 4) + chunk3), registers);
            break;

            case 0x02: /*register C - immediate mode*/
            wprintw(panel->debug_window, "[%2x]\tchunk1: %x\tregister C - immediate mode\n", *registers->program_counter-2, chunk1);
            wprintw(panel->debug_window, "[%2x]\tvalue: %x\n", *registers->program_counter-2, ((chunk2 << 4) + chunk3));
            registers->result[0] = ripple_carry_subtract(registers->reg_C, ((chunk2 << 4) + chunk3), registers);
            break;

            case 0x03: /*register A - addressing mode*/
            wprintw(panel->debug_window, "[%2x]\tchunk1: %x\tregister A - addressing mode\n", *registers->program_counter-2, chunk1);
            wprintw(panel->debug_window, "[%2x]\taddress: %x\n", *registers->program_counter-2, ((chunk2 << 4) + chunk3));
            registers->result[0] = ripple_carry_subtract(registers->reg_A, ram[((chunk2 << 4) + chunk3)], registers);
            break;

            case 0x04: /*register B - addressing mode*/
            wprintw(panel->debug_window, "[%2x]\tchunk1: %x\tregister B - addressing mode\n", *registers->program_counter-2, chunk1);
            wprintw(panel->debug_window, "[%2x]\taddress: %x\n", *registers->program_counter-2, ((chunk2 << 4) + chunk3));
            registers->result[0] = ripple_carry_subtract(registers->reg_B, ram[((chunk2 << 4) + chunk3)], registers);
            break;

            case 0x05: /*register C - addressing mode*/
            wprintw(panel->debug_window, "[%2x]\tchunk1: %x\tregister C - addressing mode\n", *registers->program_counter-2, chunk1);
            wprintw(panel->debug_window, "[%2x]\taddress: %x\n", *registers->program_counter-2, ((chunk2 << 4) + chunk3));
            registers->result[0] = ripple_carry_subtract(registers->reg_C, ram[((chunk2 << 4) + chunk3)], registers);
            break;

            default:
            wprintw(panel->debug_window, "[%2x]\tchunk1: %x\tillegal instruction\n", *registers->program_counter-2, chunk1);
            break;
        }
        break;

        case 0x07: /*multiply*/
        wprintw(panel->debug_window, "[%2x]chunk0: %x\tmultiply\n", *registers->program_counter-2, chunk0);
        switch(chunk1) {
            case 0x00: /*register A - immediate mode*/
            wprintw(panel->debug_window, "[%2x]\tchunk1: %x\tregister A - immediate mode\n", *registers->program_counter-2, chunk1);
            wprintw(panel->debug_window, "[%2x]\tvalue: %x\n", *registers->program_counter-2, ((chunk2 << 4) + chunk3));

            /*loop here*/
            ripple_carry_add(registers->reg_A, ((chunk2 << 4) + chunk3), registers);
            /*loop here*/

            break;

            case 0x01: /*register B - immediate mode*/
            break;

            case 0x02: /*register C - immediate mode*/
            break;

            case 0x03: /*register A - addressing mode*/
            break;

            case 0x04: /*register B - addressing mode*/
            break;

            case 0x05: /*register C - addressing mode*/
            break;

            default:
            wprintw(panel->debug_window, "[%2x]\tchunk1: %x\tillegal instruction\n", *registers->program_counter-2, chunk1);
            break;
        }
            
        break;

        case 0x08: /*divide*/
            
        break;

        case 0x09: /*compare registers*/
        wprintw(panel->debug_window, "[%2x]chunk0: %x\tcompare registers\n", *registers->program_counter-2, chunk0);
        switch(chunk1) {
            case 0x00: /*greater than or equal to*/
            wprintw(panel->debug_window, "[%2x]\tchunk1: %x\tgreater than or equal to\n", *registers->program_counter-2, chunk1);
            switch(chunk2) {
                case 0x00: /*register A*/
                wprintw(panel->debug_window, "[%2x]\t\tchunk2: %x\tregister A with register %c\n", *registers->program_counter-2, chunk2, chunk3+65);
                switch(chunk3) {
                    case 0x00: /*register A*/
                    compare_greater_than(registers->reg_A, registers->reg_A, registers);
                    break;

                    case 0x01: /*register B*/
                    compare_greater_than(registers->reg_A, registers->reg_B, registers);
                    break;

                    case 0x02: /*register C*/
                    compare_greater_than(registers->reg_A, registers->reg_C, registers);
                    break;

                    default:
                    wprintw(panel->debug_window, "[%2x]\t\t\tchunk3: %x\tillegal instruction\n", *registers->program_counter-2, chunk3);
                    break;
                }
                break;

                case 0x01: /*register B*/
                wprintw(panel->debug_window, "[%2x]\t\tchunk2: %x\tregister B with register %c\n", *registers->program_counter-2, chunk2, chunk3+65);
                switch(chunk3) {
                    case 0x00: /*register A*/
                    compare_greater_than(registers->reg_B, registers->reg_A, registers);
                    break;

                    case 0x01: /*register B*/
                    compare_greater_than(registers->reg_B, registers->reg_B, registers);
                    break;

                    case 0x02: /*register C*/
                    compare_greater_than(registers->reg_B, registers->reg_C, registers);
                    break;

                    default:
                    wprintw(panel->debug_window, "[%2x]\t\t\tchunk3: %x\tillegal instruction\n", *registers->program_counter-2, chunk3);
                    break;
                }
                break;

                case 0x02: /*register C*/
                wprintw(panel->debug_window, "[%2x]\t\tchunk2: %x\tregister C with register %c\n", *registers->program_counter-2, chunk2, chunk3+65);
                switch(chunk3) {
                    case 0x00: /*register A*/
                    compare_greater_than(registers->reg_C, registers->reg_A, registers);
                    break;

                    case 0x01: /*register B*/
                    compare_greater_than(registers->reg_C, registers->reg_B, registers);
                    break;

                    case 0x02: /*register C*/
                    compare_greater_than(registers->reg_C, registers->reg_C, registers);
                    break;

                    default:
                    wprintw(panel->debug_window, "[%2x]\t\t\tchunk3: %x\tillegal instruction\n", *registers->program_counter-2, chunk3);
                    break;
                }
                break;

                default:
                wprintw(panel->debug_window, "[%2x]\t\tchunk2: %x\tillegal instruction\n", *registers->program_counter-2, chunk2);
                break;
            }
            break;

            case 0x01: /*less than*/
            wprintw(panel->debug_window, "[%2x]\tchunk1: %x\tless than\n", *registers->program_counter-2, chunk1);
            switch(chunk2) {
                case 0x00: /*register A*/
                wprintw(panel->debug_window, "[%2x]\t\tchunk2: %x\tregister A with register %c\n", *registers->program_counter-2, chunk2, chunk3+65);
                switch(chunk3) {
                    case 0x00: /*register A*/
                    compare_less_than(registers->reg_A, registers->reg_A, registers);
                    break;

                    case 0x01: /*register B*/
                    compare_less_than(registers->reg_A, registers->reg_B, registers);
                    break;

                    case 0x02: /*register C*/
                    compare_less_than(registers->reg_A, registers->reg_C, registers);
                    break;

                    default:
                    wprintw(panel->debug_window, "[%2x]\t\t\tchunk3: %x\tillegal instruction\n", *registers->program_counter-2, chunk3);
                    break;
                }
                break;

                case 0x01: /*register B*/
                wprintw(panel->debug_window, "[%2x]\t\tchunk2: %x\tregister B with register %c\n", *registers->program_counter-2, chunk2, chunk3+65);
                switch(chunk3) {
                    case 0x00: /*register A*/
                    compare_less_than(registers->reg_B, registers->reg_A, registers);
                    break;

                    case 0x01: /*register B*/
                    compare_less_than(registers->reg_B, registers->reg_B, registers);
                    break;

                    case 0x02: /*register C*/
                    compare_less_than(registers->reg_B, registers->reg_C, registers);
                    break;

                    default:
                    wprintw(panel->debug_window, "[%2x]\t\t\tchunk3: %x\tillegal instruction\n", *registers->program_counter-2, chunk3);
                    break;
                }
                break;

                case 0x02: /*register C*/
                wprintw(panel->debug_window, "[%2x]\t\tchunk2: %x\tregister C with register %c\n", *registers->program_counter-2, chunk2, chunk3+65);
                switch(chunk3) {
                    case 0x00: /*register A*/
                    compare_less_than(registers->reg_C, registers->reg_A, registers);
                    break;

                    case 0x01: /*register B*/
                    compare_less_than(registers->reg_C, registers->reg_B, registers);
                    break;

                    case 0x02: /*register C*/
                    compare_less_than(registers->reg_C, registers->reg_C, registers);
                    break;

                    default:
                    wprintw(panel->debug_window, "[%2x]\t\t\tchunk3: %x\tillegal instruction\n", *registers->program_counter-2, chunk3);
                    break;
                }
                break;

                default:
                wprintw(panel->debug_window, "[%2x]\t\tchunk2: %x\tillegal instruction\n", *registers->program_counter-2, chunk2);
                break;
            }
            break;

            case 0x02: /*equal to*/
            wprintw(panel->debug_window, "[%2x]\tchunk1: %x\tequal to\n", *registers->program_counter-2, chunk1);
            switch(chunk2) {
                case 0x00: /*register A*/
                wprintw(panel->debug_window, "[%2x]\t\tchunk2: %x\tregister A with register %c\n", *registers->program_counter-2, chunk2, chunk3+65);
                switch(chunk3) {
                    case 0x00: /*register A*/
                    compare_equal_to(registers->reg_A, registers->reg_A, registers);
                    break;

                    case 0x01: /*register B*/
                    compare_equal_to(registers->reg_A, registers->reg_B, registers);
                    break;

                    case 0x02: /*register C*/
                    compare_equal_to(registers->reg_A, registers->reg_C, registers);
                    break;

                    default:
                    wprintw(panel->debug_window, "[%2x]\t\t\tchunk3: %x\tillegal instruction\n", *registers->program_counter-2, chunk3);
                    break;
                }
                break;

                case 0x01: /*register B*/
                wprintw(panel->debug_window, "[%2x]\t\tchunk2: %x\tregister B with register %c\n", *registers->program_counter-2, chunk2, chunk3+65);
                switch(chunk3) {
                    case 0x00: /*register A*/
                    compare_equal_to(registers->reg_B, registers->reg_A, registers);
                    break;

                    case 0x01: /*register B*/
                    compare_equal_to(registers->reg_B, registers->reg_B, registers);
                    break;

                    case 0x02: /*register C*/
                    compare_equal_to(registers->reg_B, registers->reg_C, registers);
                    break;

                    default:
                    wprintw(panel->debug_window, "[%2x]\t\t\tchunk3: %x\tillegal instruction\n", *registers->program_counter-2, chunk3);
                    break;
                }
                break;

                case 0x02: /*register C*/
                wprintw(panel->debug_window, "[%2x]\t\tchunk2: %x\tregister C with register %c\n", *registers->program_counter-2, chunk2, chunk3+65);
                switch(chunk3) {
                    case 0x00: /*register A*/
                    compare_equal_to(registers->reg_C, registers->reg_A, registers);
                    break;

                    case 0x01: /*register B*/
                    compare_equal_to(registers->reg_C, registers->reg_B, registers);
                    break;

                    case 0x02: /*register C*/
                    compare_equal_to(registers->reg_C, registers->reg_C, registers);
                    break;

                    default:
                    wprintw(panel->debug_window, "[%2x]\t\t\tchunk3: %x\tillegal instruction\n", *registers->program_counter-2, chunk3);
                    break;
                }
                break;

                default:
                wprintw(panel->debug_window, "[%2x]\t\tchunk2: %x\tillegal instruction\n", *registers->program_counter-2, chunk2);
                break;
            }
            break;

            default:
            wprintw(panel->debug_window, "[%2x]\tchunk1: %x\tillegal instruction\n", *registers->program_counter-2, chunk1);
            break;
        }
        break;

        case 0x0a: /*branch*/
        wprintw(panel->debug_window, "[%2x]chunk0: %x\tbranch\n", *registers->program_counter-2, chunk0);
        wprintw(panel->debug_window, "[%2x]\tchunk1: %x\tto address: %x\n", *registers->program_counter-2, chunk1, ((chunk2 << 4) + chunk3));
        /*check status register for compare flag*/
        branch(((chunk2 << 4) + chunk3), registers);
        break;

        case 0x0b: /*blank*/
        
        break;

        case 0x0c: /*blank*/

        break;

        case 0x0d: /*blank*/

        break;

        case 0x0e: /*blank*/

        break;

        case 0x0f: /*blank*/
        
        break;

        default:
        wprintw(panel->debug_window, "[%2x]chunk0: %x\tillegal instruction\n", *registers->program_counter-2, chunk0);
        break;
    }
    return;
    draw_borders(panel->instruction_window);
}

int read_bin(char* filename, uint8_t* rom, int memorysize) {
    memset(rom, 0, memorysize); /*clear memory*/

    FILE* fp = fopen(filename, "rb"); 
    if(fp==NULL) return 1;
    
    fread(rom, 4, memorysize/4, fp);
    fclose(fp);
    
    return 0;
}

void update_status_windows(PANEL* panel, REGISTERS* registers) {
    mvwprintw(panel->register_window, 1, 1, "register A      - [0x]%02x", *registers->reg_A);
    mvwprintw(panel->register_window, 2, 1, "register B      - [0x]%02x", *registers->reg_B);
    mvwprintw(panel->register_window, 3, 1, "register C      - [0x]%02x", *registers->reg_C);
    mvwprintw(panel->register_window, 4, 1, "result register - [0x]%02x", *registers->result);
    mvwprintw(panel->register_window, 5, 1, "program counter - [0x]%02x", *registers->program_counter);
    mvwprintw(panel->register_window, 6, 1, "status register - [0b]%08d", int_to_int(*registers->status_register));
    mvwprintw(panel->register_window, 7, 1, "stack pointer   - [0x]%02x", *registers->stack_pointer);
    
    print_window_titles(panel);
    wrefresh(panel->instruction_window);
    wrefresh(panel->register_window);
    wrefresh(panel->debug_window);
    wrefresh(panel->info_window);
}

void quit(REGISTERS* registers, uint8_t* ram, uint8_t* rom) {
    free(rom);
    free(ram);
    free(registers->reg_A);
    free(registers->reg_B);
    free(registers->reg_C);
    free(registers->program_counter);
    free(registers->status_register);
    free(registers->stack_pointer);
    free(registers->result);
    
    endwin();
}

int main(int argc, char* argv[]) {
    //SETUP--------------------------------------
    ////CHECK PARAMETERS-------------------------
    if(argc!=2) {printf("usage: -<program file>\n"); return -1;}

    ////NCURSES----------------------------------
    PANEL panel;
    initscr();
    switch(setup(&panel)) {
		case 1:
		endwin();
		printf("setup error: no color support\n");
		return -1;;
		break;

		case 2:
		endwin();
		printf("setup error: could not initialise window\n");
		return -1;
		break;

		default:
		break;
	}
    refresh();

    //MAIN PROGRAM-------------------------------
    /*define ROM*/
    int romsize=256; /*program size in bytes*/
    uint8_t* rom = malloc(romsize);

    /*define RAM*/
    int ramsize=256;
    uint8_t* ram = malloc(ramsize);

    /*clear RAM*/
    memset(ram, 0, ramsize);

    /*define stack*/
    int stacksize = 32;
    uint8_t* stack = &ram[224]; //malloc(stacksize);

    /*clear stack*/
    memset(stack, 0, stacksize);

    /*registers*/
    int registersize=1; /*register size in bytes*/
    REGISTERS registers;

    registers.reg_A=malloc(registersize);
    registers.reg_B=malloc(registersize);
    registers.reg_C=malloc(registersize);
    
    registers.program_counter=malloc(registersize);
    registers.status_register=malloc(registersize);
    registers.stack_pointer=malloc(registersize);
    registers.result=malloc(registersize);

    /*clear all registers*/
    memset(registers.reg_A, 0, registersize);
    memset(registers.reg_B, 0, registersize);
    memset(registers.reg_C, 0, registersize);

    memset(registers.program_counter, 0, registersize);
    memset(registers.status_register, 0, registersize);
    memset(registers.stack_pointer, 0, registersize);
    memset(registers.result, 0, registersize);

    /*read binary file into ROM*/
    if(read_bin(argv[1], rom, romsize)) {
        quit(&registers, ram, rom);
        printf("couldn't open %s\n", argv[1]);
        return -1;
    }

    /*main execution loop*/
    uint8_t next_byte = 0xff;
    uint8_t ins_parameters = 0xff;
    while(next_byte!=0xea) { /*0xea = halt*/
        /*NCURSES WINDOWS*/
        update_status_windows(&panel, &registers);
        int w = getch(); /*NUCURSES INPUT HANDLING TO STEP THROUGH PROGRAM*/
        if(w=='q') break;

        /*read instruction from program*/
        next_byte = rom[*registers.program_counter/sizeof(uint8_t)];
        
        /*increment program_counter*/
        registers.program_counter[0]++;

        /*read parameters from program*/
        ins_parameters = rom[*registers.program_counter/sizeof(uint8_t)];

        /*increment program_counter*/
        registers.program_counter[0]++;

        /*execute instruction*/
        uint16_t word = (next_byte << 8) + ins_parameters;
        exec_ins(word, &registers, ram, stack, &panel);
    }

    refresh();

    quit(&registers, ram, rom);    
    return 0;
}

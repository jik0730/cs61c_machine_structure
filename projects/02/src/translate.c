#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "tables.h"
#include "translate_utils.h"
#include "translate.h"

/* SOLUTION CODE BELOW */
const int TWO_POW_SEVENTEEN = 131072;    // 2^17
int write_divmult(uint8_t funct, FILE* output, char** args, size_t num_args);
int write_mf(uint8_t funct, FILE* output, char** args, size_t num_args);

/* Writes instructions during the assembler's first pass to OUTPUT. The case
   for general instructions has already been completed, but you need to write
   code to translate the li and other pseudoinstructions. Your pseudoinstruction 
   expansions should not have any side effects.

   NAME is the name of the instruction, ARGS is an array of the arguments, and
   NUM_ARGS specifies the number of items in ARGS.

   Error checking for regular instructions are done in pass two. However, for
   pseudoinstructions, you must make sure that ARGS contains the correct number
   of arguments. You do NOT need to check whether the registers / label are 
   valid, since that will be checked in part two.

   Also for li:
    - make sure that the number is representable by 32 bits. (Hint: the number 
        can be both signed or unsigned).
    - if the immediate can fit in the imm field of an addiu instruction, then
        expand li into a single addiu instruction. Otherwise, expand it into 
        a lui-ori pair.

   If you are going to use the $zero or $0, use $0, not $zero.

   MARS has slightly different translation rules for li, and it allows numbers
   larger than the largest 32 bit number to be loaded with li. You should follow
   the above rules if MARS behaves differently.

   Use fprintf() to write. If writing multiple instructions, make sure that 
   each instruction is on a different line.

   Returns the number of instructions written (so 0 if there were any errors).
 */
unsigned write_pass_one(FILE* output, const char* name, char** args, int num_args) {
    if (strcmp(name, "li") == 0) {
        if (num_args != 2 || !name || !output || !args || !(*args)) {
            return 0;
        }
        // Immediate range check.
        long int imm;
        int err = translate_num(&imm, args[1], INT32_MIN, UINT32_MAX);
        if (err == -1) {
            return 0;
        }
        // If we can just use addiu.
        if (imm <= INT16_MAX && imm >= INT16_MIN) {
            char hex[10];
            sprintf(hex, "%d", (int16_t) imm);
            char *t_args[3] = {args[0], "$0", hex};
            write_inst_string(output, "addiu", t_args, 3);
            return 1;
        } else {
            // Divide into upper and lower
            uint16_t upper = imm >> 16;
            uint16_t lower = 0xffff & imm;
            char hexu[10];
            sprintf(hexu, "%d", upper);
            char hexl[10];
            sprintf(hexl, "%d", lower);
            char *tu_args[2] = {"$at", hexu};
            char *tl_args[3] = {args[0], "$at", hexl};
            write_inst_string(output, "lui", tu_args, 2);
            write_inst_string(output, "ori", tl_args, 3);
            return 2;
        }
    } else if (strcmp(name, "move") == 0) {
        if (num_args != 2 || !name || !output || !args || !(*args)) {
            return 0;
        }
        // Write TAL instruction.
        char* zero = "$0";
        char* t_args[3] = {*args, *(args+1), zero};
        write_inst_string(output, "addu", t_args, 3);
        return 1;
    } else if (strcmp(name, "rem") == 0) {
        if (num_args != 3 || !name || !output || !args || !(*args)) {
            return 0;
        }
        // Write TAL instruction.
        char *t_args[2] = {*(args+1), *(args+2)};
        write_inst_string(output, "div", t_args, 2); // div.
        write_inst_string(output, "mfhi", &*args, 1); // mfhi.
        return 2;  
    } else if (strcmp(name, "bge") == 0) {
        if (num_args != 3 || !name || !output || !args || !(*args)) {
            return 0;
        }
        // Write TAL instruction.
        char *t_args1[3] = {"$at", *args, *(args+1)};
        char *t_args2[3] = {"$at", "$0", *(args+2)};
        write_inst_string(output, "slt", t_args1, 3);
        write_inst_string(output, "beq", t_args2, 3);
        return 2;
    } else if (strcmp(name, "bnez") == 0) {
        if (num_args != 2 || !name || !output || !args || !(*args)) {
            return 0;
        }
        // Write TAL instruciton.
        char *t_args[3] = {*args, "$0", *(args+1)};
        write_inst_string(output, "bne", t_args, 3);
        return 1;
    }
    write_inst_string(output, name, args, num_args);
    return 1;

}

/* Writes the instruction in hexadecimal format to OUTPUT during pass #2.
   
   NAME is the name of the instruction, ARGS is an array of the arguments, and
   NUM_ARGS specifies the number of items in ARGS. 

   The symbol table (SYMTBL) is given for any symbols that need to be resolved
   at this step. If a symbol should be relocated, it should be added to the
   relocation table (RELTBL), and the fields for that symbol should be set to
   all zeros. 

   You must perform error checking on all instructions and make sure that their
   arguments are valid. If an instruction is invalid, you should not write 
   anything to OUTPUT but simply return -1. MARS may be a useful resource for
   this step.

   Some function declarations for the write_*() functions are provided in translate.h, and you MUST implement these
   and use these as function headers. You may use helper functions, but you still must implement
   the provided write_* functions declared in translate.h.

   Returns 0 on success and -1 on error. 
 */
int translate_inst(FILE* output, const char* name, char** args, size_t num_args, uint32_t addr,
    SymbolTable* symtbl, SymbolTable* reltbl) {
    if (strcmp(name, "addu") == 0)       return write_rtype (0x21, output, args, num_args);
    else if (strcmp(name, "or") == 0)    return write_rtype (0x25, output, args, num_args);
    else if (strcmp(name, "slt") == 0)   return write_rtype (0x2a, output, args, num_args);
    else if (strcmp(name, "sltu") == 0)  return write_rtype (0x2b, output, args, num_args);
    else if (strcmp(name, "sll") == 0)   return write_shift (0x00, output, args, num_args);
    else if (strcmp(name, "addiu") == 0) return write_addiu (0x09, output, args, num_args);
    else if (strcmp(name, "ori") == 0)   return write_ori (0x0d, output, args, num_args);
    else if (strcmp(name, "lui") == 0)   return write_lui (0x0f, output, args, num_args);
    else if (strcmp(name, "lb") == 0)    return write_mem (0x20, output, args, num_args);
    else if (strcmp(name, "lbu") == 0)   return write_mem (0x24, output, args, num_args);
    else if (strcmp(name, "lw") == 0)    return write_mem (0x23, output, args, num_args);
    else if (strcmp(name, "sb") == 0)    return write_mem (0x28, output, args, num_args);
    else if (strcmp(name, "sw") == 0)    return write_mem (0x2b, output, args, num_args);
    else if (strcmp(name, "beq") == 0)   return write_branch (0x04, output, args, num_args, addr, symtbl);
    else if (strcmp(name, "bne") == 0)   return write_branch (0x05, output, args, num_args, addr, symtbl);
    else if (strcmp(name, "j") == 0)     return write_jump (0x02, output, args, num_args, addr, reltbl);
    else if (strcmp(name, "jal") == 0)   return write_jump (0x03, output, args, num_args, addr, reltbl);
    else if (strcmp(name, "mult") == 0)  return write_divmult (0x18, output, args, num_args);
    else if (strcmp(name, "div") == 0)   return write_divmult (0x1a, output, args, num_args);
    else if (strcmp(name, "mfhi") == 0)  return write_mf (0x10, output, args, num_args);
    else if (strcmp(name, "mflo") == 0)  return write_mf (0x12, output, args, num_args);
    else if (strcmp(name, "jr") == 0)    return write_jr (0x08, output, args, num_args);

    else                                 return -1;
}

/* A helper function for writing most R-type instructions. You should use
   translate_reg() to parse registers and write_inst_hex() to write to 
   OUTPUT. Both are defined in translate_utils.h.

   This function is INCOMPLETE. Complete the implementation below. You will
   find bitwise operations to be the cleanest way to complete this function.
 */
int write_rtype(uint8_t funct, FILE* output, char** args, size_t num_args) {
    if (num_args != 3 || !output || !args || !(*args)) {
        return -1;
    }

    int rd = translate_reg(args[0]);
    int rs = translate_reg(args[1]);
    int rt = translate_reg(args[2]);
    if (rd == -1 || rt == -1 || rs == -1) {
        return -1;
    }
    // Shift rd, rs, rt.
    rd = rd << 11;
    rs = rs << 21;
    rt = rt << 16;

    uint32_t instruction = 0;
    instruction = instruction | rd | rs | rt | (uint32_t) funct;
    write_inst_hex(output, instruction);
    return 0;
}

int write_divmult(uint8_t funct, FILE* output, char** args, size_t num_args) {
    if (num_args != 2 || !output || !args || !(*args)) {
        return -1;
    }

    int rs = translate_reg(args[0]);
    int rt = translate_reg(args[1]);
    if (rt == -1 || rs == -1) {
        return -1;
    }
    // Shift rs, rt.
    rs = rs << 21;
    rt = rt << 16;

    uint32_t instruction = 0;
    instruction = instruction | rs | rt | (uint32_t) funct;
    write_inst_hex(output, instruction);
    return 0;
}

int write_mf(uint8_t funct, FILE* output, char** args, size_t num_args) {
    if (num_args != 2 || !output || !args || !(*args)) {
        return -1;
    }

    int rd = translate_reg(args[0]);
    if (rd == -1) {
        return -1;
    }
    // Shift rs, rt.
    rd = rd << 11;

    uint32_t instruction = 0;
    instruction = instruction | rd | (uint32_t) funct;
    write_inst_hex(output, instruction);
    return 0;
}

/* A helper function for writing shift instructions. You should use 
   translate_num() to parse numerical arguments. translate_num() is defined
   in translate_utils.h.

   This function is INCOMPLETE. Complete the implementation below. You will
   find bitwise operations to be the cleanest way to complete this function.
 */
int write_shift(uint8_t funct, FILE* output, char** args, size_t num_args) {
	if (num_args != 3 || !output || !args || !(*args)) {
        return -1;
    }

    long int shamt;
    int rd = translate_reg(args[0]);
    int rt = translate_reg(args[1]);
    int err = translate_num(&shamt, args[2], 0, 31);
    if (err == -1 || rt == -1 || rd == -1) {
        return -1;
    }
    // Shift registers.
    rd = rd << 11;
    rt = rt << 16;
    shamt = shamt << 6;

    uint32_t instruction = 0;
    instruction = instruction | rd | (uint32_t) shamt | rt | (uint32_t) funct;
    write_inst_hex(output, instruction);
    return 0;
}

/* The rest of your write_*() functions below */

int write_jr(uint8_t funct, FILE* output, char** args, size_t num_args) {
    if (num_args != 1 || !output || !args || !(*args)) {
        return -1;
    }

    int rs = translate_reg(args[0]);
    if (rs == -1) {
        return -1;
    }
    // Shift.
    rs = rs << 21;

    uint32_t instruction = 0;
    instruction = instruction | rs | (uint32_t) funct;
    write_inst_hex(output, instruction);
    return 0;
}

int write_addiu(uint8_t opcode, FILE* output, char** args, size_t num_args) {
    if (num_args != 3 || !output || !args || !(*args)) {
        return -1;
    }
    
    long int imm;
    int rt = translate_reg(args[0]);
    int rs = translate_reg(args[1]);
    int err = translate_num(&imm, args[2], INT16_MIN, INT16_MAX);
    uint32_t _opcode = opcode;
    if (err == -1 || rt == -1 || rs == -1) {
        return -1;
    }
    rt = rt << 16;
    rs = rs << 21;
    _opcode = _opcode << 26;

    uint32_t instruction = 0;
    instruction = instruction | rs | rt | (uint16_t) imm | _opcode;
    write_inst_hex(output, instruction);
    return 0;
}

int write_ori(uint8_t opcode, FILE* output, char** args, size_t num_args) {
    if (num_args != 3 || !output || !args || !(*args)) {
        printf("Wrong input for write_ori()\n");
        return -1;
    }
    
    long int imm;
    int rt = translate_reg(args[0]);
    int rs = translate_reg(args[1]);
    int err = translate_num(&imm, args[2], 0, UINT16_MAX);
    uint32_t _opcode = opcode;
    if (err == -1 || rt == -1 || rs == -1) {
        return -1;
    }
    rt = rt << 16;
    rs = rs << 21;
    _opcode = _opcode << 26;

    uint32_t instruction = 0;
    instruction = instruction | rs | rt | (uint16_t) imm | _opcode;
    write_inst_hex(output, instruction);
    return 0;
}

int write_lui(uint8_t opcode, FILE* output, char** args, size_t num_args) {
    if (num_args != 2 || !output || !args || !(*args)) {
        printf("Wrong input for write_lui()\n");
        return -1;
    }
    
    long int imm;
    int rt = translate_reg(args[0]);
    int err = translate_num(&imm, args[1], 0, UINT16_MAX);
    uint32_t _opcode = opcode;
    if (err == -1 || rt == -1) {
        return -1;
    }
    rt = rt << 16;
    _opcode = _opcode << 26;

    uint32_t instruction = 0;
    instruction = instruction | rt | (uint16_t) imm | _opcode;
    write_inst_hex(output, instruction);
    return 0;
}

int write_mem(uint8_t opcode, FILE* output, char** args, size_t num_args) {
    if (num_args != 3 || !output || !args || !(*args)) {
        printf("Wrong input for write_mem()\n");
        return -1;
    }
    
    long int imm;
    int rt = translate_reg(args[0]);
    int rs = translate_reg(args[2]);
    int err = translate_num(&imm, args[1], INT16_MIN, INT16_MAX);
    uint32_t _opcode = opcode;
    if (err == -1 || rt == -1 || rs == -1) {
        return -1;
    }
    rt = rt << 16;
    rs = rs << 21;
    _opcode = _opcode << 26;

    uint32_t instruction = 0;
    instruction = instruction | rs | rt | (uint16_t) imm | _opcode;
    write_inst_hex(output, instruction);
    return 0;
}

/*  A helper function to determine if a destination address
    can be branched to
*/
static int can_branch_to(uint32_t src_addr, uint32_t dest_addr) {
    int32_t diff = dest_addr - src_addr;
    return (diff >= 0 && diff <= TWO_POW_SEVENTEEN) || (diff < 0 && diff >= -(TWO_POW_SEVENTEEN - 4));
}


int write_branch(uint8_t opcode, FILE* output, char** args, size_t num_args, uint32_t addr, SymbolTable* symtbl) {
    if (num_args != 3 || !output || !args || !(*args)) {
        return -1;
    }
    
    int rs = translate_reg(args[0]);
    int rt = translate_reg(args[1]);
    int label_addr = get_addr_for_symbol(symtbl, args[2]);
    if (!can_branch_to((uint32_t) label_addr, addr) || label_addr == -1 || rt == -1 || rs == -1) {
        return -1;
    }
    uint32_t _opcode = opcode;
    rt = rt << 16;
    rs = rs << 21;
    _opcode = _opcode << 26;

    //Please compute the branch offset using the MIPS rules.
    uint16_t offset = (label_addr - addr - 4) / 4;
    uint32_t instruction = 0;
    instruction = instruction | rs | rt | offset | _opcode;
    write_inst_hex(output, instruction);        
    return 0;
}


int write_jump(uint8_t opcode, FILE* output, char** args, size_t num_args, uint32_t addr, SymbolTable* reltbl) {
    if (num_args != 1 || !output || !args || !(*args)) {
        return -1;
    }
    
    uint32_t _opcode = opcode;
    _opcode = _opcode << 26;

    // Create symbol and relocate.
    if (add_to_table(reltbl, *args, addr) == -1) {
        return -1;
    }

    uint32_t instruction = 0;
    instruction = instruction | _opcode;
    write_inst_hex(output, instruction);
    return 0;
}

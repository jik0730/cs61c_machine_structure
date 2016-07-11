
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "utils.h"
#include "tables.h"

const int SYMTBL_NON_UNIQUE = 0;
const int SYMTBL_UNIQUE_NAME = 1;

#define INITIAL_SIZE 5
#define SCALING_FACTOR 2

/*******************************
 * Helper Functions
 *******************************/

void allocation_failed() {
    write_to_log("Error: allocation failed\n");
    exit(1);
}

void addr_alignment_incorrect() {
    write_to_log("Error: address is not a multiple of 4.\n");
}

void name_already_exists(const char* name) {
    write_to_log("Error: name '%s' already exists in table.\n", name);
}

void write_symbol(FILE* output, uint32_t addr, const char* name) {
    fprintf(output, "%u\t%s\n", addr, name);
}

/*******************************
 * Symbol Table Functions
 *******************************/

/* Creates a new SymbolTable containg 0 elements and returns a pointer to that
   table. Multiple SymbolTables may exist at the same time. 
   If memory allocation fails, you should call allocation_failed(). 
   Mode will be either SYMTBL_NON_UNIQUE or SYMTBL_UNIQUE_NAME. You will need
   to store this value for use during add_to_table().
 */
SymbolTable* create_table(int mode) {
    // Create a symbol table.
    SymbolTable* ST = (SymbolTable *) malloc(sizeof(SymbolTable));
    if (!ST) {
        allocation_failed();
    }
    // Initialize.
    ST->tbl = NULL;
    ST->len = 0;
    ST->cap = 0;
    ST->mode = mode;
    // Return.
    return ST;
}

/* Frees the given SymbolTable and all associated memory. */
void free_table(SymbolTable* table) {
    // Free names in symbols.
    Symbol* temp = table->tbl;
    for (int i = 0; i < table->len; i++) {
        free(temp->name);
        temp++;
    }
    // Free symbols.
    free(table->tbl);
    // Free symbol table.
    free(table);
}

/* A suggested helper function for copying the contents of a string. */
static char* create_copy_of_str(const char* str) {
    size_t len = strlen(str) + 1;
    char *buf = (char *) malloc(len);
    if (!buf) {
       allocation_failed();
    }
    strncpy(buf, str, len); 
    return buf;
}

/* Adds a new symbol and its address to the SymbolTable pointed to by TABLE. 
   ADDR is given as the byte offset from the first instruction. The SymbolTable
   must be able to resize itself as more elements are added. 

   Note that NAME may point to a temporary array, so it is not safe to simply
   store the NAME pointer. You must store a copy of the given string.

   If ADDR is not word-aligned, you should call addr_alignment_incorrect() and
   return -1. If the table's mode is SYMTBL_UNIQUE_NAME and NAME already exists 
   in the table, you should call name_already_exists() and return -1. If memory
   allocation fails, you should call allocation_failed(). 

   Otherwise, you should store the symbol name and address and return 0.
 */
int add_to_table(SymbolTable* table, const char* name, uint32_t addr) {
    // Word-aligned test.
    if ((addr & 0x3) != 0) {
        addr_alignment_incorrect();
        return -1;
    }
    // Name test.
    int btest = 0;
    for (int i = 0; i < table->len; i++) {
        char* c = table->tbl->name;
        if (strcmp(c, name) == 0) {
            btest = 1;
            break;
        }
    }
    if (table->mode == SYMTBL_UNIQUE_NAME && btest) {
        name_already_exists(name);
        return -1;
    }
    // Allocate memory for symbol.
    Symbol* sym = (Symbol *) malloc(sizeof(Symbol));
    if (!sym) {
        allocation_failed();
    }
    // Make copy of name.
    char* cpname = create_copy_of_str(name);
    // Initialize a symbol.
    sym->name = cpname;
    sym->addr = addr;
    // Resize a symbol table.
    SymbolTable* temp = table;
    table = (SymbolTable *) malloc(sizeof(temp) + sizeof(sym));
    if (!table) {
        allocation_failed();
    }
    // Copy elements which is originally located in previous memory block.
    *(table->tbl) = *(temp->tbl);
    *(table->tbl + temp->len) = *sym;

    /****** What do 'len' and 'cap' exactly mean? ******/

    table->len = temp->len + 1;
    table->cap = temp->cap;
    table->mode = temp->mode;
    // Free temporary symbol.
    free(sym);
    return 0;
}

/* Returns the address (byte offset) of the given symbol. If a symbol with name
   NAME is not present in TABLE, return -1.
 */
int64_t get_addr_for_symbol(SymbolTable* table, const char* name) {
    for (int i = 0; i < table->len; i++) {
        Symbol* c = table->tbl + i;
        if (strcmp(c->name, name) == 0) {
            return c->addr; 
        }
    }
    return -1;   
}

/* Writes the SymbolTable TABLE to OUTPUT. You should use write_symbol() to
   perform the write. Do not print any additional whitespace or characters.
 */
void write_table(SymbolTable* table, FILE* output) {
    for (int i = 0; i < table->len; i++) {
        Symbol* c = table->tbl + i;
        write_symbol(output, c->addr, c->name);
    }
}

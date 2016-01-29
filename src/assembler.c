// #include <stdlib.h>
//
// /**
//  * Unfortunately, due to the design of this complier (or my incomptence) the current status of this is that it is very
//  * limited in functionality and probably will be replaced by the time I get to using it.
//  */
//
// typedef struct {
//     unsigned char* code;
//     int codeLength;
//     int codeAlloc;
// } assembler_state_t;
//
// typedef enum {
//     eax_reg,
//     ecx_reg,
//     edx_reg,
//     ebx_reg,
//     esp_reg,
//     ebp_reg,
//     esi_reg,
//     edi_reg
// } assembler_register_t;
//
// typedef enum {
//     reg_var,
//     disp32_var,
//     deref_reg_var,
//     none_var
// } assembler_variable_t;
//
// typedef enum {
//     add_op,
//     sub_op,
//     inc_op,
//     dec_op,
//     push_op,
//     pop_op,
//     mov_op
// } assembler_instruction_t;
//
// unsigned char opcodes[] = {
//     0x01,
//     0x29,
//     0x40,
//     0x48,
//     0x50,
//     0x58,
//     0x8B
// };
//
// typedef struct {
//     assembler_instruction_t instruction;
//
//     assembler_variable_t lhs_type;
//     union {
//         int lhs_disp32;
//         assembler_register_t lhs_reg;
//     };
//
//     assembler_variable_t rhs_type;
//     union {
//         int rhs_disp32;
//         assembler_register_t rhs_reg;
//     };
// } assembler_op_t;
//
// #define MALLOC_CHUNK 8
//
// assembler_state_t* assembler_init() {
//     assembler_state_t* asmb = (assembler_state_t*) malloc(sizeof(assembler_state_t));
//
//     asmb->code = (unsigned char*) malloc(MALLOC_CHUNK * sizeof(MALLOC_CHUNK));
//     asmb->codeAlloc = MALLOC_CHUNK;
//
//     return asmb;
// }
//
// void assembler_check(assembler_state_t* asmb, int alloc) {
//     if ( (asmb->codeLength + alloc) < asmb->codeAlloc ) {
//         return;
//     }
//
//     asmb->codeAlloc += (alloc > MALLOC_CHUNK) ? alloc : MALLOC_CHUNK;
//
//     unsigned char* temp = realloc(asmb->code, asmb->codeAlloc);
//
//     if ( temp == NULL ) {
//         free(asmb->code);
//         // TODO - PRINT NO MEM & QUIT
//         return;
//     }
//
//     asmb->code = temp;
// }
//
// void assembler_compile(assembler_state_t* asmb, assembler_op_t* op) {
//     switch ( op->lhs_type ) {
//         case none_var:
//             assembler_check(asmb, 1);
//             asmb->code[asmb->codeLength++] = opcodes[op->instruction];
//             break;
//
//         case reg_var:
//             switch ( op->rhs_type ) {
//                 case none_var:
//                     assembler_check(asmb, 1);
//                     asmb->code[asmb->codeLength++] = opcodes[op->instruction] | op->lhs_reg;
//                     break;
//                 case reg_var:
//                     assembler_check(asmb, 2);
//                     asmb->code[asmb->codeLength++] = (opcodes[op->instruction] | 0x3); // go from reg to r/m w/ 32 vals
//                     asmb->code[asmb->codeLength++] = 0xC0 | (op->lhs_reg << 3) | op->rhs_reg;
//                     break;
//
//                 case disp32_var:
//                     assembler_check(asmb, 6);
//                     asmb->code[asmb->codeLength++] = (opcodes[op->instruction] | 0x3); // go from r/m to reg
//                     asmb->code[asmb->codeLength++] = (op->lhs_reg << 3) | 0x5;
//
//                     int val = op->lhs_disp32;
//
//                     for ( int i = 0; i < 4; i++ ) {
//                         asmb->code[asmb->codeLength++] = val & 0xFF;
//                         val = val >> 8;
//                     }
//                     break;
//
//                 case deref_reg_var:
//                     assembler_check(asmb, 2);
//                     asmb->code[asmb->codeLength++] = (opcodes[op->instruction] | 0x3); // go from reg to r/m w/ 32 vals
//                     asmb->code[asmb->codeLength++] = (op->lhs_reg << 3) | op->rhs_reg;
//                     break;
//             }
//
//         case disp32_var:
//             break;
//
//         default:
//             break;
//     }
// }

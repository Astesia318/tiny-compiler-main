//riscv code used
#ifndef RISCV_H
#define RISCV_H
//add sub
#define R_TYPE(op_mnemonic, dest_reg_name, rs1_reg_name, rs2_reg_name) \
    input_str(obj_file, "\t%s %s, %s, %s\n", op_mnemonic, dest_reg_name, rs1_reg_name, rs2_reg_name)
//addi,subi
#define I_TYPE_ARITH(op_mnemonic, dest_reg_name, rs1_reg_name, immediate) \
    input_str(obj_file, "\t%s %s, %s, %d\n", op_mnemonic, dest_reg_name, rs1_reg_name, immediate)
//lw lbu
#define I_TYPE_LOAD(op_mnemonic, dest_reg_name, rs1_base_reg_name, offset_immediate) \
    input_str(obj_file, "\t%s %s, %d(%s)\n", op_mnemonic, dest_reg_name, offset_immediate, rs1_base_reg_name)
//sw sb
#define S_TYPE_STORE(op_mnemonic, rs2_src_reg_name, rs1_base_reg_name, offset_immediate) \
    input_str(obj_file, "\t%s %s, %d(%s)\n", op_mnemonic, rs2_src_reg_name, offset_immediate, rs1_base_reg_name)
//beq bne
#define B_TYPE_BRANCH(op_mnemonic, rs1_reg_name, rs2_reg_name, label_name) \
    input_str(obj_file, "\t%s %s, %s, %s\n", op_mnemonic, rs1_reg_name, rs2_reg_name, label_name)
//li
#define U_TYPE_UPPER_IMM(op_mnemonic, dest_reg_name, immediate) \
    input_str(obj_file, "\t%s %s, %d\n", op_mnemonic, dest_reg_name, immediate)
//la lla
#define U_TYPE_UPPER_SYM(op_mnemonic, dest_reg_name, symbol_no) \
    input_str(obj_file, "\t%s %s, .LC%d\n", op_mnemonic, dest_reg_name, symbol_no)

    //jr 
#define J_TYPE_JUMP_REG(op_mnemonic, dest_reg_name) \
    input_str(obj_file, "\t%s %s\n", op_mnemonic, dest_reg_name)
//call j
#define J_TYPE_JUMP_PSEUDO(op_mnemonic, label_name) \
    op_mnemonic=="call"?  \
    input_str(obj_file, "\t%s %s@plt\n", op_mnemonic, label_name) : \
    input_str(obj_file, "\t%s %s\n", op_mnemonic, label_name)
//mv seqz
#define PSEUDO_2_REG(op_mnemonic,dest_reg_name,src_reg_name) \
    input_str(obj_file, "\t%s %s, %s\n", op_mnemonic, dest_reg_name, src_reg_name)






#endif


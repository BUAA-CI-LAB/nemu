/***************************************************************************************
* Copyright (c) 2021-2022 Weitong Wang, University of Chinese Academy of Sciences
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include "../local-include/rtl.h"
#include "../local-include/intr.h"
#include "../local-include/csr.h"
#include <cpu/ifetch.h>
#include <cpu/cpu.h>
#include <cpu/decode.h>
#include <isa-all-instr.h>

static word_t zero_null = 0;

def_all_THelper();


static inline uint32_t get_instr(Decode *s) {
  return s->isa.instr.val;
}

// decode operand helper
#define def_DopHelper(name) \
  void concat(decode_op_, name) (Decode *s, Operand *op, uint32_t val, bool flag)

static inline def_DopHelper(i) {
  op->imm = val;
  print_Dop(op->str, OP_STR_SIZE, (flag ? "0x%x" : "%d"), op->imm);
}

static inline def_DopHelper(r) {
  bool load_val = flag;
  zero_null = 0;
  op->preg = (!load_val && val == 0) ? &zero_null : &reg_l(val);
  print_Dop(op->str, OP_STR_SIZE, "%s", reg_name(val, 4));
}

static inline def_DHelper(r2) {
  decode_op_r(s, id_src1, s->isa.instr.r2.rj, true);
  decode_op_r(s, id_dest, s->isa.instr.r2.rd, false);
}

static inline def_DHelper(r3) {
  decode_op_r(s, id_src1, s->isa.instr.r3.rj, true);
  decode_op_r(s, id_src2, s->isa.instr.r3.rk, true);
  decode_op_r(s, id_dest, s->isa.instr.r3.rd, false);
}

static inline def_DHelper(r2_i12) {
  decode_op_r(s, id_src1, s->isa.instr.r2_i12.rj, true);
  decode_op_i(s, id_src2, s->isa.instr.r2_i12.i12, false);
  decode_op_r(s, id_dest, s->isa.instr.r2_i12.rd, false);
}

static inline def_DHelper(r1_i20) {
  decode_op_i(s, id_src1, s->isa.instr.r1_i20.i20 << 12, true);
  decode_op_r(s, id_dest, s->isa.instr.r1_i20.rd, false);
}

static inline def_DHelper(r2_i8) {
  decode_op_r(s, id_src1, s->isa.instr.r2_i8.rj, true);
  decode_op_i(s, id_src2, s->isa.instr.r2_i8.i5, true);
  decode_op_r(s, id_dest, s->isa.instr.r2_i8.rd, false);
}

static inline def_DHelper(r2_i14s) {
  decode_op_i(s, id_src1, s->isa.instr.r2_i14s.i14, false);
  decode_op_r(s, id_src2, s->isa.instr.r2_i14s.rj, true);
  decode_op_r(s, id_dest, s->isa.instr.r2_i14s.rd, false);
}

static inline def_DHelper(r2_i16) {
  int32_t offset = (s->isa.instr.r2_i16.i16 << 2);
  decode_op_i(s, id_dest, s->pc + offset, true);
  decode_op_r(s, id_src1, s->isa.instr.r2_i16.rj, true);
  decode_op_r(s, id_src2, s->isa.instr.r2_i16.rd, true);
}

static inline def_DHelper(i_26) {
  int32_t offset = ((s->isa.instr.i_26.i26_25_16 << 16) | s->isa.instr.i_26.i26_15_0) << 2 ;
  decode_op_i(s, id_src1, s->pc + offset, true);
}

static inline def_DHelper(code_15) {
  decode_op_i(s, id_src1, s->isa.instr.code_15.code, true);
}

static inline def_DHelper(bl) {
  decode_i_26(s,width);
  decode_op_r(s, id_dest, (uint32_t)1, false);
  id_src2->imm = s->snpc;
}

static inline def_DHelper(pcaddu12i) {
  decode_r1_i20(s, width);
  id_src1->imm += s->pc;
}

static inline def_DHelper(jirl) {
  decode_op_r(s, id_src1, s->isa.instr.r2_i16.rj, true);
  decode_op_i(s, id_src2, s->isa.instr.r2_i16.i16 << 2, false);
  decode_op_r(s, id_dest, s->isa.instr.r2_i16.rd, false);
}

static inline def_DHelper(cacop) {
  decode_op_i(s, id_src1, s->isa.instr.r2_i12.rd, false);
  decode_op_i(s, id_src2, s->isa.instr.r2_i12.i12, false);
  decode_op_r(s, id_dest, s->isa.instr.r2_i12.rj, true);
}

static inline def_DHelper(invtlb) {
  decode_op_r(s, id_src1, s->isa.instr.r3.rk, true);
  decode_op_r(s, id_src2, s->isa.instr.r3.rj, true);
  decode_op_i(s, id_dest, s->isa.instr.r3.rd, false);
}


def_THelper(mem) {


  def_INSTR_TAB("???? 100010 ???????????? ????? ?????", ld_w);
  def_INSTR_TAB("???? 100110 ???????????? ????? ?????", st_w);
  def_INSTR_TAB("???? 100001 ???????????? ????? ?????", ld_h);
  def_INSTR_TAB("???? 101001 ???????????? ????? ?????", ld_hu);
  def_INSTR_TAB("???? 100101 ???????????? ????? ?????", st_h);
  def_INSTR_TAB("???? 100000 ???????????? ????? ?????", ld_b);
  def_INSTR_TAB("???? 101000 ???????????? ????? ?????", ld_bu);
  def_INSTR_TAB("???? 100100 ???????????? ????? ?????", st_b);
  def_INSTR_TAB("???? 101011 ???????????? ????? ?????", preld);  
  def_INSTR_IDTAB("???? 0000 ?????????????? ????? ?????",r2_i14s, ll_w);
  def_INSTR_IDTAB("???? 0001 ?????????????? ????? ?????",r2_i14s, sc_w);
  return table_inv(s);
};

def_THelper(branch) {
  def_INSTR_IDTAB("?? 0100 ???????????????? ??????????",i_26, b);
  def_INSTR_TAB("?? 1011 ???????????????? ????? ?????",bgeu);
  def_INSTR_TAB("?? 0111 ???????????????? ????? ?????",bne);
  def_INSTR_TAB("?? 1001 ???????????????? ????? ?????",bge);   
  def_INSTR_TAB("?? 1000 ???????????????? ????? ?????",blt);  
  def_INSTR_TAB("?? 0110 ???????????????? ????? ?????",beq);
  def_INSTR_TAB("?? 1010 ???????????????? ????? ?????",bltu);
  def_INSTR_IDTAB("?? 0101 ???????????????? ??????????",bl, bl); 
  def_INSTR_IDTAB("?? 0011 ???????????????? ????? ?????",jirl, jirl);
  return table_inv(s);
};

def_THelper(intop) {
  def_INSTR_TAB("???????????? 00000 ????? ????? ?????",add_w);
  def_INSTR_TAB("???????????? 00010 ????? ????? ?????",sub_w);
  def_INSTR_TAB("???????????? 01001 ????? ????? ?????",and);
  def_INSTR_TAB("???????????? 01010 ????? ????? ?????",or);
  def_INSTR_TAB("???????????? 01011 ????? ????? ?????",xor);  
  def_INSTR_TAB("???????????? 01000 ????? ????? ?????",nor);
  def_INSTR_TAB("???????????? 00100 ????? ????? ?????",slt);
  def_INSTR_TAB("???????????? 00101 ????? ????? ?????",sltu);
  def_INSTR_TAB("???????????? 01110 ????? ????? ?????",sll_w);   
  def_INSTR_TAB("???????????? 01111 ????? ????? ?????",srl_w);
  def_INSTR_TAB("???????????? 10000 ????? ????? ?????",sra_w);  
  def_INSTR_TAB("???????????? 11000 ????? ????? ?????",mul_w);
  def_INSTR_TAB("???????????? 11001 ????? ????? ?????",mulh_w);
  def_INSTR_TAB("???????????? 11010 ????? ????? ?????",mulh_wu);  
  return table_inv(s);
};

def_THelper(divop) {
  def_INSTR_TAB("????????????? 0000 ????? ????? ?????",div_w);
  def_INSTR_TAB("????????????? 0010 ????? ????? ?????",div_wu);   
  def_INSTR_TAB("????????????? 0001 ????? ????? ?????",mod_w);
  def_INSTR_TAB("????????????? 0011 ????? ????? ?????",mod_wu);  
  return table_inv(s);
};

def_THelper(int_i12) {
  def_INSTR_TAB("??????? 010 ???????????? ????? ?????", addi_w);
  def_INSTR_TAB("??????? 000 ???????????? ????? ?????", slti);
  def_INSTR_TAB("??????? 101 ???????????? ????? ?????", andi);
  def_INSTR_TAB("??????? 110 ???????????? ????? ?????", ori);
  def_INSTR_TAB("??????? 111 ???????????? ????? ?????", xori);  
  def_INSTR_TAB("??????? 001 ???????????? ????? ?????", sltui);
  return table_inv(s);
};

def_THelper(int_i8) {
  def_INSTR_TAB("?????????? 0000001 ????? ????? ?????", slli_w);
  def_INSTR_TAB("?????????? 0001001 ????? ????? ?????", srli_w);
  def_INSTR_TAB("?????????? 0010001 ????? ????? ?????", srai_w);  
  return table_inv(s);
};

def_THelper(cnt) {
  def_INSTR_TAB("?????????????????? 1000 00000 ?????", rdcntvl_w);
  def_INSTR_TAB("?????????????????? 1001 00000 ?????", rdcntvh_w);
  def_INSTR_TAB("?????????????????? 1000 ????? 00000", rdcntid_w);
  return table_inv(s);
};

def_THelper(csr) {
  def_INSTR_TAB("???????? ?????????????? 00000 ?????", csrrd);
  def_INSTR_TAB("???????? ?????????????? 00001 ?????", csrwr);
  def_INSTR_TAB("???????? ?????????????? ????? ?????", csrxchg);
  return table_inv(s);
};

// instructions that appears more should be listed front
def_THelper(main) {
  def_INSTR_IDTAB("000000000001 ????? ????? ????? ?????",r3, intop);
  def_INSTR_IDTAB("0010 ?????? ???????????? ????? ?????",r2_i12, mem);
  def_INSTR_IDTAB("01???? ???????????????? ????? ?????",r2_i16, branch);
  def_INSTR_IDTAB("0000001 ??? ???????????? ????? ?????",r2_i12, int_i12);
  def_INSTR_IDTAB("0000000001 ??????? ????? ????? ?????",r2_i8, int_i8);

  def_INSTR_IDTAB("0001110 ???????????????????? ?????",pcaddu12i, pcaddu12i);
  def_INSTR_IDTAB("0001010 ???????????????????? ?????",r1_i20, lu12i_w);

  def_INSTR_IDTAB("0000000000100 ???? ????? ????? ?????",r3, divop);

  def_INSTR_IDTAB("00000100 ?????????????? ????? ?????",r2_i14s, csr);
  def_INSTR_IDTAB("000000000000000001 ???? ????? ?????",r2, cnt);
  
  def_INSTR_IDTAB("00000000001010110 ???????????????",code_15, syscall);
  def_INSTR_IDTAB("00000000001010100 ???????????????",code_15, break);
  def_INSTR_IDTAB("00000110010010001 ???????????????",code_15, idle);
  def_INSTR_TAB("00000110010010000011100000000000",ertn);

  def_INSTR_TAB("00000110010010000010100000000000",tlbsrch);
  def_INSTR_TAB("00000110010010000010110000000000",tlbrd);
  def_INSTR_TAB("00000110010010000011000000000000",tlbwr);
  def_INSTR_TAB("00000110010010000011010000000000",tlbfill);  
  def_INSTR_IDTAB("00000110010010011 ????? ????? ?????",invtlb, invtlb); 

  def_INSTR_TAB("00111000011100100???????????????",ibar);
  def_INSTR_TAB("00111000011100101???????????????",dbar);
  def_INSTR_IDTAB("0000011000 ???????????? ????? ?????",cacop, cacop);

  def_INSTR_TAB("10000000000000000000000000000000",nemu_trap);
  // def_INSTR_TAB("11000000000000000000000000000000",print_led);

  return table_inv(s);
}


int isa_fetch_decode(Decode *s) {
  if(s->snpc & ((vaddr_t)0x3)){
    printf("[NEMU] PC: 0x%x [NEMU]: inst fetch, PC = 0x%x, not 4 aligned\n", cpu.pc, cpu.pc);
    BADV->val = s->snpc;
    longjmp_exception(EX_ADE); // ADEF exception
  }

  s->isa.instr.val = instr_fetch(&s->snpc, 4);
  int idx = table_main(s);
  cpu.idle_pc = s->pc;
  s->type = INSTR_TYPE_N;

  return idx;
}

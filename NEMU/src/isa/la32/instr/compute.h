/* follow codes in isa/mips32/instr/compute.h */
/*******************************************************/
def_EHelper(add_w) {
  rtl_add(s, ddest, dsrc1, dsrc2);
}

def_EHelper(sub_w) {
  rtl_sub(s, ddest, dsrc1, dsrc2);
}

def_EHelper(slt) {
  rtl_setrelop(s, RELOP_LT, ddest, dsrc1, dsrc2);
}

def_EHelper(sltu) {
  rtl_setrelop(s, RELOP_LTU, ddest, dsrc1, dsrc2);
}

def_EHelper(xor) {
  rtl_xor(s, ddest, dsrc1, dsrc2);
}

def_EHelper(or) {
  rtl_or(s, ddest, dsrc1, dsrc2);
}

def_EHelper(and) {
  rtl_and(s, ddest, dsrc1, dsrc2);
}

def_EHelper(nor) {
  rtl_nor(s, ddest, dsrc1, dsrc2);
}

def_EHelper(sll_w) {
  rtl_shl(s, ddest, dsrc1, dsrc2);
}

def_EHelper(sra_w) {
  rtl_sar(s, ddest, dsrc1, dsrc2);
}

def_EHelper(srl_w) {
  rtl_shr(s, ddest, dsrc1, dsrc2);
}

def_EHelper(slli_w) {
  rtl_shli(s, ddest, dsrc1, id_src2->imm);
}

def_EHelper(srai_w) {
  rtl_sari(s, ddest, dsrc1, id_src2->imm);
}

def_EHelper(srli_w) {
  rtl_shri(s, ddest, dsrc1, id_src2->imm);
}

/********************* mul/div ************************/

def_EHelper(mul_w) {
  rtl_mulu_lo(s, ddest, dsrc1, dsrc2);
}

def_EHelper(mulh_w) {
  rtl_muls_hi(s, ddest, dsrc1, dsrc2);
}

def_EHelper(mulh_wu) {
  rtl_mulu_hi(s, ddest, dsrc1, dsrc2);
}

def_EHelper(div_w) {
    rtl_divs_q(s, ddest, dsrc1, dsrc2);
}

def_EHelper(div_wu) {
    rtl_divu_q(s, ddest, dsrc1, dsrc2);
}

def_EHelper(mod_w) {
    rtl_divs_r(s, ddest, dsrc1, dsrc2);
}

def_EHelper(mod_wu) {
    rtl_divu_r(s, ddest, dsrc1, dsrc2);
}

/*******************************************************/

def_EHelper(lu12i_w) {
  rtl_li(s, ddest, id_src1->imm);
}

def_EHelper(pcaddu12i) {
  rtl_li(s, ddest, id_src1->imm);
}

/*******************************************************/

def_EHelper(addi_w) {
  rtl_addi(s, ddest, dsrc1, id_src2->imm);
}

def_EHelper(slti) {
  rtl_setrelopi(s, RELOP_LT, ddest, dsrc1, id_src2->imm);
}

def_EHelper(sltui) {
  rtl_setrelopi(s, RELOP_LTU, ddest, dsrc1, id_src2->imm);
}

def_EHelper(xori) {
  rtl_xori(s, ddest, dsrc1, id_src2->imm);
}

def_EHelper(ori) {
  rtl_ori(s, ddest, dsrc1, id_src2->imm);
}

def_EHelper(andi) {
  rtl_andi(s, ddest, dsrc1, id_src2->imm);
}


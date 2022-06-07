/***************************************************************************************
* Copyright (c) 2014-2021 Zihao Yu, Nanjing University
* Copyright (c) 2020-2021 Institute of Computing Technology, Chinese Academy of Sciences
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

#include <isa.h>

#ifdef CONFIG_PERF_OPT
#define ENABLE_HOSTTLB 1
#endif

#include <cpu/decode.h>
#include <memory/paddr.h>
#include <memory/vaddr.h>
#include <memory/host-tlb.h>

#ifndef __ICS_EXPORT
#ifndef ENABLE_HOSTTLB
static word_t vaddr_read_cross_page(vaddr_t addr, int len, int type) {
  word_t data = 0;
  int i;
  for (i = 0; i < len; i ++, addr ++) {
    paddr_t mmu_ret = isa_mmu_translate(addr, 1, type);
    int ret = mmu_ret & PAGE_MASK;
    if (ret != MEM_RET_OK) return 0;
    paddr_t paddr = (mmu_ret & ~PAGE_MASK) | (addr & PAGE_MASK);
#ifdef CONFIG_MULTICORE_DIFF
    word_t byte = (type == MEM_TYPE_IFETCH ? golden_pmem_read : paddr_read)(paddr, 1);
#else
    word_t byte = (type == MEM_TYPE_IFETCH ? paddr_read : paddr_read)(paddr, 1);
#endif
    data |= byte << (i << 3);
  }
  return data;
}

static void vaddr_write_cross_page(vaddr_t addr, int len, word_t data) {
  int i;
  for (i = 0; i < len; i ++, addr ++) {
    paddr_t mmu_ret = isa_mmu_translate(addr, 1, MEM_TYPE_WRITE);
    int ret = mmu_ret & PAGE_MASK;
    if (ret != MEM_RET_OK) return;
    paddr_t paddr = (mmu_ret & ~PAGE_MASK) | (addr & PAGE_MASK);
    paddr_write(paddr, 1, data & 0xff);
    data >>= 8;
  }
}

__attribute__((noinline))
static word_t vaddr_mmu_read(struct Decode *s, vaddr_t addr, int len, int type) {
#ifdef XIANGSHAN_DEBUG
  vaddr_t vaddr = addr;
#endif
  paddr_t paddr = isa_mmu_translate(addr, len, type);
  if(type == MEM_TYPE_IFETCH){
    //printf("[NEMU]: inst fetch at vaddr 0x%lx\n",addr);
    //printf("[NEMU]: inst fetch at paddr 0x%lx\n",paddr);
  }

#ifdef CONFIG_MULTICORE_DIFF
  word_t rdata = (type == MEM_TYPE_IFETCH ? golden_pmem_read : paddr_read)(addr, len);
#else
  word_t rdata = paddr_read(paddr, len);
#endif
#ifdef XIANGSHAN_DEBUG
  printf("[NEMU] mmu_read: vaddr 0x%lx, paddr 0x%lx, rdata 0x%lx\n",
    vaddr, addr, rdata);
#endif
  return rdata;
}

__attribute__((noinline))
static void vaddr_mmu_write(struct Decode *s, vaddr_t addr, int len, word_t data) {
#ifdef XIANGSHAN_DEBUG
  vaddr_t vaddr = addr;
#endif
  paddr_t paddr = isa_mmu_translate(addr, len, MEM_TYPE_WRITE);
  // printf("[NEMU] mmu_write: pc 0x%lx vaddr 0x%lx, paddr 0x%lx, len %d, data 0x%lx\n", s->pc, 
  // addr, paddr, len, data);

#ifdef XIANGSHAN_DEBUG
  printf("[NEMU] mmu_write: vaddr 0x%lx, paddr 0x%lx, len %d, data 0x%lx\n",
    vaddr, addr, len, data);
#endif
  paddr_write(paddr, len, data);
}
#endif
#endif

static inline word_t vaddr_read_internal(void *s, vaddr_t addr, int len, int type, int mmu_mode) {
  if (unlikely(mmu_mode == MMU_DYNAMIC)) mmu_mode = isa_mmu_check(addr, len, type);
  if (mmu_mode == MMU_DIRECT) return paddr_read(addr, len);
#ifndef __ICS_EXPORT
  return MUXDEF(ENABLE_HOSTTLB, hosttlb_read, vaddr_mmu_read) ((struct Decode *)s, addr, len, type);
#endif
  return 0;
}

word_t vaddr_ifetch(vaddr_t addr, int len) {
  return vaddr_read_internal(NULL, addr, len, MEM_TYPE_IFETCH, MMU_DYNAMIC);
}

word_t vaddr_read(struct Decode *s, vaddr_t addr, int len, int mmu_mode) {
  return vaddr_read_internal(s, addr, len, MEM_TYPE_READ, mmu_mode);
}

void vaddr_write(struct Decode *s, vaddr_t addr, int len, word_t data, int mmu_mode) {
  if (unlikely(mmu_mode == MMU_DYNAMIC)) mmu_mode = isa_mmu_check(addr, len, MEM_TYPE_WRITE);
  if (mmu_mode == MMU_DIRECT) { paddr_write(addr, len, data); return; }
#ifndef __ICS_EXPORT
  MUXDEF(ENABLE_HOSTTLB, hosttlb_write, vaddr_mmu_write) (s, addr, len, data);
#endif
}

word_t vaddr_read_safe(vaddr_t addr, int len) {
  // FIXME: when reading fails, return an error instead of raising exceptions
  return vaddr_read_internal(NULL, addr, len, MEM_TYPE_READ, MMU_DYNAMIC);
}

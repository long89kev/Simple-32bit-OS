/*
 * Copyright (C) 2025 pdnguyen of HCMC University of Technology VNU-HCM
 */

/* Sierra release
 * Source Code License Grant: The authors hereby grant to Licensee
 * personal permission to use and modify the Licensed Source Code
 * for the sole purpose of studying while attending the course CO2018.
 */

// #ifdef MM_PAGING
/*
 * System Library
 * Memory Module Library libmem.c 
 */

#include "string.h"
#include "mm.h"
#include "syscall.h"
#include "libmem.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

static pthread_mutex_t mmvm_lock = PTHREAD_MUTEX_INITIALIZER;

/*enlist_vm_freerg_list - add new rg to freerg_list
 *@mm: memory region
 *@rg_elmt: new region
 *
 */
int enlist_vm_freerg_list(struct mm_struct *mm, struct vm_rg_struct *rg_elmt)
{

  struct vm_rg_struct *rg_node = mm->mmap->vm_freerg_list;

  if (rg_elmt->rg_start >= rg_elmt->rg_end){
    return -1;
  }

  if (rg_node != NULL)
    rg_elmt->rg_next = rg_node;

  /* Enlist the new region */
  mm->mmap->vm_freerg_list = rg_elmt;


  return 0;
}

/*get_symrg_byid - get mem region by region ID
 *@mm: memory region
 *@rgid: region ID act as symbol index of variable
 *
 */
struct vm_rg_struct *get_symrg_byid(struct mm_struct *mm, int rgid)
{
  if (rgid < 0 || rgid > PAGING_MAX_SYMTBL_SZ)
    return NULL;

  return &mm->symrgtbl[rgid];
}

/*__alloc - allocate a region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size
 *@alloc_addr: address of allocated memory region
 *
 */
int __alloc(struct pcb_t *caller, int vmaid, int rgid, int size, int *alloc_addr)
{

  pthread_mutex_lock(&mmvm_lock);

  if (rgid < 0 || rgid > PAGING_MAX_SYMTBL_SZ)
  {
    pthread_mutex_unlock(&mmvm_lock);
    return -1;
  }

  /*Allocate at the toproof */
  struct vm_rg_struct rgnode;

  /* TODO: commit the vmaid */
  rgnode.vmaid = vmaid;

  if (get_free_vmrg_area(caller, vmaid, size, &rgnode) == 0)
  {
    caller->mm->symrgtbl[rgid].rg_start = rgnode.rg_start;
    caller->mm->symrgtbl[rgid].rg_end = rgnode.rg_end;
    caller->mm->symrgtbl[rgid].vmaid = rgnode.vmaid;
 
    *alloc_addr = rgnode.rg_start;

    pthread_mutex_unlock(&mmvm_lock);
    return 0;
  }

  /* TODO get_free_vmrg_area FAILED handle the region management (Fig.6)*/

  /* TODO retrive current vma if needed, current comment out due to compiler redundant warning*/
  /*Attempt to increate limit to get space */
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);
  int inc_sz = PAGING_PAGE_ALIGNSZ(size);
  // int inc_limit_ret;
  /* TODO retrive old_sbrk if needed, current comment out due to compiler redundant warning*/
  int old_sbrk = cur_vma->sbrk;

  if(vmaid == 0){ //heap
    int gap_size = cur_vma->vm_end - old_sbrk;
    
    if(gap_size >= size){
      cur_vma->sbrk = cur_vma->sbrk + size;

      caller->mm->symrgtbl[rgid].rg_start = old_sbrk;
      caller->mm->symrgtbl[rgid].rg_end = old_sbrk + size;
      caller->mm->symrgtbl[rgid].vmaid = vmaid;
      *alloc_addr = rgnode.rg_start;
    }
    else{
      inc_vma_limit(caller, vmaid, inc_sz);
      caller->mm->symrgtbl[rgid].rg_start = old_sbrk;
      caller->mm->symrgtbl[rgid].rg_end = old_sbrk + size;
      caller->mm->symrgtbl[rgid].vmaid = vmaid;
      *alloc_addr = rgnode.rg_start;
      cur_vma->sbrk += size;
      
    }
    pthread_mutex_unlock(&mmvm_lock);
    return 0;
  } 
  else if(vmaid == 1){
    int gap_size = old_sbrk - cur_vma->vm_end;
    if(gap_size >= size){
      caller->mm->symrgtbl[rgid].rg_start = old_sbrk - size;
      caller->mm->symrgtbl[rgid].rg_end = old_sbrk;
      caller->mm->symrgtbl[rgid].vmaid = vmaid;
      *alloc_addr = rgnode.rg_end;
    }
    else{
      inc_vma_limit(caller, vmaid, inc_sz);
      caller->mm->symrgtbl[rgid].rg_start = old_sbrk - size;
      caller->mm->symrgtbl[rgid].rg_end = old_sbrk;
      caller->mm->symrgtbl[rgid].vmaid = vmaid;
      *alloc_addr = rgnode.rg_start;
      cur_vma->sbrk -= size;
    }
    pthread_mutex_unlock(&mmvm_lock);
    return 0;
  }
  else{
    pthread_mutex_unlock(&mmvm_lock);
    return -1;
  }
    /* TODO INCREASE THE LIMIT as inovking systemcall 
   * sys_memap with SYSMEM_INC_OP 
   */
  //struct sc_regs regs;
  //regs.a1 = ...
  //regs.a2 = ...
  //regs.a3 = ...
  
  /* SYSCALL 17 sys_memmap */

  /* TODO: commit the limit increment */

  /* TODO: commit the allocation address 
  // *alloc_addr = ...
  */
  pthread_mutex_unlock(&mmvm_lock);
  return 0;

}

/*__free - remove a region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size
 *
 */
int __free(struct pcb_t *caller, int vmaid, int rgid)
{
  //struct vm_rg_struct rgnode;

  // Dummy initialization for avoding compiler dummay warning
  // in incompleted TODO code rgnode will overwrite through implementing
  // the manipulation of rgid later

  if(rgid < 0 || rgid > PAGING_MAX_SYMTBL_SZ){
    return -1;
  }

  /* TODO: Manage the collect freed region to freerg_list */
  struct vm_rg_struct * cur_rg = get_symrg_byid(caller->mm, rgid);
  // struct vm_area_struct * cur_vma = get_vma_by_num(caller->mm, cur_rg->vmaid);

  struct vm_rg_struct * free_reg = (struct vm_rg_struct *)malloc(sizeof(struct vm_rg_struct));

  free_reg->rg_end = cur_rg->rg_end;
  free_reg->rg_start = cur_rg->rg_start;
  free_reg->vmaid = cur_rg->vmaid;

  free_reg->rg_next = NULL;

  caller->mm->symrgtbl[rgid].rg_start = caller->mm->symrgtbl[rgid].rg_end = -1;
  int flag = enlist_vm_freerg_list(caller->mm, free_reg);

  if(flag != 0) {
    return -1;
  }  

  /*enlist the obsoleted memory region */
  //enlist_vm_freerg_list();
  return 0;
}

/*liballoc - PAGING-based allocate a region memory
 *@proc:  Process executing the instruction
 *@size: allocated size
 *@reg_index: memory region ID (used to identify variable in symbole table)
 */
int liballoc(struct pcb_t *proc, uint32_t size, uint32_t reg_index)
{
  /* TODO Implement allocation on vm area 0 */
  // pthread_mutex_lock(&mmvm_lock);
  int addr;
  int ret = __alloc(proc, 0, reg_index, size, &addr);
  #ifdef IODUMP 
  printf("===== PHYSICAL MEMORY AFTER ALLOCATION =======\n");
  printf("PID=%d - Region=%d - Address=%08x - Size=%d bytes\n", proc->pid, reg_index, addr, size);
  print_pgtbl(proc, 0, proc->mm->mmap->vm_end);
  for (int i = 0; i < PAGING_MAX_PGN; ++i) {
    if (PAGING_PAGE_PRESENT(proc->mm->pgd[i])){
      printf("Page Number: %d -> Frame Number : %d\n", i, PAGING_FPN(proc->mm->pgd[i]));
    }
  }
  printf("==============================================\n");
  #endif
  // pthread_mutex_unlock(&mmvm_lock);
  /* By default using vmaid = 0 */
  return ret;
}

/*libfree - PAGING-based free a region memory
 *@proc: Process executing the instruction
 *@size: allocated size
 *@reg_index: memory region ID (used to identify variable in symbole table)
 */

int libfree(struct pcb_t *proc, uint32_t reg_index)
{
  // pthread_mutex_lock(&mmvm_lock);
  /* TODO Implement free region */
  int ret = __free(proc, 0, reg_index);
  /* By default using vmaid = 0 */
  #ifdef IODUMP
  printf("===== PHYSICAL MEMORY AFTER DEALLOCATION =====\n");
  printf("PID=%d - Region=%d\n", proc->pid, reg_index);
  print_pgtbl(proc, 0, proc->mm->mmap->vm_end);
  for (int i = 0; i < PAGING_MAX_PGN; ++i) {
    if (PAGING_PAGE_PRESENT(proc->mm->pgd[i])){
      printf("Page Number: %d -> Frame Number : %d\n", i, PAGING_FPN(proc->mm->pgd[i]));
    }
  }
  printf("==============================================\n");
  #endif
  // pthread_mutex_unlock(&mmvm_lock);
  return ret;
}

/*pg_getpage - get the page in ram
 *@mm: memory region
 *@pagenum: PGN
 *@framenum: return FPN
 *@caller: caller
 *
 */
int pg_getpage(struct mm_struct *mm, int pgn, int *fpn, struct pcb_t *caller)
{
  

  if (pgn < 0 || pgn >= PAGING_MAX_PGN)
  {
    return -1;
  }

  uint32_t pte = mm->pgd[pgn];

  if (!PAGING_PAGE_PRESENT(pte))
  { /* Page is not online, make it actively living */
    int vicpgn, swpfpn; 
    //int vicfpn;
    //uint32_t vicpte;

    int tgtfpn = PAGING_PTE_SWP(pte);//the target frame storing our variable

    /* TODO: Play with your paging theory here */
    /* Find victim page */
    // find_victim_page(caller->mm, &vicpgn);
    if (find_victim_page(caller->mm, &vicpgn) != 0) {
      return -1; // i think we should have case when cannot find a victim page
    }
    int vicframe_num = PAGING_FPN(caller->mm->pgd[vicpgn]);
    /* Get free frame in MEMSWP */
    MEMPHY_get_freefp(caller->active_mswp, &swpfpn);
    
    /* TODO: Implement swap frame from MEMRAM to MEMSWP and vice versa*/
    __swap_cp_page(caller->mram, vicframe_num, caller->active_mswp, swpfpn); //swap content victim từ ram vào swpfpn trong active_swap
    /* TODO copy victim frame to swap 
    * SWP(vicfpn <--> swpfpn)
    * SYSCALL 17 sys_memmap 
    * with operation SYSMEM_SWP_OP
    */
    //struct sc_regs regs;
    //regs.a1 =...
    //regs.a2 =...
    //regs.a3 =..

    /* SYSCALL 17 sys_memmap */
    __swap_cp_page(caller->active_mswp, tgtfpn, caller->mram, vicframe_num); //swap content target từ active_swap vào RAM victim 
    /* TODO copy target frame form swap to mem 
     * SWP(tgtfpn <--> vicfpn)
     * SYSCALL 17 sys_memmap
     * with operation SYSMEM_SWP_OP
     */
    /* TODO copy target frame form swap to mem 
    //regs.a1 =...
    //regs.a2 =...
    //regs.a3 =..
    */

    /* SYSCALL 17 sys_memmap */

    /* Update page table */
    pte_set_swap(&caller->mm->pgd[vicpgn], 0, swpfpn); //swap out pgd[vic] vào swpfpn đang ở secondary
    //mm->pgd;

    /* Update its online status of the target page */
    pte_set_fpn(&caller->mm->pgd[pgn], vicframe_num); //swap in cái pgn (target) vào vicpgn ở RAM
    //mm->pgd[pgn];
    //pte_set_fpn();S

    enlist_pgn_node(&caller->mm->fifo_pgn,pgn);

  }

  *fpn = PAGING_FPN(mm->pgd[pgn]);

  return 0;
}

/*pg_getval - read value at given offset
 *@mm: memory region
 *@addr: virtual address to acess
 *@value: value
 *
 */
int pg_getval(struct mm_struct *mm, int addr, BYTE *data, struct pcb_t *caller)
{
  int pgn = PAGING_PGN(addr);
  int off = PAGING_OFFST(addr);
  int fpn;

  /* Get the page to MEMRAM, swap from MEMSWAP if needed */
  if (pg_getpage(mm, pgn, &fpn, caller) != 0){
    return -1; /* invalid page access */
  }
    
  int phy_addr = (fpn << PAGING_ADDR_FPN_LOBIT) + off; //physical addr = page * bit of 1_page + offset
  MEMPHY_read(caller->mram, phy_addr, data);    //read from phy addr (the phy addr also has a BYTE to store data)


  
  /* TODO 
   *  MEMPHY READ 
   *  SYSCALL 17 sys_memmap with SYSMEM_IO_READ
   */
  // int phyaddr
  //struct sc_regs regs;
  //regs.a1 = ...
  //regs.a2 = ...
  //regs.a3 = ...

  /* SYSCALL 17 sys_memmap */

  // Update data
  // data = (BYTE)

  return 0;
}

/*pg_setval - write value to given offset
 *@mm: memory region
 *@addr: virtual address to acess
 *@value: value
 *
 */
int pg_setval(struct mm_struct *mm, int addr, BYTE value, struct pcb_t *caller)
{
  int pgn = PAGING_PGN(addr);
  int off = PAGING_OFFST(addr);
  int fpn;

  /* Get the page to MEMRAM, swap from MEMSWAP if needed */
  if (pg_getpage(mm, pgn, &fpn, caller) != 0){
    return -1; /* invalid page access */
  }

  int phy_addr = (fpn << PAGING_ADDR_FPN_LOBIT) + off; //physical addr = frame_num * bit of 1_frame + offset
  MEMPHY_write(caller->mram, phy_addr, value);    //write value to phy addr
  /* TODO
   *  MEMPHY_write(caller->mram, phyaddr, value);
   *  MEMPHY WRITE
   *  SYSCALL 17 sys_memmap with SYSMEM_IO_WRITE
   */
  // int phyaddr
  //struct sc_regs regs;
  //regs.a1 = ...
  //regs.a2 = ...
  //regs.a3 = ...

  /* SYSCALL 17 sys_memmap */

  // Update data
  // data = (BYTE) 

  return 0;
}

/*__read - read value in region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@offset: offset to acess in memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size
 *
 */
int __read(struct pcb_t *caller, int vmaid, int rgid, int offset, BYTE *data)
{
  struct vm_rg_struct *currg = get_symrg_byid(caller->mm, rgid);
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

  if (currg == NULL || cur_vma == NULL) /* Invalid memory identify */
    return -1;

  pg_getval(caller->mm, currg->rg_start + offset, data, caller);

  return 0;
}

/*libread - PAGING-based read a region memory */
int libread(
    struct pcb_t *proc, // Process executing the instruction
    uint32_t source,    // Index of source register
    uint32_t offset,    // Source address = [source] + [offset]
    uint32_t* destination)
{
  // pthread_mutex_lock(&mmvm_lock);
  BYTE data;
  int val = __read(proc, 0, source, offset, &data);

  /* TODO update result of reading action*/
  
  *destination = data;   
#ifdef IODUMP
  printf("======= PHYSICAL MEMORY AFTER READING ========\n");
  printf("read region=%d offset=%d value=%d\n", source, offset, data);
  #ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1); //print max TBL
#endif
  MEMPHY_dump(proc->mram);
  for (int i = 0; i < PAGING_MAX_PGN; ++i) {
    if (PAGING_PAGE_PRESENT(proc->mm->pgd[i])){
      printf("Page Number: %d -> Frame Number : %d\n", i, PAGING_FPN(proc->mm->pgd[i]));
    }
  }
  printf("==============================================\n");
#endif
  // pthread_mutex_unlock(&mmvm_lock);
  return val;
}

/*__write - write a region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@offset: offset to acess in memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size
 *
 */
int __write(struct pcb_t *caller, int vmaid, int rgid, int offset, BYTE value)
{
  struct vm_rg_struct *currg = get_symrg_byid(caller->mm, rgid);
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

  if (currg == NULL || cur_vma == NULL) /* Invalid memory identify */
    return -1;

  pg_setval(caller->mm, currg->rg_start + offset, value, caller);

  return 0;
}

/*libwrite - PAGING-based write a region memory */
int libwrite(
    struct pcb_t *proc,   // Process executing the instruction
    BYTE data,            // Data to be wrttien into memory
    uint32_t destination, // Index of destination register
    uint32_t offset)
{
  // pthread_mutex_lock(&mmvm_lock);
  int ret = __write(proc, 0, destination, offset, data); 
  #ifdef IODUMP
  printf("======= PHYSICAL MEMORY AFTER WRITING ========\n");
  printf("write region=%d offset=%d value=%d\n", destination, offset, data);
  
  #ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1); //print max TBL
  #endif
  MEMPHY_dump(proc->mram);
  for (int i = 0; i < PAGING_MAX_PGN; ++i) {
    if (PAGING_PAGE_PRESENT(proc->mm->pgd[i])){
      printf("Page Number: %d -> Frame Number : %d\n", i, PAGING_FPN(proc->mm->pgd[i]));
    }
  }
  printf("==============================================\n");
  
  #endif
  // pthread_mutex_unlock(&mmvm_lock);
  return ret;
}

/*free_pcb_memphy - collect all memphy of pcb
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@incpgnum: number of page
 */
int free_pcb_memph(struct pcb_t *caller)
{
  int pagenum, fpn;
  uint32_t pte;


  for(pagenum = 0; pagenum < PAGING_MAX_PGN; pagenum++)
  {
    pte= caller->mm->pgd[pagenum];

    if (!PAGING_PAGE_PRESENT(pte))
    {
      fpn = PAGING_PTE_FPN(pte);
      MEMPHY_put_freefp(caller->mram, fpn);
    } else {
      fpn = PAGING_PTE_SWP(pte);
      MEMPHY_put_freefp(caller->active_mswp, fpn);    
    }
  }

  return 0;
}


/*find_victim_page - find victim page
 *@caller: caller
 *@pgn: return page number
 *
 */
int find_victim_page(struct mm_struct *mm, int *retpgn)
{
  struct pgn_t *pg = mm->fifo_pgn;

  /* TODO: Implement the theorical mechanism to find the victim page */
  if(pg == NULL){ //nothing in the fifo queue
    *retpgn = -1;
    //free(pg);
    return -1;
  }
  else if(pg && pg->pg_next == NULL){
    *retpgn = pg->pgn; //1 element in fifo queue
    free(pg);
    mm->fifo_pgn = NULL;
    return 0;
  }
  
  struct pgn_t* prev = pg; //track behind pg
  pg = pg->pg_next; 

  while(pg->pg_next != NULL){
    pg = pg->pg_next; //pg become the last node
    prev = prev->pg_next;   //prev is second last node
  }

  *retpgn = pg->pgn;
  prev->pg_next = NULL;

  free(pg);

  return 0;
}

/*get_free_vmrg_area - get a free vm region
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@size: allocated size
 *
 */
int get_free_vmrg_area(struct pcb_t *caller, int vmaid, int size, struct vm_rg_struct *newrg)
{

  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

  struct vm_rg_struct *rgit = cur_vma->vm_freerg_list;

  if (rgit == NULL)
    return -1;
    

  /* Probe unintialized newrg */
  newrg->rg_start = newrg->rg_end = -1;

  /* TODO Traverse on list of free vm region to find a fit space */
  //while (...)
  // ..
  while(rgit != NULL && rgit->vmaid == vmaid){

    if(rgit->rg_start + size <= rgit->rg_end){
      newrg->rg_start = rgit->rg_start;
      newrg->rg_end = rgit->rg_start + size;

      if(rgit->rg_start + size < rgit->rg_end){
        rgit->rg_start = rgit->rg_start + size;
        return 0; 
      }
      else{
        if(rgit->rg_next != NULL){

          rgit->rg_start = rgit->rg_next->rg_start;
          rgit->rg_end = rgit->rg_next->rg_end;
          struct vm_rg_struct* tmp = rgit->rg_next;
          rgit->rg_next = rgit->rg_next->rg_next;
          free(tmp);
        }
        else{
          rgit->rg_start = rgit->rg_end;	//dummy, size 0 region
          rgit->rg_next = NULL;
        }
      }
    }
    else{
      rgit = rgit->rg_next;
    }
  }
  if(newrg->rg_start == -1) return -1;

  return 0;
}

//#endif

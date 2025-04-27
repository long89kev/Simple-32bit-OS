// #ifdef MM_PAGING
/*
 * PAGING based Memory Management
 * Memory management unit mm/mm.c
 */

#include "mm.h"
#include <stdlib.h>
#include <stdio.h>

/*
 * init_pte - Initialize PTE entry
 */
int init_pte(uint32_t *pte,
             int pre,    // present
             int fpn,    // FPN
             int drt,    // dirty
             int swp,    // swap
             int swptyp, // swap type
             int swpoff) // swap offset
{
  if (pre != 0)
  {
    if (swp == 0)
    { // Non swap ~ page online
      if (fpn == 0)
        return -1; // Invalid setting

      /* Valid setting with FPN */
      SETBIT(*pte, PAGING_PTE_PRESENT_MASK);
      CLRBIT(*pte, PAGING_PTE_SWAPPED_MASK);
      CLRBIT(*pte, PAGING_PTE_DIRTY_MASK);

      SETVAL(*pte, fpn, PAGING_PTE_FPN_MASK, PAGING_PTE_FPN_LOBIT);
    }
    else
    { // page swapped
      SETBIT(*pte, PAGING_PTE_PRESENT_MASK);
      SETBIT(*pte, PAGING_PTE_SWAPPED_MASK);
      CLRBIT(*pte, PAGING_PTE_DIRTY_MASK);

      SETVAL(*pte, swptyp, PAGING_PTE_SWPTYP_MASK, PAGING_PTE_SWPTYP_LOBIT);
      SETVAL(*pte, swpoff, PAGING_PTE_SWPOFF_MASK, PAGING_PTE_SWPOFF_LOBIT);
    }
  }

  return 0;
}

/*
 * pte_set_swap - Set PTE entry for swapped page
 * @pte    : target page table entry (PTE)
 * @swptyp : swap type
 * @swpoff : swap offset
 */
int pte_set_swap(uint32_t *pte, int swptyp, int swpoff)
{
  SETBIT(*pte, PAGING_PTE_PRESENT_MASK);
  SETBIT(*pte, PAGING_PTE_SWAPPED_MASK);

  SETVAL(*pte, swptyp, PAGING_PTE_SWPTYP_MASK, PAGING_PTE_SWPTYP_LOBIT);
  SETVAL(*pte, swpoff, PAGING_PTE_SWPOFF_MASK, PAGING_PTE_SWPOFF_LOBIT);

  return 0;
}

/*
 * pte_set_swap - Set PTE entry for on-line page
 * @pte   : target page table entry (PTE)
 * @fpn   : frame page number (FPN)
 */
int pte_set_fpn(uint32_t *pte, int fpn)
{
  SETBIT(*pte, PAGING_PTE_PRESENT_MASK);
  CLRBIT(*pte, PAGING_PTE_SWAPPED_MASK);

  SETVAL(*pte, fpn, PAGING_PTE_FPN_MASK, PAGING_PTE_FPN_LOBIT);

  return 0;
}

/*
 * vmap_page_range - map a range of page at aligned address
 */
// int vmap_page_range(struct pcb_t *caller,           // process call
//                     int addr,                       // start address which is aligned to pagesz
//                     int pgnum,                      // num of mapping page
//                     struct framephy_struct *frames, // list of the mapped frames
//                     struct vm_rg_struct *ret_rg)    // return mapped region, the real mapped fp
// {                                                   // no guarantee all given pages are mapped
//   struct framephy_struct *fpit = frames; // frame iterator
//   int pgit = 0; // page iterator
//   int pgn = PAGING_PGN(addr); // page number

//   // TODO: update the rg_end and rg_start of ret_rg
//   ret_rg->rg_end = addr;
//   ret_rg->rg_start = addr;
//   ret_rg->vmaid = caller->mm->mmap->vm_id;

//   //fpit->fp_next = frames;

//   while(fpit->fp_next != NULL && pgit < pgnum){
//     uint32_t * pte = &caller->mm->pgd[pgn]; //page_table_entry = caller->mm->pgd[] like the teacher said

//     pte_set_fpn(pte, fpit->fpn);  //map the pte into a frame fpit, gpt the function for explain
//     enlist_pgn_node(&caller->mm->fifo_pgn, pgn); //enlist into page queue

//     fpit = fpit->fp_next;   //move to the next physic frame
//     addr += PAGING_PAGESZ;  //move the addr to the addr of the next page
//     pgn = PAGING_PGN(addr); //reset the pgn value to match new addr
//     pgit++;                 //(pgit = page_iterator) number of pages has been mapped increase by 1
//     ret_rg->rg_end = addr;  //end of the register is set to match new addr
//   }
//   /* TODO map range of frame to address space
//    *      [addr to addr + pgnum*PAGING_PAGESZ
//    *      in page table caller->mm->pgd[]
//    */
//   while(pgit < pgnum){
//     int swap_frame_id;
//     uint32_t * pte = &caller->mm->pgd[pgn];

//     MEMPHY_get_freefp(caller->active_mswp, &swap_frame_id); //allocate frame_id from secondary storage to swap
//     pte_set_swap(pte, 0, swap_frame_id);    // set swap bit and val for the page

//     addr += PAGING_PAGESZ;  //move the addr to the addr of the next page
//     pgn = PAGING_PGN(addr); //reset the pgn value to match new addr
//     pgit++;                 //(pgit = page_iterator) number of pages has been mapped increase by 1
//     ret_rg->rg_end = addr;  //end of the register is set to match new addr
//   }
//   /* Tracking for later page replacement activities (if needed)
//    * Enqueue new usage page */

//   return 0;
// }

int vmap_page_range(struct pcb_t *caller,           // process call
                    int addr,                       // start address which is aligned to pagesz
                    int pgnum,                      // num of mapping page requested
                    struct framephy_struct *frames, // list of the mapped frames
                    struct vm_rg_struct *ret_rg)    // return mapped region
{
  // Note: No guarantee all 'pgnum' pages are mapped if 'frames' runs out.

  struct framephy_struct *fpit = frames; // Frame iterator
  int pgit = 0;                          // Counter for successfully mapped pages
  int start_pgn = PAGING_PGN(addr);      // Starting virtual page number

  // --- Fix 1: Initialize ret_rg ---
  ret_rg->rg_start = addr;
  ret_rg->rg_end = addr; // Initialize end to start (empty range initially)
  // Ensure mm and mmap are valid before dereferencing
  if (caller && caller->mm && caller->mm->mmap)
  {
    ret_rg->vmaid = caller->mm->mmap->vm_id;
  }
  else
  {
    // Handle error: caller or mm structure is invalid
    // Depending on system design, could return an error code here,
    // log an error, or assert. For now, set a default/invalid vmaid.
    ret_rg->vmaid = -1; // Indicate an issue
    // Consider returning an error if vmaid is essential
    // return -1; // Example: Return error if caller/mm is invalid
  }

  // --- Fix 2: Loop correctly maps available frames ---
  // Iterate up to pgnum pages OR until physical frames run out.
  for (pgit = 0; pgit < pgnum && fpit != NULL; pgit++)
  {
    int current_pgn = start_pgn + pgit; // Calculate current page number

    // Check if page table is large enough (optional but good practice)
    // if (current_pgn >= MAX_PAGE_TABLE_ENTRIES) {
    //     fprintf(stderr, "Error: Page number %d exceeds page table size\n", current_pgn);
    //     break; // Stop mapping if out of bounds
    // }

    // Get the Page Table Entry (PTE) address
    // Ensure pgd is valid and large enough
    if (!caller || !caller->mm || !caller->mm->pgd)
    {
      // Handle error: Invalid process or memory structure
      break; // Stop mapping
    }
    uint32_t *pte = &caller->mm->pgd[current_pgn];

    // Map the PTE to the current frame's physical frame number (PFN)
    pte_set_fpn(pte, fpit->fpn);

    // Tracking for later page replacement activities (if needed)
    // Enqueue the page number that just got mapped into physical memory
    // Ensure fifo_pgn list head pointer is valid
    if (caller->mm)
    { // Check if mm is valid
      enlist_pgn_node(&caller->mm->fifo_pgn, current_pgn);
    } // Else: Cannot enlist, potentially log this

    // Move to the next available physical frame
    fpit = fpit->fp_next;
  } // End of loop

  // --- Fix 3: Update ret_rg->rg_end based on *actual* pages mapped ---
  // 'pgit' now holds the count of pages successfully mapped.
  ret_rg->rg_end = addr + pgit * PAGING_PAGESZ;

  // Return 0 indicating success (function completed its task,
  // even if fewer pages were mapped than requested due to frame shortage).
  return 0;
}

/*
 * alloc_pages_range - allocate req_pgnum of frame in ram
 * @caller    : caller
 * @req_pgnum : request page num
 * @frm_lst   : frame list
 */

int alloc_pages_range(struct pcb_t *caller, int req_pgnum, struct framephy_struct **frm_lst)
{
  int pgit, fpn;
  struct framephy_struct *newfp_str = NULL;

  /* TODO: allocate the page
  //caller-> ...
  //frm_lst-> ...
  */

  for (pgit = 0; pgit < req_pgnum; pgit++)
  {
    /* TODO: allocate the page
     */
    if (MEMPHY_get_freefp(caller->mram, &fpn) == 0)
    {
      newfp_str = malloc(sizeof(struct framephy_struct));

      // Initialize the new frame structure
      newfp_str->fpn = fpn; //
      newfp_str->owner = caller->mm;
      newfp_str->fp_next = *frm_lst;
      *frm_lst = newfp_str;
    }
    else
    { // TODO: ERROR CODE of obtaining somes but not enough frames
      return -1;
    }
  }

  return 0;
}

/*
 * vm_map_ram - do the mapping all vm are to ram storage device
 * @caller    : caller
 * @astart    : vm area start
 * @aend      : vm area end
 * @mapstart  : start mapping point
 * @incpgnum  : number of mapped page
 * @ret_rg    : returned region
 */
int vm_map_ram(struct pcb_t *caller, int astart, int aend, int mapstart, int incpgnum, struct vm_rg_struct *ret_rg)
{
  struct framephy_struct *frm_lst = NULL;
  int ret_alloc;

  /*@bksysnet: author provides a feasible solution of getting frames
   *FATAL logic in here, wrong behaviour if we have not enough page
   *i.e. we request 1000 frames meanwhile our RAM has size of 3 frames
   *Don't try to perform that case in this simple work, it will result
   *in endless procedure of swap-off to get frame and we have not provide
   *duplicate control mechanism, keep it simple
   */
  ret_alloc = alloc_pages_range(caller, incpgnum, &frm_lst);

  if (ret_alloc < 0 && ret_alloc != -3000)
    return -1;

  /* Out of memory */
  if (ret_alloc == -3000)
  {
#ifdef MMDBG
    printf("OOM: vm_map_ram out of memory \n");
#endif
    return -1;
  }

  /* it leaves the case of memory is enough but half in ram, half in swap
   * do the swaping all to swapper to get the all in ram */
  vmap_page_range(caller, mapstart, incpgnum, frm_lst, ret_rg);

  return 0;
}

/* Swap copy content page from source frame to destination frame
 * @mpsrc  : source memphy
 * @srcfpn : source physical page number (FPN)
 * @mpdst  : destination memphy
 * @dstfpn : destination physical page number (FPN)
 **/
int __swap_cp_page(struct memphy_struct *mpsrc, int srcfpn,
                   struct memphy_struct *mpdst, int dstfpn)
{
  int cellidx;
  int addrsrc, addrdst;
  for (cellidx = 0; cellidx < PAGING_PAGESZ; cellidx++)
  {
    addrsrc = srcfpn * PAGING_PAGESZ + cellidx;
    addrdst = dstfpn * PAGING_PAGESZ + cellidx;

    BYTE data;
    MEMPHY_read(mpsrc, addrsrc, &data);
    MEMPHY_write(mpdst, addrdst, data);
  }

  return 0;
}

/*
 *Initialize a empty Memory Management instance
 * @mm:     self mm
 * @caller: mm owner
 */
int init_mm(struct mm_struct *mm, struct pcb_t *caller)
{
  struct vm_area_struct *vma0 = malloc(sizeof(struct vm_area_struct));

  mm->pgd = malloc(PAGING_MAX_PGN * sizeof(uint32_t));

  /* By default the owner comes with at least one vma */
  vma0->vm_id = 0;
  vma0->vm_start = 0;
  vma0->vm_end = vma0->vm_start;
  vma0->sbrk = vma0->vm_start;
  struct vm_rg_struct *first_rg = init_vm_rg(vma0->vm_start, vma0->vm_end);
  enlist_vm_rg_node(&vma0->vm_freerg_list, first_rg);

  /* TODO update VMA0 next */
  vma0->vm_next = NULL; // this is the only VMA at initialization, set its next pointer to NULL.

  /* Point vma owner backward */
  vma0->vm_mm = mm; //

  /* TODO: update mmap */
  mm->mmap = vma0; // associate this VMA with the mm structure by setting mm->mmap to point to it.

  return 0;
}

struct vm_rg_struct *init_vm_rg(int rg_start, int rg_end)
{
  struct vm_rg_struct *rgnode = malloc(sizeof(struct vm_rg_struct));

  rgnode->rg_start = rg_start;
  rgnode->rg_end = rg_end;
  rgnode->rg_next = NULL;

  return rgnode;
}

int enlist_vm_rg_node(struct vm_rg_struct **rglist, struct vm_rg_struct *rgnode)
{
  rgnode->rg_next = *rglist;
  *rglist = rgnode;

  return 0;
}

int enlist_pgn_node(struct pgn_t **plist, int pgn)
{
  struct pgn_t *pnode = malloc(sizeof(struct pgn_t));

  pnode->pgn = pgn;
  pnode->pg_next = *plist;
  *plist = pnode;

  return 0;
}

int print_list_fp(struct framephy_struct *ifp)
{
  struct framephy_struct *fp = ifp;

  printf("print_list_fp: ");
  if (fp == NULL)
  {
    printf("NULL list\n");
    return -1;
  }
  printf("\n");
  while (fp != NULL)
  {
    printf("fp[%d]\n", fp->fpn);
    fp = fp->fp_next;
  }
  printf("\n");
  return 0;
}

int print_list_rg(struct vm_rg_struct *irg)
{
  struct vm_rg_struct *rg = irg;

  printf("print_list_rg: ");
  if (rg == NULL)
  {
    printf("NULL list\n");
    return -1;
  }
  printf("\n");
  while (rg != NULL)
  {
    printf("rg[%ld->%ld]\n", rg->rg_start, rg->rg_end);
    rg = rg->rg_next;
  }
  printf("\n");
  return 0;
}

int print_list_vma(struct vm_area_struct *ivma)
{
  struct vm_area_struct *vma = ivma;

  printf("print_list_vma: ");
  if (vma == NULL)
  {
    printf("NULL list\n");
    return -1;
  }
  printf("\n");
  while (vma != NULL)
  {
    printf("va[%ld->%ld]\n", vma->vm_start, vma->vm_end);
    vma = vma->vm_next;
  }
  printf("\n");
  return 0;
}

int print_list_pgn(struct pgn_t *ip)
{
  printf("print_list_pgn: ");
  if (ip == NULL)
  {
    printf("NULL list\n");
    return -1;
  }
  printf("\n");
  while (ip != NULL)
  {
    printf("va[%d]-\n", ip->pgn);
    ip = ip->pg_next;
  }
  printf("n");
  return 0;
}

int print_pgtbl(struct pcb_t *caller, uint32_t start, uint32_t end)
{
  int pgn_start, pgn_end;
  int pgit;

  if (end == -1)
  {
    pgn_start = 0;
    struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, 0);
    end = cur_vma->vm_end;
  }
  pgn_start = PAGING_PGN(start);
  pgn_end = PAGING_PGN(end);

  printf("print_pgtbl: %d - %d", start, end);
  if (caller == NULL)
  {
    printf("NULL caller\n");
    return -1;
  }
  printf("\n");

  for (pgit = pgn_start; pgit < pgn_end; pgit++)
  {
    printf("%08ld: %08x\n", pgit * sizeof(uint32_t), caller->mm->pgd[pgit]);
  }

  return 0;
}

// #endif

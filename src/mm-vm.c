// #ifdef MM_PAGING
/*
 * PAGING based Memory Management
 * Virtual memory module mm/mm-vm.c
 */

#include "string.h"
#include "mm.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

/*get_vma_by_num - get vm area by numID
 *@mm: memory region
 *@vmaid: ID vm area to alloc memory region
 *
 */
struct vm_area_struct *get_vma_by_num(struct mm_struct *mm, int vmaid)
{
  struct vm_area_struct *pvma = mm->mmap;

  if (mm->mmap == NULL)
    return NULL;

  int vmait = pvma->vm_id;

  while (vmait < vmaid)
  {
    if (pvma == NULL)
      return NULL;

    pvma = pvma->vm_next;
    vmait = pvma->vm_id;
  }

  return pvma;
}

int __mm_swap_page(struct pcb_t *caller, int vicfpn , int swpfpn)
{
    __swap_cp_page(caller->mram, vicfpn, caller->active_mswp, swpfpn);
    return 0;
}

/*get_vm_area_node - get vm area for a number of pages
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@incpgnum: number of page
 *@vmastart: vma end
 *@vmaend: vma end
 *
 */
struct vm_rg_struct *get_vm_area_node_at_brk(struct pcb_t *caller, int vmaid, int size, int alignedsz)
{
  struct vm_rg_struct * newrg; //declare new memory region
  /* TODO retrive current vma to obtain newrg, current comment out due to compiler redundant warning*/
  if(caller == NULL || caller->mm == NULL) {
    return NULL; //check if caller or mm is null
  }

  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid); //get vm area by vmaid
  if(cur_vma == NULL) {
    return NULL; //check if cur_vma is null
  }
  
  newrg = malloc(sizeof(struct vm_rg_struct)); //allocate mem for new region
  // check if increase sbrk exceeds VMA boundary (forum, teacher said) not sure, need check
  // if (cur_vma->sbrk + alignedsz > cur_vma->vm_end) {
  //   free(newrg); //free new region if sbrk exceeds vma boundary to prevent memory leak
  //   return NULL; 
  // }
  
  /* TODO: update the newrg boundary
  */
  newrg->rg_start = cur_vma->sbrk;  //set start address to the top pointing of current vma, read function name
  newrg->rg_end = newrg->rg_start + alignedsz; //basic stuff
  newrg->rg_next = NULL; //set next to null to indicate end of list

  cur_vma->sbrk = newrg->rg_end; //advance sbrk to the end of the new region (sbrk lift up)
  //STILL NOT SURE, FIX IF I MISUNDERSTAND ANYTHING

  return newrg;
}

/*validate_overlap_vm_area
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@vmastart: vma end
 *@vmaend: vma end
 *
 */
int validate_overlap_vm_area(struct pcb_t *caller, int vmaid, int vmastart, int vmaend)
{
  struct vm_area_struct *vma = caller->mm->mmap; 

  /* TODO validate the planned memory area is not overlapped */
	while (vma != NULL) {
    if(vma->vm_start == vma->vm_end) { //check empty vma
        vma = vma->vm_next; //skip empty VM areas (where start == end)
        continue;
    }
    if(//exist vma overlaps with new area's start
      (vma->vm_start <= vmastart && vmastart <= vma->vm_end) ||
      //exist vma overlaps with new area's end
      (vma->vm_start <= vmaend && vmaend <= vma->vm_end) ||
      //exist vma is entirely within the new area // no need but include for sure
      (vmastart <= vma->vm_start && vma->vm_end <= vmaend) ||
      //new area inside exist vma // no need but include for sure
      (vma->vm_start <= vmastart && vmaend <= vma->vm_end)      
      )
    { 
        return -1;
    }
    vma = vma->vm_next; //move to next
  }
  return 0; 
}

/*inc_vma_limit - increase vm area limits to reserve space for new variable
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@inc_sz: increment size
 *
 */
int inc_vma_limit(struct pcb_t *caller, int vmaid, int inc_sz)
{
  struct vm_rg_struct * newrg = malloc(sizeof(struct vm_rg_struct));
  int inc_amt = PAGING_PAGE_ALIGNSZ(inc_sz);
  int incnumpage =  inc_amt / PAGING_PAGESZ;
  struct vm_rg_struct *area = get_vm_area_node_at_brk(caller, vmaid, inc_sz, inc_amt);
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

  int old_end = cur_vma->vm_end;

  if(area == NULL) {
    free(newrg); //free new region if area is null to prevent memory leak
    return -1; //check if area is null
  }

  /*Validate overlap of obtained region */
  if (validate_overlap_vm_area(caller, vmaid, area->rg_start, area->rg_end) < 0){
    free(newrg); //free new region if overlap to prevent memory leak
    free(area); //free area if overlap to prevent memory leak
    return -1; /*Overlap and failed allocation */
  }

  /* TODO: Obtain the new vm area based on vmaid */
  //cur_vma->vm_end +=  inc_sz; //increase the virtual memory area limit by inc_sz //cai nay chua allign nen co the bi sai
  /* 
  Example:
  Page size = 256B, inc_sz = 300B.
  inc_amt = 512B (next multiple of 256B).
  vm_end must grow by 512B (not 300B) to ensure alignment.
  Otherwise, the last 44B (300B - 256B) would overlap into the next page, causing corruption. MO HAM ALIGN LEN DOC
  */
  cur_vma->vm_end = old_end + inc_amt;  // use inc_amt (khi da allign) to increase. ensures vm_end always lands on a page boundary.
  // inc_limit_ret...
  if (vm_map_ram(caller, area->rg_start, area->rg_end, 
                    old_end, incnumpage , newrg) < 0)
    return -1; /* Map the memory to MEMRAM */

  return 0;
}

// #endif

#ifndef OSMM_H
#define OSMM_H


#define MM_PAGING
#define PAGING_MAX_MMSWP 4 /* max number of supported swapped space */
#define PAGING_MAX_SYMTBL_SZ 30

typedef char BYTE;
typedef uint32_t addr_t;
//typedef unsigned int uint32_t;

struct pgn_t{
   int pgn;
   struct pgn_t *pg_next; 
};

/*
 *  Memory region struct
 */
struct vm_rg_struct { //mem region, vùng nhỏ hơn trong area
   unsigned long vmaid;
   unsigned long rg_start;
   unsigned long rg_end;

   struct vm_rg_struct *rg_next;
};

/*
 *  Memory area struct
 */
struct vm_area_struct { //là area của 1 vùng nhớ trong process heap,stack,data... đc cấu tạo từ nhiều region
   unsigned long vm_id;
   unsigned long vm_start;
   unsigned long vm_end;

   unsigned long sbrk;  //cấp phát động, chỉ xài cho heap, khi sbrk chạm vm_end thì ko tiếp đc
/*
 * Derived field
 * unsigned long vm_limit = vm_end - vm_start
 */
   struct mm_struct *vm_mm;
   struct vm_rg_struct *vm_freerg_list;  //những region còn trống trong area này, ko liên quan swap
   struct vm_area_struct *vm_next;
};

/* 
 * Memory management struct
 */
struct mm_struct {
   uint32_t *pgd;

   struct vm_area_struct *mmap;

   /* Currently we support a fixed number of symbol */
   struct vm_rg_struct symrgtbl[PAGING_MAX_SYMTBL_SZ];

   /* list of free page */
   struct pgn_t *fifo_pgn;
};

/*
 * FRAME/MEM PHY struct
 */
struct framephy_struct { 
   int fpn;
   struct framephy_struct *fp_next;

   /* Resereed for tracking allocated framed */
   struct mm_struct* owner;
};

struct memphy_struct {
   /* Basic field of data and size */
   BYTE *storage;
   int maxsz;
   
   /* Sequential device fields */ 
   int rdmflg;
   int cursor;

   /* Management structure */
   struct framephy_struct *free_fp_list; //những frame free, sử dụng cho swapping (đang xài page swapping)
   struct framephy_struct *used_fp_list;
};

#endif

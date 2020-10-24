/// Copyright (C) strawberryhacker 

#ifndef MM_H
#define MM_H

#include <cinnamon/types.h>
#include <cinnamon/list.h>

#define DDR_SIZE 134217728
#define DDR_PAGES 32768

#define KERNEL_START 0x80000000
#define KERNEL_OFFSET 0x60000000

static inline void* _pa(void* va)
{
    u8* va_ptr = (u8 *)va;
    va_ptr -= KERNEL_OFFSET;
    return va_ptr;
}

static inline void* _va(void* pa)
{
    u8* pa_ptr = (u8 *)pa;
    pa_ptr += KERNEL_OFFSET;
    return pa_ptr;
}

/// Returns the kernel viertual base address for the page array continaing a 
/// struct page for every physical page
struct page* mm_get_page_array(void);

/// Main page descriptor. Keep this short
struct page {
    u32* mem_map;
    u32 order;
    struct list_node node;
};

void* page_to_va(struct page* page);
void* page_to_pa(struct page* page);
struct page* va_to_page(void* page_addr);
struct page* pa_to_page(void* page_addr);

// Used together with the allocators private data to start an allocator 
struct mm_zone {
    struct page* start;
    u32 page_cnt;

    // Points to the allocator which is used 
    void* alloc;

    u32 (*get_used)(struct mm_zone* zone);
    u32 (*get_free)(struct mm_zone* zone);
    u32 (*get_total)(struct mm_zone* zone);
};

/// For later 
struct mm {
    struct list_node zones;
};

void mm_init(void);

/// Allocation of page tables
struct page* lv1_pt_alloc(void);
struct page* lv2_pt_alloc(void);
void lv1_pt_free(struct page* page);
void lv2_pt_free(struct page* page);


/// Sets the translation table base address 0
static inline void set_ttbr0(u32 paddr)
{
    asm volatile ("mcr p15, 0, %0, c2, c0, 0" : : "r" (paddr));
    asm volatile ("isb" : : : "memory");
}

/// Sets the translation table base address 1
static inline void set_ttbr1(u32 paddr)
{
    asm volatile ("mcr p15, 0, %0, c2, c0, 1" : : "r" (paddr));
}

static inline u32 get_ttbr0(void)
{
    u32 paddr;
    asm volatile ("mrc p15, 0, %0, c2, c0, 0" : "=r" (paddr));
    return paddr;
}

static inline u32 get_ttbr1(void)
{
    u32 paddr;
    asm volatile ("mrc p15, 0, %0, c2, c0, 1" : "=r" (paddr));
    return paddr;
}

/// TLB maintenance
static inline void mm_tlb_invalidate(void)
{
    asm volatile ("mcr p15, 0, r0, c8, c7, 0");
    asm volatile ("dmb" : : : "memory");
    asm volatile ("isb" : : : "memory");
}


/// Bitmap for the secondary level page table
struct pt2 {
    u32 bitmap;
};

/// Every new process allocated a struct proc_mm from the kernel malloc. This 
/// will hold the memory space used by that process. Every child thread will 
/// keep a point to this structure
struct mm_process {
    // Base address of the physical address space going into TTBR0 (must be first)
    u32* ttbr_phys;

    // Virtual addresses
    u32* data_s;
    u32* data_e;

    u32* heap_s;
    u32* heap_e;

    u32* stack_s;
    u32* stack_e;

    // List to keep track of all the pages alloated
    struct list_node page_list;
    u32 page_cnt;

    struct page* pt2_ptr;
};

void lv2_pt_init(struct page* page);
u32* lv2_pt_find_in_page(struct page* page);


/// Adds a new allocated page to the memory manager. This will be palces in a
/// linked list, so that we can kill a process and free the memory
static inline void mm_process_add_page(struct page* page, struct mm_process* mm)
{
    list_add_first(&page->node, &mm->page_list);
}

/// Initialized the struct process_mm structure
void mm_process_init(struct mm_process* mm);


/// Page table operations 
static inline u32 pte_is_empty(u32 pte)
{
    return ((pte & 0b11) == 0) ? 1 : 0;
}

void lv1_pt_map_in_lv2_pt(u32* ttbr_paddr, u32* pt2_vaddr, u32 paddrm, u8 domain);

u32 mm_process_map_memory(struct mm_process* mm, struct page* page,
    u32 page_cnt, u32 vaddr, u32 flags, u8 domain);

u32* set_break(u32 bytes);

/// Functions for gettting the total amount of allocated memory
u32 mm_get_total_used(void);
u32 mm_get_total(void);

#endif
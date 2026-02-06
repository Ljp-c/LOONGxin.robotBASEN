/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */

/**
 * memory manager posix api implement
 */

#include "bsp.h"

#if !BSP_USE_FS

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#if defined(OS_RTTHREAD)
#include "rtthread.h"
#elif defined(OS_FREERTOS)
#include "FreeRTOS.h"
#else // uCOSIII && OS_PESUDO
#include "osal.h"
#endif

#if defined(OS_RTTHREAD)

//-----------------------------------------------------------------------------
// RTThread
//-----------------------------------------------------------------------------

void *malloc(size_t size)
{
    return rt_malloc(size);
}

void *calloc(size_t nmemb, size_t size)
{
    return rt_calloc(nmemb, size);
}

void free(void *ptr)
{
    rt_free(ptr);
}

#elif defined(OS_FREERTOS)

//-----------------------------------------------------------------------------
// FreeRTOS
//-----------------------------------------------------------------------------

#include "FreeRTOS.h"

void *malloc(size_t size)
{
    return pvPortMalloc(size);
}

void *calloc(size_t nmemb, size_t size)
{
    void *ptr = pvPortMalloc(nmemb * size);

    if (ptr)
    {
        memset(ptr, 0x00, nmemb * size);
    }

    return ptr;
}

void free(void *ptr)
{
    vPortFree(ptr);
}

#else

//-----------------------------------------------------------------------------
// BareMetal && uCOSIII
//-----------------------------------------------------------------------------

#include "osal.h"

extern void printk(const char *fmt, ...);

//-----------------------------------------------------------------------------

#ifndef align_up
#define align_up(num, align)    (((num) + ((align)-1)) & ~((align)-1))
#endif

/**
 * memory block node struct
 */
typedef struct blk_node
{
    unsigned int     flag;                  /* =1: used; =0: blank */
	unsigned int     size;
	void            *block;
    struct blk_node *prev;
    struct blk_node *next;
} blk_node_t;

#define BLOCK_NODE_SZ       (align_up(sizeof(blk_node_t), 8))

#define BLOCK_USED_FLAG     (0xdeadbeaf)

#define ALLOC_ALIGNMENT     (sizeof(void *))    /* malloc buffer align 8 bytes */

#define ALLOC_MIN_BYTES     32                  /* min malloc size */

//-----------------------------------------------------------------------------

#define INFO_NODE(node, i) { \
    printk("  Faulty block[%i] @0x%016lx\r\n", i, (long)node); \
    printk("    flag  = 0x%08x\r\n", node->flag); \
    printk("    size  = 0x%08x\r\n", node->size); \
    printk("    block = 0x%016lx\r\n", (long)node->block); \
    printk("    prev  = 0x%016lx\r\n", (long)node->prev);  \
    printk("    next  = 0x%016lx\r\n", (long)node->next); }

/*
 * lock alloc with mutex support
 */
static osal_mutex_t p_alloc_mutex = NULL;

//-----------------------------------------------------------------------------
// free memory queue head
//-----------------------------------------------------------------------------

static blk_node_t *heap_list_head = NULL;

static size_t heap_total_bytes  = 0;
static size_t heap_remain_bytes = 0;

//-----------------------------------------------------------------------------

int heap_verify_faulty_blocks(void)
{
    blk_node_t *node = heap_list_head, *prev, *next;
    int count = 0, i = 0;

    printk("Verify heap blocks:\r\n");

    while (node)
    {
        prev = node->prev;
        next = node->next;

        /*
         * Self is correct
         */
        if ((size_t)node + BLOCK_NODE_SZ != (size_t)node->block)
        {
            count++;
            if (osal_is_osrunning())
            {
                INFO_NODE(node, i);
            }
            break;
        }

        /*
         * Previous is correct
         */
        if (prev && ((size_t)prev + BLOCK_NODE_SZ + prev->size != (size_t)node))
        {
            count++;
            if (osal_is_osrunning())
            {
                INFO_NODE(node, i);
            }
            break;
        }

        /*
         * Next is correct
         */
        if (next && ((size_t)node + BLOCK_NODE_SZ + node->size != (size_t)next))
        {
            count++;
            if (osal_is_osrunning())
            {
                INFO_NODE(node, i);
            }
            break;
        }

        i++;
        node = node->next;
    }

    if (count == 0)
    {
        printk("Not found any fault block\r\n");
    }

    return -count;
}

int heap_view_isolated_blocks(void)
{
    blk_node_t *node = heap_list_head;
    int count = 0;

    printk("Seek isolated blocks:\r\n");

    while (node)
    {
        if (node->prev && node->next && (node->flag == 0))
        {
            count++;
            printk("  block @0x%016lx, size = %iB\r\n", (long)node, node->size);
        }

        node = node->next;
    }

    printk("Total %i isolated blocks\r\n", count);

    return count;
}

//-----------------------------------------------------------------------------
// OS lock
//-----------------------------------------------------------------------------

void malloc_create_oslock(void)
{
    if (!p_alloc_mutex)
    {
        p_alloc_mutex = osal_mutex_create("heap_loc", OSAL_OPT_FIFO);
    }
}

static void malloc_oslock()
{
    if (osal_is_osrunning())
    {
        osal_mutex_obtain(p_alloc_mutex, OSAL_WAIT_FOREVER);
    }
}

static void malloc_osunlock()
{
    if (osal_is_osrunning())
    {
        osal_mutex_release(p_alloc_mutex);
    }
}

//-----------------------------------------------------------------------------
// add heap addrss & size. Only can add once
//-----------------------------------------------------------------------------

int heap_add_region(void *addr, size_t size)
{
    unsigned char *first_addr;

    if (!addr || (size < 0x100000) || heap_list_head)
    {
        return -1;
    }

    first_addr = (unsigned char *)align_up((size_t)addr, ALLOC_ALIGNMENT);

    heap_list_head = (blk_node_t *)first_addr;
    heap_list_head->block = (void *)(first_addr + BLOCK_NODE_SZ);

    size &= ~(ALLOC_ALIGNMENT - 1);
    heap_total_bytes = size - BLOCK_NODE_SZ;
    heap_remain_bytes = heap_total_bytes;

    heap_list_head->size = heap_total_bytes;
    heap_list_head->flag = 0;
    heap_list_head->prev = NULL;
    heap_list_head->next = NULL;

    return 0;
}

//-----------------------------------------------------------------------------

void dump_heap_list(void)
{
    int i = 0;
    blk_node_t *node = heap_list_head;

    while (node)
    {
        INFO_NODE(node, i);

        i++;
        node = node->next;
    }
}

size_t get_heap_size(void)
{
    return heap_total_bytes;
}

size_t get_heap_free_size(void)
{
    return heap_remain_bytes;
}

//-----------------------------------------------------------------------------
// malloc() function
//-----------------------------------------------------------------------------

void *malloc(size_t size)
{
	blk_node_t *found_node = heap_list_head, *new_node = NULL;

	if (size <= 0)
    {
        return NULL;
    }

    if (size <= ALLOC_MIN_BYTES)
    {
        size = ALLOC_MIN_BYTES;
    }
    else
    {
        size = align_up(size, ALLOC_ALIGNMENT);
    }

    malloc_oslock();

    /*
     * search a free block, Only match the first. TODO optimal match
     */
    while (found_node)
    {
        if ((found_node->flag == 0) && (found_node->size >= size + BLOCK_NODE_SZ))
        {
            break;
        }

        found_node = found_node->next;
    }

    if (!found_node)
    {
        malloc_osunlock();
        return NULL;
    }

    /*
     * if found_block's remain size less than ALLOC_MIN_BYTES + BLOCK_NODE_SZ,
     * then use found_block directly.
     */
    if ((found_node->size - (size + BLOCK_NODE_SZ)) < (ALLOC_MIN_BYTES + BLOCK_NODE_SZ))
    {
        found_node->flag = BLOCK_USED_FLAG;

        heap_remain_bytes -= found_node->size + BLOCK_NODE_SZ;
        malloc_osunlock();
        return found_node->block;
    }

	/*
     * found block:
     *   ------------------------------------------------------------------
     *  |H| block, size >= size + NODE_HEAD_SZ                             |
     *   ------------------------------------------------------------------
     *
     * after split:
     *   ---------------|--------------------------------------------------
     *  | found node    | new node                                         |
     *  |H| block       |H| block                                          |
     *   ---------------|--------------------------------------------------
     *
     * memory physical address: low -→high
     *
     */

	new_node = (blk_node_t *)((size_t)found_node->block + size);
	new_node->flag  = 0;
	new_node->block = (unsigned char *)new_node + BLOCK_NODE_SZ;
	new_node->size  = found_node->size - size - BLOCK_NODE_SZ;

    found_node->flag = BLOCK_USED_FLAG;
    found_node->size = size;

	/*
     * list:
     *
     *                   found_node           new_node
     *               -→ ----------       -→ ----------       -→... next
     *             ↗ _ | prev     |    ↗ _ | prev     |    ↗ _     node
     *           ↗ ↙  |          |  ↗ ↙  |          |  ↗ ↙
     * prev ...↗ ↙    |     next |↗ ↙    |     next |↗ ↙
     * node ... ↙      |          |.↙      |          |.↙
     *                  |          |         |          |
     *                   ----------           ----------
     *
     * new_block has free memory to alloc more
     */

    new_node->prev = found_node;
    new_node->next = found_node->next;
    if (found_node->next)
        found_node->next->prev = new_node;

    found_node->next = new_node;

    heap_remain_bytes -= size + BLOCK_NODE_SZ;
	malloc_osunlock();

	return found_node->block;
}

//-----------------------------------------------------------------------------
// free() function
//-----------------------------------------------------------------------------

void free(void *ptr)
{
    blk_node_t *found_node = heap_list_head, *prev, *next;

    if (!ptr)
    {
        return;
    }

    /*
     * search for match block
     */
    while (found_node)
    {
        if ((found_node->flag == BLOCK_USED_FLAG) && (found_node->block == ptr))
            break;

        found_node = found_node->next;
    }

    if (!found_node)
    {
        if (osal_is_osrunning())
        {
            printk("fatal error: free memory @0x%016lx\r\n", (long)ptr);
        }
        return;
    }

	/*
     * list:
     *
     *               prev node            found node           next node
     *           -→ ----------       -→ ----------       -→ ----------       -→
     *         ↗ _ | prev     |    ↗ _ | prev     |    ↗ _ | prev     |    ↗ _ ...
     *       ↗ ↙  |          |  ↗ ↙  |          |  ↗ ↙  |          |  ↗ ↙
     *     ↗ ↙    |     next |↗ ↙    |     next |↗ ↙    |     next |↗ ↙
     *  ... ↙      |          |.↙      |          |.↙      |          |.↙
     *              |          |         |          |         |          |
     *               ----------           ----------           ----------
     *
     */

	/*
     * memory physical address: low -→high
     *
     *   ----------------|---------------|---------------------------------
     *  | prev node      | found node    | next node                       |
     *  |H| block        |H| block       |H| block                         |
     *   ----------------|---------------|---------------------------------
     *
     */

	malloc_oslock();

    heap_remain_bytes += found_node->size + BLOCK_NODE_SZ;
    found_node->flag = 0;

    prev = found_node->prev;
    next = found_node->next;

    /*
     * Combine previous block node
     */
    while (prev && (prev->flag == 0))
    {
        prev->size += found_node->size + BLOCK_NODE_SZ;
        prev->next = found_node->next;
        if (found_node->next)
            found_node->next->prev = prev;

        found_node = prev;
        prev = found_node->prev;
    }

    /*
     * Combine next block node
     */
    while (next && (next->flag == 0))
    {
        found_node->size += next->size + BLOCK_NODE_SZ;
        next = next->next;
        found_node->next = next;
        if (next)
            next->prev = found_node;
    }

    malloc_osunlock();
}

//-----------------------------------------------------------------------------
// calloc() function
//-----------------------------------------------------------------------------

void *calloc(size_t nmemb, size_t size)
{
    void *ptr = malloc(nmemb * size);

    if (ptr)
    {
        memset(ptr, 0x00, nmemb * size);
    }

    return ptr;
}

#endif // OS Functions

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// realloc() function
//-----------------------------------------------------------------------------

#ifdef OS_RTTHREAD

void *realloc(void *ptr, size_t size)
{
    return rt_realloc(ptr, size);
}

void *aligned_malloc(size_t size, unsigned int align)
{
    return rt_malloc_align((rt_size_t)size, (rt_size_t)align);
}

void aligned_free(void *addr)
{
    rt_free_align(addr);
}

#else

void *realloc(void *ptr, size_t size)
{
    if (size <= 0)
    {
        return NULL;
    }

    void *newptr = malloc(size);
    
    if (newptr)
    {
        if (ptr)
        {
            memcpy(newptr, ptr, size);
            free(ptr);
        }

        return newptr;
    }

    if (ptr)
    {
        free(ptr);      // 如果没有申请到内存, 是否释放?
    }

    return NULL;
}

//-----------------------------------------------------------------------------
// aligned_malloc() function
//-----------------------------------------------------------------------------

void *aligned_malloc(size_t size, unsigned int align)
{
    void *head;
    void **addr=NULL;

    if ((size <= 0) || (align == 0))
    {
        return NULL;
    }

    align = (align + 7) & ~0x7;     // atleast aligned 8

    head = (void *)malloc(size + align - 1 + sizeof(void *));

    if (head == NULL)
    {
        return NULL;
    }

    size_t i = (size_t)head + sizeof(void *);

    while (i < (size_t)head + sizeof(void *) + align - 1)
    {
        if  (i % align == 0)
        {
            addr = (void **)i;
            break;
        }

        i++;
    }

    if (addr)
    {
        addr[-1] = head;
    }

    return addr;
}

//-----------------------------------------------------------------------------
// aligned_free() function
//-----------------------------------------------------------------------------

void aligned_free(void *addr)
{
    if (addr)
    {
        void *ptr = ((void **)addr)[-1];
        free(ptr);
    }
}

#endif // #ifdef OS_RTTHREAD

//-----------------------------------------------------------------------------
// aligned_realloc() function
//-----------------------------------------------------------------------------

void *aligned_realloc(void *ptr, size_t size, unsigned int align)
{
    if (size <= 0)
    {
        return NULL;
    }

    void *newptr = aligned_malloc(size, align);

    if (newptr)
    {
        if (ptr)
        {
            memcpy(newptr, ptr, size);

            aligned_free(ptr);
        }

        return newptr;
    }

    if (ptr)
    {
        aligned_free(ptr);      // 如果没有申请到内存, 是否释放?
    }

    return NULL;
}

//-----------------------------------------------------------------------------

#endif // #if !BSP_USE_FS

/*
 * @@ END
 */

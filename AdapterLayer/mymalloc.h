#ifndef __MYMALLOC_H
#define __MYMALLOC_H

//Author: PYL
//Create Date: 2020/4/25
//Usage: To do memory allocation by user defined mem pool
#ifdef __cplusplus
 extern "C" {
#endif

#ifndef NULL
#define NULL 0
#endif

#define MEM_BLOCK_SIZE 32   //mem block size 32B
#define MAX_MEM_SIZE 4 * 1024 * 1024    //max pool size 7MB(the extern SDRAM size of STM32F769, but exclude the LCD frame buffer)
#define MEM_ALLOC_TABLE_SIZE MAX_MEM_SIZE / MEM_BLOCK_SIZE    //mem table size

#include <stdint.h>

#define SDRAM_WRITE_READ_ADDR  ((uint32_t)0xC0177000)
// __attribute__((section(".ARM.__at_0x20008000")))
//mem manage controller
struct _m_malloc_dev
{
 void (*init)(void);     //init
 uint8_t (*perused)(void);         //mem use persentage
 uint8_t* membase;   //mem pool
 uint16_t memmap[MEM_ALLOC_TABLE_SIZE];  //mem manage table
 uint8_t  memrdy;        //ready or not
};
extern struct _m_malloc_dev malloc_dev;  //defined in mymalloc.c

void mymemset(void *s,uint8_t c,uint32_t count);  //set mem
void mymemcpy(void *des,void *src,uint32_t n);  //copy mem

void mymem_init(void);    //mem manager init
uint32_t mymem_malloc(uint32_t size); //mem alloc
uint8_t mymem_free(uint32_t offset);  //mem release
uint8_t mymem_perused(void);  //get mem usage

////////////////////////////////////////////////////////////////////////////////
//User call funcs
void myfree(void *ptr);
void *mymalloc(uint32_t size);
void *myrealloc(void *ptr,uint32_t size);

#ifdef __cplusplus
}
#endif

#endif


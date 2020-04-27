#include "mymalloc.h"

//Author: PYL
//Create Date: 2020/4/25
//Usage: To do memory allocation by user defined mem pool

//内存管理控制器
struct _m_malloc_dev malloc_dev = 
{
    .init = mymem_init, //内存初始化
    .perused = mymem_perused,//内存使用率
    .membase = (uint8_t*) SDRAM_WRITE_READ_ADDR,
    .memmap = 0,   //内存管理状态表
    .memrdy = 0,     //内存管理未就绪
};

void mymemcpy(void *des, void *src, uint32_t n)  
{  
    uint8_t *xdes = des;
    uint8_t *xsrc = src; 
    while(n--) *xdes++ = *xsrc++;  
}

void mymemset(void *s, uint8_t c, uint32_t count)  
{  
    uint8_t *xs = s;
    while(count--) *xs++ = c;  
}

void mymem_init(void)  
{  
    mymemset(malloc_dev.memmap, 0, MEM_ALLOC_TABLE_SIZE); //sizeof(malloc_dev.membase));//clear memtable but NOT physical mem
    malloc_dev.memrdy = 1;
}

uint8_t mymem_perused(void)  
{  
    uint16_t used = 0;
    uint32_t i;  
    for(i = 0; i < MEM_ALLOC_TABLE_SIZE; i++)  
    {  
        if(malloc_dev.memmap[i]) used++;
    }
    return used * 100 / MEM_ALLOC_TABLE_SIZE;  
}

uint32_t mymem_malloc(uint32_t size)  
{  
    signed long offset = 0;  
    uint16_t nmemb; //需要的内存块数  
    uint16_t cmemb = 0;//连续空内存块数
    uint32_t i;  
    if(!malloc_dev.memrdy)  malloc_dev.init();//未初始化,先执行初始化 
    if(size == 0) return 0xFFFFFFFF;//不需要分配

    nmemb = size / MEM_BLOCK_SIZE;   //获取需要分配的连续内存块数
    if(size % MEM_BLOCK_SIZE)   nmemb++;  
    for(offset = MEM_ALLOC_TABLE_SIZE - 1; offset >= 0; offset--)//搜索整个内存控制区  
    //for(offset = 0; offset < MEM_ALLOC_TABLE_SIZE; offset++)//搜索整个内存控制区  
    {
        if(!malloc_dev.memmap[offset])  cmemb++; //连续空内存块数增加
        else cmemb = 0;       //连续内存块清零
        if(cmemb == nmemb)      //找到了连续nmemb个空内存块
        {
            for(i = 0; i < nmemb; i++)      //标注内存块非空 
            {  
                malloc_dev.memmap[offset + i] = nmemb;  
            }  
            return (offset * MEM_BLOCK_SIZE);//返回偏移地址
        }
    }
    return 0xFFFFFFFF;//未找到符合分配条件的内存块  
}

uint8_t mymem_free(uint32_t offset)  
{  
    int i;  
    if(!malloc_dev.memrdy)//未初始化,先执行初始化
    {
        malloc_dev.init();
        return 1;//未初始化  
    }
    if(offset < MAX_MEM_SIZE)//偏移在内存池内. 
    {  
        int index = offset / MEM_BLOCK_SIZE;//偏移所在内存块号码  
        int nmemb = malloc_dev.memmap[index];   //内存块数量
        for(i = 0; i < nmemb; i++)     //内存块清零
        {  
            malloc_dev.memmap[index + i] = 0;  
        }  
        return 0;  
    }
    else return 2;//偏移超区了.  
}

void myfree(void *ptr)  
{  
    uint32_t offset;  
    if(ptr == NULL) return;//地址为0.  
    offset = (uint32_t)ptr - (uint32_t)malloc_dev.membase;
    mymem_free(offset);
}

void *mymalloc(uint32_t size)  
{  
    uint32_t offset;  
    offset = mymem_malloc(size);  
    if(offset == 0xFFFFFFFF) return NULL;  
    else return (void*)((uint32_t)malloc_dev.membase + offset);// + offset);  
}

void *myrealloc(void *ptr,uint32_t size)  
{  
    uint32_t offset;  
    offset = mymem_malloc(size);  
    if(offset == 0xFFFFFFFF) return NULL;     
    else
    {  
        mymemcpy((void*)((uint32_t)malloc_dev.membase + offset), ptr, size);//拷贝旧内存内容到新内存   
        myfree(ptr);               //释放旧内存
        return (void*)((uint32_t)malloc_dev.membase + offset);          //返回新内存首地址
    }
}
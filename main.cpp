#include "mbed.h"
#include "stm32f769i_discovery.h"
#include "stm32f769i_discovery_lcd.h"
#include "stm32f769i_discovery_audio.h"
#include "stm32f769i_discovery_sdram.h"
#include <stdlib.h>

static void CopyBuffer(int16_t *pbuffer1, int16_t *pbuffer2, uint16_t BufferSize);
 
#define SCRATCH_BUFF_SIZE  1024
#define RECORD_BUFFER_SIZE  4096

#define BUFFER_SIZE            ((uint32_t)0xffff)
#define WRITE_READ_ADDR        ((uint32_t)0x0800)
#define SDRAM_WRITE_READ_ADDR  ((uint32_t)0xC0177000)
 
typedef enum {
    BUFFER_OFFSET_NONE = 0,
    BUFFER_OFFSET_HALF = 1,
    BUFFER_OFFSET_FULL = 2,
} BUFFER_StateTypeDef;
 
volatile uint32_t  audio_rec_buffer_state = BUFFER_OFFSET_NONE;
int32_t Scratch[SCRATCH_BUFF_SIZE];
 
/* Buffer containing the PCM samples coming from the microphone */
int16_t RecordBuffer[RECORD_BUFFER_SIZE];
 
/* Buffer used to stream the recorded PCM samples towards the audio codec. */
int16_t PlaybackBuffer[RECORD_BUFFER_SIZE];
 
int main()
{

    printf("\n\n SDRAM EXAMPLE FOR DISCO-F769NI START:\n");
    /* Init LCD and display example information */
    BSP_LCD_Init();
    BSP_LCD_LayerDefaultInit(0, LCD_FB_START_ADDRESS);
    BSP_LCD_Clear(LCD_COLOR_BLACK);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_FillRect(0, 0, BSP_LCD_GetXSize(), 40);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
    BSP_LCD_SetFont(&Font24);
    BSP_LCD_DisplayStringAt(0, 0, (uint8_t *)"SDRAM basic example", CENTER_MODE);

    HAL_Delay(2000);

    // /* SDRAM device configuration */
    // if (BSP_SDRAM_Init() != SDRAM_OK) {
    //     BSP_LCD_DisplayStringAt(20, 100, (uint8_t *)"SDRAM Initialization : FAILED", LEFT_MODE);
    // } else {
    //     BSP_LCD_DisplayStringAt(20, 100, (uint8_t *)"SDRAM Initialization : OK", LEFT_MODE);
    // }
    int16_t* test = (int16_t*) malloc(sizeof(int16_t)*10240);

    printf("test addr is %p\n", test);

    
    uint32_t audio_loop_back_init = RESET ;
 
    printf("\n\nAUDIO LOOPBACK EXAMPLE FOR DISCO-F769NI START:\n");
    
    /* Initialize Audio Recorder with 4 channels to be used */
    if (BSP_AUDIO_IN_Init(BSP_AUDIO_FREQUENCY_11K, DEFAULT_AUDIO_IN_BIT_RESOLUTION, DEFAULT_AUDIO_IN_CHANNEL_NBR) == AUDIO_ERROR) {
        printf("BSP_AUDIO_IN_Init error\n");
    }
 
    /* Allocate scratch buffers */
    if (BSP_AUDIO_IN_AllocScratch (Scratch, SCRATCH_BUFF_SIZE) == AUDIO_ERROR) {
        printf("BSP_AUDIO_IN_AllocScratch error\n");
    }
 
    /* Start Recording */
    if (BSP_AUDIO_IN_Record((uint16_t*)&RecordBuffer[0], RECORD_BUFFER_SIZE) == AUDIO_ERROR) {
        printf("BSP_AUDIO_IN_Record error\n");
    }
    uint8_t ChannelNumber = BSP_AUDIO_IN_GetChannelNumber();
    int16_t* sdram = (int16_t*)0xC0177000;

    int16_t* p = sdram;

    audio_rec_buffer_state = BUFFER_OFFSET_NONE;
    int cnt = 200;
    while (cnt > 0) {
        /* 1st or 2nd half of the record buffer ready for being copied to the playbakc buffer */
        if(audio_rec_buffer_state != BUFFER_OFFSET_NONE) {
            /* Copy half of the record buffer to the playback buffer */
            if(audio_rec_buffer_state == BUFFER_OFFSET_HALF) {
                CopyBuffer(&PlaybackBuffer[0], &RecordBuffer[0], RECORD_BUFFER_SIZE/2);
                CopyBuffer(p, &RecordBuffer[0], RECORD_BUFFER_SIZE/2);
                if (audio_loop_back_init == RESET) {
                    /* Initialize the audio device*/
                    if (BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_HEADPHONE, 70, BSP_AUDIO_FREQUENCY_11K) == AUDIO_ERROR) {
                        printf("BSP_AUDIO_OUT_Init error\n");
                    }
                    if(ChannelNumber > 2) {
                        BSP_AUDIO_OUT_SetAudioFrameSlot(CODEC_AUDIOFRAME_SLOT_0123);
                    } else {
                        BSP_AUDIO_OUT_SetAudioFrameSlot(CODEC_AUDIOFRAME_SLOT_02);
                    }
 
                    /* Play the recorded buffer */
                    if (BSP_AUDIO_OUT_Play((uint16_t *) &PlaybackBuffer[0], 2*RECORD_BUFFER_SIZE) == AUDIO_ERROR) {
                        printf("BSP_AUDIO_OUT_Play error\n");
                    }
                    /* Audio device is initialized only once */
                    audio_loop_back_init = SET;
                }
 
 
            } else { /* if(audio_rec_buffer_state == BUFFER_OFFSET_FULL)*/
                CopyBuffer(&PlaybackBuffer[RECORD_BUFFER_SIZE/2], &RecordBuffer[RECORD_BUFFER_SIZE/2], RECORD_BUFFER_SIZE/2);
                CopyBuffer(p + RECORD_BUFFER_SIZE/2, &RecordBuffer[RECORD_BUFFER_SIZE/2], RECORD_BUFFER_SIZE/2);
            }
 
            /* Wait for next data */
            //for(int i = 0; i < RECORD_BUFFER_SIZE; ++i)
            cnt--;
            p += RECORD_BUFFER_SIZE;
            audio_rec_buffer_state = BUFFER_OFFSET_NONE;
        }
        
    }
    BSP_AUDIO_OUT_Stop(CODEC_PDWN_HW);
    printf("record finished %p %p\n", p, sdram);
    // for(int i = 0; i < 9632000; ++i, ++sdram)
    //     printf("%d\n", *sdram);

    //idle
    while(1)
    {

    }
}
 
/*-------------------------------------------------------------------------------------
       Callbacks implementation:
           the callbacks API are defined __weak in the stm32f769i_discovery_audio.c file
           and their implementation should be done in the user code if they are needed.
           Below some examples of callback implementations.
  -------------------------------------------------------------------------------------*/
/**
  * @brief Manages the DMA Transfer complete interrupt.
  * @param None
  * @retval None
  */
void BSP_AUDIO_IN_TransferComplete_CallBack(void)
{
    audio_rec_buffer_state = BUFFER_OFFSET_FULL;
}
 
/**
  * @brief  Manages the DMA Half Transfer complete interrupt.
  * @param  None
  * @retval None
  */
void BSP_AUDIO_IN_HalfTransfer_CallBack(void)
{
    audio_rec_buffer_state = BUFFER_OFFSET_HALF;
}
 
/**
  * @brief  Audio IN Error callback function.
  * @param  None
  * @retval None
  */
void BSP_AUDIO_IN_Error_CallBack(void)
{
    printf("BSP_AUDIO_IN_Error_CallBack\n");
}
 
 
/**
  * @brief  Copy content of pbuffer2 to pbuffer1
  * @param1 BufferOut
  * @param2 BufferIn
  * @param3 Size
  * @retval None
  */
static void CopyBuffer(int16_t *pbuffer1, int16_t *pbuffer2, uint16_t BufferSize)
{
    uint32_t i = 0;
    for(i = 0; i < BufferSize; i++) {
        pbuffer1[i] = pbuffer2[i];
    }
}

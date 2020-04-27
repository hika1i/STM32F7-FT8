#include "mbed.h"
#include "stm32f769i_discovery.h"
#include "stm32f769i_discovery_lcd.h"
#include "stm32f769i_discovery_audio.h"
#include "stm32f769i_discovery_sdram.h"
#include <stdlib.h>
#include "mymalloc.h"
#include "constants.h"
#include "pack.h"
#include "encode.h"
#include "wave.h"
#include <math.h>

static void FillOutBuffer(int16_t *dstbuffer, int16_t *srcbuffer, uint16_t BufferSize);
 
#define SCRATCH_BUFF_SIZE  1024
#define RECORD_BUFFER_SIZE  512
#define SIZE 0x1fffffff

#define BUFFER_SIZE            ((uint32_t)0xffff)
#define WRITE_READ_ADDR        ((uint32_t)0x0800)
#define SDRAM_WRITE_READ_ADDR  ((uint32_t)0xC0177000)
 
typedef enum {
    BUFFER_OFFSET_NONE = 0,
    BUFFER_OFFSET_HALF = 1,
    BUFFER_OFFSET_FULL = 2,
} BUFFER_StateTypeDef;
 
volatile uint32_t  audio_rec_buffer_state = BUFFER_OFFSET_NONE;
volatile uint32_t  audio_out_buffer_state = BUFFER_OFFSET_NONE;
int32_t Scratch[SCRATCH_BUFF_SIZE];
 
/* Buffer containing the PCM samples coming from the microphone */
int16_t RecordBuffer[RECORD_BUFFER_SIZE];
 
/* Buffer used to stream the recorded PCM samples towards the audio codec. */
int16_t PlaybackBuffer[RECORD_BUFFER_SIZE];
 
int main()
{
    SCB_EnableICache();
    SCB_EnableDCache();

    printf("\n\n FT8 TEST ON DISCO-F769NI START:\n");
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

    HAL_Delay(1000);

    /* SDRAM device configuration */
    if (BSP_SDRAM_Init() != SDRAM_OK)
        BSP_LCD_DisplayStringAt(20, 100, (uint8_t *)"SDRAM Initialization : FAILED", LEFT_MODE);
    else
        BSP_LCD_DisplayStringAt(20, 100, (uint8_t *)"SDRAM Initialization : OK", LEFT_MODE);

    printf("\n\nAUDIO LOOPBACK EXAMPLE FOR DISCO-F769NI START:\n");
    
    //Encode Test
    const char *message = "CQ BA5AG GG77";
    printf("Input messages are: %s\n", message);

    // First, pack the text data into binary message
    uint8_t packed[K_BYTES];
    //int rc = packmsg(message, packed);
    int rc = pack77(message, packed);
    if (rc < 0)
    {
        printf("Cannot parse message!\n");
        printf("RC = %d\n", rc);
        return -2;
    }

    printf("Packed data: ");
    for (int j = 0; j < 10; ++j)
    {
        printf("%02x ", packed[j]);
    }
    printf("\n");

    // Second, encode the binary message as a sequence of FSK tones
    uint8_t tones[NN];          // FT8_NN = 79, lack of better name at the moment

    genft8(packed, tones);

    printf("FSK tones: ");
    for (int j = 0; j < NN; ++j)
    {
        printf("%d", tones[j]);
    }
    printf("\n");

    // Third, convert the FSK tones into an audio signal
    const int sample_rate = 12000;
    const float symbol_rate = 6.25f;
    const int num_samples = (int)(0.5f + NN / symbol_rate * sample_rate);   //151680
    const int num_silence = (15 * sample_rate - num_samples) / 2;   //14160
    float *signal = (float *)0xC0177000;// mymalloc((num_silence + num_samples + num_silence) * sizeof(float));
    for (int i = 0; i < num_silence + num_samples + num_silence; i++)
    {
        signal[i] = 0;
    }
    printf("Signal buffer init OK. Address is %p.\n", signal);

    synth_fsk(tones, NN, 1000, symbol_rate, symbol_rate, sample_rate, &signal[num_silence]);
    int16_t *pcmdata = (int16_t *) 0xC0477000;//mymalloc(180000 * 16 / 8);
    for (int i = 0; i < num_silence + num_samples + num_silence; i++)
    {
        pcmdata[i] = 0;
    }
    wave2pcm(signal, num_silence + num_samples + num_silence, sample_rate, pcmdata);

    printf("Encode OK, start audio output.\n");
    // for(int i = 0; i < 180000; ++i)
    //     if((pcmdata[i] - raw_data[i]) != 0)
    //         printf("%d: %d\n", i, pcmdata[i] - raw_data[i]);
    //Audio out test1
    if (BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_HEADPHONE, 70, BSP_AUDIO_FREQUENCY_12K) == AUDIO_ERROR) 
        printf("BSP_AUDIO_OUT_Init error\n");

    int i = 0, cnt = 0;
    BSP_AUDIO_OUT_SetAudioFrameSlot(CODEC_AUDIOFRAME_SLOT_02);
    audio_out_buffer_state = BUFFER_OFFSET_NONE;
    FillOutBuffer(&PlaybackBuffer[0], (int16_t*)&pcmdata[i], RECORD_BUFFER_SIZE);
    SCB_CleanDCache_by_Addr((uint32_t*)&PlaybackBuffer[0], RECORD_BUFFER_SIZE);
    if (BSP_AUDIO_OUT_Play((uint16_t *) &PlaybackBuffer[0], RECORD_BUFFER_SIZE * 2) == AUDIO_ERROR)
        printf("Play error\n");
    i += RECORD_BUFFER_SIZE >> 2;

    while(i < 180000)
    {
        if(audio_out_buffer_state != BUFFER_OFFSET_NONE)
        {
            if(audio_out_buffer_state == BUFFER_OFFSET_HALF)
                FillOutBuffer(&PlaybackBuffer[0], (int16_t*)&pcmdata[i], RECORD_BUFFER_SIZE >> 1);
            else
            {
                FillOutBuffer(&PlaybackBuffer[RECORD_BUFFER_SIZE >> 1], \
                (int16_t*)&pcmdata[i + (RECORD_BUFFER_SIZE >> 2)], RECORD_BUFFER_SIZE >> 1);
                i += RECORD_BUFFER_SIZE >> 1;
            }
            audio_out_buffer_state = BUFFER_OFFSET_NONE;
        }
    }
    // FillOutBuffer(&PlaybackBuffer[0], (int16_t*)&pcmdata[i], (180000 - i) / 2);
    BSP_AUDIO_OUT_Stop(CODEC_PDWN_HW);

    printf("Play finished\n");

    /* Initialize Audio Recorder with 4 channels to be used */
    // if (BSP_AUDIO_IN_Init(BSP_AUDIO_FREQUENCY_11K, DEFAULT_AUDIO_IN_BIT_RESOLUTION, DEFAULT_AUDIO_IN_CHANNEL_NBR) == AUDIO_ERROR)
    // {
    //     printf("BSP_AUDIO_IN_Init error\n");
    // }
 
    // /* Allocate scratch buffers */
    // if (BSP_AUDIO_IN_AllocScratch (Scratch, SCRATCH_BUFF_SIZE) == AUDIO_ERROR)
    // {
    //     printf("BSP_AUDIO_IN_AllocScratch error\n");
    // }
 
    // /* Start Recording */
    // if (BSP_AUDIO_IN_Record((uint16_t*)&RecordBuffer[0], RECORD_BUFFER_SIZE) == AUDIO_ERROR)
    // {
    //     printf("BSP_AUDIO_IN_Record error\n");
    // }
    // uint8_t ChannelNumber = BSP_AUDIO_IN_GetChannelNumber();

    // audio_rec_buffer_state = BUFFER_OFFSET_NONE;
    // int cnt = 200;
    // while (cnt > 0) {
    //     /* 1st or 2nd half of the record buffer ready for being copied to the playbakc buffer */
    //     if(audio_rec_buffer_state != BUFFER_OFFSET_NONE) {
    //         /* Copy half of the record buffer to the playback buffer */
    //         if(audio_rec_buffer_state == BUFFER_OFFSET_HALF) {
    //             FillOutBuffer(&PlaybackBuffer[0], &RecordBuffer[0], RECORD_BUFFER_SIZE/2);
    //             FillOutBuffer(p, &RecordBuffer[0], RECORD_BUFFER_SIZE/2);
    //             if (audio_loop_back_init == RESET) {
    //                 /* Initialize the audio device*/
    //                 if (BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_HEADPHONE, 70, BSP_AUDIO_FREQUENCY_11K) == AUDIO_ERROR) {
    //                     printf("BSP_AUDIO_OUT_Init error\n");
    //                 }
    //                 if(ChannelNumber > 2) {
    //                     BSP_AUDIO_OUT_SetAudioFrameSlot(CODEC_AUDIOFRAME_SLOT_0123);
    //                 } else {
    //                     BSP_AUDIO_OUT_SetAudioFrameSlot(CODEC_AUDIOFRAME_SLOT_02);
    //                 }
 
    //                 /* Play the recorded buffer */
    //                 if (BSP_AUDIO_OUT_Play((uint16_t *) &PlaybackBuffer[0], 2*RECORD_BUFFER_SIZE) == AUDIO_ERROR) {
    //                     printf("BSP_AUDIO_OUT_Play error\n");
    //                 }
    //                 /* Audio device is initialized only once */
    //                 audio_loop_back_init = SET;
    //             }
 
 
    //         } else { /* if(audio_rec_buffer_state == BUFFER_OFFSET_FULL)*/
    //             FillOutBuffer(&PlaybackBuffer[RECORD_BUFFER_SIZE/2], &RecordBuffer[RECORD_BUFFER_SIZE/2], RECORD_BUFFER_SIZE/2);
    //             FillOutBuffer(p + RECORD_BUFFER_SIZE/2, &RecordBuffer[RECORD_BUFFER_SIZE/2], RECORD_BUFFER_SIZE/2);
    //         }
 
    //         /* Wait for next data */
    //         //for(int i = 0; i < RECORD_BUFFER_SIZE; ++i)
    //         cnt--;
    //         p += RECORD_BUFFER_SIZE;
    //         audio_rec_buffer_state = BUFFER_OFFSET_NONE;
    //     }
        
    // }
    // BSP_AUDIO_OUT_Stop(CODEC_PDWN_HW);
    // printf("record finished %p %p\n", p, sdram);
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
  * @brief  Manages the DMA full Transfer complete event.
  * @retval None
  */
void BSP_AUDIO_OUT_TransferComplete_CallBack(void)
{
    audio_out_buffer_state = BUFFER_OFFSET_FULL;
    // printf("2, ");
}

/**
  * @brief  Manages the DMA Half Transfer complete event.
  * @retval None
  */
void BSP_AUDIO_OUT_HalfTransfer_CallBack(void)
{
    audio_out_buffer_state = BUFFER_OFFSET_HALF;
    // printf("1, ");
}
 
/**
  * @brief  Manages the DMA FIFO error event.
  * @retval None
  */
void BSP_AUDIO_OUT_Error_CallBack(void)
{
    printf("BSP_AUDIO_OUT_Error_CallBack\n");
}

/**
  * @brief  Copy content of pbuffer2 to pbuffer1
  * @param1 BufferOut
  * @param2 BufferIn
  * @param3 Size
  * @retval None
  */
static void FillOutBuffer(int16_t *dstbuffer, int16_t *srcbuffer, uint16_t BufferSize)
{
    uint32_t i = 0;
    for(i = 0; i < BufferSize; i++)
    {
        dstbuffer[i] = srcbuffer[i >> 1]; //To fill two channel using one data flow
    }
}

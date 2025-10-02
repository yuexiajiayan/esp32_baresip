/**
 * @file i2s.c freeRTOS I2S audio driver module
 *
 * Copyright (C) 2019 cspiel.at
 */
#include <sys/types.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include "freertos/FreeRTOS.h"
#include <re.h>
#include <rem.h>
#include <baresip.h>

#include "driver/i2s.h" 
#include "driver/i2s_common.h"   
#include "driver/i2s_std.h"
#include "esp_check.h"
#include "es8311.h"

/**
 * @defgroup i2s i2s
 *
 * I2S audio driver module for freeRTOS (ESP32 Espressif)
 *
 * This module adds an audio source for I2S MEMs microphone (mono/stereo) and
 * an audio player for I2S class D amplifiers. It was tested with:
 *
 * - ESP32-WROOM from Espressif
 * - Sparkfun I2S Audio Breakout - MAX98357A SF14809 - CLASS D stereo amplifier
 * - Adafruit I2S MEMS Microphone Breakout - SPH0645LM4H
 */

/* I2C port and GPIOs */
#define I2C_NUM         (0)
 
#define I2C_SCL_IO      (GPIO_NUM_14)
#define I2C_SDA_IO      (GPIO_NUM_15)
//#define GPIO_OUTPUT_PA  (GPIO_NUM_46)
 
/* I2S port and GPIOs */
#define I2S_NUM         (0) 
#define I2S_MCK_IO      (GPIO_NUM_16)
#define I2S_BCK_IO      (GPIO_NUM_9)
#define I2S_WS_IO       (GPIO_NUM_45)
 
#define I2S_DO_IO       (GPIO_NUM_8)
#define I2S_DI_IO       (GPIO_NUM_10)

#define EXAMPLE_SAMPLE_RATE     (8000)
#define EXAMPLE_MCLK_MULTIPLE   (384) // If not using 24-bit data width, 256 should be enough
#define EXAMPLE_MCLK_FREQ_HZ    (EXAMPLE_SAMPLE_RATE * EXAMPLE_MCLK_MULTIPLE)
#define EXAMPLE_VOICE_VOLUME    80
#define AUDIO_SAMPLES_PER_FRAME        320
#define TAG "TEst i2s"
static struct ausrc *ausrc = NULL;
static struct auplay *auplay = NULL; 


static i2s_chan_handle_t tx_handle = NULL;
static i2s_chan_handle_t rx_handle = NULL;


static esp_err_t es8311_codec_init(void)
{
    /* Initialize I2C peripheral */
#if !defined(CONFIG_EXAMPLE_BSP)
    const i2c_config_t es_i2c_cfg = {
        .sda_io_num = I2C_SDA_IO,
        .scl_io_num = I2C_SCL_IO,
        .mode = I2C_MODE_MASTER,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
	
        .master.clk_speed = 100000,
    };
    ESP_RETURN_ON_ERROR(i2c_param_config(I2C_NUM, &es_i2c_cfg), TAG, "config i2c failed");
    ESP_RETURN_ON_ERROR(i2c_driver_install(I2C_NUM, I2C_MODE_MASTER,  0, 0, 0), TAG, "install i2c driver failed");
#else
    ESP_ERROR_CHECK(bsp_i2c_init());
#endif

    /* Initialize es8311 codec */
    es8311_handle_t es_handle = es8311_create(I2C_NUM, ES8311_ADDRRES_0);
    ESP_RETURN_ON_FALSE(es_handle, ESP_FAIL, TAG, "es8311 create failed");
    const es8311_clock_config_t es_clk = {
        .mclk_inverted = false,
        .sclk_inverted = false,
        .mclk_from_mclk_pin = true,
        .mclk_frequency = EXAMPLE_MCLK_FREQ_HZ,
        .sample_frequency = EXAMPLE_SAMPLE_RATE
    };

    ESP_ERROR_CHECK(es8311_init(es_handle, &es_clk, ES8311_RESOLUTION_16, ES8311_RESOLUTION_16));
    ESP_RETURN_ON_ERROR(es8311_sample_frequency_config(es_handle, EXAMPLE_SAMPLE_RATE * EXAMPLE_MCLK_MULTIPLE, EXAMPLE_SAMPLE_RATE), TAG, "set es8311 sample frequency failed");
    ESP_RETURN_ON_ERROR(es8311_voice_volume_set(es_handle, EXAMPLE_VOICE_VOLUME, NULL), TAG, "set es8311 volume failed");
    ESP_RETURN_ON_ERROR(es8311_microphone_config(es_handle, false), TAG, "set es8311 microphone failed");
     
	
    return ESP_OK;
}

#define I2S_CHANNEL_DEFAULT_CONFIG1(i2s_num, i2s_role) { \
    .id = i2s_num, \
    .role = i2s_role, \
    .dma_desc_num = 6, \
    .dma_frame_num = 1280, \
    .auto_clear_after_cb = false, \
    .auto_clear_before_cb = false, \
    .intr_priority = 0, \
}

static esp_err_t i2s_driver_init(void)
{
#if !defined(CONFIG_EXAMPLE_BSP)
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG1(I2S_NUM, I2S_ROLE_MASTER);
    chan_cfg.auto_clear = true; // Auto clear the legacy data in the DMA buffer
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &tx_handle, &rx_handle));
    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(EXAMPLE_SAMPLE_RATE), 
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO ),
        
        .gpio_cfg = {
            .mclk = I2S_MCK_IO,
            .bclk = I2S_BCK_IO,
            .ws = I2S_WS_IO,
            .dout = I2S_DO_IO,
            .din = I2S_DI_IO, 
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
			
        },
    };
    std_cfg.clk_cfg.mclk_multiple = EXAMPLE_MCLK_MULTIPLE;
 

    ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx_handle, &std_cfg));
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(rx_handle, &std_cfg));
    ESP_ERROR_CHECK(i2s_channel_enable(tx_handle));
    ESP_ERROR_CHECK(i2s_channel_enable(rx_handle));
#else
    ESP_LOGI(TAG, "Using BSP for HW configuration");
    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(EXAMPLE_SAMPLE_RATE),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),
        .gpio_cfg = BSP_I2S_GPIO_CFG,
    };
    std_cfg.clk_cfg.mclk_multiple = EXAMPLE_MCLK_MULTIPLE;
    ESP_ERROR_CHECK(bsp_audio_init(&std_cfg, &tx_handle, &rx_handle));
    ESP_ERROR_CHECK(bsp_audio_poweramp_enable(true));
#endif
    return ESP_OK;
}











struct auplay_st {
	const struct auplay *ap;  /* pointer to base-class (inheritance) */
	pthread_t thread;
	RE_ATOMIC bool run;
	void *sampv;
	size_t sampc;
	auplay_write_h *wh;
	void *arg;
	struct auplay_prm prm;
 
};


static void auplay_destructor(void *arg)
{
	struct auplay_st *st = arg;

	/* Wait for termination of other thread */
	if (re_atomic_rlx(&st->run)) {
		info("i2s: stopping playback thread\n");
		re_atomic_rlx_set(&st->run, false);
		thrd_join(st->thread, NULL);
	}
 

	if(st->sampv)mem_deref(st->sampv);
	//if(st) mem_deref(st);
}


 /**
 * 将 8k 采样率的 buffer 上采样为 16k
 * 输入：in[N]     -> 8kHz
 * 输出：out[2*N]  -> 16kHz
 */
void upsample_8k_to_16k(const int16_t *in, int16_t *out, size_t N) {
    for (size_t i = 0; i < N; i++) {
        out[i * 2] = in[i];                    // 原始样本
        if (i + 1 < N) {
            // 插入中间值：线性插值
            out[i * 2 + 1] = (in[i] + in[i + 1]) / 2;
        } else {
            // 最后一个样本，无法插值，复制或外推
            out[i * 2 + 1] = in[i];
        }
    }
}
 

static void *write_thread(void *arg)
{
	struct auplay_st *st = arg; 
	size_t bytes_write = 0;


	// int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    // if (sock < 0) {
    //    ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
         
    //    return NULL;
    // } 
    // //设置目标地址信息
    // struct sockaddr_in dest_addr;
    // dest_addr.sin_addr.s_addr = inet_addr("192.168.1.6");
    // dest_addr.sin_family = AF_INET;
    // dest_addr.sin_port = htons(4000);


	
	int num_frames = st->prm.srate * st->prm.ptime / 1000; 

	int16_t upsampled_buffer[2 * num_frames]; 
	while (re_atomic_rlx(&st->run)) {
		 struct auframe af;

		auframe_init(&af, st->prm.fmt, st->sampv, st->sampc,  st->prm.srate, 1);

		  st->wh(&af, st->arg);  
		 
		 if ( af.sampc == 0) {
			continue; 
		}
		// memcpy( sampv, af.sampv, st->sampc * sizeof(int16_t)  );
		//ESP_LOGE("write","==%d",num_frames);
		 //if(i2s_write(I2S_PLAY_PORT,  st->sampv , num_frames * sizeof(int16_t) , &bytes_write, portMAX_DELAY) != ESP_OK){
		 	//warning("i2s write failed\n"); 
		 //}
		 
		 upsample_8k_to_16k( st->sampv, upsampled_buffer,  num_frames);
		 i2s_channel_write(tx_handle,  upsampled_buffer , num_frames *2 * sizeof(int16_t) , &bytes_write, portMAX_DELAY);	

		//sendto(sock,  st->sampv , num_frames * sizeof(int16_t)   , 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr));

	} 
	 
	info("i2s: stopped play thread ok\n");
	
	return NULL;
}


int i2s_play_alloc(struct auplay_st **stp, const struct auplay *ap,
		    struct auplay_prm *prm, const char *device,
		    auplay_write_h *wh, void *arg)
{
	struct auplay_st *st;
	int err;

	(void) device;

	if (!stp || !ap || !prm || !wh)
		return EINVAL;

	if (prm->fmt!=AUFMT_S16LE) {
		warning("i2s: unsupported sample format %s\n", aufmt_name(prm->fmt));
		return EINVAL;
	}

	st = mem_zalloc(sizeof(*st), auplay_destructor);
	if (!st)
		return ENOMEM;

	st->prm = *prm;
	st->ap  = ap;
	st->wh  = wh;
	st->arg = arg;

	st->sampc = prm->srate * prm->ch * prm->ptime / 1000;
	st->sampv = mem_zalloc(aufmt_sample_size(st->prm.fmt)*st->sampc, NULL);
	if (!st->sampv) {
		err = ENOMEM;
		goto out;
	}
	info("i2s: play format %d\n",st->sampc );
	//err = i2s_start_bus(st->prm.srate, I2O_PLAY, st->prm.ch);
	//if (err)
	//	goto out;

	re_atomic_rlx_set(&st->run, true);
	pthread_attr_t attr; 
    // 初始化属性
    pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 1024*20);
	err = pthread_create(&st->thread, &attr, write_thread, st);
	if (err) {
		warning("i2s: playback started0\n");
		re_atomic_rlx_set(&st->run, false);
		goto out;
	}

	warning("i2s: playback started\n");

 out:
	if (err)
		mem_deref(st);
	else
		*stp = st;

	return err;
}



//===================

struct ausrc_st {
	const struct ausrc *as;  /* pointer to base-class (inheritance) */
	pthread_t thread;
	RE_ATOMIC bool run;
	void *sampv;
	size_t sampc;
	ausrc_read_h *rh;
	void *arg;
	struct ausrc_prm prm;
    
	uint64_t samps;
};


static void ausrc_destructor(void *arg)
{
	struct ausrc_st *st = arg;

	/* Wait for termination of other thread */
	if (re_atomic_rlx(&st->run)) {
		info("i2s: stopping record thread\n");
		re_atomic_rlx_set(&st->run, false);
		thrd_join(st->thread, NULL);
	}

	if(st->sampv)mem_deref(st->sampv);
	//st->rh = NULL; 
	//if(st)mem_deref(st);
}


// /**
//  * Converts the pcm 32 bit I2S values to baresip 16-bit samples. A reasonable
//  * volume for the microphone is achieved by right shifting.
//  * @param st The ausrc_st.
//  * @param i  Offset for st->sampv.
//  * @param n  The number of samples that should be converted.
//  */
// static void convert_pcm(struct ausrc_st *st, size_t i, size_t n)
// {
// 	uint32_t j;
// 	uint16_t *sampv = st->sampv;
// 	for (j = 0; j < n; j++) {
// 		uint32_t v = st->pcm[j];
// 		uint16_t *o = sampv + i + j;
// 		*o = v >> 15;
// 		/* if negative fill with ff */
// 		if (v & 0x80000000)
// 			*o |= 0xfffe0000;
// 	}
// }


// 将 16bit 双声道 PCM 转换为单声道
void stereo_to_mono(int16_t *stereo, int16_t *mono, size_t num_samples)
{
    // 确保 num_samples 是偶数，或者只处理完整的立体声对
   for (size_t i = 0; i < num_samples; i += 2) { // 关键修改：i < num_samples - 1
        // 左右声道平均
         mono[i / 2] = (stereo[i] + stereo[i + 1]) / 2;
    }
    // 注意：如果 num_samples 是奇数，最后一个样本 (stereo[num_samples-1]) 会被忽略。
}
static void *read_thread(void *arg)
{
	struct ausrc_st *st = arg;
	
	//const size_t read_bytes = AUDIO_SAMPLES_PER_FRAME * sizeof(int16_t) ;
	int16_t* pcm0 = malloc(AUDIO_SAMPLES_PER_FRAME*2 * sizeof(int16_t)  );
	int16_t* pcm = malloc(AUDIO_SAMPLES_PER_FRAME * sizeof(int16_t)  );
    if (!pcm) {
        info( "Failed to allocate memory for record buffer");
        return NULL;
    } 

	 //test：
//     int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
//     if (sock < 0) {
//        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
         
//        return NULL;
//     } 
//     //设置目标地址信息
//     struct sockaddr_in dest_addr;
//     dest_addr.sin_addr.s_addr = inet_addr("192.168.1.6");
//    dest_addr.sin_family = AF_INET;
//     dest_addr.sin_port = htons(4000);

	esp_err_t i2s_status;
	while (re_atomic_rlx(&st->run)) {
		 
		size_t bytes_read = 0;
		// if (i2s_read(I2S_RECORD_PORT,  pcm, read_bytes, &bytes_read, pdMS_TO_TICKS(100) ) != ESP_OK){
		// 	ESP_LOGE("I2S","read err");
		// 	continue;
		// }
		
		i2s_status = i2s_channel_read(rx_handle, pcm0  , AUDIO_SAMPLES_PER_FRAME*2 * sizeof(int16_t) , &bytes_read, portMAX_DELAY );
		 
		if (bytes_read !=640*2) {
			ESP_LOGE("I2S","2read not full%d",bytes_read); 
			continue;
		}
		size_t num_samples = bytes_read / sizeof(int16_t);
        size_t mono_samples = num_samples / 2; // 每组双声道变成一个单声道
        // ESP_LOGE(TAG,">>%d>>%d",bytes_read,mono_samples);
        // 转换单声道
        stereo_to_mono(pcm0 , pcm , num_samples); 
	    
		//sendto(sock, pcm ,mono_samples * sizeof(int16_t)    , 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr));


		struct auframe af; 
		auframe_init(&af, st->prm.fmt, pcm , AUDIO_SAMPLES_PER_FRAME,  8000, 1);
	 
		//size_t copy_bytes = auframe_size(&af) * sizeof(int16_t)  ;
		//ESP_LOGE("I2S","=send=%d=%d=%d",copy_bytes,bytes_read,af.ch);
		//memcpy(af.sampv,  pcm, copy_bytes);  
		af.timestamp = st->samps * AUDIO_TIMEBASE / (8000  );
		st->samps += st->sampc;
		st->rh(&af, st->arg); 
		//vTaskDelay( pdMS_TO_TICKS(200)  );
	}
	free(pcm);
	 free(pcm0);
	info("i2s: stopped record thread ok\n");

	return NULL;
}


int i2s_src_alloc(struct ausrc_st **stp, const struct ausrc *as, 
			     struct ausrc_prm *prm, const char *device,
			     ausrc_read_h *rh, ausrc_error_h *errh, void *arg)
{
	struct ausrc_st *st;
	size_t sampc;
	int err;

	 
	(void) device;
	(void) errh;

	if (!stp || !as || !prm || !rh)
		return EINVAL;

	if (prm->fmt!=AUFMT_S16LE) {
		warning("i2s: unsupported sample format %s\n", aufmt_name(prm->fmt));
		return EINVAL;
	}

	sampc = prm->srate * prm->ch * prm->ptime / 1000;
	
	warning("i2s: sampc=%d=%d \n", sampc,prm->srate );
	  
	st = mem_zalloc(sizeof(*st), ausrc_destructor);
	if (!st)
		return ENOMEM;

	st->prm = *prm;
	st->as  = as;
	st->rh  = rh;
	st->arg = arg;
	st->samps=0;
	st->sampc = sampc;
	st->sampv = mem_zalloc(aufmt_sample_size(st->prm.fmt)*st->sampc, NULL);
	if (!st->sampv) {
		err = ENOMEM;
		goto out;
	}
	 
	// err = i2s_start_record_bus(st->prm.srate, I2O_RECO, st->prm.ch);
	// if (err)
	// 	goto out;

	re_atomic_rlx_set(&st->run, true);
	info("  starting src thread\n" );
	pthread_attr_t attr; 
    // 初始化属性
    pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 1024*20);
	err = pthread_create(&st->thread, &attr, read_thread, st);
	if (err) {
		info("  starting src thread fail%d\n", err);
		re_atomic_rlx_set(&st->run, false);
		goto out;
	}

	info("i2s: recording\n");

 out:
	if (err)
		mem_deref(st);
	else
		*stp = st;

	return err;
}












static int i2s_init(void)
{
	int err;

/* Initialize i2s peripheral */
    if (i2s_driver_init() != ESP_OK) {
        ESP_LOGE(TAG, "i2s driver init failed");
        abort();
    } else {
        ESP_LOGI(TAG, "i2s driver init success");
    }
    

    /* Initialize i2c peripheral and config es8311 codec by i2c */
    if (es8311_codec_init() != ESP_OK) {
        ESP_LOGE(TAG, "es8311 codec init failed");
        abort();
    } else {
        ESP_LOGI(TAG, "es8311 codec init success");
    }


	err  = ausrc_register(&ausrc, baresip_ausrcl(),  "i2s", i2s_src_alloc);
	err |= auplay_register(&auplay, baresip_auplayl(),   "i2s", i2s_play_alloc);

	return err;
}

 

static int i2s_close(void)
{
	ausrc  = mem_deref(ausrc);
	auplay = mem_deref(auplay);
	return 0;
}


EXPORT_SYM const struct mod_export DECL_EXPORTS(i2s) = {
	"i2s",
	"sound",
	i2s_init,
	i2s_close
};

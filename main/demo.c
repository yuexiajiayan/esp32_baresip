
#include <re/include/re.h>
#include <re/include/rem.h>
#include <baresip/include/baresip.h> 

static int barestate = 0;  

/*************** */

static struct ausrc *ausrc = NULL;
static struct auplay *auplay = NULL; 


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
		 esp_codec_dev_write( ecdh, st->sampv , num_frames * sizeof(int16_t) );
		// upsample_8k_to_16k( st->sampv, upsampled_buffer,  num_frames);
		 //i2s_channel_write(tx_handle,  upsampled_buffer , num_frames *2 * sizeof(int16_t) , &bytes_write, portMAX_DELAY);	

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

	// if (prm->fmt!=AUFMT_S16LE) {
	// 	warning("i2s: unsupported sample format %s\n", aufmt_name(prm->fmt));
	// 	return EINVAL;
	// }

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
#define AUDIO_SAMPLES_PER_FRAME        320
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

static void *read_thread(void *arg)
{
	struct ausrc_st *st = arg;
	
	//const size_t read_bytes = AUDIO_SAMPLES_PER_FRAME * sizeof(int16_t) ;
    
	int16_t* pcm = malloc(AUDIO_SAMPLES_PER_FRAME * sizeof(int16_t)  );
    if (!pcm) {
        info( "Failed to allocate memory for record buffer");
        return NULL;
    } 

	 //test：
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0) {
       ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
         
       return NULL;
    } 
    //设置目标地址信息
    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = inet_addr("192.168.1.5");
   dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(4000);

	esp_err_t i2s_status; size_t bytes_per_frame = AUDIO_SAMPLES_PER_FRAME * sizeof(int16_t) ;
	while (re_atomic_rlx(&st->run)) {  
		// if (i2s_read(I2S_RECORD_PORT,  pcm, read_bytes, &bytes_read, pdMS_TO_TICKS(100) ) != ESP_OK){
		// 	ESP_LOGE("I2S","read err");
		// 	continue;
		// }
		
		i2s_status =  esp_codec_dev_read(ecdh, pcm,  bytes_per_frame );
		  
        if(i2s_status!=ESP_OK){
            continue;
        }
	    
		sendto(sock, pcm ,bytes_per_frame    , 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr));
 
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

	// if (prm->fmt!=AUFMT_S16LE) {
	// 	warning("i2s: unsupported sample format %s\n", aufmt_name(prm->fmt));
	// 	return EINVAL;
	// }

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


/***************** */


void ua_exit_handler1(void* arg)
{
    ESP_LOGE(TAG,">>>>end2");
    (void)arg;
    
}
 
void signalHandler1(int sig)
{
    ESP_LOGE(TAG,">>>>end23");
    
}
static void event_handler1(enum ua_event ev, struct bevent* event, void* arg)
{
    ESP_LOGE(TAG, "ua event: %s\n", uag_event_str(ev) );
     
    if (ev == UA_EVENT_REGISTER_OK) {
 
        //mythread->sendCodedata(allinfo);
        //list * e = baresip_vidispl();
        //qDebug() << "video dis" << list_count(e); 
        greg=1;
        cxindex=0;
    }
    else if (ev == UA_EVENT_CALL_INCOMING) {
        baresipcall =   bevent_get_call(event);
        baresipcallstate=2;
    }
    else if (ev == UA_EVENT_SIPSESS_CONN) {
        const struct sip_msg* msg = bevent_get_msg(event); 
        int err = ua_accept(baresipgua, msg);
        //if (err) {
            //warning("test: could not accept incoming call (%m)\n", err);
            //return;
        //}  
        bevent_stop(event);
    }else if(ev==UA_EVENT_CALL_OUTGOING){
       baresipcallstate=1;
    }else if(ev==UA_EVENT_CALL_RINGING){
       baresipcallstate=3;
    } else if(ev==UA_EVENT_CALL_ESTABLISHED) {
       baresipcallstate=5;     
    }else if (ev == UA_EVENT_CALL_CLOSED) {
       baresipcallstate=0;
    }
}
void* my_thread(void * param){


    ecdh=bsp_audio_codec_microphone_init(); 
    cJSON *root = configRead();
    if (root) {
        cJSON *yinliang = cJSON_GetObjectItem(root, "yinliang");
        if(yinliang){
            ESP_LOGE(TAG,"init yinliang %d",(int)cJSON_GetNumberValue(yinliang));         
            esp_codec_dev_set_out_vol(ecdh,(int)cJSON_GetNumberValue(yinliang));
        }
        cJSON *maizy = cJSON_GetObjectItem(root, "maizy");
        if(maizy){
            ESP_LOGE(TAG,"init maizy %f",(float)cJSON_GetNumberValue(maizy));    
            esp_codec_dev_set_in_gain(ecdh,  (float)cJSON_GetNumberValue(maizy));     
        }
        cJSON_Delete(root);
    }  
    esp_codec_dev_sample_info_t fs ;
    fs .sample_rate =8000;
    fs .channel = 1;
    fs .bits_per_sample = 16;
    esp_codec_dev_open(ecdh, &fs); 


    int err = 0; 
    err = libre_init();
    if (err) {
        ESP_LOGE(TAG, "Could not initialize libre");
        return NULL;
    }
  conf_path_set("/sdcard/esp32/baresip"); 
   err= conf_configure();
   if (err) {
        ESP_LOGE(TAG, "Could not initialize libre");
        return NULL;
    }
    enum { ASYNC_WORKERS = 4 };
    re_thread_async_init(ASYNC_WORKERS);
    //struct config * cfg = conf_config();
    //strncpyz(cfg->sip.local, "0.0.0.0:5060", sizeof(cfg->sip.local));
 
   err = baresip_init(conf_config()); 
     if (err) {
        ESP_LOGE(TAG, "Could not baresip_init.");
        return err;
    }
    play_set_path(baresip_player(), "/sdcard/esp32/baresip/share"); 
    err = ua_init("esp baresip", true, false, false);
    if (err) {
          ESP_LOGE(TAG, "Could not ua_init.");
        return err;
    }
    
    uag_set_exit_handler(ua_exit_handler1, NULL);
    bevent_register(event_handler1, NULL);
 
    /* Load modules */
    err = conf_modules();
    if (err)
        goto out;
    //vidsrc_register(&mod_avf, baresip_vidsrcl(), "avformat", allocsrc, NULL);
 
    //ausrc_register(&ausrc, baresip_ausrcl(), "aupcm", aufile_src_alloc);
    //aufilt_register(baresip_aufiltl(), &sndfile );
 
    err  = ausrc_register(&ausrc, baresip_ausrcl(),  "i2s", i2s_src_alloc);
	err |= auplay_register(&auplay, baresip_auplayl(),   "i2s", i2s_play_alloc);


      
    barestate=1;
  ESP_LOGE(TAG, "ba start ok.");
    err = re_main(signalHandler1);
    ESP_LOGE(TAG,"baresipThread end");
 
out:
    if (err)
        ua_stop_all(true);
 
    ua_close();
    module_app_unload();
    conf_close();
    baresip_close();
    re_thread_async_close();
    /* */
   
 
    return NULL; 
}
 static void options_resp_handler(int err, const struct sip_msg *msg, void *arg)
{
    (void)arg;
 
    if (err) {
        warning("options reply error: %m\n", err);
        return;
    }
 
    if (msg->scode < 200)
        return;
 
    if (msg->scode < 300) {
 
        mbuf_set_pos(msg->mb, 0);
        // info("----- OPTIONS of %r -----\n%b",
        //      &msg->to.auri, mbuf_buf(msg->mb),
        //      mbuf_get_left(msg->mb));
        return;
    }
 
    info("%r: OPTIONS failed: %u %r\n", &msg->to.auri,   msg->scode, &msg->reason);
} 









 

void* taskB(void * param){ 
    int playbackring = 0;
    int index = 0;
    char buf2[128];
    char buf3[64];
    while (true)
    {   
        if(gwifistate==1){  
            if(greg==0 &&gcxuanze !=-1 ){
                const char* ip_str = wifi_sta_getip();
                if(ip_str==NULL){ ESP_LOGE(TAG, "getip error");  }
                ESP_LOGE(TAG,">begin reg >" ); 
                if(baresipgua!=NULL){
                    ua_unregister(baresipgua);
                    ua_destroy(baresipgua);
                    baresipgua=NULL;
                } 
                greg=-1; 

                cJSON *root = sipsRead(); 
                if (!root) {  ESP_LOGE(TAG, "Failed to read JSON");  return NULL;    } 
                cJSON *array = cJSON_GetObjectItem(root, "datas");
                if (array && cJSON_IsArray(array)) {
                    cJSON *item; 
                    int cccindex=0;
                    cJSON_ArrayForEach(item, array) { 
                        if (cJSON_IsObject(item)) {  
                            if( cccindex ==gcxuanze ){
                                cJSON *sip_username = cJSON_GetObjectItem(item, "sip_username"); 
                                cJSON *sip_password = cJSON_GetObjectItem(item, "sip_password"); 
                                cJSON *sip_host = cJSON_GetObjectItem(item, "sip_host"); 
                                cJSON *sip_sipport = cJSON_GetObjectItem(item, "sip_sipport"); 
                                cJSON *sip_type = cJSON_GetObjectItem(item, "sip_type"); 
                                char *endptr;long port_value = strtol(  cJSON_GetStringValue(sip_sipport), &endptr, 10);
                                
                                re_thread_enter();
                                char buf1[512];
                                if(sip_type->valueint==1 ){
                                   lv_snprintf(buf1, sizeof(buf1), "sip:%s@%s:%ld;dtmfmode=rtpevent;regint=180;auth_user=%s;auth_pass=%s;transport=tcp", cJSON_GetStringValue(sip_username), cJSON_GetStringValue(sip_host),port_value,cJSON_GetStringValue(sip_username), cJSON_GetStringValue(sip_password)  ) ;      
                                }else{
                                    lv_snprintf(buf1, sizeof(buf1), "sip:%s@%s:%ld;dtmfmode=rtpevent;regint=180;auth_user=%s;auth_pass=%s;", cJSON_GetStringValue(sip_username), cJSON_GetStringValue(sip_host),port_value ,cJSON_GetStringValue(sip_username), cJSON_GetStringValue(sip_password)  ) ; 
                                }
                                lv_snprintf(buf2, sizeof(buf2), "sip:0000000000@%s:%ld", cJSON_GetStringValue(sip_host),port_value ); 
                                lv_snprintf(buf3, sizeof(buf3), "%s:%ld", cJSON_GetStringValue(sip_host),port_value ); 
                                ua_alloc(&baresipgua, buf1);//write file  
                                int state = ua_register(baresipgua);

                                re_thread_leave();
                            }
                            cccindex=cccindex+1;
                        }
                    }
                }
                cJSON_Delete(root); 

            }else if(greg==1){
                if( baresipaction==100){
                    baresipaction=0;
                    re_thread_enter();
                    char callbuf[128];
                     lv_snprintf(callbuf, sizeof(callbuf), "sip:%s@%s",baresipcallbuf ,buf3 ); 
                    ua_connect(baresipgua, &baresipcall, NULL, callbuf,  VIDMODE_OFF);
                    re_thread_leave();
                }else  if( baresipaction==101){
                    baresipaction=0; 
                     if(baresipcall){ 
                        re_thread_enter();
                        ua_answer( baresipgua,baresipcall,VIDMODE_OFF  );
                        re_thread_leave();
                     }
                   
                }else  if( baresipaction==102){
                    baresipaction=0;
                    if(baresipcall){
                        re_thread_enter();
                        ua_hangup( baresipgua,baresipcall,  0, NULL );
                        re_thread_leave();
                        baresipcall=NULL; 
                    }
                }else  if( baresipaction>=103 && baresipaction<115){
                    baresipaction=0;
                    if(baresipcall){
                        re_thread_enter();
                        if(baresipaction==103){
                             call_send_digit(  baresipcall, '*' );    ESP_LOGE(TAG, "dtmf:%c","*");   
                        }else if(baresipaction==104){
                             call_send_digit(  baresipcall, '#'); 
                             ESP_LOGE(TAG, "dtmf:%c",'#');   
                        }else  {
                             call_send_digit(  baresipcall, '0' + (baresipaction - 105) );    
                             ESP_LOGE(TAG, "dtmf:%c", "0" + (baresipaction - 105) );
                             ESP_LOGE(TAG, "dtmf:%c","0" );
                        } 
                        re_thread_leave();
                    }
                }
                xQueueSend(xPageQueue, &baresipcallstate, portMAX_DELAY ); 
                if(baresipgua!=NULL&&index%12==0){
                    index=0; 
                    ua_options_send(baresipgua,buf2, options_resp_handler, NULL); 
                }
                index++;
            //    if (cstate==SIP_CALL_STATE_INCOMING){ 
            //         if(playbackring==0){ 
            //             playbackring =1;
            //             stopplayback(g_audio_pipeline); 
            //             playwav(g_audio_pipeline,"/sdcard/esp32/ringtone.wav",0); 
            //         } 
            //    }else{
            //     if( playbackring ==1  ){ 
            //         playbackring=0;
                     
            //     }
            //    }
               //ESP_LOGE(TAG,"CSTATE:%d",cstate); 
            }
            if(gplaystate>=0){  
                // 使用 snprintf 安全地拼接字符串
                if(gplaystate<10){
                    char wav_path[32];  
                    snprintf(wav_path, sizeof(wav_path), "/sdcard/esp32/%d.wav", gplaystate);
                    gplaystate=-1;
                    playwav(wav_path,0); 
                }else if(gplaystate==10){
                    gplaystate=-1;
                    playwav("/sdcard/esp32/a1.wav",0); 
                }else if(gplaystate==11){
                    gplaystate=-1;
                    playwav("/sdcard/esp32/a2.wav",0); 
                } 
            }    
        }
       // ESP_LOGE(TAG,"Minimum free heap size: %" PRIu32 " bytes=%d=%d\n", esp_get_free_heap_size(),gwifistate,greg);
        ESP_LOGE("MEM1", ">%d=%d", heap_caps_get_free_size(MALLOC_CAP_INTERNAL),heap_caps_get_free_size(MALLOC_CAP_SPIRAM) );
        vTaskDelay(pdMS_TO_TICKS(1000)); 
    } 
    vTaskDelete(NULL);   
}
  
void   app_main(void)
{
  //init wifi
	//.....
   //xTaskCreatePinnedToCore(taskB,"taskB",1024*6,(void*)xPageQueue,3,NULL,1);





// static StaticTask_t *task_buffer = NULL;
// static StackType_t *task_stack = NULL; 
// const int stack_size = 1024 * 12; // 10KB

//     // 从外部 SPI RAM 分配栈空间
//     task_stack = (StackType_t *) heap_caps_malloc(stack_size * sizeof(StackType_t), MALLOC_CAP_SPIRAM);
//     if (!task_stack) {
//         ESP_LOGE(TAG, "Failed to allocate stack in PSRAM");
//         return;
//     }

//     task_buffer = (StaticTask_t *) heap_caps_malloc(sizeof(StaticTask_t), MALLOC_CAP_INTERNAL);
//     if (!task_buffer) {
//         ESP_LOGE(TAG, "Failed to allocate TCB in PSRAM");
//         free(task_stack);
//         return;
//     }

//     // 创建任务（静态方式，栈和 TCB 都在 PSRAM）
//     xTaskCreateStaticPinnedToCore(
//         taskB,              // 任务函数
//         "taskB",            // 任务名
//         stack_size,           // 栈大小（单位：StackType_t）
//         (void*)xPageQueue,                 // 参数
//         5,                    // 优先级
//         task_stack,           // 用户分配的栈
//         task_buffer,          // 用户分配的 TCB
//         1                     // 核心 ID
//     );


    static StaticTask_t *task_bufferc = NULL;
    static StackType_t *task_stackc = NULL; 
    const int stack_size1 = 1024 * 40; // 10KB

        // 从外部 SPI RAM 分配栈空间
        task_stackc = (StackType_t *) heap_caps_malloc(stack_size1 * sizeof(StackType_t), MALLOC_CAP_SPIRAM);
        if (!task_stackc) {
            ESP_LOGE(TAG, "Failed to allocate stack in PSRAM");
            return;
        }

        task_bufferc = (StaticTask_t *) heap_caps_malloc(sizeof(StaticTask_t), MALLOC_CAP_INTERNAL);
        if (!task_bufferc) {
            ESP_LOGE(TAG, "Failed to allocate TCB in PSRAM");
            free(task_stackc);
            return;
        }

        // 创建任务（静态方式，栈和 TCB 都在 PSRAM）
        xTaskCreateStaticPinnedToCore(
            taskC,              // 任务函数
            "taskC",            // 任务名
            stack_size1,           // 栈大小（单位：StackType_t）
            NULL,                 // 参数
            6,                    // 优先级
            task_stackc,           // 用户分配的栈
            task_bufferc,          // 用户分配的 TCB
            1                     // 核心 ID
        ); 
 
} 

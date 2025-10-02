/**
 * @file i2s.h freeRTOS I2S audio driver module - internal interface
 *
 * Copyright (C) 2019 cspiel.at
 */
#include "esp_log.h"  
// #define I2S_RECORD_PORT  I2S_NUM_1
// #define I2S_PLAY_PORT  I2S_NUM_0


// enum I2SOnMask {
// 	I2O_NONE = 0,
// 	I2O_PLAY = 1,
// 	I2O_RECO = 2,
// 	I2O_BOTH = 3
// };


int i2s_src_alloc(struct ausrc_st **stp, const struct ausrc *as, 
			     struct ausrc_prm *prm, const char *device,
			     ausrc_read_h *rh, ausrc_error_h *errh, void *arg);


int i2s_play_alloc(struct auplay_st **stp, const struct auplay *ap,
		    struct auplay_prm *prm, const char *device,
		    auplay_write_h *wh, void *arg);


 

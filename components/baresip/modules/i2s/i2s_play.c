/**
 * @file i2s_play.c freeRTOS I2S audio driver module - player
 *
 * Copyright (C) 2019 cspiel.at
 */
#include <sys/types.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "freertos/FreeRTOS.h"
#include "driver/i2s.h"
#include <re.h>
#include <rem.h>
#include <baresip.h>
#include "i2s.h"
#include <re_atomic.h>
#include "es8311.h"
#include "driver/i2s_common.h"   
#include "driver/i2s_std.h"

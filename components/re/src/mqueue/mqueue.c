/**
 * @file mqueue.c Thread Safe Message Queue (ESP32 version)
 *
 * Copyright (C) 2010 Creytiv.com
 * Adapted for ESP-IDF by Qwen.
 */

#include <re_types.h>
#include <re_mem.h>
#include <re_fmt.h>
#include <re_mqueue.h>
 
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
 
#define MAGIC 0x14553399

struct msg {
    void *data;
    uint32_t magic;
    int id;
};

struct mqueue {
    QueueHandle_t queue;      // FreeRTOS 队列句柄
    TaskHandle_t task_handle;
    mqueue_h *handler;        // 消息处理回调
    void *arg;                // 回调参数
};

// FreeRTOS 任务函数，模拟 re_main() 的事件循环
static void mqueue_task(void *pvParameters)
{
    struct mqueue *mq = (struct mqueue *)pvParameters;
    struct msg msg;

    while (1) {
        if (xQueueReceive(mq->queue, &msg, portMAX_DELAY) == pdTRUE) {
            if (msg.magic != MAGIC) {
              printf( "Bad message magic:  " );
                continue;
            }
            printf("Processing message with id: %d\n", msg.id);
            mq->handler(msg.id, msg.data, mq->arg);
        }
    }

    vTaskDelete(NULL);
}

/**
 * Allocate a new Message Queue
 *
 * @param mqp   Pointer to allocated Message Queue
 * @param h     Message handler
 * @param arg   Handler argument
 *
 * @return 0 if success, otherwise errorcode
 */
int mqueue_alloc(struct mqueue **mqp, mqueue_h *h, void *arg)
{
    struct mqueue *mq;
    const int queue_size = 16; // 可调整的消息队列长度

    if (!mqp || !h)
        return EINVAL;

    mq = mem_zalloc(sizeof(*mq), NULL); // 使用你的内存管理
    if (!mq)
        return ENOMEM;

    mq->queue = xQueueCreate(queue_size, sizeof(struct msg));
    if (!mq->queue) {
        printf("Failed to create FreeRTOS queue");
        mem_deref(mq);
        return ENOMEM;
    }

    mq->handler = h;
    mq->arg = arg;

    // 创建一个 FreeRTOS 任务来监听队列
    if (xTaskCreate(mqueue_task, "mqueue_task", 1024*5, mq, tskIDLE_PRIORITY + 2, &mq->task_handle) != pdPASS) {
        printf( "Failed to create taskcc");
        vQueueDelete(mq->queue);
        mem_deref(mq);
        return ENOMEM;
    }

    *mqp = mq;
    return 0;
}

/**
 * Push a new message onto the Message Queue
 *
 * @param mq   Message Queue
 * @param id   General purpose Identifier
 * @param data Application data
 *
 * @return 0 if success, otherwise errorcode
 */
int mqueue_push(struct mqueue *mq, int id, void *data)
{
    struct msg msg;

    if (!mq)
        return EINVAL;

    msg.id = id;
    msg.data = data;
    msg.magic = MAGIC;

   if (xQueueSend(mq->queue, &msg, portMAX_DELAY) != pdTRUE) {
        printf( "Failed to send message to queue from ISR");
        return EPIPE;
    }

    return 0;
}

 
 
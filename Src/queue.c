#include <stdio.h>
#include <string.h>
#define uint8_t unsigned char
#define uint16_t unsigned short

#define QUEUE_UNINITIALIZED 0 // 队列未初始化
#define QUEUE_INITIALIZED 1   // 队列已初始化
#define QUEUE_FULL 2          // FIFO满置2
#define QUEUE_EMPTY 3         // FIFO空置3
#define QUEUE_OPERATE_OK 4    // 队列操作完成 赋值为4

#define QUEUE_SIZE 20 // FIFO队列的大小

typedef struct
{
    uint8_t ctx_id;
    uint8_t ctx_type;
    uint8_t size;
    uint8_t buff[255];
} TYP_CTX;

typedef struct
{
    uint8_t init;  // 初始化标志
    uint8_t mutex; // 互斥锁
    uint8_t front; // 队列头
    uint8_t rear;  // 队列尾
    uint8_t size;  // 队列计数
    TYP_CTX ctx[QUEUE_SIZE];
} TYP_QUEUE;

static TYP_QUEUE message_chain;
static TYP_QUEUE message_chain_mutex;

// Queue Init
void queue_init(TYP_QUEUE *queue)
{
    queue->rear = 0x00;
    queue->front = queue->rear;
    queue->mutex = 0x00;
    queue->size = 0x00;
    queue->init = 0x01;
}

// Queue In 入栈
uint8_t queue_in(TYP_QUEUE *queue, TYP_CTX ctx) // 数据进入队列
{
    if (queue->init == QUEUE_INITIALIZED)
    {
        if ((queue->front == queue->rear) && (queue->size == QUEUE_SIZE))
        {
            return QUEUE_FULL; // 返回队列满的标志
        }
        else
        {
            queue->ctx[queue->rear] = ctx;
            queue->rear = (queue->rear + 1) % QUEUE_SIZE;
            queue->size = queue->size + 1;
            return QUEUE_OPERATE_OK;
        }
    }
    else
    {
        return QUEUE_UNINITIALIZED;
    }
}

// Queue Out 出栈
uint8_t queue_out(TYP_QUEUE *queue, TYP_CTX *ctx)
{
    if (queue->init == QUEUE_INITIALIZED)
    {

        if ((queue->front == queue->rear) && (queue->size == 0))
        {
            return QUEUE_EMPTY;
        }
        else
        {
            *ctx = queue->ctx[queue->front];
            queue->front = (queue->front + 1) % QUEUE_SIZE;
            queue->size = queue->size - 1;
            return QUEUE_OPERATE_OK;
        }
    }
    else
    {
        return QUEUE_UNINITIALIZED;
    }
}

void message_chain_init(void)
{
    queue_init(&message_chain);
    queue_init(&message_chain_mutex);
}

uint8_t message_chain_receive(TYP_CTX *ctx, uint8_t *ret)
{
    *ret = 0xFF;
    if (message_chain.size)
    {
        *ret = queue_out(&message_chain, ctx);
        return 0x01;
    }
    else if (message_chain.mutex)
    {
        if (message_chain_mutex.size)
        {
            *ret = queue_out(&message_chain_mutex, ctx);
            message_chain.mutex = message_chain_mutex.size;
            return 0x01;
        }
    }
    return 0x00;
}

uint8_t message_chain_send(TYP_CTX ctx, uint8_t *ret)
{
    TYP_CTX ctx_mutex;
    *ret = 0xff;
    uint8_t ret_mutex = 0x00;
    if (message_chain.size == QUEUE_SIZE)
    {
        message_chain.mutex = 0x01;
    }
    if (message_chain.mutex)
    {
        *ret =  queue_in(&message_chain_mutex, ctx);
        if(*ret == QUEUE_FULL)
        {
            return 0x00;
        }
        return 0x01;
    }
    else
    {
        if (message_chain_mutex.size)
        {
            ret_mutex = queue_out(&message_chain_mutex, &ctx_mutex);
            if (ret_mutex == QUEUE_OPERATE_OK)
            {
                ret_mutex = queue_in(&message_chain, ctx_mutex);
                if (ret_mutex != QUEUE_OPERATE_OK)
                {
                    queue_in(&message_chain_mutex, ctx_mutex);
                }
            }
        }
        if (message_chain.size != QUEUE_SIZE)
        {
            *ret = queue_in(&message_chain, ctx);
            return 0x01;
        }
    }
}
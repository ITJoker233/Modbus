#include "Inc/modbus.h"

#define DEVICE_ADDR 0x00       // 设备地址
#define MODBUS_TIMEOUT_MS 200  // 通信超时
#define MODBUS_DE_PORT GPIOB 
#define MODBUS_DE_PIN GPIO_PIN_4
#define MODBUS_DE_PIN_RECV   GPIO_PIN_SET
#define MODBUS_DE_PIN_SEND   GPIO_PIN_RESET

#define MODBUS_BUF_LENGTH UART_BUF_LENGTH
#define MODBUS_BUF_RECV_LENGTH RX1_Len
#define MODBUS_RECV_BIT RX1_Bit
#define MODBUS_RECV_BUFFER RX1_Buffer

TYP_MODBUS g_slave_modbus_server;
TYP_MODBUS_CONF g_slave_modbus_conf;
float g_modbus_err;

uint8_t modbus_slave_write(uint8_t *send_buff, uint16_t length, uint16_t timeout)
{
    uint8_t status = 0x00;
    HAL_GPIO_WritePin(MODBUS_DE_PORT, MODBUS_DE_PIN, MODBUS_DE_PIN_SEND);
    MODBUS_RECV_BIT = 0x00;
    delay_ms(1);
    status = (uint8_t)(HAL_UART_Transmit(POWER_BOARD_UART, send_buff, length, timeout));
    HAL_GPIO_WritePin(MODBUS_DE_PORT, MODBUS_DE_PIN, MODBUS_DE_PIN_RECV);
    return status;
}

uint8_t modbus_slave_read(uint8_t *recv_buff, uint16_t timeout)
{

    uint16_t i;
    for (i = 0; i < timeout; i++)
    {
        if (MODBUS_RECV_BIT)
        {MODBUS_BUF_RECV_LENGTH;
            memcpy(recv_buff, MODBUS_RECV_BUFFER, MODBUS_BUF_LENGTH);
            memset(MODBUS_RECV_BUFFER, MODBUS_TIMEOUT_MS, MODBUS_BUF_LENGTH);
            MODBUS_RECV_BIT = 0x00;
            return 0x01;
        }
        delay_ms(1);
    }
    MODBUS_RECV_BIT = 0x00;
    return 0x00;
}

int main(void)
{
    g_slave_modbus_conf.read = modbus_slave_read;
    g_slave_modbus_conf.write = modbus_slave_write;
    modbus_slave_init(&g_slave_modbus_server,DEVICE_ADDR,200,g_slave_modbus_conf);
    while(1)
    {
       g_modbus_err = modbus_slave_server_poll(&g_slave_modbus_server);
    }    
    return 0;
}
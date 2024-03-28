
#ifndef _MODBUS_H
#define _MODBUS_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "crc.h"
#include "hashmap.h"

// modbus rtu
#define MODBUS_REG_NUM_MAX 125
#define MODBUS_BUFF_SIZE 260


typedef enum modbus_error
{
    // Library errors
    MODBUS_ERROR_INVALID_UNIT_ID = 0xF0,  /**< Received invalid unit ID in response from server */
    MODBUS_ERROR_CRC = 0xF1,              /**< Received invalid CRC */
    MODBUS_ERROR_TRANSPORT = 0xF2,        /**< Transport error */
    MODBUS_ERROR_TIMEOUT = 0xF3,          /**< Read/write timeout occurred */
    MODBUS_ERROR_INVALID_RESPONSE = 0xF4, /**< Received invalid response from server */
    MODBUS_ERROR_INVALID_ARGUMENT = 0xF5, /**< Invalid argument provided */
    MODBUS_ERROR_NONE = 0x00,              /**< No error */

    // Modbus exceptions
    MODBUS_EXCEPTION_ILLEGAL_FUNCTION = 1,      /**< Modbus exception 1 */
    MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS = 2,  /**< Modbus exception 2 */
    MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE = 3,    /**< Modbus exception 3 */
    MODBUS_EXCEPTION_SERVER_DEVICE_FAILURE = 4, /**< Modbus exception 4 */
} modbus_error;

typedef struct TYP_MODBUS_CONF
{
    uint8_t (*read)(uint8_t* buf, uint16_t timeout_ms); /*!< Bytes read transport function pointer */
    uint8_t (*write)(uint8_t* buf, uint16_t count, uint16_t timeout_ms); /*!< Bytes write transport function pointer */
} TYP_MODBUS_CONF;

typedef struct TYP_MODBUS_DATA
{
    uint16_t modbus_coils[MODBUS_REG_NUM_MAX];
    uint16_t modbus_input_regs[MODBUS_REG_NUM_MAX];
    uint16_t modbus_holding_regs[MODBUS_REG_NUM_MAX];
} TYP_MODBUS_DATA;

typedef struct TYP_MODBUS_RECV_MSG
{
    uint8_t buf[MODBUS_BUFF_SIZE];
    uint8_t addr;
    uint8_t func;
    uint16_t start_reg;
    uint16_t reg_num;
    uint16_t length;
} TYP_MODBUS_RECV_MSG;
typedef modbus_error (*Callback)(TYP_MODBUS_RECV_MSG modbus);

typedef struct TYP_MODBUS_CALLBACK
{
    Callback read_coils;
    Callback read_discrete_inputs;
    Callback read_holding_registers;
    Callback read_input_registers;
    Callback write_single_coil;
    Callback write_single_register;
    Callback write_multiple_coils;
    Callback write_multiple_registers;
} TYP_MODBUS_CALLBACK;


typedef struct TYP_MODBUS
{
    uint8_t slave_addr;
    uint16_t timeout;
    TYP_MODBUS_RECV_MSG msg;
    TYP_MODBUS_DATA data;
    TYP_MODBUS_CALLBACK callbacks;
    TYP_MODBUS_CONF platform;
}TYP_MODBUS;

uint8_t modbus_set_slave_callback(TYP_MODBUS *modbus, Callback callback,uint8_t func);
uint8_t modbus_set_platform_send_recv(TYP_MODBUS *modbus,TYP_MODBUS_CONF modbus_conf);
void modbus_slave_init(TYP_MODBUS *modbus,uint8_t slave_addr,uint16_t timeout,TYP_MODBUS_CONF modbus_conf);
uint8_t modbus_slave_server_poll(TYP_MODBUS *modbus);

#endif

#include "modbus.h"

HashMap modbus_func_set_callback_map;
uint8_t modbus_send_buff[MODBUS_BUFF_SIZE];
uint8_t modbus_send_temp_buff[MODBUS_BUFF_SIZE];
/**
* 模块名: modbus_set_slave_callback
* 代码描述: 设置 modbus slave RTU模式从机回调
* 作者:ITJoker233
* 形参:TYP_MODBUS *modbus, Callback callback,uint8_t func
* 创建时间:2023/12/22 11:18:26
*/
uint8_t modbus_set_slave_callback(TYP_MODBUS *modbus, Callback callback,uint8_t func)
{
    modbus_error err = MODBUS_ERROR_NONE;
    default_put_hashMap(&modbus_func_set_callback_map,func,0x01);
    switch (func)
    {
    case 1:
        modbus->callbacks.read_coils = callback;
        break;
    case 2:
        modbus->callbacks.read_discrete_inputs = callback;
        break;
    case 3:
        modbus->callbacks.read_holding_registers = callback;
        break;
    case 4:
        modbus->callbacks.read_input_registers = callback;
        break;
    case 5:
        modbus->callbacks.write_single_coil = callback;
        break;
    case 6:
        modbus->callbacks.write_single_register = callback;
        break;
    case 15:
        modbus->callbacks.write_multiple_coils = callback;
        break;
    case 16:
        modbus->callbacks.write_multiple_registers = callback;
        break;
    default:
        err = MODBUS_EXCEPTION_ILLEGAL_FUNCTION;
    }
    return err;
}
/**
* 模块名:modbus_set_platform_send_recv
* 代码描述: 设置硬件接口的发送和接收函数
* 作者:ITJoker233
* 形参:TYP_MODBUS *modbus,TYP_MODBUS_CONF modbus_conf
* 创建时间:2023/12/22 11:20:00
*/
uint8_t modbus_set_platform_send_recv(TYP_MODBUS *modbus,TYP_MODBUS_CONF modbus_conf)
{
    modbus_error err = MODBUS_ERROR_NONE;
    modbus->platform.read = modbus_conf.read;
    modbus->platform.write = modbus_conf.write;
    return err;
}
/**
* 模块名:modbus_slave_init
* 代码描述: 从机模式初始化
* 作者:ITJoker233
* 形参:TYP_MODBUS *modbus,uint8_t slave_addr,uint16_t timeout,TYP_MODBUS_CONF modbus_conf
* 创建时间:2023/12/22 11:20:26
*/
void modbus_slave_init(TYP_MODBUS *modbus,uint8_t slave_addr,uint16_t timeout,TYP_MODBUS_CONF modbus_conf)
{
    modbus->slave_addr = slave_addr;
    modbus->timeout = timeout;
    modbus_set_platform_send_recv(modbus,modbus_conf);
    hashMap_init(&modbus_func_set_callback_map);
    memset(modbus_send_buff,0,sizeof(modbus_send_buff));
}

uint8_t modbus_msg_get_u8(TYP_MODBUS *modbus,uint8_t idx)
{
    return ((uint8_t)(modbus->msg.buf[idx]));
}

uint16_t modbus_msg_get_u16(TYP_MODBUS *modbus,uint8_t high_idx,uint8_t low_idx)
{
    return ((uint16_t)((modbus->msg.buf[high_idx])<<8) + modbus->msg.buf[low_idx]);
}

uint8_t modbus_recv(TYP_MODBUS *modbus)
{
    uint8_t ret = modbus->platform.read(modbus->msg.buf, modbus->timeout);
    if (ret)
    {
        return MODBUS_ERROR_NONE;
    }
    else
    {
        return MODBUS_ERROR_TIMEOUT;
    }
}

uint8_t modbus_send(TYP_MODBUS *modbus,uint8_t* buff, uint16_t count)
{
    uint8_t ret = modbus->platform.write(buff, count, modbus->timeout);
    if (ret)
    {
        return MODBUS_ERROR_NONE;
    }
    else
    {
        return MODBUS_ERROR_TIMEOUT;
    }
}

uint8_t check_msg_header(TYP_MODBUS *modbus)
{
    modbus->msg.addr = modbus_msg_get_u8(modbus, 0);
    modbus->msg.func = modbus_msg_get_u8(modbus, 1);
    modbus->msg.start_reg = modbus_msg_get_u16(modbus, 2, 3);
    modbus->msg.reg_num = modbus_msg_get_u16(modbus, 4, 5);
    if (modbus->msg.addr != modbus->slave_addr)
    {
        return MODBUS_ERROR_INVALID_UNIT_ID;
    }
    if((modbus->msg.reg_num+modbus->msg.start_reg) > MODBUS_REG_NUM_MAX)
    {
        return MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
    }
    return MODBUS_ERROR_NONE;
}

uint8_t check_msg_footer(TYP_MODBUS *modbus)
{
    if(modbus->msg.length)
    {
        uint16_t crc = CRC16(modbus->msg.buf, modbus->msg.length - 2);
        uint16_t recv_crc = modbus_msg_get_u16(modbus, modbus->msg.length - 1, modbus->msg.length - 2);
        if (recv_crc != crc)
        {
            return MODBUS_ERROR_CRC;
        }
    }

    return MODBUS_ERROR_NONE;
}

uint8_t default_modbus_buff_check(TYP_MODBUS *modbus)
{
    modbus_error err = modbus_recv(modbus);
    if (err != MODBUS_ERROR_NONE)
    {
        return err;
    }
    err = check_msg_header(modbus);
    if (err != MODBUS_ERROR_NONE)
    {
        return err;
    }
    err = check_msg_footer(modbus);
    if (err != MODBUS_ERROR_NONE)
    {
        return err;
    }
    return err;
}

uint8_t handle_modbus_callback(TYP_MODBUS *modbus, Callback callback)
{
    modbus_error err = default_modbus_buff_check(modbus);
    if (err != MODBUS_ERROR_NONE)
    {
        return err;
    }
    err = callback(modbus->msg);
    return err;
}

modbus_error handle_read_coils(TYP_MODBUS *modbus)  // 0x01 读取多个会有问题
{
    uint16_t crc16_data = 0x0000;
    uint16_t byte_len = 0x0000;
    modbus_send_buff[0] = modbus->slave_addr;
    modbus_send_buff[1] = modbus->msg.func;
    byte_len = modbus->msg.reg_num;
    modbus_send_buff[2] = byte_len & 0xff;

    for(uint16_t i=0; i<byte_len; i++) // 待修改 读取多个线圈会有问题，得改成读取指定的gpio寄存器
    {
        modbus_send_temp_buff[i] = modbus->data.modbus_coils[modbus->msg.start_reg+i];
    }
    memcpy(modbus_send_buff+0x03,modbus_send_temp_buff,byte_len);
    crc16_data = CRC16(modbus_send_buff, byte_len+0x03);
    modbus_send_buff[3+byte_len] = crc16_data;
    modbus_send_buff[4+byte_len] = crc16_data>>8;
    modbus_send(modbus,modbus_send_buff, 5+byte_len);
    return MODBUS_ERROR_NONE;
}

modbus_error handle_read_discrete_inputs(TYP_MODBUS *modbus) // 0x02  读取多个会有问题
{
    uint16_t crc16_data = 0x0000;
    uint16_t byte_len = 0x0000;
    modbus_send_buff[0] = modbus->slave_addr;
    modbus_send_buff[1] = modbus->msg.func;
    byte_len = modbus->msg.reg_num;
    modbus_send_buff[2] = byte_len & 0xff;

    for(uint16_t i=0; i<byte_len; i++) // 待修改 读取多个线圈会有问题，得改成读取指定的gpio寄存器
    {
        modbus_send_temp_buff[i] = modbus->data.modbus_coils[modbus->msg.start_reg+i];
    }
    memcpy(modbus_send_buff+0x03,modbus_send_temp_buff,byte_len);
    crc16_data = CRC16(modbus_send_buff, byte_len+0x03);
    modbus_send_buff[3+byte_len] = crc16_data;
    modbus_send_buff[4+byte_len] = crc16_data>>8;
    modbus_send(modbus,modbus_send_buff, 5+byte_len);
    return MODBUS_ERROR_NONE;
}

modbus_error handle_read_holding_registers(TYP_MODBUS *modbus)  // 0x03
{
    uint16_t crc16_data = 0x0000;
    uint16_t byte_len = 0x0000;
    modbus_send_buff[0] = modbus->slave_addr;
    modbus_send_buff[1] = modbus->msg.func;
    byte_len = modbus->msg.reg_num*0x02;
    modbus_send_buff[2] = byte_len & 0xff;

    for(uint16_t i=0; i<byte_len; i+=2)
    {
        modbus_send_temp_buff[i] = modbus->data.modbus_holding_regs[modbus->msg.start_reg+i/2]>>8;
        modbus_send_temp_buff[i+1] = modbus->data.modbus_holding_regs[modbus->msg.start_reg+i/2];
    }
    memcpy(modbus_send_buff+0x03,modbus_send_temp_buff,byte_len);
    crc16_data = CRC16(modbus_send_buff, byte_len+0x03);
    modbus_send_buff[3+byte_len] = crc16_data;
    modbus_send_buff[4+byte_len] = crc16_data>>8;
    modbus_send(modbus,modbus_send_buff, 5+byte_len);
    return MODBUS_ERROR_NONE;
}

modbus_error handle_read_input_registers(TYP_MODBUS *modbus) // 0x04
{
    uint16_t crc16_data = 0x0000,addr_offset;
    addr_offset = modbus->msg.start_reg+modbus->msg.reg_num;
    if(addr_offset > MODBUS_REG_NUM_MAX)
    {
        return MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
    }
    modbus_send_buff[0] = modbus->slave_addr;
    modbus_send_buff[1] = modbus->msg.func;
    modbus_send_buff[2] = 0x02;
    modbus_send_buff[3]  = modbus->data.modbus_input_regs[addr_offset] >> 8;
    modbus_send_buff[4]  = modbus->data.modbus_input_regs[addr_offset];
    crc16_data = CRC16(modbus_send_buff, 0x05);
    modbus_send_buff[5] = crc16_data;
    modbus_send_buff[6] = crc16_data>>8;
    modbus_send(modbus,modbus_send_buff, 0x07);
    return MODBUS_ERROR_NONE;
}

modbus_error handle_write_single_coil(TYP_MODBUS *modbus)
{
    modbus_error err = MODBUS_ERROR_NONE;//modbus_send_write_respone(modbus,(modbus->data.modbus_coils));
    return err;
}

modbus_error handle_write_single_register(TYP_MODBUS *modbus)  // 0x06
{
    uint16_t start_addr = modbus->msg.start_reg;
    modbus->data.modbus_holding_regs[start_addr] = modbus_msg_get_u16(modbus, 5, 6);
    modbus_send(modbus, modbus->msg.buf,8);
    return MODBUS_ERROR_NONE;
}

modbus_error handle_write_multiple_coils(TYP_MODBUS *modbus)
{
    modbus_error err = MODBUS_ERROR_NONE;//modbus_send_write_respone(modbus,(modbus->data.modbus_coils));
    return err;
}

modbus_error handle_write_multiple_registers(TYP_MODBUS *modbus)  // 0x10
{
    uint16_t crc16_data = 0x0000;
    uint16_t byte_len = 0x0000;
    uint16_t addr = modbus->msg.start_reg;
    uint16_t pdata = 0x0000;
    uint16_t i;
    byte_len = modbus_msg_get_u8(modbus,0x06);
    for(i=0; i<byte_len; i++)
    {
        if(i%2==0x000)
        {
            pdata = modbus_msg_get_u16(modbus, (i+7), (i+8));
            modbus->data.modbus_holding_regs[addr++] = pdata;
        }
    }
    modbus_send_buff[0] = modbus->slave_addr;
    modbus_send_buff[1] = modbus->msg.func;
    modbus_send_buff[2] = modbus->msg.start_reg >> 8;
    modbus_send_buff[3] = modbus->msg.start_reg & 0xff;
    modbus_send_buff[4] = modbus->msg.reg_num >> 8;
    modbus_send_buff[5] = modbus->msg.reg_num & 0xff;
    crc16_data = CRC16(modbus_send_buff, 6);
    modbus_send_buff[6] = crc16_data;
    modbus_send_buff[7] = crc16_data >> 8;
    modbus_send(modbus, modbus_send_buff,8);
    return MODBUS_ERROR_NONE;
}

/**
* 模块名:handle_request_func
* 代码描述: modbus rtu从机模式 请求处理
* 作者:ITJoker233
* 形参: TYP_MODBUS *modbus    指向modbus结构体的指针
* 创建时间:2023/12/22 11:17:11
*/
modbus_error handle_request_func(TYP_MODBUS *modbus)  // 0x03 0x10
{
    modbus_error err;
    uint8_t has_callback = 0x00;
    default_get_hashMap(&modbus_func_set_callback_map, modbus->msg.func,&has_callback);
    switch (modbus->msg.func)
    {
//    case 0x01:
//        if(has_callback)
//        {
//            err = handle_modbus_callback(modbus, modbus->callbacks.read_coils);
//        }
//        else
//        {
//            err = handle_read_coils(modbus);
//        }
//        break;
//    case 0x02:  // 
//        if(has_callback)
//        {
//            err = handle_modbus_callback(modbus, modbus->callbacks.read_discrete_inputs);
//        }
//        else
//        {
//            err = handle_read_discrete_inputs(modbus);
//        }
//        break;
    case 0x03:  // 读取保存寄存器
        if(has_callback)
        {
            err = handle_modbus_callback(modbus, modbus->callbacks.read_holding_registers);
        }
        else
        {
            err = handle_read_holding_registers(modbus);
        }
        break;
    case 0x04:  // 读取输入寄存器
        if(has_callback)
        {
            err = handle_modbus_callback(modbus, modbus->callbacks.read_input_registers);
        }
        else
        {
            err = handle_read_input_registers(modbus);
        }
        break;
//    case 0x05:  // 设置单线圈
//        if(has_callback)
//        {
//            err = handle_modbus_callback(modbus, modbus->callbacks.write_single_coil);
//        }
//        else
//        {
//            err = handle_write_single_coil(modbus);
//        }
//        break;
    case 0x06:  // 写入单寄存器（保存寄存器）
        if(has_callback)
        {
            err = handle_modbus_callback(modbus, modbus->callbacks.write_single_register);
        }
        else
        {
            err = handle_write_single_register(modbus);
        }
        break;
//    case 0x0F:  // 设置多线圈
//        if(has_callback)
//        {
//            err = handle_modbus_callback(modbus, modbus->callbacks.write_multiple_coils);
//        }
//        else
//        {
//            err = handle_write_multiple_coils(modbus);
//        }
//        break;
    case 0x10: // 写入多寄存器（保存寄存器）
        if(has_callback)
        {
            err = handle_modbus_callback(modbus, modbus->callbacks.write_multiple_registers);
        }
        else
        {
            err = handle_write_multiple_registers(modbus);
            // todo save holding reg
        }
        break;
    default:
        err = MODBUS_EXCEPTION_ILLEGAL_FUNCTION;
    }
    return err;
}

/**
* 模块名:modbus_slave_server_poll
* 代码描述: modbus rtu从机模式服务
* 作者:ITJoker233
* 形参: TYP_MODBUS *modbus    指向modbus结构体的指针
* 创建时间:2023/12/22 11:15:10
*/
uint8_t modbus_slave_server_poll(TYP_MODBUS *modbus)
{
    modbus_error err = default_modbus_buff_check(modbus);
    if (err != MODBUS_ERROR_NONE)
    {
        return err;
    }
    err = handle_request_func(modbus);
    return err;
}

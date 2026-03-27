#include "EEPROM.h"
#include "F2837xD_device.h"
#include "PressLoop.h"
#include "math.h"
#include "string.h"
#include <limits.h>
#include <stdlib.h>

#define I2C_SLAVE_ADDR 0x50
#define I2C_NUMBYTES 2
#define I2C_EEPROM_HIGH_ADDR 0x00
#define I2C_EEPROM_LOW_ADDR 0x30

#define I2C_WAIT_FOR_BUS_BUSY_CLEAR() \
    while (I2caRegs.I2CSTR.bit.BB == 1)

#define I2C_WAIT_FOR_STOP_CONDITION() \
    while (I2caRegs.I2CSTR.bit.SCD != 1)

#define I2C_WAIT_FOR_REG_ACCESS_READY() \
    while (I2caRegs.I2CSTR.bit.ARDY != 1)

glob_cfg_t glob_cfg = {.Pos_limit.I = 8, .Pos_limit.spd = 1000, .Press_Ctrl.adc_coeff = 0.0309275743};
// I2C初始化
void I2CA_Init(void)
{
    // 配置I2C引脚
    GPIO_SetupPinMux(42, GPIO_MUX_CPU1, 6); // SDA
    GPIO_SetupPinMux(43, GPIO_MUX_CPU1, 6); // SC
    // 基本配置
    I2caRegs.I2CSAR.all = I2C_SLAVE_ADDR;
    I2caRegs.I2CPSC.all = 29;
    I2caRegs.I2CCLKL = 10;
    I2caRegs.I2CCLKH = 5;

    I2caRegs.I2CIER.all = 0x0000;

    I2caRegs.I2CMDR.all = 0x0020;  // 退出复位状态
    I2caRegs.I2CFFTX.all = 0x6000; // 启用FIFO
    I2caRegs.I2CFFRX.all = 0x2040;
}

Uint16 I2CA_WriteData(struct I2CMSG *msg)
{
    Uint16 i;

    // 等待总线空闲
    I2C_WAIT_FOR_BUS_BUSY_CLEAR();

    // 设置从机地址
    I2caRegs.I2CSAR.all = msg->SlaveAddress;

    // 配置传输字节数（地址+数据）
    I2caRegs.I2CCNT = msg->NumOfBytes + 2;

    // 写入内存地址
    I2caRegs.I2CDXR.all = msg->MemoryHighAddr;
    I2caRegs.I2CDXR.all = msg->MemoryLowAddr;

    // 写入数据
    for (i = 0; i < msg->NumOfBytes; i++)
    {
        I2caRegs.I2CDXR.all = msg->MsgBuffer[i];
    }

    // 启动传输（主模式+发送器+产生停止位）
    I2caRegs.I2CMDR.all = 0x6E20;

    // 等待传输完成
    I2C_WAIT_FOR_STOP_CONDITION();
    DELAY_US(6000);
    return 0; // 成功
}

Uint16 I2CA_ReadData(struct I2CMSG *msg)
{
    // 阶段1：发送地址设置命令
    I2C_WAIT_FOR_BUS_BUSY_CLEAR();
    I2caRegs.I2CSAR.all = msg->SlaveAddress;
    I2caRegs.I2CCNT = 2; // 只发送地址
    I2caRegs.I2CDXR.all = msg->MemoryHighAddr;
    I2caRegs.I2CDXR.all = msg->MemoryLowAddr;
    I2caRegs.I2CMDR.all = 0x2620; // 无停止位

    // 等待地址设置完成
    I2C_WAIT_FOR_REG_ACCESS_READY();

    // 阶段2：重新启动并读取数据
    I2caRegs.I2CCNT = msg->NumOfBytes;
    I2caRegs.I2CMDR.all = 0x2C20; // 主接收模式+重启

    // 等待数据接收完成
    I2C_WAIT_FOR_STOP_CONDITION();

    // 检查RXFFST寄存器确保有足够数据
    while (I2caRegs.I2CFFRX.bit.RXFFST < msg->NumOfBytes)
        ;

    // 从FIFO读取数据
    Uint16 i = 0;
    for (i = 0; i < msg->NumOfBytes; i++)
    {
        msg->MsgBuffer[i] = I2caRegs.I2CDRR.all;
    }
    DELAY_US(6000);
    return 0; // 成功
}

Uint16 EEPROM_Write(Uint16 Addr, Uint16 *data, Uint16 length)
{
    struct I2CMSG msg = {
        .SlaveAddress = I2C_SLAVE_ADDR,
        .NumOfBytes = length,
        .MemoryHighAddr = (uint16_t)(Addr >> 8),
        .MemoryLowAddr = (uint16_t)(Addr & 0x00FF)};
    Uint16 i = 0;
    for (i = 0; i < length; i++)
    {
        msg.MsgBuffer[i] = data[i];
    }

    return I2CA_WriteData(&msg);
}

Uint16 EEPROM_Read(Uint16 Addr, Uint16 *data, Uint16 length)
{
    struct I2CMSG msg = {
        .SlaveAddress = I2C_SLAVE_ADDR,
        .NumOfBytes = length,
        .MemoryHighAddr = (uint16_t)(Addr >> 8),
        .MemoryLowAddr = (uint16_t)(Addr & 0x00FF)};
    Uint16 i;
    Uint16 status = I2CA_ReadData(&msg);
    if (status == 0)
    {
        for (i = 0; i < length; i++)
        {
            data[i] = msg.MsgBuffer[i];
        }
    }
    return status;
}

/* 调整此值以匹配 EEPROM page 大小（字节） */
#ifndef EEPROM_PAGE_SIZE
#define EEPROM_PAGE_SIZE 32
#endif

/* 计算以 8-bit 字节为单位的长度（在此平台上 CHAR_BIT==16） */
// #define BYTES_OF(obj_or_len) ((size_t)(obj_or_len) * (CHAR_BIT) / 8)

#define BYTES_OF(obj_or_len) (sizeof(obj_or_len) * (CHAR_BIT) / 8)

static void set_mem_byte(void *mem_words, Uint16 index, Uint16 val)
{
    Uint16 *w = (Uint16 *)mem_words;
    Uint16 wi = index >> 1;
    if ((index & 1) == 0)
    {
        /* 低字节 */
        w[wi] = (w[wi] & 0xFF00) | (val & 0x00FF);
    }
    else
    {
        /* 高字节 */
        w[wi] = (w[wi] & 0x00FF) | ((val & 0x00FF) << 8);
    }
}

static Uint16 get_mem_byte(const void *mem_words, Uint16 index)
{
    const Uint16 *w = (const Uint16 *)mem_words;
    Uint16 word = w[index >> 1];
    if ((index & 1) == 0)
        return (Uint16)(word & 0x00FF);
    else
        return (Uint16)((word >> 8) & 0x00FF);
}

/* 等待写完成：简单轮询读取第一个字节，直到 EEPROM 响应为止（最多尝试 N 次） */
static void wait_write_complete(Uint16 addr)
{
    Uint16 tmp[2];
    int attempts = 0;
    while (attempts++ < 50)
    {
        if (EEPROM_Read(addr, tmp, 1) == 0)
            return; /* 设备已准备好 */
        DELAY_US(1000);
    }
    /* 最后的保底延时 */
    DELAY_US(5000);
}

/* 分页写：传入的是按字节组织的缓冲（每项低8位有效），len 为字节数 */
Uint16 EEPROM_WriteBytes(Uint16 Addr, const Uint16 *byte_buf, Uint16 len)
{
    Uint16 remaining = len;
    Uint16 cur_addr = Addr;
    Uint16 offset = 0;
    while (remaining > 0)
    {
        Uint16 page_off = cur_addr % EEPROM_PAGE_SIZE;
        Uint16 chunk = EEPROM_PAGE_SIZE - page_off;
        if (chunk > remaining)
            chunk = remaining;

        Uint16 tmp[EEPROM_PAGE_SIZE];
        Uint16 i;
        for (i = 0; i < chunk; ++i)
            tmp[i] = byte_buf[offset + i] & 0x00FFu;

        EEPROM_Write(cur_addr, tmp, chunk);
        wait_write_complete(cur_addr);

        cur_addr += chunk;
        offset += chunk;
        remaining -= chunk;
    }
    return 0;
}

#define READ_LEN (4)
/* 分页读：把字节读到 Uint16 缓冲（每项低8位有效） */
Uint16 EEPROM_ReadBytes(Uint16 Addr, Uint16 *byte_buf, Uint16 len)
{
    Uint16 remaining = len;
    Uint16 cur_addr = Addr;
    Uint16 offset = 0;
    while (remaining > 0)
    {
        Uint16 chunk = (remaining > READ_LEN) ? READ_LEN : remaining;
        Uint16 tmp[READ_LEN];
        EEPROM_Read(cur_addr, tmp, chunk);
        Uint16 i;
        for (i = 0; i < chunk; ++i)
            byte_buf[offset + i] = tmp[i] & 0x00FFu;

        cur_addr += chunk;
        offset += chunk;
        remaining -= chunk;
        DELAY_US(5000);
    }
    return 0;
}

/* 把内存中（以 16-bit words 表示）的结构按字节写入 EEPROM（自动分页） */
// Uint16 EEPROM_WriteStruct(Uint16 Addr, const void *struct_ptr, Uint16 byte_len)
// {
//     Uint16 remaining = byte_len;
//     Uint16 cur_addr = Addr;
//     Uint16 byte_index = 0;
//     while (remaining > 0)
//     {
//         Uint16 page_off = cur_addr % EEPROM_PAGE_SIZE;
//         Uint16 chunk = EEPROM_PAGE_SIZE - page_off;
//         if (chunk > remaining)
//             chunk = remaining;
//         Uint16 tmp[EEPROM_PAGE_SIZE];
//         for (Uint16 i = 0; i < chunk; ++i)
//             tmp[i] = get_mem_byte(struct_ptr, byte_index + i);
//         EEPROM_WriteBytes(cur_addr, tmp, chunk);
//         cur_addr += chunk;
//         byte_index += chunk;
//         remaining -= chunk;
//     }
//     return 0;
// }

#define WRITE_LEN (4)

Uint16 EEPROM_WriteStruct(Uint16 Addr, const void *struct_ptr, Uint16 byte_len)
{
    Uint16 remaining = byte_len;
    Uint16 cur_addr = Addr;
    Uint16 byte_index = 0;
    while (remaining > 0)
    {

        Uint16 chunk = WRITE_LEN;
        if (chunk > remaining)
            chunk = remaining;

        Uint16 tmp[WRITE_LEN];
        for (Uint16 i = 0; i < chunk; ++i)
            tmp[i] = get_mem_byte(struct_ptr, byte_index + i);

        EEPROM_Write(cur_addr, tmp, chunk);
        DELAY_US(5000);
        cur_addr += chunk;
        byte_index += chunk;
        remaining -= chunk;
    }
    return 0;
}

/* 从 EEPROM 读字节并合成回以 16-bit words 为单位的内存结构 */
Uint16 EEPROM_ReadStruct(Uint16 Addr, void *struct_ptr, Uint16 byte_len)
{
    uint16_t remaining = byte_len;
    uint16_t cur_addr = Addr;
    uint16_t byte_index = 0;
    uint16_t tmp[READ_LEN];
    while (remaining > 0)
    {
        Uint16 chunk = (remaining > READ_LEN) ? READ_LEN : remaining;
        // EEPROM_ReadBytes(cur_addr, tmp, chunk);
        EEPROM_Read(cur_addr, tmp, chunk);
        for (Uint16 i = 0; i < chunk; ++i)
        {
            set_mem_byte(struct_ptr, byte_index + i, tmp[i] & 0x00FFu);
        }
        DELAY_US(5000);
        cur_addr += chunk;
        byte_index += chunk;
        remaining -= chunk;
    }
    return 0;
}

static Uint32 seed = 12345;
Uint16 get_random_0_to_100(void)
{
    seed = (1664525 * seed + 1013904223) % 2147483647; // LCG 算法
    return (seed % 101);                               // 取模 101 得到 0~100
}

Uint16 EEPROM_Check(void)
{
    Uint16 i = 0;
    Uint16 write_d[4], read_d[4];
    for (i = 0; i < 4; i++)
    {
        write_d[i] = get_random_0_to_100();
    }
    EEPROM_Write(0x0000, write_d, 4);
    EEPROM_Read(0x0000, read_d, 4);
    if (read_d[0] == write_d[0] && read_d[1] == write_d[1] && read_d[2] == write_d[2] && read_d[3] == write_d[3])
    {
    }
    else
    {
        return 0;
    }
    for (i = 0; i < 4; i++)
    {
        write_d[i] = get_random_0_to_100();
    }
    EEPROM_Write(0x5555, write_d, 4);
    EEPROM_Read(0x5555, read_d, 4);
    if (read_d[0] == write_d[0] && read_d[1] == write_d[1] && read_d[2] == write_d[2] && read_d[3] == write_d[3])
    {
    }
    else
    {
        return 0;
    }
    for (i = 0; i < 4; i++)
    {
        write_d[i] = get_random_0_to_100();
    }
    EEPROM_Write(0xAAAA, write_d, 4);
    EEPROM_Read(0xAAAA, read_d, 4);
    if (read_d[0] == write_d[0] && read_d[1] == write_d[1] && read_d[2] == write_d[2] && read_d[3] == write_d[3])
    {
    }
    else
    {
        return 0;
    }
    for (i = 0; i < 4; i++)
    {
        write_d[i] = get_random_0_to_100();
    }
    EEPROM_Write(0xffee, write_d, 4);
    EEPROM_Read(0xffee, read_d, 4);

    if (read_d[0] == write_d[0] && read_d[1] == write_d[1] && read_d[2] == write_d[2] && read_d[3] == write_d[3])
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

#if 0
/// @brief todo 编译器字节大小问题
/// @return
uint16_t Cfg_write()
{
    glob_cfg.Press_Ctrl.Kp = g_lKp;
    glob_cfg.Press_Ctrl.Ki = g_lKi;
    // glob_cfg.Press_Ctrl.Kd = g_lKd;
    // glob_cfg.Press_Ctrl.Kf = g_lKf;
    // glob_cfg.Press_Ctrl.EC1 = g_lEC1;
    // return EEPROM_Write(0x0010, &glob_cfg, sizeof(glob_cfg));

    // uint16_t _len = sizeof(glob_cfg);
    // uint16_t buff[100];
    // for (size_t i = 0; i < _len; i++)
    // {
    //     buff[i] = *(((char *)&glob_cfg) + i);
    // }
    // return EEPROM_Write(0x0010, &buff, sizeof(_len));

    uint16_t idx = 0;
    uint16_t offset = 0;
    uint16_t buff[100];

    buff[idx++] = ((glob_cfg.Press_Ctrl.Kp >> 0) & 0xFFu);
    buff[idx++] = ((glob_cfg.Press_Ctrl.Kp >> 8) & 0xFFu);
    buff[idx++] = ((glob_cfg.Press_Ctrl.Kp >> 16) & 0xFFu);
    buff[idx++] = ((glob_cfg.Press_Ctrl.Kp >> 24) & 0xFFu);
    EEPROM_Write(0x0010 + offset, &buff, idx);
    offset += idx;
    idx = 0;
    DELAY_US(5000);

    buff[idx++] = ((glob_cfg.Press_Ctrl.Ki >> 0) & 0xFFu);
    buff[idx++] = ((glob_cfg.Press_Ctrl.Ki >> 8) & 0xFFu);
    buff[idx++] = ((glob_cfg.Press_Ctrl.Ki >> 16) & 0xFFu);
    buff[idx++] = ((glob_cfg.Press_Ctrl.Ki >> 24) & 0xFFu);
    EEPROM_Write(0x0010 + offset, &buff, idx);
    offset += idx;
    idx = 0;
    DELAY_US(5000);

    buff[idx++] = ((glob_cfg.Press_Ctrl.Kd >> 0) & 0xFFu);
    buff[idx++] = ((glob_cfg.Press_Ctrl.Kd >> 8) & 0xFFu);
    buff[idx++] = ((glob_cfg.Press_Ctrl.Kd >> 16) & 0xFFu);
    buff[idx++] = ((glob_cfg.Press_Ctrl.Kd >> 24) & 0xFFu);
    EEPROM_Write(0x0010 + offset, &buff, idx);
    offset += idx;
    idx = 0;
    DELAY_US(5000);

    buff[idx++] = ((glob_cfg.Press_Ctrl.Kf >> 0) & 0xFFu);
    buff[idx++] = ((glob_cfg.Press_Ctrl.Kf >> 8) & 0xFFu);
    buff[idx++] = ((glob_cfg.Press_Ctrl.Kf >> 16) & 0xFFu);
    buff[idx++] = ((glob_cfg.Press_Ctrl.Kf >> 24) & 0xFFu);
    EEPROM_Write(0x0010 + offset, &buff, idx);
    offset += idx;
    idx = 0;
    DELAY_US(5000);

    buff[idx++] = ((glob_cfg.Press_Ctrl.EC1 >> 0) & 0xFFu);
    buff[idx++] = ((glob_cfg.Press_Ctrl.EC1 >> 8) & 0xFFu);
    buff[idx++] = ((glob_cfg.Press_Ctrl.EC1 >> 16) & 0xFFu);
    buff[idx++] = ((glob_cfg.Press_Ctrl.EC1 >> 24) & 0xFFu);
    EEPROM_Write(0x0010 + offset, &buff, idx);
    offset += idx;
    idx = 0;
    DELAY_US(5000);

    buff[idx++] = ((glob_cfg.Pos_limit.I >> 0) & 0xFFu);
    buff[idx++] = ((glob_cfg.Pos_limit.I >> 8) & 0xFFu);
    EEPROM_Write(0x0010 + offset, &buff, idx);
    offset += idx;
    idx = 0;
    DELAY_US(5000);

    buff[idx++] = ((glob_cfg.Pos_limit.spd >> 0) & 0xFFu);
    buff[idx++] = ((glob_cfg.Pos_limit.spd >> 8) & 0xFFu);
    EEPROM_Write(0x0010 + offset, &buff, idx);

    return 0;
}

void Cfg_Read()
{

    //  EEPROM_Read(0x0010, &glob_cfg, sizeof(glob_cfg));

    uint16_t _len = sizeof(glob_cfg);
    uint16_t buff[100];

    uint16_t idx = 0;
    uint16_t offset = 0;

    EEPROM_Read(0x0010 + offset, buff, 4);
    glob_cfg.Press_Ctrl.Kp = (uint32_t)buff[idx++];
    glob_cfg.Press_Ctrl.Kp |= ((uint32_t)buff[idx++] << 8);
    glob_cfg.Press_Ctrl.Kp |= ((uint32_t)buff[idx++] << 16);
    glob_cfg.Press_Ctrl.Kp |= ((uint32_t)buff[idx++] << 24);
    offset += idx;
    idx = 0;
    DELAY_US(5000);

    EEPROM_Read(0x0010 + offset, buff, 4);
    glob_cfg.Press_Ctrl.Ki = (uint32_t)buff[idx++];
    glob_cfg.Press_Ctrl.Ki |= ((uint32_t)buff[idx++] << 8);
    glob_cfg.Press_Ctrl.Ki |= ((uint32_t)buff[idx++] << 16);
    glob_cfg.Press_Ctrl.Ki |= ((uint32_t)buff[idx++] << 24);
    offset += idx;
    idx = 0;
    DELAY_US(5000);

    EEPROM_Read(0x0010 + offset, buff, 4);
    glob_cfg.Press_Ctrl.Kd = (uint32_t)buff[idx++];
    glob_cfg.Press_Ctrl.Kd |= ((uint32_t)buff[idx++] << 8);
    glob_cfg.Press_Ctrl.Kd |= ((uint32_t)buff[idx++] << 16);
    glob_cfg.Press_Ctrl.Kd |= ((uint32_t)buff[idx++] << 24);
    offset += idx;
    idx = 0;
    DELAY_US(5000);

    EEPROM_Read(0x0010 + offset, buff, 4);
    glob_cfg.Press_Ctrl.Kf = (uint32_t)buff[idx++];
    glob_cfg.Press_Ctrl.Kf |= ((uint32_t)buff[idx++] << 8);
    glob_cfg.Press_Ctrl.Kf |= ((uint32_t)buff[idx++] << 16);
    glob_cfg.Press_Ctrl.Kf |= ((uint32_t)buff[idx++] << 24);
    offset += idx;
    idx = 0;
    DELAY_US(5000);

    EEPROM_Read(0x0010 + offset, buff, 4);
    glob_cfg.Press_Ctrl.EC1 = (uint32_t)buff[idx++];
    glob_cfg.Press_Ctrl.EC1 |= ((uint32_t)buff[idx++] << 8);
    glob_cfg.Press_Ctrl.EC1 |= ((uint32_t)buff[idx++] << 16);
    glob_cfg.Press_Ctrl.EC1 |= ((uint32_t)buff[idx++] << 24);
    offset += idx;
    idx = 0;
    DELAY_US(5000);

    EEPROM_Read(0x0010 + offset, buff, 2);
    glob_cfg.Pos_limit.I = (uint32_t)buff[idx++];
    glob_cfg.Pos_limit.I |= ((uint32_t)buff[idx++] << 8);
    offset += idx;
    idx = 0;
    DELAY_US(5000);

    EEPROM_Read(0x0010 + offset, buff, 2);
    glob_cfg.Pos_limit.spd = (uint32_t)buff[idx++];
    glob_cfg.Pos_limit.spd |= ((uint32_t)buff[idx++] << 8);
    idx = 0;

    // g_lKp = glob_cfg.Press_Ctrl.Kp;
    // g_lKi = glob_cfg.Press_Ctrl.Ki;
    // g_lKd = glob_cfg.Press_Ctrl.Kd;
    // g_lKf = glob_cfg.Press_Ctrl.Kf;
    // g_lEC1 = glob_cfg.Press_Ctrl.EC1;
}

#else

uint16_t Cfg_write()
{
    /* 把运行时参数回写到 glob_cfg，然后一次性写入 EEPROM */
    glob_cfg.Press_Ctrl.Kp = g_lKp;
    glob_cfg.Press_Ctrl.Ki = g_lKi;
    // glob_cfg.Press_Ctrl.Kd = g_lKd;
    // glob_cfg.Press_Ctrl.Kf = g_lKf;
    // glob_cfg.Press_Ctrl.EC1 = g_lEC1;
    glob_cfg.Press_Ctrl.PosClosed = g_lPosClosed;
    // glob_cfg.Press_Ctrl.Gain = g_lGain;
    // glob_cfg.Press_Ctrl.Offset = g_lOffset;
    glob_cfg.Press_Ctrl.BaseStep = g_lBaseStep;
    // glob_cfg.Press_Ctrl.Maxset = g_lMaxset;
    // glob_cfg.Press_Ctrl.MaxK = g_lMaxK;
    glob_cfg.Press_Ctrl.MinSpeed = g_lMinSpeed;
    glob_cfg.Press_Ctrl.MaxSpeed = g_lMaxSpeed;
    glob_cfg.Press_Ctrl.DownK = g_lDownK;
    glob_cfg.Press_Ctrl.MidSpeed = g_lMidSpeed;
    return EEPROM_WriteStruct(0x0010, &glob_cfg, (Uint16)BYTES_OF(glob_cfg));
}

void Cfg_Read()
{

    // return;
    /* 一次性从 EEPROM 读回整个结构体到 glob_cfg */
    EEPROM_ReadStruct(0x0010, &glob_cfg, (Uint16)BYTES_OF(glob_cfg));

    /* 将读回的配置同步到运行时全局变量*/
    g_lKp = glob_cfg.Press_Ctrl.Kp;
    g_lKi = glob_cfg.Press_Ctrl.Ki;
    // g_lKd = glob_cfg.Press_Ctrl.Kd;
    // g_lKf = glob_cfg.Press_Ctrl.Kf;
    // g_lEC1 = glob_cfg.Press_Ctrl.EC1;
    glob_cfg.Press_Ctrl.adc_coeff = 0.0309275743;
    g_lPosClosed =  glob_cfg.Press_Ctrl.PosClosed;
    // g_lGain = glob_cfg.Press_Ctrl.Gain;
    // g_lOffset = glob_cfg.Press_Ctrl.Offset;
    g_lBaseStep = glob_cfg.Press_Ctrl.BaseStep;
    // g_lMaxset = glob_cfg.Press_Ctrl.Maxset;
    // g_lMaxK = glob_cfg.Press_Ctrl.MaxK;
    g_lMinSpeed = glob_cfg.Press_Ctrl.MinSpeed;
    g_lMaxSpeed = glob_cfg.Press_Ctrl.MaxSpeed ;
    g_lDownK = glob_cfg.Press_Ctrl.DownK;
    g_lMidSpeed = glob_cfg.Press_Ctrl.MidSpeed;
}
#endif

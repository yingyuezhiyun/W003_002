#include "F28x_Project.h"
#include "SCI.h"

char rs232_buffer_0[RS232_BUFFER_SIZE_0];
uint16_t rx232_index_0 = 0;
bool rx232_complete_0 = false;
bool rx232_process_0 = false; // 标记是否在处理数据帧
uint16_t sci0_Rx_Cnt;
bool sci0_Rx_Cnt_En;
static char *txBuffer_0 = NULL;
static uint16_t txLength_0 = 0;

char rs232_buffer_1[RS232_BUFFER_SIZE_1];
uint16_t rx232_index_1 = 0;
bool rx232_complete_1 = false;
bool rx232_process_1 = false; // 标记是否在处理数据帧
uint16_t sci1_Rx_Cnt;
bool sci1_Rx_Cnt_En;
static char *txBuffer_1 = NULL;
static uint16_t txLength_1 = 0;

char rs232_buffer_2[RS232_BUFFER_SIZE_2];
uint16_t rx232_index_2 = 0;
bool rx232_complete_2 = false;
bool rx232_process_2 = false; // 标记是否在处理数据帧
uint16_t sci2_Rx_Cnt;
bool sci2_Rx_Cnt_En;
static char *txBuffer_2 = NULL;
static uint16_t txLength_2 = 0;

//---------------------------------------------------------------------
// SCIB配置函数
//---------------------------------------------------------------------
void InitSCI0(void)
{
    // 配置GPIO引脚为SCIB功能
    // GPIO54是SCITXDB，GPIO55是SCIRXDB
    GPIO_SetupPinMux(55, GPIO_MUX_CPU1, 6);               // 设置为功能6，即SCIRXDB
    GPIO_SetupPinOptions(55, GPIO_INPUT, GPIO_ASYNC);     // 输入方向，异步输入
    GPIO_SetupPinMux(54, GPIO_MUX_CPU1, 6);               // 设置为功能6，即SCITXDB
    GPIO_SetupPinOptions(54, GPIO_OUTPUT, GPIO_PUSHPULL); // 输出方向，推挽输出

    // 软件复位SCIB模块
    ScibRegs.SCICTL1.bit.SWRESET = 0; // 进入复位
    while (ScibRegs.SCICTL1.bit.SWRESET == 1)
        ; // 等待复位完成
    // 波特率配置：BRR = LSPCLK/(波特率*8) -1 = 50,000,000/(115200*8) -1 ≈ 53.25 → 53
    // ScibRegs.SCIHBAUD.all = ((uint16_t)SCI_PRD  & 0xFF00U) >> 8U; // 高位写入SCIHBAUD
    // ScibRegs.SCILBAUD.all = (uint16_t)SCI_PRD  & 0x00FFU; // 低位写入SCILBAUD
    ScibRegs.SCIHBAUD.all = 0x02;
    ScibRegs.SCILBAUD.all = 0x8A;
    // 通信控制寄存器设置：8N1，异步模式
    ScibRegs.SCICCR.all = 0x0007; // 停止位1，无校验，8位数据，禁止回环
    // 控制寄存器1：使能TX/RX，清除错误标志，退出复位
    ScibRegs.SCICTL1.all = 0x0003;       // 使能TX和RX, 内部SCICLK, 禁用错误中断和睡眠模式
                                         // 控制寄存器12：
    ScibRegs.SCICTL2.bit.TXINTENA = 0;   // 禁用SCI模块的发送中断
    ScibRegs.SCICTL2.bit.RXBKINTENA = 0; // 禁用SCI模块的发送中断
    // FIFO配置
    ScibRegs.SCIFFTX.all = 0xE040;     // 使能TX FIFO，清除TX FIFO
    ScibRegs.SCIFFTX.bit.TXFFIENA = 1; // 使能发送中断
    ScibRegs.SCIFFTX.bit.TXFFIL = 8;   // FIFO触发级别为8
    ScibRegs.SCIFFRX.bit.RXFFIENA = 1; // 使能RX FIFO中断
    ScibRegs.SCIFFRX.bit.RXFFIL = 1;   // FIFO触发级别为1

    ScibRegs.SCIFFCT.all = 0x00; // 禁用自动波特率检测
    // 使能SCI
    ScibRegs.SCICTL1.all = 0x0023; // 使能SCI，从复位状态释放SCI

    ScibRegs.SCIFFTX.bit.TXFIFORESET = 1;
    ScibRegs.SCIFFRX.bit.RXFIFORESET = 1;
}

__interrupt void INT_SCI0_RX_ISR(void) // 定义一个中断服务程序，用于处理SCI0接收中断
{
    int i; // 定义一个整型变量i，用于循环计数

    uint16_t numBytes = ScibRegs.SCIFFRX.bit.RXFFST; // 获取当前FIFO中数据量
    for (i = 0; i < numBytes; i++)
    {
        if (rx232_index_0 < RS232_BUFFER_SIZE_0)
        {
            rs232_buffer_0[rx232_index_0++] = ScibRegs.SCIRXBUF.all; // 读取数据
        }
        else
        {
            // 缓冲区溢出处理（例如复位或丢弃数据）
            // handleBufferOverflow();
        }
    }
    if (!sci0_Rx_Cnt_En)
    {
        sci0_Rx_Cnt_En = true;
    }
    sci0_Rx_Cnt = 0; // 超时计数器清零

    if (rs232_buffer_0[rx232_index_0 - 2] == '\r' && rs232_buffer_0[rx232_index_0 - 1] == '\n')
    {
        rx232_complete_0 = true; // 设置完成标志
        sci0_Rx_Cnt_En = false;
    }

    ScibRegs.SCIFFRX.bit.RXFFOVRCLR = 1;    // 清除溢出标志位
    ScibRegs.SCIFFRX.bit.RXFFINTCLR = 1;    // 清除中断标志位
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP9; // 应答中断
}

__interrupt void INT_SCI0_TX_ISR(void)
{
    uint16_t i;

    ScibRegs.SCIFFTX.bit.TXFFINTCLR = 1;                           // 清除中断标志位
    uint16_t numBytes = ScibRegs.SCIFFTX.bit.TXFFST;               // 获取当前FIFO中数据量
    uint16_t space = 16 - numBytes;                                // 计算剩余空间，并填充数据
    uint16_t sendSize = (txLength_0 < space) ? txLength_0 : space; // 计算填充长度，在FIFO剩余空间与待发送长度中取较小值
    for (i = 0; i < sendSize; i++)
    {
        ScibRegs.SCITXBUF.all = txBuffer_0[i];
    }

    txBuffer_0 += sendSize; // 更新指针
    txLength_0 -= sendSize; // 更新剩余长度

    // 数据发送完毕，禁用中断
    if (txLength_0 == 0)
    {
        ScibRegs.SCIFFTX.bit.TXFFIENA = 0; // 关闭FIFO中断
    }

    PieCtrlRegs.PIEACK.all = PIEACK_GROUP9; // 应答中断
}

void SCI0_SendData(char *data, uint16_t length)
{
    uint16_t i;

    txBuffer_0 = data;   // 设置数据指针
    txLength_0 = length; // 设置数据长度

    // 首次填充FIFO（最多16字节）
    uint16_t initialSend = (length > 16) ? 16 : length;
    for (i = 0; i < initialSend; i++)
    {
        ScibRegs.SCITXBUF.all = txBuffer_0[i];
    }

    txBuffer_0 += initialSend; // 更新指针
    txLength_0 -= initialSend; // 更新剩余长度

    // 若还有数据剩余，使能中断继续发送
    if (txLength_0 > 0)
    {
        ScibRegs.SCIFFTX.bit.TXFFIENA = 1; // 使能TX FIFO中断
    }
}


// 判断SCIC通道（SCI0）是否正在发送数据
// 返回true表示忙（正在发送或FIFO中有数据），false表示空闲，可以开始新的发送
bool SCI0_IsBusy(void)
{
    // txLength_0 != 0 表示还有未发送的后续数据
    // TXFFIENA == 1 表示FIFO发送中断被使能（通常用于继续发送）
    // TXFFST > 0 表示TX FIFO中还有未发送的数据
    return (txLength_0 != 0) || (ScibRegs.SCIFFTX.bit.TXFFIENA != 0) || (ScibRegs.SCIFFTX.bit.TXFFST != 0);
}

// 带判断的发送函数：如果当前总线空闲则启动发送并返回true；若忙则不覆盖当前发送并返回false
bool SCI0_SendDataChecked(char *data, uint16_t length)
{
    if (SCI0_IsBusy())
    {
        return false; // 正在发送，拒绝新的发送请求
    }
    // 空闲时调用现有发送函数开始发送
    SCI0_SendData(data, length);
    return true;
}



// SCID
void InitSCI1(void)
{
    // 配置GPIO引脚为SCID功能
    // GPIO47是SCITXDD，GPIO46是SCIRXDD
    GPIO_SetupPinMux(46, GPIO_MUX_CPU1, 6);               // 设置为功能6，即SCIRXDD
    GPIO_SetupPinOptions(46, GPIO_INPUT, GPIO_ASYNC);     // 输入方向，异步输入
    GPIO_SetupPinMux(47, GPIO_MUX_CPU1, 6);               // 设置为功能6，即SCITXDD
    GPIO_SetupPinOptions(47, GPIO_OUTPUT, GPIO_PUSHPULL); // 输出方向，推挽输出

    // 软件复位SCIB模块
    ScidRegs.SCICTL1.bit.SWRESET = 0; // 进入复位
    while (ScidRegs.SCICTL1.bit.SWRESET == 1)
        ; // 等待复位完成
    // 波特率配置：BRR = LSPCLK/(波特率*8) -1 = 50,000,000/(115200*8) -1 ≈ 53.25 → 53
    ScidRegs.SCIHBAUD.all = ((uint16_t)SCI_PRD & 0xFF00U) >> 8U; // 高位写入SCIHBAUD
    ScidRegs.SCILBAUD.all = (uint16_t)SCI_PRD & 0x00FFU;         // 低位写入SCILBAUD
    // 通信控制寄存器设置：8N1，异步模式
    ScidRegs.SCICCR.all = 0x0007; // 停止位1，无校验，8位数据，禁止回环
    // 控制寄存器1：使能TX/RX，清除错误标志，退出复位
    ScidRegs.SCICTL1.all = 0x0003;       // 使能TX和RX, 内部SCICLK, 禁用错误中断和睡眠模式
                                         // 控制寄存器12：
    ScidRegs.SCICTL2.bit.TXINTENA = 0;   // 禁用SCI模块的发送中断
    ScidRegs.SCICTL2.bit.RXBKINTENA = 0; // 禁用SCI模块的发送中断
    // FIFO配置
    ScidRegs.SCIFFTX.all = 0xE040;     // 使能TX FIFO，清除TX FIFO
    ScidRegs.SCIFFTX.bit.TXFFIENA = 1; // 使能发送中断
    ScidRegs.SCIFFTX.bit.TXFFIL = 8;   // FIFO触发级别为8
    ScidRegs.SCIFFRX.bit.RXFFIENA = 1; // 使能RX FIFO中断
    ScidRegs.SCIFFRX.bit.RXFFIL = 1;   // FIFO触发级别为1

    ScidRegs.SCIFFCT.all = 0x00; // 禁用自动波特率检测
    // 使能SCI
    ScidRegs.SCICTL1.all = 0x0023; // 使能SCI，从复位状态释放SCI

    ScidRegs.SCIFFTX.bit.TXFIFORESET = 1;
    ScidRegs.SCIFFRX.bit.RXFIFORESET = 1;
}

__interrupt void INT_SCI1_RX_ISR(void) // 定义一个中断服务程序，用于处理SCI1接收中断
{
    int i; // 定义一个整型变量i，用于循环计数

    uint16_t numBytes = ScidRegs.SCIFFRX.bit.RXFFST; // 获取当前FIFO中数据量
    for (i = 0; i < numBytes; i++)
    {
        if (rx232_index_1 < RS232_BUFFER_SIZE_1)
        {
            rs232_buffer_1[rx232_index_1++] = ScidRegs.SCIRXBUF.all; // 读取数据
        }
        else
        {
            // 缓冲区溢出处理（例如复位或丢弃数据）
            // handleBufferOverflow();
        }
    }
    if (!sci1_Rx_Cnt_En)
    {
        sci1_Rx_Cnt_En = true;
    }
    sci1_Rx_Cnt = 0; // 超时计数器清零

    ScidRegs.SCIFFRX.bit.RXFFOVRCLR = 1;    // 清除溢出标志位
    ScidRegs.SCIFFRX.bit.RXFFINTCLR = 1;    // 清除中断标志位
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP8; // 应答中断
}

__interrupt void INT_SCI1_TX_ISR(void)
{
    uint16_t i;

    ScidRegs.SCIFFTX.bit.TXFFINTCLR = 1;                           // 清除中断标志位
    uint16_t numBytes = ScidRegs.SCIFFTX.bit.TXFFST;               // 获取当前FIFO中数据量
    uint16_t space = 16 - numBytes;                                // 计算剩余空间，并填充数据
    uint16_t sendSize = (txLength_1 < space) ? txLength_1 : space; // 计算填充长度，在FIFO剩余空间与待发送长度中取较小值
    for (i = 0; i < sendSize; i++)
    {
        ScidRegs.SCITXBUF.all = txBuffer_1[i];
    }

    txBuffer_1 += sendSize; // 更新指针
    txLength_1 -= sendSize; // 更新剩余长度

    // 数据发送完毕，禁用中断
    if (txLength_1 == 0)
    {
        ScidRegs.SCIFFTX.bit.TXFFIENA = 0; // 关闭FIFO中断
    }

    PieCtrlRegs.PIEACK.all = PIEACK_GROUP8; // 应答中断
}

void SCI1_SendData(char *data, uint16_t length)
{
    uint16_t i;

    txBuffer_1 = data;   // 设置数据指针
    txLength_1 = length; // 设置数据长度

    // 首次填充FIFO（最多16字节）
    uint16_t initialSend = (length > 16) ? 16 : length;
    for (i = 0; i < initialSend; i++)
    {
        ScidRegs.SCITXBUF.all = txBuffer_1[i];
    }

    txBuffer_1 += initialSend; // 更新指针
    txLength_1 -= initialSend; // 更新剩余长度

    // 若还有数据剩余，使能中断继续发送
    if (txLength_1 > 0)
    {
        ScidRegs.SCIFFTX.bit.TXFFIENA = 1; // 使能TX FIFO中断
    }
}

// SCIC
void InitSCI2(void)
{
    // 配置GPIO引脚为SCID功能
    // GPIO57为SCIRXDC，GPIO_56为SCITXDC
    GPIO_SetupPinMux(57, GPIO_MUX_CPU1, 6);               // 设置为功能6，即SCIRXDC
    GPIO_SetupPinOptions(57, GPIO_INPUT, GPIO_ASYNC);     // 输入方向，异步输入
    GPIO_SetupPinMux(56, GPIO_MUX_CPU1, 6);               // 设置为功能6，即SCITXDC
    GPIO_SetupPinOptions(56, GPIO_OUTPUT, GPIO_PUSHPULL); // 输出方向，推挽输出

    // 软件复位SCIB模块
    ScicRegs.SCICTL1.bit.SWRESET = 0; // 进入复位
    while (ScicRegs.SCICTL1.bit.SWRESET == 1)
        ; // 等待复位完成
    // 波特率配置：BRR = LSPCLK/(波特率*8) -1 = 50,000,000/(115200*8) -1 ≈ 53.25 → 53
    ScicRegs.SCIHBAUD.all = ((uint16_t)SCI_PRD & 0xFF00U) >> 8U; // 高位写入SCIHBAUD
    ScicRegs.SCILBAUD.all = (uint16_t)SCI_PRD & 0x00FFU;         // 低位写入SCILBAUD
    // 通信控制寄存器设置：8N1，异步模式
    ScicRegs.SCICCR.all = 0x0007; // 停止位1，无校验，8位数据，禁止回环
    // 控制寄存器1：使能TX/RX，清除错误标志，退出复位
    ScicRegs.SCICTL1.all = 0x0003;       // 使能TX和RX, 内部SCICLK, 禁用错误中断和睡眠模式
                                         // 控制寄存器12：
    ScicRegs.SCICTL2.bit.TXINTENA = 0;   // 禁用SCI模块的发送中断
    ScicRegs.SCICTL2.bit.RXBKINTENA = 0; // 禁用SCI模块的发送中断
    // FIFO配置
    ScicRegs.SCIFFTX.all = 0xE040;     // 使能TX FIFO，清除TX FIFO
    ScicRegs.SCIFFTX.bit.TXFFIENA = 1; // 使能发送中断
    ScicRegs.SCIFFTX.bit.TXFFIL = 8;   // FIFO触发级别为8
    ScicRegs.SCIFFRX.bit.RXFFIENA = 1; // 使能RX FIFO中断
    ScicRegs.SCIFFRX.bit.RXFFIL = 1;   // FIFO触发级别为1

    ScicRegs.SCIFFCT.all = 0x00; // 禁用自动波特率检测
    // 使能SCI
    ScicRegs.SCICTL1.all = 0x0023; // 使能SCI，从复位状态释放SCI

    ScicRegs.SCIFFTX.bit.TXFIFORESET = 1;
    ScicRegs.SCIFFRX.bit.RXFIFORESET = 1;
}

__interrupt void INT_SCI2_RX_ISR(void) // 定义一个中断服务程序，用于处理SCI1接收中断
{
    int i; // 定义一个整型变量i，用于循环计数

    uint16_t numBytes = ScicRegs.SCIFFRX.bit.RXFFST; // 获取当前FIFO中数据量
    for (i = 0; i < numBytes; i++)
    {
        if (rx232_index_2 < RS232_BUFFER_SIZE_2)
        {
            rs232_buffer_2[rx232_index_2++] = ScicRegs.SCIRXBUF.all; // 读取数据
        }
        else
        {
            // 缓冲区溢出处理（例如复位或丢弃数据）
            // handleBufferOverflow();
        }
    }
    if (!sci2_Rx_Cnt_En)
    {
        sci2_Rx_Cnt_En = true;
    }
    sci2_Rx_Cnt = 0; // 超时计数器清零

    ScicRegs.SCIFFRX.bit.RXFFOVRCLR = 1;    // 清除溢出标志位
    ScicRegs.SCIFFRX.bit.RXFFINTCLR = 1;    // 清除中断标志位
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP8; // 应答中断
}

__interrupt void INT_SCI2_TX_ISR(void)
{
    uint16_t i;

    ScicRegs.SCIFFTX.bit.TXFFINTCLR = 1;                           // 清除中断标志位
    uint16_t numBytes = ScicRegs.SCIFFTX.bit.TXFFST;               // 获取当前FIFO中数据量
    uint16_t space = 16 - numBytes;                                // 计算剩余空间，并填充数据
    uint16_t sendSize = (txLength_2 < space) ? txLength_2 : space; // 计算填充长度，在FIFO剩余空间与待发送长度中取较小值
    for (i = 0; i < sendSize; i++)
    {
        ScicRegs.SCITXBUF.all = txBuffer_2[i];
    }

    txBuffer_2 += sendSize; // 更新指针
    txLength_2 -= sendSize; // 更新剩余长度

    // 数据发送完毕，禁用中断
    if (txLength_2 == 0)
    {
        ScicRegs.SCIFFTX.bit.TXFFIENA = 0; // 关闭FIFO中断
    }

    PieCtrlRegs.PIEACK.all = PIEACK_GROUP8; // 应答中断
}

void SCI2_SendData(char *data, uint16_t length)
{
    uint16_t i;

    txBuffer_2 = data;   // 设置数据指针
    txLength_2 = length; // 设置数据长度

    // 首次填充FIFO（最多16字节）
    uint16_t initialSend = (length > 16) ? 16 : length;
    for (i = 0; i < initialSend; i++)
    {
        ScicRegs.SCITXBUF.all = txBuffer_2[i];
    }

    txBuffer_2 += initialSend; // 更新指针
    txLength_2 -= initialSend; // 更新剩余长度

    // 若还有数据剩余，使能中断继续发送
    if (txLength_2 > 0)
    {
        ScicRegs.SCIFFTX.bit.TXFFIENA = 1; // 使能TX FIFO中断
    }
}

// 判断SCIC通道（SCI2）是否正在发送数据
// 返回true表示忙（正在发送或FIFO中有数据），false表示空闲，可以开始新的发送
bool SCI2_IsBusy(void)
{
    // txLength_2 != 0 表示还有未发送的后续数据
    // TXFFIENA == 1 表示FIFO发送中断被使能（通常用于继续发送）
    // TXFFST > 0 表示TX FIFO中还有未发送的数据
    return (txLength_2 != 0) || (ScicRegs.SCIFFTX.bit.TXFFIENA != 0) || (ScicRegs.SCIFFTX.bit.TXFFST != 0);
}

// 带判断的发送函数：如果当前总线空闲则启动发送并返回true；若忙则不覆盖当前发送并返回false
bool SCI2_SendDataChecked(char *data, uint16_t length)
{
    if (SCI2_IsBusy())
    {
        return false; // 正在发送，拒绝新的发送请求
    }

    // 空闲时调用现有发送函数开始发送
    SCI2_SendData(data, length);
    return true;
}

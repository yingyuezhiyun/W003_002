#include "elmo_can.h"
#include "stdio.h"
#include "stdlib.h"
#include <math.h>

// ??????
// static bool deviceBusy = false;
uint32_t spd_set = 2000000;
uint32_t ac_set = 60000000;
uint32_t dc_set = 60000000;
int32_t pos_fed = 0;
int32_t spd_fed = 0;
float iq_fed;
int32_t Pos_open_temp, Pos_close_temp, Len_stroke, Pos_open, Pos_close, Elmo_ec;
int32_t Pos_valve;
bool Elmo_en;
bool g_bLock = false;

#define CAN_MAX_BIT_DIVISOR (13)   // The maximum CAN bit timing divisor
#define CAN_MIN_BIT_DIVISOR (5)	   // The minimum CAN bit timing divisor
#define CAN_MAX_PRE_DIVISOR (1024) // The maximum CAN pre-divisor
#define CAN_BTR_BRP_M (0x3F)
#define CAN_BTR_BRPE_M (0xF0000)

#define TX_MSG_OBJ_TX 1
#define TX_MSG_OBJ_RX 2

//
// DS402
//
#define ELMO_NODE_ID 127	// Elmo Node ID
#define CAN_MSG_FRAME_EXT 1 // 扩展帧
#define CAN_MSG_FRAME_STD 0 // 标准帧

typedef union
{
	int32_t i32Value;	  // 用于存放读取到的原始整数
	float f32Value;		  // 用于获取真实的浮点物理量
	uint16_t u16Words[2]; // 用于映射到 CAN 邮箱的寄存器
} CastUnion;

typedef enum
{
	//! Transmit message object.
	MSG_OBJ_TYPE_TX,

	//! Receive message object.
	MSG_OBJ_TYPE_RX
} msgObjType;

float iq_fed = 0.0f;  // 电流0.0A
bool Elmo_en = false; // 控制器使能标志
int32_t Elmo_ec = 0;  // 错误码

//
// ???????
//

static const uint16_t canBitValues[] =
	{
		0x1100, // TSEG2 2, TSEG1 2, SJW 1, Divide 5
		0x1200, // TSEG2 2, TSEG1 3, SJW 1, Divide 6
		0x2240, // TSEG2 3, TSEG1 3, SJW 2, Divide 7
		0x2340, // TSEG2 3, TSEG1 4, SJW 2, Divide 8
		0x3340, // TSEG2 4, TSEG1 4, SJW 2, Divide 9
		0x3440, // TSEG2 4, TSEG1 5, SJW 2, Divide 10
		0x3540, // TSEG2 4, TSEG1 6, SJW 2, Divide 11
		0x3640, // TSEG2 4, TSEG1 7, SJW 2, Divide 12
		0x3740	// TSEG2 4, TSEG1 8, SJW 2, Divide 13
};

static uint8_t elmoNodeId = ELMO_NODE_ID; // Elmo???ID

//
// canSetBitRate - Set the CAN bit rate based on device clock (Hz)
//                 and desired bit rate (Hz)
//
/*
 * 功能: 根据外设时钟和期望的位速率设置CAN位定时寄存器
 * 参数: sourceClock - CAN 外设时钟，单位 Hz
 *        bitRate - 期望的 CAN 波特率，单位 Hz
 * 返回: 成功时返回实际设置后的位率（Hz），失败返回 0
 * 说明: 通过查表和预分频器计算 TSEG 和 BRP，并写入 CAN_BTR。
 */
static uint32_t canSetBitRate(uint32_t sourceClock, uint32_t bitRate)
{
	uint32_t desiredRatio;
	uint32_t canBits;
	uint32_t preDivide;
	uint32_t regValue;
	uint16_t canControlValue;
	//
	// Calculate the desired clock rate.
	//
	desiredRatio = sourceClock / bitRate;

	//
	// Make sure that the Desired Ratio is not too large.  This enforces the
	// requirement that the bit rate is larger than requested.
	//
	if ((sourceClock / desiredRatio) > bitRate)
	{
		desiredRatio += 1;
	}

	//
	// Check all possible values to find a matching value.
	//
	while (desiredRatio <= CAN_MAX_PRE_DIVISOR * CAN_MAX_BIT_DIVISOR)
	{
		//
		// Loop through all possible CAN bit divisors.
		//
		for (canBits = CAN_MAX_BIT_DIVISOR;
			 canBits >= CAN_MIN_BIT_DIVISOR;
			 canBits--)
		{
			//
			// For a given CAN bit divisor save the pre divisor.
			//
			preDivide = desiredRatio / canBits;

			//
			// If the calculated divisors match the desired clock ratio then
			// return these bit rate and set the CAN bit timing.
			//
			if ((preDivide * canBits) == desiredRatio)
			{
				//
				// Start building the bit timing value by adding the bit timing
				// in time quanta.
				//
				regValue = canBitValues[canBits - CAN_MIN_BIT_DIVISOR];
				//
				// To set the bit timing register, the controller must be
				// placed
				// in init mode (if not already), and also configuration change
				// bit enabled.  The state of the register should be saved
				// so it can be restored.
				//
				canControlValue = CanaRegs.CAN_CTL.all;
				CanaRegs.CAN_CTL.bit.Init = 1;
				CanaRegs.CAN_CTL.bit.CCE = 1;
				//
				// Now add in the pre-scalar on the bit rate.
				//
				regValue |= ((preDivide - 1) & CAN_BTR_BRP_M) |
							(((preDivide - 1) << 10) & CAN_BTR_BRPE_M);
				//
				// Set the clock bits in the and the bits of the
				// pre-scalar.
				//
				CanaRegs.CAN_BTR.all = regValue;
				//
				// Restore the saved CAN Control register.
				//
				CanaRegs.CAN_CTL.all = canControlValue;
				//
				// Return the computed bit rate.
				//
				return (sourceClock / (preDivide * canBits));
			}
		}

		//
		// Move the divisor up one and look again.  Only in rare cases are
		// more than 2 loops required to find the value.
		//
		desiredRatio++;
	}

	return 0;
}

//*****************************************************************************
//
// canSetupObject
//
//*****************************************************************************
/*
 * 功能: 配置一个 CAN 消息对象（Message Object）
 * 参数: objID - 消息对象编号（邮箱号）
 *        msgID - 要匹配或发送的 CAN ID
 *        frame - 帧类型，使用 CAN_MSG_FRAME_EXT（扩展）或 CAN_MSG_FRAME_STD（标准）
 *        msgType - 对象类型，TX 或 RX
 * 返回: 无
 * 说明: 设置掩码、仲裁及控制寄存器并将配置写入消息对象。
 */
static void canSetupObject(uint32_t objID, uint32_t msgID, uint8_t frame, msgObjType msgType)
{
	union CAN_IF1CMD_REG CAN_IF1CMD_SHADOW;

	// Wait for busy bit to clear
	//
	while (CanaRegs.CAN_IF1CMD.bit.Busy)
	{
	}

	//
	// Clear and Write out the registers to program the message object.
	//
	CanaRegs.CAN_IF1MSK.all = 0;
	CanaRegs.CAN_IF1ARB.all = 0;
	CanaRegs.CAN_IF1MCTL.all = 0;
	//
	// Set the Control, Mask, and Arb bit so that they get transferred to the
	// Message object.
	//
	CAN_IF1CMD_SHADOW.bit.Control = 1;
	CAN_IF1CMD_SHADOW.bit.Arb = 1;
	CAN_IF1CMD_SHADOW.bit.Mask = 1;
	CAN_IF1CMD_SHADOW.bit.DIR = 1;

	//
	// Set direction to transmit
	//
	if (msgType == MSG_OBJ_TYPE_TX)
	{
		CanaRegs.CAN_IF1ARB.bit.Dir = 1;
	}

	//
	// Set Message ID (this example assumes 11 bit ID mask)
	//
	if (frame == CAN_MSG_FRAME_EXT)
	{
		CanaRegs.CAN_IF1ARB.bit.ID = msgID;
		CanaRegs.CAN_IF1ARB.bit.MsgVal = 1;
		CanaRegs.CAN_IF1ARB.bit.Xtd = 1;
	}
	else
	{
		CanaRegs.CAN_IF1ARB.bit.ID = (msgID << 18);
		CanaRegs.CAN_IF1ARB.bit.MsgVal = 1;
	}

	//
	// Set the data length since this is set for all transfers.  This is
	// also a single transfer and not a FIFO transfer so set EOB bit.
	//
	CanaRegs.CAN_IF1MCTL.bit.DLC = 8;

	//	if(msgType == MSG_OBJ_TYPE_TX)
	//	{
	//		CanaRegs.CAN_IF1MCTL.bit.TxIE = 1;
	//	}
	//	else
	//	{
	//		CanaRegs.CAN_IF1MCTL.bit.RxIE = 1;
	//	}

	//
	// Transfer data to message object RAM
	//
	CAN_IF1CMD_SHADOW.bit.MSG_NUM = objID;
	CanaRegs.CAN_IF1CMD.all = CAN_IF1CMD_SHADOW.all;
}

/*
 * 功能: 通过指定消息对象发送 CAN 报文
 * 参数: objID - 要使用的消息对象编号
 *        data - 指向要发送的字节数组（最多 8 字节）
 *        len  - 要发送的数据长度（1..8）
 * 返回: 无
 * 说明: 将数据按 32-bit 打包写入 DATA 寄存器，并触发 TX 请求。
 */
static void canSendMsg(uint32_t objID, uint8_t *data, uint8_t len)
{
	union CAN_IF1CMD_REG CAN_IF1CMD_SHADOW;

	//
	// Wait for busy bit to clear.
	//
	while (CanaRegs.CAN_IF1CMD.bit.Busy)
	{
	}

	// 3. 数据长度，范围1-8
	CanaRegs.CAN_IF1MCTL.bit.DLC = len;

// 4. 填充数据（优化：按 32-bit 打包写入，替代逐字节重复赋值）
#if 0
	{
		uint32_t dataA = 0;
		uint32_t dataB = 0;
		int i;

		// Pack bytes 0..3 -> dataA (低字节为 Data_0)
		for (i = 0; i < len && i < 4; i++)
		{
			dataA |= ((uint32_t)data[i]) << (8 * i);
		}

		// Pack bytes 4..7 -> dataB (低字节为 Data_4)
		for (i = 4; i < len && i < 8; i++)
		{
			dataB |= ((uint32_t)data[i]) << (8 * (i - 4));
		}

		CanaRegs.CAN_IF1DATA.all = dataA;
		CanaRegs.CAN_IF1DATB.all = dataB;
	}
#endif
	// 4. 填充数据
	if (len >= 1)
	{
		CanaRegs.CAN_IF1DATA.bit.Data_0 = data[0];
	}

	if (len >= 2)
	{
		CanaRegs.CAN_IF1DATA.bit.Data_1 = data[1];
	}

	if (len >= 3)
	{
		CanaRegs.CAN_IF1DATA.bit.Data_2 = data[2];
	}

	if (len >= 4)
	{
		CanaRegs.CAN_IF1DATA.bit.Data_3 = data[3];
	}

	if (len >= 5)
	{
		CanaRegs.CAN_IF1DATB.bit.Data_4 = data[4];
	}

	if (len >= 6)
	{
		CanaRegs.CAN_IF1DATB.bit.Data_5 = data[5];
	}

	if (len >= 7)
	{
		CanaRegs.CAN_IF1DATB.bit.Data_6 = data[6];
	}

	if (len >= 8)
	{
		CanaRegs.CAN_IF1DATB.bit.Data_7 = data[7];
	}

	//
	// Set Direction to write and set DATA-A/DATA-B to be transfered to
	// message object
	//
	CAN_IF1CMD_SHADOW.all = 0;
	CAN_IF1CMD_SHADOW.bit.DIR = 1;
	CAN_IF1CMD_SHADOW.bit.DATA_A = 1;
	CAN_IF1CMD_SHADOW.bit.DATA_B = 1;
	//
	// Set Tx Request Bit
	//
	CAN_IF1CMD_SHADOW.bit.TXRQST = 1;
	//
	// Transfer the message object to the message object specified by
	// objID.
	//
	CAN_IF1CMD_SHADOW.bit.MSG_NUM = objID;
	CanaRegs.CAN_IF1CMD.all = CAN_IF1CMD_SHADOW.all;
}

//
// canRecvMsg - Check the message object for new data.
//                 If new data, data written into array and return true.
//                 If no new data, return false.
//
/*
 * 功能: 检查并接收 CAN 消息（使用 IF2 接口读取消息对象 2）
 * 参数: msgID   - 输出参数，返回接收到的消息 ID
 *        msgData - 输出参数，填充接收到的数据（8 字节缓冲）
 *        msgLen  - 输出参数，返回接收到的数据长度
 * 返回: 若有新数据则返回 true，否则返回 false
 * 说明: 读取 IF2 数据寄存器并清除 NewDat 标志。
 */
bool canRecvMsg(uint32_t *msgID, uint8_t *msgData, uint8_t *msgLen)
{
	bool status;
	//
	// Use Shadow variable for IF2CMD. IF2CMD should be written to in
	// single 32-bit write.
	//
	union CAN_IF2CMD_REG CAN_IF2CMD_SHADOW;
	//
	// Set the Message Data A, Data B, and control values to be read
	// on request for data from the message object.
	//
	CAN_IF2CMD_SHADOW.all = 0;
	CAN_IF2CMD_SHADOW.bit.Control = 1;
	CAN_IF2CMD_SHADOW.bit.DATA_A = 1;
	CAN_IF2CMD_SHADOW.bit.DATA_B = 1;
	//
	// Transfer the message object to the message object IF register.
	//
	CAN_IF2CMD_SHADOW.bit.MSG_NUM = 2;
	CanaRegs.CAN_IF2CMD.all = CAN_IF2CMD_SHADOW.all;

	//
	// Wait for busy bit to clear.
	//
	while (CanaRegs.CAN_IF2CMD.bit.Busy)
	{
	}

	//
	// See if there is new data available.
	//
	if (CanaRegs.CAN_IF2MCTL.bit.NewDat == 1)
	{
		//
		// Read out the data from the CAN registers.
		//
		msgData[0] = CanaRegs.CAN_IF2DATA.bit.Data_0;
		msgData[1] = CanaRegs.CAN_IF2DATA.bit.Data_1;
		msgData[2] = CanaRegs.CAN_IF2DATA.bit.Data_2;
		msgData[3] = CanaRegs.CAN_IF2DATA.bit.Data_3;
		msgData[4] = CanaRegs.CAN_IF2DATB.bit.Data_4;
		msgData[5] = CanaRegs.CAN_IF2DATB.bit.Data_5;
		msgData[6] = CanaRegs.CAN_IF2DATB.bit.Data_6;
		msgData[7] = CanaRegs.CAN_IF2DATB.bit.Data_7;
		*msgLen = 8;
		if (CanaRegs.CAN_IF2ARB.bit.Xtd)
		{
			*msgID = CanaRegs.CAN_IF2ARB.bit.ID;
		}
		else
		{
			*msgID = (CanaRegs.CAN_IF2ARB.bit.ID >> 18);
		}

		//
		// Populate Shadow Variable
		//
		CAN_IF2CMD_SHADOW.all = CanaRegs.CAN_IF2CMD.all;
		//
		// Clear New Data Flag
		//
		CAN_IF2CMD_SHADOW.bit.TxRqst = 1;
		//
		// Transfer the message object to the message object IF register.
		//
		CAN_IF2CMD_SHADOW.bit.MSG_NUM = 2;
		CanaRegs.CAN_IF2CMD.all = CAN_IF2CMD_SHADOW.all;
		status = true;
	}
	else
	{
		status = false;
	}

	return (status);
}

/*
 * 功能: 发送一个 SDO 请求（通常用于读取操作或通用命令）
 * 参数: index    - 对象字典索引
 *        subIndex - 对象字典子索引
 *        command  - SDO 命令码
 * 返回: 无
 */
void elmoSendSDORequest(uint16_t index, uint8_t subIndex, uint8_t command)
{
	uint8_t txMsgData[8];

	txMsgData[0] = command;		   // SDO 命令
	txMsgData[1] = index & 0x00FF; // 主索引
	txMsgData[2] = (index & 0xFF00) >> 8;
	txMsgData[3] = subIndex; // 子索引
	txMsgData[4] = 0;		 // 数据
	txMsgData[5] = 0;
	txMsgData[6] = 0;
	txMsgData[7] = 0;
	canSendMsg(1, txMsgData, 8);
}

/*
 * 功能: 发送一个 SDO 写命令，把指定长度的 value 写入对象字典
 * 参数: index    - 对象字典索引
 *        subIndex - 子索引
 *        value    - 待写入的值（32位，按小端放入数据字节）
 *        size     - 有效字节数（1..4）
 * 返回: 无
 */
void elmoSendSDOWrite(uint16_t index, uint8_t subIndex, uint32_t value, uint8_t size)
{
	uint8_t txMsgData[8];

	txMsgData[0] = 0x22;
	txMsgData[1] = index & 0x00FF;
	txMsgData[2] = (index & 0xFF00) >> 8;
	txMsgData[3] = subIndex;

	if (size >= 1)
	{
		txMsgData[4] = (value & 0x000000FF);
	}

	if (size >= 2)
	{
		txMsgData[5] = (value & 0x0000FF00) >> 8;
	}

	if (size >= 3)
	{
		txMsgData[6] = (value & 0x00FF0000) >> 16;
	}

	if (size >= 4)
	{
		txMsgData[7] = (value & 0xFF000000) >> 24;
	}

	canSendMsg(1, txMsgData, 8);
}

/*
 * 功能: 初始化 CAN A 使用的 GPIO 引脚（RX/TX）
 * 返回: 无
 */
void InitCanAGpio(void)
{
	EALLOW;

	// CANRXA
	GPIO_SetupPinMux(36, GPIO_MUX_CPU1, 6);
	GPIO_SetupPinOptions(36, GPIO_INPUT, 0);
	// CANTXA
	GPIO_SetupPinMux(37, GPIO_MUX_CPU1, 6);
	GPIO_SetupPinOptions(37, GPIO_OUTPUT, 0);

	EDIS;
}

/*
 * 功能: 初始化 CAN 外设、配置波特率，并设置用于 Elmo 的发送/接收消息对象
 * 返回: 无
 * 说明: 将 CAN 控制器置于初始化模式，清空消息对象并程序化邮箱，最后退出初始化。
 */
void elmoCanInit(void)
{
	int16_t iMsg;

	InitCanAGpio();
	//
	// Place CAN controller in init state, regardless of previous state.  This
	// will put controller in idle, and allow the message object RAM to be
	// programmed.
	//
	CanaRegs.CAN_CTL.bit.Init = 1;
	CanaRegs.CAN_CTL.bit.SWR = 1;

	//
	// Wait for busy bit to clear
	//
	while (CanaRegs.CAN_IF1CMD.bit.Busy)
	{
	}

	//
	// Clear the message value bit in the arbitration register.  This indicates
	// the message is not valid and is a "safe" condition to leave the message
	// object.  The same arb reg is used to program all the message objects.
	//
	CanaRegs.CAN_IF1CMD.bit.DIR = 1;
	CanaRegs.CAN_IF1CMD.bit.Arb = 1;
	CanaRegs.CAN_IF1CMD.bit.Control = 1;
	CanaRegs.CAN_IF1ARB.all = 0;
	CanaRegs.CAN_IF1MCTL.all = 0;
	CanaRegs.CAN_IF2CMD.bit.DIR = 1;
	CanaRegs.CAN_IF2CMD.bit.Arb = 1;
	CanaRegs.CAN_IF2CMD.bit.Control = 1;
	CanaRegs.CAN_IF2ARB.all = 0;
	CanaRegs.CAN_IF2MCTL.all = 0;

	//
	// Loop through to program all 32 message objects
	//
	for (iMsg = 1; iMsg <= 32; iMsg += 2)
	{
		//
		// Wait for busy bit to clear
		//
		while (CanaRegs.CAN_IF1CMD.bit.Busy)
		{
		}

		//
		// Initiate programming the message object
		//
		CanaRegs.CAN_IF1CMD.bit.MSG_NUM = iMsg;

		//
		// Wait for busy bit to clear
		//
		while (CanaRegs.CAN_IF2CMD.bit.Busy)
		{
		}

		//
		// Initiate programming the message object
		//
		CanaRegs.CAN_IF2CMD.bit.MSG_NUM = iMsg + 1;
	}

	//
	// Acknowledge any pending status interrupts.
	//
	volatile uint32_t discardRead = CanaRegs.CAN_ES.all;
	//
	// Setup CAN to be clocked off the SYSCLKOUT
	//
	ClkCfgRegs.CLKSRCCTL2.bit.CANABCLKSEL = 0;
	//
	// Set up the bit rate for the CAN bus.  This function sets up the CAN
	// bus timing for a nominal configuration.
	// In this example, the CAN bus is set to 500 kbps.
	//
	// Consult the TRM for more information about
	// CAN peripheral clocking.
	//
	canSetBitRate(200000000, 500000);

	canSetupObject(1, (0x600 + elmoNodeId),
				   CAN_MSG_FRAME_STD, MSG_OBJ_TYPE_TX);

	canSetupObject(2, (0x580 + elmoNodeId),
				   CAN_MSG_FRAME_STD, MSG_OBJ_TYPE_RX);
	//
	// Enable the CAN for operation.
	//
	CanaRegs.CAN_CTL.bit.Init = 0;
}

/*
 * 功能: 使能 Elmo 驱动器（通过写 SDO）
 */
void elmoEnable(void)
{

	elmoSendSDOWrite(0x3146, 1, 1, 4);
	Elmo_en = true;
}

/*
 * 功能: 失能 Elmo 驱动器（通过写 SDO）
 */
void elmoDisable(void)
{

	elmoSendSDOWrite(0x3146, 1, 0, 4);
	Elmo_en = false;
}

/*
 * 功能: 设置速度目标值（通过 SDO 写入）
 * 参数: spdVal - 目标速度值（单位取决于驱动器配置）
 */
void elmoSpdSet(int32_t spdVal)
{

	elmoSendSDOWrite(0x31e3, 1, spdVal, 4);
}

/*
 * 功能: 设置加速度（AC）参数
 * 参数: acVal - 加速度值
 */
void elmoAcSet(int32_t acVal)
{

	elmoSendSDOWrite(0x3002, 1, acVal, 4);
}

/*
 * 功能: 设置减速度（DC）参数
 * 参数: dcVal - 减速度值
 */
void elmoDcSet(int32_t dcVal)
{

	elmoSendSDOWrite(0x3050, 1, dcVal, 4);
}

/*
 * 功能: 设置相对位置目标并触发位置命令
 * 参数: posVal - 相对目标位置（单位由驱动器定义）
 */
void elmoRelPosSet(int32_t posVal)
{

	elmoSendSDOWrite(0x3197, 1, posVal, 4);
	elmoSendSDOWrite(0x3020, 1, 1, 4);
}

/*
 * 功能: 设置绝对位置目标并触发位置命令
 * 参数: posVal - 绝对目标位置值
 */
void elmoAbsPosSet(int32_t posVal)
{

	elmoSendSDOWrite(0x3186, 1, posVal, 4);
	elmoSendSDOWrite(0x3020, 1, 1, 4);
}

/*
 * 功能: 请求 Elmo 当前位置（发送 SDO 读取请求）
 */
void elmoPosRequest(void)
{

	elmoSendSDORequest(0x319d, 1, 0x40);
}

/*
 * 功能: 请求 Elmo 当前速度
 */
void elmoSpdRequest(void)
{

	elmoSendSDORequest(0x3239, 1, 0x40);
}

/*
 * 功能: 请求 Elmo 电流（Iq）值
 */
void elmoIqRequest(void)
{

	elmoSendSDORequest(0x30E0, 1, 0x40);
}

/*
 * 功能: 请求 Elmo 使能状态
 */
void elmoENRequest(void)
{

	elmoSendSDORequest(0x31e2, 1, 0x40);
}

/*
 * 功能: 请求 Elmo 错误代码（Error Code）
 */
void elmoECRequest(void)
{

	elmoSendSDORequest(0x306a, 1, 0x40);
}

/*
 * 功能: 处理来自 Elmo 的 SDO 响应或上报消息
 * 参数: msgId   - 接收到的 CAN 消息 ID
 *        msgData - 接收到的数据缓冲
 *        msgLen  - 数据长度
 * 返回: 无
 * 说明: 根据 SDO 响应的索引（index）解析并更新本地变量（位置/速度/电流/错误/使能）。
 */
void elmoProcess(uint32_t msgId, uint8_t *msgData, uint8_t msgLen)
{
	uint16_t index;
	int32_t Val;

	if ((msgData[0] & 0xE0) != 0x40)
	{
		return;
	}

	index = msgData[2];
	index <<= 8;
	index |= msgData[1];

	switch (index)
	{
	case 0x31e2:

		if (msgData[4] & 1)
		{
			Elmo_en = true;
		}
		else
		{
			Elmo_en = false;
		}

		break;

	case 0x319d:
		// 位置
	    Val = msgData[7];
	    Val <<= 8;
	    Val |= msgData[6];
	    Val <<= 8;
	    Val |= msgData[5];
	    Val <<= 8;
	    Val |= msgData[4];

	    g_bLock = true;
		pos_fed = Val;
		g_bLock = false;
		break;

	case 0x3239:
		// 速度
		spd_fed = msgData[7];
		spd_fed <<= 8;
		spd_fed |= msgData[6];
		spd_fed <<= 8;
		spd_fed |= msgData[5];
		spd_fed <<= 8;
		spd_fed |= msgData[4];
		break;

	case 0x30E0:
		// 电流
		Val = msgData[7];
		Val <<= 8;
		Val |= msgData[6];
		Val <<= 8;
		Val |= msgData[5];
		Val <<= 8;
		Val |= msgData[4];
		CastUnion converter;
		converter.i32Value = Val;
		// 直接读取浮点值
		iq_fed = fabsf(converter.f32Value);
		break;

	case 0x306a:
		Elmo_ec = msgData[5];
		Elmo_ec <<= 8;
		Elmo_ec |= msgData[4];
		break;

	default:
		break;
	}
}

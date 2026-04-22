#include "Core/inc/elmo_can.h"

#include <math.h>
#include <stdbool.h>

#include "board.h"

#define ELMO_CAN_RX_OBJ_ID (5U)

// Use multiple TX message objects to avoid overwriting a message object
// while its previous TX request is still pending.
#define ELMO_CAN_TX_OBJ_FIRST (1U)
#define ELMO_CAN_TX_OBJ_LAST (4U)
#define ELMO_CAN_TX_WAIT_STEP_US (5U)

// Elmo object dictionary indexes used in the legacy project.
#define ELMO_IDX_ENABLE (0x3146U)
#define ELMO_IDX_SPEED_CMD (0x31E3U)
#define ELMO_IDX_ACCEL_CMD (0x3002U)
#define ELMO_IDX_DECEL_CMD (0x3050U)
#define ELMO_IDX_REL_POS_CMD (0x3197U)
#define ELMO_IDX_ABS_POS_CMD (0x3186U)
#define ELMO_IDX_MOTION_TRIGGER (0x3020U)

#define ELMO_IDX_POS_FB (0x319DU)
#define ELMO_IDX_SPD_FB (0x3239U)
#define ELMO_IDX_IQ_FB (0x30E0U)
#define ELMO_IDX_ENABLE_FB (0x31E2U)
#define ELMO_IDX_ERR_FB (0x306AU)

static uint8_t g_elmoCanTxObjNext = ELMO_CAN_TX_OBJ_FIRST;

static inline uint32_t elmoCanObjBit(uint8_t objId)
{
	return (1UL << ((uint32_t)objId - 1UL));
}

// Try to pick a free TX message object (TXRQ bit cleared). Returns 0 if none free.
static uint8_t elmoCanPickFreeTxObj(uint32_t txRequests)
{
	const uint8_t count = (uint8_t)(ELMO_CAN_TX_OBJ_LAST - ELMO_CAN_TX_OBJ_FIRST + 1U);

	for (uint8_t k = 0U; k < count; k++)
	{
		uint8_t objId = (uint8_t)(g_elmoCanTxObjNext + k);
		if (objId > ELMO_CAN_TX_OBJ_LAST)
		{
			objId = (uint8_t)(ELMO_CAN_TX_OBJ_FIRST + (objId - ELMO_CAN_TX_OBJ_LAST - 1U));
		}

		if ((txRequests & elmoCanObjBit(objId)) == 0UL)
		{
			g_elmoCanTxObjNext = (uint8_t)(objId + 1U);
			if (g_elmoCanTxObjNext > ELMO_CAN_TX_OBJ_LAST)
			{
				g_elmoCanTxObjNext = ELMO_CAN_TX_OBJ_FIRST;
			}
			return objId;
		}
	}

	return 0U;
}

// Wait until any TX object becomes free, then return its objId.
static uint8_t elmoCanWaitFreeTxObj(void)
{
	int16_t loop_us = 5000;
	do
	{
		uint32_t txRequests = CAN_getTxRequests(Elmo_CAN_BASE);
		uint8_t objId = elmoCanPickFreeTxObj(txRequests);
		if (objId != 0U)
		{
			return objId;
		}
		DEVICE_DELAY_US(ELMO_CAN_TX_WAIT_STEP_US);
		loop_us -= ELMO_CAN_TX_WAIT_STEP_US;
	} while (loop_us > 0);
}

/// @brief 发送CAN消息。
/// @param msgData  消息数据指针
static void elmoCanSendObj(const uint8_t *msgData)
{
	uint8_t objId = elmoCanWaitFreeTxObj();
	CAN_sendMessage(Elmo_CAN_BASE, objId, 8U, (uint16_t *)msgData);
}

// 发送 SDO 读请求
static void elmoCanSendSDORequest(uint16_t index, uint8_t subIndex, uint8_t command)
{
	uint8_t txMsgData[8] = {0U};
	txMsgData[0] = command;
	txMsgData[1] = (uint8_t)(index & 0xFFU);
	txMsgData[2] = (uint8_t)((index >> 8U) & 0xFFU);
	txMsgData[3] = subIndex;
	elmoCanSendObj(txMsgData);
}

// 发送 SDO 写请求
static void elmoCanSendSDOWrite(uint16_t index, uint8_t subIndex, uint32_t value,
								uint8_t size)
{
	uint8_t txMsgData[8] = {0U};

	txMsgData[0] = 0x22U;
	txMsgData[1] = (uint8_t)(index & 0xFFU);
	txMsgData[2] = (uint8_t)((index >> 8U) & 0xFFU);
	txMsgData[3] = subIndex;

	if (size >= 1U)
	{
		txMsgData[4] = (uint8_t)(value & 0xFFU);
	}
	if (size >= 2U)
	{
		txMsgData[5] = (uint8_t)((value >> 8U) & 0xFFU);
	}
	if (size >= 3U)
	{
		txMsgData[6] = (uint8_t)((value >> 16U) & 0xFFU);
	}
	if (size >= 4U)
	{
		txMsgData[7] = (uint8_t)((value >> 24U) & 0xFFU);
	}

	elmoCanSendObj(txMsgData);
}
int32_t g_testVal = 0;
// 解析 Elmo 的 CAN 反馈数据
static void elmoCanProcess(const uint8_t *msgData, uint8_t msgLen)
{
	uint16_t index;
	int32_t val;

	if ((msgData == NULL) || (msgLen < 8U))
	{
		return;
	}

	if ((msgData[0] & 0xE0U) != 0x40U)
	{
		return;
	}

	index = (uint16_t)(((uint16_t)msgData[2] << 8U) | msgData[1]);

	switch (index)
	{
	case ELMO_IDX_ENABLE_FB:
		g_elmoParam.fb.en = ((msgData[4] & 0x1U) != 0U) ? 1U : 0U;
		break;

	case ELMO_IDX_POS_FB:
		val = (int32_t)(((uint32_t)msgData[7] << 24U) |
						((uint32_t)msgData[6] << 16U) |
						((uint32_t)msgData[5] << 8U) |
						msgData[4]);
		g_elmoParam.fb.pos_fed = val;
		break;

	case ELMO_IDX_SPD_FB:
		val = (int32_t)(((uint32_t)msgData[7] << 24U) |
						((uint32_t)msgData[6] << 16U) |
						((uint32_t)msgData[5] << 8U) |
						msgData[4]);
		g_elmoParam.fb.spd_fed = val;
		break;

	case ELMO_IDX_IQ_FB:
	{
		union
		{
			int32_t i32;
			float f32;
		} conv;

		conv.i32 = (int32_t)(((uint32_t)msgData[7] << 24U) |
							 ((uint32_t)msgData[6] << 16U) |
							 ((uint32_t)msgData[5] << 8U) |
							 msgData[4]);
		g_elmoParam.fb.iq_fed = fabsf(conv.f32);
		break;
	}
	case ELMO_IDX_ABS_POS_CMD:
		val = (int32_t)(((uint32_t)msgData[7] << 24U) |
						((uint32_t)msgData[6] << 16U) |
						((uint32_t)msgData[5] << 8U) |
						msgData[4]);
		g_testVal = val;
		break;

	case ELMO_IDX_ERR_FB:
		g_elmoParam.fb.ec = (int32_t)(((uint16_t)msgData[5] << 8U) | msgData[4]);
		break;

	default:
		break;
	}
}

// CAN 初始化（当前由 SysConfig 完成底层初始化）
static void elmoCanInit(void)
{
}

// CAN 主循环轮询（当前使用中断接收，轮询可留空）
static void elmoCanPoll(void)
{
}

// 下发使能命令
static void elmoCanEnable(void)
{
	elmoCanSendSDOWrite(ELMO_IDX_ENABLE, 1U, 1U, 4U);
	g_elmoParam.set.en = 1U;
}

// 下发关闭命令
static void elmoCanDisable(void)
{
	elmoCanSendSDOWrite(ELMO_IDX_ENABLE, 1U, 0U, 4U);
	g_elmoParam.set.en = 0U;
}

// 下发速度给定
static void elmoCanSetSpd(int32_t spdVal)
{
	g_elmoParam.set.spd_set = (uint32_t)spdVal;
	elmoCanSendSDOWrite(ELMO_IDX_SPEED_CMD, 1U, (uint32_t)spdVal, 4U);
}

// 下发加速度给定
static void elmoCanSetAc(int32_t acVal)
{
	g_elmoParam.set.ac_set = (uint32_t)acVal;
	elmoCanSendSDOWrite(ELMO_IDX_ACCEL_CMD, 1U, (uint32_t)acVal, 4U);
}

// 下发减速度给定
static void elmoCanSetDc(int32_t dcVal)
{
	g_elmoParam.set.dc_set = (uint32_t)dcVal;
	elmoCanSendSDOWrite(ELMO_IDX_DECEL_CMD, 1U, (uint32_t)dcVal, 4U);
}

// 下发相对位置给定
static void elmoCanSetRelPos(int32_t posVal)
{
	g_elmoParam.set.rel_pos_set = posVal;
	elmoCanSendSDOWrite(ELMO_IDX_REL_POS_CMD, 1U, (uint32_t)posVal, 4U);
	elmoCanSendSDOWrite(ELMO_IDX_MOTION_TRIGGER, 1U, 1U, 4U);
}

// 下发绝对位置给定
static void elmoCanSetAbsPos(int32_t posVal)
{
	g_elmoParam.set.abs_pos_set = posVal;
	static int32_t lastAbsPosSet = 0;
	if (posVal == lastAbsPosSet && fabs(g_elmoParam.fb.pos_fed - posVal) < 100)
	{
		return;
	}
	lastAbsPosSet = posVal;
	elmoCanSendSDOWrite(ELMO_IDX_ABS_POS_CMD, 1U, (uint32_t)posVal, 4U);
	elmoCanSendSDOWrite(ELMO_IDX_MOTION_TRIGGER, 1U, 1U, 4U);
	elmoCanSendSDORequest(ELMO_IDX_ABS_POS_CMD, 1U, 0x40U);
}

// 请求位置反馈
static void elmoCanReqPos(void)
{
	elmoCanSendSDORequest(ELMO_IDX_POS_FB, 1U, 0x40U);
}

// 请求速度反馈
static void elmoCanReqSpd(void)
{
	elmoCanSendSDORequest(ELMO_IDX_SPD_FB, 1U, 0x40U);
}

// 请求电流反馈
static void elmoCanReqIq(void)
{
	elmoCanSendSDORequest(ELMO_IDX_IQ_FB, 1U, 0x40U);
}

// 请求使能状态反馈
static void elmoCanReqEn(void)
{
	elmoCanSendSDORequest(ELMO_IDX_ENABLE_FB, 1U, 0x40U);
}

// 请求错误码反馈
static void elmoCanReqEc(void)
{
	elmoCanSendSDORequest(ELMO_IDX_ERR_FB, 1U, 0x40U);
}

// CAN 接收中断处理入口
static void elmoCanOnRxIsr(void)
{
	uint32_t cause = CAN_getInterruptCause(Elmo_CAN_BASE);

	if (cause == CAN_INT_INT0ID_STATUS)
	{
		(void)CAN_getStatus(Elmo_CAN_BASE);
	}
	else if (cause == ELMO_CAN_RX_OBJ_ID)
	{
		uint8_t msgData[8] = {0U};

		CAN_readMessage(Elmo_CAN_BASE, ELMO_CAN_RX_OBJ_ID, (uint16_t *)msgData);
		CAN_clearInterruptStatus(Elmo_CAN_BASE, ELMO_CAN_RX_OBJ_ID);
		elmoCanProcess(msgData, 8U);
	}
	else if ((cause >= ELMO_CAN_TX_OBJ_FIRST) && (cause <= ELMO_CAN_TX_OBJ_LAST))
	{
		// Clear TX interrupt sources if TX objects are configured to generate interrupts.
		CAN_clearInterruptStatus(Elmo_CAN_BASE, cause);
	}
	else
	{
		// No action for other sources on line0.
	}

	CAN_clearGlobalInterruptStatus(Elmo_CAN_BASE, CAN_GLOBAL_INT_CANINT0);
}

// CAN 操作函数表
static const ElmoOpsTable g_elmoCanOps = {
	.init = elmoCanInit,
	.poll = elmoCanPoll,
	.enable = elmoCanEnable,
	.disable = elmoCanDisable,
	.setSpd = elmoCanSetSpd,
	.setAc = elmoCanSetAc,
	.setDc = elmoCanSetDc,
	.setRelPos = elmoCanSetRelPos,
	.setAbsPos = elmoCanSetAbsPos,
	.reqPos = elmoCanReqPos,
	.reqSpd = elmoCanReqSpd,
	.reqIq = elmoCanReqIq,
	.reqEn = elmoCanReqEn,
	.reqEc = elmoCanReqEc,
	.onCanRxIsr = elmoCanOnRxIsr,
};

// CAN 描述对象
static const ElmoBackend g_elmoCanBackend = {
	.name = "elmo_can",
	.ops = &g_elmoCanOps};

// 获取 CAN 描述对象
const ElmoBackend *ElmoCan_GetBackend(void)
{
	return &g_elmoCanBackend;
}

#include "Core/inc/elmo_ctrl.h"

#include <stddef.h>

#include "Core/inc/elmo_can.h"
#include "Core/inc/elmo_rs232.h"
#include "Core/inc/glob_cfg.h"

#define ELMO_INVALID_VALUE (0x7FFFFFFF)

// elmo操作表指针（对外直接使用）
ElmoOpsTable ElmoOps = {
#if (ELMO_CONTROL_IF == ELMO_IF_CAN)
	.ctrl = &ElmoCanCtrl,
#else
	.ctrl = &ElmoRs232Ctrl,
#endif
	.fb = {
		.pos_fed = 0,
		.spd_fed = 0,
		.iq_fed = 0.0f,
		.ec = 0,
		.en = 0U,
		.abs_pos_set = 0,
		.rel_pos_set = 0,
		.ac_set = 0,
		.dc_set = 0,
		.detect = 0,
	},
	.set = {
		.en = 0U,
		.spd_set = 2000000U,
		.ac_set = 60000000U,
		.dc_set = 60000000U,
		.rel_pos_set = 0,
		.abs_pos_set = 0,
	},
};

static void elmoSetEnable(uint8_t enable)
{
	if (ElmoOps.set.en == enable && ElmoOps.fb.en == enable)
	{
		return;
	}
	ElmoOps.set.en = enable;
	if (ElmoOps.ctrl->setEnable != NULL)
	{
		ElmoOps.ctrl->setEnable(enable);
	}
	if (ElmoOps.ctrl->reqEn)
	{
		ElmoOps.ctrl->reqEn();
	}
	if (enable == 0)
	{
		ElmoOps.set.abs_pos_set = ELMO_INVALID_VALUE;
		ElmoOps.set.rel_pos_set = ELMO_INVALID_VALUE;
		ElmoOps.fb.abs_pos_set = ELMO_INVALID_VALUE;
		ElmoOps.fb.rel_pos_set = ELMO_INVALID_VALUE;
	}
}

static void elmoStop(void)
{
	if (ElmoOps.ctrl->stop != NULL)
	{
		ElmoOps.ctrl->stop();
	}
	ElmoOps.set.abs_pos_set = ELMO_INVALID_VALUE;
	ElmoOps.set.rel_pos_set = ELMO_INVALID_VALUE;
	ElmoOps.fb.abs_pos_set = ELMO_INVALID_VALUE;
	ElmoOps.fb.rel_pos_set = ELMO_INVALID_VALUE;
}

/// @brief 设置速度
/// @param spdVal
static void elmoSetSpd(int32_t spdVal)
{
	ElmoOps.set.spd_set = spdVal;
	if (ElmoOps.ctrl->setSpd != NULL)
	{
		ElmoOps.ctrl->setSpd(spdVal);
	}
}

/// @brief 设置绝对位置 (如果给定值与当前反馈值接近则不发送命令)
/// @param posVal
static void elmoSetAbsPos(int32_t posVal)
{
	if (ElmoOps.set.abs_pos_set == posVal && ElmoOps.set.abs_pos_set == ElmoOps.fb.abs_pos_set) // 值没有变化
	{
		return;
	}
	ElmoOps.set.abs_pos_set = posVal;
	if (ElmoOps.ctrl->setAbsPos != NULL)
	{
		ElmoOps.ctrl->setAbsPos(posVal); // 设置绝对位置
	}
	if (ElmoOps.ctrl->reqSetAbsPos != NULL)
	{
		ElmoOps.ctrl->reqSetAbsPos(); // 请求绝对位置给定反馈
	}
}

/// @brief 设置绝对位置（直接设置 不做反馈检测）
/// @param posVal
static void elmosetAbsPosIsr(int32_t posVal)
{
	ElmoOps.set.abs_pos_set = posVal;
	if (ElmoOps.ctrl->setAbsPosIsr != NULL)
	{
		ElmoOps.ctrl->setAbsPosIsr(posVal);
	}
}

/// @brief 设置相对位置
/// @param posVal
static void elmoSetRelPos(int32_t posVal)
{
	ElmoOps.set.rel_pos_set = posVal;
	if (ElmoOps.ctrl->setRelPos != NULL)
	{
		ElmoOps.ctrl->setRelPos(posVal);
	}
}

/// @brief 设置加速度
/// @param acVal
static void elmoSetAc(int32_t acVal)
{
	ElmoOps.set.ac_set = acVal;
	if (ElmoOps.ctrl->setAc != NULL)
	{
		ElmoOps.ctrl->setAc(acVal);
	}
}

/// @brief 设置减速度
/// @param dcVal
static void elmoSetDc(int32_t dcVal)
{
	ElmoOps.set.dc_set = dcVal;
	if (ElmoOps.ctrl->setDc != NULL)
	{
		ElmoOps.ctrl->setDc(dcVal);
	}
}

static void elmoParse(void)
{
	if (ElmoOps.ctrl->Parse != NULL)
	{
		ElmoOps.ctrl->Parse(&ElmoOps.fb);
	}
}

static void elmoParseIsr(void)
{
	if (ElmoOps.ctrl->ParseIsr != NULL)
	{
		ElmoOps.ctrl->ParseIsr(&ElmoOps.fb);
	}
}

/// @brief // Elmo 控制器初始化，完成函数指针绑定
/// @param
void ElmoCtrl_Init(void)
{
	if (ElmoOps.ctrl != NULL)
	{
		ElmoOps.reqPos = ElmoOps.ctrl->reqPos;
		ElmoOps.reqSpd = ElmoOps.ctrl->reqSpd;
		ElmoOps.reqIq = ElmoOps.ctrl->reqIq;
		ElmoOps.reqEn = ElmoOps.ctrl->reqEn;
		ElmoOps.reqEc = ElmoOps.ctrl->reqEc;
		ElmoOps.reqSetStopDc = ElmoOps.ctrl->reqSetStopDc;
	}
	ElmoOps.stop = elmoStop;
	ElmoOps.setEnable = elmoSetEnable;
	ElmoOps.setSpd = elmoSetSpd;
	ElmoOps.setAbsPos = elmoSetAbsPos;
	ElmoOps.setAbsPosIsr = elmosetAbsPosIsr;
	ElmoOps.setRelPos = elmoSetRelPos;
	ElmoOps.setAc = elmoSetAc;
	ElmoOps.setDc = elmoSetDc;
	ElmoOps.Parse = elmoParse;
	ElmoOps.ParseIsr = elmoParseIsr;
	// ElmoOps.reqSetStopDc();
}

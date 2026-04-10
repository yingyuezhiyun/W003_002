#include "Core/inc/elmo_ctrl.h"

#include <stddef.h>

#include "Core/inc/elmo_can.h"
#include "Core/inc/elmo_rs232.h"
#include "Core/inc/glob_cfg.h"

// 全局 Elmo 参数
ElmoParam g_elmoParam = {
	.set = {
		.spd_set = 2000000U,
		.ac_set = 60000000U,
		.dc_set = 60000000U,
		.rel_pos_set = 0,
		.abs_pos_set = 0,
	},
	.fb = {
		.pos_fed = 0,
		.spd_fed = 0,
		.iq_fed = 0.0f,
		.elmo_ec = 0,
		.elmo_en = 0U,
	}
};

// 当前描述指针
static const ElmoBackend *g_backend = NULL;
// 当前操作表指针（对外直接使用）
const ElmoOpsTable *ElmoOps = NULL;

// 根据 ID 获取对象
static const ElmoBackend *elmoGetBackendById(ElmoBackendId backendId)
{
	switch(backendId)
	{
		case ELMO_BACKEND_CAN:
			return ElmoCan_GetBackend();

		case ELMO_BACKEND_RS232:
			return ElmoRs232_GetBackend();

		default:
			return ElmoCan_GetBackend();
	}
}

// 获取 Elmo 参数池
ElmoParam *ElmoCtrl_GetParam(void)
{
	return &g_elmoParam;
}

// 选择通信并刷新 ElmoOps 指针
void ElmoCtrl_SelectBackend(ElmoBackendId backendId)
{
	g_backend = elmoGetBackendById(backendId);
	ElmoOps = (g_backend != NULL) ? g_backend->ops : NULL;
}

// 依据配置宏选择默认
void ElmoCtrl_SelectDefault(void)
{
	ElmoCtrl_SelectBackend((ElmoBackendId)ELMO_CONTROL_IF);
	if((ElmoOps != NULL) && (ElmoOps->init != NULL))
	{
		ElmoOps->init();
	}
}

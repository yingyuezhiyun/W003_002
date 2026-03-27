#include "Core/inc/elmo_rs232.h"

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "board.h"

#define ELMO_RS232_RX_BUF_SIZE (96U)

// RS232 接收缓存区
static char g_elmoRxBuf[ELMO_RS232_RX_BUF_SIZE];
// RS232 接收缓存长度
static uint16_t g_elmoRxLen = 0U;

// 通过 Elmo_SCI 发送字符串命令
static void elmoRs232Send(const char *cmd)
{
	const char *p = cmd;

	while(*p != '\0')
	{
		SCI_writeCharBlockingFIFO(Elmo_SCI_BASE, (uint16_t)(uint8_t)(*p));
		p++;
	}
}

// 组装带整型参数的命令字符串
static int16_t buildCmd(char *dst, uint16_t dstLen, const char *prefix,
						int32_t value, const char *suffix)
{
	uint16_t i = 0U;
	int16_t written;

	while((prefix[i] != '\0') && (i < (dstLen - 1U)))
	{
		dst[i] = prefix[i];
		i++;
	}

	written = (int16_t)snprintf(&dst[i], dstLen - i, "%ld%s", (long)value, suffix);
	if(written < 0)
	{
		return -1;
	}

	return (int16_t)(i + written);
}

// 解析 Elmo 的串口回包
static void elmoRs232ParseLine(char *line)
{
	char *eq = strchr(line, '=');
	char *payload = (eq == NULL) ? NULL : (eq + 1);

	if(payload == NULL)
	{
		return;
	}

	if(strncmp(line, "PX", 2) == 0)
	{
		g_elmoParam.fb.pos_fed = (int32_t)strtol(payload, NULL, 10);
	}
	else if(strncmp(line, "VX", 2) == 0)
	{
		g_elmoParam.fb.spd_fed = (int32_t)labs(strtol(payload, NULL, 10));
	}
	else if(strncmp(line, "IQ", 2) == 0)
	{
		float iq = (float)atof(payload);
		g_elmoParam.fb.iq_fed = fabsf(iq);
	}
	else if(strncmp(line, "SO", 2) == 0)
	{
		g_elmoParam.fb.elmo_en = (strtol(payload, NULL, 10) != 0) ? 1U : 0U;
	}
	else if(strncmp(line, "EC", 2) == 0)
	{
		g_elmoParam.fb.elmo_ec = (int32_t)strtol(payload, NULL, 10);
	}
}

// RS232 后端初始化
static void elmoRs232Init(void)
{
	g_elmoRxLen = 0U;
	memset(g_elmoRxBuf, 0, sizeof(g_elmoRxBuf));
}

// RS232 后端主循环轮询
static void elmoRs232Poll(void)
{
	while(SCI_getRxFIFOStatus(Elmo_SCI_BASE) != SCI_FIFO_RX0)
	{
		uint16_t c16 = SCI_readCharNonBlocking(Elmo_SCI_BASE);
		char c = (char)(c16 & 0xFFU);

		if((c == '\n') || (c == '\r') || (c == ';'))
		{
			if(g_elmoRxLen > 0U)
			{
				g_elmoRxBuf[g_elmoRxLen] = '\0';
				elmoRs232ParseLine(g_elmoRxBuf);
				g_elmoRxLen = 0U;
			}
			continue;
		}

		if(g_elmoRxLen < (ELMO_RS232_RX_BUF_SIZE - 1U))
		{
			g_elmoRxBuf[g_elmoRxLen++] = c;
		}
		else
		{
			g_elmoRxLen = 0U;
		}
	}
}

// 下发使能命令
static void elmoRs232Enable(void)
{
	elmoRs232Send("MO=1;\r");
	g_elmoParam.fb.elmo_en = 1U;
}

// 下发去使能命令
static void elmoRs232Disable(void)
{
	elmoRs232Send("MO=0;\r");
	g_elmoParam.fb.elmo_en = 0U;
}

// 下发速度给定
static void elmoRs232SpdSet(int32_t spdVal)
{
	char cmd[24];
	g_elmoParam.set.spd_set = (uint32_t)spdVal;

	(void)buildCmd(cmd, sizeof(cmd), "SP=", spdVal, ";\r");
	elmoRs232Send(cmd);
}

// 下发加速度给定
static void elmoRs232AcSet(int32_t acVal)
{
	char cmd[24];
	g_elmoParam.set.ac_set = (uint32_t)acVal;

	(void)buildCmd(cmd, sizeof(cmd), "AC=", acVal, ";\r");
	elmoRs232Send(cmd);
}

// 下发减速度给定
static void elmoRs232DcSet(int32_t dcVal)
{
	char cmd[24];
	g_elmoParam.set.dc_set = (uint32_t)dcVal;

	(void)buildCmd(cmd, sizeof(cmd), "DC=", dcVal, ";\r");
	elmoRs232Send(cmd);
}

// 下发相对位置给定
static void elmoRs232RelPosSet(int32_t posVal)
{
	char cmd[28];
	g_elmoParam.set.rel_pos_set = posVal;

	(void)buildCmd(cmd, sizeof(cmd), "PR=", posVal, ";BG;\r");
	elmoRs232Send(cmd);
}

// 下发绝对位置给定
static void elmoRs232AbsPosSet(int32_t posVal)
{
	char cmd[28];
	g_elmoParam.set.abs_pos_set = posVal;

	(void)buildCmd(cmd, sizeof(cmd), "PA=", posVal, ";BG;\r");
	elmoRs232Send(cmd);
}

// 请求位置反馈
static void elmoRs232PosRequest(void)
{
	elmoRs232Send("PX;\r");
}

// 请求速度反馈
static void elmoRs232SpdRequest(void)
{
	elmoRs232Send("VX;\r");
}

// 请求电流反馈
static void elmoRs232IqRequest(void)
{
	elmoRs232Send("IQ;\r");
}

// 请求使能状态反馈
static void elmoRs232ENRequest(void)
{
	elmoRs232Send("SO;\r");
}

// 请求错误码反馈
static void elmoRs232ECRequest(void)
{
	elmoRs232Send("EC;\r");
}

// RS232 后端操作函数表
static const ElmoOpsTable g_elmoRs232Ops = {
	.init = elmoRs232Init,
	.poll = elmoRs232Poll,
	.enable = elmoRs232Enable,
	.disable = elmoRs232Disable,
	.setSpd = elmoRs232SpdSet,
	.setAc = elmoRs232AcSet,
	.setDc = elmoRs232DcSet,
	.setRelPos = elmoRs232RelPosSet,
	.setAbsPos = elmoRs232AbsPosSet,
	.reqPos = elmoRs232PosRequest,
	.reqSpd = elmoRs232SpdRequest,
	.reqIq = elmoRs232IqRequest,
	.reqEn = elmoRs232ENRequest,
	.reqEc = elmoRs232ECRequest,
	.onCanRxIsr = NULL,
};

// RS232 后端描述对象
static const ElmoBackend g_elmoRs232Backend = {
	.name = "elmo_rs232",
	.ops = &g_elmoRs232Ops
};

// 获取 RS232 后端描述对象
const ElmoBackend *ElmoRs232_GetBackend(void)
{
	return &g_elmoRs232Backend;
}

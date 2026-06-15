#include "Core/inc/elmo_rs232.h"

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "device.h"
#include "driverlib.h"

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

	while (*p != '\0')
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

	while ((prefix[i] != '\0') && (i < (dstLen - 1U)))
	{
		dst[i] = prefix[i];
		i++;
	}

	written = (int16_t)snprintf(&dst[i], dstLen - i, "%ld%s", (long)value, suffix);
	if (written < 0)
	{
		return -1;
	}

	return (int16_t)(i + written);
}

// 解析 Elmo 的串口回包
static void elmoRs232ParseLine(char *line, ElmoFeedbackParam *fb)
{
	char *eq = strchr(line, '=');
	char *payload = (eq == NULL) ? NULL : (eq + 1);

	if (payload == NULL)
	{
		return;
	}

	if (strncmp(line, "PX", 2) == 0)
	{
		fb->pos_fed = (int32_t)strtol(payload, NULL, 10);
	}
	else if (strncmp(line, "VX", 2) == 0)
	{
		fb->spd_fed = (int32_t)labs(strtol(payload, NULL, 10));
	}
	else if (strncmp(line, "IQ", 2) == 0)
	{
		float iq = (float)atof(payload);
		fb->iq_fed = iq;
	}
	else if (strncmp(line, "SO", 2) == 0)
	{
		fb->en = (strtol(payload, NULL, 10) != 0) ? 1U : 0U;
		fb->detect = 1U;
	}
	else if (strncmp(line, "EC", 2) == 0)
	{
		fb->ec = (int32_t)strtol(payload, NULL, 10);
	}
	else if (strncmp(line, "PS", 2) == 0)
	{
		fb->spd_set = (int32_t)strtol(payload, NULL, 10);
	}
	else if (strncmp(line, "AC", 2) == 0)
	{
		fb->ac_set = (int32_t)strtol(payload, NULL, 10);
	}
	else if (strncmp(line, "DC", 2) == 0)
	{
		fb->dc_set = (int32_t)strtol(payload, NULL, 10);
	}
	else if (strncmp(line, "PR", 2) == 0)
	{
		fb->rel_pos_set = (int32_t)strtol(payload, NULL, 10);
	}
	else if (strncmp(line, "PA", 2) == 0)
	{
		fb->abs_pos_set = (int32_t)strtol(payload, NULL, 10);
	}
	else if (strncmp(line, "SD", 2) == 0)
	{
		fb->stop_dc_set = (int32_t)strtol(payload, NULL, 10);
	}
}

// RS232 初始化
static void elmoRs232Init(void)
{
	g_elmoRxLen = 0U;
	memset(g_elmoRxBuf, 0, sizeof(g_elmoRxBuf));
}

// RS232 主循环轮询
static void elmoRs232Poll(ElmoFeedbackParam *fb)
{
	while (SCI_getRxFIFOStatus(Elmo_SCI_BASE) != SCI_FIFO_RX0)
	{
		uint16_t c16 = SCI_readCharNonBlocking(Elmo_SCI_BASE);
		char c = (char)(c16 & 0xFFU);

		if ((c == '\n') || (c == '\r') || (c == ';'))
		{
			if (g_elmoRxLen > 0U)
			{
				g_elmoRxBuf[g_elmoRxLen] = '\0';
				elmoRs232ParseLine(g_elmoRxBuf, fb);
				g_elmoRxLen = 0U;
			}
			continue;
		}

		if (g_elmoRxLen < (ELMO_RS232_RX_BUF_SIZE - 1U))
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
static void elmoRs232SetEnable(uint8_t enable)
{
	char cmd[16];
	(void)buildCmd(cmd, sizeof(cmd), "MO=", enable ? 1 : 0, ";\r");
	elmoRs232Send(cmd);
}

// 下发速度给定
static void elmoRs232SpdSet(int32_t spdVal)
{
	char cmd[24];
	(void)buildCmd(cmd, sizeof(cmd), "SP=", spdVal, ";\r");
	elmoRs232Send(cmd);
}

// 下发加速度给定
static void elmoRs232AcSet(int32_t acVal)
{
	char cmd[24];
	(void)buildCmd(cmd, sizeof(cmd), "AC=", acVal, ";\r");
	elmoRs232Send(cmd);
}

// 下发减速度给定
static void elmoRs232DcSet(int32_t dcVal)
{
	char cmd[24];
	(void)buildCmd(cmd, sizeof(cmd), "DC=", dcVal, ";\r");
	elmoRs232Send(cmd);
}

// 下发相对位置给定
static void elmoRs232RelPosSet(int32_t posVal)
{
	char cmd[28];
	(void)buildCmd(cmd, sizeof(cmd), "PR=", posVal, ";BG;\r");
	elmoRs232Send(cmd);
}

// 下发绝对位置给定
static void elmoRs232AbsPosSet(int32_t posVal)
{
	char cmd[28];
	(void)buildCmd(cmd, sizeof(cmd), "PA=", posVal, ";BG;\r");
	elmoRs232Send(cmd);
}

static void elmoRs232SetStopDc(int32_t dcVal)
{
	char cmd[24];
	(void)buildCmd(cmd, sizeof(cmd), "SD=", dcVal, ";\r");
	elmoRs232Send(cmd);
}


// 停止
static void elmoRs232Stop(void)
{
	elmoRs232Send("ST;\r");
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

static void elmoRs232ReqSetRelPos(void)
{
	elmoRs232Send("PR;\r");
}

static void elmoRs232ReqSetAbsPos(void)
{
	elmoRs232Send("PA;\r");
}

static void elmoRs232ReqSetSpd(void)
{
	elmoRs232Send("PS;\r");
}

static void elmoRs232ReqSetAc(void)
{
	elmoRs232Send("AC;\r");
}

static void elmoRs232ReqSetDc(void)
{
	elmoRs232Send("DC;\r");
}

static void elmoRs232ReqSetStopDc(void)
{
	elmoRs232Send("SD;\r");
}

ElmoCtrl ElmoRs232Ctrl = {

	.setEnable = elmoRs232SetEnable,
	.setSpd = elmoRs232SpdSet,
	.setAc = elmoRs232AcSet,
	.setDc = elmoRs232DcSet,
	.setRelPos = elmoRs232RelPosSet,
	.setAbsPos = elmoRs232AbsPosSet,
	.setAbsPosIsr = elmoRs232AbsPosSet,
	.setStopDc = elmoRs232SetStopDc,
	.stop = elmoRs232Stop,
	.reqSetSpd = elmoRs232ReqSetSpd,
	.reqSetAc = elmoRs232ReqSetAc,
	.reqSetDc = elmoRs232ReqSetDc,
	.reqSetRelPos = elmoRs232ReqSetRelPos,
	.reqSetAbsPos = elmoRs232ReqSetAbsPos,
	.reqSetStopDc = elmoRs232ReqSetStopDc,
	.reqPos = elmoRs232PosRequest,
	.reqPosIsr = elmoRs232PosRequest,
	.reqSpd = elmoRs232SpdRequest,
	.reqIq = elmoRs232IqRequest,
	.reqEn = elmoRs232ENRequest,
	.reqEc = elmoRs232ECRequest,
	.ParseIsr = NULL,
	.Parse = elmoRs232Poll,
};

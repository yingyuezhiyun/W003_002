/*******************************************************************************
 PIC32 SPI Interface Driver

  Company:
	Microchip Technology Inc.

  File Name:
	SPIDriver.h

  Summary:
	Contains the Header File of PIC32 SPI Interface Driver

  Description:
	This file contains the Header File of PIC32 SPI Interface Driver

  Change History:
	Version		Changes
	0.1			Initial version.
	0.2			-
	0.3			-
	0.4 		-
*******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
 Copyright (c) 2015 released Microchip Technology Inc.  All rights reserved.

 Microchip licenses to you the right to use, modify, copy and distribute
 Software only when embedded on a Microchip microcontroller or digital signal
 controller that is integrated into your product or third party product
 (pursuant to the sublicense terms in the accompanying license agreement).

 You should refer to the license agreement accompanying this Software for
 additional information regarding your rights and obligations.

 SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
 MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
 IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
 CONTRACT, NEGLiPMPCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
 OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
 INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
 CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
 SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
 (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *******************************************************************************/
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#ifndef SPIDRIVER_H
#define SPIDRIVER_H

// TI C2000 (TMS320F28377D) SPIA + GPIO based LAN9252 SPI driver
// #include "F28x_Project.h"
#include "driverlib.h"
#include "board.h"
#include "src/ecat_def.h"

// NOTE (C28x): TI C2000 compiler uses 16-bit 'char' by default.
// The SSC types map UINT8 to 'unsigned char', so UINT8 is 16-bit here.
// This driver treats each UINT8 element as a logical byte stored in the low 8 bits.
// Do NOT use byte-overlay unions (UINT32_VAL/UINT16_VAL) in this environment.

#ifdef __cplusplus
extern "C"
{
#endif
// *****************************************************************************
// *****************************************************************************
// Section: File Scope or Global Data Types
// *****************************************************************************
// *****************************************************************************
#define CMD_SERIAL_READ 0x03
#define CMD_FAST_READ 0x0B
#define CMD_DUAL_OP_READ 0x3B
#define CMD_DUAL_IO_READ 0xBB
#define CMD_QUAD_OP_READ 0x6B
#define CMD_QUAD_IO_READ 0xEB
#define CMD_SERIAL_WRITE 0x02
#define CMD_DUAL_DATA_WRITE 0x32
#define CMD_DUAL_ADDR_DATA_WRITE 0xB2
#define CMD_QUAD_DATA_WRITE 0x62
#define CMD_QUAD_ADDR_DARA_WRITE 0xE2

#define CMD_SERIAL_READ_DUMMY 0
#define CMD_FAST_READ_DUMMY 1
#define CMD_DUAL_OP_READ_DUMMY 1
#define CMD_DUAL_IO_READ_DUMMY 2
#define CMD_QUAD_OP_READ_DUMMY 1
#define CMD_QUAD_IO_READ_DUMMY 4
#define CMD_SERIAL_WRITE_DUMMY 0
#define CMD_DUAL_DATA_WRITE_DUMMY 0
#define CMD_DUAL_ADDR_DATA_WRITE_DUMMY 0
#define CMD_QUAD_DATA_WRITE_DUMMY 0
#define CMD_QUAD_ADDR_DARA_WRITE_DUMMY 0

#define ESC_CSR_CMD_REG 0x304
#define ESC_CSR_DATA_REG 0x300
#define ESC_WRITE_BYTE 0x80
#define ESC_READ_BYTE 0xC0
#define ESC_CSR_BUSY 0x80

// LAN9252 identification registers (direct SPI)
#define LAN9252_CHIP_ID_REG 0x50
#define LAN9252_BYTE_TEST_REG 0x64
#define LAN9252_HW_CFG 0x74
#define LAN9252_CHIP_ID_VALUE 0x00009252UL
#define LAN9252_BYTE_TEST_VALUE 0x87654321UL

// Project assumes 200MHz CPU, LSPCLK=CPU/4=50MHz (see drive/SCI.h)
#ifndef ECAT_LSPCLK_HZ
#define ECAT_LSPCLK_HZ (50000000UL)
#endif

// LAN9252 supports up to 20MHz SPI, but with LSPCLK=50MHz max practical is 12.5MHz.
#ifndef ECAT_SPI_HZ
// #define ECAT_SPI_HZ (12500000UL)
#define ECAT_SPI_HZ (1000000UL)
#endif

#define ECAT_SPI_BRR ((ECAT_LSPCLK_HZ / (ECAT_SPI_HZ)) - 1U)

#define SPI_MODE0 0
#define SPI_MODE1 1
#define SPI_MODE2 2
#define SPI_MODE3 3

// LAN9252 supports SPI mode 0 or 3. Default matches reference (mode 2) unless overridden.
#ifndef ECAT_SPI_MODE
#define ECAT_SPI_MODE SPI_MODE2
#endif

// -------- LAN9252 wiring on this project --------
// NOTE: F28377D pinmux table (see tools/pinmux_lines*.txt) allows SPIA on:
//  - GPIO16/17/18/(19) with mux=1
//  - GPIO54/55/56/(57) with mux=1
//  - GPIO58/59/60/(61) with mux=15 (GMUX=3, MUX=3)
// This project wires LAN9252 to GPIO58/59/60 and uses a manual CS on GPIO61.
#ifndef ECAT_SPI_PINSET_GPIO58
#define ECAT_SPI_PINSET_GPIO58 1
#endif

#ifndef ECAT_SPI_GPIO_SIMO
#define ECAT_SPI_GPIO_SIMO 58U
#endif
#ifndef ECAT_SPI_GPIO_SOMI
#define ECAT_SPI_GPIO_SOMI 59U
#endif
#ifndef ECAT_SPI_GPIO_CLK
#define ECAT_SPI_GPIO_CLK 60U
#endif

// Prefer SysCfg generated GPIO numbers when available
#ifndef ECAT_SPI_GPIO_CS
#define ECAT_SPI_GPIO_CS ((uint32_t)ECAT_SPI_CS)
#endif
#ifndef ECAT_LAN9252_IRQ_GPIO
#define ECAT_LAN9252_IRQ_GPIO ((uint32_t)ECAT_ISR)
#endif
#ifndef ECAT_LAN9252_SYNC0_GPIO
#define ECAT_LAN9252_SYNC0_GPIO ((uint32_t)ECAT_SYNC0_ISR)
#endif
#ifndef ECAT_LAN9252_SYNC1_GPIO
#define ECAT_LAN9252_SYNC1_GPIO ((uint32_t)ECAT_SYNC1_ISR)
#endif
#ifndef ECAT_LAN9252_RST_GPIO
#define ECAT_LAN9252_RST_GPIO ((uint32_t)ECAT_EN)
#endif

static inline void ECAT_CS_LOW(void)
{
	GPIO_writePin((uint32_t)ECAT_SPI_CS, 0U);
}
static inline void ECAT_CS_HIGH(void)
{
	GPIO_writePin((uint32_t)ECAT_SPI_CS, 1U);
}

// Kept for source compatibility with SSC-generated code
#define CSLOW() ECAT_CS_LOW()
#define CSHIGH() ECAT_CS_HIGH()

#define SPIWriteByte(_data) SPIWrite(_data)
#define SPIReadByte() SPIRead()

	// *****************************************************************************
	// *****************************************************************************
	// Section: File Scope Functions
	// *****************************************************************************
	// *****************************************************************************

	void SPIWritePDRamRegister(UINT8 *WriteBuffer, UINT16 Address, UINT16 Count);
	void SPIReadPDRamRegister(UINT8 *ReadBuffer, UINT16 Address, UINT16 Count);
	void SPIWriteRegister(UINT8 *WriteBuffer, UINT16 Address, UINT16 Count);
	void SPIReadDRegister(UINT8 *ReadBuffer, UINT16 Address, UINT16 Count);
	void SPIReadRegUsingCSR(UINT8 *ReadBuffer, UINT16 Address, UINT8 Count);
	void SPIWriteRegUsingCSR(UINT8 *WriteBuffer, UINT16 Address, UINT8 Count);
	void SPIWriteDWord(UINT16 Address, UINT32 Val);
	UINT32 SPIReadDWord(UINT16 Address);
	void SPIOpen();
	UINT8 SPIRead();
	void SPIWrite(UINT8 data);

	void SPIWriteBurstMode(UINT32 Val);
	UINT32 SPIReadBurstMode();
	void SPISendAddr(UINT16 Address);
	


#ifdef __cplusplus
}
#endif

#endif /* PMPDRIVER_H */

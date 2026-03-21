/*
 * TI C2000 (F28377D) LAN9252 SPI PDI driver
 *
 * This file provides the low-level SPI transactions needed by the SSC LAN9252
 * port (CSR + PRAM FIFO). It is intentionally written without PIC32-specific
 * unions/bitfields.
 *
 * Important: On C28x, 'char' is 16-bit; SSC's UINT8 therefore stores a logical
 * byte in the low 8 bits of a 16-bit element. All functions below treat each
 * UINT8 element as one logical byte.
 */

#include "SPIDriver.h"
#include "9252_HW.h"

#ifndef ECAT_SPI_BASE
#define ECAT_SPI_BASE mySPI0_BASE
#endif

#define ECAT_SPI_XFER8(_tx) ((uint16_t)SPI_pollingNonFIFOTransaction(ECAT_SPI_BASE, 8U, (uint16_t)((_tx) & 0xFFU)))

static inline UINT8 ecat_u8(uint16_t v)
{
  return (UINT8)(v & 0x00FFU);
}

static inline uint16_t lo8(UINT8 v)
{
  return ((uint16_t)v) & 0x00FFU;
}

static UINT32 SPIReadDWord_NoCS(UINT16 Address);
static void SPIWriteDWord_NoCS(UINT16 Address, UINT32 Val);

static UINT32 SPIReadDWord_NoCS(UINT16 Address)
{
  UINT32 result;
  uint16_t b0, b1, b2, b3;

  (void)ECAT_SPI_XFER8(CMD_FAST_READ);
  (void)ECAT_SPI_XFER8((Address >> 8) & 0xFFU);
  (void)ECAT_SPI_XFER8(Address & 0xFFU);
  (void)ECAT_SPI_XFER8(CMD_FAST_READ_DUMMY);
	
  b0 = ECAT_SPI_XFER8(0xFFU);
  b1 = ECAT_SPI_XFER8(0xFFU);
  b2 = ECAT_SPI_XFER8(0xFFU);
  b3 = ECAT_SPI_XFER8(0xFFU);

  result = ((UINT32)(b0 & 0xFFU)) |
       ((UINT32)(b1 & 0xFFU) << 8) |
       ((UINT32)(b2 & 0xFFU) << 16) |
       ((UINT32)(b3 & 0xFFU) << 24);
  return result;
}

static void SPIWriteDWord_NoCS(UINT16 Address, UINT32 Val)
{
  (void)ECAT_SPI_XFER8(CMD_SERIAL_WRITE);
  (void)ECAT_SPI_XFER8((Address >> 8) & 0xFFU);
  (void)ECAT_SPI_XFER8(Address & 0xFFU);
  (void)ECAT_SPI_XFER8((uint16_t)(Val & 0xFFU));
  (void)ECAT_SPI_XFER8((uint16_t)((Val >> 8) & 0xFFU));
  (void)ECAT_SPI_XFER8((uint16_t)((Val >> 16) & 0xFFU));
  (void)ECAT_SPI_XFER8((uint16_t)((Val >> 24) & 0xFFU));
}

/*******************************************************************************
  Function:
	UINT32 SPIReadDWord (UINT16 Address)
  Summary:
    This function reads the LAN9252 CSR registers.        
  
*****************************************************************************/
UINT32 SPIReadDWord (UINT16 Address)
{
  UINT32 result;
  CSLOW();
  result = SPIReadDWord_NoCS(Address);
  CSHIGH();
  return result;
}

/*******************************************************************************
  Function:
	void SPISendAddr (UINT16 Address)
  Summary:
    This function write address to SPI data bus.        
  
*****************************************************************************/
void SPISendAddr (UINT16 Address)
{
  (void)ECAT_SPI_XFER8((Address >> 8) & 0xFFU);
  (void)ECAT_SPI_XFER8(Address & 0xFFU);
}

/*******************************************************************************
  Function:
	UINT32 SPIReadBurstMode ()
  Summary:
    This function read 4 bytes continuosly.        
  
*****************************************************************************/
UINT32 SPIReadBurstMode ()
{
  UINT32 result;
  uint16_t b0 = ECAT_SPI_XFER8(0xFFU);
  uint16_t b1 = ECAT_SPI_XFER8(0xFFU);
  uint16_t b2 = ECAT_SPI_XFER8(0xFFU);
  uint16_t b3 = ECAT_SPI_XFER8(0xFFU);

  result = ((UINT32)(b0 & 0xFFU)) |
       ((UINT32)(b1 & 0xFFU) << 8) |
       ((UINT32)(b2 & 0xFFU) << 16) |
       ((UINT32)(b3 & 0xFFU) << 24);
  return result;
}

/*******************************************************************************
  Function:
	void SPIWriteBurstMode (UINT32 Val)
  Summary:
    This function writes 4 bytes continuosly.        
  
*****************************************************************************/
void SPIWriteBurstMode (UINT32 Val)
{
  (void)ECAT_SPI_XFER8((uint16_t)(Val & 0xFFU));
  (void)ECAT_SPI_XFER8((uint16_t)((Val >> 8) & 0xFFU));
  (void)ECAT_SPI_XFER8((uint16_t)((Val >> 16) & 0xFFU));
  (void)ECAT_SPI_XFER8((uint16_t)((Val >> 24) & 0xFFU));
}

#define ADDRESS_AUTO_INCREMENT 0x40
/*******************************************************************************
  Function:
	void SPIWriteBytes(UINT16 Address, UINT8 *Val, UINT8 nLenght)
  Summary:
    This function writes the LAN9252 CSR registers.        
  
*****************************************************************************/
void SPIWriteBytes(UINT16 Address, UINT8 *Val, UINT8 nLenght)
{
  UINT16 i;
  CSLOW();
  (void)ECAT_SPI_XFER8(CMD_SERIAL_WRITE);
  (void)ECAT_SPI_XFER8(((Address >> 8) & 0xFFU) | ADDRESS_AUTO_INCREMENT);
  (void)ECAT_SPI_XFER8(Address & 0xFFU);
  for(i = 0U; i < (UINT16)nLenght; i++)
  {
    (void)ECAT_SPI_XFER8(lo8(Val[i]));
  }
  CSHIGH();
}

/*******************************************************************************
  Function:
	void SPIWriteDWord (UINT16 Address, UINT32 Val)
  Summary:
    This function writes the LAN9252 CSR registers.        
  
*****************************************************************************/
void SPIWriteDWord (UINT16 Address, UINT32 Val)
{
  CSLOW();
  SPIWriteDWord_NoCS(Address, Val);
  CSHIGH();
}

/*******************************************************************************
  Function:
   void SPIReadRegUsingCSR(UINT8 *ReadBuffer, UINT16 Address, UINT8 Count)
  Summary:
    This function reads the EtherCAT core registers using LAN9252 CSR registers.        
  
*****************************************************************************/
void SPIReadRegUsingCSR(UINT8 *ReadBuffer, UINT16 Address, UINT8 Count)
{
  UINT32 cmd;
  UINT32 data;
  UINT16 i;
  UINT32 busyMask = ((UINT32)ESC_CSR_BUSY) << 24;
  UINT32 timeout = 100000UL;

  cmd = ((UINT32)(Address & 0xFFU)) |
      ((UINT32)((Address >> 8) & 0xFFU) << 8) |
      ((UINT32)((UINT16)Count & 0xFFU) << 16) |
      ((UINT32)ESC_READ_BYTE << 24);

  SPIWriteDWord(ESC_CSR_CMD_REG, cmd);
  while(((SPIReadDWord(ESC_CSR_CMD_REG) & busyMask) != 0U) && (timeout-- != 0U))
  {
    ;
  }

  data = SPIReadDWord(ESC_CSR_DATA_REG);
  for(i = 0U; i < (UINT16)Count; i++)
  {
    ReadBuffer[i] = (UINT8)((data >> (8U * i)) & 0xFFU);
  }
}

/*******************************************************************************
  Function:
   void SPIWriteRegUsingCSR( UINT8 *WriteBuffer, UINT16 Address, UINT8 Count)
  Summary:
    This function writes the EtherCAT core registers using LAN9252 CSR registers.        
  
*****************************************************************************/
void SPIWriteRegUsingCSR( UINT8 *WriteBuffer, UINT16 Address, UINT8 Count)
{
  UINT32 cmd;
  UINT32 data = 0UL;
  UINT16 i;
  UINT32 busyMask = ((UINT32)ESC_CSR_BUSY) << 24;
  UINT32 timeout = 100000UL;

  for(i = 0U; i < (UINT16)Count; i++)
  {
    data |= ((UINT32)(WriteBuffer[i] & 0xFFU)) << (8U * i);
  }
  SPIWriteDWord(ESC_CSR_DATA_REG, data);

  cmd = ((UINT32)(Address & 0xFFU)) |
      ((UINT32)((Address >> 8) & 0xFFU) << 8) |
      ((UINT32)((UINT16)Count & 0xFFU) << 16) |
      ((UINT32)ESC_WRITE_BYTE << 24);
  SPIWriteDWord(ESC_CSR_CMD_REG, cmd);
  while(((SPIReadDWord(ESC_CSR_CMD_REG) & busyMask) != 0U) && (timeout-- != 0U))
  {
    ;
  }
}

/*******************************************************************************
  Function:
   void SPIReadPDRamRegister(UINT8 *ReadBuffer, UINT16 Address, UINT16 Count)
  Summary:
    This function reads the PDRAM using LAN9252 FIFO.        
  
*****************************************************************************/
void SPIReadPDRamRegister(UINT8 *ReadBuffer, UINT16 Address, UINT16 Count)
{
  UINT8 cmdBuf[8];
  UINT32 timeout = 200000UL;
  UINT32 st;
  UINT16 i;

  // Program PRAM read address/length and set busy (write 8 bytes starting at PRAM_READ_ADDR_LEN_REG)
  cmdBuf[0] = ecat_u8(Address & 0xFFU);
  cmdBuf[1] = ecat_u8((Address >> 8) & 0xFFU);
  cmdBuf[2] = ecat_u8(Count & 0xFFU);
  cmdBuf[3] = ecat_u8((Count >> 8) & 0xFFU);
  cmdBuf[4] = 0;
  cmdBuf[5] = 0;
  cmdBuf[6] = 0;
  cmdBuf[7] = ecat_u8(0x80U); // busy bit (bit31)
  SPIWriteBytes(PRAM_READ_ADDR_LEN_REG, cmdBuf, 8U);

  // Wait until data is available
  do
  {
    st = SPIReadDWord(PRAM_READ_CMD_REG);
  } while(((st & (UINT32)IS_PRAM_SPACE_AVBL_MASK) == 0U) && (timeout-- != 0U));

  // Stream bytes from PRAM read FIFO
  CSLOW();
  (void)ECAT_SPI_XFER8(CMD_FAST_READ);
  SPISendAddr(PRAM_READ_FIFO_REG);
  (void)ECAT_SPI_XFER8(CMD_FAST_READ_DUMMY);
  for(i = 0U; i < Count; i++)
  {
    ReadBuffer[i] = ecat_u8(ECAT_SPI_XFER8(0xFFU));
  }
  CSHIGH();
}
        
/*******************************************************************************
  Function:
   void SPIWritePDRamRegister(UINT8 *WriteBuffer, UINT16 Address, UINT16 Count)
  Summary:
    This function writes the PDRAM using LAN9252 FIFO.        
  
*****************************************************************************/
void SPIWritePDRamRegister(UINT8 *WriteBuffer, UINT16 Address, UINT16 Count)
{
  UINT8 cmdBuf[8];
  UINT32 timeout = 200000UL;
  UINT32 st;
  UINT16 i;

  cmdBuf[0] = ecat_u8(Address & 0xFFU);
  cmdBuf[1] = ecat_u8((Address >> 8) & 0xFFU);
  cmdBuf[2] = ecat_u8(Count & 0xFFU);
  cmdBuf[3] = ecat_u8((Count >> 8) & 0xFFU);
  cmdBuf[4] = 0;
  cmdBuf[5] = 0;
  cmdBuf[6] = 0;
  cmdBuf[7] = ecat_u8(0x80U); // busy bit (bit31)
  SPIWriteBytes(PRAM_WRITE_ADDR_LEN_REG, cmdBuf, 8U);

  // Wait until space is available
  do
  {
    st = SPIReadDWord(PRAM_WRITE_CMD_REG);
  } while(((st & (UINT32)IS_PRAM_SPACE_AVBL_MASK) == 0U) && (timeout-- != 0U));

  // Stream bytes into PRAM write FIFO
  CSLOW();
  (void)ECAT_SPI_XFER8(CMD_SERIAL_WRITE);
  SPISendAddr(PRAM_WRITE_FIFO_REG);
  for(i = 0U; i < Count; i++)
  {
    (void)ECAT_SPI_XFER8(lo8(WriteBuffer[i]));
  }
  CSHIGH();
}

// -----------------------------------------------------------------------------
// Compatibility wrappers expected by SSC port glue (9252_HW.c)

void SPIReadDRegister(UINT8 *ReadBuffer, UINT16 Address, UINT16 Count)
{
  if(Address >= 0x1000U)
  {
    SPIReadPDRamRegister(ReadBuffer, Address, Count);
  }
  else
  {
    SPIReadRegUsingCSR(ReadBuffer, Address, (UINT8)Count);
  }
}

void SPIWriteRegister(UINT8 *WriteBuffer, UINT16 Address, UINT16 Count)
{
  if(Address >= 0x1000U)
  {
    SPIWritePDRamRegister(WriteBuffer, Address, Count);
  }
  else
  {
    SPIWriteRegUsingCSR(WriteBuffer, Address, (UINT8)Count);
  }
}

void SPIOpen()
{
  // SPI module is configured by Board_init()/mySPI0_init().
}

UINT8 SPIRead()
{
  return ecat_u8(ECAT_SPI_XFER8(0xFFU));
}

void SPIWrite(UINT8 data)
{
  (void)ECAT_SPI_XFER8(lo8(data));
}

/*******************************************************************************
  Function:
   void PDIReadReg(UINT8 *ReadBuffer, UINT16 Address, UINT16 Count)
  Summary:
    This function reads the ESC registers using LAN9252 CSR or FIFO.         
  
*****************************************************************************/
void PDIReadReg(UINT8 *ReadBuffer, UINT16 Address, UINT16 Count)
{
    if (Address >= 0x1000)
    {
         SPIReadPDRamRegister(ReadBuffer, Address,Count);
    }
    else
    {
         SPIReadRegUsingCSR(ReadBuffer, Address,Count);
    }
}
/*******************************************************************************
  Function:
   void PDIWriteReg( UINT8 *WriteBuffer, UINT16 Address, UINT16 Count)
  Summary:
    This function writes the ESC registers using LAN9252 CSR or FIFO.        
  
*****************************************************************************/
void PDIWriteReg( UINT8 *WriteBuffer, UINT16 Address, UINT16 Count)
{
   
   if (Address >= 0x1000)
   {
		SPIWritePDRamRegister(WriteBuffer, Address,Count);
   }
   else
   {
		SPIWriteRegUsingCSR(WriteBuffer, Address,Count);
   }
    
}

/*******************************************************************************
  Function:
	UINT32 PDIReadLAN9252DirectReg( UINT16 Address)
  Summary:
    This function reads the LAN9252 CSR registers(Not ESC registers).        
  
*****************************************************************************/
UINT32 PDIReadLAN9252DirectReg( UINT16 Address)
{   
    UINT32 data;
    data = SPIReadDWord (Address);
    return data;
}

/*******************************************************************************
  Function:
	void PDIWriteLAN9252DirectReg( UINT32 Val, UINT16 Address)
  Summary:
    This function writes the LAN9252 CSR registers(Not ESC registers).        
  
*****************************************************************************/
void PDIWriteLAN9252DirectReg( UINT32 Val, UINT16 Address)
{
    SPIWriteDWord (Address, Val);
}


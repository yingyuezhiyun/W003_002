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

#ifndef ECAT_SPI_FIFO_TIMEOUT_CYCLES
#define ECAT_SPI_FIFO_TIMEOUT_CYCLES 200000UL
#endif

/* SPI stall diagnostics (watch these in CCS expressions). */
VARVOLATILE UINT32 gEcatSpiTxTimeoutCount = 0;
VARVOLATILE UINT32 gEcatSpiRxTimeoutCount = 0;
VARVOLATILE UINT32 gEcatSpiLastTimeoutStage = 0;

static bool ecat_spi_write_fifo_with_timeout(uint16_t txWord)
{
  uint32_t timeout = ECAT_SPI_FIFO_TIMEOUT_CYCLES;

  while((SPI_getTxFIFOStatus(ECAT_SPI_BASE) == SPI_FIFO_TXFULL) && (timeout-- != 0UL))
  {
    ;
  }

  if(timeout == 0UL)
  {
    gEcatSpiTxTimeoutCount++;
    gEcatSpiLastTimeoutStage = 1U;
    return false;
  }

  HWREGH(ECAT_SPI_BASE + SPI_O_TXBUF) = txWord;
  return true;
}

static bool ecat_spi_read_fifo_with_timeout(uint16_t *rxWord)
{
  uint32_t timeout = ECAT_SPI_FIFO_TIMEOUT_CYCLES;

  while((SPI_getRxFIFOStatus(ECAT_SPI_BASE) == SPI_FIFO_RXEMPTY) && (timeout-- != 0UL))
  {
    ;
  }

  if(timeout == 0UL)
  {
    gEcatSpiRxTimeoutCount++;
    gEcatSpiLastTimeoutStage = 2U;
    *rxWord = 0xFFFFU;
    return false;
  }

  *rxWord = HWREGH(ECAT_SPI_BASE + SPI_O_RXBUF);
  return true;
}

static inline uint16_t ecat_spi_xfer8_fifo(uint16_t tx)
{
  uint16_t txWord = (tx & 0x00FFU) << 8;
  uint16_t rxWord = 0xFFFFU;

  // Keep FIFO running across bytes; avoid SPI_pollingFIFOTransaction() which
  // resets FIFO each call and inserts large inter-byte gaps.
  if(!ecat_spi_write_fifo_with_timeout(txWord))
  {
    return 0x00FFU;
  }

  if(!ecat_spi_read_fifo_with_timeout(&rxWord))
  {
    return 0x00FFU;
  }

  return (rxWord & 0x00FFU);
}

static bool ecat_spi_transfer_fifo(const UINT8 *tx, UINT8 *rx, UINT16 len)
{
  UINT16 pos = 0U;

  while(pos < len)
  {
    UINT16 i;
    UINT16 chunk = len - pos;
    if(chunk > 16U)
    {
      chunk = 16U;
    }

    for(i = 0U; i < chunk; i++)
    {
      uint16_t txWord = ((uint16_t)(tx[pos + i] & 0x00FFU)) << 8;
      if(!ecat_spi_write_fifo_with_timeout(txWord))
      {
        return false;
      }
    }

    for(i = 0U; i < chunk; i++)
    {
      uint16_t rxWord;
      if(!ecat_spi_read_fifo_with_timeout(&rxWord))
      {
        return false;
      }

      if(rx != (UINT8 *)0)
      {
        rx[pos + i] = (UINT8)(rxWord & 0x00FFU);
      }
    }

    pos += chunk;
  }

  return true;
}

static bool ecat_spi_read_stream_fifo(UINT8 *rx, UINT16 len)
{
  UINT16 pos = 0U;

  while(pos < len)
  {
    UINT16 i;
    UINT16 chunk = len - pos;
    if(chunk > 16U)
    {
      chunk = 16U;
    }

    for(i = 0U; i < chunk; i++)
    {
      if(!ecat_spi_write_fifo_with_timeout(0xFF00U))
      {
        return false;
      }
    }

    for(i = 0U; i < chunk; i++)
    {
      uint16_t rxWord;
      if(!ecat_spi_read_fifo_with_timeout(&rxWord))
      {
        return false;
      }

      rx[pos + i] = (UINT8)(rxWord & 0x00FFU);
    }

    pos += chunk;
  }

  return true;
}

#define ECAT_SPI_XFER8(_tx) (ecat_spi_xfer8_fifo((uint16_t)(_tx)))

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
  UINT8 tx[8];
  UINT8 rx[8];
  UINT32 result;

  tx[0] = (UINT8)CMD_FAST_READ;
  tx[1] = (UINT8)((Address >> 8) & 0xFFU);
  tx[2] = (UINT8)(Address & 0xFFU);
  tx[3] = (UINT8)CMD_FAST_READ_DUMMY;
  tx[4] = (UINT8)0xFFU;
  tx[5] = (UINT8)0xFFU;
  tx[6] = (UINT8)0xFFU;
  tx[7] = (UINT8)0xFFU;

  if(!ecat_spi_transfer_fifo(tx, rx, 8U))
  {
    return 0xFFFFFFFFUL;
  }

  result = ((UINT32)(rx[4] & 0xFFU)) |
       ((UINT32)(rx[5] & 0xFFU) << 8) |
       ((UINT32)(rx[6] & 0xFFU) << 16) |
       ((UINT32)(rx[7] & 0xFFU) << 24);
  return result;
}

static void SPIWriteDWord_NoCS(UINT16 Address, UINT32 Val)
{
  UINT8 tx[7];

  tx[0] = (UINT8)CMD_SERIAL_WRITE;
  tx[1] = (UINT8)((Address >> 8) & 0xFFU);
  tx[2] = (UINT8)(Address & 0xFFU);
  tx[3] = (UINT8)(Val & 0xFFU);
  tx[4] = (UINT8)((Val >> 8) & 0xFFU);
  tx[5] = (UINT8)((Val >> 16) & 0xFFU);
  tx[6] = (UINT8)((Val >> 24) & 0xFFU);

  (void)ecat_spi_transfer_fifo(tx, (UINT8 *)0, 7U);
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
  UINT8 tx[2];

  tx[0] = (UINT8)((Address >> 8) & 0xFFU);
  tx[1] = (UINT8)(Address & 0xFFU);
  (void)ecat_spi_transfer_fifo(tx, (UINT8 *)0, 2U);
}

/*******************************************************************************
  Function:
	UINT32 SPIReadBurstMode ()
  Summary:
    This function read 4 bytes continuosly.        
  
*****************************************************************************/
UINT32 SPIReadBurstMode ()
{
  UINT8 tx[4] = {(UINT8)0xFFU, (UINT8)0xFFU, (UINT8)0xFFU, (UINT8)0xFFU};
  UINT8 rx[4];
  UINT32 result;

  if(!ecat_spi_transfer_fifo(tx, rx, 4U))
  {
    return 0xFFFFFFFFUL;
  }

  result = ((UINT32)(rx[0] & 0xFFU)) |
       ((UINT32)(rx[1] & 0xFFU) << 8) |
       ((UINT32)(rx[2] & 0xFFU) << 16) |
       ((UINT32)(rx[3] & 0xFFU) << 24);
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
  UINT8 tx[4];

  tx[0] = (UINT8)(Val & 0xFFU);
  tx[1] = (UINT8)((Val >> 8) & 0xFFU);
  tx[2] = (UINT8)((Val >> 16) & 0xFFU);
  tx[3] = (UINT8)((Val >> 24) & 0xFFU);

  (void)ecat_spi_transfer_fifo(tx, (UINT8 *)0, 4U);
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
  UINT8 hdr[3];

  hdr[0] = (UINT8)CMD_SERIAL_WRITE;
  hdr[1] = (UINT8)(((Address >> 8) & 0xFFU) | ADDRESS_AUTO_INCREMENT);
  hdr[2] = (UINT8)(Address & 0xFFU);

  CSLOW();
  if(ecat_spi_transfer_fifo(hdr, (UINT8 *)0, 3U))
  {
    (void)ecat_spi_transfer_fifo(Val, (UINT8 *)0, (UINT16)nLenght);
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
  {
    UINT8 hdr[4];
    hdr[0] = (UINT8)CMD_FAST_READ;
    hdr[1] = (UINT8)((PRAM_READ_FIFO_REG >> 8) & 0xFFU);
    hdr[2] = (UINT8)(PRAM_READ_FIFO_REG & 0xFFU);
    hdr[3] = (UINT8)CMD_FAST_READ_DUMMY;
    if(!ecat_spi_transfer_fifo(hdr, (UINT8 *)0, 4U))
    {
      CSHIGH();
      return;
    }
  }
  (void)ecat_spi_read_stream_fifo(ReadBuffer, Count);
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
  {
    UINT8 hdr[3];
    hdr[0] = (UINT8)CMD_SERIAL_WRITE;
    hdr[1] = (UINT8)((PRAM_WRITE_FIFO_REG >> 8) & 0xFFU);
    hdr[2] = (UINT8)(PRAM_WRITE_FIFO_REG & 0xFFU);
    if(!ecat_spi_transfer_fifo(hdr, (UINT8 *)0, 3U))
    {
      CSHIGH();
      return;
    }
  }
  (void)ecat_spi_transfer_fifo(WriteBuffer, (UINT8 *)0, Count);
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


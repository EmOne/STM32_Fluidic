/*
    (c) 2016 Microchip Technology Inc. and its subsidiaries. You may use this
    software and any derivatives exclusively with Microchip products.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
    WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
    TERMS.
*/

#include "CANSPI.h"
#include "MCP2515.h"

/** Local Function Prototypes */  
static uint32_t convertReg2ExtendedCANid(uint8_t tempRXBn_EIDH, uint8_t tempRXBn_EIDL, uint8_t tempRXBn_SIDH, uint8_t tempRXBn_SIDL);
static uint32_t convertReg2StandardCANid(uint8_t tempRXBn_SIDH, uint8_t tempRXBn_SIDL) ;
static void convertCANid2Reg(uint32_t tempPassedInID, uint8_t canIdType, id_reg_t *passedIdReg);

/** Local Variables */ 
ctrl_status_t ctrlStatus;
ctrl_error_status_t errorStatus;
id_reg_t idReg;

/** CAN SPI APIs */ 

/* Entering Sleep Mode */
void CANSPI_Sleep(void)
{
  /* Clear CAN bus wakeup interrupt */
  MCP2515_BitModify(MCP2515_CANINTF, 0x40, 0x00);        
  
  /* Enable CAN bus activity wakeup */
  MCP2515_BitModify(MCP2515_CANINTE, 0x40, 0x40);        
  
  MCP2515_SetSleepMode();
}

/* Initialize CAN */
bool CANSPI_Initialize(const uint8_t canSpeed, const uint8_t clock)
{
	uint8_t res;
	RXF0 RXF0reg;
	RXF1 RXF1reg;
	RXF2 RXF2reg;
	RXF3 RXF3reg;
	RXF4 RXF4reg;
	RXF5 RXF5reg;
	RXM0 RXM0reg;
	RXM1 RXM1reg;

	/* Intialize Rx Mask values */
	RXM0reg.RXM0SIDH = 0x00;
	RXM0reg.RXM0SIDL = 0x00;
	RXM0reg.RXM0EID8 = 0x00;
	RXM0reg.RXM0EID0 = 0x00;

	RXM1reg.RXM1SIDH = 0x00;
	RXM1reg.RXM1SIDL = 0x00;
	RXM1reg.RXM1EID8 = 0x00;
	RXM1reg.RXM1EID0 = 0x00;

	/* Intialize Rx Filter values */
	RXF0reg.RXF0SIDH = 0x00;
	RXF0reg.RXF0SIDL = 0x00;      //Starndard Filter
	RXF0reg.RXF0EID8 = 0x00;
	RXF0reg.RXF0EID0 = 0x00;

	RXF1reg.RXF1SIDH = 0x00;
	RXF1reg.RXF1SIDL = 0x08;      //Exntended Filter
	RXF1reg.RXF1EID8 = 0x00;
	RXF1reg.RXF1EID0 = 0x00;

	RXF2reg.RXF2SIDH = 0x00;
	RXF2reg.RXF2SIDL = 0x00;
	RXF2reg.RXF2EID8 = 0x00;
	RXF2reg.RXF2EID0 = 0x00;

	RXF3reg.RXF3SIDH = 0x00;
	RXF3reg.RXF3SIDL = 0x00;
	RXF3reg.RXF3EID8 = 0x00;
	RXF3reg.RXF3EID0 = 0x00;

	RXF4reg.RXF4SIDH = 0x00;
	RXF4reg.RXF4SIDL = 0x00;
	RXF4reg.RXF4EID8 = 0x00;
	RXF4reg.RXF4EID0 = 0x00;

	RXF5reg.RXF5SIDH = 0x00;
	RXF5reg.RXF5SIDL = 0x08;
	RXF5reg.RXF5EID8 = 0x00;
	RXF5reg.RXF5EID0 = 0x00;

  /* Intialize MCP2515, check SPI */
  if(!MCP2515_Initialize())
  {
    return false;
  }

	MCP2515_Reset();


  /* Change mode as configuration mode */
	res = mcp2515_setCANCTRL_Mode(MODE_CONFIG);
	if (res > 0)
  {
		HAL_Delay(10);
    return false;
  }
  

//  /* Configure filter & mask */
	MCP2515_WriteByteSequence(MCP2515_RXM0SIDH, MCP2515_RXM0EID0,
			&(RXM0reg.RXM0SIDH));
	MCP2515_WriteByteSequence(MCP2515_RXM1SIDH, MCP2515_RXM1EID0,
			&(RXM1reg.RXM1SIDH));
	MCP2515_WriteByteSequence(MCP2515_RXF0SIDH, MCP2515_RXF0EID0,
			&(RXF0reg.RXF0SIDH));
	MCP2515_WriteByteSequence(MCP2515_RXF1SIDH, MCP2515_RXF1EID0,
			&(RXF1reg.RXF1SIDH));
	MCP2515_WriteByteSequence(MCP2515_RXF2SIDH, MCP2515_RXF2EID0,
			&(RXF2reg.RXF2SIDH));
	MCP2515_WriteByteSequence(MCP2515_RXF3SIDH, MCP2515_RXF3EID0,
			&(RXF3reg.RXF3SIDH));
	MCP2515_WriteByteSequence(MCP2515_RXF4SIDH, MCP2515_RXF4EID0,
			&(RXF4reg.RXF4SIDH));
	MCP2515_WriteByteSequence(MCP2515_RXF5SIDH, MCP2515_RXF5EID0,
			&(RXF5reg.RXF5SIDH));
//
//  /* Accept All (Standard + Extended) */
	MCP2515_WriteByte(MCP2515_RXB0CTRL, 0x04);    //Enable BUKT, Accept Filter 0
	MCP2515_WriteByte(MCP2515_RXB1CTRL, 0x01);    //Accept Filter 1
//
//  /*
//  * tq = 2 * (brp(0) + 1) / 16000000 = 0.125us
//  * tbit = (SYNC_SEG(1 fixed) + PROP_SEG + PS1 + PS2)
//  * tbit = 1tq + 5tq + 6tq + 4tq = 16tq
//  * 16tq = 2us = 500kbps
//	 *
//  */
//	//INFO: See bit rate setting in mcp2515_can_dfs.h
////  MCP2515_WriteByte(MCP2515_CNF1, 0x00);
//	MCP2515_WriteByte(MCP2515_CNF1, MCP_16MHz_125kBPS_CFG1);
//
//  /* 1 1 100(5tq) 101(6tq) */
//  MCP2515_WriteByte(MCP2515_CNF2, MCP_16MHz_125kBPS_CFG2);
//
//  /* 1 0 000 011(4tq) */
//  MCP2515_WriteByte(MCP2515_CNF3, MCP_16MHz_125kBPS_CFG3);

	 // set boadrate
	res = mcp2515_configRate(canSpeed, clock);
	if (res > 0)
	{

		HAL_Delay(10);

		return res;
	}

  /* Normal 모드로 설정 */
	if (res == MCP2515_OK)
	{

		// init canbuffers
		mcp2515_initCANBuffers();

		// interrupt mode
		mcp2515_enableRxInterrupt(true);
		mcp2515_enableErrInterrupt(true);
		mcp2515_enableMsgErrInterrupt(true);

#if (DEBUG_RXANY==1)
	    // enable both receive-buffers to receive any message and enable rollover
		MCP2515_BitModify(MCP2515_RXB0CTRL,
	                           MCP_RXB_RX_MASK | MCP_RXB_BUKT_MASK,
	                           MCP_RXB_RX_ANY | MCP_RXB_BUKT_MASK);
	    MCP2515_BitModify(MCP2515_RXB1CTRL, MCP_RXB_RX_MASK,
	                           MCP_RXB_RX_ANY);
#else
		// enable both receive-buffers to receive messages with std. and ext. identifiers and enable rollover
		MCP2515_BitModify(MCP2515_RXB0CTRL,
		MCP_RXB_RX_MASK | MCP_RXB_BUKT_MASK,
		MCP_RXB_RX_STDEXT | MCP_RXB_BUKT_MASK);
		MCP2515_BitModify(MCP2515_RXB1CTRL, MCP_RXB_RX_MASK,
		MCP_RXB_RX_STDEXT);
#endif
		// enter normal mode
		res = mcp2515_setMode(MODE_NORMAL);
		if (res)
		{
#if DEBUG_EN
//			SERIAL_PORT_MONITOR.println(F("Enter Normal Mode Fail!!"));
#else
	        delay(10);
	        #endif
			return res;
		}

#if DEBUG_EN
//		SERIAL_PORT_MONITOR.println(F("Enter Normal Mode Success!!"));
#else
	    delay(10);
	    #endif

	}
	return res;
//  if(!MCP2515_SetNormalMode())
//    return false;
//
//  return true;
}

/* Transmit CAN message */
uint8_t CANSPI_Transmit(uCAN_MSG *tempCanMsg, bool waitSent)
{
  uint8_t returnValue = 0;
#if 0
	uint8_t res, res1, txbuf_n;
	uint16_t uiTimeOut = 0;
	do
	{
		if (uiTimeOut > 0)
		{
			HAL_Delay(10);
		}
		res = mcp2515_getNextFreeTXBuf(&txbuf_n);                // info = addr.
		uiTimeOut++;
	} while (res == MCP_ALLTXBUSY && (uiTimeOut < TIMEOUTVALUE));

	if (uiTimeOut == TIMEOUTVALUE)
	{
		return CAN_GETTXBFTIMEOUT;                       // get tx buff time out
	}
	uint8_t buf[8];
	memcpy(buf, &tempCanMsg->frame.data0, 8);
	mcp2515_write_canMsg(txbuf_n, tempCanMsg->frame.id, 0, 0,
			tempCanMsg->frame.dlc, buf);

	if (waitSent)
	{
		uiTimeOut = 0;
		do
		{
			if (uiTimeOut > 0)
			{
				HAL_Delay(10);
			}
			uiTimeOut++;
			res1 = MCP2515_ReadByte(txbuf_n - 1); // read send buff ctrl reg
			res1 = res1 & 0x08;
		} while (res1 && (uiTimeOut < TIMEOUTVALUE));

		if (uiTimeOut == TIMEOUTVALUE)
		{                                     // send msg timeout
			return CAN_SENDMSGTIMEOUT;
		}
	}

	return CAN_OK;
#else

  idReg.tempSIDH = 0;
  idReg.tempSIDL = 0;
  idReg.tempEID8 = 0;
  idReg.tempEID0 = 0;
  
  ctrlStatus.ctrl_status = MCP2515_ReadStatus();
  
  /* Finding empty buffer */
  if (ctrlStatus.TXB0REQ != 1)
  {
    /* convert CAN ID for register */
    convertCANid2Reg(tempCanMsg->frame.id, tempCanMsg->frame.idType, &idReg);
    
    /* Load data to Tx Buffer */
    MCP2515_LoadTxSequence(MCP2515_LOAD_TXB0SIDH, &(idReg.tempSIDH), tempCanMsg->frame.dlc, &(tempCanMsg->frame.data0));
    
    /* Request to transmit */
    MCP2515_RequestToSend(MCP2515_RTS_TX0);
    
    returnValue = 1;
  }
  else if (ctrlStatus.TXB1REQ != 1)
  {
    convertCANid2Reg(tempCanMsg->frame.id, tempCanMsg->frame.idType, &idReg);
    
    MCP2515_LoadTxSequence(MCP2515_LOAD_TXB1SIDH, &(idReg.tempSIDH), tempCanMsg->frame.dlc, &(tempCanMsg->frame.data0));
    MCP2515_RequestToSend(MCP2515_RTS_TX1);
    
    returnValue = 1;
  }
  else if (ctrlStatus.TXB2REQ != 1)
  {
    convertCANid2Reg(tempCanMsg->frame.id, tempCanMsg->frame.idType, &idReg);
    
    MCP2515_LoadTxSequence(MCP2515_LOAD_TXB2SIDH, &(idReg.tempSIDH), tempCanMsg->frame.dlc, &(tempCanMsg->frame.data0));
    MCP2515_RequestToSend(MCP2515_RTS_TX2);
    
    returnValue = 1;
  }
#endif
  return (returnValue);
}

/* Receive CAN message */
uint8_t CANSPI_Receive(uCAN_MSG *tempCanMsg) 
{
	uint8_t returnValue = 0, RXnIF, ext, rtrBit;
	rx_reg_t rxReg;
  ctrl_rx_status_t rxStatus;
	uint8_t rc = CAN_NOMSG;

  rxStatus.ctrl_rx_status = MCP2515_GetRxStatus();
#if 1
	if (rxStatus.ctrl_rx_status & MCP_RX0IF)
	{                                        // Msg in Buffer 0
		mcp2515_read_canMsg(MCP_READ_RX0, &tempCanMsg->frame.id, &ext, &rtrBit,
				&tempCanMsg->frame.dlc, &tempCanMsg->frame.data0);
		rc = CAN_OK;
	}
	else if (rxStatus.ctrl_rx_status & MCP_RX1IF)
	{                                 // Msg in Buffer 1
		mcp2515_read_canMsg(MCP_READ_RX1, &tempCanMsg->frame.id, &ext, &rtrBit,
				&tempCanMsg->frame.dlc, &tempCanMsg->frame.data0);
		rc = CAN_OK;
	}
	else if (rxStatus.ctrl_rx_status & MCP_RX1IF)
	{                                 // Msg in Buffer 1
		mcp2515_read_canMsg(MCP_READ_RX1, &tempCanMsg->frame.id, &ext, &rtrBit,
				&tempCanMsg->frame.dlc, &tempCanMsg->frame.data0);
		rc = CAN_OK;
	}

//	if (rc == CAN_OK)
//	{
//		rtr = *rtrBit;
//		// dta_len=*len; // not used on any interface function
//		ext_flg = *ext;
//		can_id = *id;
//	}
//	else
//	{
//		*len = 0;
//	}

#else
  /* Check receive buffer */
  if (rxStatus.rxBuffer != 0)
  {
    /* finding buffer which has a message */
    if ((rxStatus.rxBuffer == MSG_IN_RXB0)|(rxStatus.rxBuffer == MSG_IN_BOTH_BUFFERS))
    {
      MCP2515_ReadRxSequence(MCP2515_READ_RXB0SIDH, rxReg.rx_reg_array, sizeof(rxReg.rx_reg_array));
    }
    else if (rxStatus.rxBuffer == MSG_IN_RXB1)
    {
      MCP2515_ReadRxSequence(MCP2515_READ_RXB1SIDH, rxReg.rx_reg_array, sizeof(rxReg.rx_reg_array));
    }
    
    /* if the message is extended CAN type */
    if (rxStatus.msgType == dEXTENDED_CAN_MSG_ID_2_0B)
    {
      tempCanMsg->frame.idType = (uint8_t) dEXTENDED_CAN_MSG_ID_2_0B;
      tempCanMsg->frame.id = convertReg2ExtendedCANid(rxReg.RXBnEID8, rxReg.RXBnEID0, rxReg.RXBnSIDH, rxReg.RXBnSIDL);
    } 
    else 
    {
      /* Standard type */
      tempCanMsg->frame.idType = (uint8_t) dSTANDARD_CAN_MSG_ID_2_0B;
      tempCanMsg->frame.id = convertReg2StandardCANid(rxReg.RXBnSIDH, rxReg.RXBnSIDL);
    }
    
    tempCanMsg->frame.dlc   = rxReg.RXBnDLC;
    tempCanMsg->frame.data0 = rxReg.RXBnD0;
    tempCanMsg->frame.data1 = rxReg.RXBnD1;
    tempCanMsg->frame.data2 = rxReg.RXBnD2;
    tempCanMsg->frame.data3 = rxReg.RXBnD3;
    tempCanMsg->frame.data4 = rxReg.RXBnD4;
    tempCanMsg->frame.data5 = rxReg.RXBnD5;
    tempCanMsg->frame.data6 = rxReg.RXBnD6;
    tempCanMsg->frame.data7 = rxReg.RXBnD7;

    returnValue = 1;
  }
#endif
  //Clear RX interrupt flag
	RXnIF = MCP2515_ReadByte(MCP2515_CANINTF);
	RXnIF &= ~(MCP_RX0IF | MCP_RX1IF);
	MCP2515_WriteByte(MCP2515_CANINTF, RXnIF);

	return (rc);
}

/* check message buffer and return count */
uint8_t CANSPI_messagesInBuffer(void)
{
  uint8_t messageCount = 0;
  
  ctrlStatus.ctrl_status = MCP2515_ReadStatus();
  
  if(ctrlStatus.RX0IF != 0)
  {
    messageCount++;
  }
  
  if(ctrlStatus.RX1IF != 0)
  {
    messageCount++;
  }
  
  return (messageCount);
}

/* check BUS off */
uint8_t CANSPI_isBussOff(void)
{
  uint8_t returnValue = 0;
  
  errorStatus.error_flag_reg = MCP2515_ReadByte(MCP2515_EFLG);
  
  if(errorStatus.TXBO == 1)
  {
    returnValue = 1;
  }
  
  return (returnValue);
}

/* check Rx Passive Error */
uint8_t CANSPI_isRxErrorPassive(void)
{
  uint8_t returnValue = 0;
  
  errorStatus.error_flag_reg = MCP2515_ReadByte(MCP2515_EFLG);
  
  if(errorStatus.RXEP == 1)
  {
    returnValue = 1;
  }
  
  return (returnValue);
}

/* check Tx Passive Error */
uint8_t CANSPI_isTxErrorPassive(void)
{
  uint8_t returnValue = 0;
  
  errorStatus.error_flag_reg = MCP2515_ReadByte(MCP2515_EFLG);
  
  if(errorStatus.TXEP == 1)
  {
    returnValue = 1;
  }
  
  return (returnValue);
}

/* convert register value to extended CAN ID */
static uint32_t convertReg2ExtendedCANid(uint8_t tempRXBn_EIDH, uint8_t tempRXBn_EIDL, uint8_t tempRXBn_SIDH, uint8_t tempRXBn_SIDL) 
{
  uint32_t returnValue = 0;
  uint32_t ConvertedID = 0;
  uint8_t CAN_standardLo_ID_lo2bits;
  uint8_t CAN_standardLo_ID_hi3bits;
  
  CAN_standardLo_ID_lo2bits = (tempRXBn_SIDL & 0x03);
  CAN_standardLo_ID_hi3bits = (tempRXBn_SIDL >> 5);
  ConvertedID = (tempRXBn_SIDH << 3);
  ConvertedID = ConvertedID + CAN_standardLo_ID_hi3bits;
  ConvertedID = (ConvertedID << 2);
  ConvertedID = ConvertedID + CAN_standardLo_ID_lo2bits;
  ConvertedID = (ConvertedID << 8);
  ConvertedID = ConvertedID + tempRXBn_EIDH;
  ConvertedID = (ConvertedID << 8);
  ConvertedID = ConvertedID + tempRXBn_EIDL;
  returnValue = ConvertedID;    
  return (returnValue);
}

/* convert register value to standard CAN ID */
static uint32_t convertReg2StandardCANid(uint8_t tempRXBn_SIDH, uint8_t tempRXBn_SIDL) 
{
  uint32_t returnValue = 0;
  uint32_t ConvertedID;
  
  ConvertedID = (tempRXBn_SIDH << 3);
  ConvertedID = ConvertedID + (tempRXBn_SIDL >> 5);
  returnValue = ConvertedID;
  
  return (returnValue);
}

/* convert CAN ID to register value */
static void convertCANid2Reg(uint32_t tempPassedInID, uint8_t canIdType, id_reg_t *passedIdReg) 
{
  uint8_t wipSIDL = 0;
  
  if (canIdType == dEXTENDED_CAN_MSG_ID_2_0B) 
  {
    //EID0
    passedIdReg->tempEID0 = 0xFF & tempPassedInID;
    tempPassedInID = tempPassedInID >> 8;
    
    //EID8
    passedIdReg->tempEID8 = 0xFF & tempPassedInID;
    tempPassedInID = tempPassedInID >> 8;
    
    //SIDL
    wipSIDL = 0x03 & tempPassedInID;
    tempPassedInID = tempPassedInID << 3;
    wipSIDL = (0xE0 & tempPassedInID) + wipSIDL;
    wipSIDL = wipSIDL + 0x08;
    passedIdReg->tempSIDL = 0xEB & wipSIDL;
    
    //SIDH
    tempPassedInID = tempPassedInID >> 8;
    passedIdReg->tempSIDH = 0xFF & tempPassedInID;
  } 
  else
  {
    passedIdReg->tempEID8 = 0;
    passedIdReg->tempEID0 = 0;
    tempPassedInID = tempPassedInID << 5;
    passedIdReg->tempSIDL = 0xFF & tempPassedInID;
    tempPassedInID = tempPassedInID >> 8;
    passedIdReg->tempSIDH = 0xFF & tempPassedInID;
  }
}

uint8_t CANSPI_CheckReceive(void)
{
	return mcp2515_checkReceive();
}

uint8_t CANSPI_CheckErr(void)
{
	uint8_t ret = 0;
	uint8_t flags = MCP2515_ReadByte(MCP2515_CANINTF);
	if (flags & (MCP_MERRF)) // Message error
	{

		flags &= ~(MCP_MERRF);
		MCP2515_WriteByte(MCP2515_CANINTF, flags);
		ret |= 0x01;
	}

	if (flags & (MCP_ERRIF)) // Error
	{
		if (CANSPI_isBussOff())
		{
			ret |= 0x10;
		}
		if (CANSPI_isTxErrorPassive())
		{
			ret |= 0x20;
		}
		if (CANSPI_isRxErrorPassive())
		{
			ret |= 0x40;
		}
		flags &= ~(1 << 5);
		MCP2515_WriteByte(MCP2515_CANINTF, flags);
		ret |= 0x80;
	}
	return ret;
}

#include "can_comm.h"
#include <stdio.h>

// Init SPI for ATmega162
void SPI_init(void) {
    // SPI in master mode, mode 0 (CPOL=0, CPHA=0), frequency SCK = fosc/16
    SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0);
}

// Send SPI data and reception
uint8_t SPI_transfer(uint8_t data) {
    SPDR = data;  // Send data
    while (!(SPSR & (1 << SPIF)));  // Wait the end of the transmission
    return SPDR;  
}

// Selection of the MCP2515 (CS LOW)
void MCP2515_select(void) {
    PORTB &= ~(1 << SS_PIN);  // SS to LOW (activate MCP2515)
}

// Deselection of the MCP2515 (CS HIGH)
void MCP2515_deselect(void) {
    PORTB |= (1 << SS_PIN);  // SS to HIGH (deactivation of MCP2515)
}

// Reset of the MCP2515
void MCP2515_reset(void) {
    MCP2515_select();
    SPI_transfer(MCP_RESET);
    MCP2515_deselect();
    _delay_ms(10);  // Wait a moment for the reset
}

// Write of a register of the MCP2515
void MCP2515_writeRegister(uint8_t address, uint8_t data) {
    MCP2515_select();
    SPI_transfer(MCP_WRITE);
    SPI_transfer(address);
    SPI_transfer(data);
    MCP2515_deselect();
}   

// Lecture of a register of the MCP2515
uint8_t MCP2515_readRegister(uint8_t address) {
    MCP2515_select();
    SPI_transfer(MCP_READ);
    SPI_transfer(address);
	uint8_t result = SPI_transfer(0x00);
    MCP2515_deselect();
    return result;
}

void request_to_send(uint8_t bufferNbr)
{
	uint8_t data = 0;
	switch(bufferNbr)
	{
		case 0:	
			data = MCP2515_readRegister(0x30);
			data |= 0b00001000;
			MCP2515_writeRegister(0x30,data);
		break;
		case 1:
			data = MCP2515_readRegister(0x40);
			data |= 0b00001000;
			MCP2515_writeRegister(0x40,data);
		break;
		case 2:
			data = MCP2515_readRegister(0x50);
			data |= 0b00001000;
			MCP2515_writeRegister(0x50,data);
		break;
	}
}

uint8_t read_status(void)
{
	uint8_t result = MCP2515_readRegister(0x0E);
	return result;
}

void bitMode(uint8_t address, uint8_t maskByte)
{
	MCP2515_select();
	SPI_transfer(MCP_BITMOD);
	SPI_transfer(address);
	SPI_transfer(maskByte);
	MCP2515_deselect();
}

// Configuration of the MCP2515
//mode = 0 => loopback and one shot mode
//mode = 1 => normal mode
void MCP2515_init(uint8_t mode) {
    // Configuration of the registers CNF1, CNF2, CNF3 for the CAN speed (125 kbps with 16 MHz)
    MCP2515_writeRegister(CNF1, 0b11000011);  // CNF1: BRP = 3 => TQ =500ns
    MCP2515_writeRegister(CNF2, 0b10110001);  // CNF2: phase 1 = 5TQ & propagation = 6TQ
    MCP2515_writeRegister(CNF3, 0x05);  // CNF3: phase 2 = 2TQ
	
    // Configure the mode of the can controller
    uint8_t canctrl = MCP2515_readRegister(CANCTRL);
    canctrl &= ~(0xE0);  // Delete REQOP bits
	
	if(mode)
		canctrl |= (1 << 6) | (1<<3); // loopback and one shot mode
	else
		canctrl = 0;
		
    MCP2515_writeRegister(CANCTRL, canctrl);

    _delay_ms(10); // Wait a moment to stabilize
}

uint8_t can_init(uint8_t mode) {
	uint8_t value;
	
    // Init SPI
    SPI_init();
	
	MCP2515_reset();

	value = MCP2515_readRegister(MCP_CANSTAT);
	
	if((value & MODE_MASK) != MODE_CONFIG)
		return 1;

    // Initialisation of the MCP2515 in loopback
    MCP2515_init(mode);
	
	return 0;
}

//24 = 3*8 (because we have 3 buffer of 8 bytes)
uint8_t can_send_message(struct canData data,uint8_t bufferNbr)
{
	uint8_t startAddress = 0;
	
	if(data.length > 8 || data.ID > 1023) //Data error
		return 1;

	switch(bufferNbr)
	{
		case 0: 
			startAddress = 0x30;
			if((MCP2515_readRegister(0x30)&0b00001000)>>3)
				return 1;
		break;
		case 1:
			startAddress = 0x40;
			if((MCP2515_readRegister(0x40)&0b00001000)>>3)
				return 1;
		break;
		case 2:
			startAddress = 0x50;
			if((MCP2515_readRegister(0x50)&0b00001000)>>3)
				return 1;
		break;
		default:
			return 1;
		break;
	}
	
	MCP2515_writeRegister(startAddress+SIDH,(data.ID&0b11111111000)>>3);
	uint8_t dataSIDL = MCP2515_readRegister(startAddress+SIDL);
	dataSIDL &=  0b00011111;
	dataSIDL |= (data.ID<<5);
	MCP2515_writeRegister(startAddress+SIDL,dataSIDL);
	MCP2515_writeRegister(startAddress+DLC,data.length);
	
	for(uint8_t j=0; j<data.length;j++)
		MCP2515_writeRegister(startAddress+j+6,data.data[j]);
	for(uint8_t j=data.length; j<8;j++)
		MCP2515_writeRegister(startAddress+j+6,0);

	request_to_send(bufferNbr);
	
	return 0;
}

//If no reception on both buffer => return 0
//If reception on 1st buffer and no reception on 2nd buffer => return 2
//If no reception on 1st buffer and reception on 2nd buffer => return 4
//If reception on both buffer => return 6
uint8_t can_test_reception(void)
{
	uint8_t flag = MCP2515_readRegister(0x2c);	
	uint8_t result=0;
	for(uint8_t bufferNbr=0;bufferNbr<2;bufferNbr++)
	{
		if(!bufferNbr)//If buffer R0
			flag = flag&0b00000001;
		else
			flag = (flag&0b00000010)>>1;
		if(!flag)
			result += (bufferNbr+1)*2;
	}
	return result;
}

struct canData can_reception(uint8_t bufferNbr)
{
	uint8_t startAddress = 0x60 + (bufferNbr<<4);
	uint8_t flag = MCP2515_readRegister(0x2c);
	
	if(!bufferNbr)//If buffer R0
		flag = flag&0b00000001;
	else
		flag = (flag&0b00000010)>>1;
	
	struct canData result = {0xff};
	
	if(!flag)
		return result;
		
	result.ID = MCP2515_readRegister(startAddress + 1)<<3; //MSB
	result.ID += MCP2515_readRegister(startAddress + 2)>>5; //LSB
	result.length += MCP2515_readRegister(startAddress + 5);
	
	for(uint8_t j=0; j<result.length;j++)
		result.data[j] = MCP2515_readRegister(startAddress + 6 + j);
		
		
	//CANINTF 
    uint8_t buff = MCP2515_readRegister(0x2c);
	if(!bufferNbr)//If R0 (not R1)
		buff &= 0b11111110;
	else
		buff &= 0b11111101;
	MCP2515_writeRegister(0x2c,buff);
	
	return result;
}


void can_test_feedback(uint8_t param)
{
	printf("CANINTF = %d \n",MCP2515_readRegister(0x2c));
	printf("Can data send\n\n");
	struct canData dataCan = {0};
	dataCan.length = 8;
	dataCan.ID = 0x12;
	
	for(uint8_t i = 0; i<8; i++)
	{
		dataCan.data[i] = 10*i+param;
	}

	if(can_send_message(dataCan,0))
		printf("error sending ! \n");
	if(can_send_message(dataCan,0))
		printf("error sending ! \n");
	
	struct canData dataCanRecv = {0};	
	printf("\n------------------------------------\n");
	dataCanRecv = can_reception(0);
	printf("length = %d \n",dataCanRecv.length);
	printf("ID = %d \n",dataCanRecv.ID);
	if(dataCanRecv.length >= 1)
	printf("value = %d , %d \n",dataCanRecv.data[0],dataCanRecv.data[1]);
	
	dataCanRecv = can_reception(1);
	printf("length = %d \n",dataCanRecv.length);
	printf("ID = %d \n",dataCanRecv.ID);
	if(dataCanRecv.length >= 1)
	printf("value = %d , %d \n",dataCanRecv.data[0],dataCanRecv.data[1]);
	
}
/*
 * can_comm.h
 *
 * Created: 30.09.2024 09:29:30
 *  Author: matbo
 */ 
#include <avr/io.h>
#define F_CPU 4910000 //CPU frequency
#include <util/delay.h>	//To use delays

#ifndef CAN_COMM_H_
#define CAN_COMM_H_

#define CAN_FOSC 16000000

// SPI pining definition
#define SS_PIN     PB4  // Slave Select (CS)
#define MOSI_PIN   PB5  // Master Out Slave In
#define MISO_PIN   PB6  // Master In Slave Out
#define SCK_PIN    PB7  // Serial Clock

// Command of the MCP2515
#define MCP_RESET        0xC0  // Command Reset of the MCP2515
#define MCP_WRITE        0x02  // Command Write
#define MCP_READ         0x03  // Command Read
#define MCP_BITMOD       0x05  // Command Bit Modify
#define MCP_READ_STATUS  0xA0  // Command Read Status
#define MCP_RTS			 0x08

//Command for CAN
#define CTRL		0
#define SIDH		1
#define SIDL		2
#define DLC			5

// Address of the register of the MCP2515
#define MCP_CANSTAT		 0x0E
#define CANCTRL          0x0F  // Registre CAN Control
#define CNF1             0x2A  // Registre Configuration 1
#define CNF2             0x29  // Registre Configuration 2
#define CNF3             0x28  // Registre Configuration 3

//Mode
#define MODE_CONFIG 0x80
#define MODE_MASK 0b11100000
struct canData
{
	uint16_t ID;
	uint8_t length;
	uint8_t data[16];	
};

void SPI_init(void);
uint8_t SPI_transfer(uint8_t data);
void MCP2515_select(void);
void MCP2515_deselect(void);
void MCP2515_reset(void);
void MCP2515_writeRegister(uint8_t address, uint8_t data);
uint8_t MCP2515_readRegister(uint8_t address);
void MCP2515_init(uint8_t mode); //return 1 if an error occured during the init  (mode = 1 => loopback)
uint8_t can_init(uint8_t mode);
void request_to_send(uint8_t bufferNbr);
uint8_t read_status(void);
void bitMode(uint8_t address, uint8_t maskByte);

uint8_t can_send_message(struct canData data,uint8_t bufferNbr);
struct canData can_reception(uint8_t bufferNbr);
uint8_t can_test_reception(void);
void can_test_feedback(uint8_t param);

#endif /* CAN_COMM_H_ */
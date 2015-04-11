#ifndef _LIPOGAUGE_H
#define _LIPOGAUGE_H


#include <types.h>
#include "../../twi/i2c.h"


#define LIPO_SLEEP			7
#define LIPO_ALERT			5

// LIPO Fuel Gauge for 1S/2S
// uses I2C interface
// updates tend to be minimum of 125ms apart, but can be 500ms or longer.

/*
ADDRESS (HEX)	REGISTER	DESCRIPTION					R/W		DEFAULT (HEX)
-------------	--------	------------------------	------	-------------
02h - 03h		VCELL		12-bit A/D B+ voltage		R		-
04h - 05h		SOC			16-bit SOC					R		-
06h - 07h		MODE		Sends commands to IC		W		-
08h - 09h		VERSION		Returns IC version			R		-
0Ch - 0Dh		CONFIG		Adj. IC perf. per app.		R/W		971Ch
FEh - FFh		COMMAND		Sends commands to IC		W		-

Register descriptions:
VCELL:
	12 bit value, resolution is 1.25mV, update is every 500ms
	MSB:	02h, bits 11-4	(7-0)
	LSB:	03h, bits 3-0	(7-4)

SOC:
	16 bit value.  units of % can be determined for MSB only. LSB returns
	values of 1/256%.  SOC also includes residual capacity.
	MSB:	04h, bits 7-0	(7-0)
	LSB:	05h, bits -1--8 (7-0)

MODE:
	Allows the host processor to send special commands to the IC.
	MSB:	06h
	LSB:	07h
	
	Mode commands:
	Value		Command			Description
	4000h		Quick start		

VERSION:
	IC version
	MSB:	08h
	LSB:	09h

CONFIG:
	Used to compensate the SOC algorithm, controls the alert feature, forces IC
	into software-sleep mode.  CONFIG is an 8-bit value to optimize IC performance
	for different batteries or temperatures.  Default is 97h
	MSB:	0Ch, compensation register (default 97h)
	LSB:	0Dh, various flags - default is 1Ch, 0b0001 1100, or 28 (32-4 = 28)
		bits:
			7		sleep	(1 force sleep, 0 exit sleep)
			6		x		(don't care)
			5		alert	(set by IC when SOC falls below alert threshold)
			4-0		athd	(alert threshold, 2's compliment)
				ATHD: 00000 = 32%, 00001 = 31%, 11111 = 1%
				Range is 1% to 32%
				Power on default is 4%
		Note: the alert bit must be cleared by software, and the alert line
		will remain low until cleared.  Write to 0 in software to clear.

Note:
	To read a register, send DEVICE addr + WRITE bit, then send REGISTER address.
	Next, send DEVICE addr + READ bit, then read the next 2 bytes.

*/


// I2C/TWI device address is 0x6C for write, 0x6D for read
// 0b0110 1100
#define LIPO_GAUGE_DEVICE_ADDR			0x6C

#define LIPO_GAUGE_REG_VCELL			0x02
#define LIPO_GAUGE_REG_SOC				0x04
#define LIPO_GAUGE_REG_MODE				0x06
#define LIPO_GAUGE_REG_VERSION			0x08
#define LIPO_GAUGE_REG_CONFIG			0x0C
#define LIPO_GAUGE_REG_COMMAND			0xFE

#define LIPO_GAUGE_DEFAULT_CFG_H		0x97
#define LIPO_GAUGE_DEFAULT_CFG_L		0x1C


void LipoGaugeInit(void);
void LipoGaugeReset(void);
void LipoGaugeConfig(void);
WORD LipoGaugeReadVolt(void);
BYTE LipoGaugeReadSOC(void);
WORD LipoGaugeReadVersion(void);
void LipoGaugeReadConfig(BYTE * pconfig);
void LipoGaugeEnableSleep(void);
void LipoGaugeDisableSleep(void);
void LipoGaugeSetAlert(BYTE threshold);
void LipoGaugeClearAlert(void);

#endif
#include "lipogauge.h"


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Sets up the IC for a read operation from a specific register
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void _LipoGaugePrepareReg(BYTE registerAddr)
{
	i2cSendStart();
	i2cWaitForComplete();
	i2cSendByte(LIPO_GAUGE_DEVICE_ADDR);
	i2cWaitForComplete();
	i2cSendByte(registerAddr);
	i2cWaitForComplete();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// writes 0x54 & 0x00 to the COMMAND register, which causes the device to reset
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void LipoGaugeReset(void)
{
	_LipoGaugePrepareReg(LIPO_GAUGE_REG_COMMAND);
	i2cSendByte(0x54);
	i2cWaitForComplete();
	i2cSendByte(0x00);
	i2cWaitForComplete();
	i2cSendStop();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// sets default values + alert @ 32%
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void LipoGaugeConfig(void)
{
	LipoGaugeSetAlert(32);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
WORD LipoGaugeReadVolt(void)
{
	// setup IC to read from VCELL register
	_LipoGaugePrepareReg(LIPO_GAUGE_REG_VCELL);

	i2cSendStop();

	WORD value = 0;

	// read the VCELL data values
	i2cMasterReceiveNI(LIPO_GAUGE_DEVICE_ADDR, 2, (BYTE*)&value);

	// swap bytes for network to little endian byte ordering
	// move lower 4 bits to upper byte, and upper byte to lower byte
	value = (((value & 0x00F0) << 4) | ((value & 0xFF00) >> 8));

	// yields a value in millivolts?
	// 0-5v range, 12 bit, 4096 steps
	// 5 / 4095 ~= 1.22
	// 2048 * 1.22 ~= 2.5
	// 2048 is midway point, 2.5volts
	return value ;// * 1.22;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// returns the SOC, in increments of 0.390625% (100/256)
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BYTE LipoGaugeReadSOC(void)
{
	BYTE soc[2];

	// setup IC to read from VCELL register
	_LipoGaugePrepareReg(LIPO_GAUGE_REG_SOC);
	i2cSendStop();

	// read the VCELL data values
	i2cMasterReceiveNI(LIPO_GAUGE_DEVICE_ADDR, 2, soc);

	return soc[0];
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
WORD LipoGaugeReadVersion(void)
{
	return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void LipoGaugeReadConfig(BYTE * pconfig)
{
	if (NULL == pconfig)
		return;

	// setup IC to read from VCELL register
	_LipoGaugePrepareReg(LIPO_GAUGE_REG_CONFIG);

	// read the VCELL data values
	i2cMasterReceiveNI(LIPO_GAUGE_DEVICE_ADDR, 2, pconfig);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void LipoGaugeEnableSleep(void)
{
	BYTE buff[3];

	// get config bits
	LipoGaugeReadConfig(&buff[1]);

	buff[0] = LIPO_GAUGE_REG_CONFIG;
	buff[1] = LIPO_GAUGE_DEFAULT_CFG_H;
	buff[2] |= (1<<LIPO_SLEEP);			// set sleep flag

	// write out config bits
	i2cMasterSendNI(LIPO_GAUGE_DEVICE_ADDR, 3, buff);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void LipoGaugeDisableSleep(void)
{
	BYTE buff[3];

	// get config bits
	LipoGaugeReadConfig(&buff[1]);

	buff[0] = LIPO_GAUGE_REG_CONFIG;
	buff[1] = LIPO_GAUGE_DEFAULT_CFG_H;
	buff[2] &= ~(1<<LIPO_SLEEP);			// clear sleep flag

	// write out config bits
	i2cMasterSendNI(LIPO_GAUGE_DEVICE_ADDR, 3, buff);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void LipoGaugeSetAlert(BYTE threshold)
{
	_LipoGaugePrepareReg(LIPO_GAUGE_REG_CONFIG);
	i2cSendByte(0x97);
	i2cWaitForComplete();
	i2cSendByte((32 - threshold) & 0x1F);
	i2cWaitForComplete();
	i2cSendStop();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void LipoGaugeClearAlert(void)
{
	BYTE buff[3];

	// get config bits
	LipoGaugeReadConfig(&buff[1]);

	buff[0] = LIPO_GAUGE_REG_CONFIG;
	buff[1] = LIPO_GAUGE_DEFAULT_CFG_H;
	buff[2] &= ~(1<<LIPO_ALERT);			// clear alert flag

	// write out config bits
	i2cMasterSendNI(LIPO_GAUGE_DEVICE_ADDR, 3, buff);
}


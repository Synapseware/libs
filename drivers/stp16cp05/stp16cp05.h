#ifndef __STP16CP05_H__
#define __STP16CP05_H__

#include <avr/io.h>
#include <avr/eeprom.h>


class stp16cp05
{
public:
    stp16cp05(uint8_t channels, uint8_t port, uint8_t ddr, uint8_t oe, uint8_t le, uint8_t clk, uint8_t data);

    void Init(void);
    void Refresh(void);

private:
    void SetDirection(uint8_t direction);
    void UpdateOffset(void);
    uint8_t GetChannelCount(void);

    // data members
    uint8_t _channels;
    uint8_t _port;
    uint8_t _ddr;
    uint8_t _oe;
    uint8_t _le;
    uint8_t _clk;
    uint8_t _data;
    uint8_t _direction;
    uint8_t _offset;
};




#endif

#include "stp16cp05.h"


const uint8_t _oe                    = PORTD6;     // When low, output circuits respond to data
const uint8_t _le                    = PORTD3;     // When low, latch circuits hold previous data
const uint8_t _clk                   = PORTD4;     // Chips read serial data on 1->0 clock transition
const uint8_t _data                  = PORTD5;     // Write data while clock line is low only


//--------------------------------------------------------------------------------
stp16cp05::stp16cp05(uint8_t channels, uint8_t port, uint8_t ddr, uint8_t oe, uint8_t le, uint8_t clk, uint8_t data)
{
    _channels = channels;
    _port = port;
    _ddr = ddr;
    _oe = oe;
    _le = le;
    _clk = clk;
    _data = data;

    _direction = 0;
    _offset = 0;
}


//--------------------------------------------------------------------------------
void stp16cp05::Init(void)
{
    _SFR_IO8(_ddr)  |=  (1<<_oe) | (1<<_le) | (1<<_clk) | (1<<_data);
    _SFR_IO8(_port) |=  (1<<_oe) | (1<<_le);
    _SFR_IO8(_port) &=  ~((1<<_clk) | (1<<_data));
}


//--------------------------------------------------------------------------------
void stp16cp05::Refresh(void)
{

}


//--------------------------------------------------------------------------------
void stp16cp05::SetDirection(uint8_t direction)
{

}


//--------------------------------------------------------------------------------
void stp16cp05::UpdateOffset(void)
{
    if (_direction)
    {
        _offset = (_offset - 1) & (_channels - 1);
    }
    else
    {
        _offset &= (_offset - 1) & (_channels - 1);
    }
}


//--------------------------------------------------------------------------------
uint8_t stp16cp05::GetChannelCount(void)
{
    return _channels;
}
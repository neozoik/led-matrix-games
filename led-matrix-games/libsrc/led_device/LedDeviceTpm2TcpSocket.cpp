#include "LedDeviceTpm2TcpSocket.h"

LedDeviceTpm2TcpSocket::LedDeviceTpm2TcpSocket(const std::string& address) :
        TcpSocketDevice(address),
        _ledBuffer(0)
{
    // empty
}

int LedDeviceTpm2TcpSocket::write(const Screen &screen)
{
    uint8_t rOffset = 0, gOffset = 1, bOffset = 2;
    if (_colorOrder == GRB) {
        gOffset = 0;
        rOffset = 1;
    }

    if (_ledBuffer.size() == 0) {
        uint32_t count = screen.width * screen.height * 3;
        _ledBuffer.resize(5 + count);
        _ledBuffer[0] = 0xC9; // block-start byte
        _ledBuffer[1] = 0xDA; // DATA frame
        _ledBuffer[2] = (uint8_t) ((count >> 8) & 0xFF); // frame size high byte
        _ledBuffer[3] = (uint8_t) (count & 0xFF); // frame size low byte
        _ledBuffer.back() = 0x36; // block-end byte
    }

    // write data
    // special order for that special matrix -> TODO make generic handling
    //memcpy(4 + _ledBuffer.data(), ledValues.data(), ledValues.size() * 3);
    ColorRgb color;
    uint32_t index = 0;
    for (int i = 0; i < screen.height; ++i) {
        for (int j = 0; j < screen.width / 2; ++j) {
            color = screen.get(j, i);
            _ledBuffer[4 + index + rOffset] = color.red;
            _ledBuffer[4 + index + gOffset] = color.green;
            _ledBuffer[4 + index + bOffset] = color.blue;
            index += 3;
        }
    }
    for (int i = 0; i < screen.height; ++i) {
        for (int j = 8; j < screen.width; ++j) {
            color = screen.get(j, i);
            _ledBuffer[4 + index + rOffset] = color.red;
            _ledBuffer[4 + index + gOffset] = color.green;
            _ledBuffer[4 + index + bOffset] = color.blue;
            index += 3;
        }
    }

    return writeBytes(_ledBuffer.size(), _ledBuffer.data());
}

int LedDeviceTpm2TcpSocket::switchOff()
{
    memset(4 + _ledBuffer.data(), 0, _ledBuffer.size() - 5);
    return writeBytes(_ledBuffer.size(), _ledBuffer.data());
}
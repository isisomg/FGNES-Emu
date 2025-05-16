#pragma once
#include "Tipos.h"

class Mapper {
public:
    virtual Byte cpuRead(DWord addr) = 0;
    virtual void cpuWrite(DWord addr, Byte data) = 0;
    virtual Byte ppuRead(DWord addr) = 0;
    virtual void ppuWrite(DWord addr, Byte data) = 0;

    virtual ~Mapper() = default;
};

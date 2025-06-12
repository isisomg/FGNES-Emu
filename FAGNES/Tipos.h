#pragma once
#include <cstdint>
using Byte = uint8_t;
using DWord = unsigned short;

enum struct MirroringSelect {
	Horizontal,
	Vertical,
	FourScreen, // NAO IMPLEMENTTADO MEDONHO
	OneScreenLower,
	OneScreenUpper
};
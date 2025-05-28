#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

// Windows
#include <Windows.h>
#include <atlbase.h>
#include <dbghelp.h>
#include <TlHelp32.h>
#include <wincrypt.h>
#include <shlwapi.h>

// C++
#include <cassert>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <queue>
#include <sstream>
#include <string>

// Submodules
#include <gsl/gsl>
#include <MinHook.h>

/*
Size found using Ghidra
IMAGE_NT_HEADERS32 -> IMAGE_OPTIONAL_HEADER32: SizeOfImage - SizeOfHeaders
                                                DBA000h     - 1000h
*/
constexpr auto BINARY_PAYLOAD_SIZE = 0x599552d;//0x15C1000;

constexpr auto MOD_NAME = "iw1x";

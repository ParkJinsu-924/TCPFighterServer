#pragma once
#pragma comment(lib, "ws2_32")
#pragma comment(lib, "winmm.lib")
#include <WinSock2.h>
#include <unordered_map>
#include <Windows.h>
#include <iostream>
#include <conio.h>
#include <map>
#include <set>
#include "CRingBuffer.h"
#include "CSerializationBuffer.h"
#include "Packet.h"
#include "Struct.h"
using namespace std;
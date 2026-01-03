#pragma once
#include <stdint.h>

bool webota_begin(uint16_t port = 80);
void webota_loop(); // (kan vara tom, men håll API symmetriskt)

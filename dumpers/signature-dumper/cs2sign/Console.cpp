#include "Console.h"

HANDLE Console::hConsole = INVALID_HANDLE_VALUE;
WORD Console::originalAttributes = 0;


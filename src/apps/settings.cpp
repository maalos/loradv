#include <config.h>

bool settingsAppInitialized = false;

void settingsApp()
{
    if (!settingsAppInitialized)
    {
        tft.setFreeFont(&DejaVu_Sans_Mono_Bold_24);
        settingsAppInitialized = true;
    }
}
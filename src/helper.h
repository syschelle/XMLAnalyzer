#pragma once

#include <Arduino.h>

// Mount LittleFS without formatting. OTA-safe: never erases data automatically.
// If the first mount attempt fails, the failure is cached and not retried automatically.
bool ensureFsMounted();

// Cached LittleFS state. These functions do not try to mount or format.
bool isFsMounted();
bool fsMountWasTried();
bool fsMountFailed();

// Explicit recovery action. This erases only the LittleFS partition, not firmware/NVS.
// Use only after the user has confirmed it.
bool formatFsForRecovery();

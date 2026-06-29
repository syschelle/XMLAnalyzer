#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include "globals.h"
#include <cstdarg>

extern Preferences preferences;
extern int amountOfWater;

void taskShellyStatus(void *parameter){
  static UBaseType_t minFree = UINT32_MAX;
  static ShellyValues lastGoodMain;
  static ShellyValues lastGoodLight;
  static ShellyValues lastGoodHumidifier;
  static ShellyValues lastGoodHeater;
  static ShellyValues lastGoodFan;
  static ShellyValues lastGoodExhaust;
  static bool haveMainGood = false;
  static bool haveLightGood = false;
  static bool haveHumidifierGood = false;
  static bool haveHeaterGood = false;
  static bool haveFanGood = false;
  static bool haveExhaustGood = false;

  for (;;) {
    UBaseType_t freeWords = uxTaskGetStackHighWaterMark(NULL);
    if (freeWords < minFree) {
      minFree = freeWords;
    }

    static uint32_t lastLogMs = 0;
    const uint32_t logIntervalMs = debugLog ? 5000UL : 60000UL;

    if (millis() - lastLogMs > logIntervalMs) {
      lastLogMs = millis();

      char buf[96];

      snprintf(
        buf,
        sizeof(buf),
        "[TASK][CheckShellyStatus] free=%u words (%u bytes), min=%u words",
        freeWords,
        freeWords * sizeof(StackType_t),
        minFree
      );

      logPrint(String(buf));
  }

    // Read Shelly main; keep previous good value on transient request failure
    ShellyValues mainNow = getShellyValues(settings.shelly.main, 0);
    if (mainNow.ok) {
      shelly.main.values = mainNow;
      lastGoodMain = mainNow;
      haveMainGood = true;
    } else if (haveMainGood) {
      shelly.main.values = lastGoodMain;
    }

    // Read Shelly light; keep previous good value on transient request failure
    ShellyValues lightNow = getShellyValues(settings.shelly.light, 0);
    if (lightNow.ok) {
      shelly.light.values = lightNow;
      lastGoodLight = lightNow;
      haveLightGood = true;
    } else if (haveLightGood) {
      shelly.light.values = lastGoodLight;
    }

    ShellyValues humNow = getShellyValues(settings.shelly.humidifier, 0);
    if (humNow.ok) {
      shelly.humidifier.values = humNow;
      lastGoodHumidifier = humNow;
      haveHumidifierGood = true;
    } else if (haveHumidifierGood) {
      shelly.humidifier.values = lastGoodHumidifier;
    }

    ShellyValues heaterNow = getShellyValues(settings.shelly.heater, 0);
    if (heaterNow.ok) {
      shelly.heater.values = heaterNow;
      lastGoodHeater = heaterNow;
      haveHeaterGood = true;
    } else if (haveHeaterGood) {
      shelly.heater.values = lastGoodHeater;
    }

    ShellyValues fanNow = getShellyValues(settings.shelly.fan, 0);
    if (fanNow.ok) {
      shelly.fan.values = fanNow;
      lastGoodFan = fanNow;
      haveFanGood = true;
    } else if (haveFanGood) {
      shelly.fan.values = lastGoodFan;
    }

    ShellyValues exhaustNow = getShellyValues(settings.shelly.exhaust, 0);
    if (exhaustNow.ok) {
      shelly.exhaust.values = exhaustNow;
      lastGoodExhaust = exhaustNow;
      haveExhaustGood = true;
    } else if (haveExhaustGood) {
      shelly.exhaust.values = lastGoodExhaust;
    }


    // task delay 10 seconds
    vTaskDelay(pdMS_TO_TICKS(10000)); 
  }
}
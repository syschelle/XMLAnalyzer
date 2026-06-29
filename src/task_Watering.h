#pragma once
#include <Arduino.h>
#include "globals.h"
#include "function.h"

void taskWatering(void *parameter) {
  static UBaseType_t minFree = UINT32_MAX;

  for (;;) {
    UBaseType_t freeWords = uxTaskGetStackHighWaterMark(NULL);

    if (freeWords < minFree) {
      minFree = freeWords;
    }

    static uint32_t last = 0;
    if (millis() - last > 5000) {
      last = millis();

      char buf[96];
      snprintf(
        buf,
        sizeof(buf),
        "[TASK][Watering] free=%u words (%u bytes), min=%u words",
        freeWords,
        freeWords * sizeof(StackType_t),
        minFree
      );

      logPrint(String(buf));
    }

    if (irrigation.irrigationRuns <= 0) {
      irrigation.wTimeLeft = "00:00";
      vTaskDelay(pdMS_TO_TICKS(10000));
      continue;
    }

    if (activeRelayCount < 8) {
      logPrint("[IRRIGATION] Aborted: irrigation requires 8 relays.");
      irrigation.irrigationRuns = 0;
      irrigation.wTimeLeft = "00:00";
      vTaskDelay(pdMS_TO_TICKS(1000));
      continue;
    }

    irrigation.wTimeLeft = calculateEndtimeWatering();

    const int pumpRelays[] = {5, 6, 7};
    const size_t pumpRelayCount = sizeof(pumpRelays) / sizeof(pumpRelays[0]);

    for (size_t i = 0; i < pumpRelayCount; i++) {
      setRelay(pumpRelays[i], true);
      vTaskDelay(pdMS_TO_TICKS(secondsToMilliseconds(irrigation.timePerTask)));
      setRelay(pumpRelays[i], false);

      if (i + 1 < pumpRelayCount) {
        vTaskDelay(pdMS_TO_TICKS(250));
      }
    }

    irrigation.irrigationRuns--;

    logPrint("[IRRIGATION] Remaining irrigation runs: " + String(irrigation.irrigationRuns));

    if (irrigation.irrigationRuns <= 0) {
      irrigation.wTimeLeft = "00:00";

      if (language == "de") {
        sendPushover(boxName + "Bewässerung abgeschlossen.", "Bewässerung abgeschlossen.");
        sendGotify(boxName + "Bewässerung abgeschlossen.", "Bewässerung abgeschlossen.");
      } else {
        sendPushover(boxName + "Irrigation completed.", "Irrigation completed.");
        sendGotify(boxName + "Irrigation completed.", "Irrigation completed.");
      }

      vTaskDelay(pdMS_TO_TICKS(1000));
      continue;
    }

    float level = pingTankLevel(TRIG, ECHO);
    if (level >= 0.0f) {
      tankLevelCm = level;
    }

    logPrint("[IRRIGATION] Remaining irrigation runs: " + String(irrigation.irrigationRuns));

    vTaskDelay(pdMS_TO_TICKS(minutesToMilliseconds(irrigation.betweenTasks)));
  }
}
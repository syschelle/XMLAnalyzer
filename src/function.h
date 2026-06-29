#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <LittleFS.h>
#include <index_html.h>
#include <time.h>
#include <deque>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFiClientSecure.h>
#include <IPAddress.h>
#include <HTTPClient.h>
#include <mbedtls/md.h>
// Base64 (for Basic auth) via mbedTLS (available in ESP32 Arduino core)
#include <mbedtls/base64.h>

// global, functions, html code, js code and css code includes
#include "globals.h"
#include <runtime.h>

// Include platform-specific headers for inet_ntoa
#if defined(ARDUINO_ARCH_ESP32) || defined(ESP_PLATFORM)
#include <lwip/inet.h>

// Light schedule refresh window state (file-scope)
static uint32_t g_scheduleInitStartMs = 0;
static uint32_t g_lastScheduleFetchMs = 0;
static bool g_scheduleResolved = false;

// Forward declarations (used before definitions)
struct ShellyDevice;
struct ShellyValues;
ShellyValues getShellyValues(ShellyDevice& dev, int switchId, int port = 80);
static bool shellyResetEnergyCounters(ShellyDevice &dev, uint8_t switchId, uint16_t port);

// Called early (e.g. from handleNewGrow) but implemented further below.
static bool applyGrowLightSchedule();

#endif


// If no pin is defined elsewhere, default to GPIO4
#ifndef DS18B20_PIN
#define DS18B20_PIN 4
#endif

// Declare OneWire + DallasTemperature objects (defined in function.cpp to avoid multiple/conflicting definitions)
extern OneWire oneWire;
extern DallasTemperature sensors;

// declare the global WebServer instance defined elsewhere
extern WebServer server;
extern Preferences preferences;
extern const char* htmlPage;
extern std::deque<String> logBuffer;
extern volatile float DS18B20STemperature;
extern unsigned long relayOffTime[];
extern bool relayActive[];

String getTimeString() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "--:--:--";
  }

  char buf[9];
  snprintf(buf, sizeof(buf), "%02d:%02d:%02d",
           timeinfo.tm_hour,
           timeinfo.tm_min,
           timeinfo.tm_sec);
  return String(buf);
}

// log buffer to store recent log lines
void logPrint(const String& msg) {
  time_t now = time(nullptr);
  struct tm t;
  localtime_r(&now, &t);

  char ts[20];
  strftime(ts, sizeof(ts), "%H:%M:%S", &t);

  String line = String(ts) + " - " + msg;

  Serial.println(line);

  if (logBufferMutex) {
    if (xSemaphoreTake(logBufferMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
      logBuffer.push_back(line);
      if (logBuffer.size() > LOG_MAX_LINES) {
        logBuffer.pop_front();
      }
      xSemaphoreGive(logBufferMutex);
    } else {
      // Kein logPrint() hier aufrufen, sonst Rekursion möglich
      Serial.println("[LOG] logBufferMutex timeout, line not buffered");
    }
  } else {
    // Fallback falls Mutex noch nicht existiert
    logBuffer.push_back(line);
    if (logBuffer.size() > LOG_MAX_LINES) {
      logBuffer.pop_front();
    }
  }
}

static void requestSafeRestart(uint32_t delayMs = 1500) {
  g_restartRequested = true;
  g_restartAtMs = millis() + delayMs;
}

// Helper function to calculate day/week counters based on a start date string (returns false if date is invalid)
bool calculateCounter(const String& date, int& day, int& week) {
  if (date == "") {
    day = 0;
    week = 0;
    return false;
  }

  calculateTimeSince(date, day, week);
  return true;
}

// Updates the grow time values (currentGrowDay, currentGrowWeek, currentPhaseDay, currentPhaseWeek) based on the configured start dates and current time.
void updateGrowTimeValues() {
  if (startDate == "") {
    settings.grow.currentGrowDay = 0;
    settings.grow.currentGrowWeek = 0;
    settings.grow.currentPhaseDay = 0;
    settings.grow.currentPhaseWeek = 0;
    return;
  }

  calculateCounter(
    startDate,
    settings.grow.currentGrowDay,
    settings.grow.currentGrowWeek
  );

  switch (settings.grow.currentPhase) {
    case 1:
      calculateCounter(startDate, settings.grow.currentPhaseDay, settings.grow.currentPhaseWeek);
      break;

    case 2:
      calculateCounter(startFlowering, settings.grow.currentPhaseDay, settings.grow.currentPhaseWeek);
      break;

    case 3:
      calculateCounter(startDrying, settings.grow.currentPhaseDay, settings.grow.currentPhaseWeek);
      break;

    default:
      settings.grow.currentPhaseDay = 0;
      settings.grow.currentPhaseWeek = 0;
      break;
  }
}

bool checkFS() {
  // Info abfragen (geht nur wenn gemountet)
  size_t total = LittleFS.totalBytes();
  size_t used  = LittleFS.usedBytes();

  if (total == 0) {
    logPrint("[LITTLEFS][ERROR] totalBytes=0 (not mounted?)");
    return false;
  }

  // Test: Root existiert / Datei kann geöffnet werden
  File f = LittleFS.open("/.health", FILE_WRITE);
  if (!f) {
    logPrint("[LITTLEFS][ERROR] cannot open /.health");
    return false;
  }
  f.println("ok");
  f.close();

  logPrint("[LITTLEFS] OK total=" + String(total) + " used=" + String(used));
  return true;
}

void sensorTask(void* pvParameters) {
    static uint32_t lastLogMs = 0;
    const uint32_t debugLogIntervalMs = 60000; // 60s

    for (;;) {
        if (debugLog && (millis() - lastLogMs > debugLogIntervalMs)) {
            lastLogMs = millis();

            UBaseType_t freeWords = uxTaskGetStackHighWaterMark(NULL);
            logPrint(
                "[TASK][sensor] free stack: " + String(freeWords) +
                " words (" + String(freeWords * sizeof(StackType_t)) + " bytes)"
            );
        }

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

// Helper function: Send JSON from PROGMEM
void sendJSON_P(const char* jsonP) {
  server.sendHeader("Cache-Control", "no-store");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "application/json");
  // stream the content:
  WiFiClient client = server.client();
  // Attention: PROGMEM -> Read in pieces
  PGM_P p = reinterpret_cast<PGM_P>(jsonP);
  char c;
  while ((c = pgm_read_byte(p++)) != 0) {
    client.write(c);
  }
}

// soft ap configuration
void startSoftAP() {
  IPAddress ip = WiFi.softAPIP();
  Serial.println("[SoftAP] Start SoftAP mode. IP address: " + ip.toString());

  
  // Disconnect previous connections
  WiFi.disconnect(true, true);  
  delay(100);
  
  WiFi.mode(WIFI_AP_STA);

  //last parameter 'false' = do not hide SSID
  String chipId = String((uint32_t)ESP.getEfuseMac(), HEX);
  String apName = String(KEY_APSSID) + "_" + chipId;

  // Harden weak/default AP password at runtime.
  String apPass = String(KEY_APPASSWORD);
  
  bool ok = WiFi.softAP(apName.c_str(), apPass.c_str(), /*channel*/ 1, /*hidden*/ false);
  if (!ok) {
    Serial.println("[SoftAP] Error starting the SoftAP!");
    return;
  }
}

// Helper function to save a string preference if the corresponding argument is present
void savePrefString(
  const char* argName,
  const char* prefKey,
  String& targetVar,
  bool logValue = true,
  const char* logLabel = nullptr
) {
  if (!server.hasArg(argName)) return;

  targetVar = server.arg(argName);
  preferences.putString(prefKey, targetVar);

  if (logLabel == nullptr) logLabel = prefKey;

  if (logValue) {
    logPrint("[PREFERENCES SAVE] " + String(logLabel) + " written = " + targetVar);
  } else {
    logPrint("[PREFERENCES SAVE] " + String(logLabel) + " updated (hidden)");
  }
}

// Helper function to save an integer preference if the corresponding argument is present
void savePrefInt(
  const char* argName,
  const char* prefKey,
  int& targetVar,
  bool logValue = true,
  const char* logLabel = nullptr
) {
  if (!server.hasArg(argName)) return;

  targetVar = server.arg(argName).toInt();
  preferences.putInt(prefKey, targetVar);

  if (logLabel == nullptr) logLabel = prefKey;

  if (logValue) {
    logPrint("[PREFERENCES SAVE] " + String(logLabel) + " written = " + String(targetVar));
  } else {
    logPrint("[PREFERENCES SAVE] " + String(logLabel) + " updated (hidden)");
  }
}

void savePrefFloat(
  const char* argName,
  const char* prefKey,
  float& targetVar,
  bool logValue = true,
  const char* logLabel = nullptr
) {
  if (!server.hasArg(argName)) return;

  String v = server.arg(argName);
  targetVar = v.toFloat();

  preferences.putFloat(prefKey, targetVar);

  if (logLabel == nullptr) logLabel = prefKey;

  if (logValue) {
    logPrint("[PREFERENCES SAVE] " + String(logLabel) +
             " = " + String(targetVar, 2));
  } else {
    logPrint("[PREFERENCES SAVE] " + String(logLabel) + " updated (hidden)");
  }
}

// Helper function to save a boolean preference if the corresponding argument is present
void savePrefBool(
  const char* argName,
  const char* prefKey,
  bool& targetVar,
  bool logValue = true,
  const char* logLabel = nullptr
) {
  if (!server.hasArg(argName)) return;

  String val = server.arg(argName);

  // HTML Checkbox: "on", "1", "true"
  targetVar = (val == "1" || val == "on" || val == "true");

  preferences.putBool(prefKey, targetVar);

  if (logLabel == nullptr) logLabel = prefKey;

  if (logValue) {
    logPrint("[PREFERENCES SAVE] " + String(logLabel) +
             " = " + String(targetVar ? "true" : "false"));
  } else {
    logPrint("[PREFERENCES SAVE] " + String(logLabel) + " updated (hidden)");
  }
}

void loadPrefInt(
  const char* prefKey,
  int& targetVar,
  int defaultValue,
  bool logValue,
  const char* logLabel
) {
  targetVar = preferences.getInt(prefKey, defaultValue);

  if (logLabel == nullptr) logLabel = prefKey;

  if (logValue) {
    logPrint("[PREFERENCES LOAD] " + String(logLabel) + " read = " + String(targetVar));
  } else {
    logPrint("[PREFERENCES LOAD] " + String(logLabel) + " read (hidden)");
  }
}

void loadPrefFloat(
  const char* prefKey,
  float& targetVar,
  float defaultValue,
  bool logValue,
  const char* logLabel,
  uint8_t decimals
) {
  targetVar = preferences.getFloat(prefKey, defaultValue);

  if (logLabel == nullptr) logLabel = prefKey;

  if (logValue) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%.*f", decimals, targetVar);
    logPrint("[PREFERENCES LOAD] " + String(logLabel) + " read = " + String(buf));
  } else {
    logPrint("[PREFERENCES LOAD] " + String(logLabel) + " read (hidden)");
  }
}

void loadPrefBool(
  const char* prefKey,
  bool& targetVar,
  bool defaultValue,
  bool logValue,
  const char* logLabel
) {
  targetVar = preferences.getBool(prefKey, defaultValue);

  if (logLabel == nullptr) logLabel = prefKey;

  if (logValue) {
    logPrint("[PREFERENCES LOAD] " + String(logLabel) + " read = " + String(targetVar ? "true" : "false"));
  } else {
    logPrint("[PREFERENCES LOAD] " + String(logLabel) + " read (hidden)");
  }
}

void loadPrefString(
  const char* prefKey,
  String& targetVar,
  const char* defaultValue,
  bool logValue,
  const char* logLabel
) {
  targetVar = preferences.getString(prefKey, defaultValue);

  if (logLabel == nullptr) logLabel = prefKey;

  if (logValue) {
    logPrint("[PREFERENCES LOAD] " + String(logLabel) + " read = " + targetVar);
  } else {
    logPrint("[PREFERENCES LOAD] " + String(logLabel) + " read (hidden)");
  }
}

// Handler for POST /saverunsettings - saves run settings sent as JSON in the request body
void handleSaveRunsettings() {
  if (!server.hasArg("plain")) {
    server.send(
      400,
      "application/json; charset=utf-8",
      "{\"ok\":false,\"err\":\"missing_body\"}"
    );
    return;
  }

  JsonDocument doc;
  if (deserializeJson(doc, server.arg("plain"))) {
    server.send(
      400,
      "application/json; charset=utf-8",
      "{\"ok\":false,\"err\":\"bad_json\"}"
    );
    return;
  }

  if (!preferences.begin(PREF_NS, false)) {
    logPrint("[PREF][ERROR] preferences.begin() failed. Check PREF_NS.");
    server.send(
      500,
      "application/json; charset=utf-8",
      "{\"ok\":false,\"err\":\"prefs\"}"
    );
    return;
  }

  auto clampIntLocal = [](int value, int minValue, int maxValue) -> int {
    if (value < minValue) return minValue;
    if (value > maxValue) return maxValue;
    return value;
  };

  auto clampFloatLocal = [](float value, float minValue, float maxValue) -> float {
    if (value < minValue) return minValue;
    if (value > maxValue) return maxValue;
    return value;
  };

  auto saveStringJson = [&](const char* jsonKey, const char* prefKey, String& targetVar, const char* label) {
    if (doc[jsonKey].isNull()) return;
    targetVar = String(doc[jsonKey].as<const char*>());
    preferences.putString(prefKey, targetVar);
    logPrint("[PREFERENCES SAVE] " + String(label) + " written = " + targetVar);
  };

  auto saveIntJson = [&](const char* jsonKey, const char* prefKey, int& targetVar, const char* label) {
    if (doc[jsonKey].isNull()) return;
    targetVar = doc[jsonKey].as<int>();
    preferences.putInt(prefKey, targetVar);
    logPrint("[PREFERENCES SAVE] " + String(label) + " written = " + String(targetVar));
  };

  auto saveFloatJson = [&](const char* jsonKey, const char* prefKey, float& targetVar, const char* label) {
    if (doc[jsonKey].isNull()) return;
    targetVar = doc[jsonKey].as<float>();
    preferences.putFloat(prefKey, targetVar);
    logPrint("[PREFERENCES SAVE] " + String(label) + " = " + String(targetVar, 2));
  };

  auto saveBoolJson = [&](const char* jsonKey, const char* prefKey, bool& targetVar, const char* label) {
    if (doc[jsonKey].isNull()) return;
    targetVar = doc[jsonKey].as<bool>();
    preferences.putBool(prefKey, targetVar);
    logPrint("[PREFERENCES SAVE] " + String(label) + " = " + String(targetVar ? "true" : "false"));
  };

  // -----------------------------
  // Runsettings - grow settings
  // -----------------------------
  saveStringJson("webGrowStart",       KEY_STARTDATE,     startDate,                    "Grow Start Date");
  saveStringJson("webFloweringStart",  KEY_FLOWERDATE,    startFlowering,               "Flowering Start Date");
  saveStringJson("webDryingStart",     KEY_DRYINGDATE,    startDrying,                  "Drying Start Date");

  saveIntJson("webCurrentPhase",       KEY_CURRENTPHASE,  settings.grow.currentPhase,   "Current Phase");
  settings.grow.currentPhase = clampIntLocal(settings.grow.currentPhase, 1, 3);
  preferences.putInt(KEY_CURRENTPHASE, settings.grow.currentPhase);
  curPhase = settings.grow.currentPhase;

  saveFloatJson("webTargetTemp",       KEY_TARGETTEMP,    settings.grow.targetTemperature, "Target Temperature");
  settings.grow.targetTemperature = clampFloatLocal(settings.grow.targetTemperature, 18.0f, 30.0f);
  preferences.putFloat(KEY_TARGETTEMP, settings.grow.targetTemperature);
  targetTemperature = settings.grow.targetTemperature;

  saveFloatJson("webTargetVPD",        KEY_TARGETVPD,     settings.grow.targetVPD,      "Target VPD");
  settings.grow.targetVPD = clampFloatLocal(settings.grow.targetVPD, 0.50f, 3.50f);
  preferences.putFloat(KEY_TARGETVPD, settings.grow.targetVPD);
  targetVPD = settings.grow.targetVPD;

  saveBoolJson("webMinVPDMonitoring", KEY_MINVPD_MON, settings.grow.minVpdMonEnabled, "Min VPD Monitoring Enabled");
  saveFloatJson("webMinVPD", KEY_MINVPD, settings.grow.minVPD, "Min VPD");
  saveFloatJson("webHysteresis", KEY_HYSTERESIS, settings.grow.vpdHysteresis, "VPD Hysteresis");

  saveFloatJson("webOffsetLeafTemp",   KEY_LEAFTEMP,      settings.grow.offsetLeafTemperature, "Leaf Temperature Offset");
  settings.grow.offsetLeafTemperature = clampFloatLocal(settings.grow.offsetLeafTemperature, -3.0f, 0.0f);
  preferences.putFloat(KEY_LEAFTEMP, settings.grow.offsetLeafTemperature);
  offsetLeafTemperature = settings.grow.offsetLeafTemperature;

  saveFloatJson("webAmountOfWater",    KEY_AMOUNTOFWATER, irrigation.amountOfWater,     "Amount of Water");
  irrigation.amountOfWater = clampFloatLocal(irrigation.amountOfWater, 10.0f, 500.0f);
  preferences.putFloat(KEY_AMOUNTOFWATER, irrigation.amountOfWater);

  saveIntJson("webTimePerTask",        KEY_TIMEPERTASK,   irrigation.timePerTask,       "Time Per Task");
  irrigation.timePerTask = clampIntLocal(irrigation.timePerTask, 1, 10);
  preferences.putInt(KEY_TIMEPERTASK, irrigation.timePerTask);

  saveIntJson("webBetweenTasks",       KEY_BETWEENTASKS,  irrigation.betweenTasks,      "Between Tasks");
  irrigation.betweenTasks = clampIntLocal(irrigation.betweenTasks, 1, 10);
  preferences.putInt(KEY_BETWEENTASKS, irrigation.betweenTasks);

  saveFloatJson("webIrrigation",       KEY_IRRIGATION,    irrigation.amount,  "Irrigation Amount");
  irrigation.amount = clampFloatLocal(irrigation.amount, 100.0f, 3000.0f);
  preferences.putFloat(KEY_IRRIGATION, irrigation.amount);

  saveFloatJson("webMinTank",          KEY_MINTANK,       irrigation.tank.min,          "Tank Min Level (cm)");
  saveFloatJson("webMaxTank",          KEY_MAXTANK,       irrigation.tank.max,          "Tank Max Level (cm)");

  if (irrigation.tank.min > irrigation.tank.max) {
    float tmp = irrigation.tank.min;
    irrigation.tank.min = irrigation.tank.max;
    irrigation.tank.max = tmp;
  }
  preferences.putFloat(KEY_MINTANK, irrigation.tank.min);
  preferences.putFloat(KEY_MAXTANK, irrigation.tank.max);

  saveIntJson("webHeatingSource",      KEY_HEATING_SOURCE, settings.heating.sourceType, "Heating Source");
  settings.heating.sourceType = clampIntLocal(settings.heating.sourceType, 0, 2);
  preferences.putInt(KEY_HEATING_SOURCE, settings.heating.sourceType);

  saveIntJson("webHeatingRelay",       KEY_HEATING_RELAY, settings.heating.Relay,       "Heating Relay");
  settings.heating.Relay = clampIntLocal(settings.heating.Relay, 0, 4);

  if (settings.heating.sourceType != 1) {
    settings.heating.Relay = 0;
  }
  preferences.putInt(KEY_HEATING_RELAY, settings.heating.Relay);
  heatingRelay = settings.heating.Relay;

  // -----------------------------
  // store relay scheduling settings (up to 5 relays, each with enabled, ifLightOff, onMin and offMin)
  // -----------------------------
  JsonArray arr = doc["relays"].as<JsonArray>();
  if (!arr.isNull()) {
    const int maxScheduledRelay = (activeRelayCount < 5) ? activeRelayCount : 5;

    for (JsonObject r : arr) {
      int relay = r["relay"] | 0;
      if (relay < 1 || relay > maxScheduledRelay) continue;

      int idx = relay - 1;

      bool enabled = r["enabled"] | false;
      bool ifLightOff = r["ifLightOff"] | false;
      int onMin  = clampIntLocal((int)(r["onMin"]  | 0), 0, 59);
      int offMin = clampIntLocal((int)(r["offMin"] | 0), 0, 59);

      settings.relay.schedule[idx].enabled    = enabled;
      settings.relay.schedule[idx].ifLightOff = ifLightOff;
      settings.relay.schedule[idx].onMin      = onMin;
      settings.relay.schedule[idx].offMin     = offMin;

      String keyEn  = "relay_enable_" + String(relay);
      String keyILO = "ilo_" + String(relay);

      const char* kOn =
        (relay == 1) ? KEY_RELAY_START_1 :
        (relay == 2) ? KEY_RELAY_START_2 :
        (relay == 3) ? KEY_RELAY_START_3 :
        (relay == 4) ? KEY_RELAY_START_4 :
                       KEY_RELAY_START_5;

      const char* kOff =
        (relay == 1) ? KEY_RELAY_END_1 :
        (relay == 2) ? KEY_RELAY_END_2 :
        (relay == 3) ? KEY_RELAY_END_3 :
        (relay == 4) ? KEY_RELAY_END_4 :
                       KEY_RELAY_END_5;

      preferences.putBool(keyEn.c_str(), enabled);
      preferences.putBool(keyILO.c_str(), ifLightOff);
      preferences.putInt(kOn, onMin);
      preferences.putInt(kOff, offMin);
    }
  }

  preferences.end();

  readPreferences();
  server.send(
    200,
    "application/json; charset=utf-8",
    "{\"ok\":true}"
  );
}

// Handle Shelly settings save

void handleNewGrow() {
  if (!preferences.begin(PREF_NS, false)) {
    logPrint("[PREF][ERROR] preferences.begin() failed (newGrow)");
    server.send(500, "application/json; charset=utf-8", "{\"ok\":false,\"error\":\"prefs\"}");
    return;
  }

  // Today in local time: YYYY-MM-DD
  char dateBuf[11] = {0};
  time_t now = time(nullptr);
  struct tm tmNow {};
  localtime_r(&now, &tmNow);
  snprintf(dateBuf, sizeof(dateBuf), "%04d-%02d-%02d",
           tmNow.tm_year + 1900, tmNow.tm_mon + 1, tmNow.tm_mday);
  const String today(dateBuf);

  // Reset grow dates/phases
  startDate = today;
  startFlowering = "";
  startDrying = "";
  settings.grow.currentPhase = 1; // Vegetativ

  preferences.putString(KEY_STARTDATE, startDate);
  preferences.putString(KEY_FLOWERDATE, startFlowering);
  preferences.putString(KEY_DRYINGDATE, startDrying);
  preferences.putInt(KEY_CURRENTPHASE, curPhase);

  // Reset energy display: store current totals as offset (so UI shows 0)
  // Then try to reset device counters (Gen2), if successful -> offset can be 0.
  auto resetOne = [&](ShellyDevice &d, const char* keyOff) -> bool {
    if (d.ip.length() == 0) return false;

    // read current raw total (offset currently applied inside getShellyValues, so temporarily disable)
    const float prevOff = d.energyOffsetWh;
    d.energyOffsetWh = 0.0f;
    ShellyValues v = getShellyValues(d, 0, 80);
    float rawWh = (!isnan(v.energyWh) ? v.energyWh : 0.0f);

    // baseline offset to show 0
    d.energyOffsetWh = rawWh;
    preferences.putFloat(keyOff, d.energyOffsetWh);

    // try to reset counters on device; if succeeds -> keep offset 0
    bool resetOk = shellyResetEnergyCounters(d, 0, 80);
    if (resetOk) {
      d.energyOffsetWh = 0.0f;
      preferences.putFloat(keyOff, 0.0f);
    }

    // refresh values
    return resetOk;
  };

  bool okMain = resetOne(settings.shelly.main, KEY_SHELLYMAINOFF);
  bool okLight= resetOne(settings.shelly.light,KEY_SHELLYLIGHTOFF);
  bool okHum = resetOne(settings.shelly.humidifier, KEY_SHELLYHUMOFF);

  // Response
  String resp = "{";
  resp += "\"ok\":true,";
  resp += "\"startDate\":\"" + startDate + "\",";
  resp += "\"reset\":{";
  resp += "\"main\":" + String(okMain ? "true" : "false") + ",";
  resp += "\"light\":" + String(okLight ? "true" : "false") + ",";
  resp += "\"humidifier\":" + String(okHum ? "true" : "false") + ",";

  resp += "}}";

  server.send(200, "application/json; charset=utf-8", resp);
}

void handleSaveShellySettings() {
    if (!preferences.begin(PREF_NS, false)) {
        server.send(500, "text/plain", "Failed to open Preferences");
        return;
    }

    struct PreferencesGuard {
        Preferences& prefs;
        ~PreferencesGuard() { prefs.end(); }
    } guard{preferences};

    auto clampInt = [](int value, int minValue, int maxValue) -> int {
        if (value < minValue) return minValue;
        if (value > maxValue) return maxValue;
        return value;
    };

    auto normalizeIPv4 = [](String& ip) {
        ip.trim();
        if (ip.isEmpty()) return;

        IPAddress parsedIp;
        if (!parsedIp.fromString(ip)) {
            ip = "";
            return;
        }

        ip = parsedIp.toString();
    };

    auto isValidTimeHHMM = [](const String& value) -> bool {
        if (value.length() != 5 || value[2] != ':') return false;

        if (!isDigit(value[0]) || !isDigit(value[1]) ||
            !isDigit(value[3]) || !isDigit(value[4])) {
            return false;
        }

        const int hh = (value[0] - '0') * 10 + (value[1] - '0');
        const int mm = (value[3] - '0') * 10 + (value[4] - '0');

        return (hh >= 0 && hh <= 23 && mm >= 0 && mm <= 59);
    };

    auto saveShellyDevice = [&](String& ip,
                                int& gen,
                                const char* webKeyIp,
                                const char* prefKeyIp,
                                const char* labelIp,
                                const char* webKeyGen,
                                const char* prefKeyGen,
                                const char* labelGen) {
        normalizeIPv4(ip);
        gen = clampInt(gen, 1, 2);  // falls nur Gen1 / Gen2 erlaubt sind

        savePrefString(webKeyIp, prefKeyIp, ip, true, labelIp);
        savePrefInt(webKeyGen, prefKeyGen, gen, true, labelGen);
    };

    auto saveGrowLightSettings = [&]() {
        if (server.hasArg("webShellyLightOnTime")) {
            String newTime = server.arg("webShellyLightOnTime");
            newTime.trim();

            if (isValidTimeHHMM(newTime)) {
                settings.grow.lightOnTime = newTime;
                savePrefString(
                    "webShellyLightOnTime",
                    KEY_LIGHT_ON_TIME,
                    settings.grow.lightOnTime,
                    true,
                    "Shelly Light ON Time"
                );
            }
        }

        if (server.hasArg("webShellyLightDayHours")) {
            settings.grow.lightDayHours = clampInt(
                server.arg("webShellyLightDayHours").toInt(),
                1,
                20
            );

            savePrefInt(
                "webShellyLightDayHours",
                KEY_LIGHT_DAY_HOURS,
                settings.grow.lightDayHours,
                true,
                "Light Day Hours"
            );
        }
    };

    auto saveAuthSettings = [&]() {
        savePrefString(
            "webShellyUsername",
            KEY_SHELLYUSERNAME,
            settings.shelly.username,
            true,
            "User"
        );

        savePrefString(
            "webShellyPassword",
            KEY_SHELLYPASSWORD,
            settings.shelly.password,
            false,
            "Pass"
        );
    };

    // Shelly devices
    saveShellyDevice(
        settings.shelly.main.ip,
        settings.shelly.main.gen,
        "webShellyMainIP",
        KEY_SHELLYMAINIP,
        "Main IP",
        "webShellyMainGen",
        KEY_SHELLYMAINGEN,
        "Main Gen"
    );

    saveShellyDevice(
        settings.shelly.light.ip,
        settings.shelly.light.gen,
        "webShellyLightIP",
        KEY_SHELLYLIGHTIP,
        "Light IP",
        "webShellyLightGen",
        KEY_SHELLYLIGHTGEN,
        "Light Gen"
    );

    saveShellyDevice(
        settings.shelly.humidifier.ip,
        settings.shelly.humidifier.gen,
        "webShellyHumidifierIP",
        KEY_SHELLYHUMIP,
        "Humidifier IP",
        "webShellyHumidifierGen",
        KEY_SHELLYHUMGEN,
        "Humidifier Gen"
    );

    saveShellyDevice(
        settings.shelly.heater.ip,
        settings.shelly.heater.gen,
        "webShellyHeaterIP",
        KEY_SHELLYHEATERIP,
        "Heater IP",
        "webShellyHeaterGen",
        KEY_SHELLYHEATERGEN,
        "Heater Gen"
    );

    saveShellyDevice(
        settings.shelly.fan.ip,
        settings.shelly.fan.gen,
        "webShellyFanIP",
        KEY_SHELLYFANIP,
        "Fan IP",
        "webShellyFanGen",
        KEY_SHELLYFANGEN,
        "Fan Gen"
    );

    saveShellyDevice(
        settings.shelly.exhaust.ip,
        settings.shelly.exhaust.gen,
        "webShellyExhaustIP",
        KEY_SHELLYEXHAUSTIP,
        "Exhaust IP",
        "webShellyExhaustGen",
        KEY_SHELLYEXHAUSTGEN,
        "Exhaust Gen"
    );

    // Other settings
    saveGrowLightSettings();
    saveAuthSettings();

    // Update runtime values immediately
    lightOnTime = settings.grow.lightOnTime;
    lightDayHours = settings.grow.lightDayHours;
    applyGrowLightSchedule();

    // Re-open 3-minute schedule discovery window after Shelly settings change
    g_scheduleResolved = false;
    g_scheduleInitStartMs = millis();
    g_lastScheduleFetchMs = 0;


    // Redirect back to status page
    server.sendHeader("Location", "/");
    server.send(303);
}

// Helper function to save a string preference to a C-style string if the corresponding argument is present
void savePrefStringToCString(
  const char* argName,
  const char* prefKey,
  char*& targetPtr,
  bool logValue = true,
  const char* logLabel = nullptr
) {
  if (!server.hasArg(argName)) return;

  String v = server.arg(argName);
  preferences.putString(prefKey, v);

  // 🔥 alten Speicher freigeben
  if (targetPtr != nullptr) {
    free(targetPtr);
    targetPtr = nullptr;
  }

  targetPtr = strdup(v.c_str());

  if (logLabel == nullptr) logLabel = prefKey;

  if (logValue) {
    logPrint("[PREFERENCES] " + String(logLabel) + " = " + v);
  } else {
    logPrint("[PREFERENCES] " + String(logLabel) + " updated");
  }
}

// Handle general settings save
void handleSaveSettings() {
  // Open the Preferences namespace with write access (readOnly = false)
  // Only call begin() once — calling it twice can cause writes to fail!
  if (!preferences.begin(PREF_NS)) {
    logPrint("[PREFERENCES][ERROR] preferences.begin() failed. "
             "Check that PREF_NS length <= 15 characters.");
    server.send(500, "text/plain", "Failed to open Preferences");
    return;
  }

  savePrefBool("webDebugEnable", KEY_DEBUG_ENABLED, debugLog, true, "Debug Enable");
  savePrefInt("webRelayCount", KEY_RELAYCOUNT, activeRelayCount, true, "Relay Count");
  savePrefString("webBoxName", KEY_NAME, settings.ui.boxName, "Boxname");
  savePrefString("webNTPServer", KEY_NTPSRV, ntpServer);
  savePrefString("webTimeZoneInfo", KEY_TZINFO, tzInfo);
  savePrefString("webLanguage", KEY_LANG, language);
  savePrefString("webTheme", KEY_THEME, theme);
  savePrefString("webTimeFormat", KEY_TFMT, timeFormat);
  savePrefString("webTempUnit", KEY_UNIT, unit);
  savePrefString("webDS18B20Name", KEY_DS18NAME, DS18B20Name);
  savePrefBool("webDS18B20Enable", KEY_DS18B20ENABLE, DS18B20, "DS18B20 Enable");
  savePrefFloat("webPowerPriceKwh", KEY_POWER_PRICE_KWH, powerPriceKwhEur, true, "Power Price €/kWh");
  savePrefString("webDS18B20Name", KEY_DS18NAME, DS18B20Name);
  savePrefString("webRelayName1", KEY_RELAY_1, relayNames[0], "Relay 1 Name");
  savePrefString("webRelayName2", KEY_RELAY_2, relayNames[1], "Relay 2 Name");
  savePrefString("webRelayName3", KEY_RELAY_3, relayNames[2], "Relay 3 Name");
  savePrefString("webRelayName4", KEY_RELAY_4, relayNames[3], "Relay 4 Name");
  if (activeRelayCount == 8) {
    savePrefString("webRelayName5", KEY_RELAY_5, relayNames[4], "Relay 5 Name");
  }

  preferences.end(); // always close Preferences handle

  // 11) Send redirect response and restart the ESP
  server.sendHeader("Location", "/");
  server.send(303);
  requestSafeRestart(1500);
}

void handleSaveMessageSettings() {
  if (!preferences.begin(PREF_NS)) {
    logPrint("[PREFERENCES][ERROR] preferences.begin() failed. "
             "Check that PREF_NS length <= 15 characters.");
    server.send(500, "text/plain", "Failed to open Preferences");
    return;
  }

  if (server.arg("webPushoverEnabled") == "on") {
    pushoverEnabled = "checked";
  } else {
    pushoverEnabled = "";
  }

  pushoverSent = (pushoverEnabled == "checked");
  preferences.putString(KEY_PUSHOVER, pushoverEnabled);

  savePrefString("webPushoverUserKey", KEY_PUSHOVERUSER, pushoverUserKey, "Pushover User Key");
  savePrefString("webPushoverAppKey", KEY_PUSHOVERAPP, pushoverAppKey, "Pushover App Key");
  savePrefString("webPushoverDevice", KEY_PUSHOVERDEVICE, pushoverDevice, "Pushover Device");
  
  if (server.arg("webGotifyEnabled") == "on") {
    gotifyEnabled = "checked";
  } else {
    gotifyEnabled = "";
  }
  logPrint("[PREFERENCES] Gotify " + String(gotifyEnabled));
  preferences.putString(KEY_GOTIFY, gotifyEnabled);

  savePrefString("webGotifyURL", KEY_GOTIFYSERVER, gotifyServer, "Gotify Server URL");
  savePrefString("webGotifyToken", KEY_GOTIFYTOKEN, gotifyToken, "Gotify Token");


  if (pushoverEnabled == "checked") {
    if (language == "de") {
      sendPushover("Testnachricht", "Die Pushover-Benachrichtigungen wurden erfolgreich eingerichtet.");
    } else {
      sendPushover("Test Message", "Pushover notifications have been set up successfully.");
    }
  }

  if (gotifyEnabled == "checked") {
    if (language == "de") {
      sendGotify("Testnachricht", "Die Gotify-Benachrichtigungen wurden erfolgreich eingerichtet.", 5);
    } else {
      sendGotify("Test Message", "Gotify notifications have been set up successfully.", 5);
    }
  }
  
  gotifySent = (gotifyEnabled == "checked");
  preferences.putString(KEY_GOTIFY, gotifyEnabled);

  preferences.end(); // always close Preferences handle

  // 11) Send redirect response and restart the ESP
  server.sendHeader("Location", "/");
  server.send(303);  // HTTP redirect to status page
  delay(250);
  ESP.restart();
}

// Handle form submission save WIFI credentials
void handleSaveWiFi() {
  if (server.hasArg("ssid") && server.hasArg("password")) {
    String ssid = server.arg("ssid");
    String password = server.arg("password");

    preferences.begin(PREF_NS, false);
    preferences.putString("ssid", ssid);
    preferences.putString("password", password);
    preferences.end();

    server.send(200, "text/html", "<h1>Saved! Restarting...</h1>");
    requestSafeRestart(2000);
  } else {
    server.send(400, "text/plain", "Missing data");
  }
}

// key, detailsJson = "{}"
void pushHintKey(const String& key, const String& detailsJson = "{}") {
  hintKey = key;
  hintDetailsJson = detailsJson;
  hintId++;
}

// API: /get hint
void handleHint() {
  // {"id":1,"key":"hint.saved","details":{"percent":12}}
  String json = "{\"id\":" + String(hintId) + ",\"key\":\"";

  // escape quotes/backslashes in key
  String escapedKey = hintKey;
  escapedKey.replace("\\", "\\\\");
  escapedKey.replace("\"", "\\\"");
  json += escapedKey + "\",\"details\":" + hintDetailsJson + "}";

  server.send(200, "application/json", json);
}

// Handle factory reset
void handleFactoryReset() {
  preferences.begin(PREF_NS, false);
  preferences.clear();  // Deletes all keys in the namespace"
  preferences.end();

  server.send(200, "text/html", "<h1>Saved! Restarting...</h1>");
  requestSafeRestart(2000);
}

// initial NTP sync (called at boot)
void initialSyncBlocking() {
  logPrint("[BOOT] Initial NTP sync...");

  // Start Sync
  configTzTime(tzInfo.c_str(), ntpServer.c_str());

  struct tm local;
  unsigned long start = millis();

  // Wait up to 2 seconds for time
  while (millis() - start < 2000) {
    if (getLocalTime(&local, 50)) {
      char buf[64];
      strftime(buf, sizeof(buf), "%d.%m.%Y %H:%M:%S", &local);
      logPrint(String("[BOOT] Time initialized: ") + buf);
      return;
    }
    delay(50); // do not block completely
  }

  logPrint("[BOOT] Failed initial NTP sync");
}

// Daily NTP trigger at 01:00
void dailyNtpTrigger() {
  if (!wifiReady) return;

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo, 50)) return;  // try to get time (non-blocking, max 50ms)

  if (timeinfo.tm_hour == 1 && timeinfo.tm_min == 0 && timeinfo.tm_mday != lastSyncDay) {
    logPrint("Performing daily NTP sync...");
    configTzTime(tzInfo.c_str(), ntpServer.c_str());

    ntpSyncPending = true;
    ntpStartMs = millis();

    // so it really only fires once per day:
    lastSyncDay = timeinfo.tm_mday;
  }
}

// NTP sync tick (non-blocking)
void ntpSyncTick() {
  if (!ntpSyncPending) return;

  struct tm local;
  // try to get time (non-blocking, max 50ms)
  if (getLocalTime(&local, 50)) {
    char readDate[11];
    strftime(readDate, sizeof(readDate), "%Y-%m-%d", &local);

    char buf[64];
    strftime(buf, sizeof(buf), "now: %d.%m.%y  Zeit: %H:%M:%S", &local);
    logPrint(String("[DATETIME] ") + buf);

    ntpSyncPending = false;
    return;
  }

  // timeout after 15 seconds
  if (millis() - ntpStartMs > 15000) {
    logPrint("[DATETIME] NTP sync timeout (non-blocking)");
    ntpSyncPending = false;
  }
}

// calculate elapsed days and weeks from defined date
void calculateTimeSince(const String& startDate, int &days, int &weeks) {
  // Compute "day N / week M" since a given YYYY-MM-DD (local time).
  // NOTE: weeks must be computed as ((days-1)/7)+1 to avoid an off-by-one at day 7, 14, ...
  days = 0;
  weeks = 0;

  int y = 0, m = 0, d = 0;
  if (sscanf(startDate.c_str(), "%d-%d-%d", &y, &m, &d) != 3) {
    return;
  }

  struct tm tmStart {};
  tmStart.tm_mday = d;
  tmStart.tm_mon  = m - 1;
  tmStart.tm_year = y - 1900;
  tmStart.tm_hour = 0;
  tmStart.tm_min  = 0;
  tmStart.tm_sec  = 0;
  tmStart.tm_isdst = -1;

  const time_t startEpoch = mktime(&tmStart);
  const time_t nowEpoch   = time(nullptr);
  if (startEpoch <= 0 || nowEpoch <= 0) return;

  long diffSec = (long)(nowEpoch - startEpoch);
  if (diffSec < 0) diffSec = 0;

  days = (int)(diffSec / 86400L) + 1;
  weeks = (days > 0) ? ((days - 1) / 7 + 1) : 0;
}


// Convert minutes to milliseconds (int return type)
uint32_t minutesToMilliseconds(uint32_t minutes) {
    if (minutes > 60000UL) minutes = 60000UL;
    return minutes * 60UL * 1000UL;
}

// Convert seconds to milliseconds (int return type)
uint32_t secondsToMilliseconds(uint32_t seconds) {
    if (seconds > 86400UL) seconds = 86400UL;
    return seconds * 1000UL;
}

// CSV: ts_ms,tempC,hum,vpd\n
// Append one CSV line: ts_ms,tempC,humPct,vpdKpa
void appendLog(unsigned long timestamp, float temperature, float humidity, float vpd) {
  File f = LittleFS.open(LOG_PATH, FILE_APPEND);
  if (!f) {
    logPrint("[LITTLEFS][ERROR] Failed to open log for append: " + String(LOG_PATH));
    return;
  }

  f.print(String(timestamp));
  f.print(',');
  f.print(String(temperature, 2));
  f.print(',');
  f.print(String(humidity, 2));
  f.print(',');
  f.print(String(vpd, 3));
  f.print('\n');
  f.close();
}

inline float avgValue(float sum, uint32_t count) {
  return (count == 0) ? 0.0f : (sum / count);
}

// Calculate VPD with smoothing for temperature and humidity
float calcVPD(float valLastTemperature, float valOffsetLeafTemperature, float valLastHumidity) {
      static float smoothTemp = NAN;
      static float smoothHum  = NAN;

      const float alpha = 0.10f;

      // Rohwerte
      float FT_raw   = valLastTemperature;
      float FARH_raw = valLastHumidity;

      // clamp RH
      if (FARH_raw < 0.0f)   FARH_raw = 0.0f;
      if (FARH_raw > 100.0f) FARH_raw = 100.0f;

      // init
      if (isnan(smoothTemp)) smoothTemp = FT_raw;
      if (isnan(smoothHum))  smoothHum  = FARH_raw;

      // smoothing
      smoothTemp = alpha * FT_raw   + (1.0f - alpha) * smoothTemp;
      smoothHum  = alpha * FARH_raw + (1.0f - alpha) * smoothHum;

      // ab hier dein original code
      float FT = smoothTemp;
      float FARH = smoothHum;
      float FLTO = valOffsetLeafTemperature;
      float FLT = FT + FLTO;

      float VPLEAF = (610.7f * pow(10.0f, (7.5f * FLT) / (237.3f + FLT)) / 1000.0f);
      float ASVPF  = (610.7f * pow(10.0f, (7.5f * FT)  / (237.3f + FT))  / 1000.0f);
      float VPAIR  = (FARH / 100.0f) * ASVPF;
      float VPD    = VPLEAF - VPAIR;

      if (VPD < 0.0f) VPD = 0.0f;

      return VPD;
}

// Store new reading and update running sums
void addReading(float temp, float hum, float vpd) {
  // Remove old values from sums
  sumTemp -= temps[index_pos];
  sumHum  -= hums[index_pos];
  sumVPD  -= vpds[index_pos];
  sumWaterTemp -= waterTemps[index_pos];

  // Add new values
  temps[index_pos] = temp;
  hums[index_pos]  = hum;
  vpds[index_pos]  = vpd;
  waterTemps[index_pos] = DS18B20STemperature;

  sumTemp += temp;
  sumHum  += hum;
  sumVPD  += vpd;
  sumWaterTemp += waterTemps[index_pos];

  // Move index (circular buffer)
  index_pos = (index_pos + 1) % NUM_VALUES;

  // Count how many values are valid (up to NUM_VALUES)
  if (count < NUM_VALUES) {
    count++;
  }
}

// check HCSR04 sensor
float pingTankLevel(uint8_t trigPin, uint8_t echoPin,
                     uint32_t timeout_us = 30000,   // ~5m max, praktisch weniger
                     uint8_t samples = 3) {         // Mittelwert über N Samples
  // Pins konfigurieren (idempotent, stört nicht wenn öfter aufgerufen)
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  float sum = 0.0f;
  uint8_t ok = 0;

  for (uint8_t i = 0; i < samples; i++) {
    // Trigger-Puls (10 µs HIGH)
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    // Echo-Pulsdauer messen
    unsigned long duration = pulseIn(echoPin, HIGH, timeout_us);
    if (duration == 0) {
      // Timeout -> ungültig
      continue;
    }

    // Schallgeschwindigkeit ~343 m/s => 0.0343 cm/µs
    // Hin- und Rückweg => /2
    float cm = (duration * 0.0343f) / 2.0f;

    // Plausibilitätsfilter (HC-SR04 typ. 2..400cm)
    if (cm >= 2.0f && cm <= 400.0f) {
      sum += cm;
      ok++;
    }

    delay(10); // kleine Pause zwischen Messungen
  }

  if (ok == 0) return -1.0f;
  return sum / ok;
}

void handleApiLogBuffer() {
  server.sendHeader("Cache-Control", "no-store");

  if (!logBufferMutex || xSemaphoreTake(logBufferMutex, pdMS_TO_TICKS(100)) != pdTRUE) {
    server.send(503, "text/plain; charset=utf-8", "log busy");
    return;
  }

  // Heap-friendly streaming: avoid building one large String for every poll.
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/plain; charset=utf-8", "");

  for (const auto& line : logBuffer) {
    server.sendContent(line);
    server.sendContent("\n");
  }

  xSemaphoreGive(logBufferMutex);
  server.sendContent("");
}

void handleClearLog() {
  if (logBufferMutex && xSemaphoreTake(logBufferMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    logBuffer.clear();
    xSemaphoreGive(logBufferMutex);
    server.send(204);
  } else {
    server.send(503, "text/plain; charset=utf-8", "log busy");
  }
}

void handleDownloadLog() {
  if (!logBufferMutex || xSemaphoreTake(logBufferMutex, pdMS_TO_TICKS(100)) != pdTRUE) {
    server.send(503, "text/plain; charset=utf-8", "log busy");
    return;
  }

  // Heap-friendly streaming download: avoid an 8 KB temporary String.
  server.sendHeader("Content-Type", "text/plain; charset=utf-8");
  server.sendHeader("Content-Disposition", "attachment; filename=weblog.txt");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/plain; charset=utf-8", "");

  for (const auto& line : logBuffer) {
    server.sendContent(line);
    server.sendContent("\n");
  }

  xSemaphoreGive(logBufferMutex);
  server.sendContent("");
}

// helper: read actual relay pin and convert to bool  
bool isRelayOn(int idx) {
  if (idx < 0 || idx >= NUM_RELAYS) return false;
  if (idx >= activeRelayCount) return false;
  return digitalRead(relayPins[idx]) == HIGH;
}

// set relay state by index (0-based) with mutex protection and update state array
void setRelay(int idx, bool on) {
  if (idx < 0 || idx >= NUM_RELAYS) return;
  if (idx >= activeRelayCount) return;

  if (relayMutex) {
    if (xSemaphoreTake(relayMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
      digitalWrite(relayPins[idx], on ? HIGH : LOW);
      relayStates[idx] = on;
      xSemaphoreGive(relayMutex);
    } else {
      logPrint("[RELAY] mutex timeout in setRelay()");
    }
  } else {
    digitalWrite(relayPins[idx], on ? HIGH : LOW);
    relayStates[idx] = on;
  }
}

// toggle relay state by index and return new state as JSON
void handleRelayToggleIdx(int idx) {
  bool cur = isRelayOn(idx);
  bool next = !cur;
  setRelay(idx, next);

  String res = "{";
  res += "\"id\":" + String(idx + 1);
  res += ",\"state\":" + String(next ? "true" : "false");
  res += "}";
  server.send(200, "application/json", res);
}

void handleRelayIrrigationIdx(int idx) {
  setRelay(idx, true);
  relayOffTime[idx] = millis() + 10000;
  relayActive[idx] = true;

  String res = "{";
  res += "\"id\":" + String(idx + 1);
  res += ",\"state\":true";
  res += "}";
  server.send(200, "application/json", res);
}

// Send notification via Pushover
bool sendPushover(const String& message, const String& title) {
  if (pushoverSent) {
    WiFiClientSecure client;
    client.setInsecure(); // simpel & schnell (ohne Zertifikatsprüfung)

    HTTPClient https;
    if (!https.begin(client, "https://api.pushover.net/1/messages.json")) {
      return false;
    }

    https.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String body =
      "token=" + String(pushoverAppKey) +
      "&user="  + String(pushoverUserKey) +
      "&device=" + String(pushoverDevice) +
      "&title=" + title +
      "&priority=1" +
      "&message=" + message;

    int code = https.POST(body);
    String resp = https.getString();
    https.end();

    logPrint("[MESSAGE] Pushover HTTP send code: " + String(code));
    logPrint("[MESSAGE] " + resp);

    return (code == 200);
  } else {
    logPrint("[MESSAGE] Pushover notification not sent: disabled");
    return false;
  }
}

// Send notification via Gotify
bool sendGotify(const String& msg, const String& title, int priority) {
  if (gotifySent) {
    String GOTIFY_URL = String("https://") + gotifyServer + "/message?token=" + gotifyToken;
  
    // priority: -2 (lowest) to 10 (highest)
    WiFiClientSecure client;
    client.setInsecure(); // schnell & unkompliziert (ohne Zertifikatsprüfung)

    HTTPClient http;
    if (!http.begin(client, GOTIFY_URL.c_str())) return false;

    http.addHeader("Content-Type", "application/json");

    // JSON Body
    String body = "{";
    body += "\"title\":\"" + title + "\",";
    body += "\"message\":\"" + msg + "\",";
    body += "\"priority\":" + String(priority);
    body += "}";

    int code = http.POST(body);
    String resp = http.getString();
    http.end();

    logPrint("[MESSAGE] Gotify HTTP send code: " + String(code));
    logPrint("[MESSAGE] " + resp);
    return (code >= 200 && code < 300);
  } else {
    logPrint("[MESSAGE] Gotify notification not sent: disabled");
    return false;
  }
  
}

// =======================
// URL HELPERS
// =======================

// Build base URL without credentials (safe for logging)
static String makeBaseUrl(const String& host, uint16_t port) {
  String u = "http://" + host;
  if (port != 80) u += ":" + String(port);
  return u;
}

// =======================
// HASH HELPERS (Digest)
// =======================

// Compute hash (MD5/SHA-256) and return lowercase hex
static String hashHex(mbedtls_md_type_t mdType, const String& s) {
  const mbedtls_md_info_t* info = mbedtls_md_info_from_type(mdType);
  if (!info) return "";

  unsigned char out[64];
  mbedtls_md_context_t ctx;
  mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, info, 0);
  mbedtls_md_starts(&ctx);
  mbedtls_md_update(&ctx, (const unsigned char*)s.c_str(), s.length());
  mbedtls_md_finish(&ctx, out);
  mbedtls_md_free(&ctx);

  const size_t outLen = mbedtls_md_get_size(info);
  static const char* hex = "0123456789abcdef";
  String res;
  res.reserve(outLen * 2);
  for (size_t i = 0; i < outLen; i++) {
    res += hex[(out[i] >> 4) & 0xF];
    res += hex[out[i] & 0xF];
  }
  return res;
}

static String toLowerCopy(String s) { s.toLowerCase(); return s; }

// Extract parameter value from Digest header, supports:
// key="value"  OR  key=value
static String digestParam(const String& header, const String& key) {
  String h = header;
  h.trim();
  String hl = toLowerCopy(h);

  String kl = key;
  kl.trim();
  kl.toLowerCase();

  int p = hl.indexOf(kl + "=");
  if (p < 0) return "";

  p += kl.length() + 1; // skip "key="
  if (p >= h.length()) return "";

  // quoted value
  if (h[p] == '"') {
    p++;
    int e = h.indexOf('"', p);
    if (e < 0) return "";
    return h.substring(p, e);
  }

  // unquoted value until comma
  int e = h.indexOf(',', p);
  if (e < 0) e = h.length();
  String v = h.substring(p, e);
  v.trim();
  return v;
}

static String makeCnonce() {
  // Simple pseudo-random cnonce, good enough for LAN usage
  uint32_t r1 = (uint32_t)esp_random();
  uint32_t r2 = (uint32_t)esp_random();
  char buf[17];
  snprintf(buf, sizeof(buf), "%08lx%08lx", (unsigned long)r1, (unsigned long)r2);
  return String(buf);
}

// Build Digest Authorization header for RFC7616 (SHA-256) and MD5 (fallback)
// Supports "-sess" variants as well.
static String buildDigestAuth(const String& wwwAuth,
                              const String& user,
                              const String& pass,
                              const String& method,
                              const String& uri) {
  String realm  = digestParam(wwwAuth, "realm");
  String nonce  = digestParam(wwwAuth, "nonce");
  String qop    = digestParam(wwwAuth, "qop");
  String opaque = digestParam(wwwAuth, "opaque");
  String alg    = digestParam(wwwAuth, "algorithm"); // e.g. SHA-256, SHA-256-sess, MD5

  if (realm.length() == 0 || nonce.length() == 0) return "";

  bool sess = false;
  String algLower = toLowerCopy(alg);
  if (algLower.endsWith("-sess")) {
    sess = true;
    algLower.replace("-sess", "");
  }

  mbedtls_md_type_t mdType = MBEDTLS_MD_MD5;
  if (algLower == "sha-256") mdType = MBEDTLS_MD_SHA256;
  else if (algLower == "md5" || algLower.length() == 0) mdType = MBEDTLS_MD_MD5;

  // Prefer qop=auth if offered
  String qopUse = (toLowerCopy(qop).indexOf("auth") >= 0) ? "auth" : "";

  String nc = "00000001";
  String cnonce = makeCnonce();

  // HA1 = H(user:realm:pass)
  String ha1 = hashHex(mdType, user + ":" + realm + ":" + pass);
  // HA1-sess = H(HA1:nonce:cnonce)
  if (sess) {
    ha1 = hashHex(mdType, ha1 + ":" + nonce + ":" + cnonce);
  }

  // HA2 = H(method:uri)
  String ha2 = hashHex(mdType, method + ":" + uri);

  // response
  String resp;
  if (qopUse.length()) {
    resp = hashHex(mdType, ha1 + ":" + nonce + ":" + nc + ":" + cnonce + ":" + qopUse + ":" + ha2);
  } else {
    resp = hashHex(mdType, ha1 + ":" + nonce + ":" + ha2);
  }

  // Build header line
  String h = "Authorization: Digest ";
  h += "username=\"" + user + "\", ";
  h += "realm=\"" + realm + "\", ";
  h += "nonce=\"" + nonce + "\", ";
  h += "uri=\"" + uri + "\", ";
  if (alg.length()) h += "algorithm=" + alg + ", ";

  if (qopUse.length()) {
    h += "response=\"" + resp + "\", ";
    h += "qop=" + qopUse + ", ";
    h += "nc=" + nc + ", ";
    h += "cnonce=\"" + cnonce + "\", ";
  } else {
    h += "response=\"" + resp + "\", ";
  }

  if (opaque.length()) h += "opaque=\"" + opaque + "\", ";

  if (h.endsWith(", ")) h.remove(h.length() - 2);
  h += "\r\n";
  return h;
}

// =======================
// HTTP HELPERS
// =======================

// Fetch WWW-Authenticate header using HTTPClient (no auth).
// This is used to discover whether the device requires Digest or Basic.
static bool fetchWwwAuthenticate(const String& url, String& outWwwAuth, int& outCode) {
  outWwwAuth = "";
  outCode = -1;

  HTTPClient http;
  http.setTimeout(1200);
  http.useHTTP10(true);

  const char* keys[] = {"WWW-Authenticate"};
  http.collectHeaders(keys, 1);

  if (!http.begin(url)) return false;

  int code = http.GET();
  outCode = code;

  if (code == 401) {
    outWwwAuth = http.header("WWW-Authenticate");
  }

  http.end();
  return true;
}

// Read the complete HTTP response (status line + headers + body)
static bool readAllFromClient(WiFiClient& client, String& outRaw) {
  outRaw = "";
  unsigned long start = millis();

  // Wait for first bytes (up to 2s)
  while (!client.available() && client.connected() && (millis() - start) < 2000) {
    delay(5);
  }

  // Read until connection closes or timeout (4s max)
  start = millis();
  while ((client.connected() || client.available()) && (millis() - start) < 4000) {
    while (client.available()) {
      outRaw += client.readString();
      start = millis(); // reset timeout while data arrives
    }
    delay(5);
  }

  return outRaw.length() > 0;
}

// Parse HTTP status code and body from a raw response string
static bool parseHttpResponse(const String& raw, int& outCode, String& outBody) {
  outCode = -1;
  outBody = "";

  int sep = raw.indexOf("\r\n\r\n");
  if (sep < 0) sep = raw.indexOf("\n\n");
  if (sep < 0) {
    // No headers found, treat everything as body
    outBody = raw;
    return true;
  }

  String head = raw.substring(0, sep);
  outBody = raw.substring(sep + ((raw[sep] == '\r') ? 4 : 2));

  // Status line is first header line
  int lineEnd = head.indexOf("\r\n");
  if (lineEnd < 0) lineEnd = head.indexOf('\n');
  String statusLine = (lineEnd >= 0) ? head.substring(0, lineEnd) : head;
  statusLine.trim();

  int sp1 = statusLine.indexOf(' ');
  int sp2 = statusLine.indexOf(' ', sp1 + 1);
  if (sp1 > 0 && sp2 > sp1) {
    outCode = statusLine.substring(sp1 + 1, sp2).toInt();
  }

  return true;
}

// Low-level GET using WiFiClient, with optional Authorization header line.
static bool rawHttpGet(const String& host, uint16_t port, const String& path,
                       const String& authHeaderLine,
                       int& outCode, String& outBody) {
  outCode = -1;
  outBody = "";

  WiFiClient client;
  client.setTimeout(4); // seconds

  if (!client.connect(host.c_str(), port)) {
    return false;
  }

  String p = path;
  if (!p.startsWith("/")) p = "/" + p;

  // Use HTTP/1.0 to avoid chunked transfer encoding
  String req =
    "GET " + p + " HTTP/1.0\r\n"
    "Host: " + host + "\r\n" +
    authHeaderLine +
    "Connection: close\r\n\r\n";

  client.print(req);

  String raw;
  bool ok = readAllFromClient(client, raw);
  client.stop();
  if (!ok) return false;

  parseHttpResponse(raw, outCode, outBody);
  return true;
}



// Low-level HTTP request using WiFiClient, with optional Authorization header line.
static bool rawHttpRequest(const String& method,
                           const String& host, uint16_t port,
                           const String& path,
                           const String& authHeaderLine,
                           const String& contentType,
                           const String& body,
                           int& outCode, String& outBody) {
  outCode = -1;
  outBody = "";

  WiFiClient client;
  client.setTimeout(6); // seconds

  if (!client.connect(host.c_str(), port)) {
    return false;
  }

  String p = path;
  if (!p.startsWith("/")) p = "/" + p;

  String req = method + " " + p + " HTTP/1.0\r\n"
               "Host: " + host + "\r\n" +
               authHeaderLine;

  if (method == "POST" || method == "PUT" || method == "PATCH") {
    const String ct = contentType.length() ? contentType : String("application/json");
    req += "Content-Type: " + ct + "\r\n";
    req += "Content-Length: " + String(body.length()) + "\r\n";
  }

  req += "Connection: close\r\n\r\n";

  client.print(req);
  if (method == "POST" || method == "PUT" || method == "PATCH") {
    client.print(body);
  }

  String raw;
  bool ok = readAllFromClient(client, raw);
  client.stop();
  if (!ok) return false;

  parseHttpResponse(raw, outCode, outBody);
  return true;
}

// Auto-auth request (GET/POST): first fetch WWW-Authenticate, then perform Digest/BASIC accordingly
static bool parseHttpUrl(const String& url, String& host, uint16_t& port, String& path) {
  String u = url;
  u.trim();
  if (u.startsWith("http://")) {
    u.remove(0, 7);
    port = 80;
  } else if (u.startsWith("https://")) {
    // NOTE: raw WiFiClient (no TLS) can't handle https. Use http on Shelly LAN.
    u.remove(0, 8);
    port = 443;
  } else {
    port = 80;
  }

  int slash = u.indexOf('/');
  String hostPort = (slash >= 0) ? u.substring(0, slash) : u;
  path = (slash >= 0) ? u.substring(slash) : "/";

  int colon = hostPort.indexOf(':');
  if (colon >= 0) {
    host = hostPort.substring(0, colon);
    port = (uint16_t) hostPort.substring(colon + 1).toInt();
  } else {
    host = hostPort;
  }

  host.trim();
  if (host.length() == 0) return false;
  if (path.length() == 0) path = "/";
  return true;
}

static String buildBasicAuthHeaderLine(const String& user, const String& pass) {
  // mbedTLS base64 encoder (avoid dependency on non-standard Arduino base64 libs)
  const String token = user + ":" + pass;

  // Base64 length: 4 * ceil(n/3) + 1 for NUL
  const size_t inLen  = (size_t)token.length();
  const size_t outCap = 4 * ((inLen + 2) / 3) + 1;
  unsigned char* out = (unsigned char*)malloc(outCap);
  if (!out) return "";

  size_t outLen = 0;
  const int rc = mbedtls_base64_encode(out, outCap, &outLen,
                                      (const unsigned char*)token.c_str(), inLen);
  if (rc != 0) {
    free(out);
    return "";
  }
  out[outLen] = 0;

  String line = "Authorization: Basic " + String((const char*)out) + "\r\n";
  free(out);
  return line;
}

static bool httpRequestWithDigestAutoAuth(const String& method,
                                          const String& host, uint16_t port,
                                          const String& path,
                                          const String& user, const String& pass,
                                          const String& contentType,
                                          const String& body,
                                          int& outCode,
                                          String& outBody,
                                          uint32_t timeoutMs = 4000) {
  // 1) Try without auth first
  if (rawHttpRequest(method, host, port, path, "", contentType, body, outCode, outBody)) {
    if (outCode != 401) return (outCode >= 200 && outCode < 300);
  }

  // 2) Ask for WWW-Authenticate via lightweight GET
  String www;
  const String url = String("http://") + host + ":" + String(port) + path;
  if (!fetchWwwAuthenticate(url, www, outCode)) return false;

  if (outCode != 401 || www.length() == 0) {
    // Not an auth challenge (or server didn't include header)
    return (outCode >= 200 && outCode < 300);
  }

  // 3) Build auth header line
  String authLine;
  if (www.startsWith("Digest")) {
    authLine = buildDigestAuth(www, user, pass, method, path); // already includes \r\n
  } else if (www.startsWith("Basic")) {
    authLine = buildBasicAuthHeaderLine(user, pass);
  } else {
    Serial.println("[SHELLY][AUTH] Unsupported auth scheme: " + www);
    return false;
  }

  // 4) Retry with auth header
  return rawHttpRequest(method, host, port, path, authLine, contentType, body, outCode, outBody);
}

// Convenience wrapper used by schedule code (URL-based POST)
static bool httpPostWithDigestAutoAuth(const String& host, uint16_t port,
                                      const String& path,
                                      const String& payload,
                                      const String& user,
                                      const String& pass,
                                      int& outCode,
                                      String& outBody,
                                      uint32_t timeoutMs = 4000) {
  return httpRequestWithDigestAutoAuth("POST", host, port, path, user, pass,
                                      "application/json", payload,
                                      outCode, outBody, timeoutMs);
}

static bool httpPostWithDigestAutoAuth(const String& url,
                                      const String& payload,
                                      const String& contentType,
                                      String& outResp,
                                      uint32_t timeoutMs,
                                      const String& user,
                                      const String& pass) {
  String host, path;
  uint16_t port = 80;
  if (!parseHttpUrl(url, host, port, path)) return false;

  int code = 0;
  String body;
  const bool ok = httpRequestWithDigestAutoAuth("POST", host, port, path, user, pass,
                                               contentType, payload, code, body, timeoutMs);
  outResp = body;
  return ok;
}

// Auto-auth GET: first fetch WWW-Authenticate, then perform Digest/BASIC accordingly
static bool httpGetWithDigestAutoAuth(const String& host, uint16_t port, const String& path,
                                      const String& user, const String& pass,
                                      int& outCode, String& outBody) {
  outCode = -1;
  outBody = "";

  // Discover auth scheme (no credentials sent here)
  String url = makeBaseUrl(host, port) + path;
  String www;
  int firstCode = -1;

  if (!fetchWwwAuthenticate(url, www, firstCode)) {
    return false;
  }

  // If endpoint is open (200), just request without auth
  if (firstCode == 200) {
    return rawHttpGet(host, port, path, "", outCode, outBody);
  }

  // If unauthorized, decide scheme
  if (firstCode != 401) {
    // Unexpected (404, 500, etc.). Still try without auth to get body.
    return rawHttpGet(host, port, path, "", outCode, outBody);
  }

  String wwwLower = toLowerCopy(www);

  if (wwwLower.startsWith("digest")) {
    // Build Digest Authorization header
    String digestLine = buildDigestAuth(www, user, pass, "GET", path);
    if (digestLine.length() == 0) return false;
    return rawHttpGet(host, port, path, digestLine, outCode, outBody);
  }

  // Basic auth (rare with Shelly Gen2/3 but supported sometimes)
  if (wwwLower.startsWith("basic")) {
    // We'll avoid putting credentials in logs; we only build the header
    // NOTE: If you need Basic, you can implement base64 header here.
    // Many Shelly Gen2/3 use Digest, so this branch is typically not needed.
    return false;
  }

  return false;
}

// =======================
// SIMPLE IP VALIDATION
// =======================
static bool hasValidIPv4(const String& ip) {
  IPAddress tmp;
  return ip.length() > 0 && tmp.fromString(ip);
}

// =======================
// SIMPLE HTTP GET FOR GEN1 DEVICES (BASIC AUTH)
// =======================
static bool httpGetGen1(
  const String& host, int port, const String& path,
  const String& user, const String& pass,
  int& code, String& body
) {
  WiFiClient client;
  HTTPClient http;

  http.setTimeout(2000); // Gen1 braucht manchmal etwas länger
  http.setReuse(false);

  String url = "http://" + host + ":" + String(port) + path;

  if (!http.begin(client, url)) {
    code = -1;
    return false;
  }

  // Nur wenn user gesetzt ist (sonst schickst du leere Auth-Header)
  if (user.length() > 0) {
    http.setAuthorization(user.c_str(), pass.c_str());  // Basic Auth
  }

  code = http.GET();
  if (code > 0) body = http.getString();

  http.end();
  return (code >= 200 && code < 300);
}

// =======================
// MAIN: GET VALUES
// =======================
ShellyValues getShellyValues(ShellyDevice& dev, int switchId, int port) {
  ShellyValues v; // default ok=false

  // Validate IP
  if (!hasValidIPv4(dev.ip)) {
    logPrint("[SHELLY] Invalid IP: '" + dev.ip + "'");
    return v;
  }

  String path = (dev.gen == 1)
    ? "/status"
    : ("/rpc/Switch.GetStatus?id=" + String(switchId));

  int code = 0;
  String body;
  bool ok = false;

  if (dev.gen == 1) {
    ok = httpGetGen1(dev.ip, port, path,
                     settings.shelly.username, settings.shelly.password,
                     code, body);
  } else {
    ok = httpGetWithDigestAutoAuth(dev.ip, port, path,
                                   settings.shelly.username, settings.shelly.password,
                                   code, body);
  }

  if (!ok) {
    logPrint("[SHELLY] request failed gen=" + String(dev.gen) +
             " HTTP=" + String(code) + " " + dev.ip + ":" + String(port) + path);
    if (body.length()) logPrint("[SHELLY] body(first200): " + body.substring(0, 200));
    return v;
  }

  // Parse JSON (Gen1 /status can be bigger)
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, body);
  if (err) {
    logPrint("[SHELLY] JSON parse error: " + String(err.c_str()));
    return v;
  }

  if (dev.gen == 1) {
    // Gen1: relays[x].ison, meters[x].power, meters[x].total
    v.isOn     = doc["relays"][switchId]["ison"] | false;
    v.powerW   = doc["meters"][switchId]["power"] | NAN;
    v.energyWh = doc["meters"][switchId]["total"] | NAN;
  } else {
    // Gen2+: Switch.GetStatus
    v.isOn     = doc["output"] | false;
    v.powerW   = doc["apower"] | NAN;
    v.energyWh = doc["aenergy"]["total"] | NAN;
  }

  v.ok = true;
  dev.values = v;

  // ------------------------------------------------------------
  // EXTRA: Read exactly one ON and one OFF schedule (Gen2+ only).
  // Assumption: One ON time + one OFF time applies to every day.
  // Fills dev.schedules.days[0..6] with identical times.
  // Unset values are -1.
  // ------------------------------------------------------------
  const bool isLightShelly =
  settings.shelly.light.ip.length() > 0 && dev.ip == settings.shelly.light.ip;

  // Startup-only Schedule.List strategy:
  // - during first 3 minutes after boot: try every 15s
  // - stop forever once at least ON or OFF time is found
  // - stop forever after 3 minutes even if nothing found
  if (g_scheduleInitStartMs == 0) g_scheduleInitStartMs = millis();

  const uint32_t scheduleFetchIntervalMs = 15000UL; // 15s during startup window
  const uint32_t scheduleInitWindowMs = 180000UL; // 3 minutes
  const bool inInitWindow = (uint32_t)(millis() - g_scheduleInitStartMs) < scheduleInitWindowMs;

  const bool scheduleDue =
    !g_scheduleResolved &&
    inInitWindow &&
    ((g_lastScheduleFetchMs == 0) || ((uint32_t)(millis() - g_lastScheduleFetchMs) >= scheduleFetchIntervalMs));

  if (dev.gen >= 2 && isLightShelly && scheduleDue) {
    g_lastScheduleFetchMs = millis();


    // Initialize schedules to "unset"
    for (int i = 0; i < 7; i++) {
      dev.schedules.days[i].onHour = -1;
      dev.schedules.days[i].onMinute = -1;
      dev.schedules.days[i].offHour = -1;
      dev.schedules.days[i].offMinute = -1;
    }

    int scode = 0;
    String sbody;

    // Use existing digest auth helper and existing credentials
    bool sok = httpGetWithDigestAutoAuth(dev.ip, port, "/rpc/Schedule.List",
                                         settings.shelly.username, settings.shelly.password,
                                         scode, sbody);

    if (!sok) {
      logPrint("[SHELLY] Schedule.List failed gen=" + String(dev.gen) +
               " HTTP=" + String(scode) + " " + dev.ip + ":" + String(port));
      if (sbody.length()) logPrint("[SHELLY] Schedule body(first200): " + sbody.substring(0, 200));
      return v; // device values are fine; schedules stay unset
    }

    JsonDocument sdoc;
    DeserializationError serr = deserializeJson(sdoc, sbody);
    if (serr) {
      logPrint("[SHELLY] Schedule JSON parse error: " + String(serr.c_str()));
      return v; // device values are fine; schedules stay unset
    }

    bool haveOn = false;
    bool haveOff = false;

    int onHour = -1, onMinute = -1;
    int offHour = -1, offMinute = -1;

    JsonArray jobs = sdoc["jobs"].as<JsonArray>();
    for (JsonObject job : jobs) {
      if (!(job["enable"] | false)) continue;

      String timespec = job["timespec"] | "";
      if (timespec.length() == 0) continue;

      // Expected cron-like format: "SEC MIN HOUR * * ..."
      // We only extract MIN and HOUR.
      int sp1 = timespec.indexOf(' ');
      if (sp1 < 0) continue;
      int sp2 = timespec.indexOf(' ', sp1 + 1);
      if (sp2 < 0) continue;
      int sp3 = timespec.indexOf(' ', sp2 + 1);
      if (sp3 < 0) continue;

      int minute = timespec.substring(sp1 + 1, sp2).toInt();
      int hour   = timespec.substring(sp2 + 1, sp3).toInt();

      // Check calls: we only care about switch.set for this switchId
      JsonArray calls = job["calls"].as<JsonArray>();
      for (JsonObject c : calls) {
        String method = c["method"] | "";
        String methodLower = method;
        methodLower.toLowerCase();

        // Shelly returns "switch.set" (lowercase) in Schedule.List
        if (methodLower != "switch.set") continue;

        int cid = c["params"]["id"] | -1;
        if (cid != switchId) continue;

        bool on = c["params"]["on"] | false;

        // First ON and first OFF win
        if (on && !haveOn) {
          haveOn = true;
          onHour = hour;
          onMinute = minute;
        } else if (!on && !haveOff) {
          haveOff = true;
          offHour = hour;
          offMinute = minute;
        }
      }

      // Stop early once both are found
      if (haveOn && haveOff) break;
    }

    // Log if schedules are missing
    if (!haveOn && !haveOff) {
      logPrint("[SHELLY] Schedule: no ON/OFF time found for switchId=" + String(switchId) +
               " on " + dev.ip + ":" + String(port));
    } else if (!haveOn) {
      logPrint("[SHELLY] Schedule: missing ON time for switchId=" + String(switchId) +
               " on " + dev.ip + ":" + String(port));
    } else if (!haveOff) {
      logPrint("[SHELLY] Schedule: missing OFF time for switchId=" + String(switchId) +
               " on " + dev.ip + ":" + String(port));
    }

    // Apply the same schedule to all days (and log what was applied)
    if (haveOn || haveOff) {
      g_scheduleResolved = true; // stop trying in future
      for (int i = 0; i < 7; i++) {
        if (haveOn) {
          dev.schedules.days[i].onHour = onHour;
          dev.schedules.days[i].onMinute = onMinute;
        }
        if (haveOff) {
          dev.schedules.days[i].offHour = offHour;
          dev.schedules.days[i].offMinute = offMinute;
        }
      }

      // Build time strings with leading zeros for minutes
      String onStr  = haveOn
        ? (String(onHour)  + ":" + (onMinute  < 10 ? "0" : "") + String(onMinute))
        : "unset";

      String offStr = haveOff
        ? (String(offHour) + ":" + (offMinute < 10 ? "0" : "") + String(offMinute))
        : "unset";

      logPrint("[SHELLY] Schedule applied for switchId=" + String(switchId) +
               " on " + dev.ip + ":" + String(port) +
               " ON=" + onStr + " OFF=" + offStr + " (all days)");
    }
  }

  return v;
}

// =======================
// Reset Shelly energy counters (best-effort).
// Gen2: Switch.ResetCounters / PM1.ResetCounters
// Gen1: there is no universal reset for Plug/1PM; we keep a local offset instead.
// =======================
static bool shellyResetEnergyCounters(ShellyDevice &dev, uint8_t switchId = 0, uint16_t port = 80) {
  if (WiFi.status() != WL_CONNECTED) return false;

  int code = 0;
  String body;

  auto callGen2 = [&](const String& path) -> bool {
    bool ok = httpGetWithDigestAutoAuth(dev.ip, port, path,
                                       settings.shelly.username, settings.shelly.password,
                                       code, body);
    return ok && code == 200;
  };

  auto callGen1 = [&](const String& path) -> bool {
    bool ok = httpGetGen1(dev.ip, port, path,
                         settings.shelly.username, settings.shelly.password,
                         code, body);
    return ok && code == 200;
  };

  if (dev.gen >= 2) {
    // Try Switch.ResetCounters first (works on many Plus/Pro switch devices)
    if (callGen2("/rpc/Switch.ResetCounters?id=" + String(switchId))) return true;
    // Some devices expose metering via PM1 component
    if (callGen2("/rpc/PM1.ResetCounters?id=" + String(switchId))) return true;
    return false;
  }

  // Gen1 best-effort for EM/3EM style devices (if present)
  // (Plug/1PM typically has no supported reset endpoint; offset will handle display reset)
  if (callGen1("/reset_data")) return true;
  return false;
}

// =======================
// Uses the same Digest auto-auth mechanism.
// =======================
static bool shellySwitchSet(const String& host, uint8_t gen, bool on, uint8_t switchId = 0, uint16_t port = 80) {
  if (WiFi.status() != WL_CONNECTED) return false;

  String path;
  if (gen == 1) {
    path = "/relay/" + String(switchId) + "?turn=" + String(on ? "on" : "off");
  } else {
    path = "/rpc/Switch.Set?id=" + String(switchId) + "&on=" + String(on ? "true" : "false");
  }

  logPrint("[SHELLY] SET " + host + ":" + String(port) + " " + path);

  int code = -1;
  String body;

  bool ok = httpGetWithDigestAutoAuth(host, port, path,
                                     settings.shelly.username,
                                     settings.shelly.password,
                                     code, body);
  logPrint("[SHELLY] HTTP=" + String(code) + " bodyLen=" + String(body.length()));
  return ok && (code == 200);
}

static bool shellySwitchOn(const String& host, uint8_t gen, uint8_t switchId = 0, uint16_t port = 80) {
  return shellySwitchSet(host, gen, true, switchId, port);
}

static bool shellySwitchOff(const String& host, uint8_t gen, uint8_t switchId = 0, uint16_t port = 80) {
  return shellySwitchSet(host, gen, false, switchId, port);
}

// --- GrowLight schedule support -------------------------------------------------
// Encodes query string parts (for Gen1 schedule_rules)
static String _urlEncodeQS(const String& s){
  String o; o.reserve(s.length()*3);
  const char* hex = "0123456789ABCDEF";
  for (size_t i=0; i<s.length(); i++) {
    const uint8_t c = (uint8_t)s[i];
    const bool ok = (c>='a' && c<='z') || (c>='A' && c<='Z') || (c>='0' && c<='9') || c=='-' || c=='_' || c=='.' || c=='~';
    if (ok) {
      o += (char)c;
    } else {
      o += '%';
      o += hex[(c >> 4) & 0xF];
      o += hex[c & 0xF];
    }
  }
  return o;
}

static bool _parseHHMM(const String& hhmm, int& h, int& m){
  if (hhmm.length() < 4) return false;
  int colon = hhmm.indexOf(':');
  if (colon > 0) {
    h = hhmm.substring(0, colon).toInt();
    m = hhmm.substring(colon + 1).toInt();
  } else {
    h = hhmm.substring(0, 2).toInt();
    m = hhmm.substring(2, 4).toInt();
  }
  if (h < 0 || h > 23 || m < 0 || m > 59) return false;
  return true;
}

static String _fmtHHMM(int h, int m){
  char b[6];
  snprintf(b, sizeof(b), "%02d:%02d", h, m);
  return String(b);
}

static String _fmtRuleTime4(int h, int m){
  char b[5];
  snprintf(b, sizeof(b), "%02d%02d", h, m);
  return String(b);
}

// Applies a daily ON/OFF schedule directly to the Shelly (so it can run autonomously).
// Gen1: /settings/relay/0?schedule=true&schedule_rules=....
// Gen2+: /rpc/Schedule.DeleteAll + /rpc/Schedule.Create (2 jobs)
static bool applyShellyLightSchedule(const String& onTimeHHMM, int dayHours){
  const String ip = settings.shelly.light.ip;
  if (!ip.length()) {
    Serial.println("[SHELLY][LIGHT][SCHEDULE] No IP configured");
    return false;
  }

  int h = 0, mi = 0;
  if (!_parseHHMM(onTimeHHMM, h, mi)) {
    Serial.println("[SHELLY][LIGHT][SCHEDULE] Invalid onTime");
    return false;
  }

  int dh = dayHours;
  if (dh < 1) dh = 1;
  if (dh > 20) dh = 20;

  const int offH = (h + dh) % 24;
  const int offM = mi;
  const String offTime = _fmtHHMM(offH, offM);

  const uint8_t gen = settings.shelly.light.gen ? settings.shelly.light.gen : 1;
  const String user = settings.shelly.username;
  const String pass = settings.shelly.password;

  int status = 0;
  String body;

  if (gen <= 1) {
    const String on4  = _fmtRuleTime4(h, mi);
    const String off4 = _fmtRuleTime4(offH, offM);
    const String rules = on4 + "-0123456-on," + off4 + "-0123456-off";
    const String path = String("/settings/relay/0?schedule=true&schedule_rules=") + _urlEncodeQS(rules);

    const bool ok = httpGetWithDigestAutoAuth(ip, 80, path, user, pass, status, body);
    Serial.printf("[SHELLY][LIGHT][SCHEDULE] Gen1 set rules=%s status=%d ok=%d\n", rules.c_str(), status, (int)ok);
    return ok && (status >= 200 && status < 300);
  }

  // Gen2+: clear + create two schedule jobs
  const bool okDel = httpPostWithDigestAutoAuth(ip, 80, "/rpc/Schedule.DeleteAll", "{}", user, pass, status, body);
  Serial.printf("[SHELLY][LIGHT][SCHEDULE] Gen2 DeleteAll status=%d ok=%d\n", status, (int)okDel);

  auto mkCreate = [&](bool turnOn, int hh, int mm) -> String {
    // Shelly Gen2 timespec format: "SEC MIN HOUR * * DOW"
    // Example: "0 0 3 * * SUN,MON,TUE,WED,THU,FRI,SAT"
    char ts[64];
    snprintf(ts, sizeof(ts), "0 %d %d * * SUN,MON,TUE,WED,THU,FRI,SAT", mm, hh);

    String payload = String("{\"enable\":true,\"timespec\":\"") + ts +"\",\"calls\":[{\"method\":\"Switch.Set\",\"params\":{\"id\":0,\"on\":" + (turnOn ? "true" : "false") + "}}]}";
    return payload;
  };

  const String createOn = mkCreate(true, h, mi);
  const String createOff = mkCreate(false, offH, offM);

  const bool ok1 = httpPostWithDigestAutoAuth(ip, 80, "/rpc/Schedule.Create", createOn, user, pass, status, body);
  Serial.printf("[SHELLY][LIGHT][SCHEDULE] Gen2 Create ON %s status=%d ok=%d\n", onTimeHHMM.c_str(), status, (int)ok1);

  const bool ok2 = httpPostWithDigestAutoAuth(ip, 80, "/rpc/Schedule.Create", createOff, user, pass, status, body);
  Serial.printf("[SHELLY][LIGHT][SCHEDULE] Gen2 Create OFF %s status=%d ok=%d\n", offTime.c_str(), status, (int)ok2);

  return okDel && ok1 && ok2;
}

// Apply the configured growlight schedule to the Shelly "light" device.
// Uses:
//   - settings.grow.lightOnTime  ("HH:MM")
//   - settings.grow.lightDayHours (1..20)
// Derives OFF time = ON + dayHours (wrap around midnight).
static bool applyGrowLightSchedule() {
  // must have device configured
  if (settings.shelly.light.ip.length() == 0) {
    Serial.println("[SHELLY][LIGHT][SCHEDULE] No IP configured -> skip");
    return false;
  }

  // basic validation of on-time
  String on = settings.grow.lightOnTime;
  on.trim();
  int h = -1, m = -1;
  if (on.length() >= 4) {
    const int colon = on.indexOf(':');
    if (colon > 0) {
      h = on.substring(0, colon).toInt();
      m = on.substring(colon + 1).toInt();
    }
  }
  if (h < 0 || h > 23 || m < 0 || m > 59) {
    Serial.printf("[SHELLY][LIGHT][SCHEDULE] Invalid onTime '%s'\n", on.c_str());
    return false;
  }

  int dayHours = (int)settings.grow.lightDayHours;
  if (dayHours < 1) dayHours = 1;
  if (dayHours > 20) dayHours = 20;

  const int totalOnMin  = h * 60 + m;
  const int totalOffMin = (totalOnMin + dayHours * 60) % (24 * 60);
  const int offH = totalOffMin / 60;
  const int offM = totalOffMin % 60;
  char offBuf[6];
  snprintf(offBuf, sizeof(offBuf), "%02d:%02d", offH, offM);
  String off = String(offBuf);

  Serial.printf("[SHELLY][LIGHT][SCHEDULE] Apply %s -> %s (%dh)\n", on.c_str(), off.c_str(), dayHours);
  return applyShellyLightSchedule(on, dayHours);
}

static void controlHeaterByTemperature() {
    // 0 = disabled, 1 = ESP relay, 2 = Shelly heater
    if (settings.heating.sourceType == 0) return;

    // Abort if there is no valid temperature reading
    if (isnan(cur.temperatureC)) return;

    // Control tuning
    const float HYST = 0.25f;
    const float PREDICT_SEC = 180.0f;
    const uint32_t MIN_ON_MS = 120000UL;
    const uint32_t MIN_OFF_MS = 120000UL;

    // Persistent controller state
    static float lastTemp = NAN;
    static uint32_t lastMs = 0;
    static uint32_t lastSwitchMs = 0;
    static bool bootHandled = false;

    const uint32_t now = millis();

    // Determine current heater state depending on selected source
    bool isOn = false;

    if (settings.heating.sourceType == 1) {
        const int heatRelay = settings.heating.Relay;
        if (heatRelay < 1 || heatRelay > activeRelayCount) return;

        const int idx = heatRelay - 1;
        isOn = (digitalRead(relayPins[idx]) == HIGH);
    } else if (settings.heating.sourceType == 2) {
        if (!shelly.heater.values.ok) return; // no reliable state yet
        isOn = shelly.heater.values.isOn;
    } else {
        return;
    }

    // Estimate slope (°C/s)
    float slope = 0.0f;

    if (!isnan(lastTemp) && lastMs > 0 && now > lastMs) {
        float dt = (now - lastMs) / 1000.0f;
        if (dt > 0.1f) {
            slope = (cur.temperatureC - lastTemp) / dt;
        }
    }

    // Predict short-term temp
    float predicted = cur.temperatureC + slope * PREDICT_SEC;

    // Thresholds
    const float tOn = settings.grow.targetTemperature - HYST;
    const float tOff = settings.grow.targetTemperature + HYST;

    // Minimum on/off time protection
    bool canSwitch = true;

    if (bootHandled) {
        uint32_t elapsed = now - lastSwitchMs;
        if (isOn && elapsed < MIN_ON_MS) canSwitch = false;
        if (!isOn && elapsed < MIN_OFF_MS) canSwitch = false;
    }

    bool wantOn = isOn;

    if (canSwitch) {
        if (!isOn && cur.temperatureC <= tOn) {
            wantOn = true;
        } else if (
            isOn &&
            (cur.temperatureC >= tOff || predicted >= settings.grow.targetTemperature)
        ) {
            wantOn = false;
        }
    }

    // Apply only if state must change
    if (wantOn != isOn) {
        bool switched = false;

        if (settings.heating.sourceType == 1) {
            const int idx = settings.heating.Relay - 1;
            digitalWrite(relayPins[idx], wantOn ? HIGH : LOW);
            relayStates[idx] = wantOn;
            switched = true;
        } else if (settings.heating.sourceType == 2) {
            if (settings.shelly.heater.ip.length() > 0) {
                switched = shellySwitchSet(
                    settings.shelly.heater.ip,
                    settings.shelly.heater.gen,
                    wantOn,
                    0,
                    80
                );

                if (switched) {
                    shelly.heater.values.isOn = wantOn;
                    shelly.heater.values.ok = true;
                }
            }
        }

        if (switched) {
            lastSwitchMs = now;
            logPrint(String("[HEAT] ") + (wantOn ? "ON" : "OFF"));
        }
    }

    // Update slope history
    bootHandled = true;
    lastTemp = cur.temperatureC;
    lastMs = now;
}

void controlMinVPD() {
  // --- static runtime state to avoid request spam ---
  static bool exhaustStateKnown = false;
  static bool exhaustStateCache = false; // last known ON/OFF
  static uint32_t lastSwitchMs = 0;
  const uint32_t minSwitchIntervalMs = 10000; // 10s Sperre zwischen Schaltvorgängen

  if (!settings.grow.minVpdMonEnabled) return;

  String ip = String(settings.shelly.exhaust.ip);
  ip.trim();
  if (ip.length() < 7 || ip.length() > 15) return;

  float currentVpd = cur.vpdKpa;
  float targetVpd = settings.grow.targetVPD;
  float minOffset = settings.grow.minVPD; // target - offset
  float hysteresis = settings.grow.vpdHysteresis;

  if (!isfinite(currentVpd) || !isfinite(targetVpd)) return;
  if (!isfinite(minOffset) || minOffset < 0.0f) minOffset = 0.0f;
  if (minOffset > 0.30f) minOffset = 0.30f;
  if (!isfinite(hysteresis) || hysteresis < 0.0f) hysteresis = 0.0f;

  float minEff = targetVpd - minOffset;
  if (minEff < 0.1f) minEff = 0.1f;

  // inital request to fill cache (and check connectivity) 
    if (!exhaustStateKnown) {
    exhaustStateCache = getShellyValues(shelly.exhaust, 0, 80).isOn;
    exhaustStateKnown = true;
  }

  // Backend-gleiche Hysterese-Logik
  bool shouldBeOn = exhaustStateCache
  ? (currentVpd < (minEff + hysteresis))
  : (currentVpd < minEff);

  // Apply only if state must change (and avoid request spam with timing check)
  if (shouldBeOn == exhaustStateCache) return;

  // Minimum interval check to avoid rapid toggling (e.g. due to sensor noise around threshold)
  uint32_t now = millis();
  if ((now - lastSwitchMs) < minSwitchIntervalMs) return;

  bool ok = false;
  if (shouldBeOn) {
    ok = shellySwitchOn(settings.shelly.exhaust.ip, settings.shelly.exhaust.gen, 0, 80);
    if (ok) logPrint("[minVPD] Exhaust ON minEff=" + String(minEff, 2) + " current=" + String(currentVpd, 2));
  } else {
    ok = shellySwitchOff(settings.shelly.exhaust.ip, settings.shelly.exhaust.gen, 0, 80);
    if (ok) logPrint("[minVPD] Exhaust OFF minEff=" + String(minEff, 2) + " current=" + String(currentVpd, 2));
  }

  if (ok) {
    exhaustStateCache = shouldBeOn; // local cache update only on successful request
    lastSwitchMs = now;
  }
}

// Returns true if current minute-of-hour is inside the schedule window (0..59).
// Supports wrapped windows, e.g. 50 -> 10.
static bool isMinuteInWindowHour(int nowMin, int startMin, int endMin) {

  // Same start/end means no active window
  if (startMin == endMin) return true;

  // Normal window (e.g. 10 -> 40)
  if (startMin < endMin) {
    return (nowMin >= startMin && nowMin < endMin);
  }

  // Wrapped window across hour boundary (e.g. 50 -> 10)
  return (nowMin >= startMin || nowMin < endMin);
}

// Applies relay schedules once per task cycle.
// Scheduling works per hour and compares only the minute-of-hour.
static void applyRelaySchedules() {
  // Abort if local time is not ready yet (e.g. NTP not synced)
  struct tm t;
  if (!getLocalTime(&t, 50)) return;

  // Per-hour mode: use the current minute inside the hour (0..59)
  const int nowMin = t.tm_min;

  // Determine whether Shelly light status is reliable
  const bool lightStatusValid = shelly.light.values.ok;
  const bool lightIsOn = lightStatusValid ? shelly.light.values.isOn : false;

  // Only relays 1..5 currently support minute scheduling
  const int scheduledRelayCount = (activeRelayCount < 5) ? activeRelayCount : 5;

  for (int i = 0; i < scheduledRelayCount; i++) {
    const RelaySchedule& sc = settings.relay.schedule[i];

    // Skip relay if scheduling is disabled
    if (!sc.enabled) {
      relayStates[i] = (digitalRead(relayPins[i]) == HIGH);
      continue;
    }

    // Validate minute values for per-hour scheduling mode
    if (sc.onMin < 0 || sc.onMin > 59 || sc.offMin < 0 || sc.offMin > 59) {
      relayStates[i] = (digitalRead(relayPins[i]) == HIGH);
      continue;
    }

    // Same start/end (e.g. 0/0) means schedule is always active
    const bool permanentOn = (sc.onMin == sc.offMin);

    // Determine desired relay state by schedule window
    bool shouldBeOn = permanentOn ? true : isMinuteInWindowHour(nowMin, sc.onMin, sc.offMin);

    // Light gating always applies:
    // - If light status is unknown -> fail-safe OFF
    // - If light is OFF -> relay may run only if ifLightOff is true
    if (!lightStatusValid) {
      shouldBeOn = false;
    } else if (!lightIsOn && !sc.ifLightOff) {
      shouldBeOn = false;
    }

    // Read current physical relay state (HIGH = ON in this project)
    const bool isOn = (digitalRead(relayPins[i]) == HIGH);

    // Apply state only if needed
    if (shouldBeOn != isOn) {
      digitalWrite(relayPins[i], shouldBeOn ? HIGH : LOW);
      relayStates[i] = shouldBeOn;

      logPrint(
        "[SCHED] Relay " + String(i + 1) +
        (shouldBeOn ? " ON" : " OFF") +
        " | enabled=" + String(sc.enabled ? "1" : "0") +
        " | onMin=" + String(sc.onMin) +
        " | offMin=" + String(sc.offMin) +
        " | permanentOn=" + String(permanentOn ? "1" : "0") +
        " | lightValid=" + String(lightStatusValid ? "1" : "0") +
        " | lightOn=" + String(lightIsOn ? "1" : "0") +
        " | ifLightOff=" + String(sc.ifLightOff ? "1" : "0")
      );
    } else {
      relayStates[i] = isOn;
    }
  }
}

// Clamp an integer value between a minimum (lo) and maximum (hi)
static int clampInt(int v, int lo, int hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

// Resets Shelly energy counters for both main and light devices (if configured).
static void handleResetShellyEnergy() {
    if (!preferences.begin(PREF_NS, false)) {
        server.send(500, "application/json; charset=utf-8", "{\"ok\":false,\"err\":\"prefs\"}");
        return;
    }

    auto resetOne = [&](ShellyDevice& device, const char* offsetKey) -> bool {
        if (device.ip.isEmpty()) {
            return false;
        }

        device.energyOffsetWh = 0.0f;

        const ShellyValues values = getShellyValues(device, 0, 80);
        const float rawEnergyWh = isnan(values.energyWh) ? 0.0f : values.energyWh;

        device.energyOffsetWh = rawEnergyWh;
        preferences.putFloat(offsetKey, rawEnergyWh);

        const bool resetSucceeded = shellyResetEnergyCounters(device, 0, 80);

        if (resetSucceeded) {
            device.energyOffsetWh = 0.0f;
            preferences.putFloat(offsetKey, 0.0f);
        }

        return resetSucceeded;
    };

    const bool mainResetOk  = resetOne(settings.shelly.main, KEY_SHELLYMAINOFF);
    const bool lightResetOk = resetOne(settings.shelly.light, KEY_SHELLYLIGHTOFF);
    const bool humidifierResetOk = resetOne(settings.shelly.humidifier, KEY_SHELLYHUMOFF);
    const bool heaterResetOk = resetOne(settings.shelly.heater, KEY_SHELLYHEATEROFF);
    const bool fanResetOk = resetOne(settings.shelly.fan, KEY_SHELLYFANOFF);
    const bool exhaustResetOk = resetOne(settings.shelly.exhaust, KEY_SHELLYEXHAUSTOFF);

    preferences.end();

    String response =
        String("{\"ok\":true,\"main\":") +
        (mainResetOk ? "true" : "false") +
        ",\"light\":" +
        (lightResetOk ? "true" : "false") +
        ",\"humidifier\":" +
        (humidifierResetOk ? "true" : "false") +
        ",\"heater\":" +
        (heaterResetOk ? "true" : "false") +
        "}";

    server.send(200, "application/json; charset=utf-8", response);
}

// Humidifier control:
// Trigger Shelly humidifier ON when VPD is above target.
// Shelly auto-off handles the OFF time on the Shelly device.
static void controlHumidifierByVPD() {
    // Run check only every 20 seconds
    static uint32_t lastCheckMs = 0;
    const uint32_t checkIntervalMs = 20000UL;

    uint32_t now = millis();
    if (now - lastCheckMs < checkIntervalMs) return;
    lastCheckMs = now;

    // Validate sensor and target values
    if (!isfinite(cur.vpdKpa)) return;
    if (!isfinite(settings.grow.targetVPD)) return;
    if (settings.shelly.humidifier.ip.length() == 0) return;

    // Use configured VPD hysteresis only for logging / future tuning.
    // IMPORTANT:
    // Do NOT add hysteresis to target here, otherwise the controller will
    // intentionally keep VPD above the target.
    float target = settings.grow.targetVPD;
    float hyst   = settings.grow.vpdHysteresis;

    if (!isfinite(hyst) || hyst < 0.0f) hyst = 0.0f;

    logPrint(
        "[HUM] cur=" + String(cur.vpdKpa, 3) +
        " target=" + String(target, 3) +
        " hyst=" + String(hyst, 3)
    );

    // Humidifier lowers VPD.
    // Therefore trigger as soon as VPD is above target.
    if (cur.vpdKpa <= target) {
        return;
    }

    // Only send ON if Shelly status is known and currently OFF.
    // Shelly auto-off will switch it off again after the configured pulse.
    if (shelly.humidifier.values.ok && !shelly.humidifier.values.isOn) {
        bool ok = shellySwitchOn(
            settings.shelly.humidifier.ip,
            settings.shelly.humidifier.gen,
            0,
            80
        );

        if (ok) {
            logPrint("[HUM] Triggered humidifier ON because VPD is above target");
        } else {
            logPrint("[HUM] Failed to trigger humidifier ON");
        }
    }
}

// Handles a pump pulse request for a specific relay index (1-based).
void handlePumpPulseIdx(int idx, uint32_t ms = 10000UL) {
    // allow only relay 6..8 (index 5..7)
    if (idx < 5 || idx > 7) {
        server.send(
            400,
            "application/json",
            "{\"ok\":false,\"err\":\"invalid_pump_idx\"}"
        );
        return;
    }

    // physical bounds check
    if (idx < 0 || idx >= NUM_RELAYS) {
        server.send(
            400,
            "application/json",
            "{\"ok\":false,\"err\":\"out_of_range\"}"
        );
        return;
    }

    // turn ON now
    setRelay(idx, true);

    // arm auto-off timer
    relayOffTime[idx] = millis() + ms;
    relayActive[idx] = true;

    String res = "{";
    res += "\"ok\":true";
    res += ",\"id\":" + String(idx + 1);
    res += ",\"state\":true";
    res += ",\"pulseMs\":" + String(ms);
    res += "}";

    server.send(200, "application/json", res);
    logPrint("[PUMP] Relay " + String(idx + 1) + " ON for " + String(ms) + "ms");
}

static void processPumpAutoOff() {
  uint32_t now = millis();

  for (int relayNo = 6; relayNo <= 8; relayNo++) {
    if (relayNo > activeRelayCount) continue;

    int idx = relayNo - 1;
    if (relayActive[idx] && (int32_t)(now - relayOffTime[idx]) >= 0) {
      digitalWrite(relayPins[idx], LOW);
      relayStates[idx] = false;
      relayActive[idx] = false;
    }
  }
}

// Handles the /startWatering endpoint to initiate an irrigation cycle.
void handleStartWatering() {
    // Do not start a new cycle while one is already running
    if (irrigation.irrigationRuns > 0) {
        logPrint("[IRRIGATION] Already running. Start request ignored.");
        server.sendHeader("Location", "/");
        server.send(303);
        return;
    }

    // Validate configuration
    if (
        irrigation.timePerTask <= 0 ||
        irrigation.betweenTasks <= 0 ||
        irrigation.amountOfWater <= 0.0f ||
        irrigation.amount <= 0.0f
    ) {
        logPrint(
            "[IRRIGATION] Invalid configuration. Check timePerTask, "
            "betweenTasks, amountOfWater, irrigationAmount."
        );
        server.sendHeader("Location", "/");
        server.send(303);
        return;
    }

    // amountOfWater = ml delivered in 10 seconds PER POT / PER RELAY
    // timePerTask   = seconds watering time per task
    // amount        = target total ml PER POT
    const float mlPerSecondPerPot = irrigation.amountOfWater / 10.0f;
    const float mlPerTaskPerPot =
        mlPerSecondPerPot * (float)irrigation.timePerTask;

    if (mlPerTaskPerPot <= 0.0f) {
        logPrint("[IRRIGATION] Computed mlPerTaskPerPot <= 0. Aborting.");
        server.sendHeader("Location", "/");
        server.send(303);
        return;
    }

    // Number of tasks needed so that each pot reaches the configured target amount
    irrigation.irrigationRuns =
        (int)ceilf(irrigation.amount / mlPerTaskPerPot);

    if (irrigation.irrigationRuns <= 0) {
        logPrint("[IRRIGATION] Computed runs <= 0. Aborting.");
        server.sendHeader("Location", "/");
        server.send(303);
        return;
    }

    // Initialize remaining time display
    irrigation.wTimeLeft = calculateEndtimeWatering();

    logPrint(
        "[IRRIGATION] Starting watering: targetPerPot=" +
        String(irrigation.amount, 1) +
        " ml, mlPerTaskPerPot=" + String(mlPerTaskPerPot, 1) +
        ", runs=" + String(irrigation.irrigationRuns) +
        ", plannedPerPot=" + String(irrigation.irrigationRuns * mlPerTaskPerPot, 1) + " ml"
    );

    if (language == "de") {
        sendPushover(
            boxName + "Bewässerung startet. Dauer: " + calculateEndtimeWatering(),
            boxName + "Bewässerung startet."
        );
        sendGotify(
            boxName + "Bewässerung startet. Dauer: " + calculateEndtimeWatering(),
            boxName + "Bewässerung startet."
        );
    } else {
        sendPushover(
            boxName + "Irrigation started. Duration: " + calculateEndtimeWatering(),
            boxName + "Irrigation started."
        );
        sendGotify(
            boxName + "Irrigation started. Duration: " + calculateEndtimeWatering(),
            boxName + "Irrigation started."
        );  
    }

    server.sendHeader("Location", "/");
    server.send(303);
}

// Calculates the percentage fill level of a tank based on current, minimum, and maximum values.
float calculateTankPercent(float current, float minTank, float maxTank) {
  if (maxTank == minTank) return 0;
  float percent = (current - minTank) / (maxTank - minTank) * 100;
  percent = fmax(0, fmin(100, percent));
  return round(percent);
}

// Reads the tank level using an ultrasonic sensor and updates the internal state and logs the result.
void readTankLevel() {
  tankLevelCm = pingTankLevel(TRIG, ECHO);
  if (!isnan(tankLevelCm)) {
    logPrint("[TANK LEVEL] Current distance to water: " + String(tankLevelCm, 0) + " cm");
    server.sendHeader("Location", "/");
    server.send(303);
    if (irrigation.tank.max == 0 || irrigation.tank.max == irrigation.tank.min) return;
    tankLevel = calculateTankPercent(tankLevelCm, irrigation.tank.min, irrigation.tank.max);
    logPrint("[TANK LEVEL] Current tank level: " + String(tankLevel) + " %");
  } else {
    logPrint("[TANK LEVEL] Error reading tank level.");
    server.sendHeader("Location", "/");
    server.send(303);
  }
}

// Calculates the estimated end time for the watering process based on the number of runs, time per task, and pause between tasks.
String calculateEndtimeWatering() {
    const unsigned long runs =
        (irrigation.irrigationRuns > 0) ? (unsigned long)irrigation.irrigationRuns : 0;

    const unsigned long relayCount = 3;
    const unsigned long interRelayGapMs = 250;

    // One task = all 3 relays sequentially
    const unsigned long taskTimeMs =
        relayCount * secondsToMilliseconds(irrigation.timePerTask) +
        (relayCount - 1) * interRelayGapMs;

    // Pause only between tasks, not after the last one
    const unsigned long pauseTimeMs =
        (runs > 1)
            ? (runs - 1) * minutesToMilliseconds(irrigation.betweenTasks)
            : 0;

    const unsigned long totalMs = runs * taskTimeMs + pauseTimeMs;

    const unsigned long totalSeconds = totalMs / 1000;
    const unsigned long hours = totalSeconds / 3600;
    const unsigned long minutes = (totalSeconds % 3600) / 60;

    char buf[6];
    snprintf(buf, sizeof(buf), "%02lu:%02lu", hours, minutes);

    logPrint(
        "[IRRIGATION] Estimated total irrigation time: " +
        String(hours) + " hours and " + String(minutes) + " minutes."
    );

    return String(buf);
}
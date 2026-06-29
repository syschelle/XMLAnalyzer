#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "freertos/semphr.h"

// -------------------- Konstanten / Defines --------------------
#define MAX_RELAYS 8
#define DS18B20_PIN 4
#define STATUS_LED_PIN 23
#define BME_ADDR 0x76

#ifndef I2C_SDA
  #define I2C_SDA 21
#endif
#ifndef I2C_SCL
  #define I2C_SCL 22
#endif

// -------------------- Constants --------------------
#define MAX_RELAYS 8
#define NUM_RELAYS MAX_RELAYS // keep compatibility in existing code

extern int activeRelayCount; // runtime: 4 or 8
extern const char* KEY_RELAY_COUNT;
extern SemaphoreHandle_t relayMutex;

// -------------------- Structs (erst Typen, dann Settings) --------------------
struct DailySchedule {
    int onHour;
    int onMinute;
    int offHour;
    int offMinute;
};

struct ShellySchedules {
    DailySchedule days[7];  // 0 = Montag, 6 = Sonntag
};


// Shelly device values
struct ShellyValues {
  bool  ok = false;
  bool  isOn = false;
  float powerW = 0.0f;
  float energyWh = 0.0f;
};

// Shelly device info
struct ShellyDevice {
  String ip;
  int    gen  = 0;
  float  energyOffsetWh = 0.0f; // baseline to show 0 at "new grow"
  ShellyValues values;
  ShellySchedules schedules;
};

// Shelly overall settings
struct ShellySettings {
  ShellyDevice main;
  ShellyDevice light;
  ShellyDevice humidifier;
  ShellyDevice heater;
  ShellyDevice fan;
  ShellyDevice exhaust;

  String username;
  String password;
};

// Relay schedule
struct RelaySchedule {
  bool enabled = false;
  bool ifLightOff = false;
  int onMin = 0;
  int offMin   = 0;
};

// Relay settings
struct RelayGroup {
  String name[NUM_RELAYS];
  RelaySchedule schedule[NUM_RELAYS];
};

struct GrowGroup {
  String startDate;
  String startFlowering;
  String startDrying;
  int currentGrowDay = 0;
  int currentGrowWeek = 0;
  int currentPhaseDay = 0;
  int currentPhaseWeek = 0;

  int currentPhase = 1;

  float targetTemperature = 22.0f;
  float offsetLeafTemperature = -1.5f;
  float targetVPD = 1.0f;

  bool minVpdMonEnabled = false;
  float minVPD = 0.06f;
  float vpdHysteresis = 0.05f;

  String lightOnTime = "06:00";   // HH:MM
  int lightDayHours = 18;          // 1..20
};

struct UIGroup {
  String boxName = "newGrowTent";
  String ntpServer = "pool.ntp.org";
  String tzInfo = "Europe/Berlin";
  String language = "de";
  String theme = "light";
  String unit = "metric";
  String timeFormat = "24h";

  bool ds18b20Enabled = false;
  String ds18b20Name;
};

struct NotifyGroup {
  bool pushoverEnabled = false;
  String pushoverAppKey;
  String pushoverUserKey;
  String pushoverDevice;

  bool gotifyEnabled = false;
  String gotifyServer;
  String gotifyToken;
};

struct DebugGroup {
  bool enabled = false;
};

struct HeatingGroup {
int sourceType = 0; // 0=disabled, 1=ESP relay, 2=Shelly plug
int Relay = 0; // used only when sourceType == 1
};

struct Tank {
  float amount = 0.0f;
  float min = 0.0f;
  float max = 0.0f;
};

struct Irrigation {
  float amount = 0.0f; 
  int irrigationRuns = 0;
  int timePerTask = 0;
  int betweenTasks = 0;
  float amountOfWater = 0.0f;
  float tankLevelPercent = 0.0f;
  String wTimeLeft = "00:00";
  Tank tank;
};

struct Settings {
  DebugGroup debug;
  RelayGroup relay;
  HeatingGroup heating;
  GrowGroup grow;
  ShellySettings shelly;
  UIGroup ui;
  NotifyGroup notify;

  // Pump labels for relay 6..8
  String pump1 = "Pump 1";
  String pump2 = "Pump 2";
  String pump3 = "Pump 3";
};

extern Settings settings;

// Runtime structs (ok im Header)
struct SensorReadings {
  float temperatureC           = NAN;
  float humidityPct            = NAN;
  float extTempC               = NAN;
  float vpdKpa                 = NAN;

  float temperatureRawC        = NAN;
  float humidityRawPct         = NAN;
  float vpdRawKpa              = NAN;

  float temperatureSmoothedC   = NAN;
  float humiditySmoothedPct    = NAN;
  float vpdSmoothedKpa         = NAN;

  float alphaTemp              = 0.15f;
  float alphaTempEffective     = NAN; 
  float alphaHumidity          = 0.15f;
  float alphaHumidityEffective = NAN;

  uint32_t lastUpdateMs = 0;

  bool ok() const { 
    return !isnan(temperatureC) && !isnan(humidityPct); 
  }
};

struct Targets {
  float targetVpdKpa = NAN;
  float targetTempC  = NAN;
  float targetHumPct = NAN;
};

struct AvgAccumulator {
  float sum = 0.0f;
  uint32_t count = 0;
  inline void add(float v) { sum += v; count++; }
  inline float avg() const { return (count == 0) ? 0.0f : (sum / count); }
  inline void reset() { sum = 0.0f; count = 0; }
};

// -------------------- Globale Variablen: nur extern! --------------------

// Debug
extern const char* KEY_DEBUG_ENABLED;
extern bool debugLog;

// request restart (handled in loop)
extern volatile bool g_restartRequested;
extern volatile uint32_t g_restartAtMs;

// Logging
extern const size_t LOG_MAX_LINES;

// WiFi/AP
extern const char* KEY_APSSID;
extern const char* KEY_APPASSWORD;
extern String ssidName;
extern String ssidPassword;
extern bool wifiReady;
extern float txPower;

// Preferences namespace + keys
extern const char* PREF_NS;
extern const char* KEY_SSID;
extern const char* KEY_PASS;

// log buffer (for UI)
extern SemaphoreHandle_t relayMutex;
extern SemaphoreHandle_t logBufferMutex;

// Running settings keys
extern const char* KEY_STARTDATE;
extern const char* KEY_FLOWERDATE;
extern const char* KEY_DRYINGDATE;
extern const char* KEY_CURRENTPHASE;
extern const char* KEY_TARGETTEMP;
extern const char* KEY_LEAFTEMP;
extern const char* KEY_TARGETVPD;
extern const char* KEY_MINVPD_MON;
extern const char* KEY_MINVPD;
extern const char* KEY_HYSTERESIS;
extern const char* KEY_LIGHT_ON_TIME;
extern const char* KEY_LIGHT_DAY_HOURS;
extern const char* KEY_HEATING_RELAY;

// Relay scheduling keys (enabled as stored in Preferences)
extern const char* KEY_HEATING_SOURCE;

// Relay schedule keys (start/end as stored in Preferences)
extern const char* KEY_RELAY_START_1;
extern const char* KEY_RELAY_END_1;
extern const char* KEY_RELAY_START_2;
extern const char* KEY_RELAY_END_2;
extern const char* KEY_RELAY_START_3;
extern const char* KEY_RELAY_END_3;
extern const char* KEY_RELAY_START_4;
extern const char* KEY_RELAY_END_4;
extern const char* KEY_RELAY_START_5;
extern const char* KEY_RELAY_END_5;
// Shelly device keys
extern const char* KEY_SHELLYMAINIP;
extern const char* KEY_SHELLYMAINGEN;
extern const char* KEY_SHELLYLIGHTIP;
extern const char* KEY_SHELLYLIGHTGEN;
extern const char* KEY_SHELLYMAINOFF;
extern const char* KEY_SHELLYLIGHTOFF;
extern const char* KEY_SHELLYHEATOFF;
extern const char* KEY_SHELLYHUMIP;
extern const char* KEY_SHELLYHUMGEN;
extern const char* KEY_SHELLYHUMOFF;
extern const char* KEY_SHELLYFANOFF;
extern const char* KEY_SHELLYHEATERIP;
extern const char* KEY_SHELLYHEATERGEN;
extern const char* KEY_SHELLYHEATEROFF;
extern const char* KEY_SHELLYFANIP;
extern const char* KEY_SHELLYFANGEN;
extern const char* KEY_SHELLYFANOFF;
extern const char* KEY_SHELLYEXHAUSTIP;
extern const char* KEY_SHELLYEXHAUSTGEN;
extern const char* KEY_SHELLYEXHAUSTOFF;

extern const char* KEY_SHELLYUSERNAME;
extern const char* KEY_SHELLYUSERNAME;
extern const char* KEY_SHELLYPASSWORD;
extern const char* KEY_POWER_PRICE_KWH;
extern float powerPriceKwhEur;

// UI/settings keys
extern const char* KEY_NAME;
extern const char* KEY_LANG;
extern const char* KEY_THEME;
extern const char* KEY_UNIT;
extern const char* KEY_TFMT;
extern const char* KEY_NTPSRV;
extern const char* KEY_TZINFO;
extern const char* KEY_DS18B20ENABLE;
extern const char* KEY_DS18NAME;
extern const char* KEY_RELAYCOUNT;
extern const char* KEY_RELAY_1;
extern const char* KEY_RELAY_2;
extern const char* KEY_RELAY_3;
extern const char* KEY_RELAY_4;
extern const char* KEY_RELAY_5;
extern const char* KEY_RELAY_6;
extern const char* KEY_RELAY_7;
extern const char* KEY_RELAY_8;

// Irrigation keys
extern const char* KEY_TIMEPERTASK;
extern const char* KEY_BETWEENTASKS;
extern const char* KEY_AMOUNTOFWATER;
extern const char* KEY_IRRIGATION;
extern const char* KEY_MINTANK;
extern const char* KEY_MAXTANK;

// Notification keys
extern const char* KEY_PUSHOVER;
extern const char* KEY_PUSHOVERAPP;
extern const char* KEY_PUSHOVERUSER;
extern const char* KEY_PUSHOVERDEVICE;
extern const char* KEY_GOTIFY;
extern const char* KEY_GOTIFYSERVER;
extern const char* KEY_GOTIFYTOKEN;

// Legacy/global UI vars (used throughout runtime.h/function.h)
extern String boxName;
extern String language;
extern String theme;
extern String unit;
extern String timeFormat;
extern bool espMode;

extern int heatingRelay;

// Relay scheduling arrays + relay names (legacy)
extern bool relaySchedulesEnabled[NUM_RELAYS];
extern int  relaySchedulesStart[NUM_RELAYS];
extern int  relaySchedulesEnd[NUM_RELAYS];
extern String relayNames[NUM_RELAYS];

// Grow / phase globals (legacy)
extern String startDate;
extern String startFlowering;
extern String startDrying;
extern int curPhase;
extern float targetTemperature;
extern float offsetLeafTemperature;
extern float targetVPD;
extern bool minVpdMonEnabled;
extern float minVPD;
extern float vpdHysteresis;
extern String lightOnTime;
extern int lightDayHours;

// Notifications (legacy)
extern bool pushoverSent;
extern String pushoverEnabled;
extern String pushoverAppKey;
extern String pushoverUserKey;
extern String pushoverDevice;

extern bool gotifySent;
extern String gotifyEnabled;
extern String gotifyServer;
extern String gotifyToken;

// Relays
extern const int relayPins[NUM_RELAYS];
extern bool relayStates[NUM_RELAYS];
extern const int PUMP_COUNT;
extern unsigned long relayOffTime[NUM_RELAYS];
extern bool relayActive[NUM_RELAYS];

// Sensors
extern Adafruit_BME280 bme;
extern bool bmeAvailable;

extern volatile float DS18B20STemperature;
extern bool DS18B20;
extern String DS18B20Enable;
extern String DS18B20Name;

//Irrigation
extern Irrigation irrigation;

 // to detect changes and update UI accordingly
extern const char* KEY_IRRIGATION_TIME_PER_TASK;
extern const char* KEY_IRRIGATION_BETWEEN_TASKS;

// Tank / HC-SR04
extern volatile float tankLevel;
extern float minTank, maxTank, tankLevelCm;
extern const uint8_t TRIG;
extern const uint8_t ECHO;
extern unsigned long lastMeasureTime;
extern const unsigned long MEASURE_INTERVAL;

// LED
extern unsigned long previousMillis;
extern const unsigned long blinkInterval;
extern bool ledState;

// Averages instances
extern AvgAccumulator tempAvg;
extern AvgAccumulator humAvg;
extern AvgAccumulator vpdAvg;
extern AvgAccumulator waterTempAvg;

// Time/NTP
extern const char* DEFAULT_NTP_SERVER;
extern String ntpServer;
extern const char* DEFAULT_TZ_INFO;
extern String tzInfo;
extern struct tm local;
extern int lastSyncDay;
extern bool ntpSyncPending;
extern unsigned long ntpStartMs;

// Hint-State
extern String hintKey;
extern String hintDetailsJson;
extern uint32_t hintId;

// Buffers / circular logs
extern const uint32_t READ_INTERVAL_MS;
extern uint32_t lastRead;
extern const uint32_t MEASUREMENT_INTERVAL_MS;

#define HISTORY_INTERVAL_SEC 600  // record one point every 10 minutes
#define HISTORY_WINDOW_HOURS 12
#define NUM_VALUES (HISTORY_WINDOW_HOURS * 3600 / HISTORY_INTERVAL_SEC)  // store up to 12 hours of history
extern float temps[NUM_VALUES];
extern float hums[NUM_VALUES];
extern float vpds[NUM_VALUES];
extern float waterTemps[NUM_VALUES];
extern float sumTemp, sumHum, sumVPD, sumWaterTemp;
extern int index_pos;
extern int count;
extern int head;

// Grow runtime
extern char actualDate[11];
extern int daysSinceStart, weeksSinceStart;
extern int daysSinceFlowering, weeksSinceFlowering;
extern int daysSinceDrying, weeksSinceDrying;
extern const char* phaseNames[3];

// Log file
extern const char* LOG_PATH;
extern const uint32_t RETAIN_MS;
extern uint32_t lastCompact;
extern const uint32_t COMPACT_EVERY_MS;
extern const unsigned long LOG_INTERVAL_MS;
extern unsigned long lastLog;

static String csvFieldToString(const String& s);

// Web UI payloads (PROGMEM)
extern const char jsContent[] PROGMEM;
extern const char cssContent[] PROGMEM;
extern const char htmlContent[] PROGMEM;

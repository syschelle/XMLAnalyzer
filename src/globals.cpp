#include "globals.h"

// Settings Root
Settings settings;

// Logging
const size_t LOG_MAX_LINES = 50;

// rquest restart (handled in loop)
volatile bool g_restartRequested = false;
volatile uint32_t g_restartAtMs = 0;

// WiFi/AP
const char* KEY_APSSID = "new growtent";
const char* KEY_APPASSWORD = "GT-12$34";
String ssidName = "";
String ssidPassword = "";
bool wifiReady = false;
float txPower;

// Preferences namespace + keys
const char* PREF_NS = "growtent";
const char* KEY_SSID = "ssid";
const char* KEY_PASS = "password";
const char* KEY_DEBUG_ENABLED = "dbg";

// Running settings keys
const char* KEY_STARTDATE = "startDate";
const char* KEY_FLOWERDATE = "startFlowering";
const char* KEY_DRYINGDATE = "startDrying";
const char* KEY_CURRENTPHASE = "curPhase";
const char* KEY_TARGETTEMP = "targetTemp";
const char* KEY_LEAFTEMP = "offsetLeaf";
const char* KEY_TARGETVPD = "targetVPD";
const char* KEY_MINVPD = "minVPD";
const char* KEY_MINVPD_MON = "minVPDMon";
const char* KEY_HYSTERESIS = "vpdHysteresis";
const char* KEY_LIGHT_ON_TIME = "lightOnTime";
const char* KEY_LIGHT_DAY_HOURS = "lightDayHours";
const char* KEY_LIGHT_OFF_TIME = "lightOffTime";

// heating relay key
const char* KEY_HEATING_RELAY = "heatingRelay";

const char* KEY_RELAYCOUNT = "relayCount";

// Relay schedule keys
const char* KEY_RELAY_START_1 = "relay_start_1";
const char* KEY_RELAY_END_1   = "relay_end_1";
const char* KEY_RELAY_START_2 = "relay_start_2";
const char* KEY_RELAY_END_2   = "relay_end_2";
const char* KEY_RELAY_START_3 = "relay_start_3";
const char* KEY_RELAY_END_3   = "relay_end_3";
const char* KEY_RELAY_START_4 = "relay_start_4";
const char* KEY_RELAY_END_4   = "relay_end_4";
const char* KEY_RELAY_START_5 = "relay_start_5";
const char* KEY_RELAY_END_5   = "relay_end_5";

// Shelly device keys
const char* KEY_SHELLYMAINIP = "shMainIP";
const char* KEY_SHELLYMAINGEN = "shMainGen";
const char* KEY_SHELLYMAINOFF = "shMainOff";
const char* KEY_SHELLYLIGHTOFF = "shLightOff";
const char* KEY_SHELLYLIGHTIP = "shLightIP";
const char* KEY_SHELLYLIGHTGEN = "shLightGen";
const char* KEY_SHELLYHUMIP = "shHumIP";
const char* KEY_SHELLYHUMGEN = "shHumGen";
const char* KEY_SHELLYHUMOFF = "shHumOff";
const char* KEY_SHELLYHEATERIP = "shHeaterIP";
const char* KEY_SHELLYHEATERGEN = "shHeaterGen";
const char* KEY_SHELLYHEATEROFF = "shHeaterOff";
const char* KEY_SHELLYFANIP = "shFanIP";
const char* KEY_SHELLYFANGEN = "shFanGen";
const char* KEY_SHELLYFANOFF = "shFanOff";
const char* KEY_SHELLYEXHAUSTIP = "shExhaustIP";
const char* KEY_SHELLYEXHAUSTGEN = "shExhaustGen";
const char* KEY_SHELLYEXHAUSTOFF = "shExhaustOff";


const char* KEY_SHELLYUSERNAME = "shUser";
const char* KEY_SHELLYPASSWORD = "shPass";

// Irrigation keys
const char* KEY_TIMEPERTASK = "timePerTask";
const char* KEY_BETWEENTASKS = "betweenTasks";
const char* KEY_AMOUNTOFWATER = "amountOfWater";
const char* KEY_IRRIGATION = "amount";
const char* KEY_MINTANK = "minTank";
const char* KEY_MAXTANK = "maxTank";

// UI/settings keys
const char* KEY_NAME   = "boxName";
const char* KEY_LANG   = "lang";
const char* KEY_THEME  = "theme";
const char* KEY_UNIT   = "unit";
const char* KEY_TFMT   = "timeFmt";
const char* KEY_NTPSRV = "ntpSrv";
const char* KEY_TZINFO = "tzInfo";
const char* KEY_DS18B20ENABLE = "ds18b20enable";
const char* KEY_DS18NAME = "ds18b20Name";
const char* KEY_RELAY_1 = "relay1";
const char* KEY_RELAY_2 = "relay2";
const char* KEY_RELAY_3 = "relay3";
const char* KEY_RELAY_4 = "relay4";
const char* KEY_RELAY_5 = "relay5";
const char* KEY_RELAY_6 = "relay6";
const char* KEY_RELAY_7 = "relay7";
const char* KEY_RELAY_8 = "relay8";

// Notification keys
const char* KEY_PUSHOVER = "pushover";
const char* KEY_PUSHOVERAPP = "pushoverAppKey";
const char* KEY_PUSHOVERUSER = "pushoverUser";
const char* KEY_PUSHOVERDEVICE = "pushoverDevice";
const char* KEY_GOTIFY = "gotify";
const char* KEY_GOTIFYSERVER = "gotifyServer";
const char* KEY_GOTIFYTOKEN = "gotifyToken";

// Global variables
bool debugLog;

// NTP/time globals
int lastSyncDay = -1;
bool ntpSyncPending = false;
unsigned long ntpStartMs = 0;

// Component: Hint-State
String hintKey = "hint.none";
String hintDetailsJson = "{}";
uint32_t hintId = 0;

// Legacy/global UI vars (used throughout runtime.h/function.h)
String boxName = "newGrowTent";
String language = "de";
String theme = "light";
String unit = "metric";
String timeFormat = "24h";
bool espMode = false;

// Relay scheduling arrays + relay names (legacy)
bool relaySchedulesEnabled[NUM_RELAYS] = {false};
int  relaySchedulesStart[NUM_RELAYS] = {0};
int  relaySchedulesEnd[NUM_RELAYS] = {0};
String relayNames[NUM_RELAYS] = {"relay 1","relay 2","relay 3","relay 4,","relay 5","relay 6","relay 7","relay 8"};

// Irrigation
Irrigation irrigation;
const char* KEY_IRRIGATION_TIME_PER_TASK = "irrTimeTask";
const char* KEY_IRRIGATION_BETWEEN_TASKS = "irrBetween";

// Tank / HC-SR04
volatile float tankLevel = 0.0f;
float minTank = 0.0f;
float maxTank = 0.0f;
float tankLevelCm = NAN;

const uint8_t TRIG = 16; // adjust pins to your wiring
const uint8_t ECHO = 17; // adjust pins to your wiring

// Grow / phase globals (legacy)
String startDate = "";
String startFlowering = "";
String startDrying = "";
int curPhase = 1;
float targetTemperature = 22.0f;
float offsetLeafTemperature = -1.5f;
float targetVPD = 1.0f;
int heatingRelay = 0;
String lightOnTime = "06:00";
int lightDayHours = 18;

// Notifications (legacy)
bool pushoverSent = false;
String pushoverEnabled = "";
String pushoverAppKey = "";
String pushoverUserKey = "";
String pushoverDevice = "";

bool gotifySent = false;
String gotifyEnabled = "";
String gotifyServer = "";
String gotifyToken = "";

// Heating relay
const char* KEY_HEATING_SOURCE = "heatingSource";

// Mutexes log buffer
SemaphoreHandle_t logBufferMutex = nullptr;

// Relays
int activeRelayCount = 4;
const int relayPins[NUM_RELAYS] = { 32, 33, 25, 26, 27, 14, 12, 13 };
bool relayStates[NUM_RELAYS] = { false };
unsigned long relayOffTime[NUM_RELAYS] = {0};
bool relayActive[NUM_RELAYS] = {false};
// Mutex for relay state changes
SemaphoreHandle_t relayMutex = nullptr;

// Sensors
Adafruit_BME280 bme;
bool bmeAvailable = false;

volatile float DS18B20STemperature = NAN;
bool DS18B20 = false;
String DS18B20Enable = "";
String DS18B20Name = "";

// Power price
const char* KEY_POWER_PRICE_KWH = "pwrPriceKwh";
float powerPriceKwhEur = 0.30f; // default €/kWh

// LED
unsigned long previousMillis = 0;
const unsigned long blinkInterval = 500;
bool ledState = false;

// Averages
AvgAccumulator tempAvg;
AvgAccumulator humAvg;
AvgAccumulator vpdAvg;
AvgAccumulator waterTempAvg;

// Time/NTP
const char* DEFAULT_NTP_SERVER = "de.pool.ntp.org";
String ntpServer = DEFAULT_NTP_SERVER;

const char* DEFAULT_TZ_INFO = "WEST-1DWEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00";
String tzInfo = DEFAULT_TZ_INFO;

struct tm local;

// Timing
const uint32_t READ_INTERVAL_MS = 1000;
uint32_t lastRead = 0;
const uint32_t MEASUREMENT_INTERVAL_MS = 30000;

// Circular buffers
float temps[NUM_VALUES] = {0};
float hums[NUM_VALUES] = {0};
float vpds[NUM_VALUES] = {0};
float waterTemps[NUM_VALUES] = {0};
float sumTemp = 0.0f, sumHum = 0.0f, sumVPD = 0.0f, sumWaterTemp = 0.0f;
int index_pos = 0;
int count = 0;

// Grow runtime
char actualDate[11] = {0};
int daysSinceStart = 0, weeksSinceStart = 0;
int daysSinceFlowering = 0, weeksSinceFlowering = 0;
int daysSinceDrying = 0, weeksSinceDrying = 0;
const char* phaseNames[3] = { "Vegetative", "Flowering", "Drying" };

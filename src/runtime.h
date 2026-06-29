#pragma once

#include <Arduino.h>
#include <StreamString.h>
#include <WString.h>
#include <Preferences.h>
#include <WebServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Include global definitions and functions
#include <globals.h>
#include <globals.h>
#include <function.h>

// external DallasTemperature sensor instance (defined in a single .cpp file)
extern DallasTemperature sensors;

extern WebServer server;

void loadPrefInt(
  const char* prefKey,
  int& targetVar,
  int defaultValue = 0,
  bool logValue = true,
  const char* logLabel = nullptr
);
void loadPrefFloat(
  const char* prefKey,
  float& targetVar,
  float defaultValue = NAN,
  bool logValue = true,
  const char* logLabel = nullptr,
  uint8_t decimals = 2
);
void loadPrefBool(
  const char* prefKey,
  bool& targetVar,
  bool defaultValue = false,
  bool logValue = true,
  const char* logLabel = nullptr
);
void loadPrefString(
  const char* prefKey,
  String& targetVar,
  const char* defaultValue = "",
  bool logValue = true,
  const char* logLabel = nullptr
);



extern Preferences preferences;
extern ShellySettings shelly;
extern SensorReadings cur;
extern Targets target;
extern String readSensorData();
extern void calculateTimeSince(const String& dateStr, int& daysOut, int& weeksOut);
extern void logPrint(const String& msg);
extern void appendLog(unsigned long timestamp, float temperature, float humidity, float vpd);
extern void updateGrowTimeValues();

// forward declaration for calcVPD (defined elsewhere)
float calcVPD(float temperatureC, float leafOffset, float humidityPct);

// Handle root path "/"
void handleRoot() {
  
  String html;
  if (espMode) {
    String sensorData = readSensorData();

    // Build HTML
    html = FPSTR(apPage);
    // Replace placeholders in index_html.h
    //html.replace("%DBG_CHECKED%", debugLog ? "checked" : "");
    html.replace("%CONTROLLERNAME%", settings.ui.boxName);
    } else {
    html = FPSTR(htmlPage);

    updateGrowTimeValues();

    if (startDate != "") {
      String days = String(settings.grow.currentGrowDay);
      String weeks = String(settings.grow.currentGrowWeek);
      if (language == "de") {
        html.replace("%CURRENTGROW%", "Grow seit: Tag " + days + " / Woche " + weeks);
      } else {
        html.replace("%CURRENTGROW%", "Growing since: day " + days + " / week " + weeks);
      } 
    } else {
      html.replace("%CURRENTGROW%", "");
    }

    if (settings.grow.currentPhase == 1) {
      String days = String(settings.grow.currentPhaseDay);
      String weeks = String(settings.grow.currentPhaseWeek);
      if (language == "de") {
        html.replace("%CURRENTPHASE%", "<font color=\"lightgreen\">Wachstum: Tag " + days + " / Woche " + weeks + "</font>");
      } else {
        html.replace("%CURRENTPHASE%", "<font color=\"lightgreen\">Vegetative: day " + days + " / week " + weeks + "</font>");
      }
    } else if (settings.grow.currentPhase == 2) {
      String days = String(settings.grow.currentPhaseDay);
      String weeks = String(settings.grow.currentPhaseWeek);
      if (language == "de") {
        html.replace("%CURRENTPHASE%", "<font color=\"#ff9900\">Blüte: Tag " + days + " / Woche " + weeks + "</font>");
      } else {
        html.replace("%CURRENTPHASE%", "<font color=\"#ff9900\">Flowering: day " + days + " / week " + weeks + "</font>");
      }
    } else if (settings.grow.currentPhase == 3) {
      String days = String(settings.grow.currentPhaseDay);
      String weeks = String(settings.grow.currentPhaseWeek);
      if (language == "de") {
        html.replace("%CURRENTPHASE%", "<font color=\"lightblue\">Trocknung: Tage " + days + " / Woche " + weeks + "</font>");
      } else {
        html.replace("%CURRENTPHASE%", "<font color=\"lightblue\">Drying: day " + days + " / week " + weeks + "</font>");
      }
    } else {
      html.replace("%CURRENTPHASE%", "");
    }

    html.replace("%CONTROLLERNAME%", settings.ui.boxName);

    // Replace placeholders in index_html.h
    html.replace("%TARGETTEMPERATURE%", String(settings.grow.targetTemperature, 1));
    html.replace("%LIGHTONTIME%", lightOnTime);
    html.replace("%LIGHTDAYHOURS%", String(lightDayHours));
    html.replace("%WATERTEMPERATURE%", String(DS18B20STemperature, 1));
    html.replace("%LEAFTEMPERATURE%", String(settings.grow.offsetLeafTemperature, 1));
    html.replace("%HUMIDITY%", String(cur.humidityPct, 0));
    html.replace("%TARGETVPD%",  String(settings.grow.targetVPD, 2));
    html.replace("%MINVPDMONITORING%", settings.grow.minVpdMonEnabled ? "checked" : "");
    html.replace("%MINVPD%",  String(settings.grow.minVPD, 2));
    html.replace("%VPDHYSTERESIS%",  String(settings.grow.vpdHysteresis, 2));

    // Irrigation settings
    html.replace("%TIMEPERTASK%", String(irrigation.timePerTask));
    html.replace("%BETWEENTASKS%", String(irrigation.betweenTasks));
    html.replace("%AMOUNTOFWATER%", String(irrigation.amountOfWater));
    html.replace("%IRRIGATION%", String(irrigation.amount));
    
    String minTankStr = String(irrigation.tank.min, 1); minTankStr.trim();
    String maxTankStr = String(irrigation.tank.max, 1); maxTankStr.trim();
    html.replace("%MINTANK%", minTankStr);
    html.replace("%MAXTANK%", maxTankStr);

    html.replace("%RELAYNAMES1%", String(settings.relay.name[0]));
    html.replace("%ESPRELAY1_ENABLED_CHECKED%", settings.relay.schedule[0]. enabled ? "checked" : "");
    html.replace("%ESPRELAY1_IFLIGHTOFF_CHECKED%", settings.relay.schedule[0].ifLightOff ? "checked" : "");
    html.replace("%ESPRELAY1_ONMIN%", String(settings.relay.schedule[0].onMin));
    html.replace("%ESPRELAY1_OFFMIN%", String(settings.relay.schedule[0].offMin));
    html.replace("%RELAYNAMES2%", String(settings.relay.name[1]));
    html.replace("%ESPRELAY2_ENABLED_CHECKED%", settings.relay.schedule[1]. enabled ? "checked" : "");
    html.replace("%ESPRELAY2_IFLIGHTOFF_CHECKED%", settings.relay.schedule[1].ifLightOff ? "checked" : "");
    html.replace("%ESPRELAY2_ONMIN%", String(settings.relay.schedule[1].onMin));
    html.replace("%ESPRELAY2_OFFMIN%", String(settings.relay.schedule[1].offMin));
    html.replace("%RELAYNAMES3%", String(settings.relay.name[2]));
    html.replace("%ESPRELAY3_ENABLED_CHECKED%", settings.relay.schedule[2]. enabled ? "checked" : "");
    html.replace("%ESPRELAY3_IFLIGHTOFF_CHECKED%", settings.relay.schedule[2].ifLightOff ? "checked" : "");
    html.replace("%ESPRELAY3_ONMIN%", String(settings.relay.schedule[2].onMin));
    html.replace("%ESPRELAY3_OFFMIN%", String(settings.relay.schedule[2].offMin));
    html.replace("%RELAYNAMES4%", String(settings.relay.name[3]));
    html.replace("%ESPRELAY4_ENABLED_CHECKED%", settings.relay.schedule[3]. enabled ? "checked" : "");
    html.replace("%ESPRELAY4_IFLIGHTOFF_CHECKED%", settings.relay.schedule[3].ifLightOff ? "checked" : "");
    html.replace("%ESPRELAY4_ONMIN%", String(settings.relay.schedule[3].onMin));
    html.replace("%ESPRELAY4_OFFMIN%", String(settings.relay.schedule[3].offMin));
    html.replace("%RELAYNAMES5%", String(settings.relay.name[4]));
    html.replace("%ESPRELAY5_ENABLED_CHECKED%", settings.relay.schedule[4]. enabled ? "checked" : "");
    html.replace("%ESPRELAY5_IFLIGHTOFF_CHECKED%", settings.relay.schedule[4].ifLightOff ? "checked" : "");
    html.replace("%ESPRELAY5_ONMIN%", String(settings.relay.schedule[4].onMin));
    html.replace("%ESPRELAY5_OFFMIN%", String(settings.relay.schedule[4].offMin));

    html.replace("%HEATSRC0_SEL%", settings.heating.sourceType == 0 ? "selected" : "");
    html.replace("%HEATSRC1_SEL%", settings.heating.sourceType == 1 ? "selected" : "");
    html.replace("%HEATSRC2_SEL%", settings.heating.sourceType == 2 ? "selected" : "");

    html.replace("%DBG_CHECKED%", debugLog ? "checked" : "");

    html.replace("%GROWSTARTDATE%", String(startDate));
    html.replace("%GROWFLOWERDATE%", String(startFlowering));
    html.replace("%GROWDRAYINGDATE%", String(startDrying));

    html.replace("%PHASE1_SEL%", settings.grow.currentPhase == 1 ? "selected" : "");
    html.replace("%PHASE2_SEL%", settings.grow.currentPhase == 2 ? "selected" : "");
    html.replace("%PHASE3_SEL%", settings.grow.currentPhase == 3 ? "selected" : "");

    html.replace("%TARGETVPD%", String(settings.grow.targetVPD, 1));

    html.replace("%HEATRELAY1_SEL%", settings.heating.Relay == 1 ? "selected" : "");
    html.replace("%HEATRELAY2_SEL%", settings.heating.Relay == 2 ? "selected" : "");
    html.replace("%HEATRELAY3_SEL%", settings.heating.Relay == 3 ? "selected" : "");
    html.replace("%HEATRELAY4_SEL%", settings.heating.Relay == 4 ? "selected" : "");

    html.replace("%SHELLYMAINIP%", settings.shelly.main.ip);
    html.replace("%SHMAINSWKIND1%", settings.shelly.main.gen == 1 ? "selected" : "");
    html.replace("%SHMAINSWKIND2%", settings.shelly.main.gen == 2 ? "selected" : "");
    html.replace("%SHMAINSWKIND3%", settings.shelly.main.gen == 3 ? "selected" : "");

    html.replace("%SHELLYLIGHTIP%", settings.shelly.light.ip);
    html.replace("%SHLIGHTKIND1%", settings.shelly.light.gen == 1 ? "selected" : "");
    html.replace("%SHLIGHTKIND2%", settings.shelly.light.gen == 2 ? "selected" : "");
    html.replace("%SHLIGHTKIND3%", settings.shelly.light.gen == 3 ? "selected" : "");

    html.replace("%SHELLYHUMIDIFIERIP%", settings.shelly.humidifier.ip);
    html.replace("%SHHUMIDIFIERKIND1%", settings.shelly.humidifier.gen == 1 ? "selected" : "");
    html.replace("%SHHUMIDIFIERKIND2%", settings.shelly.humidifier.gen == 2 ? "selected" : "");
    html.replace("%SHHUMIDIFIERKIND3%", settings.shelly.humidifier.gen == 3 ? "selected" : "");

    html.replace("%SHELLYHEATERIP%", settings.shelly.heater.ip);
    html.replace("%SHHEATERKIND1%", settings.shelly.heater.gen == 1 ? "selected" : "");
    html.replace("%SHHEATERKIND2%", settings.shelly.heater.gen == 2 ? "selected" : "");
    html.replace("%SHHEATERKIND3%", settings.shelly.heater.gen == 3 ? "selected" : "");

    html.replace("%SHELLYFANIP%", settings.shelly.fan.ip);
    html.replace("%SHFANKIND1%", settings.shelly.fan.gen == 1 ? "selected" : "");
    html.replace("%SHFANKIND2%", settings.shelly.fan.gen == 2 ? "selected" : "");
    html.replace("%SHFANKIND3%", settings.shelly.fan.gen == 3 ? "selected" : "");

    html.replace("%SHELLYEXHAUSTIP%", settings.shelly.exhaust.ip);
    html.replace("%SHEXHAUSTKIND1%", settings.shelly.exhaust.gen == 1 ? "selected" : "");
    html.replace("%SHEXHAUSTKIND2%", settings.shelly.exhaust.gen == 2 ? "selected" : "");
    html.replace("%SHEXHAUSTKIND3%", settings.shelly.exhaust.gen == 3 ? "selected" : "");

    html.replace("%POWERPRICEKWH%", String(powerPriceKwhEur, 2));

    html.replace("%SHUSER%", settings.shelly.username);
    html.replace("%SHPASSWORD%", settings.shelly.password);

    html.replace("%RELAYCOUNT4_SEL%", activeRelayCount == 4 ? "selected" : "");
    html.replace("%RELAYCOUNT8_SEL%", activeRelayCount == 8 ? "selected" : "");


    html.replace("%NTPSERVER%", ntpServer);
    html.replace("%TZINFO%", tzInfo);
    html.replace("%THEME%", theme);
    html.replace("%LANGUAGE%", language);
    html.replace("%TIMEFORMAT%", timeFormat);
    html.replace("%UNIT%", unit);
    html.replace("%DS18B20ENABLE%", DS18B20Enable);
    html.replace("%DS18B20NAME%", DS18B20Name);

    html.replace("%PUSHOVERENABLED%", pushoverEnabled);
    html.replace("%PUSHOVERAPPKEY%", pushoverAppKey);
    html.replace("%PUSHOVERUSERKEY%", pushoverUserKey);
    html.replace("%PUSHOVERDEVICE%", pushoverDevice);
    html.replace("%GOTIFYENABLED%", gotifyEnabled);
    html.replace("%GOTIFYURL%", gotifyServer);
    html.replace("%GOTIFYTOKEN%", gotifyToken);
  }

  server.send(200, "text/html", html);
}

// Read stored preferences
void readPreferences() {
  preferences.begin(PREF_NS, true);

  // debug
  loadPrefBool(KEY_DEBUG_ENABLED, debugLog, false, true, "debugLog");

  // active relay count (4 or 8)
  loadPrefInt(KEY_RELAYCOUNT, activeRelayCount, 4, true, "activeRelayCount");
  activeRelayCount = (activeRelayCount == 8) ? 8 : 4;

  // relays
  settings.relay.name[0] = preferences.getString(KEY_RELAY_1, "relay 1");
  settings.relay.name[1] = preferences.getString(KEY_RELAY_2, "relay 2");
  settings.relay.name[2] = preferences.getString(KEY_RELAY_3, "relay 3");
  settings.relay.name[3] = preferences.getString(KEY_RELAY_4, "relay 4");
  if (activeRelayCount == 8) {
    settings.relay.name[4] = preferences.getString(KEY_RELAY_5, "relay 5");
    settings.relay.name[5] = preferences.getString(KEY_RELAY_6, "relay 6");
    settings.relay.name[6] = preferences.getString(KEY_RELAY_7, "relay 7");
    settings.relay.name[7] = preferences.getString(KEY_RELAY_8, "relay 8");
  }

  // running settings
  loadPrefString(KEY_STARTDATE, startDate, "", true, "startDate");
  loadPrefString(KEY_FLOWERDATE, startFlowering, "", true, "startFlowering");
  loadPrefString(KEY_DRYINGDATE, startDrying, "", true, "startDrying");
  loadPrefInt(KEY_CURRENTPHASE, settings.grow.currentPhase, 1, true, "settings.grow.currentPhase");
  loadPrefFloat(KEY_LEAFTEMP, offsetLeafTemperature, -1.5f, true, "offsetLeafTemperature");

  // Load settings.grow targets from same persisted keys (future primary path)
  loadPrefFloat(KEY_TARGETTEMP, settings.grow.targetTemperature, 22.0f, true, "settings.grow.targetTemperature");
  loadPrefFloat(KEY_LEAFTEMP, settings.grow.offsetLeafTemperature, -1.5f, true, "settings.grow.offsetLeafTemperature");
  loadPrefFloat(KEY_TARGETVPD, settings.grow.targetVPD, 1.0f, true, "settings.grow.targetVPD");
  loadPrefBool(KEY_MINVPD_MON, settings.grow.minVpdMonEnabled, false, true, "settings.grow.minVpdMonEnabled");

  loadPrefFloat(KEY_MINVPD, settings.grow.minVPD, 0.75f, true, "settings.grow.minVPD");
  loadPrefFloat(KEY_HYSTERESIS, settings.grow.vpdHysteresis, 0.05f, true, "settings.grow.vpdHysteresis");

  //load heating settings
  loadPrefInt(KEY_HEATING_SOURCE, settings.heating.sourceType, 0, true, "heatingSource");
  loadPrefInt(KEY_HEATING_RELAY, settings.heating.Relay, 0, true, "heatingRelay");
  // sanitize
  if (settings.heating.sourceType < 0 || settings.heating.sourceType > 2) {
    settings.heating.sourceType = 0;
  }
  if (settings.heating.sourceType != 1) {
    settings.heating.Relay = 0;
  }

  settings.grow.lightOnTime = lightOnTime;
  settings.grow.lightDayHours = lightDayHours;

  // relay schedules
  // Load relay schedules into settings.relay.schedule[0..3]
  for (int i = 0; i < activeRelayCount; i++) {
    int relay = i + 1;

    String keyEn = "relay_enable_" + String(relay);
    String keyILO = "ilo_" + String(relay);

    const char* keyOn =
    (relay == 1) ? KEY_RELAY_START_1 :
    (relay == 2) ? KEY_RELAY_START_2 :
    (relay == 3) ? KEY_RELAY_START_3 :
    (relay == 4) ? KEY_RELAY_START_4 :
    KEY_RELAY_START_5;

    const char* keyOff =
    (relay == 1) ? KEY_RELAY_END_1 :
    (relay == 2) ? KEY_RELAY_END_2 :
    (relay == 3) ? KEY_RELAY_END_3 :
    (relay == 4) ? KEY_RELAY_END_4 :
    KEY_RELAY_END_5;

    settings.relay.schedule[i].enabled = preferences.getBool(keyEn.c_str(), false);
    settings.relay.schedule[i].ifLightOff = preferences.getBool(keyILO.c_str(), false);
    settings.relay.schedule[i].onMin = preferences.getInt(keyOn, 0);
    settings.relay.schedule[i].offMin = preferences.getInt(keyOff, 0);
  }

  // Shelly devices
  loadPrefString(KEY_SHELLYMAINIP, settings.shelly.main.ip, "", true, "Shelly Main IP");
  loadPrefInt(KEY_SHELLYMAINGEN, settings.shelly.main.gen, 0, true, "Shelly Main Generation");
  loadPrefFloat(KEY_SHELLYMAINOFF, settings.shelly.main.energyOffsetWh, 0.0f, true, "Shelly Main Energy Offset");
  loadPrefString(KEY_SHELLYLIGHTIP, settings.shelly.light.ip, "", true, "Shelly Light IP");
  loadPrefInt(KEY_SHELLYLIGHTGEN, settings.shelly.light.gen, 0, true, "Shelly Light Generation");
  loadPrefFloat(KEY_SHELLYLIGHTOFF, settings.shelly.light.energyOffsetWh, 0.0f, true, "Shelly Light Energy Offset");
  loadPrefString(KEY_SHELLYHUMIP, settings.shelly.humidifier.ip, "", true, "Shelly Humidifier IP");
  loadPrefInt(KEY_SHELLYHUMGEN, settings.shelly.humidifier.gen, 0, true, "Shelly Humidifier Generation");
  loadPrefFloat(KEY_SHELLYHUMOFF, settings.shelly.humidifier.energyOffsetWh, 0.0f, true, "Shelly Humidifier Energy Offset");
  loadPrefString(KEY_SHELLYHEATERIP, settings.shelly.heater.ip, "", true, "Shelly Heater IP");
  loadPrefInt(KEY_SHELLYHEATERGEN, settings.shelly.heater.gen, 0, true, "Shelly Heater Generation");
  loadPrefFloat(KEY_SHELLYHEATEROFF, settings.shelly.heater.energyOffsetWh, 0.0f, true, "Shelly Heater Energy Offset");
  loadPrefString(KEY_SHELLYFANIP, settings.shelly.fan.ip, "", true, "Shelly Fan IP");
  loadPrefInt(KEY_SHELLYFANGEN, settings.shelly.fan.gen, 0, true, "Shelly Fan Generation");
  loadPrefString(KEY_SHELLYEXHAUSTIP, settings.shelly.exhaust.ip, "", true, "Shelly Exhaust IP");
  loadPrefInt(KEY_SHELLYEXHAUSTGEN, settings.shelly.exhaust.gen, 0, true, "Shelly Exhaust Generation");
  loadPrefFloat(KEY_SHELLYEXHAUSTOFF, settings.shelly.exhaust.energyOffsetWh, 0.0f, true, "Shelly Exhaust Energy Offset");
  loadPrefFloat(KEY_SHELLYFANOFF, settings.shelly.fan.energyOffsetWh, 0.0f, true, "Shelly Fan Energy Offset");

  // Shelly credentials (optional Basic Auth)
  loadPrefString(KEY_SHELLYUSERNAME, settings.shelly.username, "", true, "Shelly Username");
  loadPrefString(KEY_SHELLYPASSWORD, settings.shelly.password, "", false, "Shelly Password");

  // Power price
  loadPrefFloat(KEY_POWER_PRICE_KWH, powerPriceKwhEur, 0.30f, true, "powerPriceKwhEur", 3);

  // settings
  loadPrefString(KEY_NAME, settings.ui.boxName, "newGrowTent", true, "boxName");
  loadPrefString(KEY_NTPSRV, ntpServer, DEFAULT_NTP_SERVER, true, "ntpServer");
  loadPrefString(KEY_TZINFO, tzInfo, DEFAULT_TZ_INFO, true, "tzInfo");
  loadPrefString(KEY_LANG, language, "de", true, "language");
  loadPrefString(KEY_THEME, theme, "light", true, "theme");
  loadPrefString(KEY_UNIT, unit, "metric", true, "unit");
  loadPrefString(KEY_TFMT, timeFormat, "24h", true, "timeFormat");
  loadPrefBool(KEY_DS18B20ENABLE, DS18B20, false, true, "DS18B20Enable");
  DS18B20Enable = DS18B20 ? "checked" : "";
  loadPrefString(KEY_DS18NAME, DS18B20Name, "", true, "DS18B20Name");

  loadPrefInt(KEY_TIMEPERTASK, irrigation.timePerTask, 10, true, "timePerTask");
  loadPrefInt(KEY_BETWEENTASKS, irrigation.betweenTasks, 5, true, "betweenTasks");
  loadPrefFloat(KEY_AMOUNTOFWATER, irrigation.amountOfWater, 20, true, "amountOfWater");
  loadPrefFloat(KEY_IRRIGATION, irrigation.amount, 500, true, "irrigationAmount");

  loadPrefFloat(KEY_MINTANK, irrigation.tank.min, 0.0f, true, "minTank");
  loadPrefFloat(KEY_MAXTANK, irrigation.tank.max, 0.0f, true, "maxTank");


  // notification settings
  loadPrefString(KEY_PUSHOVER, pushoverEnabled, "", true, "pushoverEnabled");
  if (pushoverEnabled == "checked") pushoverSent = true;
  loadPrefString(KEY_PUSHOVERAPP, pushoverAppKey, "", true, "pushoverAppKey");
  loadPrefString(KEY_PUSHOVERUSER, pushoverUserKey, "", true, "pushoverUserKey");
  loadPrefString(KEY_PUSHOVERDEVICE, pushoverDevice, "", true, "pushoverDevice");
  loadPrefString(KEY_GOTIFY, gotifyEnabled, "", true, "gotifyEnabled");
  if (gotifyEnabled == "checked") gotifySent = true;
  loadPrefString(KEY_GOTIFYSERVER, gotifyServer, "", true, "gotifyServer");
  loadPrefString(KEY_GOTIFYTOKEN, gotifyToken, "", true, "gotifyToken");

  preferences.end();
}

// Forward declaration so this header can call the function defined later
String readSensorData();
void calculateTimeSince(const String& startDate, int& daysSinceStartInt, int& weeksSinceStartInt);

// Forward-declare notification functions used before their definitions
bool sendPushover(const String& message, const String& title);
bool sendGotify(const String& msg, const String& title, int priority = 5);
String calculateEndtimeWatering();


float getAdaptiveAlpha(float diff, float slowAlpha, float fastAlpha, float threshold) {
  return (diff > threshold) ? fastAlpha : slowAlpha;
}

void updateSmoothedClimate(float rawTemp, float rawHum) {
  if (isnan(rawTemp) || isnan(rawHum)) return;

  if (rawHum < 0.0f) rawHum = 0.0f;
  if (rawHum > 100.0f) rawHum = 100.0f;

  if (isnan(cur.temperatureSmoothedC)) {
    cur.temperatureSmoothedC = rawTemp;
  }

  if (isnan(cur.humiditySmoothedPct)) {
    cur.humiditySmoothedPct = rawHum;
  }

  float tempDiff = fabs(rawTemp - cur.temperatureSmoothedC);
  float humDiff  = fabs(rawHum - cur.humiditySmoothedPct);

  float alphaTemp = getAdaptiveAlpha(tempDiff, 0.15f, 0.40f, 0.8f);
  float alphaHum  = getAdaptiveAlpha(humDiff, 0.15f, 0.35f, 3.0f);

  // Store effective alphas for debugging/monitoring
  cur.alphaTempEffective     = alphaTemp;
  cur.alphaHumidityEffective = alphaHum;

  cur.temperatureSmoothedC =
      alphaTemp * rawTemp + (1.0f - alphaTemp) * cur.temperatureSmoothedC;

  cur.humiditySmoothedPct =
      alphaHum * rawHum + (1.0f - alphaHum) * cur.humiditySmoothedPct;
}

// Read sensors and build JSON string for API response
String readSensorData() {

  // read DS18B20 water temperature if enabled
  if (DS18B20) {
    if (sensors.isConversionComplete()) {
      float dsTemp = sensors.getTempCByIndex(0);
      if (dsTemp != DEVICE_DISCONNECTED_C && dsTemp != 85.0f && dsTemp > -100.0f) {
        DS18B20STemperature = dsTemp;
        cur.extTempC = dsTemp;
      }
    }
    sensors.requestTemperatures();
  }
  
  unsigned long now = millis();
  struct tm timeinfo;
  char timeStr[32] = "";
  if (getLocalTime(&timeinfo)) {
    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
  }

  if (bmeAvailable) {
    if (now - previousMillis >= blinkInterval) {
      previousMillis = now;
      ledState = !ledState;
      digitalWrite(STATUS_LED_PIN, ledState);
    }

    if (now - lastRead >= READ_INTERVAL_MS) {
      lastRead = now;

      // === RAW SENSOR READ ===
      float rawTemp = bme.readTemperature();
      float rawHum  = bme.readHumidity();

      // === STORE RAW VALUES ===
      cur.temperatureRawC = rawTemp;
      cur.humidityRawPct  = rawHum;
      cur.vpdRawKpa       = calcVPD(cur.temperatureRawC, offsetLeafTemperature, cur.humidityRawPct);

      // === SMOOTHING ===
      updateSmoothedClimate(rawTemp, rawHum);

      // === CALCULATE SMOOTHED VPD ===
      cur.vpdSmoothedKpa = calcVPD(cur.temperatureSmoothedC, offsetLeafTemperature, cur.humiditySmoothedPct);

      // === USE SMOOTHED VALUES FOR SYSTEM ===
      cur.temperatureC = cur.temperatureSmoothedC;
      cur.humidityPct  = cur.humiditySmoothedPct;
      cur.vpdKpa       = cur.vpdSmoothedKpa;
    }
  }

  // === JSON BUILDING (unchanged) ===
  String json = "{\n";

  if (!isnan(settings.grow.currentPhase)) {
    json += "\"curGrowPhase\":" + String(settings.grow.currentPhase) + ",\n";
  } else {
    json += "\"curGrowPhase\":null,\n";
  } 

  if (!isnan(cur.temperatureC)) {
    json += "\"curTemperature\":" + String(cur.temperatureC, 1) + ",\n";
  } else {
    json += "\"curTemperature\":null,\n";
  } 

  if (!isnan(cur.extTempC)) {
    json += "\"curDS18B20Se1\":" + String(cur.extTempC, 1) + ",\n";
  } else {
    json += "\"curDS18B20Se1\":null,\n";
  }

  if (!isnan(cur.humidityPct)) {
    json += "\"curHumidity\":" + String(cur.humidityPct, 0) + ",\n";
  } else {
    json += "\"curHumidity\":null,\n";
  }

  if (!isnan(cur.vpdKpa)) {
    json += "\"curVpd\":" + String(cur.vpdKpa, 1) + ",\n";
  } else {
    json += "\"curVpd\":null,\n";
  }

  json += "\"relays\":[";
  for (int i = 0; i < NUM_RELAYS; i++) {
    int state = digitalRead(relayPins[i]);
    bool on = (state == HIGH);
    json += (on ? "true" : "false");
    if (i < NUM_RELAYS - 1) json += ",";
  }
  json += "],\n";
  
  if (!shelly.main.values.ok) {
    logPrint("[API] MAIN SWITCH request not ok");
    json += "\"shellyMainSwitchStatus\":false,\n";
    json += "\"shellyMainSwitchPower\":null,\n";
    json += "\"shellyMainSwitchTotalWh\":null,\n";
  } else {
    json += "\"shellyMainSwitchStatus\":" + String(shelly.main.values.isOn ? "true" : "false") + ",\n";
    json += "\"shellyMainSwitchPower\":" + String(shelly.main.values.powerW, 2) + ",\n";
    json += "\"shellyMainSwitchTotalWh\":" + String(shelly.main.values.energyWh, 2) + ",\n";
  }

  if (!isnan(shelly.main.values.energyWh)) {
    const float mainCost = (shelly.main.values.energyWh / 1000.0f) * powerPriceKwhEur;
    json += "\"shellyMainSwitchCostEur\":" + String(mainCost, 2) + ",\n";
  } else {
    json += "\"shellyMainSwitchCostEur\":null,\n";
  } 

  if (!shelly.light.values.ok) {
    logPrint("[API] LIGHT request not ok");
    json += "\"shellyLightStatus\":false,\n";
    json += "\"shellyLightPower\":null,\n";
    json += "\"shellyLightTotalWh\":null,\n";
  } else {
    json += "\"shellyLightStatus\":" + String(shelly.light.values.isOn ? "true" : "false") + ",\n";
    json += "\"shellyLightPower\":" + String(shelly.light.values.powerW, 2) + ",\n";
    json += "\"shellyLightTotalWh\":" + String(shelly.light.values.energyWh, 2) + ",\n";
  }

  if (!isnan(shelly.light.values.energyWh)) {
    const float lightCost = (shelly.light.values.energyWh / 1000.0f) * powerPriceKwhEur;
    json += "\"shellyLightCostEur\":" + String(lightCost, 2) + ",\n";
  } else {
    json += "\"shellyLightCostEur\":null,\n";
  }

  json += "\"espFreeHeap\":" + String(ESP.getFreeHeap()) + ",\n";
  json += "\"espMinFreeHeap\":" + String(ESP.getMinFreeHeap()) + ",\n";
  json += "\"espCpuMhz\":" + String(ESP.getCpuFreqMHz()) + ",\n";
  json += "\"espUptimeS\":" + String((uint32_t)(millis() / 1000UL)) + ",\n";
  json += "\"captured\":\"" + String(timeStr)  + "\"\n";

  json += "}";

  return json;
}
#include "vars_registry.h"
#include "globals.h"
#include <esp_heap_caps.h>

// These are defined in main.cpp (kept there intentionally)
extern SensorReadings cur;
extern Targets target;
extern ShellySettings shelly;

// Average helpers are defined in your runtime module.
extern float avgTemp();
extern float avgHum();
extern float avgVPD();
extern float avgWaterTemp();

// -------------- small JSON helpers --------------
static String jBool(bool v) { return v ? "true" : "false"; }

static String jNumOrNull(float v, uint8_t dec = 2) {
  if (isnan(v) || isinf(v)) return "null";
  return String((double)v, (unsigned int)dec);
}

static String jInt(int v) { return String(v); }
static String jUInt(uint32_t v) { return String(v); }

static String jStr(const String& s) {
  String out = "\"";
  out.reserve(s.length() + 2);
  for (size_t i = 0; i < s.length(); i++) {
    char c = s[i];
    if (c == '"' || c == '\\') out += '\\';
    out += c;
  }
  out += "\"";
  return out;
}

static String jMasked() { return jStr("********"); }

static String g_buildTag() {
    String tag = "\"BUILD_";
    tag += __DATE__;  // z.B. Jan 27 2026
    tag += "_";
    tag += __TIME__;  // z.B. 14:52:10
    tag += "\"";
    return tag;
}

static String jTimeOrNull(int h, int m) {
  if (h < 0 || m < 0) return "null";
  String s = "\"";
  s += String(h);
  s += ":";
  if (m < 10) s += "0";
  s += String(m);
  s += "\"";
  return s;
}

static String jShellyLine(const ShellyDevice& d) {
  // Uses day 0 because your schedule is "same for every day"
  const DailySchedule& ds = d.schedules.days[0];

  String line;
  line.reserve(48);

  line += d.ip;
  line += " | Gen ";
  line += String(d.gen);
  line += " | ON ";

  if (ds.onHour >= 0 && ds.onMinute >= 0) {
    line += String(ds.onHour);
    line += ":";
    if (ds.onMinute < 10) line += "0";
    line += String(ds.onMinute);
  } else {
    line += "--:--";
  }

  line += " | OFF ";

  if (ds.offHour >= 0 && ds.offMinute >= 0) {
    line += String(ds.offHour);
    line += ":";
    if (ds.offMinute < 10) line += "0";
    line += String(ds.offMinute);
  } else {
    line += "--:--";
  }

  return jStr(line);
}

// -------------- getters --------------

String g_apiVersion() {
  return "1";
}

static String g_uptime() { return jUInt((uint32_t)(millis() / 1000UL)); }
static String g_heapSize() { return jUInt((uint32_t)ESP.getHeapSize()); }
static String g_heap() { return jUInt((uint32_t)ESP.getFreeHeap()); }
static String g_minheap() { return jUInt((uint32_t)ESP.getMinFreeHeap()); }
static String g_largestFreeHeapBlock() { return jUInt((uint32_t)heap_caps_get_largest_free_block(MALLOC_CAP_8BIT)); }
static String g_cpumhz() { return jUInt((uint32_t)ESP.getCpuFreqMHz()); }

static String g_wifiReady() { return jBool(wifiReady); }
static String g_espMode() { return jBool(espMode); }
static String g_ssid() { return jStr(ssidName); }
static String g_txPower() { return jNumOrNull(txPower, 1); }

static String g_temp() { return jNumOrNull(cur.temperatureC, 1); }
static String g_temp_raw() { return jNumOrNull(cur.temperatureRawC, 2); }
static String g_hum() { return jNumOrNull(cur.humidityPct, 1); }
static String g_hum_raw() { return jNumOrNull(cur.humidityRawPct, 1); }
static String g_extName() { return jStr(DS18B20Name); }
static String g_extTemp() { return jNumOrNull(cur.extTempC, 1); }
static String g_vpd() { return jNumOrNull(cur.vpdKpa, 2); }
static String g_vpd_raw() { return jNumOrNull(cur.vpdRawKpa, 2); }

static String g_reffetiveAlfaTemp() { return jNumOrNull(cur.alphaTempEffective, 2); }
static String g_reffetiveAlfaHum() { return jNumOrNull(cur.alphaHumidityEffective, 2); }

static String g_avgTemp() { return jNumOrNull(avgTemp(), 1); }
static String g_avgHum() { return jNumOrNull(avgHum(), 1); }
static String g_avgVpd() { return jNumOrNull(avgVPD(), 2); }
static String g_avgWater() { return jNumOrNull(avgWaterTemp(), 1); }

// Settings shortcuts
static String g_ui_box() { return jStr(settings.ui.boxName); }
static String g_ui_lang() { return jStr(settings.ui.language); }
static String g_ui_theme() { return jStr(settings.ui.theme); }
static String g_ui_unit() { return jStr(settings.ui.unit); }
static String g_ui_timeFmt() { return jStr(settings.ui.timeFormat); }
static String g_ui_ntp() { return jStr(settings.ui.ntpServer); }
static String g_ui_tz() { return jStr(settings.ui.tzInfo); }

static String g_grow_phase() { return jInt(settings.grow.currentPhase); }
static String g_grow_currentGrowDay() { return jInt(settings.grow.currentGrowDay); }
static String g_grow_currentGrowWeek() { return jInt(settings.grow.currentGrowWeek); }
static String g_grow_currentPhaseDay() { return jInt(settings.grow.currentPhaseDay); }
static String g_grow_currentPhaseWeek() { return jInt(settings.grow.currentPhaseWeek); }
static String g_grow_tTemp() { return jNumOrNull(settings.grow.targetTemperature, 1); }
static String g_grow_leafOff() { return jNumOrNull(settings.grow.offsetLeafTemperature, 1); }
static String g_grow_tVpd() { return jNumOrNull(settings.grow.targetVPD, 2); }
static String g_grow_minVPDMonitoring() { return jBool(settings.grow.minVpdMonEnabled); }
static String g_grow_minVPD() { return jNumOrNull(settings.grow.minVPD, 2); }
static String g_grow_vpdHysteresis() { return jNumOrNull(settings.grow.vpdHysteresis, 2); }

// Notifications (masked tokens)
static String g_notify_pushoverEnabled() { return jBool(settings.notify.pushoverEnabled); }
static String g_notify_pushoverAppKey() { return jMasked(); }
static String g_notify_pushoverUserKey() { return jMasked(); }
static String g_notify_pushoverDevice() { return jStr(settings.notify.pushoverDevice); }
static String g_notify_gotifyEnabled() { return jBool(settings.notify.gotifyEnabled); }
static String g_notify_gotifyServer() { return jStr(settings.notify.gotifyServer); }
static String g_notify_gotifyToken() { return jMasked(); }

static String g_irrigation_runs_left() { return jInt(irrigation.irrigationRuns); }
static String g_irrigation_time_left() { return jStr(irrigation.wTimeLeft); }
static String g_irrigation_amount() { return jNumOrNull(irrigation.amountOfWater, 1); }
static String g_irrigation_amount_total() { return jNumOrNull(irrigation.amount, 1); } 
static String g_irrigation_time_per_task() { return jInt(irrigation.timePerTask); }
static String g_irrigation_between_tasks() { return jInt(irrigation.betweenTasks); }
static String g_irrigation_tank_level_cm() { return jNumOrNull(tankLevelCm, 1); }
static String g_irrigation_tank_level_percent() { return jNumOrNull(tankLevel, 0); }



// Shelly settings
// For the main device we also show schedule & generation in one line
static String g_sh_main_ip() { return jStr(settings.shelly.main.ip); }
static String g_sh_main_gen() { return jInt(settings.shelly.main.gen); }
static String g_sh_main_on()  { return String("null"); }
static String g_sh_main_off() { return String("null"); }
static String g_sh_main_line(){ return jShellyLine(settings.shelly.main); }
static String g_sh_main_isOn() { return jBool(shelly.main.values.isOn); }
static String g_sh_main_Watt() { return jNumOrNull(shelly.main.values.powerW, 1); }
static String g_sh_main_Wh() { return jNumOrNull(shelly.main.values.energyWh, 1); }
static String g_sh_main_Cost() { return jNumOrNull((shelly.main.values.energyWh / 1000.0f) * powerPriceKwhEur, 2); }
// For the light devices we show schedule & generation in one line
static String g_sh_light_ip()  { return jStr(settings.shelly.light.ip); }
static String g_sh_light_gen() { return jInt(settings.shelly.light.gen); }
static String g_sh_light_on()  { return jTimeOrNull(settings.shelly.light.schedules.days[0].onHour, settings.shelly.light.schedules.days[0].onMinute); }
static String g_sh_light_off() { return jTimeOrNull(settings.shelly.light.schedules.days[0].offHour, settings.shelly.light.schedules.days[0].offMinute); }
static String g_sh_light_line(){ return jShellyLine(settings.shelly.light); }
static String g_sh_light_isOn() { return jBool(shelly.light.values.isOn); }
static String g_sh_light_Watt() { return jNumOrNull(shelly.light.values.powerW, 1); } 
static String g_sh_light_Wh() { return jNumOrNull(shelly.light.values.energyWh, 1); }
static String g_sh_light_Cost() { return jNumOrNull((shelly.light.values.energyWh / 1000.0f) * powerPriceKwhEur, 2); }
// For the humidifier we don't have schedule info, so we show generation + IP in one line
static String g_sh_hum_ip() { return jStr(settings.shelly.humidifier.ip); }
static String g_sh_hum_gen() { return jInt(settings.shelly.humidifier.gen); }
static String g_sh_hum_on()  { return String("null"); }
static String g_sh_hum_off() { return String("null"); }
static String g_sh_hum_line(){ return jShellyLine(settings.shelly.humidifier); }
static String g_sh_hum_isOn() { return jBool(shelly.humidifier.values.isOn); }
static String g_sh_hum_Watt() { return jNumOrNull(shelly.humidifier.values.powerW, 1); }
static String g_sh_hum_Wh() { return jNumOrNull(shelly.humidifier.values.energyWh, 1); }
static String g_sh_hum_Cost() { return jNumOrNull((shelly.humidifier.values.energyWh / 1000.0f) * powerPriceKwhEur, 2); }
// For the heater we also don't have schedule info, so we show generation + IP in one line
static String g_sh_heater_ip() { return jStr(settings.shelly.heater.ip); }
static String g_sh_heater_gen() { return jInt(settings.shelly.heater.gen); }
static String g_sh_heater_on()  { return String("null"); } 
static String g_sh_heater_off() { return String("null"); }
static String g_sh_heater_line(){ return jShellyLine(settings.shelly.heater); }
static String g_sh_heater_isOn() { return jBool(shelly.heater.values.isOn); }
static String g_sh_heater_Watt() { return jNumOrNull(shelly.heater.values.powerW, 1); }
static String g_sh_heater_Wh() { return jNumOrNull(shelly.heater.values.energyWh, 1); }
static String g_sh_heater_Cost() { return jNumOrNull((shelly.heater.values.energyWh / 1000.0f) * powerPriceKwhEur, 2); }
// For the fan we also don't have schedule info, so we show generation + IP in one line
static String g_sh_fan_ip() { return jStr(settings.shelly.fan.ip); }
static String g_sh_fan_gen() { return jInt(settings.shelly.fan.gen); }
static String g_sh_fan_on()  { return String("null"); }
static String g_sh_fan_off() { return String("null"); }
static String g_sh_fan_line(){ return jShellyLine(settings.shelly.fan); }
static String g_sh_fan_isOn() { return jBool(shelly.fan.values.isOn); }
static String g_sh_fan_Watt() { return jNumOrNull(shelly.fan.values.powerW, 1); }
static String g_sh_fan_Wh() { return jNumOrNull(shelly.fan.values.energyWh, 1); }
static String g_sh_fan_Cost() { return jNumOrNull((shelly.fan.values.energyWh / 1000.0f) * powerPriceKwhEur, 2); }
// For the exhaust we also don't have schedule info, so we show generation + IP in one line
static String g_sh_exhaust_ip() { return jStr(settings.shelly.exhaust.ip); }
static String g_sh_exhaust_gen() { return jInt(settings.shelly.exhaust.gen); }
static String g_sh_exhaust_on()  { return String("null"); }
static String g_sh_exhaust_off() { return String("null"); }
static String g_sh_exhaust_line(){ return jShellyLine(settings.shelly.exhaust); }
static String g_sh_exhaust_isOn() { return jBool(shelly.exhaust.values.isOn); }
static String g_sh_exhaust_Watt() { return jNumOrNull(shelly.exhaust.values.powerW, 1); }
static String g_sh_exhaust_Wh() { return jNumOrNull(shelly.exhaust.values.energyWh, 1); }
static String g_sh_exhaust_Cost() { return jNumOrNull((shelly.exhaust.values.energyWh / 1000.0f) * powerPriceKwhEur, 2); }
// For username/password we always return masked value
static String g_sh_user() { return jStr(settings.shelly.username); }
static String g_sh_pass() { return jMasked(); }

static String g_heating_relay() { return jInt(settings.heating.Relay); }

// Active relay count (4 or 8 depending on config)
static String g_active_relay_count() { return jInt(activeRelayCount); }

// Relay getters (state + name)
#define RELAY_GETTERS(i) \
  static String g_relay_state_##i() { return jBool(relayStates[i]); } \
  static String g_relay_name_##i() { return jStr(settings.relay.name[i]); } \
  static String g_relay_schen_##i() { return jBool(settings.relay.schedule[i].enabled); } \
  static String g_relay_schst_##i() { return jInt(settings.relay.schedule[i].onMin); } \
  static String g_relay_schen_##i##_end() { return jInt(settings.relay.schedule[i].offMin); }

RELAY_GETTERS(0)
RELAY_GETTERS(1)
RELAY_GETTERS(2)
RELAY_GETTERS(3)
RELAY_GETTERS(4)

// For relays 5-7 we only have state + name, no schedule (yet)
static String g_relay_state_5() { return jBool(relayStates[5]); }
static String g_relay_name_5() { return jStr(settings.pump1); }
static String g_relay_schen_5() { return jBool(false); }
static String g_relay_schst_5() { return String("null"); }
static String g_relay_schen_5_end() { return String("null"); }

static String g_relay_state_6() { return jBool(relayStates[6]); }
static String g_relay_name_6() { return jStr(settings.pump2); }
static String g_relay_schen_6() { return jBool(false); }
static String g_relay_schst_6() { return String("null"); }
static String g_relay_schen_6_end() { return String("null"); }

static String g_relay_state_7() { return jBool(relayStates[7]); }
static String g_relay_name_7() { return jStr(settings.pump3); }
static String g_relay_schen_7() { return jBool(false); }
static String g_relay_schst_7() { return String("null"); }
static String g_relay_schen_7_end() { return String("null"); }

const VarItem VARS[] = {
  {"api.version", g_apiVersion, false, "api"},
  {"debug.buildTag", g_buildTag, false, "debug"},
  // --- system ---
  {"sys.heapSize", g_heapSize, false, "system"},
  {"sys.uptimeS", g_uptime, false, "system"},
  {"sys.freeHeap", g_heap, false, "system"},
  {"sys.minFreeHeap", g_minheap, false, "system"},
  {"sys.largestFreeHeapBlock", g_largestFreeHeapBlock, false, "system"},
  {"sys.cpuMhz", g_cpumhz, false, "system"},

  // --- wifi ---
  {"wifi.ready", g_wifiReady, false, "wifi"},
  {"wifi.apMode", g_espMode, false, "wifi"},
  {"wifi.ssid", g_ssid, false, "wifi"},
  {"wifi.txPowerDB", g_txPower, false, "wifi"},

  // --- sensors ---
  {"sensors.cur.temperatureC", g_temp, false, "sensors"},
  {"sensors.cur.temperatureRawC", g_temp_raw, false, "sensors"},
  {"sensors.cur.humidityPct", g_hum, false, "sensors"},
  {"sensors.cur.humidityRawPct", g_hum_raw, false, "sensors"},
  {"sensors.cur.vpdKpa", g_vpd, false, "sensors"},
  {"sensors.cur.vpdRawKpa", g_vpd_raw, false, "sensors"},
  {"sensors.cur.ds18b20Name", g_extName, false, "sensors"},
  {"sensors.cur.extTempC", g_extTemp, false, "sensors"},
  {"sensors.cur.vpdKpa", g_vpd, false, "sensors"},
  
  {"sensors.cur.effectiveAlfaTempC", g_reffetiveAlfaTemp, false, "sensors"},
  {"sensors.cur.effectiveAlfaHumPct", g_reffetiveAlfaHum, false, "sensors"},

  // --- settings.ui ---
  {"settings.ui.boxName", g_ui_box, false, "settings.ui"},
  {"settings.ui.language", g_ui_lang, false, "settings.ui"},
  {"settings.ui.theme", g_ui_theme, false, "settings.ui"},
  {"settings.ui.unit", g_ui_unit, false, "settings.ui"},
  {"settings.ui.timeFormat", g_ui_timeFmt, false, "settings.ui"},
  {"settings.ui.ntpServer", g_ui_ntp, false, "settings.ui"},
  {"settings.ui.tzInfo", g_ui_tz, false, "settings.ui"},

  // --- settings.grow ---
  {"settings.grow.currentPhase", g_grow_phase, false, "settings.grow"},
  {"settings.grow.currentGrowDay", g_grow_currentGrowDay, false, "settings.grow"},
  {"settings.grow.currentGrowWeek", g_grow_currentGrowWeek, false, "settings.grow"},
  {"settings.grow.currentPhaseDay", g_grow_currentPhaseDay, false, "settings.grow"},
  {"settings.grow.currentPhaseWeek", g_grow_currentPhaseWeek, false, "settings.grow"},
  {"settings.grow.targetTemperature", g_grow_tTemp, false, "settings.grow"},
  {"settings.grow.offsetLeafTemperature", g_grow_leafOff, false, "settings.grow"},
  {"settings.grow.targetVPD", g_grow_tVpd, false, "settings.grow"},
  {"settings.grow.minVPDMonitoring", g_grow_minVPDMonitoring, false, "settings.grow"},
  {"settings.grow.minVPD", g_grow_minVPD, false, "settings.grow"},
  {"settings.grow.vpdHysteresis", g_grow_vpdHysteresis, false, "settings.grow"},

  // --- notifications ---
  {"settings.notify.pushoverEnabled", g_notify_pushoverEnabled, false, "settings.notify"},
  {"settings.notify.pushoverAppKey", g_notify_pushoverAppKey, true, "settings.notify"},
  {"settings.notify.pushoverUserKey", g_notify_pushoverUserKey, true, "settings.notify"},
  {"settings.notify.pushoverDevice", g_notify_pushoverDevice, false, "settings.notify"},
  {"settings.notify.gotifyEnabled", g_notify_gotifyEnabled, false, "settings.notify"},
  {"settings.notify.gotifyServer", g_notify_gotifyServer, false, "settings.notify"},
  {"settings.notify.gotifyToken", g_notify_gotifyToken, true, "settings.notify"},

  // --- shelly ---
  {"settings.shelly.main.ip", g_sh_main_ip, false, "settings.shelly"},
  {"settings.shelly.main.gen", g_sh_main_gen, false, "settings.shelly"},
  {"settings.shelly.main.on",  g_sh_main_on,  false, "settings.shelly"},
  {"settings.shelly.main.off", g_sh_main_off, false, "settings.shelly"},
  {"settings.shelly.main.line", g_sh_main_line, false, "settings.shelly"},
  {"cur.shelly.main.isOn", g_sh_main_isOn, false, "settings.shelly"},
  {"cur.shelly.main.Watt", g_sh_main_Watt, false, "settings.shelly"},
  {"cur.shelly.main.Wh", g_sh_main_Wh, false, "settings.shelly"},
  {"cur.shelly.main.Cost", g_sh_main_Cost, false, "settings.shelly"},
  {"settings.shelly.light.ip", g_sh_light_ip, false, "settings.shelly"},
  {"settings.shelly.light.gen", g_sh_light_gen, false, "settings.shelly"},
  {"settings.shelly.light.on",  g_sh_light_on,  false, "settings.shelly"},
  {"settings.shelly.light.off", g_sh_light_off, false, "settings.shelly"},
  {"settings.shelly.light.line", g_sh_light_line, false, "settings.shelly"},
  {"cur.shelly.light.isOn", g_sh_light_isOn, false, "settings.shelly"},
  {"cur.shelly.light.Watt", g_sh_light_Watt, false, "settings.shelly"},
  {"cur.shelly.light.Wh", g_sh_light_Wh, false, "settings.shelly"},
  {"cur.shelly.light.Cost", g_sh_light_Cost, false, "settings.shelly"},
  {"settings.shelly.humidifier.ip", g_sh_hum_ip, false, "settings.shelly"},
  {"settings.shelly.humidifier.gen", g_sh_hum_gen, false, "settings.shelly"},
  {"settings.shelly.humidifier.on",  g_sh_hum_on,  false, "settings.shelly"},
  {"settings.shelly.humidifier.off", g_sh_hum_off, false, "settings.shelly"},
  {"settings.shelly.humidifier.line", g_sh_hum_line, false, "settings.shelly"},
  {"cur.shelly.humidifier.isOn", g_sh_hum_isOn, false, "settings.shelly"},
  {"cur.shelly.humidifier.Watt", g_sh_hum_Watt, false, "settings.shelly"},
  {"cur.shelly.humidifier.Wh", g_sh_hum_Wh, false, "settings.shelly"},
  {"cur.shelly.humidifier.Cost", g_sh_hum_Cost, false, "settings.shelly"},
  {"settings.shelly.heater.ip", g_sh_heater_ip, false, "settings.shelly"},
  {"settings.shelly.heater.gen", g_sh_heater_gen, false, "settings.shelly"},
  {"settings.shelly.heater.on",  g_sh_heater_on,  false, "settings.shelly"},
  {"settings.shelly.heater.off", g_sh_heater_off, false, "settings.shelly"},
  {"settings.shelly.heater.line", g_sh_heater_line, false, "settings.shelly"},
  {"cur.shelly.heater.isOn", g_sh_heater_isOn, false, "settings.shelly"},
  {"cur.shelly.heater.Watt", g_sh_heater_Watt, false, "settings.shelly"},
  {"cur.shelly.heater.Wh", g_sh_heater_Wh, false, "settings.shelly"},
  {"cur.shelly.heater.Cost", g_sh_heater_Cost, false, "settings.shelly"},
  {"settings.shelly.fan.ip", g_sh_fan_ip, false, "settings.shelly"},
  {"settings.shelly.fan.gen", g_sh_fan_gen, false, "settings.shelly"},
  {"settings.shelly.fan.on",  g_sh_fan_on,  false, "settings.shelly"},
  {"settings.shelly.fan.off", g_sh_fan_off, false, "settings.shelly"},
  {"settings.shelly.fan.line", g_sh_fan_line, false, "settings.shelly"},
  {"cur.shelly.fan.isOn", g_sh_fan_isOn, false, "settings.shelly"},
  {"cur.shelly.fan.Watt", g_sh_fan_Watt, false, "settings.shelly"},
  {"cur.shelly.fan.Wh", g_sh_fan_Wh, false, "settings.shelly"},
  {"cur.shelly.fan.Cost", g_sh_fan_Cost, false, "settings.shelly"},
  {"settings.shelly.exhaust.ip", g_sh_exhaust_ip, false, "settings.shelly"},
  {"settings.shelly.exhaust.gen", g_sh_exhaust_gen, false, "settings.shelly"},
  {"settings.shelly.exhaust.on",  g_sh_exhaust_on,  false, "settings.shelly"},
  {"settings.shelly.exhaust.off", g_sh_exhaust_off, false, "settings.shelly"},
  {"settings.shelly.exhaust.line", g_sh_exhaust_line, false, "settings.shelly"},
  {"cur.shelly.exhaust.isOn", g_sh_exhaust_isOn, false, "settings.shelly"},
  {"cur.shelly.exhaust.Watt", g_sh_exhaust_Watt, false, "settings.shelly"},
  {"cur.shelly.exhaust.Wh", g_sh_exhaust_Wh, false, "settings.shelly"},
  {"cur.shelly.exhaust.Cost", g_sh_exhaust_Cost, false, "settings.shelly"},
  {"settings.shelly.username", g_sh_user, false, "settings.shelly"},
  {"settings.shelly.password", g_sh_pass, true, "settings.shelly"},

  // --- heating relay ---
  {"settings.heatingrelay", g_heating_relay, false, "settingsheating"},

  {"settings.active_relay_count", g_active_relay_count, false, "settingsheating"},

  // --- relays (names + states + schedule) ---
  {"relays[0].name", g_relay_name_0, false, "relays"},
  {"relays[0].state", g_relay_state_0, false, "relays"},
  {"relays[0].schedule.enabled", g_relay_schen_0, false, "relays"},
  {"relays[0].schedule.start", g_relay_schst_0, false, "relays"},
  {"relays[0].schedule.end", g_relay_schen_0_end, false, "relays"},

  {"relays[1].name", g_relay_name_1, false, "relays"},
  {"relays[1].state", g_relay_state_1, false, "relays"},
  {"relays[1].schedule.enabled", g_relay_schen_1, false, "relays"},
  {"relays[1].schedule.start", g_relay_schst_1, false, "relays"},
  {"relays[1].schedule.end", g_relay_schen_1_end, false, "relays"},

  {"relays[2].name", g_relay_name_2, false, "relays"},
  {"relays[2].state", g_relay_state_2, false, "relays"},
  {"relays[2].schedule.enabled", g_relay_schen_2, false, "relays"},
  {"relays[2].schedule.start", g_relay_schst_2, false, "relays"},
  {"relays[2].schedule.end", g_relay_schen_2_end, false, "relays"},

  {"relays[3].name", g_relay_name_3, false, "relays"},
  {"relays[3].state", g_relay_state_3, false, "relays"},
  {"relays[3].schedule.enabled", g_relay_schen_3, false, "relays"},
  {"relays[3].schedule.start", g_relay_schst_3, false, "relays"},
  {"relays[3].schedule.end", g_relay_schen_3_end, false, "relays"},

  {"relays[4].name", g_relay_name_4, false, "relays"},
  {"relays[4].state", g_relay_state_4, false, "relays"},
  {"relays[4].schedule.enabled", g_relay_schen_4, false, "relays"},
  {"relays[4].schedule.start", g_relay_schst_4, false, "relays"},
  {"relays[4].schedule.end", g_relay_schen_4_end, false, "relays"},

  {"relays[5].name", g_relay_name_5, false, "relays"},
  {"relays[5].state", g_relay_state_5, false, "relays"},

  {"relays[6].name", g_relay_name_6, false, "relays"},
  {"relays[6].state", g_relay_state_6, false, "relays"},

  {"relays[7].name", g_relay_name_7, false, "relays"},
  {"relays[7].state", g_relay_state_7, false, "relays"},

  // --- irrigation ---
  {"irrigation.runsLeft", g_irrigation_runs_left, false, "irrigation"},
  {"irrigation.timeLeft", g_irrigation_time_left, false, "irrigation"},
  {"irrigation.amount", g_irrigation_amount, false, "irrigation"},
  {"irrigation.timePerTask", g_irrigation_time_per_task, false, "irrigation"},
  {"irrigation.betweenTasks", g_irrigation_between_tasks, false, "irrigation"},
  {"irrigation.amountTotal", g_irrigation_amount_total, false, "irrigation"},
  {"irrigation.tankLevelCm", g_irrigation_tank_level_cm, false, "irrigation"},
  {"irrigation.tankLevelPercent", g_irrigation_tank_level_percent, false, "irrigation"},

};

const size_t VARS_COUNT = sizeof(VARS) / sizeof(VARS[0]);

// main.cpp (optimized fast boot)

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <WebServer.h>
#include <Preferences.h>
#include <time.h>
#include <LittleFS.h>
#include <deque>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// global, functions, html code, js code and css code includes
#include "globals.h"
#include "function.h"
#include "helper.h"
#include "index_html.h"
#include "style_css.h"
#include "java_script.h"
#include "ota.h"

// Debug variable registry ("Variables" page)
#include "vars_registry.h"

// tasks
#include "task_Check_Sensor.h"
#include "task_CheckShellyStatus.h"
#include "task_Watering.h"

extern "C" void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
  Serial.printf("\n*** STACK OVERFLOW in task: %s ***\n", pcTaskName ? pcTaskName : "(null)");
  Serial.flush();
  esp_restart();
}

Preferences preferences;
WebServer server(80);

// define weblog buffer variable
std::deque<String> logBuffer;

// ShellySettings is still used as a global across runtime.h/task headers
ShellySettings shelly;

// BME280 and DS18B20 sensor
SensorReadings cur;
Targets target;

// Global OneWire + DallasTemperature instance for DS18B20
OneWire oneWire(DS18B20_PIN);

// Definition for the extern declared in runtime.h
DallasTemperature sensors(&oneWire);

// Task handle for sensor task
TaskHandle_t sensorTaskHandle = nullptr;

// Forward declarations
void startSoftAP();

// -------------------- Grow Diary (CSV in LittleFS) --------------------
static const char* DIARY_PATH = "/growdiary.csv";

// -------------------- LittleFS recovery API --------------------
// OTA-safe: normal boot never formats LittleFS. Formatting is only possible
// through explicit user-triggered endpoints with a confirmation token.
static bool readFsFormatConfirmation() {
  String confirm;

  if (server.hasArg("confirm")) confirm = server.arg("confirm");

  if (!confirm.length() && server.hasArg("plain") && server.arg("plain").length()) {
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, server.arg("plain"));
    if (!err) confirm = (const char*)(doc["confirm"] | "");
  }

  confirm.trim();
  return confirm == "FORMAT_LITTLEFS";
}

static void sendFsConfirmationRequired() {
  server.send(403, "application/json; charset=utf-8",
              "{\"ok\":false,\"err\":\"confirmation_required\",\"confirm\":\"FORMAT_LITTLEFS\"}");
}

static void handleFsStatus() {
  server.sendHeader("Cache-Control", "no-store");

  // Status endpoint must not trigger another mount attempt.
  // Mount is checked once at boot; failure is cached until manual recovery.
  const bool ok = isFsMounted();
  String json = "{\"ok\":";
  json += ok ? "true" : "false";
  json += ",\"mountTried\":";
  json += fsMountWasTried() ? "true" : "false";

  if (ok) {
    json += ",\"mounted\":true";
    json += ",\"total\":" + String((uint32_t)LittleFS.totalBytes());
    json += ",\"used\":" + String((uint32_t)LittleFS.usedBytes());
    json += ",\"diaryExists\":";
    json += LittleFS.exists(DIARY_PATH) ? "true" : "false";
  } else {
    json += ",\"mounted\":false";
    json += ",\"mountFailed\":";
    json += fsMountFailed() ? "true" : "false";
    json += ",\"err\":\"fs_not_mounted\"";
  }

  json += "}";
  server.send(ok ? 200 : 503, "application/json; charset=utf-8", json);
}

static void handleFsFormat() {
  server.sendHeader("Cache-Control", "no-store");

  // Intentional friction: this endpoint erases all LittleFS files, including growdiary.csv.
  if (!readFsFormatConfirmation()) {
    sendFsConfirmationRequired();
    return;
  }

  const bool ok = formatFsForRecovery();
  if (!ok) {
    server.send(500, "application/json; charset=utf-8", "{\"ok\":false,\"err\":\"format_failed\"}");
    return;
  }

  server.send(200, "application/json; charset=utf-8", "{\"ok\":true,\"formatted\":true,\"mounted\":true}");
}


// Parse "YYYY-MM-DD" -> time_t (local midnight). Returns 0 if invalid.
static time_t parseYmdToLocalMidnight(const String& ymd) {
  if (ymd.length() < 10) return 0;
  const int y = ymd.substring(0, 4).toInt();
  const int m = ymd.substring(5, 7).toInt();
  const int d = ymd.substring(8, 10).toInt();
  if (y < 1970 || m < 1 || m > 12 || d < 1 || d > 31) return 0;

  struct tm tmv {};
  tmv.tm_year = y - 1900;
  tmv.tm_mon  = m - 1;
  tmv.tm_mday = d;
  tmv.tm_hour = 0;
  tmv.tm_min  = 0;
  tmv.tm_sec  = 0;
  tmv.tm_isdst = -1; // let libc determine DST

  // mktime interprets tm as local time
  return mktime(&tmv);
}

static int daysSince(const String& startYmd, time_t nowLocal) {
  const time_t start = parseYmdToLocalMidnight(startYmd);
  if (start <= 0 || nowLocal <= 0) return -1;

  const double diffSec = difftime(nowLocal, start);
  const int diffDays = (int)floor(diffSec / 86400.0);
  return diffDays;
}

// CSV-safe: remove newlines and escape quotes by doubling them.
static String csvEscape(const String& in) {
  String s = in;
  s.replace("\r", " ");
  s.replace("\n", " ");
  s.replace("\"", "\"\"");
  return "\"" + s + "\"";
}

// CSV: split a line into fields (supports quotes and commas inside quoted fields)
static bool csvSplitLine(const String& line, String* out, int outMax, int& outCount) {
  outCount = 0;
  bool inQuotes = false;
  String cur;
  cur.reserve(line.length());

  for (size_t i = 0; i < (size_t)line.length(); i++) {
    const char c = line[i];
    if (c == '"') {
      // doubled quote inside quoted field -> literal quote
      if (inQuotes && (i + 1 < (size_t)line.length()) && line[i + 1] == '"') {
        cur += '"';
        i++;
      } else {
        inQuotes = !inQuotes;
      }
      continue;
    }
    if (c == ',' && !inQuotes) {
      if (outCount < outMax) out[outCount] = cur;
      outCount++;
      cur = "";
      continue;
    }
    cur += c;
  }

  if (outCount < outMax) out[outCount] = cur;
  outCount++;
  return outCount > 0;
}

// CSV: remove surrounding quotes already handled by csvSplitLine; trim and keep as-is
String csvFieldToString(const String& s) {
  String out = s;
  out.trim();
  return out;
}

// ---- Grow Diary storage helpers ----
// We store each diary row with a stable numeric id in column 0.
// New schema (8 fields):
//   id, tsLocal, phase, growDay, growWeek, phaseDay, phaseWeek, note
// Old schema (7 fields) without id is migrated on first access.

static uint32_t fnv1a32(const String& s) {
  uint32_t h = 2166136261u;
  for (size_t i = 0; i < (size_t)s.length(); i++) {
    h ^= (uint8_t)s[i];
    h *= 16777619u;
  }
  return h;
}

static bool ensureDiaryHasId() {
  if (!ensureFsMounted()) return false;
  if (!LittleFS.exists(DIARY_PATH)) return true;

  File f = LittleFS.open(DIARY_PATH, FILE_READ);
  if (!f) return false;

  String first = f.readStringUntil('\n');
  first.trim();
  f.close();

  // Quick check: if first field looks like a numeric id and there are 8 fields, we're done.
  String fields[12]; int n = 0;
  int cnt = 0;
  if (!csvSplitLine(first, fields, 12, cnt)) return true; // empty file is fine

  // If file starts with a proper header "id,ts_local,...", it's already new-format.
  if (first.startsWith("id,")) return true;

  if (cnt >= 8) {
    const String id0 = csvFieldToString(fields[0]);
    bool allDigits = id0.length() > 0;
    for (size_t i = 0; i < (size_t)id0.length(); i++) {
      if (id0[i] < '0' || id0[i] > '9') { allDigits = false; break; }
    }
    if (allDigits) return true;
  }

  // Migrate old 7-field rows -> prepend id
  File in = LittleFS.open(DIARY_PATH, FILE_READ);
  if (!in) return false;

  const char* TMP = "/growdiary.tmp";
  if (LittleFS.exists(TMP)) LittleFS.remove(TMP);
  File out = LittleFS.open(TMP, FILE_WRITE);
  if (!out) { in.close(); return false; }

  uint32_t lineNo = 0;
  while (in.available()) {
    String line = in.readStringUntil('\n');
    line.trim();
    if (!line.length()) continue;

    String flds[12]; int c = 0;
    if (!csvSplitLine(line, flds, 12, c) || c < 7) continue;

    // If this row already looks like new format (>=8 fields AND first field numeric) -> keep as-is
    if (c >= 8) {
      String id0 = csvFieldToString(flds[0]);
      bool allDigits = id0.length() > 0;
      for (size_t i = 0; i < (size_t)id0.length(); i++) {
        if (id0[i] < '0' || id0[i] > '9') { allDigits = false; break; }
      }
      if (allDigits) {
        out.println(line);
        continue;
      }
    }

    const String tsLocal = csvFieldToString(flds[0]);
    const String phase   = csvFieldToString(flds[1]);
    const String note    = csvFieldToString(flds[6]);

    uint32_t id = fnv1a32(tsLocal + "|" + phase + "|" + note + "|" + String(lineNo++));
    String row;
    row.reserve(line.length() + 16);
    row += String(id);
    row += ",";
    row += line; // rest of old row
    out.println(row);
  }
  in.close();
  out.close();

  // Replace old file
  LittleFS.remove(DIARY_PATH);
  return LittleFS.rename(TMP, DIARY_PATH);
}

static bool rewriteDiaryWithMutation(const String& idStr, bool doDelete, const String& newPhase, const String& newNote) {
  if (!ensureDiaryHasId()) return false;
  if (!LittleFS.exists(DIARY_PATH)) return false;

  File in = LittleFS.open(DIARY_PATH, FILE_READ);
  if (!in) return false;

  const char* TMP = "/growdiary.tmp";
  if (LittleFS.exists(TMP)) LittleFS.remove(TMP);
  File out = LittleFS.open(TMP, FILE_WRITE);
  if (!out) { in.close(); return false; }

  bool changed = false;
  while (in.available()) {
    String line = in.readStringUntil('\n');
    line.trim();
    if (!line.length()) continue;

    String flds[12]; int c = 0;
    if (!csvSplitLine(line, flds, 12, c) || c < 8) {
      out.println(line);
      continue;
    }

    const String rid = csvFieldToString(flds[0]);
    if (rid != idStr) {
      out.println(line);
      continue;
    }

    // matched id
    if (doDelete) {
      changed = true;
      continue; // skip line
    }

    // Update: keep all columns except phase (col 2) and note (col 7)
    // Schema: 0 id, 1 tsLocal, 2 phase, 3 growDay, 4 growWeek, 5 phaseDay, 6 phaseWeek, 7 note
    String row;
    row.reserve(line.length() + 16);
    row += rid; row += ",";
    row += csvEscape(csvFieldToString(flds[1])); row += ","; // tsLocal (re-escaped to normalize)
    row += csvEscape(newPhase.length() ? newPhase : csvFieldToString(flds[2])); row += ",";
    row += csvFieldToString(flds[3]); row += ",";
    row += csvFieldToString(flds[4]); row += ",";
    row += csvFieldToString(flds[5]); row += ",";
    row += csvFieldToString(flds[6]); row += ",";
    row += csvEscape(newNote.length() ? newNote : csvFieldToString(flds[7]));
    out.println(row);
    changed = true;
  }

  in.close();
  out.close();

  if (!changed) {
    LittleFS.remove(TMP);
    return false;
  }

  LittleFS.remove(DIARY_PATH);
  return LittleFS.rename(TMP, DIARY_PATH);
}



// Parse local timestamp "YYYY-MM-DD HH:MM:SS" -> unix seconds (local time). Returns 0 on failure.
static time_t parseLocalTimestamp(const String& tsLocal) {
  if (tsLocal.length() < 19) return 0;
  const int y = tsLocal.substring(0, 4).toInt();
  const int m = tsLocal.substring(5, 7).toInt();
  const int d = tsLocal.substring(8, 10).toInt();
  const int hh = tsLocal.substring(11, 13).toInt();
  const int mm = tsLocal.substring(14, 16).toInt();
  const int ss = tsLocal.substring(17, 19).toInt();
  if (y < 1970 || m < 1 || m > 12 || d < 1 || d > 31) return 0;
  if (hh < 0 || hh > 23 || mm < 0 || mm > 59 || ss < 0 || ss > 59) return 0;

  struct tm tmv {};
  tmv.tm_year = y - 1900;
  tmv.tm_mon  = m - 1;
  tmv.tm_mday = d;
  tmv.tm_hour = hh;
  tmv.tm_min  = mm;
  tmv.tm_sec  = ss;
  tmv.tm_isdst = -1;
  return mktime(&tmv);
}

// GET /api/diary/list  -> returns JSON list for UI
static void handleDiaryList() {
  server.sendHeader("Cache-Control", "no-store");
  if (!ensureFsMounted() || !LittleFS.exists(DIARY_PATH)) {
    server.send(200, "application/json; charset=utf-8", "{\"items\":[]}");
    return;
  }

  ensureDiaryHasId();

  File f = LittleFS.open(DIARY_PATH, FILE_READ);
  if (!f) {
    server.send(500, "application/json; charset=utf-8", "{\"items\":[],\"err\":\"open\"}");
    return;
  }

  // We keep the last N entries to avoid large RAM usage
  const int MAX_ITEMS = 50;
  String items[MAX_ITEMS];
  int count = 0;

  bool firstLine = true;
  while (f.available()) {
    String line = f.readStringUntil('\n');
    line.trim();
    if (!line.length()) continue;
    if (firstLine) { firstLine = false; continue; } // skip header

    // Parse fields
    String fields[8];
    int n = 0;
    int outCount = 0;
    csvSplitLine(line, fields, 8, outCount);
    if (outCount < 8) continue; // malformed

    const String diaryId = csvFieldToString(fields[0]);
    const String tsLocal = csvFieldToString(fields[1]);
    const String phase   = csvFieldToString(fields[2]);
    const String note    = csvFieldToString(fields[7]);
    const time_t ts = parseLocalTimestamp(tsLocal);

    String preview = note;
    preview.trim();

    // Build JSON object string (escape quotes/backslashes minimally)
    auto jEsc = [](const String& in) -> String {
      String s = in;
      s.replace("\\", "\\\\");
      s.replace("\"", "\\\"");
      s.replace("\r", " ");
      s.replace("\n", " ");
      return s;
    };

    String obj;
    obj.reserve(256);
    obj += "{\"id\":\"" + jEsc(diaryId) + "\",\"date\":\"" + jEsc(tsLocal) + "\"";
    if (ts > 0) obj += ",\"ts\":" + String((uint32_t)ts);
    if (phase.length()) obj += ",\"phase\":\"" + jEsc(phase) + "\"";
    if (note.length())  obj += ",\"note\":\""  + jEsc(note)  + "\"";
    if (preview.length()) obj += ",\"preview\":\"" + jEsc(preview) + "\"";
    obj += "}";

    // ring buffer: keep last MAX_ITEMS lines
    if (count < MAX_ITEMS) {
      items[count++] = obj;
    } else {
      for (int i = 1; i < MAX_ITEMS; i++) items[i - 1] = items[i];
      items[MAX_ITEMS - 1] = obj;
    }
  }
  f.close();

  String json;
  json.reserve(2048);
  json += "{\"items\":[";
  for (int i = 0; i < count; i++) {
    if (i) json += ',';
    json += items[i];
  }
  json += "]}";

  server.send(200, "application/json; charset=utf-8", json);
}

// POST /api/diary/add
// Accepts either JSON body {"note":"...","phase":"grow|flower|dry"} or form fields note=...&phase=...
static void handleDiaryAdd() {
  // --- read note + optional phase ---
  String note;
  String phaseStr;
  String lang;
  String metaStr;
  if (server.hasArg("plain") && server.arg("plain").length()) {
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, server.arg("plain"));
    if (!err) {
      note = (const char*) (doc["note"] | "");
      phaseStr = (const char*) (doc["phase"] | "");
      lang = (const char*) (doc["lang"] | "");
      metaStr = (const char*) (doc["meta"] | "");
    }
  }
  if (!note.length() && server.hasArg("note")) note = server.arg("note");
  if (!phaseStr.length() && server.hasArg("phase")) phaseStr = server.arg("phase");
  if (!lang.length() && server.hasArg("lang")) lang = server.arg("lang");
  if (!metaStr.length() && server.hasArg("meta")) metaStr = server.arg("meta");
  note.trim();
  if (note.length() > 400) note = note.substring(0, 400);

  // Determine current phase (fallback to global settings.grow.currentPhase if not provided)
  int phase = settings.grow.currentPhase;
  if (phaseStr == "grow") phase = 1;
  else if (phaseStr == "flower") phase = 2;
  else if (phaseStr == "dry") phase = 3;

  const char* phaseCsv = (phase == 2) ? "flower" : (phase == 3) ? "dry" : "grow";

  // --- compute day/week for total grow and current phase ---
  time_t nowT = time(nullptr);
  struct tm nowLocalTm {};
  localtime_r(&nowT, &nowLocalTm);

  // normalize "now" to local midnight for stable day counts
  struct tm midnightTm = nowLocalTm;
  midnightTm.tm_hour = 0; midnightTm.tm_min = 0; midnightTm.tm_sec = 0;
  time_t nowLocalMidnight = mktime(&midnightTm);

  const int growDiff = daysSince(startDate, nowLocalMidnight);
  const int growDay  = (growDiff >= 0) ? (growDiff + 1) : 0;
  const int growWeek = (growDay > 0) ? ((growDay - 1) / 7 + 1) : 0;

  String phaseStart = startDate;
  if (phase == 2 && startFlowering.length() >= 10) phaseStart = startFlowering;
  if (phase == 3 && startDrying.length()   >= 10) phaseStart = startDrying;

  const int phaseDiff = daysSince(phaseStart, nowLocalMidnight);
  const int phaseDay  = (phaseDiff >= 0) ? (phaseDiff + 1) : 0;
  const int phaseWeek = (phaseDay > 0) ? ((phaseDay - 1) / 7 + 1) : 0;
  // --- enrich note with current grow status (localized) ---
  String meta = metaStr;
  meta.trim();

  if (!meta.length()) {
    // Fallback to backend computation if frontend didn't send meta
    String langCode = lang;
    langCode.toLowerCase();
    if (!(langCode == "en")) langCode = "de";

    String phaseName = phaseStr;
    phaseName.toLowerCase();

    auto phaseLabel = [&](const String& code) -> String {
      const String c = String(code);
      if (langCode == "en") {
        if (c == "flower") return "Flower";
        if (c == "dry")    return "Dry";
        return "Grow";
      } else {
        if (c == "flower") return "Blüte";
        if (c == "dry")    return "Trocknen";
        return "Wachstum";
      }
    };

    const String wLabel = (langCode == "en") ? "Week" : "Woche";
    const String dLabel = (langCode == "en") ? "Day"  : "Tag";

    meta.reserve(96);
    meta += "Grow: ";
    meta += wLabel + " " + String(growWeek) + " " + dLabel + " " + String(growDay);
    meta += " | ";
    meta += phaseLabel(phaseName) + ": ";
    meta += wLabel + " " + String(phaseWeek) + " " + dLabel + " " + String(phaseDay);
  }

  if (meta.length()) {
    if (note.length()) note = "(" + meta + ")  " + note;
    else note = meta;
  }

  // --- timestamp ---
  char tsBuf[32];
  // ISO-like local time: YYYY-MM-DD HH:MM:SS
  snprintf(tsBuf, sizeof(tsBuf), "%04d-%02d-%02d %02d:%02d:%02d",
           nowLocalTm.tm_year + 1900, nowLocalTm.tm_mon + 1, nowLocalTm.tm_mday,
           nowLocalTm.tm_hour, nowLocalTm.tm_min, nowLocalTm.tm_sec);

  // --- write file ---
  if (!ensureFsMounted()) {
    server.send(503, "application/json; charset=utf-8", "{\"ok\":false,\"err\":\"fs_not_mounted\"}");
    return;
  }

  // Ensure schema (adds stable id column for edit/delete)
  ensureDiaryHasId();

  const bool exists = LittleFS.exists(DIARY_PATH);
  File f = LittleFS.open(DIARY_PATH, FILE_APPEND);
  if (!f) {
    server.send(500, "application/json; charset=utf-8", "{\"ok\":false,\"err\":\"open\"}");
    return;
  }

  // Write header once
  if (!exists || f.size() == 0) {
    f.println("id,ts_local,phase,grow_day,grow_week,phase_day,phase_week,note");
  }

  // CSV row
  // note is quoted/escaped
  String row;
  row.reserve(528);
  // Stable numeric id: epoch seconds mixed with millis (avoids duplicates within one second)
  const uint32_t diaryId = ((uint32_t)time(nullptr) << 6) ^ (uint32_t)(millis() & 0x3Fu);
  row += String(diaryId);
  row += ",";
  row += tsBuf;
  row += ",";
  row += phaseCsv;
  row += ",";
  row += String(growDay);
  row += ",";
  row += String(growWeek);
  row += ",";
  row += String(phaseDay);
  row += ",";
  row += String(phaseWeek);
  row += ",";
  row += csvEscape(note);

  f.println(row);
  f.close();

  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json; charset=utf-8", "{\"ok\":true}");
}


// POST /api/diary/delete
// Accepts JSON {"id":"123"} or form id=123
static void handleDiaryDelete() {
  String id;
  if (server.hasArg("plain") && server.arg("plain").length()) {
    JsonDocument doc;
    if (!deserializeJson(doc, server.arg("plain"))) {
      id = (const char*)(doc["id"] | "");
    }
  }
  if (!id.length() && server.hasArg("id")) id = server.arg("id");
  id.trim();
  if (!id.length()) {
    server.send(400, "application/json; charset=utf-8", "{\"ok\":false,\"err\":\"id\"}");
    return;
  }

  const bool ok = rewriteDiaryWithMutation(id, true, "", "");
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json; charset=utf-8", ok ? "{\"ok\":true}" : "{\"ok\":false}");
}

// POST /api/diary/update
// Accepts JSON {"id":"123","note":"...","phase":"grow|flower|dry"} or form fields
static void handleDiaryUpdate() {
  String id, note, phaseStr;

  if (server.hasArg("plain") && server.arg("plain").length()) {
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, server.arg("plain"));
    if (!err) {
      id = (const char*) (doc["id"] | "");
      note = (const char*) (doc["note"] | "");
      phaseStr = (const char*) (doc["phase"] | "");
    }
  }
  if (!id.length() && server.hasArg("id")) id = server.arg("id");
  if (!note.length() && server.hasArg("note")) note = server.arg("note");
  if (!phaseStr.length() && server.hasArg("phase")) phaseStr = server.arg("phase");

  id.trim(); note.trim(); phaseStr.trim();
  if (!id.length()) {
    server.send(400, "application/json; charset=utf-8", "{\"ok\":false,\"err\":\"id\"}");
    return;
  }
  if (!note.length() && !phaseStr.length()) {
    server.send(400, "application/json; charset=utf-8", "{\"ok\":false,\"err\":\"empty\"}");
    return;
  }

  // Normalize phase to the same labels used elsewhere
  String phaseNorm = phaseStr;
  phaseNorm.toLowerCase();
  if (!(phaseNorm == "grow" || phaseNorm == "flower" || phaseNorm == "dry")) {
    // allow empty to mean "keep existing"
    phaseNorm = "";
  }

  const bool ok = rewriteDiaryWithMutation(id, false, phaseNorm, note);
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json; charset=utf-8", ok ? "{\"ok\":true}" : "{\"ok\":false}");
}

// GET /api/diary.csv  -> download the diary CSV
static void handleDiaryDownload() {
  server.sendHeader("Cache-Control", "no-store");

  if (!ensureFsMounted() || !LittleFS.exists(DIARY_PATH)) {
    // Return empty file with header so the browser still downloads something
    server.sendHeader("Content-Disposition", "attachment; filename=\"growdiary.csv\"");
    server.send(200, "text/csv; charset=utf-8", "ts_local,phase,grow_day,grow_week,phase_day,phase_week,note\n");
    return;
  }

  File f = LittleFS.open(DIARY_PATH, FILE_READ);
  if (!f) {
    server.send(500, "text/plain; charset=utf-8", "open failed");
    return;
  }

  server.sendHeader("Content-Disposition", "attachment; filename=\"growdiary.csv\"");
  server.streamFile(f, "text/csv; charset=utf-8");
  f.close();
}

// POST /api/diary/clear
// Manual, user-triggered recovery action:
// - requires confirmation token
// - formats LittleFS
// - mounts LittleFS again
// This intentionally deletes the diary and any other LittleFS files, but never runs automatically.
static void handleDiaryClear() {
  server.sendHeader("Cache-Control", "no-store");

  if (!readFsFormatConfirmation()) {
    sendFsConfirmationRequired();
    return;
  }

  const bool ok = formatFsForRecovery();
  if (!ok) {
    server.send(500, "application/json; charset=utf-8", "{\"ok\":false,\"err\":\"format_failed\"}");
    return;
  }

  server.send(200, "application/json; charset=utf-8", "{\"ok\":true,\"cleared\":true,\"formatted\":true,\"mounted\":true}");
}

// -------------------- State API caching (registry -> JSON) --------------------
static String g_apiStateCache;
static uint32_t g_apiStateCacheMs = 0;
static const uint32_t API_STATE_CACHE_TTL_MS = 5000;

static void buildApiStateJson(String& json) {
  const char* nl  = "\n";
  const char* ind = "  ";

  json = "";
  json.reserve(4096);

  json += "{";
  json += nl;

  for (size_t i = 0; i < VARS_COUNT; i++) {
    if (i) {
      json += ",";
      json += nl;
    }
    json += ind;
    json += "\"";
    json += VARS[i].key;
    json += "\": ";
    json += VARS[i].get();
  }

  json += nl;
  json += "}";
}

// -------------------- State/Variables API (registry -> JSON) --------------------
void handleApiState() {
  uint32_t now = millis();

  if (g_apiStateCache.length() == 0 || (now - g_apiStateCacheMs) > API_STATE_CACHE_TTL_MS) {
    buildApiStateJson(g_apiStateCache);
    g_apiStateCacheMs = now;
  }

  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json; charset=utf-8", g_apiStateCache);
}

// -------------------- Deferred init task (moves slow stuff out of setup) --------------------
static void taskDeferredInit(void* /*pv*/) {
  // Read full preferences (relays, run settings, shelly, notifications, etc.)
  readPreferences();

  // DS18B20 init (quick)
  sensors.begin();
  sensors.setWaitForConversion(false); // async mode; we will check for completion in the sensor task
  sensors.setResolution(12); // max precision, takes ~750ms per conversion, but we start it now so it can run in parallel with other init. Subsequent readings will be faster due to async mode.
  sensors.requestTemperatures(); // start first conversion ASAP (takes ~750ms), subsequent readings will be faster due to async mode

  // I2C + BME280 init
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(100000); // robust for longer wiring

  // I2C bus scan
  logPrint("[I2C] Scanning bus...");
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      logPrint("[I2C] Found device at 0x" + String(addr, HEX));
      delay(2);
    }
  }

  // Initialize BME280 sensor (retry up to 10s)
  bmeAvailable = false;
  uint8_t candidates[2] = { 0x76, 0x77 };
  bool bmeInit = false;

  unsigned long startTime = millis();
  while (!bmeInit && (millis() - startTime < 10000)) {
    for (uint8_t i = 0; i < 2 && !bmeInit; i++) {
      uint8_t a = candidates[i];
      logPrint("[SENSOR] Trying BME280 at 0x" + String(a, HEX) + " ...");

      if (bme.begin(a, &Wire)) { // wichtig: &Wire
        logPrint("[SENSOR] BME280 initialized at 0x" + String(a, HEX));
        bmeAvailable = true;

        // seed with one reading
        readSensorData();
        addReading(cur.temperatureC, cur.humidityPct, cur.vpdKpa);

        bmeInit = true;
      } else {
        delay(250);
      }
    }

    if (!bmeInit) {
      logPrint("[SENSOR] BME280 not found, retrying in 500 ms. Check wiring!");
      delay(500);
    }
  }

  if (!bmeAvailable) {
    logPrint("[SENSOR] BME280 not found (will run without it)");
  }

  // Start tasks (they should be written defensively: check wifiReady/bmeAvailable)
  xTaskCreatePinnedToCore(sensorTask, "sensor", 2048, nullptr, 1, &sensorTaskHandle, 1);

  if (bmeAvailable) {
    xTaskCreatePinnedToCore(taskCheckBMESensor, "BME", 4096, nullptr, 1, nullptr, 1);
  }

  // Shelly status task can run even if WiFi isn't ready yet; it should handle that.
  xTaskCreatePinnedToCore(taskShellyStatus, "Shelly", 4096, nullptr, 1, nullptr, 1);

  // Watering task (relay control) can also run without WiFi, but should check activeRelayCount and handle it gracefully.
  xTaskCreatePinnedToCore(taskWatering, "Watering", 4096, nullptr, 1, nullptr, 1);

  vTaskDelete(nullptr);
}

// -------------------- setup --------------------
void setup() {
  Serial.begin(115200);

  // Create mutex for relay state access (used by both HTTP handlers and watering task)
  relayMutex = xSemaphoreCreateMutex();
  if (relayMutex == nullptr) {
    logPrint("[BOOT] relayMutex create failed");
  }

  // Create mutex for log buffer access (used by multiple tasks)
  logBufferMutex = xSemaphoreCreateMutex();
  if (logBufferMutex == nullptr) {
    logPrint("[BOOT] logBufferMutex create failed");
  }

  // LittleFS is checked exactly once during boot.
  // On failure, the result is cached and no automatic retries are performed.
  ensureFsMounted();

  // Load WiFi credentials only (fast)
  preferences.begin(PREF_NS, true);
  loadPrefString(KEY_SSID, ssidName, "", true, "ssidName");
  loadPrefString(KEY_PASS, ssidPassword, "", false, "ssidPassword");
  loadPrefInt(KEY_RELAYCOUNT, activeRelayCount, 4, true, "activeRelayCount");
  activeRelayCount = (activeRelayCount == 8) ? 8 : 4;
  preferences.end();
  
  // Initialize relay outputs ASAP (prevent random toggles)
  for (int i = 0; i < activeRelayCount; i++) {
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], LOW);
  }

  // Decide WiFi mode:
  // - no credentials -> AP only
  // - credentials present -> try STA first
  // - if STA is not up in time, start AP as fallback but keep STA reconnect possible
  wifiReady = false;

  if (ssidName.length() == 0) {
    espMode = true;
    startSoftAP();
  } else {
    WiFi.mode(WIFI_STA);
    WiFi.persistent(false);
    WiFi.setAutoReconnect(true);
    WiFi.begin(ssidName.c_str(), ssidPassword.c_str());
    txPower = (WiFi.getTxPower() / 4);
    //WiFi.setTxPower(WIFI_POWER_19_5dBm); // max power (range boost, but more current)

    Serial.println("[WIFI] Connecting to: " + ssidName);

    const uint32_t WIFI_BOOT_CONNECT_TIMEOUT_MS = 10000UL;
    const uint32_t t0 = millis();

    while (WiFi.status() != WL_CONNECTED &&
           (uint32_t)(millis() - t0) < WIFI_BOOT_CONNECT_TIMEOUT_MS) {
      delay(100);
    }

    if (WiFi.status() == WL_CONNECTED) {
      wifiReady = true;
      espMode = false;

      Serial.println("[WIFI] Connected: " + WiFi.localIP().toString());
      Serial.println("[BOOT] FW build: " + String(__DATE__) + " " + String(__TIME__));

      logPrint("[BOOT] Starting initial NTP sync...");
      configTzTime(tzInfo.c_str(), ntpServer.c_str());
      ntpSyncPending = true;
      ntpStartMs = millis();
    } else {
      Serial.println("[WIFI] STA connect timeout -> start AP fallback, keep STA reconnect enabled");

      wifiReady = false;

      // Important:
      // We do NOT want to end up in AP-only mode if credentials exist.
      // Keep reconnect logic alive in loop().
      espMode = false;

      // Start fallback AP, but do not disable STA operation.
      WiFi.mode(WIFI_AP_STA);
      startSoftAP();
    }
  }

  // Routes (unchanged)
  server.on("/", handleRoot);
  server.on("/save", HTTP_POST, handleSaveWiFi);
  server.on("/saverunsettings", HTTP_POST, handleSaveRunsettings);
  server.on("/saveshellysettings", HTTP_POST, handleSaveShellySettings);
  // For resetting shelly energy counters, we have separate endpoints per device (main/light) to simplify frontend logic (no need to send which one in body).
  server.on("/api/shelly/reset-energy", HTTP_POST, handleResetShellyEnergy);
  server.on("/savesettings", HTTP_POST, handleSaveSettings);
  server.on("/savemessagesettings", HTTP_POST, handleSaveMessageSettings);
  server.on("/api/newgrow", HTTP_POST, handleNewGrow);
  // Static assets (from PROGMEM)
  server.on("/style.css", []() { server.send_P(200, "text/css", cssContent); });
  server.on("/script.js", []() { server.send_P(200, "application/javascript", jsContent); });

  // LittleFS status / explicit recovery format (OTA-safe)
  server.on("/api/fs/status", HTTP_GET, handleFsStatus);
  server.on("/api/fs/format", HTTP_POST, handleFsFormat);

  // grow diary (LittleFS CSV)
  server.on("/api/diary/add", HTTP_POST, handleDiaryAdd);
  server.on("/api/diary/update", HTTP_POST, handleDiaryUpdate);
  server.on("/api/diary/delete", HTTP_POST, handleDiaryDelete);
  server.on("/api/diary.csv", HTTP_GET, handleDiaryDownload);
  server.on("/api/diary/clear", HTTP_POST, handleDiaryClear);

  // full state / variables (debug page)
  server.on("/api/state", HTTP_GET, handleApiState);

  // relay toggle
  server.on("/relay/1/toggle", HTTP_POST, []() { handleRelayToggleIdx(0); });
  server.on("/relay/2/toggle", HTTP_POST, []() { handleRelayToggleIdx(1); });
  server.on("/relay/3/toggle", HTTP_POST, []() { handleRelayToggleIdx(2); });
  server.on("/relay/4/toggle", HTTP_POST, []() { handleRelayToggleIdx(3); });
  server.on("/relay/5/toggle", HTTP_POST, []() { handleRelayToggleIdx(4); });

  // pump toggle + auto-off trigger (10s)
  server.on("/pump/6/triggerPump10s", HTTP_POST, []() { handlePumpPulseIdx(5, 10000UL); });
  server.on("/pump/7/triggerPump10s", HTTP_POST, []() { handlePumpPulseIdx(6, 10000UL); });
  server.on("/pump/8/triggerPump10s", HTTP_POST, []() { handlePumpPulseIdx(7, 10000UL); });

  // Shelly toggle endpoints (read current state, invert, set new state, return result). We have separate endpoints per device to simplify frontend logic (no need to send which one in body).
  server.on("/shelly/main/toggle", HTTP_POST, []() {
    bool ok = false;
    bool newState = false;

    ShellyValues v = getShellyValues(settings.shelly.main, 0);
    if (v.ok) {
      newState = !v.isOn;
      ok = shellySwitchSet(settings.shelly.main.ip, settings.shelly.main.gen, newState, 0, 80);
    }

    String resp = String("{\"ok\":") + (ok ? "true" : "false") +
                  String(",\"isOn\":") + (newState ? "true" : "false") + "}";

    server.send(ok ? 200 : 500, "application/json", resp);
  });

  // Note: we have separate endpoints for each Shelly device to simplify frontend logic (no need to send which one in body).
  server.on("/shelly/light/toggle", HTTP_POST, []() {
    bool ok = false;
    bool newState = false;

    ShellyValues v = getShellyValues(settings.shelly.light, 0);
    if (v.ok) {
      newState = !v.isOn;
      ok = shellySwitchSet(settings.shelly.light.ip, settings.shelly.light.gen, newState, 0, 80);
    }

    String resp = String("{\"ok\":") + (ok ? "true" : "false") +
                  String(",\"isOn\":") + (newState ? "true" : "false") + "}";

    server.send(ok ? 200 : 500, "application/json", resp);
  });

  // Note: we have separate endpoints for each Shelly device to simplify frontend logic (no need to send which one in body).
  server.on("/shelly/humidifier/toggle", HTTP_POST, []() {
    bool ok = false;
    bool newState = false;

    ShellyValues v = getShellyValues(settings.shelly.humidifier, 0);
    if (v.ok) {
      newState = !v.isOn;
      ok = shellySwitchSet(settings.shelly.humidifier.ip, settings.shelly.humidifier.gen, newState, 0, 80);
    }

    String resp = String("{\"ok\":") + (ok ? "true" : "false") +
                  String(",\"isOn\":") + (newState ? "true" : "false") + "}";

    server.send(ok ? 200 : 500, "application/json", resp);
  });

  // Note: we have separate endpoints for each Shelly device to simplify frontend logic (no need to send which one in body).
  server.on("/shelly/heater/toggle", HTTP_POST, []() {
    bool ok = false;
    bool newState = false;

    ShellyValues v = getShellyValues(settings.shelly.heater, 0);
    if (v.ok) {
      newState = !v.isOn;
      ok = shellySwitchSet(settings.shelly.heater.ip, settings.shelly.heater.gen, newState, 0, 80);
    }

    String resp = String("{\"ok\":") + (ok ? "true" : "false") +
                  String(",\"isOn\":") + (newState ? "true" : "false") + "}";

    server.send(ok ? 200 : 500, "application/json", resp);
  });

  // Note: we have separate endpoints for each Shelly device to simplify frontend logic (no need to send which one in body).
  server.on("/shelly/fan/toggle", HTTP_POST, []() {
    bool ok = false;
    bool newState = false;

    ShellyValues v = getShellyValues(settings.shelly.fan, 0);
    if (v.ok) {
      newState = !v.isOn;
      ok = shellySwitchSet(settings.shelly.fan.ip, settings.shelly.fan.gen, newState, 0, 80);
    }

    String resp = String("{\"ok\":") + (ok ? "true" : "false") +
                  String(",\"isOn\":") + (newState ? "true" : "false") + "}";

    server.send(ok ? 200 : 500, "application/json", resp);
  });

  // Note: we have separate endpoints for each Shelly device to simplify frontend logic (no need to send which one in body).
  server.on("/shelly/exhaust/toggle", HTTP_POST, []() {
    bool ok = false;
    bool newState = false;

    ShellyValues v = getShellyValues(settings.shelly.exhaust, 0);
    if (v.ok) {
      newState = !v.isOn;
      ok = shellySwitchSet(settings.shelly.exhaust.ip, settings.shelly.exhaust.gen, newState, 0, 80);
    }

    String resp = String("{\"ok\":") + (ok ? "true" : "false") +
                  String(",\"isOn\":") + (newState ? "true" : "false") + "}";

    server.send(ok ? 200 : 500, "application/json", resp);
  });

  // Watering and tank level (used by pump toggle and tank level read buttons in UI)
  server.on("/startWatering", HTTP_POST, handleStartWatering);
  server.on("/pingTank", HTTP_POST, readTankLevel);

  // hint message
  server.on("/api/hint", HTTP_GET, handleHint);

  // diary list (for UI)
  server.on("/api/diary/list", HTTP_GET, handleDiaryList);
  // Factory reset (clears WiFi credentials and restarts)
  server.on("/factory-reset", handleFactoryReset);
  // Log buffer (for UI)
  server.on("/api/logbuffer", HTTP_GET, handleApiLogBuffer);
  server.on("/api/logbuffer/clear", HTTP_POST, handleClearLog);
  server.on("/log", HTTP_GET, handleDownloadLog);
  server.on("/api/ota/check", HTTP_GET, handleOtaCheck);
  server.on("/api/ota/update", HTTP_POST, handleOtaUpdate);
  // Catch-all for undefined routes
  server.onNotFound([]() {
    Serial.printf("404 Not Found: %s (method %d)\n", server.uri().c_str(), (int)server.method());
  });

  server.begin();
  pushHintKey("hint.systemStarted");
  logPrint("[APP] system started");

  // Do the slow init after the webserver is up.
  xTaskCreatePinnedToCore(taskDeferredInit, "deferredInit", 4096, nullptr, 1, nullptr, 0);
}

// -------------------- loop --------------------
void loop() {
  // WiFi reconnect policy:
  // - check every 10s
  // - gentle reconnect only every 15s
  // - hard reset only after 60s offline
  // - hard reset retry only every 60s
  static uint32_t lastWifiCheck = 0;
  static uint32_t wifiDownSince = 0;
  static uint32_t lastWifiReconnectAttempt = 0;
  static uint32_t lastWifiHardResetAttempt = 0;

  const uint32_t WIFI_CHECK_MS = 10000UL;
  const uint32_t WIFI_RECONNECT_RETRY_MS = 15000UL;
  const uint32_t WIFI_HARD_RESET_AFTER_MS = 60000UL;
  const uint32_t WIFI_HARD_RESET_RETRY_MS = 60000UL;

  if ((uint32_t)(millis() - lastWifiCheck) >= WIFI_CHECK_MS) {
    lastWifiCheck = millis();

    // Only try STA reconnects when we actually have credentials and are not in AP mode
    if (!espMode && ssidName.length() > 0) {
      wl_status_t st = WiFi.status();

      if (st == WL_CONNECTED) {
        txPower = (WiFi.getTxPower() / 4);
        wifiReady = true;
        wifiDownSince = 0;
        lastWifiReconnectAttempt = 0;
        lastWifiHardResetAttempt = 0;
      } else {
        wifiReady = false;

        if (wifiDownSince == 0) {
          wifiDownSince = millis();
        }

        const bool longOffline =
          (uint32_t)(millis() - wifiDownSince) >= WIFI_HARD_RESET_AFTER_MS;

        // Avoid reconnect hammering while WiFi stack is already busy connecting
        const bool wifiBusyConnecting =
          (st == WL_IDLE_STATUS);

        if (!longOffline) {
          if (!wifiBusyConnecting &&
              (lastWifiReconnectAttempt == 0 ||
               (uint32_t)(millis() - lastWifiReconnectAttempt) >= WIFI_RECONNECT_RETRY_MS)) {

            lastWifiReconnectAttempt = millis();
            WiFi.reconnect();
          }
        } else {
          if (lastWifiHardResetAttempt == 0 ||
              (uint32_t)(millis() - lastWifiHardResetAttempt) >= WIFI_HARD_RESET_RETRY_MS) {

            lastWifiHardResetAttempt = millis();

            // Important: do NOT erase saved WiFi/AP config
            WiFi.disconnect(false, false);
            delay(100);
            WiFi.begin(ssidName.c_str(), ssidPassword.c_str());
          }
        }
      }
    }
  }

  processPumpAutoOff();

  server.handleClient();
  delay(1);

  dailyNtpTrigger();
  ntpSyncTick();

  if (g_restartRequested && (int32_t)(millis() - g_restartAtMs) >= 0) {
    g_restartRequested = false;
    ESP.restart();
  }
}
#include "ota.h"

#include <WiFi.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFiClientSecure.h>
#include <WebServer.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
extern WebServer server;

#if __has_include("build_version.h")
#include "build_version.h"
#endif

#ifndef FW_VERSION
#define FW_VERSION "dev"
#endif

// ---- configuration ----
static const char* GITHUB_OWNER = "syschelle";
static const char* GITHUB_REPO = "GrowTent";
static const char* OTA_ASSET_NAME = "firmware.bin";

// v0.1.12 -> 0,1,12
static bool parseSemver(const String& in, int& major, int& minor, int& patch) {
  String s = in;
  s.trim();

  if (s.startsWith("v") || s.startsWith("V")) {
    s.remove(0, 1);
  }

  int p1 = s.indexOf('.');
  int p2 = s.indexOf('.', p1 + 1);

  if (p1 <= 0 || p2 <= p1) {
    return false;
  }

  major = s.substring(0, p1).toInt();
  minor = s.substring(p1 + 1, p2).toInt();
  patch = s.substring(p2 + 1).toInt();
  return true;
}

static int compareSemver(const String& a, const String& b) {
  int aMajor, aMinor, aPatch;
  int bMajor, bMinor, bPatch;

  bool aOk = parseSemver(a, aMajor, aMinor, aPatch);
  bool bOk = parseSemver(b, bMajor, bMinor, bPatch);

  // If the installed version is "dev" or otherwise invalid,
  // treat it as older than any valid release tag.
  if (!aOk && bOk) return -1;

  // If the latest version is invalid but the installed one is valid,
  // do not offer an update.
  if (aOk && !bOk) return 1;

  // If both are invalid, consider them equal.
  if (!aOk && !bOk) return 0;

  if (aMajor != bMajor) return (aMajor < bMajor) ? -1 : 1;
  if (aMinor != bMinor) return (aMinor < bMinor) ? -1 : 1;
  if (aPatch != bPatch) return (aPatch < bPatch) ? -1 : 1;

  return 0;
}

void handleOtaCheck() {
  if (WiFi.status() != WL_CONNECTED) {
    server.send(503, "application/json; charset=utf-8",
                "{\"ok\":false,\"error\":\"wifi_not_connected\"}");
    return;
  }

  const String apiUrl =
      String("https://api.github.com/repos/") +
      GITHUB_OWNER + "/" + GITHUB_REPO + "/releases/latest";

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.setTimeout(12000);

  if (!http.begin(client, apiUrl)) {
    server.send(500, "application/json; charset=utf-8",
                "{\"ok\":false,\"error\":\"http_begin_failed\"}");
    return;
  }

  http.addHeader("User-Agent", "GrowTent-ESP32");
  http.addHeader("Accept", "application/vnd.github+json");

  const int code = http.GET();
  if (code != 200) {
    String err = String("{\"ok\":false,\"error\":\"github_http_") + code + "\"}";
    http.end();
    server.send(502, "application/json; charset=utf-8", err);
    return;
  }

  JsonDocument filter;
  filter["tag_name"] = true;
  filter["body"] = true;
  filter["assets"][0]["name"] = true;
  filter["assets"][0]["browser_download_url"] = true;

  JsonDocument doc;
  DeserializationError jerr =
    deserializeJson(doc, http.getStream(),
                  DeserializationOption::Filter(filter));

  http.end();

  if (jerr) {
    server.send(500, "application/json; charset=utf-8",
                "{\"ok\":false,\"error\":\"json_parse_failed\"}");
    return;
  }

  String latestTag = doc["tag_name"] | "";
  String changelog = doc["body"] | "";
  String firmwareUrl = "";

  JsonVariant assetsVar = doc["assets"];
  if (assetsVar.is<JsonArray>()) {
    JsonArray assets = assetsVar.as<JsonArray>();

    for (JsonObject asset : assets) {
      String name = asset["name"] | "";
      String url  = asset["browser_download_url"] | "";

      if (name.equalsIgnoreCase(OTA_ASSET_NAME)) {
        firmwareUrl = url;
        break;
      }
    }

    if (firmwareUrl.isEmpty()) {
      for (JsonObject asset : assets) {
        String name = asset["name"] | "";
        String url  = asset["browser_download_url"] | "";

        if (name.endsWith(".bin")) {
          firmwareUrl = url;
          break;
        }
      }
    }
  }

  bool updateAvailable = false;
  if (!latestTag.isEmpty()) {
    updateAvailable = (compareSemver(String(FW_VERSION), latestTag) < 0);
  }

  JsonDocument out;
  out["ok"] = true;
  out["currentVersion"] = FW_VERSION;
  out["latestVersion"] = latestTag;
  out["updateAvailable"] = updateAvailable;
  out["changelog"] = changelog;
  out["firmwareUrl"] = firmwareUrl;

  String resp;
  serializeJson(out, resp);

  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json; charset=utf-8", resp);
}

void handleOtaUpdate() {
  if (WiFi.status() != WL_CONNECTED) {
    server.send(503, "application/json; charset=utf-8",
                "{\"ok\":false,\"error\":\"wifi_not_connected\"}");
    return;
  }

  Serial.println("[OTA] Update requested");
  Serial.println("[OTA] WiFi connected");
  Serial.print("[OTA] IP: ");
  Serial.println(WiFi.localIP());

  const char* firmwareUrl =
    "https://github.com/syschelle/GrowTent/releases/latest/download/firmware.bin";
  Serial.print("[OTA] URL: ");
  Serial.println(firmwareUrl);

  WiFiClientSecure client;
  client.setInsecure();
  client.setTimeout(30);

  HTTPUpdate otaUpdater;
  otaUpdater.rebootOnUpdate(false);
  otaUpdater.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);

  otaUpdater.onStart([]() {
    Serial.println("[OTA] Start");
  });

  otaUpdater.onProgress([](int current, int total) {
    Serial.printf("[OTA] Progress: %d / %d\n", current, total);
  });

  otaUpdater.onEnd([]() {
    Serial.println("[OTA] End");
  });

  otaUpdater.onError([](int error) {
    Serial.printf("[OTA] Error callback: %d\n", error);
  });

  t_httpUpdate_return ret = otaUpdater.update(client, firmwareUrl);

  if (ret == HTTP_UPDATE_OK) {
    Serial.println("[OTA] Update OK, restarting...");
    server.send(200, "application/json; charset=utf-8",
                "{\"ok\":true,\"message\":\"updated_restarting\"}");
    delay(1000);
    ESP.restart();
    return;
  }

  if (ret == HTTP_UPDATE_NO_UPDATES) {
    Serial.println("[OTA] No updates");
    server.send(200, "application/json; charset=utf-8",
                "{\"ok\":true,\"message\":\"no_update\"}");
    return;
  }

  String err = otaUpdater.getLastErrorString();
  Serial.print("[OTA] Failed: ");
  Serial.println(err);

  err.replace("\\", "\\\\");
  err.replace("\"", "\\\"");

  String json = "{\"ok\":false,\"error\":\"";
  json += err;
  json += "\"}";

  server.send(500, "application/json; charset=utf-8", json);
}
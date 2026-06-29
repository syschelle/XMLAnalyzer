#include "helper.h"

#include <LittleFS.h>

// kommt aus deinem Projekt
void logPrint(const String& msg);

static bool g_fsMounted = false;
static bool g_fsMountTried = false;
static bool g_fsMountFailed = false;

bool ensureFsMounted() {
  if (g_fsMounted) return true;

  // OTA/HAP safe: after one failed mount attempt, never retry automatically.
  // Manual recovery is still possible via formatFsForRecovery().
  if (g_fsMountTried && g_fsMountFailed) return false;

  g_fsMountTried = true;
  g_fsMountFailed = false;

  // OTA-safe: false = do NOT auto-format on mount failure.
  // Auto-formatting here could erase existing diary data after an OTA update.
  g_fsMounted = LittleFS.begin(false);

  if (!g_fsMounted) {
    g_fsMountFailed = true;
    logPrint("[LITTLEFS] mount failed - cached, no automatic retry (OTA/HAP safe)");
  } else {
    g_fsMountFailed = false;
    logPrint("[LITTLEFS] mounted");
  }

  return g_fsMounted;
}

bool isFsMounted() {
  return g_fsMounted;
}

bool fsMountWasTried() {
  return g_fsMountTried;
}

bool fsMountFailed() {
  return g_fsMountTried && g_fsMountFailed && !g_fsMounted;
}

bool formatFsForRecovery() {
  logPrint("[LITTLEFS] recovery format requested");

  if (g_fsMounted) {
    LittleFS.end();
    g_fsMounted = false;
  }

  // Reset cached failure only for this explicit manual recovery action.
  g_fsMountTried = true;
  g_fsMountFailed = false;

  if (!LittleFS.format()) {
    g_fsMountFailed = true;
    logPrint("[LITTLEFS] format failed");
    return false;
  }

  g_fsMounted = LittleFS.begin(false);
  if (!g_fsMounted) {
    g_fsMountFailed = true;
    logPrint("[LITTLEFS] mount after format failed - cached, no automatic retry");
    return false;
  }

  g_fsMountFailed = false;
  logPrint("[LITTLEFS] formatted and mounted");
  return true;
}

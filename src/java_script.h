// java_script.h 
#pragma once

// Pure JavaScript payload (no <script> tags, no HTML comments)
const char jsContent[] PROGMEM = R"rawliteral(
// === Inject i18n JSON <script type="application/json"> tags ===
(function injectI18N(){
  const addJSON = (id, obj) => {
    const s = document.createElement('script');
    s.type = 'application/json';
    s.id   = id;
    s.textContent = JSON.stringify(obj);
    document.head.appendChild(s);
  };

  addJSON('i18n-manifest', {
    "default": "de",
    "languages": [
      { "code": "de", "name": "Deutsch", "dir": "ltr" },
      { "code": "en", "name": "English", "dir": "ltr" }
    ]
  });

  addJSON('i18n', {

  /* -------------------- a11y / info -------------------- */
  "a11y.menu": { de: "Menü öffnen/schließen", en: "Open/close menu" },

  "info.Grow": { de: "Aktueller Grow", en: "Current Grow" },
  "info.Days": { de: "Tage", en: "Days" },
  "info.Weeks": { de: "Wochen", en: "Weeks" },
  "info.growLine": {
    de: "Grow seit: {days} {daysLabel} | {weeks} {weeksLabel}",
    en: "Growing since: {days} {daysLabel} | {weeks} {weeksLabel}"
  },

  "settings.unsaved": {
    de: "Änderungen – bitte speichern",
    en: "Changes pending – please save"
  },

  /* -------------------- Navigation -------------------- */
  "nav.status":      { de: "Status", en: "Status" },
  "nav.runsetting":  { de: "Betriebseinstellungen", en: "Operating Settings" },
  "nav.shelly":      { de: "Shelly Einstellungen", en: "Shelly Settings" },
  "nav.settings":    { de: "Systemeinstellungen", en: "System Settings" },
  "nav.message":     { de: "Push-Einstellungen", en: "Push Settings" },
  "nav.logging":     { de: "Systemprotokoll", en: "System Log" },
  "nav.ota":         { de: "OTA Update", en: "OTA Update" },
  "nav.factory":     { de: "Werkseinstellungen", en: "Factory Reset" },
  "nav.diary":      { de: "Grow Tagebuch", en: "Grow Diary" },

  /* -------------------- Status -------------------- */
  "status.title": { de: "Status", en: "Status" },
  "status.updated": { de: "Letztes Update:", en: "Last update:" },
  "status.download": { de: "History herunterladen", en: "Download History" },
  "status.delete": { de: "History löschen", en: "Delete History" },
  "status.currentValues": { de: "Aktuelle Werte", en: "Current values" },

  "status.temperature": { de: "Temperatur", en: "Temperature" },
  "status.targetTemp": { de: "Soll-Temperatur", en: "Target temperature" },
  "status.last": { de: "akt. ", en: "current " },

  "status.irrigation": { de: "Bewässerung", en: "Irrigation" },
  "status.irrigationRelay6": { de: "Pumpe1 (Relay6)", en: "Pump1 (Relay6)" },
  "status.irrigationRelay7": { de: "Pumpe2 (Relay7)", en: "Pump2 (Relay7)" },
  "status.irrigationRelay8": { de: "Pumpe3 (Relay8)", en: "Pump3 (Relay8)" },

  "status.lastWaterTemperature": {
    de: "akt. Wassertemperatur",
    en: "current Water Temperature"
  },

  "status.humidity": { de: "Luftfeuchte", en: "Humidity" },
  "status.lastvpd": { de: "akt. VPD", en: "current VPD" },
  "status.targetVpd": { de: "Soll-VPD:", en: "Target VPD" },

  "status.averagesLastHour": {
    de: "Durchschnittswerte der letzten Stunde",
    en: "Averages last hour"
  },
  "status.history": { de: "Verlauf (Aufzeichnung alle 10 Minuten)", en: "History (recorded every 10 minutes)" },
  "status.refresh": { de: "Aktualisieren", en: "Refresh" },

  "status.avgTemperature": { de: "Ø Temperatur", en: "Ø Temperature" },
  "status.avgHumidity": { de: "Ø Luftfeuchte", en: "Ø Humidity" },
  "status.avgVpd": { de: "Ø VPD", en: "Ø VPD" },
  "status.avg": { de: "Ø ", en: "Ø " },
  "status.avgWaterTemperature": {
    de: "Ø Wassertemperatur",
    en: "Ø Water temperature"
  },

  /* Relay / Irrigation */
  "status.watering": { de: "Bewässerung", en: "Watering" },
  "status.wateringLeft": { de: " verbleibend", en: " remaining" },
  "status.endIn": { de: "Ende in ", en: "End in " },
  "status.tank": { de: "Tank Füllung", en: "Tank Level" },
  "status.pingTank": { de: "Ping", en: "Ping" },
  "status.relayControl": { de: "Relais Steuerung", en: "Relay Control" },
  "status.toggleRelay": { de: "umschalten", en: "toggle" },
  "status.toggleRelayLabel": { de: "umschalten", en: "toggle" },

  "status.togglePump10s": { de: "einschalten (10s)", en: "turn on (10s)" },

  /* Shelly */
  "status.shellyControl": { de: "Shelly Steuerung", en: "Shelly Control" },
  "status.shellyMainSw": { de: "Hauptschalter", en: "Main Switch" },
  "shelly.shellyIPMainSw": { de: "Shelly IP Adresse für Hauptschalter:", en: "Shelly IP address for main switch:" },
  "status.shellyHeater": { de: "Heizung", en: "Heater" },
  "shelly.shellyIPLight": { de: "Shelly IP Adresse für Pflanzlicht:", en: "Shelly IP address for grow light:" },
  "status.shellyHumidifier": { de: "Luftbefeuchter", en: "Humidifier" },
  "shelly.shellyIPHumidifier": { de: "Shelly IP Adresse für Luftbefeuchter:", en: "Shelly IP address for humidifier:" },
  "status.shellyHeater": { de: "Heizung", en: "Heater" },
  "shelly.shellyIPHeater": { de: "Shelly IP Adresse für Heizgerät:", en: "Shelly IP address for heater:" },
  "status.shellyFan": { de: "Ventilator", en: "Fan" },
  "shelly.shellyIPFan": { de: "Shelly IP Adresse für Ventilator:", en: "Shelly IP address for fan:" },
  "status.shellyLight": { de: "Pflanzlicht", en: "Grow Light" },
  "shelly.shellyIPExhaust": { de: "Shelly IP Adresse für Abluft:", en: "Shelly IP address for exhaust:" },
  "status.shellyExhaust": { de: "Abluft", en: "Exhaust" },
  
  "status.shellyAuth": { de: "Authentifizierung", en: "Authentication" },
  "status.shellyDevices": { de: "Shelly Geräte", en: "Shelly Devices" },

  "shelly.shellyAuthUser": { de: "Shelly Benutzername (optional):", en: "Shelly username (optional):" },
  "shelly.shellyAuthPassword": { de: "Shelly Passwort (optional):", en: "Shelly password (optional):" },


  "status.relayOn": { de: "einschalten (10s)", en: "turn on (10s)" },

  /* -------------------- runsetting.* -------------------- */
  "runsetting.title": { de: "Betriebseinstellungen", en: "Operating settings" },
  "runsetting.resetkWh": { de: "kWh Zähler zurücksetzen", en: "Reset kWh counter" },
  "runsetting.newGrowError": {"de": "Fehler: Neuer Grow konnte nicht gesetzt werden.", "en": "Error: could not set new grow."},
  "runsetting.newGrowDone": {"de": "Neuer Grow wurde gesetzt.", "en": "New grow has been set."},
  "runsetting.newGrowWorking": {"de": "Setze neuen Grow…", "en": "Setting new grow…"},
  "runsetting.newGrowConfirm": {"de": "Neuen Grow starten?\n\n• Startdatum = heute\n• Blüte/Trocknung wird geleert\n• Energieanzeige wird zurückgesetzt", "en": "Start a new grow?\n\n• Start date = today\n• Flowering/Drying cleared\n• Energy display reset"},
  "runsetting.newGrowBtn": {"de": "Neuer Grow", "en": "New grow"},
  "runsetting.startGrow": { de: "Startdatum:", en: "Start Date:" },
  "runsetting.startFlower": { de: "Startdatum Blüte:", en: "Start Flowering Date:" },
  "runsetting.startDry": { de: "Startdatum Trocknung:", en: "Start Drying Date:" },

  "runsetting.phase": { de: "aktuelle Phase:", en: "Current Phase:" },
  "runsetting.phase.grow": { de: "VEGETATIV", en: "VEGETATIVE" },
  "runsetting.phase.flower": { de: "BLÜTE", en: "FLOWERING" },
  "runsetting.phase.dry": { de: "TROCKNUNG", en: "DRYING" },

  "runsetting.targetTemp": { de: "Soll-Temperatur", en: "Target temperature" },
  "runsetting.offsetLeafTemperature": {
    de: "Offset Blatttemperatur:",
    en: "Offset leaf temperature"
  },
  "runsetting.vpdSettings": { de: "VPD-Einstellungen", en: "VPD Settings" },
  "runsetting.lowVPD.enabled": { de: "Min-VPD Überwachung aktivieren", en: "Enable min VPD monitoring" },
  "runsetting.targetVPD": { de: "Soll-VPD", en: "Target VPD" },
  "runsetting.minVPD": { de: "Offset Min-VPD", en: "Offset min VPD" },
  "runsetting.hysteresis": { de: "Hysterese", en: "Hysteresis" },

  "runsetting.relayScheduling": {
    de: "Relais Zeitsteuerung",
    en: "Relay Scheduling"
  },
  "runsettings.espSchedRelay1": { de: "Relay1", en: "Relay1" },
  "runsettings.espSchedRelay2": { de: "Relay2", en: "Relay2" },
  "runsettings.espSchedRelay3": { de: "Relay3", en: "Relay3" },
  "runsettings.espSchedRelay4": { de: "Relay4", en: "Relay4" },
  "runsetting.relay.enable": { de: "Aktivieren:", en: "Enable:" },
  "runsetting.relay.start": { de: "Start:", en: "Start:" },
  "runsetting.relay.stop": { de: "Stopp:", en: "Stop:" },

  "runsetting.relay.minutesHint": {
  de: "Minutenformat: 0–59 (Minute innerhalb der Stunde). Oder 0 / 0 für permanent an. Einschränkung: Wenn 'auch wenn Licht aus' nicht aktiviert ist, wird das Relay nur geschaltet, wenn auch das Licht an ist (Shelly Gerät).",
  en: "Minute format: 0–59 (minute within the hour). Or 0 / 0 for always on. Restriction: If 'also if light off' is not enabled, the relay will only operate when the light is on (Shelly device)."
  },
  "runsetting.relay.ifLightOff": { de: "auch wenn Licht aus", en: "also if light off" },
  "runsetting.relay.onMinute": { de: "Einschaltminute", en: "On minute" },
  "runsetting.relay.offMinute": { de: "Ausschaltminute", en: "Off minute" },
  "runsetting.relay.enabledShort": { de: "Aktiv", en: "Enabled" },

  /* -------------------- shelly.* -------------------- */
  "shelly.title": { de: "Shelly Einstellungen", en: "Shelly Settings" },
  "shelly.shellyIP": { de: "Shelly IP Adresse:", en: "Shelly IP address:" },

  "shelly.shellyUsername": {
    de: "Shelly Benutzername (optional):",
    en: "Username (optional):"
  },
  "shelly.shellyPassword": {
    de: "Shelly Passwort (optional):",
    en: "Password (optional):"
  },

  "shelly.shellyIPNames": { de: "Shelly Gerätenamen:", en: "Shelly device names:" },
  "shelly.shellyIPNamesHeater": { de: "Heizung:", en: "Heater:" },

  /* -------------------- settings.* -------------------- */
  "settings.title": { de: "Systemeinstellungen", en: "Settings" },
  "settings.debugEnabled": { de: "Debug aktivieren", en: "Enable debug" },

  "settings.relayBoard": { de: "Welches Relay-Board?", en: "Which relay board?" },

  "settings.boxName": { de: "Boxname:", en: "Box name:" },
  "settings.boxName.ph": { de: "z. B. Growtent-1", en: "e.g. Growtent-1" },

  "settings.ntpserver": { de: "NTP-Server:", en: "NTP server:" },
  "settings.ntpserver.ph": { de: "z. B. de.pool.ntp.org", en: "e.g. pool.ntp.org" },

  "settings.timeZoneInfo": { de: "Zeitzone:", en: "Time zone:" },
  "settings.timeZoneInfo.ph": {
    de: "z.B. WEST-1D/WEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00",
    en: "e.g. WEST-1D/WEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00"
  },

  "settings.language": { de: "Sprache:", en: "Language:" },
  "settings.theme": { de: "Theme:", en: "Theme:" },
  "settings.themeLight": { de: "Hell", en: "Light" },
  "settings.themeDark": { de: "Dunkel", en: "Dark" },

  "settings.dateFormat": { de: "Datumsformat:", en: "Date format:" },
  "settings.df_ymd": { de: "YYYY-MM-DD", en: "YYYY-MM-DD" },
  "settings.df_dmy": { de: "DD.MM.YYYY", en: "DD.MM.YYYY" },

  "settings.timeFormat": { de: "Zeitformat:", en: "Time format:" },
  "settings.tf_HHmm": { de: "24h", en: "24h" },
  "settings.tf_hhmma": { de: "12h AM/PM", en: "12h AM/PM" },

  "settings.save": { de: "Speichern", en: "Save" },

  "settings.tempUnit": { de: "Temperatur-Einheit:", en: "Temperature unit:" },
  "settings.celsius": { de: "°C (Celsius)", en: "°C (Celsius)" },
  "settings.fahrenheit": { de: "°F (Fahrenheit)", en: "°F (Fahrenheit)" },

  "settings.DS18B20": { de: "DS18B20 Sensor", en: "DS18B20 Sensor" },
  "settings.DS18B20Address": { de: "DS18B20 Name:", en: "DS18B20 Name:" },

  "runsetting.relayHeating": { de: "Heizungsrelais (optional)", en: "Heating relay (optional)" },
  "runsetting.relay.heatingHint": {
    de: "Wird ein Board Relay für die Heizung verwendet, was für die Temperaturregelung genutzt wird und kann hier definiert werden.",
    en: "If a board relay is used for heating, which is used for temperature control, it can be defined here."
  },
  "runsetting.relay.heatingSource": {
    de: "Heizungsquelle",
    en: "Heating source"
  },  
  "runsetting.relay.heatingRelay": {
    de: "Heizungs-Relay",
    en: "Heating relay"
  },

  "settings.relaySettings": {
    de: "Relais Einstellungen",
    en: "Relay Settings"
  },
  "settings.relay1": { de: "Relaisname 1:", en: "Relay name 1:" },
  "settings.relay2": { de: "Relaisname 2:", en: "Relay name 2:" },
  "settings.relay3": { de: "Relaisname 3:", en: "Relay name 3:" },
  "settings.relay4": { de: "Relaisname 4:", en: "Relay name 4:" },

  /* -------------------- message.* -------------------- */
  "message.title": { de: "Nachrichteneinstellungen", en: "Message Settings" },
  "message.enabled": { de: "aktivieren", en: "Enable" },

  "message.pushoverSettings": { de: "Pushover Einstellungen", en: "Pushover settings" },
  "message.pushoverUserKey": { de: "Pushover Benutzer:", en: "Pushover User:" },
  "message.pushoverUserKey.ph": { de: "Your User Key", en: "Your User Key" },

  "message.pushoverAppKey": { de: "Pushover App Token:", en: "Pushover App Token:" },
  "message.pushoverAppKey.ph": { de: "API Token/Key", en: "API Token/Key" },

  "message.pushoverDevice": {
    de: "Pushover Gerät (optional):",
    en: "Pushover Device (optional):"
  },
  "message.pushoverDevice.ph": { de: "z. B. mein-telefon", en: "e.g. my-phone" },

  "message.gotifySettings": { de: "Gotify Einstellungen", en: "Gotify settings" },
  "message.gotifyServer": { de: "Gotify Server URL:", en: "Gotify Server URL:" },
  "message.gotifyUrl.ph": {
    de: "z. B. https://gotify.me/message",
    en: "e.g. https://gotify.me/message"
  },

  "message.gotifyToken": { de: "Gotify App Token:", en: "Gotify App Token:" },
  "message.gotifyToken.ph": { de: "z. B. a1b2c3", en: "e.g. a1b2c3" },

  /* -------------------- Vars / Logging / Factory -------------------- */
  "vars.variables": { de: "Variablen", en: "Variables" },
  "vars.hint": {
    de: "Debug-Ansicht: alle registrierten Werte (automatisch aus /api/state). Tokens/Passwörter werden maskiert.",
    en: "Debug view: all registered values (automatically from /api/state). Tokens/passwords are masked."
  },

  "logging.title": { de: "Systemprotokoll", en: "Logging Settings" },

  "ota.hint": {
    de: "Die Firmware wird von \"https://github.com/syschelle/GrowTent/releases/latest/download/firmware.bin\" heruntergeladen.",
    en: "The firmware will be downloaded from \"https://github.com/syschelle/GrowTent/releases/latest/download/firmware.bin\"."
  },

  "factory.title": { de: "Werkseinstellungen", en: "Factory Settings" },
  "factory.reset": { de: "Zurücksetzen / Neustart", en: "Reset / Restart" },

  /* -------------------- diary.* -------------------- */
  "diary.title": { de: "Grow Tagebuch", en: "Grow Diary" },
  "diary.total": { de: "Gesamtgrow", en: "Total grow" },
  "diary.phase": { de: "Phase", en: "Phase" },
  "diary.day": { de: "Tag", en: "Day" },
  "diary.week": { de: "Woche", en: "Week" },
  "diary.note": { de: "Notiz (max. 400 Zeichen)", en: "Note (max 400 characters)" },
  "diary.note.ph": { de: "z. B. Gießen, Dünger, Beobachtungen…", en: "e.g. watering, nutrients, observations…" },
  "diary.save": { de: "Eintrag speichern", en: "Save entry" },
  "diary.download": { de: "CSV herunterladen", en: "Download CSV" },
  "diary.clear": { de: "Tagebuch löschen / Speicher reparieren", en: "Clear diary / repair storage" },
  "diary.saved": { de: "Gespeichert ✓", en: "Saved ✓" },
  "diary.cleared": { de: "Tagebuch gelöscht, LittleFS formatiert und gemountet ✓", en: "Diary cleared, LittleFS formatted and mounted ✓" },
  "diary.error": { de: "Fehler beim Speichern", en: "Save failed" },
  "diary.fsError": { de: "Tagebuch-Speicher ist nicht bereit. Bitte 'Tagebuch löschen / Speicher reparieren' ausführen.", en: "Diary storage is not ready. Please run 'Clear diary / repair storage'." },
  "diary.confirmClear": { de: "Tagebuch wirklich löschen und LittleFS neu formatieren? Dadurch werden alle Tagebuchdaten im Gerätespeicher gelöscht. Firmware, WLAN und Einstellungen bleiben erhalten.", en: "Really clear the diary and reformat LittleFS? This deletes all diary data in device storage. Firmware, Wi-Fi and settings remain unchanged." },
  "diary.confirmClear2": { de: "Sicher? Dieser manuelle Schritt formatiert LittleFS und mountet es danach neu.", en: "Are you sure? This manual step formats LittleFS and mounts it again." },

  /* -------------------- Hints -------------------- */
  "hint.systemStarted": { de: "System gestartet", en: "System started" },
  "hint.saved": { "de": "Gespeichert ✓", "en": "Saved ✓" },
  "hint.unsaved": { "de": "Änderungen – bitte speichern", "en": "Changes pending – please save" },

  });

  
})();

// ---------- Toast ----------
window.showToast = function (message, type = "info", duration = 2600) {
  let container = document.getElementById("toastContainer");

  if (!container) {
    container = document.createElement("div");
    container.id = "toastContainer";
    container.style.cssText = [
      "position:fixed",
      "right:16px",
      "bottom:16px",
      "z-index:9999",
      "display:flex",
      "flex-direction:column",
      "gap:10px",
      "max-width:360px",
      "pointer-events:none"
    ].join(";");
    document.body.appendChild(container);
  }

  const toast = document.createElement("div");

  let borderColor = "#3b82f6";
  if (type === "success") borderColor = "#16a34a";
  else if (type === "error") borderColor = "#dc2626";
  else if (type === "warn") borderColor = "#d97706";

  toast.style.cssText = [
    "background:#1f2937",
    "color:#ffffff",
    "border-left:4px solid " + borderColor,
    "border-radius:10px",
    "padding:12px 14px",
    "box-shadow:0 10px 24px rgba(0,0,0,0.25)",
    "font-size:14px",
    "line-height:1.4",
    "opacity:0",
    "transform:translateY(8px)",
    "transition:opacity 180ms ease, transform 180ms ease",
    "word-break:break-word",
    "pointer-events:auto"
  ].join(";");

  toast.textContent = String(message || "");
  container.appendChild(toast);

  requestAnimationFrame(() => {
    toast.style.opacity = "1";
    toast.style.transform = "translateY(0)";
  });

  setTimeout(() => {
    toast.style.opacity = "0";
    toast.style.transform = "translateY(8px)";

    setTimeout(() => {
      toast.remove();
      if (!container.children.length) {
        container.remove();
      }
    }, 220);
  }, Math.max(1000, Number(duration) || 2600));
};

// ---------- Sensor polling ----------
window.sensorTimer = null;
// Sensor poll interval (ms). We'll dynamically slow down when Status page is not visible.
window._sensorPollMs = 10000;

// ---- relay state (firmware can be 4 or 8) ----
let RELAY_COUNT = 4;
let relayStates = Array(8).fill(false);

// Funktion zum Aktualisieren der Sensorwerte
window._stopSensorPoll = function () {
  if (window.sensorTimer) {
    clearInterval(window.sensorTimer);
    window.sensorTimer = null;
  }
};

window._startSensorPoll = function () {
  if (!window.sensorTimer && typeof window.updateSensorValues === 'function') {
    window.sensorTimer = setInterval(window.updateSensorValues, window._sensorPollMs || 10000);
  }
};

// Change poll interval without duplicating timers
window._setSensorPollInterval = function(ms){
  const next = Math.max(2000, Number(ms) || 10000);
  if (next === window._sensorPollMs) return;
  window._sensorPollMs = next;
  if (window.sensorTimer) {
    clearInterval(window.sensorTimer);
    window.sensorTimer = null;
    window._startSensorPoll();
  }
};

// function to show/hide relay controls based on detected relay count (4 or 8)
function applyRelayVisibility(relayCount) {
  const count = (relayCount === 8) ? 8 : 4;

  // Show irrigation section only for 8-relay board
  const irr = document.getElementById('irrigationSection');
  if (irr) irr.style.display = (count === 8) ? '' : 'none';

  // Status cards 1..8 (if present)
  for (let i = 1; i <= 8; i++) {
    const card = document.querySelector(`.relay-card[data-relay="${i}"]`);
    if (card) card.style.display = (i <= count) ? '' : 'none';
  }

  // Scheduling rows 1..8 (if present)
  for (let i = 1; i <= 8; i++) {
    const row = document.getElementById(`espRelay${i}Row`);
    if (row) row.style.display = (i <= count) ? '' : 'none';
  }
  
  const show5 = (relayCount === 8);
  const settingsRelay5 = document.getElementById('settingsRelay5Row');
  if (settingsRelay5) settingsRelay5.style.display = show5 ? '' : 'none';

  const irrSettings = document.getElementById('irrigationSettingsSection');
  if (irrSettings) irrSettings.style.display = (count === 8) ? '' : 'none';
}

function updateHeatingSourceUi() {
  const src = document.getElementById('heatingSource')?.value || '0';
  const row = document.getElementById('heatingRelayRow');
  if (row) row.style.display = (src === '1') ? '' : 'none';
  }

  document.addEventListener('DOMContentLoaded', () => {
  const srcSel = document.getElementById('heatingSource');
  if (srcSel) {
    srcSel.addEventListener('change', updateHeatingSourceUi);
    updateHeatingSourceUi();
  }
});

// activate correct relay controls on page load
window.updateRelayButtons = function () {
    // cache relay elements for max 8
    if (!window._relayEls || window._relayEls.length !== 8) {
        window._relayEls = Array.from({ length: 8 }, (_, i) =>
            document.getElementById(`relay-Status${i + 1}`)
        );
    }

    // only apply state to active relays
    for (let i = 0; i < RELAY_COUNT; i++) {
        const btn = window._relayEls[i];
        if (!btn) continue;

        if (relayStates[i]) {
            btn.classList.add('on');
            btn.classList.remove('off');
        } else {
            btn.classList.add('off');
            btn.classList.remove('on');
        }
    }
};

// toggle relay state on button click, then update buttons based on response
window.toggleRelay = function (nr) {
    // guard for 4/8 mode
    if (nr < 1 || nr > RELAY_COUNT) return;

    const idx = nr - 1;

    fetch(`/relay/${nr}/toggle`, { method: 'POST' })
        .then(r => r.json())
        .then(data => {
            if (typeof data.state !== 'undefined') {
                relayStates[idx] = !!data.state;
            } else {
                relayStates[idx] = !relayStates[idx];
            }

            updateRelayButtons();
        })
        .catch(err => {
            console.error('toggle relay failed:', err);
        });
};

window.triggerPump10s = function (relayNo) {
    fetch(`/pump/${relayNo}/triggerPump10s`, { method: 'POST' })
        .then(r => r.json())
        .then(data => {
            if (!data?.ok) {
                console.warn('triggerPump10s failed', data);
            }
        })
        .catch(err => {
            console.error('triggerPump10s error', err);
        });
};

// ---------- Shelly status update ----------
function setShellyStateClass(el, isOn) {
  if (!el) return;

  el.classList.remove('shelly-on', 'shelly-off');
  el.classList.add(isOn ? 'shelly-on' : 'shelly-off');
}

window.toggleShellyRelay = async function(device) {
  const url = (device === 'heater')
    ? '/shelly/heater/toggle'
    : (device === 'humidifier')
      ? '/shelly/humidifier/toggle'
      : (device === 'light')
        ? '/shelly/light/toggle'
        : (device === 'fan')
          ? '/shelly/fan/toggle'
          : (device === 'exhaust')
            ? '/shelly/exhaust/toggle'
            : null;

  if (!url) {
    console.error('[SHELLY][JS] Unknown device:', device);
    return;
  }

  // stop polling while toggling
  window._stopSensorPoll?.();

  try {
    const res = await fetch(url, { method: 'POST', cache: 'no-store' });

    if (!res.ok) {
      console.error('[SHELLY][JS] Toggle failed:', res.status);
      return;
    }

    console.log('[SHELLY][JS] Toggle OK:', device);

    // refresh once AFTER toggle finished
    if (typeof window.updateSensorValues === 'function') {
      setTimeout(() => window.updateSensorValues(), 1200);
    }
  } catch (err) {
    console.error('[SHELLY][JS] Toggle exception:', err);
  } finally {
    // restart polling after everything settled
    setTimeout(() => window._startSensorPoll?.(), 2000);
  }
};



// Run after DOM is ready
window.addEventListener('DOMContentLoaded', () => {

  // ---------- Chart style cache (MUST be before applyTheme) ----------
  let chartStrokeCache = null;

  function getChartStroke(){
    if (chartStrokeCache) return chartStrokeCache;
    const css = getComputedStyle(document.documentElement);
    chartStrokeCache =
      (css.getPropertyValue('--link') ||
       css.getPropertyValue('--text') ||
       '#888').trim();
    return chartStrokeCache;
  }

  function invalidateChartStyleCache(){
    chartStrokeCache = null;
  }  

  // ---------- Small DOM helpers ----------
  const $  = (id) => document.getElementById(id);
  const setText = (id, val) => {
    const el = $(id);
    if (!el) return;
    const next = (val == null) ? '' : String(val);
    // avoid pointless DOM writes (each write can trigger style/layout)
    if (el.textContent !== next) el.textContent = next;
  };
  const isNum = x => typeof x === 'number' && !Number.isNaN(x);
  const escapeHtml = (s) => {
    const str = String(s);
    return str.replace(/[&<>"']/g, ch => ({
      '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;'
    }[ch]));
  };


  const escAttr = (s) => escapeHtml(String(s));
  // ===================================================================
  //  Unsaved/Status hint (8s, resets on new message)
  //  Target element: <span id="unsavedHint" class="dirty-hint" hidden ...>
  // ===================================================================
  const unsavedHintEl = document.getElementById('unsavedHint');
  let _unsavedHintTimer = null;

  /**
   * Show a status hint for a limited time (default 8s). If called again within
   * that time, the previous timer is cleared and the new message stays visible
   * for the full duration.
   */
  function showStatusHint(message, { durationMs = 8000 } = {}) {
    if (!unsavedHintEl) return;

    if (_unsavedHintTimer) {
      clearTimeout(_unsavedHintTimer);
      _unsavedHintTimer = null;
    }

    const msg = (message == null) ? '' : String(message);
    if (!msg.trim()) {
      unsavedHintEl.hidden = true;
      unsavedHintEl.classList.remove('is-visible');
      return;
    }

    unsavedHintEl.textContent = msg;
    unsavedHintEl.hidden = false;
    unsavedHintEl.classList.add('is-visible');

    _unsavedHintTimer = setTimeout(() => {
      unsavedHintEl.hidden = true;
      unsavedHintEl.classList.remove('is-visible');
      _unsavedHintTimer = null;
    }, durationMs);
  }

  /**
   * Convenience helper: shows the localized "settings.unsaved" text plus optional details.
   * details can be a string or an object like { temp: 23.1, vpd: 1.2 }.
   */
  function formatI18n(key, details = {}) {
    const tpl = (typeof I18N === 'object' && I18N && I18N[key]) ? I18N[key] : key;
    return String(tpl).replace(/\{(\w+)\}/g, (_, k) => {
      const v = details?.[k];
      return (v === null || typeof v === 'undefined') ? '' : String(v);
    });
  }


  // ===================================================================
  //  Hint polling from ESP32 WebServer endpoint (/api/hint)
  //  ESP should return: {"id":123,"message":"..."}
  //  - When id changes, message is shown for 8s (and resets if new comes)
  // ===================================================================
  (function pollHint(){
     let lastId = -1;

     async function tick(){
       try {
         const r = await fetch('/api/hint', { cache: 'no-store' });
         if (!r.ok) return;
         const d = await r.json();

         if (typeof d.id === 'number' && d.id !== lastId) {
           lastId = d.id;

           const msg = formatI18n(d.key, d.details || {});
           if (msg && msg.trim()) window.showStatusHint(msg); // 8s + reset
         }
       } catch(e) {}
     }

     tick();
     setInterval(tick, 1000);
  })();

  // ---------- Sidebar & SPA ----------
  const mqDesktop = window.matchMedia('(min-width:1024px)');
  const sidebar   = $('sidebar');
  const overlay   = $('overlay');
  const burgerBtn = $('hamburgerBtn');
  const pages     = document.querySelectorAll('.page');

  function openSidebar(){ if(mqDesktop.matches) return; sidebar?.classList.add('sidebar--open'); overlay?.classList.add('overlay--show'); burgerBtn?.setAttribute('aria-expanded','true'); }
  function closeSidebar(){ sidebar?.classList.remove('sidebar--open'); overlay?.classList.remove('overlay--show'); burgerBtn?.setAttribute('aria-expanded','false'); }
  function toggleSidebar(){ if(mqDesktop.matches) return; const o = sidebar?.classList.contains('sidebar--open'); o ? closeSidebar() : openSidebar(); }
  function syncLayout(){ if(mqDesktop.matches){ closeSidebar(); } }

  burgerBtn?.addEventListener('click', toggleSidebar);
  overlay?.addEventListener('click', closeSidebar);
  window.addEventListener('keydown', e => { if(e.key === 'Escape') closeSidebar(); });
  mqDesktop.addEventListener('change', syncLayout);

  // ---------- SPA navigation ----------
  function activatePage(id){
    pages.forEach(p => p.classList.remove('active'));
    document.querySelectorAll('.navlink').forEach(a => a.removeAttribute('aria-current'));
    const pageEl = $(id);
    if (pageEl) pageEl.classList.add('active');
    const currentLink = sidebar?.querySelector(`.navlink[data-page="${id}"]`);
    currentLink?.setAttribute('aria-current', 'page');
    onPageChanged(id);
  }

  sidebar?.addEventListener('click', e => {
    const link = e.target.closest('.navlink'); if(!link) return;
    const id = link.getAttribute('data-page');
    activatePage(id);
    closeSidebar();
  });
  syncLayout();

  // ---------- Theme ----------
  function applyTheme(theme){
    document.documentElement.setAttribute('data-theme', theme);
    localStorage.setItem('theme', theme);
    const sel = $('theme');
    if (sel && sel.value !== theme) sel.value = theme;
  }
  (function initTheme(){
    const saved = localStorage.getItem('theme');
    const prefersDark = window.matchMedia('(prefers-color-scheme: dark)').matches;
    applyTheme(saved || (prefersDark ? 'dark' : 'light'));
  })();
  $('theme')?.addEventListener('change', e => applyTheme(e.target.value));

  // ---------- Date / Time helpers ----------
  function pad2(n){ return String(n).padStart(2,'0'); }


// ---------- Light schedule helpers (ON time + day hours -> OFF time) ----------
function parseHHMM(str){
  // Accepts "H:MM" or "HH:MM"
  const s = String(str || '').trim();
  const m = s.match(/^(\d{1,2}):(\d{2})$/);
  if (!m) return null;
  const hh = Number(m[1]), mm = Number(m[2]);
  if (!Number.isFinite(hh) || !Number.isFinite(mm)) return null;
  if (hh < 0 || hh > 23 || mm < 0 || mm > 59) return null;
  return hh * 60 + mm;
}

function fmtHHMM(totalMin){
  const t = ((totalMin % 1440) + 1440) % 1440;
  const hh = Math.floor(t / 60);
  const mm = t % 60;
  return `${hh}:${pad2(mm)}`;
}

function clampInt(x, lo, hi){
  const n = Number(x);
  if (!Number.isFinite(n)) return lo;
  return Math.min(hi, Math.max(lo, Math.round(n)));
}
  function getDefaultDateFormatFor(lang){ return lang === 'de' ? 'DD.MM.YYYY' : 'YYYY-MM-DD'; }
  function getDefaultTimeFormatFor(lang){ return lang === 'de' ? 'HH:mm' : 'hh:mm A'; }
  function formatDateWithPattern(d, pat){
    const y=d.getFullYear(), m=pad2(d.getMonth()+1), day=pad2(d.getDate());
    return pat === 'DD.MM.YYYY' ? `${day}.${m}.${y}` : `${y}-${m}-${day}`;
  }
  function formatTimeWithPattern(d, pat){
    let h=d.getHours(); const m=pad2(d.getMinutes()); const s=pad2(d.getSeconds());
    if(pat.includes('A')){ const ap=h>=12?'PM':'AM'; let hh=h%12; if(hh===0) hh=12; hh=pad2(hh); return `${hh}:${m}${pat.includes('ss')?':'+s:''} ${ap}`; }
    const HH=pad2(h); return `${HH}:${m}${pat.includes('ss')?':'+s:''}`;
  }
  function getCurDateFmt(){ return localStorage.getItem('dateFormat') || getDefaultDateFormatFor(currentLang || 'de'); }
  function getCurTimeFmt(){ return localStorage.getItem('timeFormat') || getDefaultTimeFormatFor(currentLang || 'de'); }
  function renderHeaderDateTime(){
    const d=new Date();
    const hd=$('headerDate');
    const ht=$('headerTime');
    if(!hd || !ht) return;
    hd.textContent = formatDateWithPattern(d, getCurDateFmt());
    ht.textContent = formatTimeWithPattern(d, getCurTimeFmt());
  }
  $('dateFormat')?.addEventListener('change', e => { localStorage.setItem('dateFormat', e.target.value); renderHeaderDateTime(); });
  $('timeFormat')?.addEventListener('change', e => { localStorage.setItem('timeFormat', e.target.value); renderHeaderDateTime(); });
  setInterval(renderHeaderDateTime, 1000);

  // ---------- Date input localization ----------
  const DATE_INPUT_IDS = ['webGrowStart','webFloweringStart','webDryingStart'];

  function setDateInputsLang(lang) {
    DATE_INPUT_IDS.forEach(id => {
      const el = document.getElementById(id);
      if (el) el.setAttribute('lang', lang || 'de');
    });
  }

  function updateDatePreview(inputId, previewId) {
    const inp = document.getElementById(inputId);
    const prev = document.getElementById(previewId);
    if (!inp || !prev) return;
    const val = inp.value;
    if (!val) { prev.textContent = '—'; return; }
    const dt = new Date(val + 'T12:00:00');
    prev.textContent = formatDateWithPattern(dt, getCurDateFmt());
  }

  function rerenderDatePreviews() {
    updateDatePreview('webGrowStart', 'prevGrowStart');
    updateDatePreview('webFloweringStart', 'prevFloweringStart');
    updateDatePreview('webDryingStart', 'prevDryingStart');
  }

  // Initial setup
  rerenderDatePreviews();

  // Event listeners for date inputs
  document.getElementById('dateFormat')?.addEventListener('change', e => {
    localStorage.setItem('dateFormat', e.target.value);
    renderHeaderDateTime();
    setDateInputsLang(currentLang);
    rerenderDatePreviews();
  });

  // beim Sprachwechsel -> applyTranslations patchen
  (function patchApplyTranslations(){
    const _origApply = applyTranslations;
    applyTranslations = function(){
      _origApply();
      setDateInputsLang(currentLang);
      rerenderDatePreviews();
    };
  })();

  // ---------- Temperature unit ----------
  function getTempUnit(){ return localStorage.getItem('tempUnit') || (currentLang === 'en' ? 'F' : 'C'); }
  function setTempUnit(unit){
    localStorage.setItem('tempUnit', unit);
    const sel=$('tempUnit'); if(sel && sel.value !== unit) sel.value = unit;
  }
  $('tempUnit')?.addEventListener('change', e => setTempUnit(e.target.value));

  // ---------- i18n ----------
  let manifest=null, I18N_RAW={}, I18N={}, currentLang='de';
  function readJsonTag(id){
    const el=$(id); if(!el) throw new Error('Missing tag: '+id);
    return JSON.parse(el.textContent.trim());
  }
  function buildLanguageSelect(activeCode){
    const sel=$('language'); if(!sel) return;
    sel.innerHTML='';
    (manifest.languages || [{code:'de',name:'Deutsch'},{code:'en',name:'English'}]).forEach(({code,name})=>{
      const opt=document.createElement('option'); opt.value=code; opt.textContent=name||code.toUpperCase(); sel.appendChild(opt);
    });
    if(activeCode) sel.value=activeCode;
    sel.onchange = e => setLanguage(e.target.value);
  }
  function applyTranslations(){
    document.querySelectorAll('[data-i18n]').forEach(el=>{
      const key=el.getAttribute('data-i18n');
      const attr=el.getAttribute('data-i18n-attr');
      const val=I18N[key];
      if(val!==undefined){ if(attr){ el.setAttribute(attr,val); } else { el.textContent=val; } }
    });
    rerenderDatePreviews();
    const df=$('dateFormat');
    if(df){ const saved=localStorage.getItem('dateFormat')||getDefaultDateFormatFor(currentLang); if(df.value!==saved) df.value=saved; }
    const tf=$('timeFormat');
    if(tf){ const saved=localStorage.getItem('timeFormat')||getDefaultTimeFormatFor(currentLang); if(tf.value!==saved) tf.value=saved; }
    const tu=$('tempUnit'); if(tu){ const savedTU=getTempUnit(); if(tu.value!==savedTU) tu.value=savedTU; }
    renderHeaderDateTime();
  }
  function setLanguage(code){
    try { I18N_RAW = readJsonTag('i18n'); }
    catch { I18N_RAW = {}; }

    currentLang = code || 'de';

    // flatten: I18N[key] = string for currentLang (with fallbacks)
    I18N = {};
    for (const [k, v] of Object.entries(I18N_RAW)) {
      if (v && typeof v === 'object') {
        I18N[k] = v[currentLang] ?? v.de ?? v.en;
      } else {
        // allow legacy plain strings if any slipped in
        I18N[k] = v;
      }
    }
    
    localStorage.setItem('lang', currentLang);
    if(!localStorage.getItem('dateFormat')) localStorage.setItem('dateFormat', getDefaultDateFormatFor(currentLang));
    if(!localStorage.getItem('timeFormat')) localStorage.setItem('timeFormat', getDefaultTimeFormatFor(currentLang));
    if(!localStorage.getItem('tempUnit'))   localStorage.setItem('tempUnit', currentLang === 'en' ? 'F' : 'C');

    validateI18n();
    applyTranslations();

    const sel=$('language'); if(sel && sel.value !== currentLang) sel.value = currentLang;
  }
  (function initI18n(){
    try{ manifest = readJsonTag('i18n-manifest'); }
    catch{ manifest = { default:'de', languages:[{code:'de',name:'Deutsch'},{code:'en',name:'English'}] }; }
    const urlLang   = new URLSearchParams(location.search).get('lang');
    const savedLang = localStorage.getItem('lang');
    const initial   = urlLang || savedLang || (manifest.default || 'de');
    buildLanguageSelect(initial);
    setLanguage(initial);
  

async function startNewGrow(){
  const msgObj = (typeof I18N_RAW==='object' && I18N_RAW) ? I18N_RAW['runsetting.newGrowConfirm'] : null;
  const msg = (msgObj && (msgObj[currentLang] || msgObj['de'] || msgObj['en'])) ||
              'Neuer Grow starten? (setzt Startdatum auf heute, löscht Blüte/Trocknung, setzt Energie zurück)';
  if (!confirm(msg)) return;

  try{
    showStatusHint((I18N_RAW['runsetting.newGrowWorking']?.[currentLang] || I18N_RAW['runsetting.newGrowWorking']?.de || 'Setze neuen Grow...'), {durationMs: 8000});
    const r = await fetch('/api/newgrow', { method:'POST' });
    if(!r.ok) throw new Error('HTTP '+r.status);
    const d = await r.json();
    if(!d.ok) throw new Error('API');
    // Update UI fields immediately
    const gs = document.getElementById('webGrowStart'); if(gs) gs.value = d.startDate || '';
    const fs = document.getElementById('webFloweringStart'); if(fs) fs.value = '';
    const ds = document.getElementById('webDryingStart'); if(ds) ds.value = '';
    const ps = document.getElementById('phaseSelect');
    if(ps){ ps.value = '1'; ps.dispatchEvent(new Event('change')); }

    showStatusHint((I18N_RAW['runsetting.newGrowDone']?.[currentLang] || I18N_RAW['runsetting.newGrowDone']?.de || 'Neuer Grow gesetzt.'), {durationMs: 6000});
  }catch(e){
    console.warn(e);
    showStatusHint((I18N_RAW['runsetting.newGrowError']?.[currentLang] || I18N_RAW['runsetting.newGrowError']?.de || 'Fehler beim Setzen des neuen Grows.'), {durationMs: 9000});
  }
}



  // expose for onclick handlers
  window.startNewGrow = startNewGrow;
})();

  // ---------- i18n validation ----------
  function validateI18n(){
    // nur prüfen, wenn wir das RAW-Objekt haben
    if (!I18N_RAW || typeof I18N_RAW !== 'object') return;

    const missingKey = new Set();
    const missingLang = new Set();

    document.querySelectorAll('[data-i18n]').forEach(el => {
      const key = el.getAttribute('data-i18n');
      if (!key) return;

      const entry = I18N_RAW[key];

      // Key fehlt komplett
      if (typeof entry === 'undefined') {
        missingKey.add(key);
        return;
      }

      // Entry ist {de,en} aber aktuelle Sprache fehlt
      if (entry && typeof entry === 'object') {
        const val = entry[currentLang];
        if (typeof val === 'undefined' || val === null || String(val).trim() === '') {
          missingLang.add(`${key} (missing "${currentLang}")`);
        }
      }
    });

    if (missingKey.size) {
      console.warn(`[i18n] Missing keys (${missingKey.size}):`, Array.from(missingKey).sort());
    }
    if (missingLang.size) {
      console.warn(`[i18n] Missing translations (${missingLang.size}):`, Array.from(missingLang).sort());
    }
  }

  const shellyMap = {
    main: ['shelly-main-switch-state', 'shellyMainInfo', 'settings.shelly.main'],
    light: ['shelly-light-switch-state', 'shellyLightInfo', 'settings.shelly.light'],
    heater: ['shelly-heater-switch-state', 'shellyHeaterInfo', 'settings.shelly.heater'],
    humidifier: ['shelly-humidifier-switch-state', 'shellyHumidifierInfo', 'settings.shelly.humidifier'],
    fan: ['shelly-fan-switch-state', 'shellyFanInfo', 'settings.shelly.fan'],
    exhaust: ['shelly-exhaust-switch-state', 'shellyExhaustInfo', 'settings.shelly.exhaust']
  };

  function fmtNum(value, digits, unit) {
    return isNum(value) ? `${value.toFixed(digits)} ${unit}` : 'n/a';
  }

  function fmtStateValue(value) {
    return value == null || value === '' ? '—' : String(value);
  }

  function setSwitchWithMetrics(baseId, status, power, totalWh, costEur) {
    const cfg = shellyMap[baseId];
    if (!cfg) return;

    const [stateId, infoId, prefix] = cfg;
    const stateEl = document.getElementById(stateId);
    const infoEl = document.getElementById(infoId);

    const p = fmtNum(power, 1, 'W');
    const kwh = isNum(totalWh) ? `${(totalWh / 1000).toFixed(3)} kWh` : 'n/a';
    const eur = fmtNum(costEur, 2, '€');

    if (stateEl) {
        stateEl.classList.toggle('shelly-on', !!status);
        stateEl.classList.toggle('shelly-off', !status);
        stateEl.innerHTML = `
            <div>${p}</div>
            <div class="sub">${kwh} - ${eur}</div>
        `;
    }

    if (infoEl && window._lastStateObj) {
        const s = window._lastStateObj;
        infoEl.textContent =
            `${s[`${prefix}.ip`] || '—'} | ` +
            `Gen ${fmtStateValue(s[`${prefix}.gen`])} | ` +
            `ON ${fmtStateValue(s[`${prefix}.on`])} | ` +
            `OFF ${fmtStateValue(s[`${prefix}.off`])}`;
    }
  }  

  function updateRelayStates(relays) {
    if (!Array.isArray(relays)) return;

    for (let i = 0; i < RELAY_COUNT; i++) {
      if (i < relays.length && typeof relays[i] === 'object') {
        relayStates[i] = !!relays[i].state;
      } else {
        relayStates[i] = false;
      }
    }

    window.updateRelayButtons();
  }

  // ---------- Sensor fetch (/api/state) ----------
  let sensorFetchInFlight = false;

  async function updateSensorValues() {
    if (sensorFetchInFlight) return;
    sensorFetchInFlight = true;

    function isNum(v) {
      return typeof v === 'number' && Number.isFinite(v);
    }

    function formatNum(v, digits = 1, fallback = 'n/a') {
      return isNum(v) ? v.toFixed(digits) : fallback;
    }

    function safeText(v, fallback = 'n/a') {
      return (v === null || v === undefined || v === '') ? fallback : String(v);
    }

    function setText(id, value) {
      const el = document.getElementById(id);
      if (el) el.textContent = value;
    }

    function buildRelayArrayFromApiState(data, maxRelays) {
      const relays = [];

      for (let i = 0; i < maxRelays; i++) {
        const stateKey = `relays[${i}].state`;
        const nameKey = `relays[${i}].name`;

        relays.push({
          index: i,
          name: data[nameKey] ?? `Relay ${i + 1}`,
          state: Boolean(data[stateKey]),
          scheduleEnabled: data[`relays[${i}].schedule.enabled`] ?? false,
          scheduleStart: data[`relays[${i}].schedule.start`] ?? 0,
          scheduleEnd: data[`relays[${i}].schedule.end`] ?? 0
        });
      }

      return relays;
    }

    try {
      const response = await fetch('/api/state', { cache: 'no-store' });

      if (!response.ok) {
        console.error('Error retrieving /api/state:', response.status);
        setNA();
        return;
      }

      const data = await response.json();

      // Last update: prefer backend timestamp, fallback to local time
      const capturedIso = data['capturedAt'] || data['sys.capturedAt'] || null;
      if (capturedIso) {
        const dt = new Date(capturedIso);
        setText('capturedSpan', Number.isNaN(dt.getTime()) ? 'N/A' : dt.toLocaleString());
      } else {
        setText('capturedSpan', new Date().toLocaleString());
      }

      const relayCount = Number(data['settings.active_relay_count']) || 4;
      RELAY_COUNT = relayCount;
      applyRelayVisibility(relayCount);

      if (getActivePageId() !== 'status') return;

      // Sensoren aktuell
      setText('tempSpan', formatNum(data['sensors.cur.temperatureC'], 1));
      setText('ext1TempSpan', formatNum(data['sensors.cur.extTempC'], 1));
      setText('humSpan', formatNum(data['sensors.cur.humidityPct'], 1));
      setText('vpdSpan', formatNum(data['sensors.cur.vpdKpa'], 2));

      // Bewässerung
      setText('irrigationSpan', safeText(data['irrigation.runsLeft'], '0'));
      setText('irTimeLeftSpan', safeText(data['irrigation.timeLeft'], '00:00'));

      const tankCm = data['irrigation.tankLevelCm'] ?? data['tankLevelCm'];
      setText('tankCMDistanceSpan', formatNum(tankCm, 1, '–'));
      setText('tankLevelSpan', formatNum(data['irrigation.tankLevelPercent'], 0, '–'));

      // System
      setText('espFreeHeapSpan', safeText(data['sys.freeHeap']));
      setText('espMinFreeHeapSpan', safeText(data['sys.minFreeHeap']));
      setText('espCpuMhzSpan', safeText(data['sys.cpuMhz']));
      setText('espUptimeSpan', safeText(data['sys.uptimeS']));

      // Shelly / Verbraucher
      setSwitchWithMetrics(
        'main',
        data['cur.shelly.main.isOn'],
        data['cur.shelly.main.Watt'],
        data['cur.shelly.main.Wh'],
        data['cur.shelly.main.Cost']
      );

      setSwitchWithMetrics(
        'light',
        data['cur.shelly.light.isOn'],
        data['cur.shelly.light.Watt'],
        data['cur.shelly.light.Wh'],
        data['cur.shelly.light.Cost']
      );

      setSwitchWithMetrics(
        'humidifier',
        data['cur.shelly.humidifier.isOn'],
        data['cur.shelly.humidifier.Watt'],
        data['cur.shelly.humidifier.Wh'],
        data['cur.shelly.humidifier.Cost']
      );

      setSwitchWithMetrics(
        'heater',
        data['cur.shelly.heater.isOn'],
        data['cur.shelly.heater.Watt'],
        data['cur.shelly.heater.Wh'],
        data['cur.shelly.heater.Cost']
      );

      setSwitchWithMetrics(
        'fan',
        data['cur.shelly.fan.isOn'],
        data['cur.shelly.fan.Watt'],
        data['cur.shelly.fan.Wh'],
        data['cur.shelly.fan.Cost']
      );

      setSwitchWithMetrics(
        'exhaust',
        data['cur.shelly.exhaust.isOn'],
        data['cur.shelly.exhaust.Watt'],
        data['cur.shelly.exhaust.Wh'],
        data['cur.shelly.exhaust.Cost']
      );

      // Relays
      const relays = buildRelayArrayFromApiState(data, relayCount);
      if (typeof updateRelayStates === 'function') {
        updateRelayStates(relays);
      }

    } catch (error) {
      console.error('Exception in updateSensorValues():', error?.message || error, error);
      setNA();
    } finally {
      sensorFetchInFlight = false;
    }
  }

  window.updateSensorValues = updateSensorValues;

  // ---------- Shelly light schedule poll (only active on Shelly page) ----------
  window._shellyLightTimer = null;

  function getActivePageId(){
    const p = document.querySelector('.page.active');
    return p ? p.id : '';
  }

  function setNA(){
    setText('tempSpan', 'N/A');
    setText('waterTempSpan', 'N/A');
    setText('humSpan',  'N/A');
    setText('vpdSpan',  'N/A');
    setText('capturedSpan', 'N/A');
    // averages optional
  }
  window._startSensorPoll();     // intervall start
  window.updateSensorValues();
  updateRelayButtons();

  
  // ---------- Embedded Web Log ----------

  // Timer handle for periodic log polling
  let logTimer = null;

  // If true, keep scrolling to bottom when new log data arrives
  let autoScroll = true;

  // Cache of last rendered log text to avoid unnecessary DOM updates/reflows
  let lastWebLogText = '';

  async function fetchWebLog() {
    try {
        // Request latest log buffer from backend
        const r = await fetch('/api/logbuffer', { cache: 'no-store' });
        if (!r.ok) return;

        const t = await r.text();
        const pre = document.getElementById('weblog');
        if (!pre) return;

        // Update DOM only when content actually changed
        if (t !== lastWebLogText) {
            const atBottom =
                Math.abs(pre.scrollTop + pre.clientHeight - pre.scrollHeight) < 5;

            pre.textContent = t || '—';
            lastWebLogText = t;

            // Keep view pinned to bottom only if user was already near bottom
            if (autoScroll && atBottom) {
                pre.scrollTop = pre.scrollHeight;
            }
        }
    } catch (e) {
        console.warn('weblog fetch failed', e);
    }
  }

  const pre = document.getElementById('weblog');
  if (!pre) return;

  // Detect whether user manually scrolled away from bottom
  pre.addEventListener('scroll', () => {
    const nearBottom =
        Math.abs(pre.scrollTop + pre.clientHeight - pre.scrollHeight) < 10;
    autoScroll = nearBottom;
  });

  // Start periodic polling (every 10 seconds to reduce ESP32 heap pressure)
  function startWebLog() {
    if (logTimer) return;

    fetchWebLog(); // initial immediate fetch
    logTimer = setInterval(fetchWebLog, 10000);
  }

  // Stop periodic polling
  function stopWebLog() {
    if (!logTimer) return;

    clearInterval(logTimer);
    logTimer = null;
  }

  // Pause polling when tab/page is hidden; resume only on logging page
  document.addEventListener('visibilitychange', () => {
    const loggingActive =
        document.getElementById('logging')?.classList.contains('active');

    if (document.visibilityState === 'visible' && loggingActive) {
        startWebLog();
    } else {
        stopWebLog();
    }
  });


  // ---------- Shelly settings summary line ----------
  // Reads schedule summary lines from /api/state and shows them on the Shelly settings page.
  async function loadShellyLines() {
    const map = {
      main:  'shellyMainLine',
      light: 'shellyLightLine',
      humidifier: 'shellyHumidifierLine',
      heater: 'shellyHeaterLine'
    };

    try {
      const res = await fetch('/api/state', { cache: 'no-store' });
      if (!res.ok) throw new Error('HTTP ' + res.status);
      const s = await res.json();

      // Update Shelly info lines (if elements exist)
      updateShellyInfoLinesFromState(s);

      const setLine = (key, elId) => {
        const el = document.getElementById(elId);
        if (!el) return;
        const v = s[key];
        el.textContent = (v === null || typeof v === 'undefined') ? '—' : String(v);
      };

      setLine('settings.shelly.main.line',  map.main);
      setLine('settings.shelly.light.line', map.light);
      setLine('settings.shelly.humidifier.line', map.humidifier);
      setLine('settings.shelly.heater.line', map.heater);
    } catch (e) {
      console.warn('[SHELLY][JS] loadShellyLines failed:', e);
    }
  }

  // SPA navigation handler: called when switching pages via sidebar links
  function onPageChanged(activeId) {
    // Status page has lots of grid cards; keep sensor polling fast there,
    // slow down elsewhere to reduce layout work.
    if (activeId === 'status') {
      window._setSensorPollInterval?.(10000);
      // refresh immediately when returning to status
      window.updateSensorValues?.();
    } else {
      window._setSensorPollInterval?.(60000);
    }

    if (activeId === 'logging') startWebLog(); else stopWebLog();

    // SHELLY light schedule poll: keep UI in sync (fixes sporadic updates)
    if (activeId === 'shelly') {
      try { initLightScheduleControls(); } catch(e) {}
      // Immediate update (no waiting for timer)
      try { fetchStateAndUpdateShellyUI(); } catch(e) {}
      if (!window._shellyLightTimer) {
        window._shellyLightTimer = setInterval(() => {
          // Immediate update (no waiting for timer)
      try { fetchStateAndUpdateShellyUI(); } catch(e) {}
        }, 15000);
      }
    } else {
      if (window._shellyLightTimer) {
        clearInterval(window._shellyLightTimer);
        window._shellyLightTimer = null;
      }
    }

    if (activeId === 'shelly') {
      // Load Shelly summary lines (IP | Gen | ON | OFF) from /api/state
      loadShellyLines();
    }

    if (activeId === 'vars') {
      // Load once when opening the page (and on refresh)
      loadVars();
    }

    if (activeId === 'diary') {
      updateDiaryUI();
      loadDiaryList();
    }
  }

  // ---------- Diary list (load + render) ----------
  async function loadDiaryList() {
    const listEl = document.getElementById('diaryList');
    if (!listEl) return;

    // optional: kleine Ladeanzeige
    listEl.innerHTML = `<div class="muted">Loading…</div>`;

    try {
      // <- HIER ggf. Endpoint anpassen:
      const res = await fetch('/api/diary/list', { cache: 'no-store' });
      if (!res.ok) throw new Error('HTTP ' + res.status);

      const data = await res.json();
      const items = Array.isArray(data?.items) ? data.items : [];

      if (!items.length) {
        listEl.innerHTML = `<div class="muted">—</div>`;
        return;
      }

      // Sort: newest first (wenn ts vorhanden)
      items.sort((a,b) => (b.ts || 0) - (a.ts || 0));

      const rows = items.map(it => {
        // Unterstützt ts (unix sec), id als ISO, oder date string
        let dt = null;
        if (typeof it.ts === 'number' && isFinite(it.ts)) dt = new Date(it.ts * 1000);
        else if (typeof it.id === 'string') dt = new Date(it.id);
        else if (typeof it.date === 'string') dt = new Date(it.date);

        const dtStr = dt && !isNaN(dt.getTime()) ? dt.toLocaleString() : (it.id || it.date || '—');
        const phase = it.phase ? escapeHtml(String(it.phase)) : '';
        const note  = it.note ? escapeHtml(String(it.note)) : '';
        const prev  = it.preview ? escapeHtml(String(it.preview)) : note;

        return `
          <div class="diary-row">
            <div class="diary-row-left">
              <div class="diary-date">${escapeHtml(dtStr)}${phase ? ` • <span class="diary-phase">${phase}</span>` : ''}</div>
              <div class="diary-preview">${prev || ''}</div>
            </div>
            <div class="diary-row-actions">
              <button class="mini-btn" onclick="diaryEdit('${escAttr(it.id||'')}', '${escAttr(it.note||'')}', '${escAttr(it.phase||'')}')">Edit</button>
              <button class="mini-btn danger" onclick="diaryDelete('${escAttr(it.id||'')}')">Del</button>
            </div>
          </div>
        `;
      });

      listEl.innerHTML = rows.join('');
    } catch (e) {
      console.warn('[DIARY] list load failed', e);
      listEl.innerHTML = `<div class="muted">Error loading diary</div>`;
    }
  }

  // optional: CSV Download (Button kann einfach window.location nutzen)
  function downloadDiaryCsv(){
    window.location.href = '/api/diary.csv'; // ggf. anpassen
  }



  async function diaryDelete(id) {
    if (!id) return;
    if (!confirm('Eintrag wirklich löschen?')) return;
    try {
      const res = await fetch('/api/diary/delete', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ id: String(id) })
      });
      const data = await res.json().catch(() => null);
      if (!res.ok || !data || !data.ok) throw new Error('delete');
      await loadDiaryList();
    } catch (e) {
      alert('Löschen fehlgeschlagen');
    }
  }

  async function diaryEdit(id, curNote, curPhase) {
    if (!id) return;
    const note = prompt('Notiz bearbeiten:', curNote || '');
    if (note === null) return; // cancelled
    const phase = prompt('Phase (grow|flower|dry) – leer lassen um unverändert zu lassen:', curPhase || '');
    try {
      const res = await fetch('/api/diary/update', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ id: String(id), note: String(note), phase: String(phase || '') })
      });
      const data = await res.json().catch(() => null);
      if (!res.ok || !data || !data.ok) throw new Error('update');
      await loadDiaryList();
    } catch (e) {
      alert('Ändern fehlgeschlagen');
    }
  }

  window.diaryDelete = diaryDelete;
  window.diaryEdit = diaryEdit;

  // expose if you want to call from HTML onclick
  window.loadDiaryList = loadDiaryList;
  window.downloadDiaryCsv = downloadDiaryCsv;

  // Phase-Init
  function initPhaseSelect(){
    const currentPhase = '%PHASE%';
    const sel = document.getElementById('phaseSelect');

    if (!sel) return;

    const validPhases = ['grow', 'flower', 'dry'];
    const phase = validPhases.includes(currentPhase) ? currentPhase : 'grow';

    sel.value = phase;

    const phaseLabel = document.getElementById('currentPhase');
    if (phaseLabel) {
      const phaseNames = {
        grow:   'Wuchs (Grow)',
        flower: 'Blüte (Flower)',
        dry:    'Trocknung (Dry)',
      };
      phaseLabel.textContent = phaseNames[phase] || phase;
    }
  }
  
  

  // ---------- Grow Diary (client-side helpers) ----------
  function parseYmd(str){
    if (!str || str.length < 10) return null;
    const y = Number(str.slice(0,4));
    const m = Number(str.slice(5,7));
    const d = Number(str.slice(8,10));
    if (!y || !m || !d) return null;
    // local midnight
    return new Date(y, m-1, d, 0, 0, 0, 0);
  }

  function daysBetween(start, end){
    if (!start || !end) return null;
    const ms = end.getTime() - start.getTime();
    if (!isFinite(ms)) return null;
    return Math.floor(ms / 86400000);
  }

  function getPhaseCode(){
    const sel = document.getElementById('phaseSelect');
    let v = sel ? sel.value : null;
    // Fallback to the current runtime phase provided by the backend template
    if (!v) v = '%PHASE%';
    if (v === 'flower') return 'flower';
    if (v === 'dry') return 'dry';
    return 'grow';
  }

  function phaseNameFor(code){
    // Use i18n phase strings if present
    const key = (code === 'flower') ? 'runsetting.phase.flower' : (code === 'dry') ? 'runsetting.phase.dry' : 'runsetting.phase.grow';
    return I18N[key] || code;
  }

  function updateDiaryCounter(){
    const ta = document.getElementById('diaryNote');
    const out = document.getElementById('diaryCount');
    if (!ta || !out) return;
    out.textContent = `${ta.value.length}/400`;
  }

  function updateDiaryUI(){
    // Prefer the header grow-info because it is already computed correctly (phase + total grow) and localized.
    const gi = document.querySelector('.grow-info');
    if (gi) {
      const raw = (gi.innerText || gi.textContent || '').trim();
      const lines = raw.split(/\r?\n/).map(s => s.trim()).filter(Boolean);

      const parseLine = (line) => {
        // Supports DE: "... Tag 63 / Woche 9" and EN: "... day 63 / week 9"
        const m = line.match(/(?:Tag|Tage|day|days)\s*(\d+)\s*\/\s*(?:Woche|week)\s*(\d+)/i);
        const day = m ? m[1] : null;
        const week = m ? m[2] : null;
        // label before ":" if present
        const lm = line.match(/^\s*([^:]+):/);
        const label = lm ? lm[1].trim() : '';
        return { day, week, label };
      };

      if (lines.length >= 1) {
        const g = parseLine(lines[0]);
        if (g.day)  setText('diaryGrowDay', g.day);
        if (g.week) setText('diaryGrowWeek', g.week);
      }
      if (lines.length >= 2) {
        const p = parseLine(lines[1]);
        if (p.label) setText('diaryPhaseName', p.label.toUpperCase());
        if (p.day)  setText('diaryPhaseDay', p.day);
        if (p.week) setText('diaryPhaseWeek', p.week);
      }

      // Also keep meta inside the note field in sync
      applyDiaryMetaToTextarea();
      return;
    }

    // compute "total grow" from grow start date input (if present)
    const now = new Date();
    const nowMid = new Date(now.getFullYear(), now.getMonth(), now.getDate(), 0,0,0,0);

    const growStartEl = document.getElementById('webGrowStart');
    const flowerStartEl = document.getElementById('webFloweringStart');
    const dryStartEl = document.getElementById('webDryingStart');

    const growStart = parseYmd(growStartEl ? growStartEl.value : '');
    const flowerStart = parseYmd(flowerStartEl ? flowerStartEl.value : '');
    const dryStart = parseYmd(dryStartEl ? dryStartEl.value : '');

    const gDiff = daysBetween(growStart, nowMid);
    const gDay  = (gDiff === null || gDiff < 0) ? '–' : String(gDiff + 1);
    const gWeek = (gDiff === null || gDiff < 0) ? '–' : String(Math.floor(gDiff / 7) + 1);

    setText('diaryGrowDay', gDay);
    setText('diaryGrowWeek', gWeek);

    const phaseCode = getPhaseCode();
    setText('diaryPhaseName', phaseNameFor(phaseCode));

    let phaseStart = growStart;
    if (phaseCode === 'flower' && flowerStart) phaseStart = flowerStart;
    if (phaseCode === 'dry' && dryStart) phaseStart = dryStart;

    const pDiff = daysBetween(phaseStart, nowMid);
    const pDay  = (pDiff === null || pDiff < 0) ? '–' : String(pDiff + 1);
    const pWeek = (pDiff === null || pDiff < 0) ? '–' : String(Math.floor(pDiff / 7) + 1);

    setText('diaryPhaseDay', pDay);
    setText('diaryPhaseWeek', pWeek);

    // Show the current grow/phase status directly inside the note field
    applyDiaryMetaToTextarea();
    updateDiaryCounter();
    loadDiaryList();
  }

  
  function computeDiaryMetaText(){
    // Prefer the header info because it already reflects the correct start dates and is localized.
    const gi = document.querySelector('.grow-info');
    if (gi) {
      const raw = (gi.innerText || gi.textContent || '').trim();
      const lines = raw.split(/\r?\n/).map(s => s.trim()).filter(Boolean);
      if (lines.length) return lines.join(' | ');
    }

    // Fallback: compute from diary UI fields
    const gW = (document.getElementById('diaryGrowWeek')?.textContent || '–').trim();
    const gD = (document.getElementById('diaryGrowDay')?.textContent  || '–').trim();
    const pW = (document.getElementById('diaryPhaseWeek')?.textContent || '–').trim();
    const pD = (document.getElementById('diaryPhaseDay')?.textContent  || '–').trim();

    const phaseCode = getPhaseCode();
    const phaseName = phaseNameFor(phaseCode);

    const lang = (typeof currentLang === 'string' ? currentLang : 'de');
    const wLabel = (lang === 'en') ? 'Week' : 'Woche';
    const dLabel = (lang === 'en') ? 'Day'  : 'Tag';
    const growLabel = (lang === 'en') ? 'Grow' : 'Grow';

    return `${growLabel}: ${wLabel} ${gW} ${dLabel} ${gD} | ${phaseName}: ${wLabel} ${pW} ${dLabel} ${pD}`;
  }

  function stripDiaryMeta(raw){
    const s = String(raw || '');
    // Remove a leading "(meta ...)" block that contains grow/phase timing hints (localized).
    return s.replace(/^\s*\((?=[^)]+(?:Woche|Week|Grow|Blüte|Flower|Trock|Dry))[^)]*\)\s*/i, '').trimStart();
  }

  function applyDiaryMetaToTextarea(){
    const ta = document.getElementById('diaryNote');
    if (!ta) return;

    const base = stripDiaryMeta(ta.value);
    const meta = computeDiaryMetaText();

    // Only append meta if we have numeric values
    const hasNumbers = /\b\d+\b/.test(meta);
    const nextVal = hasNumbers ? (base.length ? `(${meta})  ${base}` : `(${meta})`) : base;

    // Avoid cursor jumps if nothing changes
    if (ta.value !== nextVal) {
      const wasAtEnd = (ta.selectionStart === ta.value.length && ta.selectionEnd === ta.value.length);
      ta.value = nextVal;
      if (wasAtEnd) {
        ta.selectionStart = ta.selectionEnd = ta.value.length;
      }
      updateDiaryCounter();
    }
  }

// diary events
  document.getElementById('diaryNote')?.addEventListener('input', () => { updateDiaryCounter(); applyDiaryMetaToTextarea(); });

  document.getElementById('diarySaveBtn')?.addEventListener('click', async () => {
    const ta = document.getElementById('diaryNote');
    const status = document.getElementById('diaryStatus');
    if (!ta) return;

    const note = stripDiaryMeta((ta.value || '')).trim();
    const phase = getPhaseCode();

    if (status) status.textContent = '';

    try {
      const res = await fetch('/api/diary/add', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ note, phase, lang: (typeof currentLang === "string" ? currentLang : "de"), meta: computeDiaryMetaText() }),
        cache: 'no-store'
      });

      if (!res.ok) {
        if (res.status === 503 && status) {
          status.textContent = I18N['diary.fsError'] || 'Diary storage is not ready';
        }
        throw new Error('HTTP ' + res.status);
      }

      ta.value = '';
      updateDiaryCounter();

      // refresh list so the new entry appears immediately under the form
      loadDiaryList();

      if (status) status.textContent = I18N['diary.saved'] || 'Saved ✓';
      setTimeout(() => { if (status) status.textContent = ''; }, 2500);
    } catch (e) {
      console.warn('[DIARY] save failed', e);
      if (status && !status.textContent) status.textContent = I18N['diary.error'] || 'Save failed';
    }
  });

  document.getElementById('diaryClearBtn')?.addEventListener('click', async () => {
    const msg = I18N['diary.confirmClear'] || 'Really clear the diary and reformat LittleFS?';
    if (!confirm(msg)) return;

    const msg2 = I18N['diary.confirmClear2'] || 'Are you sure? This manual step formats LittleFS and mounts it again.';
    if (!confirm(msg2)) return;

    const status = document.getElementById('diaryStatus');
    if (status) status.textContent = '';

    try {
      const res = await fetch('/api/diary/clear', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ confirm: 'FORMAT_LITTLEFS' }),
        cache: 'no-store'
      });
      const data = await res.json().catch(() => null);
      if (!res.ok || !data || !data.ok || !data.mounted) throw new Error('HTTP ' + res.status);
      if (status) status.textContent = (I18N['diary.cleared'] || 'Diary cleared, LittleFS formatted and mounted ✓');
      setTimeout(() => { if (status) status.textContent = ''; }, 3500);
      loadDiaryList();
    } catch (e) {
      console.warn('[DIARY] clear/repair failed', e);
      if (status) status.textContent = I18N['diary.error'] || 'Error';
    }
  });

  // update diary KPIs when dates / phase change
  document.getElementById('webGrowStart')?.addEventListener('change', updateDiaryUI);
  document.getElementById('webFloweringStart')?.addEventListener('change', updateDiaryUI);
  document.getElementById('webDryingStart')?.addEventListener('change', updateDiaryUI);
  document.getElementById('phaseSelect')?.addEventListener('change', updateDiaryUI);

document.getElementById('toggleScrollBtn')?.addEventListener('click', () => {
    autoScroll = !autoScroll;
    document.getElementById('toggleScrollBtn').textContent = `AutoScroll: ${autoScroll ? 'ON' : 'OFF'}`;
  });

// ---------- Shelly info line updater (optional DOM elements) ----------
// If these elements exist in index_html.h, they will be filled from /api/state:
//   <div id="shellyMainInfo"></div>
//   <div id="shellyLightInfo"></div>


// Fetch /api/state and update Shelly info lines (and light schedule controls if present).
async function fetchStateAndUpdateShellyUI(){
  try{
    const res = await fetch('/api/state', { cache: 'no-store' });
    if (!res.ok) throw new Error('HTTP ' + res.status);
    const s = await res.json();

    // Always update info lines (these are simple <div> elements)
    updateShellyInfoLinesFromState(s);

    // If the Light schedule controls exist, also prefill them
    try{
      const onSel  = document.getElementById('shellyLightOnTime');
      const daySel = document.getElementById('shellyLightDayHours');
      const offInp = document.getElementById('shellyLightOffTime');
      if (onSel && daySel && offInp) {
        let onStr  = s['settings.shelly.light.on'];
        let offStr = s['settings.shelly.light.off'];

        // Fallback: parse from "settings.shelly.light.line"
        if ((!onStr || !offStr) && typeof s['settings.shelly.light.line'] === 'string') {
          const line = s['settings.shelly.light.line'];
          const mOn  = line.match(/\bON\s+(\d{1,2}:\d{2})\b/i);
          const mOff = line.match(/\bOFF\s+(\d{1,2}:\d{2})\b/i);
          if (!onStr  && mOn)  onStr  = mOn[1];
          if (!offStr && mOff) offStr = mOff[1];
        }

        // Apply ON (normalize H:MM -> HH:MM to match select option values)
        if (typeof onStr === 'string' && onStr.includes(':')) {
          const m = onStr.match(/^(\d{1,2}):(\d{2})$/);

          if (m) {
            const hh = String(
              Math.max(0, Math.min(23, parseInt(m[1], 10)))
            ).padStart(2, '0');

            const mm = m[2];

            onSel.value = `${hh}:${mm}`;
          } else {
            onSel.value = onStr;
          }
        }

        // Derive day hours from ON/OFF if possible (12..23)
        if (typeof onStr === 'string' && typeof offStr === 'string') {
          const onMin = parseHHMM(onStr);
          const offMin = parseHHMM(offStr);
          if (onMin != null && offMin != null) {
            let diff = offMin - onMin;
            if (diff <= 0) diff += 1440;
            const hours = Math.round(diff / 60);
            daySel.value = String(clampInt(hours, 12, 23));
          }
        }

        // Compute OFF from UI values
        const onMin2 = parseHHMM(onSel.value);
        const dayHours2 = clampInt(daySel.value, 12, 23);
        offInp.value = (onMin2 != null) ? fmtHHMM((onMin2 + dayHours2 * 60) % 1440) : '—';
      }
    } catch(e) {}

    return s;
  } catch(e) {
    // No console error by default; uncomment if needed:
    // console.warn('[SHELLY] fetchStateAndUpdateShellyUI failed', e);
    return null;
  }
}
function updateShellyInfoLinesFromState(s){
  try{
    const map = [
      { ids: ['shellyMainInfo'],                         key: 'main'  },
      { ids: ['shellyLightInfo'],                        key: 'light' },
      { ids: ['shellyHumidifierInfo'],                   key: 'humidifier' },
      { ids: ['shellyHeaterInfo'],                       key: 'heater' },
      { ids: ['shellyFanInfo'],                          key: 'fan' },
      { ids: ['shellyExhaustInfo'],                      key: 'exhaust' }
    ];

    for (const it of map) {
      const els = (it.ids || []).map(id => document.getElementById(id)).filter(Boolean);
      if (!els.length) continue;

      const base = `settings.shelly.${it.key}`;
      const line = s[`${base}.line`];

      if (typeof line === 'string' && line.length) {
        for (const el of els) el.textContent = line;
        continue;
      }

      const ip  = s[`${base}.ip`];
      const gen = s[`${base}.gen`];
      const on  = s[`${base}.on`];
      const off = s[`${base}.off`];

      const hasAny = (ip != null || gen != null || on != null || off != null);
      if (!hasAny) {
        for (const el of els) el.textContent = '—';
        continue;
      }

      const genTxt = (gen === null || typeof gen === 'undefined') ? '—' : String(gen);
      const onTxt  = (on  === null || typeof on  === 'undefined') ? '—' : String(on);
      const offTxt = (off === null || typeof off === 'undefined') ? '—' : String(off);
      const txt = `${ip || '—'} | Gen ${genTxt} | ON ${onTxt} | OFF ${offTxt}`;
      for (const el of els) el.textContent = txt;
    }
  } catch(e) {}
}

// ---------- Light schedule UI (Shelly settings page) ----------
// Requires these elements in index_html.h (Shelly page):
//   <select id="shellyLightOnTime"></select>
//   <select id="shellyLightDayHours"></select>
//   <input  id="shellyLightOffTime" readonly>
function initLightScheduleControls(){
  const onSel  = document.getElementById('shellyLightOnTime');
  const daySel = document.getElementById('shellyLightDayHours');
  const offInp = document.getElementById('shellyLightOffTime');
  if (!onSel || !daySel || !offInp) return;

  // Fill ON select (15-minute steps)
  if (!onSel._filled) {
    onSel.innerHTML = '';
    for (let h = 0; h < 24; h++) {
      for (let m = 0; m < 60; m += 15) {
        const opt = document.createElement('option');
        const v = `${pad2(h)}:${pad2(m)}`;
        opt.value = v;
        opt.textContent = v;
        onSel.appendChild(opt);
      }
    }
    onSel._filled = true;
  }

  // Fill Day length select (12..23 hours)
  if (!daySel._filled) {
    daySel.innerHTML = '';
    for (let h = 1; h <= 20; h++) {
      const opt = document.createElement('option');
      opt.value = String(h);
      opt.textContent = String(h);
      daySel.appendChild(opt);
    }
    daySel._filled = true;
  }

  function recalcOff(){
    const onMin = parseHHMM(onSel.value);
    const dayHours = clampInt(daySel.value, 1, 20);
    if (onMin == null) { offInp.value = '—'; return; }
    const offMin = (onMin + dayHours * 60) % 1440;
    offInp.value = fmtHHMM(offMin);
  }

  const markDirty = () => {
    try {
      window.showStatusHint?.(I18N?.['settings.unsaved'] || 'Changes pending – please save');
    } catch(e) {}
  };

  // Recompute OFF time live
  onSel.addEventListener('change', () => { recalcOff(); markDirty(); });
  daySel.addEventListener('change', () => { recalcOff(); markDirty(); });

  // Initial calc
  recalcOff();
}

// Prefill ON/OFF from /api/state (read-only device schedule).
// This uses /api/state values:
//   settings.shelly.light.on  ("H:MM" or "HH:MM")
//   settings.shelly.light.off ("H:MM" or "HH:MM")
async function loadLightScheduleFromState(){
  try{
    // This already updates Shelly info lines + Light schedule controls (if present).
    await fetchStateAndUpdateShellyUI();
  } catch (e) {
    console.warn('[SHELLY] loadLightScheduleFromState failed', e);
  }
}

// ---------- Variables / State page ----------
  let _lastStateObj = null;

  function renderVars(stateObj, filterText) {
    const tbody = document.getElementById('varsTbody');
    if (!tbody) return;
    const f = (filterText || '').trim().toLowerCase();

    // flatten keys and sort for stable UX
    const entries = Object.entries(stateObj || {}).
      sort((a,b) => a[0] < b[0] ? -1 : a[0] > b[0] ? 1 : 0)
      .filter(([k,v]) => !f || k.toLowerCase().includes(f) || String(v).toLowerCase().includes(f));

    if (entries.length === 0) {
      tbody.innerHTML = '<tr><td colspan="2">—</td></tr>';
      return;
    }

    const rows = entries.map(([k, v]) => {
      // JSON values can be numbers, booleans, strings, null
      const val = (v === null || typeof v === 'undefined') ? 'null' : String(v);
      return `<tr><td>${escapeHtml(k)}</td><td>${escapeHtml(val)}</td></tr>`;
    });
    tbody.innerHTML = rows.join('');
  }

  async function loadVars() {
    const meta = document.getElementById('varsMeta');
    const search = document.getElementById('varsSearch');
    if (meta) meta.textContent = 'Loading…';
    try {
      const res = await fetch('/api/state', { cache: 'no-store' });
      if (!res.ok) throw new Error('HTTP ' + res.status);
      const obj = await res.json();
      _lastStateObj = obj;
      const n = obj ? Object.keys(obj).length : 0;
      if (meta) meta.textContent = `${n} values • updated ${new Date().toLocaleTimeString()}`;
      renderVars(obj, search ? search.value : '');
    } catch (e) {
      console.error('[VARS] load failed', e);
      if (meta) meta.textContent = 'Error loading /api/state';
    }
  }

  document.getElementById('varsRefreshBtn')?.addEventListener('click', loadVars);
  document.getElementById('varsSearch')?.addEventListener('input', (e) => renderVars(_lastStateObj, e.target.value));

  // Initial call to set the correct page on load
  const initiallyActive = document.querySelector('.page.active')?.id || 'status';
  onPageChanged(initiallyActive);
  updateRelayButtons();


  // Initialize Light schedule controls once on boot as well.
  // Even if the Shelly page isn't active yet, the elements exist in the DOM (hidden),
  // so we can safely prepare them here.
  try {
    initLightScheduleControls();
    loadLightScheduleFromState();
  } catch(e) {
    console.warn('[SHELLY] light schedule init failed', e);
  }


}); // end DOMContentLoaded

//---------- save Runsettings ----------
window.saveRunsettingsAll = async function () {
  try {
    const val = (id, fallback = "") => {
      const el = document.getElementById(id);
      return el ? (el.value ?? fallback) : fallback;
    };

    const checked = (id) => {
      const el = document.getElementById(id);
      return !!el?.checked;
    };

    let maxRelay = 4;
    try {
      const relayCountEl = document.getElementById("webRelayCount");
      const relayCount = parseInt(relayCountEl?.value || "4", 10);
      if (Number.isFinite(relayCount)) {
        maxRelay = Math.max(4, Math.min(5, relayCount));
      }
    } catch (e) {}

    const relays = [];
    for (let relay = 1; relay <= maxRelay; relay++) {
      const onMinRaw = parseInt(document.getElementById(`espRelay${relay}OnMin`)?.value || "0", 10);
      const offMinRaw = parseInt(document.getElementById(`espRelay${relay}OffMin`)?.value || "0", 10);

      relays.push({
        relay,
        enabled: checked(`espRelay${relay}Enabled`),
        ifLightOff: checked(`espRelay${relay}IfLightOff`),
        onMin: Number.isFinite(onMinRaw) ? Math.max(0, Math.min(59, onMinRaw)) : 0,
        offMin: Number.isFinite(offMinRaw) ? Math.max(0, Math.min(59, offMinRaw)) : 0
      });
    }

    const payload = {
      webGrowStart: val("webGrowStart"),
      webFloweringStart: val("webFloweringStart"),
      webDryingStart: val("webDryingStart"),
      webCurrentPhase: val("phaseSelect"),

      webTargetTemp: val("webTargetTemp"),
      webOffsetLeafTemp: val("webOffsetLeafTemp"),

      webMinVPDMonitoring: checked("webMinVPDMonitoring"),
      webTargetVPD: val("webTargetVPD"),
      webMinVPD: val("webMinVPD"),
      webHysteresis: val("webHysteresis"),

      webAmountOfWater: val("webAmountOfWater"),
      webTimePerTask: val("webTimePerTask"),
      webBetweenTasks: val("webBetweenTasks"),
      webIrrigation: val("webIrrigation"),
      webMinTank: val("webMinTank"),
      webMaxTank: val("webMaxTank"),

      webHeatingSource: val("heatingSource"),
      webHeatingRelay: val("heatingRelay"),

      relays
    };

    const res = await fetch("/saverunsettings", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(payload)
    });

    let json = null;
    try { json = await res.json(); } catch (e) {}

    if (!res.ok || json?.ok === false) {
      window.showToast("Failed to save operating settings.", "error");
      return;
    }

    window.showToast("All operating settings saved.", "success");
  } catch (err) {
    console.error("[RUN] saveRunsettingsAll failed:", err);
    window.showToast("Failed to save operating settings.", "error");
  }
};

window.startOtaUpdate = async function () {
    try {
        const res = await fetch("/api/ota/update", {
            method: "POST",
            cache: "no-store"
        });

        if (!res.ok) {
            alert("OTA update failed.");
            return;
        }

        alert("OTA update started. Device will reboot if successful.");
    } 
    catch (err) {
        console.error("[OTA] request failed:", err);
        alert("OTA request failed.");
    }
};

window.resetShellyEnergy = async function (buttonEl) {

    // verhindert Formular-Submit
    if (buttonEl && typeof buttonEl.preventDefault === "function") {
        buttonEl.preventDefault();
    }

    const evt = window.event;
    if (evt && typeof evt.preventDefault === "function") {
        evt.preventDefault();
    }

    try {
        const res = await fetch('/api/shelly/reset-energy', { method: 'POST' });

        if (!res.ok) {
            window.showToast("Failed to reset Shelly energy.", "error");
            return;
        }

        window.showToast("Shelly energy reset.", "success");

    } catch (err) {
        window.showToast("Failed to reset Shelly energy.", "error");
    }
};

// ---------- start watering ----------
function startWatering() {
  fetch('/startWatering', { method: 'POST' })
    .then(() => {
      console.log('Irrigation started');
    })
    .catch(err => {
      console.error('start watering failed:', err);
    });
}

// ---------- ping tank level ----------
function pingTank() {
  fetch('/pingTank', { method: 'POST' })
    .then(() => {
      console.log('Tank level pinged');
    })
    .catch(err => {
      console.error('ping tank failed:', err);
    });
}


// ---------- OTA Update Checker (Settings page) ----------
let otaInfo = null;

async function checkForOtaUpdate() {
  const status = document.getElementById('otaStatus');
  const ver = document.getElementById('otaVersion');
  const log = document.getElementById('otaChangelog');
  const btn = document.getElementById('otaInstallBtn');

  status.textContent = 'Prüfe GitHub Release...';
  ver.textContent = '';
  log.textContent = '';
  btn.disabled = true;
  otaInfo = null;

  try {
    const r = await fetch('/api/ota/check', { cache: 'no-store' });
    if (!r.ok) throw new Error('HTTP ' + r.status);
    const d = await r.json();

    if (!d.ok) throw new Error(d.error || 'Check failed');

    ver.textContent = `Installiert: ${d.currentVersion} | Neueste: ${d.latestVersion}`;

    if (d.updateAvailable) {
      status.textContent = 'Update verfügbar ✅';
      log.textContent = d.changelog || '(kein Changelog)';
      otaInfo = d;
      btn.disabled = false;
    } else {
      status.textContent = 'Kein Update verfügbar.';
      log.textContent = d.changelog || '';
    }
  } catch (e) {
    status.textContent = 'Fehler beim Prüfen: ' + (e.message || e);
  }
}

)rawliteral";
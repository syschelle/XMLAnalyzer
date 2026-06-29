// index_html.h
#pragma once

// ------- i18n: manifest + Sprachen (separat, eigene Routen) -------
static const char I18N_MANIFEST[] PROGMEM = R"json(
{
  "default": "de",
  "languages": [
    { "code": "de", "name": "Deutsch", "dir": "ltr", "file": "de" },
    { "code": "en", "name": "English", "dir": "ltr", "file": "en" }
  ]
}
)json";

static const char I18N_DE[] PROGMEM = R"json(
{
  "app.title": "Mein Webfrontend",
  "settings.themeLight": "Hell",
  "settings.themeDark": "Dunkel",
  "status.title": "Status",
  "status.ok": "System läuft normal ✅"
}
)json";

static const char I18N_EN[] PROGMEM = R"json(
{
  "app.title": "My Web Frontend",
  "settings.themeLight": "Light",
  "settings.themeDark": "Dark",
  "status.title": "Status",
  "status.ok": "System is running ✅"
}
)json";

const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<meta name="viewport" content="width=device-width, initial-scale=1">
<head>
  <title>%CONTROLLERNAME%</title>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="/style.css">
<body>
  <header class="header">
    <button class="hamburger" id="hamburgerBtn" data-i18n="a11y.menu" data-i18n-attr="aria-label" aria-label="Menü öffnen/schließen" aria-expanded="false" aria-controls="sidebar">☰</button>
    <div class="title" data-i18n="app.title">%CONTROLLERNAME%</div>
    <span id="unsavedHint" class="dirty-hint" hidden data-i18n="settings.unsaved"></span>
    <div class='grow-info'>%CURRENTGROW%<BR>%CURRENTPHASE%</div>
    <div id="grow-line" data-i18n-key="info.growLine"></div>
    <div class="datetime">
      <div id="headerDate"></div>
      <div id="headerTime"></div>
    </div>
  </header>
  <div class="layout">
    <nav class="sidebar" id="sidebar">
      <a class="navlink" data-page="status"   data-i18n="nav.status">Status</a>
      <a class="navlink" data-page="diary"   data-i18n="nav.diary">Grow Diary</a>
      <a class="navlink" data-page="runsettings" data-i18n="nav.runsetting">Betriebseinstellungen</a>
      <a class="navlink" data-page="shelly" data-i18n="nav.shelly">Shelly Einstellungen</a>
      <a class="navlink" data-page="message" data-i18n="nav.message">Push-Einstellungen</a>
      <a class="navlink" data-page="settings" data-i18n="nav.settings">Systemeinstellungen</a>
      <a class="navlink" data-page="logging" data-i18n="nav.logging">Systemprotokoll</a>
      <a class="navlink" data-page="vars" data-i18n="nav.vars">Variablen</a>
      <a class="navlink" data-page="ota" data-i18n="nav.ota">OTA Update</a>
      <a class="navlink" data-page="factory" data-i18n="nav.factory">Werkseinstellungen</a>
    </nav>

  <div class="overlay" id="overlay"></div>

  <main class="content" id="content">

    <!-- status section -->
    <section id="status" class="page active card">
      <h1 data-i18n="status.title">Status</h1>
      <!-- Letztes Update direkt unter dem Statustext -->
      <p class="last-update">
        <span data-i18n="status.updated">Letztes Update:</span>
        <span id="capturedSpan">--</span>
      </p>
      <div class="spacer"></div>
      <h2 data-i18n="status.currentValues">aktuelle Werte</h2>
      <!-- 3 values side by side -->
      <div class="metrics-row">
        <div class="metric">
          <div class="twoinone-label">
            <div class="metric-label" data-i18n="status.temperature">Temperatur</div>
            <div class="metric-value">
              <span id="tempSpan">–</span><span class="unit">°C</span>
            </div>
          </div>
          <div class="spacer"></div>
          <div class="metric-sub">
            <div class="twoinone-label">
              <span data-i18n="status.targetTemp">Soll</span>
              <div class="metric-value">
                <span id="targetTempStatus">%TARGETTEMPERATURE%</span> <span class="unit">°C</span>
              </div>
            </div>
          </div>
          <div class="spacer"></div>
          <div class="metric-submetric">
            <div class="twoinone-label">
              <div class="metric-label">%DS18B20NAME%</div>
              <div class="metric-value">
                <span id="ext1TempSpan">%WATERTEMPERATURE%</span><span class="unit">°C</span>
            </div>
          </div>
        </div>
        </div>

        <div class="metric">
          <div class="twoinone-label">
            <div class="metric-label" data-i18n="status.humidity">Luftfeuchte</div>
            <div class="metric-value">
              <span id="humSpan">–</span><span class="unit">%</span>
            </div>
          </div>
        </div>

        <div class="metric">
          <div class="twoinone-label">
            <div class="metric-label" data-i18n="status.lastvpd">VPD</div>
            <div class="metric-value">
              <span id="vpdSpan">–</span><span class="unit">kPa</span>
            </div>
          </div>
          <div class="spacer"></div>
          <div class="metric-sub">
            <div class="twoinone-label">
              <span data-i18n="status.targetVpd">Soll</span>
              <div class="metric-value">
                <span id="targetVpdStatus">%TARGETVPD%</span> <span class="unit">kPa</span>
              </div>
            </div>
          </div>
          <div class="spacer"></div>
          <div class="metric-sub">
            <div class="twoinone-label">
              <span data-i18n="runsetting.offsetLeafTemperature">Offset Blatttemperatur:</span>
              <div class="metric-value">
                <span id="leafTempStatus">%LEAFTEMPERATURE%</span> <span class="unit">°C</span>
              </div>
            </div>
          </div>
        </div>
      </div>
      <div class="spacer"></div>

    <h2 data-i18n="status.relayControl">Relais Steuerung</h2>
    <div class="relay-row" id="relayRow">
      <div class="relay-card" data-relay="1">
        <div class="relay-title">%RELAYNAMES1%</div>
        <div class="relay-status" id="relay-Status1"></div>
        <div class="spacer"></div>
        <button class="primary" data-i18n="status.toggleRelay" onclick="toggleRelay(1)">Toggle</button>
      </div>
      <div class="relay-card" data-relay="2">
        <div class="relay-title">%RELAYNAMES2%</div>
        <div class="relay-status" id="relay-Status2"></div>
        <div class="spacer"></div>
        <button class="primary" data-i18n="status.toggleRelay" onclick="toggleRelay(2)">Toggle</button>
      </div>
      <div class="relay-card" data-relay="3">
        <div class="relay-title">%RELAYNAMES3%</div>
        <div class="relay-status" id="relay-Status3"></div>
        <div class="spacer"></div>
        <button class="primary" data-i18n="status.toggleRelay" onclick="toggleRelay(3)">Toggle</button>
      </div>
      <div class="relay-card" data-relay="4">
        <div class="relay-title">%RELAYNAMES4%</div>
        <div class="relay-status" id="relay-Status4"></div>
        <div class="spacer"></div>
        <button class="primary" data-i18n="status.toggleRelay" onclick="toggleRelay(4)">Toggle</button>
      </div>
      <div class="relay-card relay-extra" data-relay="5" id="relayCard5" style="display:none;">
        <div class="relay-title">%RELAYNAMES5%</div>
        <div class="relay-status" id="relay-Status5"></div>
        <div class="spacer"></div>
        <button class="primary" data-i18n="status.toggleRelay" onclick="toggleRelay(5)">Toggle</button>
      </div>
    </div>
    <div class="spacer"></div>
    
    <div class="tile-row" id="irrigationSection" style="display:none;">
      <div class="tile">
        <h2 data-i18n="status.irrigation">Irrigation</h2>
         <div class="relay-row" id="relayRow">
          <div class="relay-card irrigation" data-relay="6" id="irrigationRelay6">
            <div class="relay-title" data-i18n="status.irrigationRelay6">Pumpe 1 Relay 6</div>
            <div class="relay-status" id="relay-Status6"></div>
            <div class="spacer"></div>
            <button type="button" class="primary" data-i18n="status.togglePump10s" onclick="triggerPump10s(6)">Toggle</button>
          </div>

          <div class="relay-card irrigation" data-relay="7" id="irrigationRelay7">
            <div class="relay-title" data-i18n="status.irrigationRelay7">Pumpe 2 Relay 7</div>
            <div class="relay-status" id="relay-Status7"></div>
            <div class="spacer"></div>
            <button type="button" class="primary" data-i18n="status.togglePump10s" onclick="triggerPump10s(7)">Toggle</button>
          </div>

          <div class="relay-card irrigation" data-relay="8" id="irrigationRelay8">
            <div class="relay-title" data-i18n="status.irrigationRelay8">Pumpe 3 Relay 8</div>
            <div class="relay-status" id="relay-Status8"></div>
            <div class="spacer"></div>
            <button type="button" class="primary" data-i18n="status.togglePump10s" onclick="triggerPump10s(8)">Toggle</button>
          </div>

          <!-- watering section -->
          <div class="relay-card" data-relay="watering">
            <div class="relay-title" data-i18n="status.watering">Bewässerung</div>
            <div class="metric-value">
              <span id="irrigationSpan"  >-</span><span class="unit" data-i18n="status.wateringLeft"> verbleibend</span>
            </div>
            <div class="metric-value">
              <span class="unit" data-i18n="status.endIn" >Ende in </span><span id="irTimeLeftSpan"  ></span>
            </div> 
            <div class="spacermini"></div>
            <button class="primary" onclick="startWatering()">Toggle</button>
          </div>
          <div class="relay-card" data-relay="TankFilling">
            <div class="relay-title" data-i18n="status.tank">Tank Füllung</div>
            <div class="metric-value">
              <span id="tankLevelSpan" >–</span><span class="unit" >&nbsp;%</span>
            </div>
            <div class="metric-value">
              <span id="tankCMDistanceSpan" >–</span><span class="unit" >&nbsp;cm</span>
            </div>
            <div class="spacermini"></div>
              <button class="primary" data-i18n="status.pingTank" onclick="pingTank()">Ping</button>
          </div>

        </div>
      </div>
    </div>

    <div class="spacer"></div>
    <h2 data-i18n="status.shellyControl">Shelly Steuerung</h2>
    <div class="relay-row" id="relayRow">
      <div class="relay-card" data-relay="shellyMainSw">
        <div class="relay-title" data-i18n="status.shellyMainSw">Hauptschalter</div>
        <div id="shelly-main-switch-state" class="shelly-status shelly-off"></div>
        <div class="info" id="shellyMainInfo">—</div>
        <button class="primary" data-i18n="status.toggleRelay" onclick="toggleShellyRelay('mainSw')">Toggle</button>
      </div>
      <div class="relay-card" data-relay="shellyLight">
        <div class="relay-title" data-i18n="status.shellyLight">Grow Licht</div>
        <div id="shelly-light-switch-state" class="shelly-status shelly-off"></div>
        <div class="info" id="shellyLightInfo">—</div>
        <button class="primary" data-i18n="status.toggleRelay" onclick="toggleShellyRelay('light')">Toggle</button>
      </div>
      <div class="relay-card" data-relay="shellyHumidifier">
        <div class="relay-title" data-i18n="status.shellyHumidifier">Luftbefeuchter</div>
        <div id="shelly-humidifier-switch-state" class="shelly-status shelly-off"></div>
        <div class="info" id="shellyHumidifierInfo">—</div>
        <button class="primary" data-i18n="status.toggleRelay" onclick="toggleShellyRelay('humidifier')">Toggle</button>
      </div>
      <div class="relay-card" data-relay="shellyHeater">
        <div class="relay-title" data-i18n="status.shellyHeater">Heizung</div>
        <div id="shelly-heater-switch-state" class="shelly-status shelly-off"></div>
        <div class="info" id="shellyHeaterInfo">—</div>
        <button class="primary" data-i18n="status.toggleRelay" onclick="toggleShellyRelay('heater')">Toggle</button>
      </div>
      <div class="relay-card" data-relay="shellyFan">
        <div class="relay-title" data-i18n="status.shellyFan">Lüfter</div>
        <div id="shelly-fan-switch-state" class="shelly-status shelly-off"></div>
        <div class="info" id="shellyFanInfo">—</div>
        <button class="primary" data-i18n="status.toggleRelay" onclick="toggleShellyRelay('fan')">Toggle</button>
      </div>
      <div class="relay-card" data-relay="shellyExhaust">
        <div class="relay-title" data-i18n="status.shellyExhaust">Abluft</div>
        <div id="shelly-exhaust-switch-state" class="shelly-status shelly-off"></div>
        <div class="info" id="shellyExhaustInfo">—</div>
        <button class="primary" data-i18n="status.toggleRelay" onclick="toggleShellyRelay('exhaust')">Toggle</button>
      </div>
    </div>
    
    </section>

    <!-- diary section -->
    <section id="diary" class="page card">
      <h1 data-i18n="diary.title">Grow Diary</h1>

      <div class="diary-grid">
        <div class="diary-kpi">
          <div class="diary-kpi-title" data-i18n="diary.total">Total grow</div>
          <div class="diary-kpi-val">
            <span id="diaryGrowDay">–</span>
            <span class="unit" data-i18n="diary.day">Day</span>
            &nbsp;•&nbsp;
            <span id="diaryGrowWeek">–</span>
            <span class="unit" data-i18n="diary.week">Week</span>
          </div>
        </div>

        <div class="diary-kpi">
          <div class="diary-kpi-title" data-i18n="diary.phase">Phase</div>
          <div class="diary-kpi-val">
            <span id="diaryPhaseName">–</span>
            &nbsp;•&nbsp;
            <span id="diaryPhaseDay">–</span>
            <span class="unit" data-i18n="diary.day">Day</span>
            &nbsp;•&nbsp;
            <span id="diaryPhaseWeek">–</span>
            <span class="unit" data-i18n="diary.week">Week</span>
          </div>
        </div>
      </div>

      <div class="spacer"></div>

      <div class="form-group">
        <label for="diaryNote" data-i18n="diary.note">Note (max 400 characters)</label>
        <textarea id="diaryNote" maxlength="400" rows="4" placeholder="..." data-i18n-attr="placeholder" data-i18n="diary.note.ph"></textarea>
        <div class="diary-foot">
          <span id="diaryCount">0/400</span>
          <span id="diaryStatus" class="muted"> </span>
        </div>
      </div>

      <div class="btn-row">
        <button type="button" class="btn primary" id="diarySaveBtn" data-i18n="diary.save">Save entry</button>
        <a class="btn danger" href="/api/diary.csv" id="diaryDownloadBtn" data-i18n="diary.download">Download CSV</a>
        <button type="button" class="btn danger" id="diaryClearBtn" data-i18n="diary.clear">Clear diary</button>
      </div>

      <div id="diaryList" class="diary-list"></div>
    </section>

    <div class="spacer"></div>
    <!-- shellysettings section -->
    <form action="/saveshellysettings" method="POST">
    <section id="shelly" class="page card">
      <h1 data-i18n="shelly.title">Shelly Einstellungen</h1>
      <h2 data-i18n="status.shellyDevices">Shelly Geräte</h2>
      <div class="form-group">
        <label for="shellyIP" data-i18n="shelly.shellyIPMainSw">Shelly IP Adresse für Hauptschalter:</label>
        <div class="twoinone-label">
          <input name="webShellyMainIP" id="shellyMainSwIP" class="control-sm shelly-ip" maxlength="15" type="text" inputmode="decimal" value="%SHELLYMAINIP%">
          <select name="webShellyMainGen" id="shellyMainSwGen" class="control-sm shelly-other">
            <option value="1" %SHMAINSWKIND1%>Gen1</option>
            <option value="2" %SHMAINSWKIND2%>Gen2</option>
            <option value="3" %SHMAINSWKIND3%>Gen3</option>
          </select>
        </div>
      </div>

      <div class="form-group">
        <label for="shellyIP" data-i18n="shelly.shellyIPLight">Shelly IP Adresse für Pflanzlicht:</label>
        <div class="twoinone-label">
          <input name="webShellyLightIP" id="shellyLightIP" class="control-sm shelly-ip" maxlength="15" type="text" inputmode="decimal" value="%SHELLYLIGHTIP%">
          <select name="webShellyLightGen" id="shellyLightGen" class="control-sm shelly-other">
            <option value="1" %SHLIGHTKIND1%>Gen1</option>
            <option value="2" %SHLIGHTKIND2%>Gen2</option>
            <option value="3" %SHLIGHTKIND3%>Gen3</option>
          </select>
          <select name="webShellyLightOnTime" id="shellyLightOnTime" class="control-sm shelly-other"></select>
          <select name="webShellyLightDayHours" id="shellyLightDayHours" class="control-sm shelly-other"></select>
          <input name="webShellyLightOffTime" id="shellyLightOffTime" class="control-sm shelly-other" type="text" readonly value="—">
        </div>
      </div>

      <div class="form-group">
        <label for="shellyIP" data-i18n="shelly.shellyIPHumidifier">Shelly IP Adresse für Luftbefeuchter:</label>
        <div class="twoinone-label">
          <input name="webShellyHumidifierIP" id="shellyHumidifierIP" class="control-sm shelly-ip" maxlength="15" type="text" inputmode="decimal" value="%SHELLYHUMIDIFIERIP%">
          <select name="webShellyHumidifierGen" id="shellyHumidifierGen" class="control-sm shelly-other">
            <option value="1" %SHHUMIDIFIERKIND1%>Gen1</option>
            <option value="2" %SHHUMIDIFIERKIND2%>Gen2</option>
            <option value="3" %SHHUMIDIFIERKIND3%>Gen3</option>
          </select>
        </div>
      </div>

      <div class="form-group">
        <label for="shellyIP" data-i18n="shelly.shellyIPHeater">Shelly IP Adresse für Heizgerät:</label>
        <div class="twoinone-label">
          <input name="webShellyHeaterIP" id="shellyHeaterIP" class="control-sm shelly-ip" maxlength="15" type="text" inputmode="decimal" value="%SHELLYHEATERIP%">
          <select name="webShellyHeaterGen" id="shellyHeaterGen" class="control-sm shelly-other">
            <option value="1" %SHHEATERKIND1%>Gen1</option>
            <option value="2" %SHHEATERKIND2%>Gen2</option>
            <option value="3" %SHHEATERKIND3%>Gen3</option>
          </select>
        </div>
      </div>

      <div class="form-group">
        <label for="shellyIP" data-i18n="shelly.shellyIPFan">Shelly IP Adresse für Lüfter:</label>
        <div class="twoinone-label">
          <input name="webShellyFanIP" id="shellyFanIP" class="control-sm shelly-ip" maxlength="15" type="text" inputmode="decimal" value="%SHELLYFANIP%">
          <select name="webShellyFanGen" id="shellyFanGen" class="control-sm shelly-other">
            <option value="1" %SHFANKIND1%>Gen1</option>
            <option value="2" %SHFANKIND2%>Gen2</option>
            <option value="3" %SHFANKIND3%>Gen3</option>
          </select>
        </div>
      </div>

      <div class="form-group">
        <label for="shellyIP" data-i18n="shelly.shellyExhaust">Shelly IP Adresse für Abluft:</label>
        <div class="twoinone-label">
          <input name="webShellyExhaustIP" id="shellyExhaustIP" class="control-sm shelly-ip" maxlength="15" type="text" inputmode="decimal" value="%SHELLYEXHAUSTIP%">
          <select name="webShellyExhaustGen" id="shellyExhaustGen" class="control-sm shelly-other">
            <option value="1" %SHEXHAUSTKIND1%>Gen1</option>
            <option value="2" %SHEXHAUSTKIND2%>Gen2</option>
            <option value="3" %SHEXHAUSTKIND3%>Gen3</option>
          </select>
        </div>
      </div>

      <h2 data-i18n="status.shellyAuth">Shelly Authentifizierung</h2>
      <div class="form-group">
        <label for="shellyUsername" data-i18n="shelly.shellyAuthUser">Shelly Benutzername:</label>
        <input name="webShellyUsername" id="shellyUsername" class="control-sm shelly-other" type="text" value="%SHUSER%">
      </div>

      <div class="form-group">
        <label for="shellyPassword" data-i18n="shelly.shellyAuthPassword">Shelly Passwort:</label>
        <input name="webShellyPassword" id="shellyPassword" class="control-sm shelly-other" type="password" value="%SHPASSWORD%">
      </div>
      
      <div class="spacer"></div>
        <button class="primary" id="saveshellysettingsBtn" data-i18n="settings.save">Speichern</button>
    </form>
    </section>

    <!-- runsettings section -->
    <section id="runsettings" class="page card">
      <h1 data-i18n="runsetting.title">Betriebseinstellungen</h1>

      <div class="form-group">
        <div class="tile-right-settings">
          <div class="form-group">
            <button type="button" class="primary" id="saverunsettingsBtn" data-i18n="runsetting.resetkWh" onclick="resetShellyEnergy(this)">Reset Shelly kWh</button>
          </div>
          <div class="form-group">
            <label for="webGrowStart" data-i18n="runsetting.startGrow">Start Grow Date:</label>
            <input id="webGrowStart" name="webGrowStart" type="date" style="width: 170px;" value="%GROWSTARTDATE%">
          </div>
          <div class="form-group">
            <label for="webFloweringStart" data-i18n="runsetting.startFlower">Start Flowering Date:</label>
            <input id="webFloweringStart" name="webFloweringStart" type="date" style="width: 170px;" value="%GROWFLOWERDATE%">
          </div>
          <div class="form-group">
            <label for="webDryingStart" data-i18n="runsetting.startDry">Start Drying Date:</label>
            <input id="webDryingStart" name="webDryingStart" type="date" style="width: 170px;" value="%GROWDRAYINGDATE%">
          </div>
        </div>

        <div class="form-group">
        <label for="phaseSelect" data-i18n="runsetting.phase">Phase:</label>
        <select id="phaseSelect" name="webCurrentPhase" style="width: 170px;">
          <option value="1" %PHASE1_SEL% data-i18n="runsetting.phase.grow">Wuchs</option>
          <option value="2" %PHASE2_SEL% data-i18n="runsetting.phase.flower">Blüte</option>
          <option value="3" %PHASE3_SEL% data-i18n="runsetting.phase.dry">Trocknung</option>
        </select>
        </div>

      <div class="form-group">
        <label for="targetTemp" data-i18n="runsetting.targetTemp">Soll-Temperatur:</label>
        <input name="webTargetTemp" id="webTargetTemp" style="width: 65px;" type="number" step="0.1" min="18" max="30" value="%TARGETTEMPERATURE%">&nbsp;°C
      </div>

      <div class="form-group">
        <label for="leafTemp" data-i18n="runsetting.offsetLeafTemperature">Offset Blatttemperatur:</label>
        <input name="webOffsetLeafTemp" id="webOffsetLeafTemp" style="width: 65px;" type="number" step="0.1" min="-3.0" max="0.0" value="%LEAFTEMPERATURE%">&nbsp;°C
      </div>

      <div class="tile-row relay-extra" id="vpdSettingsSection">
        <h2 data-i18n="runsetting.vpdSettings">VPD-Einstellungen</h2>

        <div class="form-group checkbox">
          <label class="inline-checkbox">
            <input type="checkbox" name="webMinVPDMonitoring" id="webMinVPDMonitoring" %MINVPDMONITORING%>
            <span data-i18n="runsetting.lowVPD.enabled">aktivieren</span>
          </label>
        </div>

        <div class="form-group">
          <label for="targetVPD" data-i18n="runsetting.targetVPD">Soll-VPD:</label>
          <input name="webTargetVPD" id="webTargetVPD" style="width: 65px;" type="number" step="0.01" min="0.50" max="3.50" value="%TARGETVPD%">&nbsp;kPa
        </div>

        <div class="form-group">
          <label for="minVPD" data-i18n="runsetting.minVPD">Offset Min-VPD:</label>
          <input name="webMinVPD" id="webMinVPD" style="width: 65px;" type="number" step="0.01" min="0.00" max="0.30" value="%MINVPD%">&nbsp;kPa
        </div>

        <div class="form-group">
          <label for="hysteresis" data-i18n="runsetting.hysteresis">Hysterese:</label>
          <input name="webHysteresis" id="webHysteresis" style="width: 65px;" type="number" step="0.01" min="0.00" max="0.1" value="%VPDHYSTERESIS%">&nbsp;kPa
        </div>
      </div>

      <div class="tile-row relay-extra" id="irrigationSettingsSection" style="display:none;">
        <h2 data-i18n="runsetting.wateringSettings">Bewässerungseinstellung</h2>

        <div class="form-group">
          <label for="timePerTask" data-i18n="runsetting.timePerTask">Bewässerungszeit pro Task:</label>
          <input name="webTimePerTask" id="webTimePerTask" style="width: 65px;" type="number" step="1" min="1" max="10" value="%TIMEPERTASK%">&nbsp;s&nbsp;(min 1s, max 10s, step 1s)
        </div>

        <div class="form-group">
          <label for="betweenTasks" data-i18n="runsetting.betweenTasks">Pause zwischen Bewässerungen:</label>
          <input name="webBetweenTasks" id="webBetweenTasks" style="width: 65px;" type="number" step="1" min="1" max="10" value="%BETWEENTASKS%">&nbsp;Min&nbsp;(min 1Min, max 10Min, step 1Min)
        </div>

        <div class="form-group">
          <label for="amountOfWater" data-i18n="runsetting.amountOfWater">Wassermenge nach 10 Sekunden:</label>
          <input name="webAmountOfWater" id="webAmountOfWater" style="width: 65px;" type="number" step="5" min="10" max="500" value="%AMOUNTOFWATER%">&nbsp;ml&nbsp;(min 10ml, max 500ml, step 5ml)
        </div>

        <div class="form-group">
          <label for="Irrigation" data-i18n="runsetting.irrigation">Gesamte Bewässerungsmenge:</label>
          <input name="webIrrigation" id="webIrrigation" style="width: 65px;" type="number" step="10" min="100" max="3000" value="%IRRIGATION%">&nbsp;ml&nbsp;(min 100ml, max 3000ml, step 10ml)
        </div>

        <div class="form-group">
          <label for="minTank">min. Tank:</label>
          <input name="webMinTank" id="webMinTank" style="width: 65px;" type="number" value="%MINTANK%">&nbsp;cm
        </div>
      
        <div class="form-group">
          <label for="maxTank">max. Tank:</label>
          <input name="webMaxTank" id="webMaxTank" style="width: 65px;" type="number" value="%MAXTANK%">&nbsp;cm
        </div>
      </div>

      <div class="spacer"></div>

      <!-- Heating Relay Selection -->
      <div class="relay-heating">
        <h2 class="relay-heating-title" data-i18n="runsetting.relayHeating">ESP32 Relay for heating</h2>
        <p class="relay-heating-hint" data-i18n="runsetting.relay.heatingHint">
          Wird ein Relay für die Heizung verwendet, kann es hier konfigutiert werden.
        </p>

        <div class="relay-heat-select">
          <label for="heatingSource" data-i18n="runsetting.relay.heatingSource">Heizungsquelle</label>
          <select name="webHeatingSource" id="heatingSource">
            <option value="0" %HEATSRC0_SEL%>Disabled</option>
            <option value="1" %HEATSRC1_SEL%>ESP Relay</option>
            <option value="2" %HEATSRC2_SEL%>Shelly Plug</option>
          </select>
        </div>

        <div class="relay-heat-select">
          <label for="heatingRelay" data-i18n="runsetting.relay.heatingRelay">
            Heizungs-Relay
          </label>

          <select name="webHeatingRelay" id="heatingRelay" name="heatingRelay">
            <option value="" %HEATRELAY0_SEL% data-i18n="runsetting.relay.none">Keines</option>
            <option value="1" %HEATRELAY1_SEL%>%RELAYNAMES1%</option>
            <option value="2" %HEATRELAY2_SEL%>%RELAYNAMES2%</option>
            <option value="3" %HEATRELAY3_SEL%>%RELAYNAMES3%</option>
            <option value="4" %HEATRELAY4_SEL%>%RELAYNAMES4%</option>
          </select>
        </div>

        <!-- ESP32 Relay Scheduling -->
        <div class="relay-sched">
          <h2 class="relay-sched-title" data-i18n="runsetting.relayScheduling">ESP32 Relay Scheduling</h2>
          <p class="relay-sched-hint" data-i18n="runsetting.relay.minutesHint">
            Minutenformat: <b>0–59</b> (Minute innerhalb der Stunde).
          </p>

          <div class="relay-sched-list">

            <!-- ESP Relay 1 -->
            <div class="relay-sched-row">
              <div class="relay-sched-name">
                <div class="relay-sched-name-label" data-i18n="runsettings.espSchedRelay1">Relay</div>
                <div class="relay-sched-name-value">%RELAYNAMES1%</div>
              </div>

              <div class="sched-field chk">
                <label class="inline-checkbox">
                  <input id="espRelay1Enabled" name="webEspRel1Enbl" type="checkbox" %ESPRELAY1_ENABLED_CHECKED%>
                  <span data-i18n="runsetting.relay.enabledShort">Enabled</span>
                </label>
              </div>

              <div class="sched-field chk">
                <label class="inline-checkbox">
                  <input id="espRelay1IfLightOff" name="webEspRel1ILOff" type="checkbox" %ESPRELAY1_IFLIGHTOFF_CHECKED%>
                  <span data-i18n="runsetting.relay.ifLightOff">wenn Licht aus</span>
                </label>
              </div>

              <div class="sched-field minute">
                <label for="espRelay1OnMin" data-i18n="runsetting.relay.onMinute">Einschaltminute</label>
                <input id="espRelay1OnMin" name="webEspRel1On" type="number" min="0" max="59" step="1" value="%ESPRELAY1_ONMIN%">
              </div>

              <div class="sched-field minute">
                <label for="espRelay1OffMin" data-i18n="runsetting.relay.offMinute">Ausschaltminute</label>
                <input id="espRelay1OffMin" name="webEspRel1Off" type="number" min="0" max="59" step="1" value="%ESPRELAY1_OFFMIN%">
              </div>
            </div>

            <!-- ESP Relay 2 -->
            <div class="relay-sched-row">
              <div class="relay-sched-name">
                <div class="relay-sched-name-label" data-i18n="runsettings.espSchedRelay2">Relay</div>
                <div class="relay-sched-name-value">%RELAYNAMES2%</div>
              </div>

              <div class="sched-field chk">
                <label class="inline-checkbox">
                  <input id="espRelay2Enabled" name="webEspRel2Enbl" type="checkbox" %ESPRELAY2_ENABLED_CHECKED%>
                  <span data-i18n="runsetting.relay.enabledShort">Enabled</span>
                </label>
              </div>

              <div class="sched-field chk">
                <label class="inline-checkbox">
                  <input id="espRelay2IfLightOff" name="webEspRel2ILOff" type="checkbox" %ESPRELAY2_IFLIGHTOFF_CHECKED%>
                  <span data-i18n="runsetting.relay.ifLightOff">wenn Licht aus</span>
                </label>
              </div>

              <div class="sched-field minute">
                <label for="espRelay2OnMin" data-i18n="runsetting.relay.onMinute">Einschaltminute</label>
                <input id="espRelay2OnMin" name="webEspRel2On" type="number" min="0" max="59" step="1" value="%ESPRELAY2_ONMIN%">
              </div>

              <div class="sched-field minute">
                <label for="espRelay2OffMin" data-i18n="runsetting.relay.offMinute">Ausschaltminute</label>
                <input id="espRelay2OffMin" name="webEspRel2Off" type="number" min="0" max="59" step="1" value="%ESPRELAY2_OFFMIN%">
              </div>
            </div>

            <!-- ESP Relay 3 -->
            <div class="relay-sched-row">
              <div class="relay-sched-name">
                <div class="relay-sched-name-label" data-i18n="runsettings.espSchedRelay3">Relay</div>
                <div class="relay-sched-name-value">%RELAYNAMES3%</div>
              </div>

              <div class="sched-field chk">
                <label class="inline-checkbox">
                  <input id="espRelay3Enabled" name="webEspRel3Enbl" type="checkbox" %ESPRELAY3_ENABLED_CHECKED%>
                  <span data-i18n="runsetting.relay.enabledShort">Enabled</span>
                </label>
              </div>

              <div class="sched-field chk">
                <label class="inline-checkbox">
                  <input id="espRelay3IfLightOff" name="webEspRel3ILOff" type="checkbox" %ESPRELAY3_IFLIGHTOFF_CHECKED%>
                  <span data-i18n="runsetting.relay.ifLightOff">wenn Licht aus</span>
                </label>
              </div>

              <div class="sched-field minute">
                <label for="espRelay3OnMin" data-i18n="runsetting.relay.onMinute">Einschaltminute</label>
                <input id="espRelay3OnMin" name="webEspRel3On" type="number" min="0" max="59" step="1" value="%ESPRELAY3_ONMIN%">
              </div>

              <div class="sched-field minute">
                <label for="espRelay3OffMin" data-i18n="runsetting.relay.offMinute">Ausschaltminute</label>
                <input id="espRelay3OffMin" name="webEspRel3Off" type="number" min="0" max="59" step="1" value="%ESPRELAY3_OFFMIN%">
              </div>
            </div>

            <!-- ESP Relay 4 -->
            <div class="relay-sched-row">
              <div class="relay-sched-name">
                <div class="relay-sched-name-label" data-i18n="runsettings.espSchedRelay4">Relay</div>
                <div class="relay-sched-name-value">%RELAYNAMES4%</div>
              </div>

              <div class="sched-field chk">
                <label class="inline-checkbox">
                  <input id="espRelay4Enabled" name="webEspRel4Enbl" type="checkbox" %ESPRELAY4_ENABLED_CHECKED%>
                  <span data-i18n="runsetting.relay.enabledShort">Enabled</span>
                </label>
              </div>

              <div class="sched-field chk">
                <label class="inline-checkbox">
                  <input id="espRelay4IfLightOff" name="webEspRel4ILOff" type="checkbox" %ESPRELAY4_IFLIGHTOFF_CHECKED%>
                  <span data-i18n="runsetting.relay.ifLightOff">wenn Licht aus</span>
                </label>
              </div>

              <div class="sched-field minute">
                <label for="espRelay4OnMin" data-i18n="runsetting.relay.onMinute">Einschaltminute</label>
                <input id="espRelay4OnMin" name="webEspRel4On" type="number" min="0" max="59" step="1" value="%ESPRELAY4_ONMIN%">
              </div>

              <div class="sched-field minute">
                <label for="espRelay4OffMin" data-i18n="runsetting.relay.offMinute">Ausschaltminute</label>
                <input id="espRelay4OffMin" name="webEspRel4Off" type="number" min="0" max="59" step="1" value="%ESPRELAY4_OFFMIN%">
              </div>
            </div>
      
            <!-- ESP Relay 5 -->
            <div class="relay-sched-row relay-extra" id="espRelay5Row" style="display:none;">
                <div class="relay-sched-name">
                    <div class="relay-sched-name-label" data-i18n="runsettings.espSchedRelay5">Relay</div>
                    <div class="relay-sched-name-value">%RELAYNAMES5%</div>
                </div>

                <div class="sched-field chk">
                    <label class="inline-checkbox">
                        <input id="espRelay5Enabled" name="webEspRel5Enbl" type="checkbox" %ESPRELAY5_ENABLED_CHECKED%>
                        <span data-i18n="runsetting.relay.enabledShort">Enabled</span>
                    </label>
                </div>

                <div class="sched-field chk">
                    <label class="inline-checkbox">
                        <input id="espRelay5IfLightOff" name="webEspRel5ILOff" type="checkbox" %ESPRELAY5_IFLIGHTOFF_CHECKED%>
                        <span data-i18n="runsetting.relay.ifLightOff">wenn Licht aus</span>
                    </label>
                </div>

                <div class="sched-field minute">
                    <label for="espRelay5OnMin" data-i18n="runsetting.relay.onMinute">Einschaltminute</label>
                    <input id="espRelay5OnMin" name="webEspRel5On" type="number" min="0" max="59" step="1" value="%ESPRELAY5_ONMIN%">
                </div>

                <div class="sched-field minute">
                    <label for="espRelay5OffMin" data-i18n="runsetting.relay.offMinute">Ausschaltminute</label>
                    <input id="espRelay5OffMin" name="webEspRel5Off" type="number" min="0" max="59" step="1" value="%ESPRELAY5_OFFMIN%">
                </div>
            </div>

          <div class="relay-sched-actions">
            <button type="button" class="primary" data-i18n="settings.save" onclick="saveRunsettingsAll()">speichern</button>
          </div>
          
        </div>

    </section> 
    </form>

    <!-- runsettings section -->
    <form action="/savemessagesettings" method="POST">
    <section id="message" class="page card">
      <h1 data-i18n="message.title">Nachrichteneinstellungen</h1>

      <h2 data-i18n="message.pushoverSettings">Pushover Einstellungen</h2>

      <div class="form-group checkbox">
        <label class="inline-checkbox">
         <input type="checkbox" name="webPushoverEnabled" id="webPushoverEnabled" %PUSHOVERENABLED%>
         <span data-i18n="message.enabled">aktivieren</span>
        </label>
      </div>

      <div class="form-group">
        <label for="webPushoverUserKey" data-i18n="message.pushoverUserKey">Pushover Benutzer:</label>
        <input name="webPushoverUserKey" id="webPushoverUserKey" type="text" data-i18n="message.pushoverUserKey.ph" data-i18n-attr="placeholder" style="width: 320px;" value="%PUSHOVERUSERKEY%">
      </div>

      <div class="form-group">
        <label for="webPushoverAppKey" data-i18n="message.pushoverAppKey">Pushover Token:</label>
        <input name="webPushoverAppKey" id="webPushoverAppKey" type="text" data-i18n="message.pushoverAppKey.ph" data-i18n-attr="placeholder" style="width: 320px;" value="%PUSHOVERAPPKEY%">
      </div>

      <div class="form-group">
        <label for="webPushoverDevice" data-i18n="message.pushoverDevice">Pushover Gerät:</label>
        <input name="webPushoverDevice" id="webPushoverDevice" type="text" data-i18n="message.pushoverDevice.ph" data-i18n-attr="placeholder" style="width: 320px;" value="%PUSHOVERDEVICE%">
      </div>

      <h2 data-i18n="message.gotifySettings">Gotify Einstellungen</h2>

      <div class="form-group checkbox">
        <label class="inline-checkbox">
         <input type="checkbox" name="webGotifyEnabled" id="webGotifyEnabled" %GOTIFYENABLED%>
         <span data-i18n="message.enabled">aktivieren</span>
        </label>
      </div>

      <div class="form-group">
        <label for="webGotifyURL" data-i18n="message.gotifyURL">Gotify URL:</label>
        <input name="webGotifyURL" id="webGotifyURL" type="text" data-i18n="message.gotifyUrl.ph" data-i18n-attr="placeholder" style="width: 320px;" value="%GOTIFYURL%">
      </div>

      <div class="form-group">
        <label for="webGotifyToken" data-i18n="message.gotifyToken">Gotify Token:</label>
        <input name="webGotifyToken" id="webGotifyToken" type="text" data-i18n="message.gotifyToken.ph" data-i18n-attr="placeholder" style="width: 320px;" value="%GOTIFYTOKEN%">
      </div>

      <button class="primary" id="saveMessageBtn" data-i18n="settings.save">Speichern</button>
    </section>
    </form>

    <!-- setting section -->
    <form action="/savesettings" method="POST">
      <section id="settings" class="page card">
        <h1 data-i18n="settings.title">Systemeinstellungen</h1>

        <div class="form-group checkbox">
          <label class="inline-checkbox">
            <input type="checkbox" name="webDebugEnable" id="webDebugEnable" %DBG_CHECKED%>
            <span>Debug mode</span>
          </label>
        </div>

        <div class="form-group">
          <label for="webRelayCount" data-i18n="settings.relayBoard">welches Relay-Board?</label>
          <select name="webRelayCount" id="webRelayCount" class="control-sm shelly-other">
            <option value="4" %RELAYCOUNT4_SEL%>4x</option>
            <option value="8" %RELAYCOUNT8_SEL%>8x</option>
          </select>
        </div>

        <div class="form-group">
          <label for="webBoxName" data-i18n="settings.boxName">Boxname:</label>
          <input name="webBoxName" id="webBoxName" type="text" data-i18n="settings.boxName.ph" data-i18n-attr="placeholder" style="width: 320px;" value="%CONTROLLERNAME%">
          </div>

        <div class="form-group">
          <label for="webNTPServer" data-i18n="settings.ntpserver">NTP-Server:</label>
          <input name="webNTPServer" id="webNTPServer" type="text" data-i18n="settings.ntpserver.ph" data-i18n-attr="placeholder" style="width: 250px;" value="%NTPSERVER%">
        </div>

        <div class="form-group">
          <div class="label-inline">
            <label for="webTimeZoneInfo" data-i18n="settings.timeZoneInfo">Zeitzone:</label>
            &nbsp;<a href="https://github.com/nayarsystems/posix_tz_db/blob/master/zones.json" target="_blank" rel="noopener noreferrer">🌐</a>
          </div>
          <input name="webTimeZoneInfo" id="webTimeZoneInfo" type="text" data-i18n="settings.timeZoneInfo.ph" data-i18n-attr="placeholder" style="width: 350px;" value="%TZINFO%">
        </div>

        <div class="form-group">
          <label for="language" data-i18n="settings.language">Sprache:</label>
          <select name="webLanguage" id="language" style="width: 100px;">
            <!-- Optionen vermutlich per JS/i18n gefüllt -->
          </select>
        </div>

        <div class="form-group">
          <label for="theme" data-i18n="settings.theme">Theme:</label>
          <select name="webTheme" id="theme" style="width: 100px;">
            <option value="light" data-i18n="settings.themeLight">Hell</option>
            <option value="dark"  data-i18n="settings.themeDark">Dunkel</option>
          </select>
        </div>

        <div class="form-group">
          <label for="dateFormat" data-i18n="settings.dateFormat">Datumsformat:</label>
          <select name="webDateFormat" id="dateFormat" style="width: 140px;">
            <option value="YYYY-MM-DD" data-i18n="settings.df_ymd">YYYY-MM-DD</option>
            <option value="DD.MM.YYYY" data-i18n="settings.df_dmy">DD.MM.YYYY</option>
          </select>
        </div>

        <div class="form-group">
          <label for="timeFormat" data-i18n="settings.timeFormat">Zeitformat:</label>
          <select name="webTimeFormat" id="timeFormat" style="width: 100px;">
            <option value="24" data-i18n="settings.tf_HHmm">24h</option>
            <option value="12" data-i18n="settings.tf_hhmma">12h AM/PM</option>
          </select>
        </div>

        <div class="form-group">
          <label for="tempUnit" data-i18n="settings.tempUnit">Temperatur-Einheit:</label>
          <select name="webTempUnit" id="tempUnit" style="width: 140px;">
            <option value="C" data-i18n="settings.celsius">°C (Celsius)</option>
            <option value="F" data-i18n="settings.fahrenheit">°F (Fahrenheit)</option>
          </select>
        </div>

        <h2 data-i18n="settings.powerPrice">Preis pro kWh</h2> 
        <div class="form-group">
          <label for="webPowerPriceKwh">Power price (€/kWh):</label>
          <input name="webPowerPriceKwh" id="webPowerPriceKwh" type="number" step="0.01" min="0" style="width: 120px;" value="%POWERPRICEKWH%">
        </div>

        <h2 data-i18n="settings.DS18B20">DS18B20 Sensor</h2>        
        <div class="form-group checkbox">
          <label class="inline-checkbox">
           <input type="checkbox" name="webDS18B20Enable" id="webDS18B20Enable" %DS18B20ENABLE%>
           <span data-i18n="settings.enabled">aktivieren</span>
          </label>
        </div>

        <div class="form-group">
          <input name="webDS18B20Name" id="webDS18B20Name" type="text" data-i18n="settings.DS18B20Name.ph" data-i18n-attr="placeholder" style="width: 250px;" maxlength="400" value="%DS18B20NAME%">
        </div>

        <h2 data-i18n="settings.relaySettings">Relais Einstellungen</h2>
        <div class="form-group">
          <label for="webRelay1" data-i18n="settings.relay1">Relay 1:</label>
          <input name="webRelayName1" id="webRelayName1" type="text" data-i18n="settings.relay1.ph" data-i18n-attr="placeholder" style="width: 120px;" maxlength="15" value="%RELAYNAMES1%">
        </div>

        <div class="form-group">
          <label for="webRelay2" data-i18n="settings.relay2">Relay 2:</label>
          <input name="webRelayName2" id="webRelayName2" type="text" data-i18n="settings.relay2.ph" data-i18n-attr="placeholder" style="width: 120px;" maxlength="15" value="%RELAYNAMES2%">
        </div>

        <div class="form-group">
          <label for="webRelay3" data-i18n="settings.relay3">Relay 3:</label>
          <input name="webRelayName3" id="webRelayName3" type="text" data-i18n="settings.relay3.ph" data-i18n-attr="placeholder" style="width: 120px;" maxlength="15" value="%RELAYNAMES3%">
        </div>

        <div class="form-group">
          <label for="webRelay4" data-i18n="settings.relay4">Relay 4:</label>
          <input name="webRelayName4" id="webRelayName4" type="text" data-i18n="settings.relay4.ph" data-i18n-attr="placeholder" style="width: 120px;" maxlength="15" value="%RELAYNAMES4%">
        </div>

        <div class="form-group relay-extra" id="settingsRelay5Row" style="display:none;">
          <label for="webRelayName5" data-i18n="settings.relay5">Relay 5:</label>
          <input name="webRelayName5" id="webRelayName5" type="text" data-i18n="settings.relay5.ph" data-i18n-attr="placeholder" style="width: 120px;" maxlength="15" value="%RELAYNAMES5%">
        </div>

        <button class="primary" id="saveSettingsBtn" data-i18n="settings.save">Speichern</button>
      </section>
    </form>

    <!-- system log section -->
    <section id="logging" class="page card">
      <h1 data-i18n="logging.title">Systemprotokoll</h1>
      <div class="weblog-card">
        <div class="weblog-head">
          <strong>System-Log</strong>
          <div class="weblog-actions">
            <a class="btn" href="/download/log">CSV/TXT Download</a>
            <button class="btn" id="toggleScrollBtn" type="button">AutoScroll: ON</button>
            <button class="btn" id="clearLogBtn" type="button" title="Log löschen">Clear</button>
          </div>
        </div>
        <pre id="weblog" class="weblog" aria-live="polite" aria-label="Laufende Logausgabe">…</pre>
      </div>
    </section>

    <!-- variables/state section -->
    <section id="vars" class="page card">
      <h1 data-i18n="vars.title" data-i18n="vars.variables">Variablen</h1>
      <p class="hint" data-i18n="vars.hint" data-i18n="vars.debugHint">Debug-Ansicht: alle registrierten Werte (automatisch aus /api/state). Tokens/Passwörter werden maskiert.</p>

      <div class="vars-toolbar">
        <input id="varsSearch" class="input" type="search" placeholder="Search…" aria-label="Search variables">
        <button class="btn" id="varsRefreshBtn" type="button">Refresh</button>
      </div>

      <div id="varsMeta" class="vars-meta">--</div>

      <div class="table-wrap">
        <table class="vars-table" id="varsTable" aria-label="Variables table">
          <thead>
            <tr><th>Key</th><th>Value</th></tr>
          </thead>
          <tbody id="varsTbody">
            <tr><td colspan="2">—</td></tr>
          </tbody>
        </table>
      </div>
    </section>


    <!-- OTA update section -->
    <section id="ota" class="page card">
      <h1 data-i18n="ota.title">OTA Update</h1>
      <p class="hint">
        Firmware-Quelle:
        <code>https://github.com/syschelle/GrowTent/releases/latest/download/firmware.bin</code>
      </p>

      <div id="otaStatus" class="muted">Noch nicht geprüft.</div>
      <div id="otaVersion"></div>
      <pre id="otaChangelog" style="white-space:pre-wrap;"></pre>

      <div class="spacer"></div>
        <button type="button" class="primary" onclick="checkForOtaUpdate()">Auf Update prüfen</button>
      <div class="spacer"></div>
        <button type="button" class="primary" id="otaInstallBtn" onclick="startOtaUpdate()" disabled>Update installieren</button>
    </section>

    <!-- factory reset section -->
    <section id="factory" class="page card">
      <form action="/factory-reset" method="post" id="factoryResetForm">
        <h1 data-i18n="factory.title">Werkseinstellungen</h1>
        <input type="hidden" name="confirm" value="1">
        <button class="primary" id="factoryResetBtn" type="submit" data-i18n="factory.reset">factory reset</button>
      </form>
    </section>
  </main>
  </div>
  
 <script src="/script.js"></script>
</body>
</html>
)rawliteral";

const char* apPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>%CONTENTCONTROLLERNAME%</title>
  <meta charset="UTF-8">
  <link rel="stylesheet" href="/style.css">
</head>
<body>
  <header class="header">
    <button class="hamburger" id="hamburgerBtn" data-i18n="a11y.menu" data-i18n-attr="aria-label" aria-label="Menü öffnen/schließen" aria-expanded="false" aria-controls="sidebar">☰</button>
    <div class="title" data-i18n="app.title">%CONTENTCONTROLLERNAME%</div>
    </div>
  </header>
  <div class="layout">
    <nav class="sidebar" id="sidebar">
      <a class="navlink" data-page="settings"   data-i18n="nav.wifisettings">WIFI Setting</a>
    </nav>

  <div class="overlay" id="overlay"></div>

    <main class="content" id="content">
      <section id="status" class="page active card">
        <form action="/save" method="post">
          <h1 data-i18n="settings.title">WIFI Setting</h1>
          <label for="ssid">WIFI SSID:</label>
          <input type="text" id="ssid" name="ssid" required><br><br>
          <label for="password">WIFI Passwort:</label>
          <input type="password" id="password" name="password" required><br><br>
          <button class="primary" id="saveBtn" data-i18n="settings.save">save & reboot</button>
        </form>
      </section>
      <section id="status" class="page active card">
        <form action="/factory-reset" method="post" id="factoryResetForm">
          <h1 data-i18n="settings.title">Factory Reset</h1>
          <input type="hidden" name="confirm" value="1">
          <button class="primary" id="factoryResetBtn" type="submit" data-i18n="settings.factoryreset.button">factory reset</button>
        </form>
      </section>
    </main>
  </div>
  
 <script src="/script.js"></script>
</body>
</html>
)rawliteral";
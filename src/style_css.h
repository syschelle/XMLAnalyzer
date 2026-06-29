// style_css.h

#pragma once

const char cssContent[] PROGMEM = R"rawliteral(
:root{
      --header:#2c3e50; --side:#34495e; --bg:#f5f5f5; --text:#333;
      --border:#dddddd; --muted:#ffffff; --link:#2c3e50;
      --sidebar-w:200px; --radius:10px;
    }
    :root[data-theme='dark']{
      --header:#111; --side:#1b1b1b; --bg:#121212; --text:#eaeaea;
      --border:#2a2a2a; --muted:#1f1f1f; --link:#9ec1ff;
      --primary-hover: #1e40af;
    }
    .hidden { display:none; }
    *{box-sizing:border-box}
    html{font-size:16px}
    body{margin:0;font-family:system-ui,-apple-system,Segoe UI,Roboto,Arial,sans-serif;background:var(--bg);color:var(--text);line-height:1.45}
    a{color:var(--link); text-decoration: none;}
    h1{font-size:clamp(1.25rem,1.2vw+1rem,1.8rem);margin:0 0 .75rem}
    h2{font-size:clamp(0.6rem,.5vw+1rem,1.1rem);margin:0 0 .75rem;opacity: 0.8}
    p,label,input,select,button{font-size:clamp(.95rem,.4vw+.85rem,1.05rem)}
    /* Header */
    .header{
      position:sticky;top:0;display:flex;align-items:center;justify-content:space-between;
      gap:.75rem;height:56px;padding:0 16px;background:var(--header);color:#fff;z-index:50
    }
    .hamburger{inline-size:40px;block-size:40px;display:inline-flex;align-items:center;justify-content:center;background:transparent;border:0;color:inherit;cursor:pointer;border-radius:var(--radius);font-size:22px}
    .hamburger:focus-visible{outline:2px solid #fff;outline-offset:2px}
    .title{font-weight:600;font-size:clamp(1rem,.6vw+.9rem,1.2rem)}
    .datetime{display:flex;flex-direction:column;align-items:flex-end;font-size:.85rem;line-height:1.2}

    .dirty-hint{
      margin-left:8px;
      background:#ffdf91;
      color:#222;
      padding:3px 10px;
      border-radius:999px;
      font-size:.8rem;
      font-weight:600;
      display:inline-flex;
      align-items:center;
      gap:.4rem;
      box-shadow:0 1px 3px rgba(0,0,0,.15);
    }

    .dirty-hint::before{ content:"⚠️"; }
    :root[data-theme="dark"] .dirty-hint{
      background:#3d320e; color:#ffe9b0;
    }

    /* Sidebar / Overlay */
    .sidebar{position:fixed;inset:0 auto 0 0;inline-size:var(--sidebar-w);background:var(--side);color:#fff;transform:translateX(-100%);transition:transform .3s ease;padding-top:62px;z-index:40}
    .sidebar--open{transform:translateX(0)}
    .navlink{display:block;padding:12px 18px;text-decoration:none;color:#fff}
    .navlink:hover{background:rgba(255,255,255,.08)}
    .navlink[aria-current='page']{background:rgba(255,255,255,.12)}
    .navlinkside{
      display:block; padding: 12px 14px; color: #1e40af; text-decoration: none; transition: background .2s;
    }
    .navlinkside:hover{ background: rgba(255,255,255,.08); text-decoration: underline }
    .navlinkside[aria-current="page"]{ background: rgba(255,255,255,.14) }
    .overlay{position:fixed;inset:0;background:rgba(0,0,0,.35);opacity:0;visibility:hidden;pointer-events:none;transition:opacity .3s ease,visibility 0s linear .3s;z-index:20}
    .overlay--show{opacity:1;visibility:visible;pointer-events:auto;transition-delay:0s}

    .date-row label {
      flex: 1;
      white-space: nowrap;
    }

    .date-row input[type="date"] {
      flex: 0 0 160px; /* feste Breite, anpassbar */
      padding: 4px 6px;
      border: 1px solid #ccc;
      border-radius: 6px;
      background: #fff;
    }

    .weblog-card{
      margin-top:16px;
      background: var(--muted);
      border: 1px solid var(--border);
      border-radius: var(--radius);
      padding: 10px;
      box-shadow: var(--shadow-sm);
    }
    .weblog-head{
      display:flex; align-items:center; justify-content:space-between;
      margin-bottom: 8px;
    }
    .weblog-actions{ display:flex; gap:8px }
    .weblog {
      background-color: #000;       /* tiefes Schwarz */
      color: #00ff00;               /* grelles Grün */
      font-family: "Courier New", Courier, monospace;
      font-size: 0.9rem;
      line-height: 1.4;
      padding: 10px;
      border-radius: 8px;
      border: 1px solid #0f0;
      height: 420px;                /* anpassen wie du willst */
      overflow-y: auto;
      white-space: pre-wrap;
      box-shadow: 0 0 8px #00ff00a0 inset; /* leichter Glow-Effekt */
    }
    .weblog::-webkit-scrollbar {
      width: 10px;
    }
    .weblog::-webkit-scrollbar-track {
      background-color: #001a00;
    }

    .btn{
      display:inline-block;
      padding:6px 10px;
      border-radius: 8px;
      border: 1px solid var(--border);
      background: var(--muted);
      color: var(--text);
      text-decoration: none;
      cursor: pointer;
      transition: transform .15s, filter .15s;
    }
    .btn:hover{ transform: translateY(-1px) }
    .btn:active{ transform: none }

    /* Content / Layout */
    .content{padding:clamp(14px,1.5vw,24px)}
    .page{display:none;animation:fade .2s}
    .page.active{display:block}
    @keyframes fade{from{opacity:0}to{opacity:1}}
    .form-group{margin-block:0 14px}
    label{display:block;margin-block-end:6px}
    input,select{width:100%;padding:10px;border:1px solid var(--border);border-radius:var(--radius);background:var(--muted);color:var(--text)}
    /* button.primary{width:100%;padding:10px;border:0;border-radius:var(--radius);background:var(--header);color:#fff;cursor:pointer} */
    button.primary {
      width: 100%;
      padding: 10px;
      border: 0;
      border-radius: var(--radius);
      background: var(--header);
      color: #fff;
      cursor: pointer;
      transition: all 0.25s ease;
      box-shadow: 0 2px 6px rgba(0,0,0,0.15);
    }

    .label-inline {
      display: flex;
      align-items: center;
      gap: 6px;
      margin-bottom: 4px; /* sorgt für Abstand nach unten */
    }

    .label-inline label {
      margin: 0;
    }

    .label-inline .tz-link {
      text-decoration: none;
      font-size: 1.1rem;
      opacity: 0.7;
      transition: opacity 0.2s ease;
    }

    .label-inline .tz-link:hover {
      opacity: 1;
    }

    #timeZoneInfo {
      display: block;     /* zwingt neue Zeile */
      width: 260px;
    }

    button.primary:hover {
      transform: translateY(-2px);         /* leichter „Lift“-Effekt */
      box-shadow: 0 4px 10px rgba(0,0,0,0.25);
      filter: brightness(1.05);            /* etwas heller */
    }

    button.primary:active {
      transform: translateY(0);            /* wieder runter beim Klick */
      box-shadow: 0 2px 6px rgba(0,0,0,0.15);
      filter: brightness(.95);
    }

    @media (min-width:1024px){
      .hamburger{display:none}
      .sidebar{transform:none;position:sticky;inset:auto;top:56px;height:calc(100dvh - 56px)}
      .layout{display:grid;grid-template-columns:var(--sidebar-w) 1fr;min-height:calc(100dvh - 56px)}
      .overlay{display:none}
      .content{padding:clamp(18px,2vw,32px)}
    }

    /* Card */
    .card{
      background:var(--muted);
      border:1px solid var(--border);
      border-radius:var(--radius);
      padding:16px
    }

    /* Status: last update under the status text */
    .last-update {
      margin: 6px 0 14px;
      opacity: .9;
    }

    /* Three columns side by side, responsive */
    .metrics-row {
      display: flex;
      gap: 16px;
      flex-wrap: wrap;
    }

    .metric {
      flex: 1 1 200px;              /* wraps on smaller screens */
      min-width: 180px;
      border: 1px solid var(--border, #ddd);
      border-radius: 10px;
      padding: 10px 12px;
      background: var(--muted, #fafafa);
      /* isolate layout/paint to reduce reflow cost on frequent value updates */
      contain: layout paint;
    }

    /* -------- History charts -------- */
    .history-head{display:flex; align-items:center; justify-content:space-between; gap:12px}
    .history-head h2{margin:0;}
    .history-grid{
      display:grid;
      grid-template-columns: 1fr;
      gap:12px;
      margin-top:10px;
      margin-bottom:16px;
    }
    @media (min-width:700px){
      .history-grid{ grid-template-columns: 1fr 1fr; }
    }
    .chart-card{
      border:1px solid var(--border);
      border-radius: var(--radius);
      padding:10px;
      background: var(--muted);
      /* isolate layout/paint to reduce redraw cost */
      contain: layout paint;
    }
    .chart-title{font-weight:600; font-size:.95rem; opacity:.9; margin-bottom:6px}
    .chart-foot {
      display: flex;
      align-items: center;
      gap: 4px;
      font-size: 12px;
      opacity: 0.85;
    }

    .chart-foot .sep {
      margin: 0 6px;
      opacity: 0.5;
    }

    .chart-foot .unit {
      margin-left: 2px;
      opacity: 0.8;
    }
    canvas{width:100%; height:auto; display:block}

    .metric-label {
      font-size: .95rem;
      opacity: .85;
      margin-bottom: 6px;
    }

    .metric-value {
      font-size: clamp(1.15rem, 1.2vw + 0.8rem, 1.45rem);
      font-weight: 700;
      display: flex;
      align-items: baseline;
      gap: 6px;
    }

    .metric-value .unit {
      font-size: 1rem;
      opacity: .8;
    }

    .metric-sub{
      margin-top: 4px;
      font-size: 0.92rem;
      opacity: 0.8;
    }

    .metric-submetric{
      margin-top: 4px;
      font-size: 0.92rem;
    }

    .grow-info {
      font-weight: 500;
      font-size: 1em;
      opacity: 0.9;
    }
    .spacer { height: 16px; }

    .spacermini { height: 8px; }

    .relay-row {
      display: flex;
      gap: 1rem;
      margin-top: 1.5rem;
      flex-wrap: wrap;
    }

    .relay-card {
      flex: 1 1 180px;
      max-width: 320px;
      background: var(--muted);
      border: 1px solid var(--border);
      border-radius: var(--radius);
      padding: 16px;
      box-shadow: 0 1px 3px rgba(0, 0, 0, 0.08);
      min-width: 140px;
      /* isolate layout/paint; these update often */
      contain: layout paint;
    }

    /* Chromium-based browsers: skip rendering off-screen cards */
    @supports (content-visibility: auto){
      .metric, .chart-card, .relay-card{
        content-visibility: auto;
        contain-intrinsic-size: 260px;
      }
    }

    .relay-card.active {
      background-color: #dc3545;
    }
    
    .relay-card.tank-green {
      background-color: #28a745;
    }

    .relay-card.tank-yellow {
      background-color: #f1c40f;
      color: #000;
    }

    .relay-card.tank-red {
      background-color: #dc3545;
      color: #fff;
    }

    .relay-title {
      font-weight: 600;
      margin-bottom: 0.4rem;
    }

    .relay-status {
      font-size: 0.85rem;     /* kleiner als Standard */
      font-weight: 700; 
      height: 36px;
      display: flex;
      padding: 0.2rem 0.6rem;
      border-radius: 6px;
      min-width: 60px;
      text-align: center;
      transition: all 0.2s ease;
    }

    .relay-status.on {
      background: #28a745;
    }

    .relay-status.off {
      background: #dc3545;
    }
       
    .relay-scheduled-label{ flex: 0 0 auto; }

    .relay-scheduled-check{ flex: 0 0 auto; }

    .relay-scheduled-row input[type="number"] {
      width: 80px;
      padding:4px 6px;
    }

    .relay-scheduled-check {
      display: flex;
      align-items: center;
      gap: 4px;
    }

    .relay-scheduled-label {
      display: flex;
      align-items: center;
      gap: 4px;
    }

    .shelly-status {
      font-size: 0.85rem;
      font-weight: 700;
      height: 36px;
      display: flex;
      align-items: center;
      justify-content: center;
      flex-direction: column;
      padding: 0.2rem 0.6rem;
      border-radius: 6px;
      min-width: 60px;
      text-align: center;
      transition: all 0.2s ease;
    }

    .shelly-status .sub {
      font-size: 0.70rem;
      font-weight: 600;
      opacity: 0.95;
    }

    .shelly-on {
      background-color: #28a745;
    }

    .shelly-off {
      background-color: #dc3545;
    }

    .inline-checkbox {
      display: inline-flex;
      gap: 8px;
      align-items: center;
    }

    .twoinone-label {
      display: flex;
      align-items: center;
      gap: 4px;
    }


    /* Inline controls: responsive + compact */
    .twoinone-label{
      display:flex;
      align-items:center;
      gap:10px;
      flex-wrap:wrap;
    }
    .twoinone-label > span{
      white-space:nowrap;
    }
    .twoinone-label input,
    .twoinone-label select{
      flex: 1 1 180px;
      min-width: 140px;
    }
    .control-sm{
      padding:8px;
      border-radius: 8px;
    }
    .control-xs{
      flex: 0 0 auto;
      min-width: 120px;
      max-width: 160px;
    }
    
    .shelly-ip {
      width: 120px; /* adjust as you like */
      max-width: 120px;
    }

    .shelly-other {
      width: 90px; /* adjust as you like */
      max-width: 90px;
    }

    .info {
      font-size: 0.6rem;
      opacity: 0.8;
      margin-bottom: 8px;
    }

    /* --- Variables / State page --- */
    .vars-toolbar{
      display:flex;
      gap:10px;
      align-items:center;
      flex-wrap:wrap;
      margin: 12px 0;
    }
    .vars-meta{
      font-size: 0.85rem;
      opacity: 0.8;
      margin-bottom: 10px;
    }
    .table-wrap{ overflow:auto; border-radius: 12px; border: 1px solid var(--border); }
    .vars-table{
      width:100%;
      border-collapse: collapse;
      font-size: 0.9rem;
      min-width: 520px;
    }
    .vars-table th, .vars-table td{ padding: 10px 12px; border-bottom: 1px solid var(--border); vertical-align: top; }
    .vars-table thead th{ position: sticky; top: 0; background: var(--card); z-index: 1; text-align:left; }
    .vars-table td:first-child{ font-family: ui-monospace, SFMono-Regular, Menlo, Monaco, Consolas, "Liberation Mono", "Courier New", monospace; font-size: 0.85rem; }
    .vars-table td:last-child{ word-break: break-word; }
    @media (max-width: 640px){
      .vars-table{ min-width: 0; }
    }

    .diary-grid{
      display: grid;
      grid-template-columns: 1fr;
      gap: 12px;
    }
    @media (min-width: 720px){
      .diary-grid{ grid-template-columns: 1fr 1fr; }
    }
    .diary-kpi{
      padding: 12px;
      border: 1px solid rgba(127,127,127,0.25);
      border-radius: 12px;
    }
    .diary-kpi-title{
      font-size: 0.9rem;
      opacity: 0.85;
      margin-bottom: 6px;
    }
    .diary-kpi-val{
      font-size: 1.1rem;
      font-weight: 700;
    }
    .diary-foot{
      display:flex;
      justify-content: space-between;
      align-items: center;
      margin-top: 6px;
      font-size: 0.85rem;
      opacity: 0.85;
    }
    .btn-row{
      display:flex;
      flex-wrap: wrap;
      gap: 10px;
      align-items: center;
    }
    .btn.danger{
      padding: 10px;
      border: 0;
      border-radius: var(--radius);
      background: var(--header);
      color: #fff;
      cursor: pointer;
      transition: all 0.25s ease;
      box-shadow: 0 2px 6px rgba(0,0,0,0.15);
    }
    .btn.danger:hover{
      transform: translateY(-2px);         /* leichter „Lift“-Effekt */
      box-shadow: 0 4px 10px rgba(0,0,0,0.25);
      filter: brightness(1.05);            /* etwas heller */
    }

/* -------------------- Grow Diary -------------------- */

#diaryNote{
  width:100%;
  max-width:100%;
  box-sizing:border-box;
  min-height:140px;
  resize:vertical;
}



  .diary-row{
    display:flex;
    justify-content:space-between;
    gap:10px;
    padding:10px;
    border-radius:12px;
    border:1px solid rgba(255,255,255,0.08);
    margin:8px 0;
  }
  .diary-row-left{ flex:1; min-width:0; }
  .diary-date{ font-size:0.9rem; opacity:0.9; margin-bottom:4px; }
  .diary-preview{ font-size:0.95rem; opacity:0.95; word-break:break-word; }
  .diary-row-actions{
    display:flex;
    gap:6px;
    align-items:flex-start;
    flex-shrink:0;
  }
  .mini-btn{
    font-size:0.8rem;
    padding:6px 8px;
    border-radius:10px;
    border:1px solid rgba(255,255,255,0.12);
    background:rgba(255,255,255,0.06);
    color:inherit;
    cursor:pointer;
  }
  .mini-btn:hover{ background:rgba(255,255,255,0.10); }
  .mini-btn.danger{
    border-color: rgba(255,80,80,0.35);
  }

  /* --- ESP32 Relay Scheduling (Run Settings) --- */
  .relay-sched{
    margin-top:18px;
    padding-top:14px;
    border-top:1px solid var(--border);
  }

  .relay-sched-title{
    margin:0 0 8px 0;
  }

  .relay-sched-hint{
    margin:0 0 12px 0;
    font-size:0.9em;
    opacity:0.85;
  }

  .relay-sched-list{
    display:flex;
    flex-direction:column;
    gap:10px;
  }

  .relay-sched-row{
    display:flex;
    flex-wrap:wrap;
    gap:12px;
    align-items:flex-end;
    padding:10px 12px;
    border:1px solid var(--border);
    border-radius:var(--radius);
    background: var(--side); /* gleiche Farbe wie Sidebar (hell & dunkel) */
    color: #fff;             /* Lesbarkeit im Light-Theme */
  }

  .relay-sched-row label{ color: inherit; }

  /* Checkboxen NICHT von "input{width:100%}" kaputt machen */
  .relay-sched-row input[type="checkbox"]{
    width:auto;
    height:auto;
    padding:0;
    margin:0;
    transform:scale(1);
    accent-color: var(--header);
  }

  /* Minuten-Inputs: weißer Hintergrund (wie gewünscht) */
  .relay-sched-row .sched-field.minute input[type="number"]{
    width: 90px;
    max-width: 110px;
    padding: 8px 10px;
    background: #fff;
    color: #000;
    border: 1px solid var(--border);
    border-radius: var(--radius);
  }
  :root[data-theme='dark'] .relay-sched-row .sched-field.minute input[type="number"]{
    background: #e8e8e8;
    color: #000;
  }

  .relay-sched-row .sched-field.chk{
    min-width: 0;
  }

  .sched-field.chk{
    flex: 0 0 auto;
    min-width: max-content;
  }

  .sched-field.chk .inline-checkbox{
    display: inline-flex;
    align-items: center;
    gap: 8px;
    flex-wrap: nowrap;
  }

  .sched-field.chk .inline-checkbox span{
    white-space: nowrap;
  }

  .relay-sched-name{
    flex:1 1 170px;
    min-width:160px;
    max-width:220px;
  }

  .relay-sched-name-label{
    font-size:0.85em;
    opacity:0.9;
    margin-bottom:4px;
  }

  .relay-sched-name-value{
    font-weight:600;
    white-space:nowrap;
    overflow:hidden;
    text-overflow:ellipsis;
  }

  .sched-field.minute{
    flex:0 1 160px;
    min-width:140px;
}

)rawliteral";
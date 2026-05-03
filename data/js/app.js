/* ========================================
   PV Dimmer - SPA Application
   ======================================== */

const App = {
  refreshTimer: null,
  logTimer: null,
  logId: 0,
  currentPage: null,

  // ---------- Init ----------
  init() {
    this.initTheme();
    this.initNav();
    this.initClock();
    this.initMenuToggle();
    this.route(location.hash || '#dashboard');
    window.addEventListener('hashchange', () => this.route(location.hash));
  },

  // ---------- Routing ----------
  route(hash) {
    this.stopRefresh();
    const page = hash.replace('#', '') || 'dashboard';
    this.currentPage = page;

    document.querySelectorAll('#sidebar nav a').forEach(a => {
      a.classList.toggle('active', a.getAttribute('href') === '#' + page);
    });

    // Close mobile sidebar
    document.getElementById('sidebar').classList.remove('open');
    document.getElementById('overlay').classList.remove('show');

    const pages = {
      dashboard: () => this.loadDashboard(),
      config: () => this.loadConfig(),
      mqtt: () => this.loadMqtt(),
      minuteur: () => this.loadMinuteur(),
      log: () => this.loadLog(),
      backup: () => this.loadBackup(),
    };

    const loader = pages[page];
    if (loader) {
      document.getElementById('pageContent').innerHTML = '<div class="spinner"></div>';
      loader();
    }
  },

  // ---------- Dashboard ----------
  async loadDashboard() {
    document.getElementById('pageContent').innerHTML = `
      <h2 class="page-title">Dashboard</h2>
      <div id="alertBox" class="alert alert-danger"></div>
      <div class="card-grid">
        <div class="card">
          <div class="card-header">Puissance</div>
          <div class="card-body">
            <div class="gauge-container">
              <svg class="gauge-svg" viewBox="0 0 160 100">
                <path class="gauge-bg" d="M20,80 A60,60 0 0,1 140,80"/>
                <path class="gauge-fill" id="gaugePower" d="M20,80 A60,60 0 0,1 140,80" stroke="var(--primary)"/>
                <text class="gauge-text" x="80" y="72" id="gaugePowerVal">0</text>
                <text class="gauge-label" x="80" y="92" id="gaugePowerLabel">W</text>
              </svg>
            </div>
          </div>
        </div>
        <div class="card">
          <div class="card-header">Temperature</div>
          <div class="card-body">
            <div class="gauge-container">
              <svg class="gauge-svg" viewBox="0 0 160 100">
                <path class="gauge-bg" d="M20,80 A60,60 0 0,1 140,80"/>
                <path class="gauge-fill" id="gaugeTemp" d="M20,80 A60,60 0 0,1 140,80" stroke="var(--info)"/>
                <text class="gauge-text" x="80" y="72" id="gaugeTempVal">0</text>
                <text class="gauge-label" x="80" y="92">°C</text>
              </svg>
            </div>
          </div>
        </div>
        <div class="card">
          <div class="card-header">Etats du systeme</div>
          <div class="card-body">
            <div class="state-list">
              <div class="state-item">
                <span class="state-label">Ballon</span>
                <span class="state-value off" id="st-alerte">N/A</span>
              </div>
              <div class="state-item">
                <span class="state-label">Minuteur</span>
                <span class="state-value off" id="st-minuteur">N/A</span>
              </div>
              <div class="state-item">
                <span class="state-label">Relais 1</span>
                <span class="state-value off clickable" id="st-relay1" data-action="relay1">N/A</span>
              </div>
              <div class="state-item">
                <span class="state-label">Relais 2</span>
                <span class="state-value off clickable" id="st-relay2" data-action="relay2">N/A</span>
              </div>
              <div class="state-item">
                <div>
                  <span class="state-label">Boost</span>
                  <div class="state-sub" id="st-boost-info">max: N/A°C</div>
                </div>
                <span class="state-value off clickable" id="st-boost" data-action="boost">N/A</span>
              </div>
            </div>
          </div>
        </div>
        <div class="card">
          <div class="card-header">Sondes Dallas</div>
          <div class="card-body">
            <div class="dallas-grid" id="dallasContainer">
              <p style="color:var(--text-muted);font-size:.85rem">Chargement...</p>
            </div>
          </div>
        </div>
      </div>
    `;

    // Bind click actions
    document.querySelectorAll('[data-action]').forEach(el => {
      el.addEventListener('click', () => this.toggleAction(el.dataset.action));
    });

    // Init gauges
    this.initGauges();
    await this.refreshDashboard();
    this.refreshTimer = setInterval(() => this.refreshDashboard(), 5000);
  },

  initGauges() {
    // Calculate arc length for the semi-circle gauge
    // The arc path: M20,80 A60,60 0,0,1 140,80 is a semi-circle
    const arcLength = Math.PI * 60; // ~188.5
    document.querySelectorAll('.gauge-fill').forEach(el => {
      el.style.strokeDasharray = arcLength;
      el.style.strokeDashoffset = arcLength;
    });
  },

  setGauge(id, value, max, colorStops) {
    const el = document.getElementById(id);
    if (!el) return;
    const arcLength = Math.PI * 60;
    const pct = Math.min(value / max, 1);
    el.style.strokeDashoffset = arcLength * (1 - pct);

    // Color based on percentage
    if (colorStops) {
      const color = pct < 0.5 ? colorStops[0] : pct < 0.75 ? colorStops[1] : colorStops[2];
      el.setAttribute('stroke', color);
    }
  },

  async refreshDashboard() {
    try {
      const res = await fetch('/state');
      if (!res.ok) throw new Error('HTTP ' + res.status);
      const data = await res.json();
      this.updateDashboard(data);
    } catch (e) {
      console.error('State fetch error:', e);
    }
  },

  updateDashboard(d) {
    const power = parseFloat(d.power) || 0;
    const temp = parseFloat(d.temperature) || 0;
    const dimmer = parseFloat(d.dimmer) || 100;

    // Update gauges
    const maxPower = Math.max(dimmer, power, 100);
    this.setGauge('gaugePower', power, maxPower, ['var(--success)', 'var(--warning)', 'var(--danger)']);
    const valEl = document.getElementById('gaugePowerVal');
    if (valEl) valEl.textContent = Math.round(power);
    const lblEl = document.getElementById('gaugePowerLabel');
    if (lblEl) lblEl.textContent = Math.round(power) + ' W';

    this.setGauge('gaugeTemp', temp, 100, ['var(--info)', 'var(--warning)', 'var(--danger)']);
    const tempEl = document.getElementById('gaugeTempVal');
    if (tempEl) tempEl.textContent = Math.round(temp);

    // States
    this.setState('st-alerte', d.alerte === 1 || d.alerte === '1' || d.alerte === true,
      'Refroidissement', 'Normal', 'danger', 'on');
    this.setState('st-minuteur', d.minuteur === 1 || d.minuteur === '1' || d.minuteur === true,
      'Actif', 'Inactif', 'warn', 'off');
    this.setState('st-relay1', d.relay1 === 1 || d.relay1 === '1' || d.relay1 === true,
      'ON', 'OFF', 'on', 'off');
    this.setState('st-relay2', d.relay2 === 1 || d.relay2 === '1' || d.relay2 === true,
      'ON', 'OFF', 'on', 'off');

    const boostActive = d.boost === 1 || d.boost === '1' || d.boost === true;
    this.setState('st-boost', boostActive, 'ON', 'OFF', 'on', 'off');
    const boostInfo = document.getElementById('st-boost-info');
    if (boostInfo) {
      const maxT = d.boost_max_temp || 'N/A';
      boostInfo.textContent = boostActive && d.boost_endtime
        ? `Fin: ${d.boost_endtime} - max: ${maxT}°C`
        : `max: ${maxT}°C`;
    }

    // Alert
    const alertBox = document.getElementById('alertBox');
    if (alertBox) {
      if (typeof d.alerte === 'string' && d.alerte.trim() && d.alerte !== 'RAS') {
        alertBox.textContent = 'Alerte: ' + d.alerte;
        alertBox.classList.add('show');
      } else {
        alertBox.classList.remove('show');
      }
    }

    // Dallas
    this.updateDallas(d);

    // Update sidebar RSSI
    const rssiEl = document.getElementById('sidebar-rssi');
    if (rssiEl && d.RSSI !== undefined) rssiEl.textContent = 'RSSI: ' + d.RSSI + ' dBm';

    // Update topbar WiFi badge based on RSSI
    const wifiBadge = document.getElementById('topbar-wifi');
    if (wifiBadge) {
      const connected = d.RSSI !== undefined && d.RSSI !== null && d.RSSI !== 0 && d.RSSI !== -127;
      wifiBadge.classList.toggle('on', connected);
      wifiBadge.classList.toggle('off', !connected);
    }
  },

  setState(id, active, onText, offText, onClass, offClass) {
    const el = document.getElementById(id);
    if (!el) return;
    el.textContent = active ? onText : offText;
    el.className = 'state-value ' + (active ? onClass : offClass);
    if (el.dataset.action) el.classList.add('clickable');
  },

  updateDallas(data) {
    const container = document.getElementById('dallasContainer');
    if (!container) return;

    const sensors = [];
    for (const key in data) {
      if (key.startsWith('dallas')) {
        const num = key.substring(6);
        const temp = parseFloat(data[key]);
        if (!isNaN(temp)) {
          sensors.push({
            num,
            temp: temp.toFixed(1),
            addr: data['addr' + num] || 'inconnue'
          });
        }
      }
    }

    if (sensors.length === 0) {
      container.innerHTML = '<p style="color:var(--text-muted);font-size:.85rem">Aucun capteur Dallas</p>';
      return;
    }

    container.innerHTML = sensors.map(s => `
      <div class="dallas-item">
        <div class="dallas-icon">
          <svg viewBox="0 0 16 16"><use href="/icons.svg#icon-thermometer"/></svg>
        </div>
        <div>
          <div class="dallas-temp">${s.temp}°C</div>
          <div class="dallas-addr">${this.esc(s.addr)}</div>
        </div>
      </div>
    `).join('');
  },

  async toggleAction(action) {
    try {
      if (action === 'relay1') await fetch('/get?relay1=2');
      else if (action === 'relay2') await fetch('/get?relay2=2');
      else if (action === 'boost') await fetch('/boost');
      setTimeout(() => this.refreshDashboard(), 500);
    } catch (e) {
      console.error('Toggle error:', e);
    }
  },

  // ---------- Config ----------
  async loadConfig() {
    document.getElementById('pageContent').innerHTML = `
      <h2 class="page-title">Configuration</h2>
      <div id="alertBox" class="alert alert-danger"></div>
      <div style="display:flex;gap:.75rem;margin-bottom:1rem;flex-wrap:wrap">
        <button class="btn btn-primary btn-sm" id="btn-apply-config">Appliquer</button>
        <button class="btn btn-success btn-sm" id="btn-save-flash">Sauvegarder sur la flash</button>
        <button class="btn btn-outline btn-sm" id="btn-onoff">Dimmer: ...</button>
      </div>
      <div id="config-status" class="alert alert-success" style="display:none"></div>
      <form id="configForm">
        <div class="card" style="margin-bottom:1rem">
          <div class="card-header">Plage d'utilisation</div>
          <div class="card-body">
            <div class="form-row">
              <div class="form-group">
                <label>Max Temp (°C)</label>
                <input type="number" class="form-control" id="maxtemp">
              </div>
              <div class="form-group">
                <label>Min Temp (°C)</label>
                <input type="number" class="form-control" id="mintemp">
              </div>
              <div class="form-group">
                <label>Trigger (%)</label>
                <input type="number" class="form-control" id="trigger">
              </div>
              <div class="form-group">
                <label>Min Power (%)</label>
                <input type="number" class="form-control" id="minpow">
              </div>
              <div class="form-group">
                <label>Max Power (%)</label>
                <input type="number" class="form-control" id="maxpow">
              </div>
            </div>
          </div>
        </div>
        <div class="card" style="margin-bottom:1rem">
          <div class="card-header">Charges</div>
          <div class="card-body">
            <div class="form-row">
              <div class="form-group">
                <label>Charge 1 - Dimmer (W)</label>
                <input type="number" class="form-control" id="charge1">
              </div>
              <div class="form-group">
                <label>Charge 2 - Jotta (W)</label>
                <input type="number" class="form-control" id="charge2">
              </div>
              <div class="form-group">
                <label>Charge 3 - Relay2 (W)</label>
                <input type="number" class="form-control" id="charge3">
              </div>
            </div>
          </div>
        </div>
        <div class="card" style="margin-bottom:1rem">
          <div class="card-header">Child &amp; Hostname</div>
          <div class="card-body">
            <div class="form-row">
              <div class="form-group">
                <label>Child Dimmer IP</label>
                <input type="text" class="form-control" id="child">
              </div>
              <div class="form-group">
                <label>Child Mode</label>
                <select class="form-control" id="delester">
                  <option value="off">Off</option>
                  <option value="delester">Delester</option>
                  <option value="equal">Egal</option>
                </select>
              </div>
              <div class="form-group">
                <label>Dimmer Name</label>
                <input type="text" class="form-control" id="dimmername">
              </div>
            </div>
          </div>
        </div>
        <div class="card" style="margin-bottom:1rem">
          <div class="card-header">Pilote MQTT</div>
          <div class="card-body">
            <div class="form-row">
              <div class="form-group">
                <label>MQTT state subscription</label>
                <input type="text" class="form-control" id="SubscribePV" placeholder="none">
              </div>
              <div class="form-group">
                <label>MQTT temp subscription</label>
                <input type="text" class="form-control" id="SubscribeTEMP" placeholder="none">
              </div>
            </div>
            <div class="form-row">
              <div class="form-group">
                <label>Puissance de demarrage</label>
                <input type="number" class="form-control" id="startingpow">
              </div>
              <div class="form-group">
                <label>Etat au demarrage</label>
                <select class="form-control" id="dimmer_on_off">
                  <option value="1">On</option>
                  <option value="0">Off</option>
                </select>
              </div>
            </div>
          </div>
        </div>
        <div class="card" style="margin-bottom:1rem">
          <div class="card-header">Dallas Local</div>
          <div class="card-body">
            <div class="form-group">
              <label>Adresse sonde Dallas maitre</label>
              <input type="text" class="form-control" id="DALLAS">
            </div>
            <div id="config-dallas" style="margin-top:.5rem;font-size:.82rem;color:var(--text-muted)"></div>
          </div>
        </div>
      </form>
    `;

    // Load config
    try {
      const [configRes, stateRes] = await Promise.all([fetch('/config'), fetch('/state')]);
      const config = await configRes.json();
      const state = await stateRes.json();

      // Populate form
      for (const key in config) {
        const el = document.getElementById(key);
        if (el) {
          if (el.type === 'checkbox') el.checked = config[key];
          else el.value = config[key];
        }
      }

      // ON/OFF button
      const onoffBtn = document.getElementById('btn-onoff');
      onoffBtn.textContent = 'Dimmer: ' + (state.onoff ? 'ON' : 'OFF');
      onoffBtn.className = 'btn btn-sm ' + (state.onoff ? 'btn-success' : 'btn-danger');

      // Dallas info from state
      this.updateConfigDallas(state);
    } catch (e) {
      console.error('Config load error:', e);
    }

    // Bind buttons
    document.getElementById('btn-apply-config').addEventListener('click', () => this.applyConfig());
    document.getElementById('btn-save-flash').addEventListener('click', () => this.saveFlash());
    document.getElementById('btn-onoff').addEventListener('click', () => this.toggleOnOff());
  },

  updateConfigDallas(data) {
    const container = document.getElementById('config-dallas');
    if (!container) return;
    const sensors = [];
    for (const key in data) {
      if (key.startsWith('dallas')) {
        const num = key.substring(6);
        sensors.push(`Sonde ${num}: ${data[key]}°C (${data['addr' + num] || '?'})`);
      }
    }
    container.innerHTML = sensors.length
      ? '<strong>Sondes presentes:</strong><br>' + sensors.join('<br>')
      : 'Aucune sonde detectee';
  },

  async applyConfig() {
    const fields = ['maxtemp', 'mintemp', 'startingpow', 'minpow', 'maxpow', 'child',
      'SubscribePV', 'SubscribeTEMP', 'charge1', 'charge2', 'charge3',
      'DALLAS', 'dimmername', 'trigger'];
    const params = new URLSearchParams();
    fields.forEach(f => {
      const el = document.getElementById(f);
      if (el) params.set(f, el.value);
    });
    params.set('mode', document.getElementById('delester').value);

    try {
      await fetch('/get?' + params.toString());
      this.showStatus('config-status', 'Configuration appliquee');
    } catch (e) {
      this.showStatus('config-status', 'Erreur: ' + e.message, true);
    }
  },

  async saveFlash() {
    try {
      await fetch('/get?save=yes');
      this.showStatus('config-status', 'Configuration sauvegardee sur la flash');
    } catch (e) {
      this.showStatus('config-status', 'Erreur: ' + e.message, true);
    }
  },

  async toggleOnOff() {
    try {
      const res = await fetch('/onoff');
      const val = await res.text();
      const btn = document.getElementById('btn-onoff');
      const isOn = val.trim() === '1';
      btn.textContent = 'Dimmer: ' + (isOn ? 'ON' : 'OFF');
      btn.className = 'btn btn-sm ' + (isOn ? 'btn-success' : 'btn-danger');
    } catch (e) {
      console.error('Toggle error:', e);
    }
  },

  // ---------- MQTT ----------
  async loadMqtt() {
    document.getElementById('pageContent').innerHTML = `
      <h2 class="page-title">Configuration MQTT</h2>
      <div style="display:flex;gap:.75rem;margin-bottom:1rem;flex-wrap:wrap">
        <button class="btn btn-primary btn-sm" id="btn-apply-mqtt">Appliquer</button>
        <button class="btn btn-success btn-sm" id="btn-save-mqtt">Sauvegarder</button>
      </div>
      <div id="mqtt-status" class="alert alert-success" style="display:none"></div>
      <form id="mqttForm">
        <div class="card" style="margin-bottom:1rem">
          <div class="card-header">Connexion MQTT</div>
          <div class="card-body">
            <div class="form-check">
              <input type="checkbox" id="MQTT" onchange="App.sendServermode('MQTT')">
              <label for="MQTT">Activer MQTT</label>
            </div>
            <div class="form-row">
              <div class="form-group">
                <label>Serveur</label>
                <input type="text" class="form-control" id="server" placeholder="IP ou hostname">
              </div>
              <div class="form-group">
                <label>Port</label>
                <input type="number" class="form-control" id="port" placeholder="1883">
              </div>
              <div class="form-group">
                <label>Topic Domoticz</label>
                <input type="text" class="form-control" id="topic">
              </div>
            </div>
            <div class="form-row">
              <div class="form-group">
                <label>Utilisateur</label>
                <input type="text" class="form-control" id="user">
              </div>
              <div class="form-group">
                <label>Mot de passe</label>
                <input type="password" class="form-control" id="password">
              </div>
            </div>
          </div>
        </div>
        <div class="card" style="margin-bottom:1rem">
          <div class="card-header">Domoticz IDX</div>
          <div class="card-body">
            <div class="form-row">
              <div class="form-group">
                <label>IDX Temperature</label>
                <input type="number" class="form-control" id="idxtemp">
              </div>
              <div class="form-group">
                <label>IDX Puissance</label>
                <input type="number" class="form-control" id="IDX">
              </div>
              <div class="form-group">
                <label>IDX Alarme</label>
                <input type="number" class="form-control" id="IDXAlarme">
              </div>
            </div>
          </div>
        </div>
      </form>
    `;

    try {
      const res = await fetch('/getmqtt');
      const data = await res.json();
      for (const key in data) {
        const el = document.getElementById(key);
        if (el) {
          if (el.type === 'checkbox') el.checked = data[key];
          else el.value = data[key];
        }
      }
    } catch (e) {
      console.error('MQTT config load error:', e);
    }

    document.getElementById('btn-apply-mqtt').addEventListener('click', () => this.applyMqtt());
    document.getElementById('btn-save-mqtt').addEventListener('click', async () => {
      try {
        await fetch('/getmqtt?save=yes');
        this.showStatus('mqtt-status', 'Configuration sauvegardee');
      } catch (e) {
        this.showStatus('mqtt-status', 'Erreur', true);
      }
    });
  },

  async applyMqtt() {
    const params = new URLSearchParams();
    ['server', 'port', 'topic', 'user', 'password', 'idxtemp', 'IDX', 'IDXAlarme'].forEach(f => {
      const el = document.getElementById(f);
      if (el) params.set(f === 'server' ? 'hostname' : f === 'topic' ? 'Publish' :
        f === 'user' ? 'mqttuser' : f === 'password' ? 'mqttpassword' : f, el.value);
    });
    try {
      await fetch('/get?' + params.toString());
      this.showStatus('mqtt-status', 'Configuration appliquee');
    } catch (e) {
      this.showStatus('mqtt-status', 'Erreur: ' + e.message, true);
    }
  },

  async sendServermode(mode) {
    try {
      const res = await fetch('/get?servermode=' + encodeURIComponent(mode));
      const text = await res.text();
      const config = text.split(';');
      const el = document.getElementById(mode);
      if (el && config[18] !== undefined) {
        el.checked = config[18] === '1' || config[18] === 'true' || config[18] === 'on';
      }
      this.showStatus('mqtt-status', 'Configuration appliquee');
    } catch (e) {
      this.showStatus('mqtt-status', 'Erreur: ' + e.message, true);
    }
  },

  // ---------- Minuteur ----------
  async loadMinuteur() {
    const tabs = ['dimmer', 'relay1', 'relay2'];
    document.getElementById('pageContent').innerHTML = `
      <h2 class="page-title">Minuteur d'appoint</h2>
      <div id="minuteur-status" class="alert alert-success" style="display:none"></div>
      <div class="card">
        <div class="card-header">
          <div class="tabs" style="border:none;margin:0">
            ${tabs.map((t, i) => `<button class="tab-btn ${i === 0 ? 'active' : ''}" data-tab="${t}">${t === 'dimmer' ? 'Dimmer' : t === 'relay1' ? 'Relais 1' : 'Relais 2'}</button>`).join('')}
          </div>
        </div>
        <div class="card-body">
          ${tabs.map((t, i) => `
            <div class="tab-panel ${i === 0 ? 'active' : ''}" id="tab-${t}">
              <form id="form-${t}">
                <div class="form-row">
                  <div class="form-group">
                    <label>Heure de demarrage (HH:MM)</label>
                    <input type="text" class="form-control" id="heure_demarrage_${t}" placeholder="HH:MM">
                  </div>
                  <div class="form-group">
                    <label>Heure d'arret (HH:MM)</label>
                    <input type="text" class="form-control" id="heure_arret_${t}" placeholder="HH:MM">
                  </div>
                  <div class="form-group">
                    <label>Temperature consigne (°C)</label>
                    <input type="number" class="form-control" id="temperature_${t}">
                  </div>
                  ${t === 'dimmer' ? `
                  <div class="form-group">
                    <label>Puissance (%)</label>
                    <input type="number" class="form-control" id="puissance_${t}">
                  </div>` : ''}
                </div>
                <button type="submit" class="btn btn-primary btn-sm" style="margin-top:.5rem">Appliquer ${t === 'dimmer' ? 'Dimmer' : t === 'relay1' ? 'Relais 1' : 'Relais 2'}</button>
              </form>
            </div>
          `).join('')}
        </div>
      </div>
    `;

    // Tabs
    document.querySelectorAll('.tab-btn').forEach(btn => {
      btn.addEventListener('click', () => {
        document.querySelectorAll('.tab-btn').forEach(b => b.classList.remove('active'));
        document.querySelectorAll('.tab-panel').forEach(p => p.classList.remove('active'));
        btn.classList.add('active');
        document.getElementById('tab-' + btn.dataset.tab).classList.add('active');
      });
    });

    // Load data
    for (const t of tabs) {
      try {
        const res = await fetch('/getminuteur?' + t);
        const data = await res.json();
        const fields = ['heure_demarrage', 'heure_arret', 'temperature'];
        if (t === 'dimmer') fields.push('puissance');
        fields.forEach(f => {
          const el = document.getElementById(f + '_' + t);
          if (el && data[f] !== undefined) el.value = data[f];
        });
      } catch (e) {
        console.error('Minuteur load error:', e);
      }
    }

    // Bind forms
    for (const t of tabs) {
      document.getElementById('form-' + t).addEventListener('submit', async (e) => {
        e.preventDefault();
        const params = new URLSearchParams();
        params.set('heure_demarrage', document.getElementById('heure_demarrage_' + t).value);
        params.set('heure_arret', document.getElementById('heure_arret_' + t).value);
        params.set('temperature', document.getElementById('temperature_' + t).value);
        if (t === 'dimmer') params.set('puissance', document.getElementById('puissance_' + t).value);
        try {
          await fetch('/setminuteur?' + t + '&' + params.toString());
          this.showStatus('minuteur-status', 'Configuration ' + t + ' appliquee');
        } catch (err) {
          this.showStatus('minuteur-status', 'Erreur', true);
        }
      });
    }
  },

  // ---------- Log ----------
  loadLog() {
    document.getElementById('pageContent').innerHTML = `
      <h2 class="page-title">Console Logs</h2>
      <textarea class="log-console" id="logArea" readonly></textarea>
    `;
    this.logId = 0;
    this.fetchLog();
  },

  async fetchLog() {
    if (this.currentPage !== 'log') return;
    try {
      const res = await fetch('/cs?c2=' + this.logId);
      const text = await res.text();
      const parts = text.split(/\}1/);
      this.logId = parts.shift();
      if (parts.shift() === '0') {
        const area = document.getElementById('logArea');
        if (area) area.value = '';
      }
      const content = parts.shift();
      if (content && content.length > 0) {
        const area = document.getElementById('logArea');
        if (area) {
          area.value += content;
          area.scrollTop = area.scrollHeight;
        }
      }
    } catch (e) {
      console.error('Log fetch error:', e);
    }
    this.logTimer = setTimeout(() => this.fetchLog(), 2500);
  },

  // ---------- Backup ----------
  loadBackup() {
    document.getElementById('pageContent').innerHTML = `
      <h2 class="page-title">Sauvegarde &amp; Restauration</h2>
      <div class="card-grid" style="grid-template-columns:repeat(auto-fill,minmax(320px,1fr))">
        <div class="card">
          <div class="card-header">Sauvegarder</div>
          <div class="card-body" style="text-align:center">
            <button class="btn btn-primary" id="btn-backup">
              <svg viewBox="0 0 16 16"><use href="/icons.svg#icon-backup"/></svg>
              Telecharger la sauvegarde
            </button>
            <div id="backup-log" style="margin-top:1rem;font-size:.82rem;text-align:left"></div>
          </div>
        </div>
        <div class="card">
          <div class="card-header">Restaurer</div>
          <div class="card-body" style="text-align:center">
            <div class="form-group">
              <input type="file" class="form-control" id="restoreFile" accept=".json">
            </div>
            <button class="btn btn-primary btn-sm" id="btn-restore">Restaurer</button>
            <div style="margin-top:.75rem">
              <button class="btn btn-outline btn-sm" id="btn-save-restore">Sauvegarder sur la flash</button>
            </div>
            <div id="restore-log" style="margin-top:1rem;font-size:.82rem;text-align:left"></div>
          </div>
        </div>
      </div>
    `;

    document.getElementById('btn-backup').addEventListener('click', () => this.doBackup());
    document.getElementById('btn-restore').addEventListener('click', () => this.doRestore());
    document.getElementById('btn-save-restore').addEventListener('click', async () => {
      try {
        await fetch('/get?save=yes');
        this.appendLog('restore-log', 'Configuration sauvegardee sur la flash', 'success');
      } catch (e) {
        this.appendLog('restore-log', 'Erreur', 'danger');
      }
    });
  },

  async doBackup() {
    const log = 'backup-log';
    document.getElementById(log).innerHTML = '';
    const requests = [
      { title: 'Config generale', url: '/config', key: 'general' },
      { title: 'Config MQTT', url: '/getmqtt', key: 'mqtt' },
      { title: 'Minuteur dimmer', url: '/getminuteur?dimmer', key: 'dimmer_timer' },
      { title: 'Minuteur relais 1', url: '/getminuteur?relay1', key: 'relay1_timer' },
      { title: 'Minuteur relais 2', url: '/getminuteur?relay2', key: 'relay2_timer' },
    ];

    const backup = {};
    let hasError = false;

    for (const req of requests) {
      this.appendLog(log, req.title + '...', 'info');
      try {
        const res = await fetch(req.url);
        backup[req.key] = await res.json();
        this.replaceLastLog(log, req.title + ' OK', 'success');
      } catch (e) {
        this.replaceLastLog(log, req.title + ' ERREUR', 'danger');
        hasError = true;
      }
    }

    if (!hasError) {
      const now = new Date().toISOString().replace(/[TZ]/g, '-').replace(/\..+/, '');
      const blob = new Blob([JSON.stringify(backup, null, 2)], { type: 'application/json' });
      const a = document.createElement('a');
      a.href = URL.createObjectURL(blob);
      a.download = `${now}-pvdimmer-backup.json`;
      a.click();
      URL.revokeObjectURL(a.href);
      this.appendLog(log, 'Telechargement lance', 'success');
    }
  },

  async doRestore() {
    const log = 'restore-log';
    document.getElementById(log).innerHTML = '';
    const input = document.getElementById('restoreFile');
    if (!input.files.length) {
      this.appendLog(log, 'Selectionnez un fichier', 'warn');
      return;
    }

    let data;
    try {
      const text = await input.files[0].text();
      data = JSON.parse(text);
      this.appendLog(log, 'Fichier charge', 'success');
    } catch (e) {
      this.appendLog(log, 'Fichier invalide ou corrompu', 'danger');
      return;
    }

    const toBool = v => v === true || v === 'true' || v === 1 || v === '1' || v === 'on';
    const mqttToggles = ['MQTT'];
    const mqttRemap = { server: 'hostname', topic: 'Publish', user: 'mqttuser', password: 'mqttpassword' };

    let currentMqtt = {};
    try { currentMqtt = await (await fetch('/getmqtt')).json(); } catch (e) {}

    // Config generale
    if (data.general) {
      this.appendLog(log, 'Config generale...', 'info');
      const params = new URLSearchParams();
      for (const [k, v] of Object.entries(data.general)) params.set(k, v);
      try {
        await fetch('/get?' + params.toString());
        this.replaceLastLog(log, 'Config generale OK', 'success');
      } catch (e) { this.replaceLastLog(log, 'Config generale ERREUR', 'danger'); }
    } else {
      this.appendLog(log, 'Config generale : absente du fichier', 'warn');
    }

    // Config MQTT (sans le toggle servermode, avec remap)
    if (data.mqtt) {
      this.appendLog(log, 'Config MQTT...', 'info');
      const params = new URLSearchParams();
      for (const [k, v] of Object.entries(data.mqtt)) {
        if (mqttToggles.includes(k)) continue;
        params.set(mqttRemap[k] || k, v);
      }
      try {
        await fetch('/get?' + params.toString());
        this.replaceLastLog(log, 'Config MQTT OK', 'success');
      } catch (e) { this.replaceLastLog(log, 'Config MQTT ERREUR', 'danger'); }
    } else {
      this.appendLog(log, 'Config MQTT : absente du fichier', 'warn');
    }

    // Toggle servermode (bascule si l'etat differe)
    for (const key of mqttToggles) {
      if (!data.mqtt) continue;
      const target = data.mqtt[key];
      const current = currentMqtt[key];
      if (target === undefined || current === undefined) continue;
      if (toBool(target) === toBool(current)) continue;
      try {
        await fetch('/get?servermode=' + key);
        this.appendLog(log, key + ' bascule -> ' + (toBool(target) ? 'ON' : 'OFF'), 'success');
      } catch (e) { this.appendLog(log, key + ' ERREUR', 'danger'); }
    }

    // Minuteurs
    const timers = [
      ['dimmer_timer', 'dimmer', 'Minuteur dimmer'],
      ['relay1_timer', 'relay1', 'Minuteur relais 1'],
      ['relay2_timer', 'relay2', 'Minuteur relais 2'],
    ];
    for (const [field, type, label] of timers) {
      if (!data[field]) {
        this.appendLog(log, label + ' : absent du fichier', 'warn');
        continue;
      }
      this.appendLog(log, label + '...', 'info');
      const params = new URLSearchParams();
      for (const [k, v] of Object.entries(data[field])) params.set(k, v);
      try {
        await fetch('/setminuteur?' + type + '&' + params.toString());
        this.replaceLastLog(log, label + ' OK', 'success');
      } catch (e) { this.replaceLastLog(log, label + ' ERREUR', 'danger'); }
    }

    this.appendLog(log, 'Restauration terminee. Pensez a sauvegarder sur la flash.', 'info');
  },

  // ---------- Helpers ----------
  appendLog(containerId, text, type) {
    const colors = { success: 'var(--success)', danger: 'var(--danger)', warn: 'var(--warning)', info: 'var(--info)' };
    const icons = { success: '\u2705', danger: '\u26D4', warn: '\u2753', info: '\u23F3' };
    const el = document.getElementById(containerId);
    if (el) el.innerHTML += `<div style="color:${colors[type] || 'var(--text)'};padding:2px 0">${icons[type] || ''} ${this.esc(text)}</div>`;
  },

  replaceLastLog(containerId, text, type) {
    const el = document.getElementById(containerId);
    if (el && el.lastElementChild) {
      const colors = { success: 'var(--success)', danger: 'var(--danger)', warn: 'var(--warning)', info: 'var(--info)' };
      const icons = { success: '\u2705', danger: '\u26D4', warn: '\u2753', info: '\u23F3' };
      el.lastElementChild.style.color = colors[type] || 'var(--text)';
      el.lastElementChild.textContent = (icons[type] || '') + ' ' + text;
    }
  },

  showStatus(id, msg, isError) {
    const el = document.getElementById(id);
    if (!el) return;
    el.textContent = msg;
    el.className = 'alert ' + (isError ? 'alert-danger' : 'alert-success') + ' show';
    setTimeout(() => el.classList.remove('show'), 4000);
  },

  stopRefresh() {
    if (this.refreshTimer) { clearInterval(this.refreshTimer); this.refreshTimer = null; }
    if (this.logTimer) { clearTimeout(this.logTimer); this.logTimer = null; }
  },

  esc(str) {
    const d = document.createElement('div');
    d.textContent = str;
    return d.innerHTML;
  },

  // ---------- Theme ----------
  initTheme() {
    const saved = localStorage.getItem('theme');
    if (saved === 'dark') document.documentElement.classList.add('dark-theme');

    document.getElementById('themeToggle').addEventListener('click', () => {
      document.documentElement.classList.toggle('dark-theme');
      const isDark = document.documentElement.classList.contains('dark-theme');
      localStorage.setItem('theme', isDark ? 'dark' : 'light');
      // Update icon
      const use = document.querySelector('#themeToggle use');
      if (use) use.setAttribute('href', '/icons.svg#' + (isDark ? 'icon-sun' : 'icon-moon'));
    });
  },

  // ---------- Clock ----------
  initClock() {
    const el = document.getElementById('clock');
    const update = () => {
      const d = new Date();
      el.textContent = [d.getHours(), d.getMinutes(), d.getSeconds()]
        .map(n => String(n).padStart(2, '0')).join(':');
    };
    update();
    setInterval(update, 1000);
  },

  // ---------- Navigation ----------
  initNav() {
    // Nothing extra needed - hash-based routing handles it
  },

  // ---------- Mobile Menu ----------
  initMenuToggle() {
    const sidebar = document.getElementById('sidebar');
    const overlay = document.getElementById('overlay');
    const toggle = document.getElementById('menuToggle');

    if (toggle) {
      toggle.addEventListener('click', () => {
        sidebar.classList.toggle('open');
        overlay.classList.toggle('show');
      });
    }

    if (overlay) {
      overlay.addEventListener('click', () => {
        sidebar.classList.remove('open');
        overlay.classList.remove('show');
      });
    }
  },
};

// Boot
document.addEventListener('DOMContentLoaded', () => App.init());

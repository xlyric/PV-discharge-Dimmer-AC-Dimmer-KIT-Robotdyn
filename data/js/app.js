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
    if (window.I18n) I18n.init();
    this.initTheme();
    this.initNav();
    this.initClock();
    this.initMenuToggle();
    this.initLang();
    this.initReboot();
    this.route(location.hash || '#dashboard');
    window.addEventListener('hashchange', () => this.route(location.hash));
    document.addEventListener('langchange', () => {
      if (this.currentPage) this.route('#' + this.currentPage);
    });
  },

  t(key, vars) { return window.I18n ? I18n.t(key, vars) : key; },

  initLang() {
    const sel = document.getElementById('langSelect');
    if (!sel || !window.I18n) return;
    sel.value = I18n.current;
    sel.addEventListener('change', () => I18n.set(sel.value));
  },

  initReboot() {
    const btn = document.getElementById('rebootBtn');
    if (!btn) return;
    btn.addEventListener('click', () => {
      if (confirm(this.t('common.confirm_reboot_dimmer'))) fetch('/reboot');
    });
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
    const menuToggle = document.getElementById('menuToggle');
    if (menuToggle) menuToggle.setAttribute('aria-expanded', 'false');

    const pages = {
      dashboard: () => this.loadDashboard(),
      config: () => this.loadConfig(),
      mqtt: () => this.loadMqtt(),
      minuteur: () => this.loadMinuteur(),
      log: () => this.loadLog(),
      backup: () => this.loadBackup(),
      security: () => this.loadSecurity(),
      ota: () => this.loadOta(),
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
      <h2 class="page-title">${this.t('page.dashboard')}</h2>
      <div id="alertBox" class="alert alert-danger"></div>
      <div class="card-grid">
        <div class="card">
          <div class="card-header">${this.t('dash.power')}</div>
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
          <div class="card-header">${this.t('dash.temperature')}</div>
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
          <div class="card-header">${this.t('dash.system_states')}</div>
          <div class="card-body">
            <div class="state-list">
              <div class="state-item">
                <span class="state-label">${this.t('dash.boiler')}</span>
                <span class="state-value off" id="st-alerte">N/A</span>
              </div>
              <div class="state-item">
                <span class="state-label">${this.t('dash.timer')}</span>
                <span class="state-value off" id="st-minuteur">N/A</span>
              </div>
              <div class="state-item">
                <span class="state-label">${this.t('dash.relay1')}</span>
                <span class="state-value off clickable" id="st-relay1" data-action="relay1">N/A</span>
              </div>
              <div class="state-item">
                <span class="state-label">${this.t('dash.relay2')}</span>
                <span class="state-value off clickable" id="st-relay2" data-action="relay2">N/A</span>
              </div>
              <div class="state-item">
                <div>
                  <span class="state-label">${this.t('dash.boost')}</span>
                  <div class="state-sub" id="st-boost-info">${this.t('dash.boost_max_label', { v: 'N/A°C' })}</div>
                </div>
                <span class="state-value off clickable" id="st-boost" data-action="boost">N/A</span>
              </div>
            </div>
          </div>
        </div>
        <div class="card">
          <div class="card-header">${this.t('dash.dallas_probes')}</div>
          <div class="card-body">
            <div class="dallas-grid" id="dallasContainer">
              <p style="color:var(--text-muted);font-size:.85rem">${this.t('common.loading')}</p>
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
      const wifiBadge = document.getElementById('topbar-wifi');
      if (wifiBadge) { wifiBadge.classList.remove('on'); wifiBadge.classList.add('off'); }
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
      this.t('state.cooling'), this.t('state.normal'), 'danger', 'on');
    this.setState('st-minuteur', d.minuteur === 1 || d.minuteur === '1' || d.minuteur === true,
      this.t('state.active'), this.t('state.inactive'), 'warn', 'off');
    this.setState('st-relay1', d.relay1 === 1 || d.relay1 === '1' || d.relay1 === true,
      this.t('state.on'), this.t('state.off'), 'on', 'off');
    this.setState('st-relay2', d.relay2 === 1 || d.relay2 === '1' || d.relay2 === true,
      this.t('state.on'), this.t('state.off'), 'on', 'off');

    const boostActive = d.boost === 1 || d.boost === '1' || d.boost === true;
    this.setState('st-boost', boostActive, this.t('state.on'), this.t('state.off'), 'on', 'off');
    const boostInfo = document.getElementById('st-boost-info');
    if (boostInfo) {
      const maxT = d.boost_max_temp || 'N/A';
      boostInfo.textContent = boostActive && d.boost_endtime
        ? this.t('state.boost_info', { end: d.boost_endtime, max: maxT })
        : this.t('state.boost_max', { max: maxT });
    }

    // Alert
    const alertBox = document.getElementById('alertBox');
    if (alertBox) {
      if (typeof d.alerte === 'string' && d.alerte.trim() && d.alerte !== 'RAS') {
        alertBox.textContent = this.t('state.alert_prefix') + ': ' + d.alerte;
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
    const v = document.getElementById('sidebar-version');
    if (v && d.version) v.textContent = d.version;
    const n = document.getElementById('sidebar-name');
    if (n && d.dimmername) {  n.textContent = d.dimmername + '.local';
      document.title = 'PV Dimmer - ' + (d.dimmername || ''); }

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
          <svg viewBox="0 0 16 16" aria-hidden="true"><use href="/icons.svg#icon-thermometer"/></svg>
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
      <h2 class="page-title">${this.t('page.config')}</h2>
      <div id="alertBox" class="alert alert-danger"></div>
      <div style="display:flex;gap:.75rem;margin-bottom:1rem;flex-wrap:wrap">
        <button class="btn btn-primary btn-sm" id="btn-apply-config">${this.t('btn.apply')}</button>
        <button class="btn btn-success btn-sm" id="btn-save-flash">${this.t('btn.save_flash')}</button>
        <button class="btn btn-outline btn-sm" id="btn-onoff">${this.t('dash.dimmer_status', { state: '...' })}</button>
      </div>
      <div id="config-status" class="alert alert-success" style="display:none"></div>
      <form id="configForm">
        <div class="card" style="margin-bottom:1rem">
          <div class="card-header">${this.t('config.range')}</div>
          <div class="card-body">
            <div class="form-row">
              <div class="form-group">
                <label>${this.t('form.max_temp')}</label>
                <input type="number" class="form-control" id="maxtemp" min="0" max="100" step="1">
              </div>
              <div class="form-group">
                <label>${this.t('form.min_temp')}</label>
                <input type="number" class="form-control" id="mintemp" min="0" max="100" step="1">
              </div>
              <div class="form-group">
                <label>${this.t('form.trigger')}</label>
                <input type="number" class="form-control" id="trigger" min="0" max="100" step="1">
              </div>
              <div class="form-group">
                <label>${this.t('form.min_power')}</label>
                <input type="number" class="form-control" id="minpow" min="0" max="100" step="1">
              </div>
              <div class="form-group">
                <label>${this.t('form.max_power')}</label>
                <input type="number" class="form-control" id="maxpow" min="0" max="100" step="1">
              </div>
            </div>
          </div>
        </div>
        <div class="card" style="margin-bottom:1rem">
          <div class="card-header">${this.t('config.charges')}</div>
          <div class="card-body">
            <div class="form-row">
              <div class="form-group">
                <label>${this.t('form.charge1')}</label>
                <input type="number" class="form-control" id="charge1" min="0" max="10000" step="1">
              </div>
              <div class="form-group">
                <label>${this.t('form.charge2')}</label>
                <input type="number" class="form-control" id="charge2" min="0" max="10000" step="1">
              </div>
              <div class="form-group">
                <label>${this.t('form.charge3')}</label>
                <input type="number" class="form-control" id="charge3" min="0" max="10000" step="1">
              </div>
            </div>
          </div>
        </div>
        <div class="card" style="margin-bottom:1rem">
          <div class="card-header">${this.t('config.child_hostname')}</div>
          <div class="card-body">
            <div class="form-row">
              <div class="form-group">
                <label>${this.t('form.child_dimmer_ip')}</label>
                <input type="text" class="form-control" id="child">
              </div>
              <div class="form-group">
                <label>${this.t('form.child_mode')}</label>
                <select class="form-control" id="delester">
                  <option value="off">${this.t('form.opt_off')}</option>
                  <option value="delester">${this.t('form.opt_delester')}</option>
                  <option value="equal">${this.t('form.opt_equal')}</option>
                </select>
              </div>
              <div class="form-group">
                <label>${this.t('form.dimmer_name')}</label>
                <input type="text" class="form-control" id="dimmername">
              </div>
            </div>
          </div>
        </div>
        <div class="card" style="margin-bottom:1rem">
          <div class="card-header">${this.t('card.pilote_mqtt')}</div>
          <div class="card-body">
            <div class="form-row">
              <div class="form-group">
                <label>${this.t('form.mqtt_state_sub')}</label>
                <input type="text" class="form-control" id="SubscribePV" placeholder="none">
              </div>
              <div class="form-group">
                <label>${this.t('form.mqtt_temp_sub')}</label>
                <input type="text" class="form-control" id="SubscribeTEMP" placeholder="none">
              </div>
            </div>
            <div class="form-row">
              <div class="form-group">
                <label>${this.t('form.start_power')}</label>
                <input type="number" class="form-control" id="startingpow" min="0" max="10000" step="1">
              </div>
              <div class="form-group">
                <label>${this.t('form.start_state')}</label>
                <select class="form-control" id="dimmer_on_off">
                  <option value="1">${this.t('form.opt_on')}</option>
                  <option value="0">${this.t('form.opt_off')}</option>
                </select>
              </div>
            </div>
          </div>
        </div>
        <div class="card" style="margin-bottom:1rem">
          <div class="card-header">${this.t('config.dallas_local')}</div>
          <div class="card-body">
            <div class="form-group">
              <label>${this.t('form.dallas_master_addr')}</label>
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
      onoffBtn.textContent = this.t('dash.dimmer_status', { state: state.onoff ? this.t('state.on') : this.t('state.off') });
      onoffBtn.className = 'btn btn-sm ' + (state.onoff ? 'btn-success' : 'btn-danger');

      // Dallas info from state
      this.updateConfigDallas(state);
    } catch (e) {
      console.error('Config load error:', e);
      this.showStatus('config-status', this.t('status.error_with', { msg: e.message }), true);
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
        sensors.push(`Sonde ${this.esc(num)}: ${this.esc(String(data[key]))}°C (${this.esc(data['addr' + num] || '?')})`);
      }
    }
    container.innerHTML = sensors.length
      ? '<strong>' + this.t('dash.probes_present') + '</strong><br>' + sensors.join('<br>')
      : this.t('dash.no_probe');
  },

  async applyConfig() {
    const form = document.getElementById('configForm');
    if (form && !form.checkValidity()) { form.reportValidity(); return; }
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
      this.showStatus('config-status', this.t('status.applied'));
    } catch (e) {
      this.showStatus('config-status', this.t('status.error_with', { msg: e.message }), true);
    }
  },

  async saveFlash() {
    try {
      await fetch('/get?save=yes');
      this.showStatus('config-status', this.t('status.saved_flash'));
    } catch (e) {
      this.showStatus('config-status', this.t('status.error_with', { msg: e.message }), true);
    }
  },

  async toggleOnOff() {
    try {
      const res = await fetch('/onoff');
      const val = await res.text();
      const btn = document.getElementById('btn-onoff');
      const isOn = val.trim() === '1';
      btn.textContent = this.t('dash.dimmer_status', { state: isOn ? this.t('state.on') : this.t('state.off') });
      btn.className = 'btn btn-sm ' + (isOn ? 'btn-success' : 'btn-danger');
    } catch (e) {
      console.error('Toggle error:', e);
    }
  },

  // ---------- MQTT ----------
  async loadMqtt() {
    document.getElementById('pageContent').innerHTML = `
      <h2 class="page-title">${this.t('page.mqtt')}</h2>
      <div style="display:flex;gap:.75rem;margin-bottom:1rem;flex-wrap:wrap">
        <button class="btn btn-primary btn-sm" id="btn-apply-mqtt">${this.t('btn.apply')}</button>
        <button class="btn btn-success btn-sm" id="btn-save-mqtt">${this.t('btn.save')}</button>
      </div>
      <div id="mqtt-status" class="alert alert-success" style="display:none"></div>
      <form id="mqttForm">
        <div class="card" style="margin-bottom:1rem">
          <div class="card-header">${this.t('card.connection_mqtt')}</div>
          <div class="card-body">
            <div class="form-check">
              <input type="checkbox" id="MQTT" onchange="App.sendServermode('MQTT')">
              <label for="MQTT">${this.t('form.activate_mqtt')}</label>
            </div>
            <div class="form-row">
              <div class="form-group">
                <label>${this.t('form.server')}</label>
                <input type="text" class="form-control" id="server" placeholder="${this.t('form.server_placeholder')}">
              </div>
              <div class="form-group">
                <label>${this.t('form.port')}</label>
                <input type="number" class="form-control" id="port" placeholder="1883" min="1" max="65535" step="1">
              </div>
              <div class="form-group">
                <label>${this.t('form.topic_domoticz')}</label>
                <input type="text" class="form-control" id="topic">
              </div>
            </div>
            <div class="form-row">
              <div class="form-group">
                <label>${this.t('form.user')}</label>
                <input type="text" class="form-control" id="user">
              </div>
              <div class="form-group">
                <label>${this.t('form.password')}</label>
                <input type="password" class="form-control" id="password">
              </div>
            </div>
          </div>
        </div>
        <div class="card" style="margin-bottom:1rem">
          <div class="card-header">${this.t('card.domoticz_idx')}</div>
          <div class="card-body">
            <div class="form-row">
              <div class="form-group">
                <label>${this.t('form.idx_temp_label')}</label>
                <input type="number" class="form-control" id="idxtemp" min="0" max="65535" step="1">
              </div>
              <div class="form-group">
                <label>${this.t('form.idx_power_label')}</label>
                <input type="number" class="form-control" id="IDX" min="0" max="65535" step="1">
              </div>
              <div class="form-group">
                <label>${this.t('form.idx_alarm_label')}</label>
                <input type="number" class="form-control" id="IDXAlarme" min="0" max="65535" step="1">
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
      this.showStatus('mqtt-status', this.t('status.error_with', { msg: e.message }), true);
    }

    document.getElementById('btn-apply-mqtt').addEventListener('click', () => this.applyMqtt());
    document.getElementById('btn-save-mqtt').addEventListener('click', async () => {
      try {
        await fetch('/getmqtt?save=yes');
        this.showStatus('mqtt-status', this.t('status.saved'));
      } catch (e) {
        this.showStatus('mqtt-status', this.t('status.error'), true);
      }
    });
  },

  async applyMqtt() {
    const form = document.getElementById('mqttForm');
    if (form && !form.checkValidity()) { form.reportValidity(); return; }
    const params = new URLSearchParams();
    ['server', 'port', 'topic', 'user', 'password', 'idxtemp', 'IDX', 'IDXAlarme'].forEach(f => {
      const el = document.getElementById(f);
      if (el) params.set(f === 'server' ? 'hostname' : f === 'topic' ? 'Publish' :
        f === 'user' ? 'mqttuser' : f === 'password' ? 'mqttpassword' : f, el.value);
    });
    try {
      await fetch('/get', {
        method: 'POST',
        headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
        body: params.toString(),
      });
      this.showStatus('mqtt-status', this.t('status.applied'));
    } catch (e) {
      this.showStatus('mqtt-status', this.t('status.error_with', { msg: e.message }), true);
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
      this.showStatus('mqtt-status', this.t('status.applied'));
    } catch (e) {
      this.showStatus('mqtt-status', this.t('status.error_with', { msg: e.message }), true);
    }
  },

  // ---------- Minuteur ----------
  async loadMinuteur() {
    const tabs = ['dimmer', 'relay1', 'relay2'];
    const labels = { dimmer: this.t('tab.dimmer'), relay1: this.t('tab.relay1'), relay2: this.t('tab.relay2') };
    document.getElementById('pageContent').innerHTML = `
      <h2 class="page-title">${this.t('page.minuteur')}</h2>
      <div id="minuteur-status" class="alert alert-success" style="display:none"></div>
      <div class="card">
        <div class="card-header">
          <div class="tabs" style="border:none;margin:0">
            ${tabs.map((t, i) => `<button class="tab-btn ${i === 0 ? 'active' : ''}" data-tab="${t}">${labels[t]}</button>`).join('')}
          </div>
        </div>
        <div class="card-body">
          ${tabs.map((t, i) => `
            <div class="tab-panel ${i === 0 ? 'active' : ''}" id="tab-${t}">
              <form id="form-${t}">
                <div class="form-row">
                  <div class="form-group">
                    <label>${this.t('form.heure_demarrage')}</label>
                    <input type="text" class="form-control" id="heure_demarrage_${t}" placeholder="HH:MM">
                  </div>
                  <div class="form-group">
                    <label>${this.t('form.heure_arret')}</label>
                    <input type="text" class="form-control" id="heure_arret_${t}" placeholder="HH:MM">
                  </div>
                  <div class="form-group">
                    <label>${this.t('form.temperature_consigne')}</label>
                    <input type="number" class="form-control" id="temperature_${t}" min="0" max="100" step="1">
                  </div>
                  ${t === 'dimmer' ? `
                  <div class="form-group">
                    <label>${this.t('form.puissance')}</label>
                    <input type="number" class="form-control" id="puissance_${t}" min="0" max="100" step="1">
                  </div>` : ''}
                </div>
                <button type="submit" class="btn btn-primary btn-sm" style="margin-top:.5rem">${this.t('btn.apply_target', { target: labels[t] })}</button>
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
        if (!e.target.checkValidity()) { e.target.reportValidity(); return; }
        const params = new URLSearchParams();
        params.set('heure_demarrage', document.getElementById('heure_demarrage_' + t).value);
        params.set('heure_arret', document.getElementById('heure_arret_' + t).value);
        params.set('temperature', document.getElementById('temperature_' + t).value);
        if (t === 'dimmer') params.set('puissance', document.getElementById('puissance_' + t).value);
        try {
          await fetch('/setminuteur?' + t + '&' + params.toString());
          this.showStatus('minuteur-status', this.t('status.applied_target', { target: labels[t] }));
        } catch (err) {
          this.showStatus('minuteur-status', this.t('status.error'), true);
        }
      });
    }
  },

  // ---------- Log ----------
  loadLog() {
    document.getElementById('pageContent').innerHTML = `
      <h2 class="page-title">${this.t('page.log')}</h2>
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
      <h2 class="page-title">${this.t('page.backup')}</h2>
      <div class="card-grid" style="grid-template-columns:repeat(auto-fill,minmax(320px,1fr))">
        <div class="card">
          <div class="card-header">${this.t('backup.card_backup')}</div>
          <div class="card-body" style="text-align:center">
            <button class="btn btn-primary" id="btn-backup">
              <svg viewBox="0 0 16 16" aria-hidden="true"><use href="/icons.svg#icon-backup"/></svg>
              ${this.t('btn.download_backup')}
            </button>
            <div id="backup-log" style="margin-top:1rem;font-size:.82rem;text-align:left"></div>
          </div>
        </div>
        <div class="card">
          <div class="card-header">${this.t('backup.card_restore')}</div>
          <div class="card-body" style="text-align:center">
            <div class="form-group">
              <input type="file" class="form-control" id="restoreFile" accept=".json">
            </div>
            <button class="btn btn-primary btn-sm" id="btn-restore">${this.t('btn.restore')}</button>
            <div style="margin-top:.75rem">
              <button class="btn btn-outline btn-sm" id="btn-save-restore">${this.t('btn.save_flash')}</button>
            </div>
            <div id="restore-log" style="margin-top:1rem;font-size:.82rem;text-align:left"></div>
          </div>
        </div>
        <div class="card">
          <div class="card-header">${this.t('backup.card_browser')}</div>
          <div class="card-body" style="text-align:center">
            <div id="browser-save-date" style="font-size:.82rem;color:var(--text-muted);margin-bottom:.75rem"></div>
            <div style="display:flex;gap:.5rem;justify-content:center;flex-wrap:wrap;margin-bottom:.5rem">
              <button class="btn btn-primary btn-sm" id="btn-browser-save">${this.t('btn.browser_save')}</button>
              <button class="btn btn-outline btn-sm" id="btn-browser-restore" disabled>${this.t('btn.browser_restore')}</button>
            </div>
            <div id="browser-log" style="margin-top:1rem;font-size:.82rem;text-align:left"></div>
          </div>
        </div>
      </div>
    `;

    const stored = localStorage.getItem('pvdimmer_backup');
    if (stored) {
      try {
        const d = JSON.parse(stored);
        if (d._saved) {
          document.getElementById('browser-save-date').textContent =
            this.t('backup.browser_last_save', { date: new Date(d._saved).toLocaleString() });
        }
        document.getElementById('btn-browser-restore').disabled = false;
      } catch (e) {}
    } else {
      document.getElementById('browser-save-date').textContent = this.t('backup.browser_no_save');
    }

    document.getElementById('btn-backup').addEventListener('click', () => this.doBackup());
    document.getElementById('btn-restore').addEventListener('click', () => this.doRestore());
    document.getElementById('btn-save-restore').addEventListener('click', async () => {
      try {
        await fetch('/get?save=yes');
        this.appendLog('restore-log', this.t('status.saved_flash'), 'success');
      } catch (e) {
        this.appendLog('restore-log', this.t('status.error'), 'danger');
      }
    });
    document.getElementById('btn-browser-save').addEventListener('click', () => this.doBrowserBackup());
    document.getElementById('btn-browser-restore').addEventListener('click', () => this.doBrowserRestore());
  },

  async doBackup() {
    const log = 'backup-log';
    document.getElementById(log).innerHTML = '';
    const requests = [
      { title: this.t('backup.req.general'), url: '/config', key: 'general' },
      { title: this.t('backup.req.mqtt'), url: '/getmqtt', key: 'mqtt' },
      { title: this.t('backup.req.timer_dimmer'), url: '/getminuteur?dimmer', key: 'dimmer_timer' },
      { title: this.t('backup.req.timer_relay1'), url: '/getminuteur?relay1', key: 'relay1_timer' },
      { title: this.t('backup.req.timer_relay2'), url: '/getminuteur?relay2', key: 'relay2_timer' },
    ];

    const backup = {};
    let hasError = false;

    for (const req of requests) {
      this.appendLog(log, this.t('backup.loading', { title: req.title }), 'info');
      try {
        const res = await fetch(req.url);
        backup[req.key] = await res.json();
        this.replaceLastLog(log, this.t('backup.ok', { title: req.title }), 'success');
      } catch (e) {
        this.replaceLastLog(log, this.t('backup.fail', { title: req.title }), 'danger');
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
      this.appendLog(log, this.t('backup.download_started'), 'success');
    }
  },

  async doRestore() {
    const log = 'restore-log';
    document.getElementById(log).innerHTML = '';
    const input = document.getElementById('restoreFile');
    if (!input.files.length) {
      this.appendLog(log, this.t('backup.select_file'), 'warn');
      return;
    }
    let data;
    try {
      data = JSON.parse(await input.files[0].text());
      this.appendLog(log, this.t('backup.file_loaded'), 'success');
    } catch (e) {
      this.appendLog(log, this.t('backup.file_invalid'), 'danger');
      return;
    }
    await this.applyRestoreData(data, log);
  },

  async doBrowserBackup() {
    const log = 'browser-log';
    document.getElementById(log).innerHTML = '';
    const requests = [
      { title: this.t('backup.req.general'), url: '/config', key: 'general' },
      { title: this.t('backup.req.mqtt'), url: '/getmqtt', key: 'mqtt' },
      { title: this.t('backup.req.timer_dimmer'), url: '/getminuteur?dimmer', key: 'dimmer_timer' },
      { title: this.t('backup.req.timer_relay1'), url: '/getminuteur?relay1', key: 'relay1_timer' },
      { title: this.t('backup.req.timer_relay2'), url: '/getminuteur?relay2', key: 'relay2_timer' },
    ];
    const backup = {};
    let hasError = false;
    for (const req of requests) {
      this.appendLog(log, this.t('backup.loading', { title: req.title }), 'info');
      try {
        backup[req.key] = await (await fetch(req.url)).json();
        this.replaceLastLog(log, this.t('backup.ok', { title: req.title }), 'success');
      } catch (e) {
        this.replaceLastLog(log, this.t('backup.fail', { title: req.title }), 'danger');
        hasError = true;
      }
    }
    if (!hasError) {
      backup._saved = new Date().toISOString();
      try {
        localStorage.setItem('pvdimmer_backup', JSON.stringify(backup));
        document.getElementById('browser-save-date').textContent =
          this.t('backup.browser_last_save', { date: new Date(backup._saved).toLocaleString() });
        document.getElementById('btn-browser-restore').disabled = false;
        this.appendLog(log, this.t('backup.browser_saved'), 'success');
      } catch (e) {
        this.appendLog(log, this.t('backup.browser_save_error'), 'danger');
      }
    }
  },

  async doBrowserRestore() {
    const log = 'browser-log';
    document.getElementById(log).innerHTML = '';
    let data;
    try {
      const stored = localStorage.getItem('pvdimmer_backup');
      if (!stored) { this.appendLog(log, this.t('backup.browser_empty'), 'warn'); return; }
      data = JSON.parse(stored);
    } catch (e) {
      this.appendLog(log, this.t('backup.file_invalid'), 'danger');
      return;
    }
    this.appendLog(log, this.t('backup.file_loaded'), 'success');
    await this.applyRestoreData(data, log);
  },

  async applyRestoreData(data, log) {
    const toBool = v => v === true || v === 'true' || v === 1 || v === '1' || v === 'on';
    const mqttToggles = ['MQTT'];
    const mqttRemap = { server: 'hostname', topic: 'Publish', user: 'mqttuser', password: 'mqttpassword' };

    let currentMqtt = {};
    try { currentMqtt = await (await fetch('/getmqtt')).json(); } catch (e) {}

    const titleGen = this.t('backup.req.general');
    const titleMqtt = this.t('backup.req.mqtt');

    if (data.general) {
      this.appendLog(log, this.t('backup.loading', { title: titleGen }), 'info');
      const params = new URLSearchParams();
      for (const [k, v] of Object.entries(data.general)) params.set(k, v);
      try {
        await fetch('/get?' + params.toString());
        this.replaceLastLog(log, this.t('backup.ok', { title: titleGen }), 'success');
      } catch (e) { this.replaceLastLog(log, this.t('backup.fail', { title: titleGen }), 'danger'); }
    } else {
      this.appendLog(log, this.t('backup.absent', { title: titleGen }), 'warn');
    }

    if (data.mqtt) {
      this.appendLog(log, this.t('backup.loading', { title: titleMqtt }), 'info');
      const params = new URLSearchParams();
      for (const [k, v] of Object.entries(data.mqtt)) {
        if (mqttToggles.includes(k)) continue;
        params.set(mqttRemap[k] || k, v);
      }
      try {
        await fetch('/get?' + params.toString());
        this.replaceLastLog(log, this.t('backup.ok', { title: titleMqtt }), 'success');
      } catch (e) { this.replaceLastLog(log, this.t('backup.fail', { title: titleMqtt }), 'danger'); }
    } else {
      this.appendLog(log, this.t('backup.absent', { title: titleMqtt }), 'warn');
    }

    for (const key of mqttToggles) {
      if (!data.mqtt) continue;
      const target = data.mqtt[key];
      const current = currentMqtt[key];
      if (target === undefined || current === undefined) continue;
      if (toBool(target) === toBool(current)) continue;
      try {
        await fetch('/get?servermode=' + key);
        this.appendLog(log, this.t('backup.toggle_to', { key, state: toBool(target) ? this.t('state.on') : this.t('state.off') }), 'success');
      } catch (e) { this.appendLog(log, this.t('backup.fail', { title: key }), 'danger'); }
    }

    const timers = [
      ['dimmer_timer', 'dimmer', this.t('backup.req.timer_dimmer')],
      ['relay1_timer', 'relay1', this.t('backup.req.timer_relay1')],
      ['relay2_timer', 'relay2', this.t('backup.req.timer_relay2')],
    ];
    for (const [field, type, label] of timers) {
      if (!data[field]) {
        this.appendLog(log, this.t('backup.absent', { title: label }), 'warn');
        continue;
      }
      this.appendLog(log, this.t('backup.loading', { title: label }), 'info');
      const params = new URLSearchParams();
      for (const [k, v] of Object.entries(data[field])) params.set(k, v);
      try {
        await fetch('/setminuteur?' + type + '&' + params.toString());
        this.replaceLastLog(log, this.t('backup.ok', { title: label }), 'success');
      } catch (e) { this.replaceLastLog(log, this.t('backup.fail', { title: label }), 'danger'); }
    }

    this.appendLog(log, this.t('backup.restore_done'), 'info');
  },

  // ---------- OTA Manuel ----------
  async loadOta() {
    document.getElementById('pageContent').innerHTML = `
      <h2 class="page-title">${this.t('page.ota')}</h2>
      <div class="card" style="max-width:520px">
        <div class="card-header">${this.t('ota.card_title')}</div>
        <div class="card-body">
          <div class="form-row" style="margin-bottom:1rem">
            <div class="form-group">
              <label>${this.t('ota.current_version')}</label>
              <div id="ota-current" style="font-weight:600;font-size:1.1rem;padding:.35rem 0">--</div>
            </div>
            <div class="form-group">
              <label>${this.t('ota.remote_version')}</label>
              <div id="ota-remote" style="font-weight:600;font-size:1.1rem;padding:.35rem 0">--</div>
            </div>
          </div>
          <div id="ota-result" style="margin-bottom:1rem"></div>
          <div style="display:flex;gap:.75rem;flex-wrap:wrap">
            <button class="btn btn-primary btn-sm" id="btn-ota-check">${this.t('ota.btn_check')}</button>
            <button class="btn btn-success btn-sm" id="btn-ota-install" style="display:none">${this.t('ota.btn_install')}</button>
          </div>
          <div id="ota-log" style="margin-top:1rem;font-size:.82rem"></div>
        </div>
      </div>
    `;

    try {
      const res = await fetch('/state');
      const d = await res.json();
      document.getElementById('ota-current').textContent = d.version || '--';
    } catch (e) {
      document.getElementById('ota-current').textContent = this.t('status.error');
    }

    document.getElementById('btn-ota-check').addEventListener('click', () => this.otaCheck());
    document.getElementById('btn-ota-install').addEventListener('click', () => this.otaInstall());
  },

  async otaCheck() {
    const btn = document.getElementById('btn-ota-check');
    const resultEl = document.getElementById('ota-result');
    const installBtn = document.getElementById('btn-ota-install');
    const origLabel = btn.textContent;
    btn.disabled = true;
    btn.textContent = this.t('ota.checking');
    resultEl.innerHTML = '';
    installBtn.style.display = 'none';

    let remote;
    try {
      const res = await fetch('/otacheck');
      remote = await res.json();
    } catch (e) {
      resultEl.innerHTML = `<div class="alert alert-danger show">${this.t('ota.check_error')}</div>`;
      btn.textContent = origLabel;
      btn.disabled = false;
      return;
    }

    const remoteVersion = String(remote.version || '').trim();
    document.getElementById('ota-remote').textContent = remoteVersion || '--';

    const currentRaw = document.getElementById('ota-current').textContent.trim();
    const currentText = currentRaw.replace(/\D/g, '');

    if (!remoteVersion) {
      resultEl.innerHTML = `<div class="alert alert-danger show">${this.t('ota.check_error')}</div>`;
    } else if (remoteVersion > currentText) {
      resultEl.innerHTML = `<div class="alert alert-success show">${this.t('ota.update_available', { ver: remoteVersion })}</div>`;
      installBtn.style.display = '';
    } else {
      resultEl.innerHTML = `<div class="alert alert-success show">${this.t('ota.up_to_date')}</div>`;
    }

    btn.textContent = origLabel;
    btn.disabled = false;
  },

  async otaInstall() {
    if (!confirm(this.t('ota.confirm_install'))) return;
    const installBtn = document.getElementById('btn-ota-install');
    const log = 'ota-log';
    installBtn.disabled = true;
    this.appendLog(log, this.t('ota.installing'), 'info');
    try {
      await fetch('/otaupdate');
      this.appendLog(log, this.t('ota.install_started'), 'success');
      this.otaFollowLog();
    } catch (e) {
      this.appendLog(log, this.t('ota.install_error'), 'danger');
      installBtn.disabled = false;
    }
  },

  otaFollowLog() {
    let logId = 0;
    const log = 'ota-log';
    const poll = async () => {
      if (this.currentPage !== 'ota') return;
      try {
        const res = await fetch('/cs?c2=' + logId);
        const text = await res.text();
        const parts = text.split(/\}1/);
        logId = parts.shift();
        parts.shift();
        const content = parts.shift();
        if (content && content.trim()) this.appendLog(log, content.trim(), 'info');
        setTimeout(poll, 1500);
      } catch (e) {
        this.appendLog(log, this.t('ota.rebooting'), 'warn');
      }
    };
    setTimeout(poll, 2000);
  },

  // ---------- Security ----------
  async loadSecurity() {
    document.getElementById('pageContent').innerHTML = `
      <h2 class="page-title">${this.t('page.security')}</h2>
      <div class="card" style="max-width:480px">
        <div class="card-body">
          <p style="color:var(--text-muted);margin-bottom:1.25rem">${this.t('security.subtitle')}</p>
          <div class="form-check" style="margin-bottom:1.25rem">
            <input type="checkbox" id="auth_enabled">
            <label for="auth_enabled">${this.t('security.enable')}</label>
          </div>
          <div class="form-group" style="margin-bottom:1.75rem">
            <label for="auth_pass">${this.t('security.password')}</label>
            <input type="password" class="form-control" id="auth_pass" autocomplete="new-password">
          </div>
          <div id="security-status" class="alert alert-success" style="display:none;margin-bottom:1rem"></div>
          <button class="btn btn-primary" id="btn-save-security" style="min-width:200px">${this.t('btn.save_settings')}</button>
        </div>
      </div>
    `;

    const cb = document.getElementById('auth_enabled');
    const passInput = document.getElementById('auth_pass');

    const syncPassState = () => {
      passInput.disabled = !cb.checked;
      passInput.style.opacity = cb.checked ? '1' : '0.5';
    };
    cb.addEventListener('change', syncPassState);

    try {
      const res = await fetch('/getauth');
      const data = await res.json();
      cb.checked = data.auth_enabled === true || data.auth_enabled === 1 || data.auth_enabled === '1';
      syncPassState();
    } catch (e) {
      console.error('Auth config load error:', e);
      syncPassState();
    }

    document.getElementById('btn-save-security').addEventListener('click', () => this.saveSecurity());
  },

  async saveSecurity() {
    const enabled = document.getElementById('auth_enabled').checked;
    const pass = document.getElementById('auth_pass').value;

    if (enabled && !pass) {
      this.showStatus('security-status', this.t('security.password_required'), true);
      return;
    }

    const params = new URLSearchParams();
    params.set('auth_enabled', enabled ? '1' : '0');
    if (pass) params.set('auth_pass', pass);

    try {
      await fetch('/setauth', {
        method: 'POST',
        headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
        body: params.toString(),
      });
      document.getElementById('auth_pass').value = '';
      this.showStatus('security-status', this.t('status.saved'));
    } catch (e) {
      this.showStatus('security-status', this.t('status.error_with', { msg: e.message }), true);
    }
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

    const openSidebar = () => {
      sidebar.classList.add('open');
      overlay.classList.add('show');
      if (toggle) toggle.setAttribute('aria-expanded', 'true');
      const firstLink = sidebar.querySelector('nav a');
      if (firstLink) firstLink.focus();
    };

    const closeSidebar = (returnFocus) => {
      sidebar.classList.remove('open');
      overlay.classList.remove('show');
      if (toggle) toggle.setAttribute('aria-expanded', 'false');
      if (returnFocus && toggle) toggle.focus();
    };

    if (toggle) {
      toggle.addEventListener('click', () => {
        if (sidebar.classList.contains('open')) closeSidebar(true);
        else openSidebar();
      });
    }

    if (overlay) {
      overlay.addEventListener('click', () => closeSidebar(true));
    }
  },
};

// Boot
document.addEventListener('DOMContentLoaded', () => App.init());

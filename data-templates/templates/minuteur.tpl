<%inherit file="config.tpl"/>
<%namespace name="config" file="config.tpl"/>
<%block name="page_title">Configuration minuteur d'appoint</%block>
<%block name="topbar_content">
            <ul class="navbar-nav ml-auto">
              <!-- Nav Item - Search Dropdown (Visible Only XS) -->
              <li class="nav-item dropdown no-arrow d-sm-none">
                <!-- Dropdown - Messages -->
                <div
                  class="dropdown-menu dropdown-menu-right p-3 shadow animated--grow-in"
                  aria-labelledby="searchDropdown"
                ></div>
              </li>
              <!-- Nav Item - Alerts -->
              <li class="nav-item mx-1">
                <a id="heure"> </a>
              </li>
              <div class="topbar-divider d-none d-sm-block"></div>
            </ul>
</%block>
<%block name="menu">
${config.generate_menu(page="minuteur")}
</%block>
<%block name="content">
            <!-- Page Heading -->
            <h1 class="h3 mb-2 text-gray-800">Configuration minuteur d'appoint</h1>
            <div class="col-lg-6">
              <div class="card">
                <div class="card-header">
                  <nav id="tabs">
                    <div class="nav nav-pills" id="nav-tab" role="tablist">
                      <button
                        class="nav-link active"
                        data-toggle="tab"
                        type="button"
                        role="tab"
                        data-target="#dimmer"
                        aria-controls="dimmer"
                        aria-selected="true"
                      >
                        Configuration dimmer
                      </button>
                      <button
                        class="nav-link"
                        data-toggle="tab"
                        type="button"
                        role="tab"
                        data-target="#relay1"
                        aria-controls="relay1"
                        aria-selected="false"
                      >
                        Configuration relais 1
                      </button>
                      <button
                        class="nav-link"
                        data-toggle="tab"
                        type="button"
                        role="tab"
                        data-target="#relay2"
                        aria-controls="relay2"
                        aria-selected="false"
                      >
                        Configuration relais 2
                      </button>
                    </div>
                  </nav>
                </div>

                <div class="tab-content">
                  <div role="tabpanel" class="card-body tab-pane fade show active" id="dimmer">
                    <form class="user" id="dimmer-form" method="post" action="">
                      <div class="form-group">
                        Heure de démarrage: (format hh:mm)
                        <input
                          type="text"
                          class="form-control form-control-user"
                          id="heure_demarrage_dimmer"
                          placeholder="HH:MM"
                        />
                      </div>
                      <div class="form-group">
                        Heure d'arrêt:
                        <input
                          type="text"
                          class="form-control form-control-user"
                          id="heure_arret_dimmer"
                          placeholder="HH:MM"
                        />
                      </div>
                      <div class="form-group">
                        Température de consigne (°C):
                        <input
                          type="number"
                          step="1"
                          class="form-control form-control-user"
                          id="temperature_dimmer"
                          placeholder="X (en °C)"
                        />
                      </div>
                      <div class="form-group">
                        Puissance de sortie (0-100&percnt;):
                        <input
                          type="number"
                          step="1"
                          class="form-control form-control-user"
                          id="puissance_dimmer"
                          placeholder="X (en &percnt;)"
                        />
                      </div>
                      <input
                        type="submit"
                        value="Application des paramètres dimmer"
                        class="btn btn-primary btn-user btn-block"
                      />
                    </form>
                  </div>

                  <div role="tabpanel" class="card-body tab-pane fade" id="relay1">
                    <form class="user" id="relay1-form" method="post" action="">
                      <div class="form-group">
                        Heure de démarrage: (format hh:mm)
                        <input
                          type="text"
                          class="form-control form-control-user"
                          id="heure_demarrage_relay1"
                          placeholder="HH:MM"
                        />
                      </div>
                      <div class="form-group">
                        Heure d'arrêt:
                        <input
                          type="text"
                          class="form-control form-control-user"
                          id="heure_arret_relay1"
                          placeholder="HH:MM"
                        />
                      </div>
                      <div class="form-group">
                        Température de consigne (°C):
                        <input
                          type="number"
                          step="1"
                          class="form-control form-control-user"
                          id="temperature_relay1"
                          placeholder="X (en °C)"
                        />
                      </div>
                      <input
                        type="submit"
                        value="Application des paramètres relais 1"
                        class="btn btn-primary btn-user btn-block"
                      />
                    </form>
                  </div>

                  <div role="tabpanel" class="card-body tab-pane fade" id="relay2">
                    <form class="user" id="relay2-form" method="post" action="">
                      <div class="form-group">
                        Heure de démarrage: (format hh:mm)
                        <input
                          type="text"
                          class="form-control form-control-user"
                          id="heure_demarrage_relay2"
                          placeholder="HH:MM"
                        />
                      </div>
                      <div class="form-group">
                        Heure d'arrêt:
                        <input
                          type="text"
                          class="form-control form-control-user"
                          id="heure_arret_relay2"
                          placeholder="HH:MM"
                        />
                      </div>
                      <div class="form-group">
                        Température de consigne (°C):
                        <input
                          type="number"
                          step="1"
                          class="form-control form-control-user"
                          id="temperature_relay2"
                          placeholder="X (en °C)"
                        />
                      </div>
                      <input
                        type="submit"
                        value="Application des paramètres relais 2"
                        class="btn btn-primary btn-user btn-block"
                      />
                    </form>
                  </div>
                </div>
              </div>
            </div>
</%block>
<%block name="pagescript">
<%text>
    <script type="text/javascript">
      $(document).ready(function () {
        // Configuration centralisée
        const CONFIG = {
          UPDATE_INTERVAL: 15000,
          DEVICES: ['relay1', 'relay2', 'dimmer']
        };

        // Utilitaires
        const Utils = {
          formatTime: (hour, minute) => {
            const h = hour < 10 ? "0" + hour : hour;
            const m = minute < 10 ? "0" + minute : minute;
            return `${h}:${m}`;
          },

          showNotification: (message, type = 'info') => {
            // Placeholder pour système de notification
            console.log(`[${type.toUpperCase()}] ${message}`);
          },

          handleError: (error, context) => {
            console.error(`Erreur dans ${context}:`, error);
            Utils.showNotification(`Erreur: ${context}`, 'error');
          }
        };

        // Gestionnaire des onglets
        const TabManager = {
          init() {
            $("#tabs button").on("click", this.handleTabClick);
            this.showLastTab();
          },

          handleTabClick(e) {
            e.preventDefault();
            const target = $(this).attr("data-target");
            if (target) {
              window.location.hash = target;
              $(this).tab("show");
            }
          },

          showLastTab() {
            if (window.location.hash) {
              const tabSelector = `#tabs button[aria-controls=${window.location.hash.substring(1)}]`;
              $(tabSelector).tab("show");
              window.scrollTo({ top: 0, behavior: 'smooth' });
            }
          }
        };

        // Gestionnaire des données de minuteurs
        const TimerDataManager = {
          cache: new Map(),
          
          async loadDeviceData(device) {
            try {
              const data = await $.get(`/getminuteur?${device}`);
              this.cache.set(device, data);
              this.updateUI(device, data);
              
              // Mise à jour spéciale de l'horloge pour relay2
              if (device === 'relay2' && data.heure !== undefined && data.minute !== undefined) {
                this.updateClock(data.heure, data.minute);
              }
              
              return data;
            } catch (error) {
              Utils.handleError(error, `Chargement des données ${device}`);
              throw error;
            }
          },

          updateUI(device, data) {
            const prefix = `#heure_demarrage_${device}`;
            $(`${prefix.replace('demarrage', 'demarrage')}`).val(data.heure_demarrage);
            $(`${prefix.replace('demarrage', 'arret')}`).val(data.heure_arret);
            $(`#temperature_${device}`).val(data.temperature);
            
            // Champ spécifique au dimmer
            if (device === 'dimmer' && data.puissance !== undefined) {
              $(`#puissance_${device}`).val(data.puissance);
            }
          },

          updateClock(hour, minute) {
            const timeString = Utils.formatTime(hour, minute);
            $("#heure").html(timeString);
          },

          async saveDeviceData(device, formData) {
            try {
              await $.get(`/setminuteur?${device}`, formData);
              Utils.showNotification(`Configuration ${device} sauvegardée`, 'success');
              
              // Mise à jour du cache
              this.cache.set(device, { ...this.cache.get(device), ...formData });
            } catch (error) {
              Utils.handleError(error, `Sauvegarde ${device}`);
              throw error;
            }
          }
        };

        // Gestionnaire des formulaires
        const FormManager = {
          init() {
            CONFIG.DEVICES.forEach(device => {
              $(`#${device}-form`).on("submit", (e) => this.handleSubmit(e, device));
            });
          },

          async handleSubmit(event, device) {
            event.preventDefault();
            
            try {
              const formData = this.getFormData(device);
              await TimerDataManager.saveDeviceData(device, formData);
            } catch (error) {
              // L'erreur est déjà gérée dans saveDeviceData
            }
          },

          getFormData(device) {
            const data = {
              heure_demarrage: $(`#heure_demarrage_${device}`).val(),
              heure_arret: $(`#heure_arret_${device}`).val(),
              temperature: $(`#temperature_${device}`).val()
            };

            // Champ spécifique au dimmer
            if (device === 'dimmer') {
              data.puissance = $(`#puissance_${device}`).val();
            }

            return data;
          }
        };

        // Gestionnaire de mise à jour périodique
        const UpdateManager = {
          intervalId: null,

          start() {
            this.intervalId = setInterval(() => {
              this.updateClock();
            }, CONFIG.UPDATE_INTERVAL);
          },

          stop() {
            if (this.intervalId) {
              clearInterval(this.intervalId);
              this.intervalId = null;
            }
          },

          async updateClock() {
            try {
              const data = await $.getJSON("/getminuteur");
              if (data.heure !== undefined && data.minute !== undefined) {
                TimerDataManager.updateClock(data.heure, data.minute);
              }
            } catch (error) {
              Utils.handleError(error, 'Mise à jour de l\'horloge');
            }
          }
        };

        // Initialisation de l'application
        const App = {
          async init() {
            try {
              // Initialisation des gestionnaires
              TabManager.init();
              FormManager.init();

              // Chargement initial des données
              await Promise.allSettled(
                CONFIG.DEVICES.map(device => TimerDataManager.loadDeviceData(device))
              );

              // Démarrage des mises à jour périodiques
              UpdateManager.start();

              console.log('Application initialisée avec succès');
            } catch (error) {
              Utils.handleError(error, 'Initialisation de l\'application');
            }
          },

          destroy() {
            UpdateManager.stop();
          }
        };

        // Nettoyage lors du déchargement de la page
        $(window).on('beforeunload', () => {
          App.destroy();
        });

        // Démarrage de l'application
        App.init();
      });
    </script>
</%text>
</%block>

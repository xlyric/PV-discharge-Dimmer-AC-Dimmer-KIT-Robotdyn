<%inherit file="config.tpl"/>
<%namespace name="config" file="config.tpl"/>
<%block name="page_title">Sauvegarde et restauration de la configuration</%block>
<%block name="topbar_content"></%block>
<%block name="menu">
${config.generate_menu(page="backup")}
</%block>
<%block name="content">
            <!-- Page Heading -->
            <h1 class="h3 mb-2 text-gray-800">Sauvegarde et restauration de la configuration</h1>
            <div class="row">
              <div class="col-lg-6">
                <div class="card position-relative">
                  <div class="card-header py-3">
                    <h6 class="m-0 font-weight-bold text-primary">Sauvegarder la configuration</h6>
                  </div>
                  <div class="card-body text-center">
                    <button id="backup" class="btn btn-primary">
                      <i class="fas fa-download"></i>
                      Sauvegarder
                    </button>
                  </div>
                </div>
              </div>
              <div class="col-lg-6">
                <div class="card position-relative">
                  <div class="card-header py-3">
                    <h6 class="m-0 font-weight-bold text-primary">Restaurer la configuration</h6>
                  </div>
                  <div class="card-body text-center">
                    <form id="restoreForm" enctype="multipart/form-data">
                      <div class="form-group">
                        Fichier de sauvegarde :
                        <input
                          type="file"
                          step="1"
                          class="form-control form-control-user"
                          id="restoreFile"
                          accept=".json" required"
                        />
                      </div>
                      <div class="form-group">
                        <button type="submit" class="btn btn-primary">
                          <i class="fas fa-upload"></i>
                          Restaurer la configuration
                        </button>
                      </div>
                    </form
                    <div class="form-group">
                      <button id="save" class="btn btn-secondary">
                        <i class="fas fa-download"></i>
                        Sauvegarder sur la flash
                      </button>
                    </div>
                    <div id="alertBox" style="display: none"></div>
                  </div>
                </div>
              </div>
            </div>
</%block>
<%block name="pagescript">
<%text>
    <script type="text/javascript">
      var alertBox = $("#alertBox");

      $("#backup").click(function () {
        const urls = {
          "general": "config",
          "mqtt": "getmqtt",
          "dimmer_timer": "getminuteur?dimmer",
          "relay1_timer": "getminuteur?relay1",
          "relay2_timer": "getminuteur?relay2",
        };

        function fetchBackupData(urls) {
          const promises = Object.keys(urls).map(key =>
            $.ajax({
              url: urls[key],
              method: 'GET',
              dataType: 'json'
            })
          );
          return Promise.all(promises);
        }

        function computeBackupFile(urls, results) {
          const data = {};
          Object.keys(urls).forEach((key, index) => {
            data[key] = results[index];
          });
          return data;
        }

        function createDownloadLink(data, fileName) {
          const jsonString = JSON.stringify(data, null, 2);
          const blob = new Blob([jsonString], { type: 'application/json' });
          const url = URL.createObjectURL(blob);

          const a = document.createElement('a');
          a.href = url;
          a.download = fileName;
          document.body.appendChild(a);
          a.click();
          document.body.removeChild(a);
          URL.revokeObjectURL(url);
        }

        fetchBackupData(urls).then(results => {
          const data = computeBackupFile(urls, results);
          createDownloadLink(data, 'backup.json');
        }).catch(error => {
          console.error('Error fetching backup data:', error);
        });
      });

      function getKey(data, keyString) {
        var keys = keyString.split('.');
        var current = data;
        for (var i = 0; i < keys.length; i++) {
          if (current[keys[i]] === undefined) return undefined;
          current = current[keys[i]];
        }
        return current;
      }

      function restoreConfig(data) {
        alertBox.css("display", "none");
        alertBox.html("");
        const urls = [
          {
            "title": "Configuration générale",
            "url": "get",
            "data": {
              "maxtemp": "general.maxtemp",
              "startingpow": "general.startingpow",
              "minpow": "general.minpow",
              "maxpow": "general.maxpow",
              "child": "general.child",
              "SubscribePV": "general.SubscribePV",
              "SubscribeTEMP": "general.SubscribeTEMP",
              "mode": "general.delester",
              "charge1": "general.charge1",
              "charge2": "general.charge2",
              "charge3": "general.charge3",
              "DALLAS": "general.DALLAS",
              "dimmername": "general.dimmername",
              "trigger": "general.trigger",
            },
          },
          {
            "title": "Configuration MQTT",
            "url": "get",
            "data": {
              "hostname": "mqtt.server",
              "port": "mqtt.port",
              "Publish": "mqtt.topic",
              "mqttuser": "mqtt.user",
              "mqttpassword": "mqtt.password",
              "idxtemp": "mqtt.idxtemp",
              "IDXAlarme": "mqtt.IDXAlarme",
              "IDX": "mqtt.IDX",
            },
          },
          {
            "title": "Configuration du minuteur d'appoint du dimmer",
            "url": "setminuteur",
            "data": {
              "dimmer": "",
              "heure_demarrage": "dimmer_timer.heure_demarrage",
              "heure_arret": "dimmer_timer.heure_arret",
              "temperature": "dimmer_timer.temperature",
              "puissance": "dimmer_timer.puissance",
            },
          },
          {
            "title": "Configuration du minuteur d'appoint du relai 1",
            "url": "setminuteur",
            "data": {
              "relay1": "",
              "heure_demarrage": "relay1_timer.heure_demarrage",
              "heure_arret": "relay1_timer.heure_arret",
              "temperature": "relay1_timer.temperature",
              "puissance": "relay1_timer.puissance",
            },
          },
          {
            "title": "Configuration du minuteur d'appoint du relai 2",
            "url": "setminuteur",
            "data": {
              "relay2": "",
              "heure_demarrage": "relay2_timer.heure_demarrage",
              "heure_arret": "relay2_timer.heure_arret",
              "temperature": "relay2_timer.temperature",
              "puissance": "relay2_timer.puissance",
            },
          },
        ];

        urls.forEach(request => {
          const params = {};
          for (const key in request.data) {
            if (request.data.hasOwnProperty(key)) {
              const value = getKey(data, request.data[key]);
              if (value !== undefined) {
                params[key] = value;
              }
            }
          }
          if (Object.keys(params).length == 0) {
            alertBox.append(`<div class="alert alert-warning" role="alert">❓ ${request.title} : aucune information trouvée dans la sauvegarde</div>`);
            alertBox.css("display", "initial");
            return;
          }

          $.ajax({
            url: request.url,
            method: 'GET',
            data: params,
            success: function(data) {
              alertBox.append(`<div class="alert alert-success" role="alert">✅ ${request.title}</div>`);
              alertBox.css("display", "initial");
            },
            error: function(xhr, status, error) {
              alertBox.append(`<div class="alert alert-danger" role="alert">⛔ ${request.title} : ${error?error:"Une erreur est survenue durant la restauration."}</div>`);
              alertBox.css("display", "initial");
            }
          });
        });
      }

      $('#restoreForm').on('submit', function(e) {
        e.preventDefault(); // Empêche le formulaire de soumettre normalement

        var input = $('#restoreFile')[0];
        if (input.files.length > 0) {
          var file = input.files[0];
          var reader = new FileReader();

          reader.onload = function(e) {
            try {
              var data = JSON.parse(e.target.result);
              restoreConfig(data);
            } catch (e) {
              console.error('Error parsing JSON file:', e);
            }
          };

          reader.onerror = function(e) {
            console.error('Error loading JSON file:', e);
          };

          reader.readAsText(file);
        }
    });

    $("#save").click(function() {
      alertBox.css("display", "none");
      alertBox.html("");
      $.ajax({
            url: "get",
            method: "GET",
            data: {"save": "yes"},
            success: function(data) {
              alertBox.append(`<div class="alert alert-success" role="alert">✅ Configuration sauvegardée sur la flash</div>`);
              alertBox.css("display", "initial");
            },
            error: function(xhr, status, error) {
              alertBox.append(`<div class="alert alert-danger" role="alert">⛔ Une erreur est survenue durant l'enregistrement de la configuration sur la flash.</div>`);
              alertBox.css("display", "initial");
            }
          });
    });
    </script>
</%text>
</%block>

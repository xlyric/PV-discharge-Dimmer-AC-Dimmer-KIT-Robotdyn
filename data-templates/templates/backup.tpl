<%inherit file="config.tpl"/>
<%namespace name="config" file="config.tpl"/>
<%block name="page_title">Sauvegarde et restauration de la configuration</%block>
<%block name="topbar_content"><!-- Put something here to avoid prettier reformat all page --></%block>
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
                    <div class="m-3 text-left" id="backupAlerts" style="display: none"></div>
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
                          accept=".json"
                          required
                        />
                      </div>
                      <div class="form-group">
                        <button type="submit" class="btn btn-primary">
                          <i class="fas fa-upload"></i>
                          Restaurer la configuration
                        </button>
                      </div>
                    </form>
                    <div class="form-group">
                      <button id="save" class="btn btn-sm btn-secondary">
                        <i class="fas fa-download"></i>
                        Sauvegarder sur la flash
                      </button>
                    </div>
                    <div class="m-3 text-left" id="restoreAlerts" style="display: none"></div>
                  </div>
                </div>
              </div>
            </div>
</%block>
<%block name="pagescript">
<%text>
    <script type="text/javascript">
      /*
       * Sauvergarde de la configuration
       */
      var backupAlerts = $("#backupAlerts");
      $("#backup").click(function () {
        const requests = [
          {
            title: "Configuration générale",
            url: "config",
            key: "general",
          },
          {
            title: "Configuration MQTT",
            url: "getmqtt",
            key: "mqtt",
          },
          {
            title: "Configuration du minuteur d'appoint du dimmer",
            url: "getminuteur?dimmer",
            key: "dimmer_timer",
          },
          {
            title: "Configuration du minuteur d'appoint du relai 1",
            url: "getminuteur?relay1",
            key: "relay1_timer",
          },
          {
            title: "Configuration du minuteur d'appoint du relai 1",
            url: "getminuteur?relay2",
            key: "relay2_timer",
          },
        ];
        var backup = {};
        var error = false;
        function fetchBackupData(request) {
          var backupAlert = $(
            `<div class="alert alert-info" role="alert">⌛ ${request.title}...</div>`,
          );
          backupAlerts.append(backupAlert);
          return $.getJSON(request.url)
            .then(function (data) {
              backup[request.key] = data;
              backupAlert.removeClass("alert-info").addClass("alert-success");
              backupAlert.text(`✅ ${request.title}`);
            })
            .catch(function (error) {
              backupAlert.removeClass("alert-info").addClass("alert-danger");
              backupAlert.text(
                `⛔ ${request.title} : ${error ? error : "Une erreur est survenue durant le téléchargement."}`,
              );
              error = true;
            });
        }
        function createDownloadLink() {
          const now = new Date().toISOString().replace(/[TZ]/g, "-").replace(/\..+/, "");
          const dimmer_name = $("#dimmer_name").text().replace(".local", "");
          const jsonString = JSON.stringify(backup, null, 2);
          const blob = new Blob([jsonString], { type: "application/json" });
          const url = URL.createObjectURL(blob);
          const a = document.createElement("a");
          a.href = url;
          a.download = `${now}-${dimmer_name}.json`;
          document.body.appendChild(a);
          a.click();
          document.body.removeChild(a);
          URL.revokeObjectURL(url);
        }
        backupAlerts.html(
          `<div class="alert alert-info" role="alert">
            ⌛ Construction de la sauvegarde...
          </div>`,
        );
        backupAlerts.css("display", "");
        requests
          .reduce(function (promise, request) {
            return promise.then(function () {
              return fetchBackupData(request);
            });
          }, Promise.resolve())
          .then(function () {
            if (error) {
              backupAlerts.append(
                `<div class="alert alert-danger" role="alert">
                  Une erreur est survenue durant la construction de la sauvegarde.
                </div>`,
              );
            } else {
              backupAlerts.append(
                `<div class="alert alert-info" role="alert">
                ✨ Construction de la sauvegarde terminée.
              </div>`,
              );
              createDownloadLink();
            }
          })
          .catch(function (error) {
            console.error("Error fetching backup data:", error);
          });
      });
      /*
       * Restauration
       */
      var restoreAlerts = $("#restoreAlerts");
      function getKey(data, keyString) {
        var keys = keyString.split(".");
        var current = data;
        for (var i = 0; i < keys.length; i++) {
          if (current[keys[i]] === undefined) return undefined;
          current = current[keys[i]];
        }
        return current;
      }
      var error = false;
      function restoreConfig(data) {
        const requests = [
          {
            title: "Configuration générale",
            url: "get",
            data: {
              maxtemp: "general.maxtemp",
              startingpow: "general.startingpow",
              minpow: "general.minpow",
              maxpow: "general.maxpow",
              child: "general.child",
              SubscribePV: "general.SubscribePV",
              SubscribeTEMP: "general.SubscribeTEMP",
              mode: "general.delester",
              charge1: "general.charge1",
              charge2: "general.charge2",
              charge3: "general.charge3",
              DALLAS: "general.DALLAS",
              dimmername: "general.dimmername",
              trigger: "general.trigger",
            },
          },
          {
            title: "Configuration MQTT",
            url: "get",
            data: {
              hostname: "mqtt.server",
              port: "mqtt.port",
              Publish: "mqtt.topic",
              mqttuser: "mqtt.user",
              mqttpassword: "mqtt.password",
              idxtemp: "mqtt.idxtemp",
              IDXAlarme: "mqtt.IDXAlarme",
              IDX: "mqtt.IDX",
            },
          },
          {
            title: "Configuration du minuteur d'appoint du dimmer",
            url: "setminuteur?dimmer",
            data: {
              dimmer: "",
              heure_demarrage: "dimmer_timer.heure_demarrage",
              heure_arret: "dimmer_timer.heure_arret",
              temperature: "dimmer_timer.temperature",
              puissance: "dimmer_timer.puissance",
            },
          },
          {
            title: "Configuration du minuteur d'appoint du relai 1",
            url: "setminuteur?relay1",
            data: {
              relay1: "",
              heure_demarrage: "relay1_timer.heure_demarrage",
              heure_arret: "relay1_timer.heure_arret",
              temperature: "relay1_timer.temperature",
              puissance: "relay1_timer.puissance",
            },
          },
          {
            title: "Configuration du minuteur d'appoint du relai 2",
            url: "setminuteur?relay2",
            data: {
              relay2: "",
              heure_demarrage: "relay2_timer.heure_demarrage",
              heure_arret: "relay2_timer.heure_arret",
              temperature: "relay2_timer.temperature",
              puissance: "relay2_timer.puissance",
            },
          },
        ];
        function restoreOne(request) {
          // Add alert message for this restoration
          var restoreAlert = $(
            `<div class="alert alert-info" role="alert">⌛ ${request.title} : restoration...</div>`,
          );
          restoreAlerts.append(restoreAlert);
          // Compute request parameters
          const params = {};
          for (const key in request.data) {
            if (request.data.hasOwnProperty(key)) {
              const value = getKey(data, request.data[key]);
              if (value !== undefined) {
                params[key] = value;
              }
            }
          }
          // Check we have at lest one parameter to restore
          if (Object.keys(params).length == 0) {
            restoreAlert.removeClass("alert-info").addClass("alert-warning");
            restoreAlert.text(
              `❓ ${request.title} : aucune information trouvée dans la sauvegarde`,
            );
            return;
          }
          // Make restoration request
          return $.getJSON(request.url, params)
            .then(function (data) {
              restoreAlert.removeClass("alert-info").addClass("alert-success");
              restoreAlert.text(`✅ ${request.title} : restaurée.`);
            })
            .catch(function (error) {
              restoreAlert.removeClass("alert-info").addClass("alert-danger");
              restoreAlert.text(
                `⛔ ${request.title} : ${error ? error : "Une erreur est survenue durant la restauration."}`,
              );
              error = true;
            });
        }
        requests
          .reduce(function (promise, request) {
            return promise.then(function () {
              return restoreOne(request);
            });
          }, Promise.resolve())
          .then(function () {
            console.log("Restoration finish");
            if (!error)
              restoreAlerts.append(
                `<div class="alert alert-info" role="alert">
                ✨ Restauration terminée. Pensez à sauvegarder la configuration sur la flash.
              </div>`,
              );
          })
          .catch(function (error) {
            console.error("Error occurred restoring configuraion:", error);
          });
      }
      $("#restoreForm").on("submit", function (e) {
        e.preventDefault(); // Empêche le formulaire de soumettre normalement
        restoreAlerts.text("");
        restoreAlerts.css("display", "");
        var input = $("#restoreFile")[0];
        if (input.files.length > 0) {
          loadingAlert = $(
            `<div class="alert alert-info" role="alert">⌛ Chargement du fichier sauvegarde...</div>`,
          );
          restoreAlerts.append(loadingAlert);
          var file = input.files[0];
          var reader = new FileReader();
          reader.onload = function (e) {
            try {
              var data = JSON.parse(e.target.result);
              loadingAlert.removeClass("alert-info").addClass("alert-success");
              loadingAlert.text("✅ Sauvegarde chargée");
              restoreConfig(data);
            } catch (e) {
              loadingAlert.removeClass("alert-info").addClass("alert-danger");
              loadingAlert.text(
                "⛔ Une erreur est survenue en chargeant le fichier de sauvegarde : fichier invalide ou corrompu.",
              );
            }
          };
          reader.onerror = function (e) {
            loadingAlert.removeClass("alert-info").addClass("alert-danger");
            loadingAlert.text(
              "⛔ Une erreur est survenue en chargeant le fichier de sauvegarde : impossible de lire le contenu du fichier.",
            );
          };
          reader.readAsText(file);
        } else {
          restoreAlerts.append(
            `<div class="alert alert-warning" role="alert">⛔ Sélectionnez un fichier de sauvegarde pour commencer.</div>`,
          );
        }
      });
      $("#save").click(function () {
        var saverAlert = $(
          `<div class="alert alert-info" role="alert">⌛ Sauvegarde de la configuration sur la flash...</div>`,
        );
        restoreAlerts.text("");
        restoreAlerts.append(saverAlert);
        restoreAlerts.css("display", "");
        $.ajax({
          url: "get",
          method: "GET",
          data: { save: "yes" },
          success: function (data) {
            saverAlert.removeClass("alert-info").addClass("alert-success");
            saverAlert.text("✅ Configuration sauvegardée sur la flash");
          },
          error: function (xhr, status, error) {
            saverAlert.removeClass("alert-info").addClass("alert-danger");
            saverAlert.text(
              "⛔ Une erreur est survenue durant l'enregistrement de la configuration sur la flash.",
            );
          },
        });
      });
    </script>
</%text>
</%block>

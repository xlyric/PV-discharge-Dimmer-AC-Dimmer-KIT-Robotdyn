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

    <script type="text/javascript" defer>
      $(document).ready(function () {
        // Enable tabs
        $("#tabs button").click(function (e) {
          e.preventDefault();
          window.location.hash = $(this).attr("data-target");
          $(this).tab("show");
        });

        // Show last tab on refresh
        if (window.location.hash) {
          $(`#tabs button[aria-controls=${window.location.hash.substring(1)}]`).tab("show");
          window.scrollTo({ top: 0 });
        }

        // Récupération des heures de démarrage et d'arrêt depuis le serveur
        $.get("/getminuteur?relay2", function (data) {
          $("#heure_demarrage_relay2").val(data.heure_demarrage);
          $("#heure_arret_relay2").val(data.heure_arret);
          $("#temperature_relay2").val(data.temperature);
          // Affichage de l'heure au format 2 digits
          var heure = data.heure < 10 ? "0" + data.heure : data.heure;
          var minute = data.minute < 10 ? "0" + data.minute : data.minute;
          $("#heure").html(heure + ":" + minute);
        });

        $.get("/getminuteur?relay1", function (data) {
          $("#heure_demarrage_relay1").val(data.heure_demarrage);
          $("#heure_arret_relay1").val(data.heure_arret);
          $("#temperature_relay1").val(data.temperature);
        });

        $.get("/getminuteur?dimmer", function (data) {
          $("#heure_demarrage_dimmer").val(data.heure_demarrage);
          $("#heure_arret_dimmer").val(data.heure_arret);
          $("#temperature_dimmer").val(data.temperature);
          $("#puissance_dimmer").val(data.puissance);
        });

        // Envoi des données de formulaire au serveur lors de la soumission du formulaire
        $("#dimmer-form").submit(function (event) {
          event.preventDefault();
          var heure_demarrage = $("#heure_demarrage_dimmer").val();
          var heure_arret = $("#heure_arret_dimmer").val();
          var temperature = $("#temperature_dimmer").val();
          var puissance = $("#puissance_dimmer").val();
          $.get("/setminuteur?dimmer", {
            heure_demarrage: heure_demarrage,
            heure_arret: heure_arret,
            temperature: temperature,
            puissance: puissance,
          });
        });

        $("#relay1-form").submit(function (event) {
          event.preventDefault();
          var heure_demarrage = $("#heure_demarrage_relay1").val();
          var heure_arret = $("#heure_arret_relay1").val();
          var temperature = $("#temperature_relay1").val();
          $.get("/setminuteur?relay1", {
            heure_demarrage: heure_demarrage,
            heure_arret: heure_arret,
            temperature: temperature,
          });
        });

        $("#relay2-form").submit(function (event) {
          event.preventDefault();
          var heure_demarrage = $("#heure_demarrage_relay2").val();
          var heure_arret = $("#heure_arret_relay2").val();
          var temperature = $("#temperature_relay2").val();
          $.get("/setminuteur?relay2", {
            heure_demarrage: heure_demarrage,
            heure_arret: heure_arret,
            temperature: temperature,
          });
        });
      });

      setInterval(function () {
        $.getJSON("/getminuteur", function (data) {
          // Affichage de l'heure au format 2 digits
          var heure = data.heure < 10 ? "0" + data.heure : data.heure;
          var minute = data.minute < 10 ? "0" + data.minute : data.minute;
          $("#heure").html(heure + ":" + minute);
        });
      }, 15000);
    </script>
</%text>
</%block>

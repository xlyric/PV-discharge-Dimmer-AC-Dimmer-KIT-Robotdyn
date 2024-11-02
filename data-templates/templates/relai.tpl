<%inherit file="config.tpl"/>
<%block name="page_title">Configuration Relais</%block>
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
<%block name="content">
            <!-- Page Heading -->
            <h1 class="h3 mb-2 text-gray-800">Configuration Relais</h1>
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
                        data-target="#relay1"
                        aria-controls="relay1"
                        aria-selected="true"
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
                  <div role="tabpanel" class="card-body tab-pane fade show active" id="relay1">
                    <form class="user" id="relay1-form" method="post" action="">
                      <div class="form-group">
                        Seuil de démarrage en &percnt;
                        <input
                          type="text"
                          class="form-control form-control-user"
                          id="seuil_demarrage_relay1"
                          placeholder="X (en &percnt;)"
                        />
                      </div>
                      <div class="form-group">
                        Seuil d'arrêt en &percnt;
                        <input
                          type="text"
                          class="form-control form-control-user"
                          id="seuil_arret_relay1"
                          placeholder="X (en &percnt;)"
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
                        value="Application des paramètres relay1"
                        class="btn btn-primary btn-user btn-block"
                      />
                    </form>
                  </div>

                  <div role="tabpanel" class="card-body tab-pane fade" id="relay2">
                    <form class="user" id="relay2-form" method="post" action="">
                      <div class="form-group">
                        Seuil de démarrage en &percnt;
                        <input
                          type="text"
                          class="form-control form-control-user"
                          id="seuil_demarrage_relay2"
                          placeholder="X (en &percnt;)"
                        />
                      </div>
                      <div class="form-group">
                        Seuil d'arrêt en &percnt;
                        <input
                          type="text"
                          class="form-control form-control-user"
                          id="seuil_arret_relay2"
                          placeholder="X (en &percnt;)"
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
                        value="Application des paramètres relay2"
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
        $.get("/getseuil?relay2", function (data) {
          $("#seuil_demarrage_relay2").val(data.seuil_start);
          $("#seuil_arret_relay2").val(data.seuil_stop);
          $("#temperature_relay2").val(data.seuil_temp);
          // Affichage de l'heure au format 2 digits
          var heure = data.heure < 10 ? "0" + data.heure : data.heure;
          var minute = data.minute < 10 ? "0" + data.minute : data.minute;
          $("#heure").html(heure + ":" + minute);
        });

        $.get("/getseuil?relay1", function (data) {
          $("#seuil_demarrage_relay1").val(data.seuil_start);
          $("#seuil_arret_relay1").val(data.seuil_stop);
          $("#temperature_relay1").val(data.seuil_temp);
        });

        // Envoi des données de formulaire au serveur lors de la soumission du formulaire

        $("#relay1-form").submit(function (event) {
          event.preventDefault();
          var seuil_demarrage = $("#seuil_demarrage_relay1").val();
          var seuil_arret = $("#seuil_arret_relay1").val();
          var temperature = $("#temperature_relay1").val();
          $.get("/setseuil?relay1", {
            seuil_demarrage: seuil_demarrage,
            seuil_arret: seuil_arret,
            temperature: temperature,
          });
        });

        $("#relay2-form").submit(function (event) {
          event.preventDefault();
          var seuil_demarrage = $("#seuil_demarrage_relay2").val();
          var seuil_arret = $("#seuil_arret_relay2").val();
          var temperature = $("#temperature_relay2").val();
          $.get("/setseuil?relay2", {
            seuil_demarrage: seuil_demarrage,
            seuil_arret: seuil_arret,
            temperature: temperature,
          });
        });
      });
    </script>
</%text>
</%block>
<%inherit file="index.tpl"/>
<%block name="page_title">Configuration MQTT</%block>
<%block name="topbar_content">
            <ul class="navbar-nav ml-auto">
              <!-- Nav Item - Search Dropdown (Visible Only XS) -->
              <li class="nav-item dropdown no-arrow d-sm-none">
                <i class="fas fa-search fa-fw"></i>

                <!-- Dropdown - Messages -->
                <div
                  class="dropdown-menu dropdown-menu-right p-3 shadow animated--grow-in"
                  aria-labelledby="searchDropdown"
                ></div>
              </li>
              <!-- Nav Item - Alerts -->
              <li class="nav-item dropdown no-arrow mx-1"></li>
              <div class="topbar-divider d-none d-sm-block"></div>
            </ul>
</%block>
<%block name="menu">
        <li class="nav-item active">
          <a class="nav-link" href="/">
            <i class="fas fa-fw fa-plug"></i>
            <span>Retour Index</span>
          </a>
        </li>
        <li class="nav-item active">
          <a class="nav-link" href="config.html">
            <i class="fas fa-fw fa-cog"></i>
            <span>Configuration</span>
          </a>
        </li>
        <li class="nav-item active">
          <a class="nav-link" href="readmqtt">
            <i class="fas fa-fw fa-download"></i>
            <span>Reprise conf par MQTT</span>
          </a>
        </li>
        <li class="nav-item active">
          <a class="nav-link" href="save">
            <i class="fas fa-fw fa-download"></i>
            <span>Enregistrer fichier conf</span>
          </a>
        </li>
        <li class="nav-item active">
          <a class="nav-link" href="/reboot">
            <i class="fas fa-fw fa-power-off"></i>
            <span>Reboot</span>
          </a>
        </li>
</%block>
<%block name="content">
            <!-- Page Heading -->
            <h1 class="h3 mb-2 text-gray-800">Configuration MQTT</h1>
            <div class="row">
              <div class="col-xl-3 col-md-6 mb-4">
                <div class="card border-left-primary shadow h-100 py-2">
                  <div class="card-body">
                    <div class="row no-gutters align-items-center">
                      <div class="col mr-2">
                        <div class="text-xs font-weight-bold text-primary text-uppercase mb-1">
                          Sauvegarder sur la flash
                        </div>
                        <div class="h5 mb-0 font-weight-bold text-gray-800">
                          <span id="save">
                            <a href="#">SAUVEGARDER</a>
                          </span>
                          <br />
                          <span id="savemsg"></span>
                        </div>
                      </div>
                      <div class="col-auto">
                        <i class="fas fa-download fa-2x text-gray-300"></i>
                      </div>
                    </div>
                  </div>
                </div>
              </div>
            </div>
            <div class="col-lg-6">
              <div class="card position-relative">
                <div class="card-header py-3">
                  <h6 class="m-0 font-weight-bold text-primary">Configuration</h6>
                  <span id="saveform"></span>
                </div>
                <form class="user" id="formulaire" method="post" action="">
                  <div class="card-body">
                    <div class="col-lg-12">
                      <div class="col-sm-3">
                        <div class="custom-control custom-checkbox small">
                          <input
                            type="checkbox"
                            class="custom-control-input"
                            id="MQTT"
                            onchange="sendservermode('MQTT');"
                          />
                          <label class="custom-control-label" for="MQTT">MQTT</label>
                        </div>
                      </div>
                      <div class="form-group">
                        Serveur :
                        <input
                          type="text"
                          class="form-control form-control-user"
                          id="server"
                          placeholder="IP ou nom d'hôte du serveur MQTT"
                        />
                      </div>
                      <div class="form-group">
                        Port :
                        <input
                          type="number"
                          step="1"
                          class="form-control form-control-user"
                          id="port"
                          placeholder="Port du serveur MQTT (port standard : 1883)"
                        />
                      </div>
                      <div class="form-group">
                        Topic Domoticz :
                        <input
                          type="text"
                          class="form-control form-control-user"
                          id="topic"
                          placeholder="Topic Domoticz"
                        />
                      </div>
                      <div class="form-group">
                        User :
                        <input
                          type="text"
                          class="form-control form-control-user"
                          id="user"
                          placeholder="Nom d'utilisateur de connexion au serveur MQTT"
                        />
                      </div>
                      <div class="form-group">
                        Password :
                        <input
                          type="password"
                          class="form-control form-control-user"
                          id="password"
                          placeholder="Mot de passe de connexion au serveur MQTT"
                        />
                      </div>
                    </div>
                    <br />
                    <div class="card position-relative">
                      <div class="card-header py-3">
                        <h6 class="m-0 font-weight-bold text-primary">Domoticz</h6>
                        <span id="saveform"></span>
                      </div>
                      <div class="card-body">
                        <div class="form-group row">
                          <div class="col-sm-3">IDX temp</div>
                          <div class="col-sm-3">IDX pow</div>
                          <div class="col-sm-3">IDX alarme</div>
                        </div>
                        <div class="form-group row">
                          <div class="col-sm-3">
                            <input
                              type="number"
                              step="1"
                              class="form-control form-control-user"
                              id="idxtemp"
                              placeholder="Index température"
                            />
                          </div>
                          <div class="col-sm-3">
                            <input
                              type="number"
                              step="1"
                              class="form-control form-control-user"
                              id="IDX"
                              placeholder="Index puissance"
                            />
                          </div>
                          <div class="col-sm-3">
                            <input
                              type="number"
                              step="1"
                              class="form-control form-control-user"
                              id="IDXAlarme"
                              placeholder="Index alarme"
                            />
                          </div>
                        </div>
                      </div>
                    </div>
                    <input
                      type="submit"
                      value=" Application des paramètres"
                      class="btn btn-primary btn-user btn-block"
                    />
                  </div>
                </form>
              </div>
            </div>
</%block>
<%block name="pagescript">
<%text>
    <script type="text/javascript">
      function save() {
        $.get("/getmqtt", { save: "yes" }).done(function (data) {});
      }

      function sendservermode(mode) {
        $.get("/get", { servermode: mode }).done(function (data) {
          $("#saveform").text("Configuration appliquée").show().fadeOut(5000);
          var config = data.split(";");
          document.getElementById("MQTT").value = config[18];
        });
      }

      <!-- sauvegarde de la configuration -->

      $("#save").click(function () {
        $.get("getmqtt", { save: "yes" }).done(function (data) {
          $("#savemsg").text("Configuration sauvegardée").show().fadeOut(5000);
        });
      });

      $("#formulaire").submit(function () {
        var data = {
          hostname: $("#server").val(),
          port: $("#port").val(),
          Publish: $("#topic").val(),
          mqttuser: $("#user").val(),
          mqttpassword: $("#password").val(),
          idxtemp: $("#idxtemp").val(),
          IDXAlarme: $("#IDXAlarme").val(),
          IDX: $("#IDX").val(),
        };

        $.ajax({
          type: "GET",
          data: data,
          url: "get",

          success: function (retour) {
            $("#saveform").text("Configuration appliquée").show().fadeOut(5000);
          },
        });
        return false;
      });

      var recupconfig = $.getJSON("/getmqtt", function (data) {
        for (var key in data) {
          if (data.hasOwnProperty(key)) {
            var element = document.getElementById(key);
            if (element) {
              element.value = data[key];
              if (element.type === "checkbox") {
                element.checked = data[key];
              }
            }
          }
        }
      });
    </script>
</%text>
</%block>

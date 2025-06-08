<%inherit file="index.tpl"/>
<%block name="page_title">Configuration</%block>

<%def name="generate_menu(page='config', support_mqtt=True)">

        <li class="nav-item">
          <a class="nav-link" href="./">
            <i class="fas fa-cog"></i>
            <span>Retour Index</span>
          </a>
        </li>
        <li class="nav-item${' active' if page == 'config' else ''}">
          <a class="nav-link" href="config.html">
            <i class="fas fa-book"></i>
            <span>Configuration</span>
          </a>
        </li>
% if support_mqtt:
        <li class="nav-item" id="menu_mqtt">
          <a class="nav-link" href="mqtt.html">
            <i class="fas fa-book"></i>
            <span>Configuration MQTT</span>
          </a>
        </li>
%endif
        <li class="nav-item${' active' if page == 'minuteur' else ''}">
          <a class="nav-link" href="minuteur.html">
            <i class="fas fa-moon"></i>
            <span>Minuteur d'appoint</span>
          </a>
        </li>
        <li class="nav-item">
          <a class="nav-link" href="log.html">
            <i class="fas fa-info"></i>
            <span>Console logs</span>
          </a>
        </li>
        <li class="nav-item${' active' if page == 'backup' else ''}">
          <a class="nav-link" href="backup.html">
            <i class="fas fa-download"></i>
            <span>Sauvegardes</span>
          </a>
        </li>
        <li class="nav-item">
          <a class="nav-link" href="update">
            <i class="fas fa-download"></i>
            <span>OTA</span>
          </a>
        </li>
        <li class="nav-item">
          <a class="nav-link" href="reboot">
            <i class="fas fa-power-off"></i>
            <span>Reboot</span>
          </a>
        </li>
</%def>
<%block name="menu">
${generate_menu()}
</%block>
<%block name="topbar_content">
            <div class="alert alert-danger" id="alertBox">
              <span class="mr-2 d-none d-lg-inline text-gray-600"></span>
              <p role="alert" id="alertContainer"></p>
            </div>
            <!-- Sidebar Toggle (Topbar) -->

            <!-- Topbar Navbar -->
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
              <li class="nav-item dropdown no-arrow mx-1"></li>
              <div class="topbar-divider d-none d-sm-block"></div>
            </ul>
</%block>
<%def name="generate_content(support_mqtt=True)">
            <!-- Page Heading -->
            <h1 class="h3 mb-2 text-gray-800">Configuration</h1>
            <p class="mb-4">
              Page de configuration de votre pv dimmer, pensez à sauvegarder votre configuration sur
              la flash après l'avoir appliquée.
            </p>
            <p class="mb-4">
              page rapide d'aide :
              <a href="https://wiki.apper-solaire.org/" target="_blank" rel="noopener"> ici </a>
            </p>

            <!-- Content Row -->
            <div class="row">
              <!-- Earnings (Monthly) Card Example -->
              <div class="col-xl-3 col-md-6 mb-4">
                <div class="card border-left-primary shadow h-100 py-2">
                  <div class="card-body">
                    <div class="row no-gutters align-items-center">
                      <div class="col mr-2">
                        <div class="text-xs font-weight-bold text-primary text-uppercase mb-1">
                          Puissance
                        </div>
                        <div class="h5 mb-0 font-weight-bold text-gray-800">
                          <span id="state">%STATE%</span>(&#37;) <span id="power">%POWER%</span>(W)
                        </div>
                      </div>
                      <div class="col-auto">
                        <i class="fas fa-calendar fa-2x "></i>
                      </div>
                    </div>
                  </div>
                </div>
              </div>
              <!-- Earnings (Monthly) Card Example -->
              <div class="col-xl-3 col-md-6 mb-4">
                <div class="card border-left-info shadow h-100 py-2">
                  <div class="card-body">
                    <div class="row no-gutters align-items-center">
                      <div class="col mr-2">
                        <div class="text-xs font-weight-bold text-success text-uppercase mb-1">
                          Température
                        </div>
                        <div class="h5 mb-0 font-weight-bold text-gray-800">
                          <span id="sigma">%SIGMA%</span>(°C)
                        </div>
                      </div>
                      <div class="col-auto">
                        <i class="fas fa-dollar-sign fa-2x "></i>
                      </div>
                    </div>
                  </div>
                </div>
              </div>
            </div>
            <!-- Content Row -->
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
                        <i class="fas fa-download_2 fa-2x"></i>
                      </div>
                    </div>
                  </div>
                </div>
              </div>
              <div class="col-xl-3 col-md-6 mb-4">
                <div class="card border-left-info shadow h-100 py-2">
                  <div class="card-body">
                    <div class="row no-gutters align-items-center">
                      <div class="col mr-2">
                        <div class="text-xs font-weight-bold text-success text-uppercase mb-1">
                          Mise en route du dimmer
                        </div>
                        <div class="h5 mb-0 font-weight-bold text-gray-800">
                          <span><a href="#" id="ONOFF">Reading State</a></span>
                        </div>
                      </div>
                      <div class="col-auto">
                        <i class="fas fa-dollar-sign fa-2x "></i>
                      </div>
                    </div>
                  </div>
                </div>
              </div>
            </div>
            <div class="col-lg-6">
              <form class="user" id="formulaire" method="post" action="">
                <div class="card position-relative">
                  <div class="card-header py-3">
                    <h6 class="m-0 font-weight-bold text-primary">Paramètres</h6>
                    <span id="saveform"></span>
                  </div>
                  <div class="card-body">
                    <div class="col-lg-12">
                      <div class="card position-relative">
                        <div class="card-header py-3">
                          <h6 class="m-0 font-weight-bold text-primary">Plage d'utilisation</h6>
                          <span id="saveform"></span>
                        </div>
                        <div class="card-body">
                          <div class="form-group row">
                            <div class="col-sm-3">Max Temp (°C)</div>
                            <div class="col-sm-3">Trigger (&percnt;)</div>
                            <div class="col-sm-3">Min Power (&percnt;)</div>
                            <div class="col-sm-3">Max Power (&percnt;)</div>
                          </div>
                          <div class="form-group row">
                            <div class="col-sm-3">
                              <input
                                type="number"
                                step="1"
                                class="form-control form-control-user"
                                id="maxtemp"
                                placeholder="%MAXTEMP%"
                              />
                            </div>
                            <div class="col-sm-3">
                              <input
                                type="number"
                                step="1"
                                class="form-control form-control-user"
                                id="trigger"
                                placeholder="%TRIGGER%"
                              />
                            </div>
                            <div class="col-sm-3">
                              <input
                                type="number"
                                step="1"
                                class="form-control form-control-user"
                                id="minpow"
                                placeholder="%MINPOW%"
                              />
                            </div>
                            <div class="col-sm-3">
                              <input
                                type="number"
                                step="1"
                                class="form-control form-control-user"
                                id="maxpow"
                                placeholder="%MAXPOW%"
                              />
                            </div>
                          </div>
                        </div>
                      </div>
                      <br />
                      <div class="card position-relative">
                        <div class="card-header py-3">
                          <h6 class="m-0 font-weight-bold text-primary">Charges</h6>
                          <span id="saveform"></span>
                        </div>
                        <div class="card-body">
                          <div class="form-group row">
                            <div
                              class="col-sm-4"
                              data-toggle="popover"
                              title="Aide"
                              data-content="Charge branchée sur la sortie Robotdyn ou SSR 1 (GND et PWM pour le SSR + pontage pins synchro)"
                              style="cursor: help"
                            >
                              Charge 1 (W) ( dimmer )
                              <i class="fas fa-info-circle"></i>
                            </div>
                            <div
                              class="col-sm-4"
                              data-toggle="popover"
                              title="Aide"
                              data-content="Charge branchée sur la sortie Jotta ou SSR2 (pontage pins synchro)"
                              style="cursor: help"
                            >
                              Charge 2 (W) ( Jotta )
                              <i class="fas fa-info-circle"></i>
                            </div>
                            <div
                              class="col-sm-4"
                              data-toggle="popover"
                              title="Aide"
                              data-content="Charge branchée sur la sortie Relay 2 ou SSR 3 (pontage pins synchro)"
                              style="cursor: help"
                            >
                              Charge 3 (W) ( relay2 )
                              <i class="fas fa-info-circle"></i>
                            </div>
                          </div>
                          <div class="form-group row">
                            <div class="col-sm-4">
                              <input
                                type="number"
                                step="1"
                                class="form-control form-control-user"
                                id="charge1"
                                placeholder="%CHARGE1%"
                              />
                            </div>
                            <div class="col-sm-4">
                              <input
                                type="number"
                                step="1"
                                class="form-control form-control-user"
                                id="charge2"
                                placeholder="%CHARGE2%"
                              />
                            </div>
                            <div class="col-sm-4">
                              <input
                                type="number"
                                step="1"
                                class="form-control form-control-user"
                                id="charge3"
                                placeholder="%CHARGE3%"
                              />
                            </div>
                          </div>
                        </div>
                      </div>
                      <br />

                      <div class="card position-relative">
                        <div class="card-header py-3">
                          <h6 class="m-0 font-weight-bold text-primary">Child</h6>
                          <span id="saveform"></span>
                        </div>
                        <div class="card-body">
                          <div class="form-group row">
                            <div class="col-sm-4">Child Dimmer IP</div>
                            <div class="col-sm-4">Child Dimmer Mode</div>
                            <div class="col-sm-4" id="dimmer-url">
                              <!-- L'url sera ajoutée ici -->
                            </div>
                          </div>
                          <div class="form-group row">
                            <div class="col-sm-6">
                              <input
                                type="text"
                                class="form-control form-control-user"
                                id="child"
                                placeholder="%CHILD%"
                              />
                            </div>
                            <div class="col-sm-4">
                              <select id="delester" class="form-control form-control-user">
                                <option value="off">off</option>
                                <option value="delester">délester</option>
                                <option value="equal">égal</option>
                              </select>
                            </div>
                          </div>
                        </div>
                      </div>
                      <br />
                      <div class="card position-relative">
                        <div class="card-header py-3">
                          <h6 class="m-0 font-weight-bold text-primary">Hostname</h6>
                          <span id="saveform"></span>
                        </div>
                        <div class="card-body">
                          <div class="form-group row">
                            <div class="col-sm-6">Dimmer Name</div>
                            <div class="col-sm-4" id="dimmer-mDNS">
                              <!-- Le nom mDNS sera rajouté ici-->
                            </div>
                          </div>

                          <div class="form-group row">
                            <div class="col-sm-6">
                              <input
                                type="text"
                                class="form-control form-control-user"
                                id="dimmername"
                                placeholder="%DIMMERNAME%"
                              />
                            </div>
                          </div>
                        </div>
                      </div>
                      <br />
% if support_mqtt:
                      <div class="card position-relative" id="menu_mqtt">
                        <div class="card-header py-3">
                          <h6 class="m-0 font-weight-bold text-primary">Pilote MQTT</h6>
                          <span id="saveform"></span>
                        </div>
                        <div class="card-body">
                          <div class="form-group row">
                            <div class="col-sm-6">
                              <a
                                href="https://wiki.apper-solaire.org/index.php?page=adv-dimmer"
                                target="_blank"
                                rel="noopener"
                                >MQTT state dimmer subscription (or : none)</a
                              >
                            </div>
                            <div class="col-sm-6">
                              <a
                                href="https://wiki.apper-solaire.org/index.php?page=adv-dimmer"
                                target="_blank"
                                rel="noopener"
                                >MQTT dimmer temp subscription (or : none)</a
                              >
                            </div>
                          </div>
                          <div class="form-group row">
                            <div class="col-sm-6">
                              <input
                                type="text"
                                class="form-control form-control-user"
                                id="SubscribePV"
                                placeholder="%SUBSCRIBEPV%"
                              />
                            </div>
                            <div class="col-sm-6">
                              <input
                                type="text"
                                class="form-control form-control-user"
                                id="SubscribeTEMP"
                                placeholder="%SUBSCRIBETEMP%"
                              />
                            </div>
                          </div>
                          <div class="form-group row">
                            <div class="col-sm-6">Puissance de démarrage</div>
                            <div class="col-sm-6">Etat au démarrage</div>
                          </div>
                          <div class="form-group row">
                            <div class="col-sm-6">
                              <input
                                type="number"
                                step="1"
                                class="form-control form-control-user"
                                id="startingpow"
                                placeholder="%STARTINGPOW%"
                              />
                            </div>
                            <div class="col-sm-3">
                              <select id="dimmer_on_off" class="form-control form-control-user">
                                <option value="1">on</option>
                                <option value="0">off</option>
                              </select>
                            </div>
                          </div>
                        </div>
                      </div>
%endif
                      <br />
                      <div class="card position-relative" id="DALLAS-LOCAL">
                        <div class="card-header py-3">
                          <h6 class="m-0 font-weight-bold text-primary">Dallas Local</h6>
                          <span id="saveform"></span>
                        </div>
                        <div class="card-body">
                          <div class="form-group row">
                            <div class="col-sm-6">
                              Adresse sonde DALLAS maitre
                              <br /><br />
                              <b>Sonde présentes: </b>
                              <span id="dallas"></span>
                            </div>
                            <div class="col-sm-6">
                              <input
                                type="text"
                                class="form-control form-control-user"
                                id="DALLAS"
                                placeholder="%DALLAS%"
                              />
                            </div>
                          </div>
                        </div>
                      </div>
                    </div>
                    <!--  fin du formumaire et envoie -->
                    <input
                      type="submit"
                      value=" Application des paramètres"
                      class="btn btn-primary btn-user btn-block"
                    />
                    <hr />
                  </div>
                  <hr />
                </div>
              </form>
            </div>
</%def>
<%block name="content">
${generate_content()}
</%block>
<%block name="pagescript">
<%text>
    <script type="text/javascript">
      $('[data-toggle="popover"]').popover();

      //<!-- rafraichissement valeurs -->
      function refreshvalue() {
        $.getJSON("/state", function (data) {
          // Récupérer les données du JSON
          var dimmer = data.dimmer;
          var temperature = data.temperature;
          var power = data.power;
          var onoff = data.onoff;
          document.getElementById("state").innerHTML = dimmer;
          document.getElementById("sigma").innerHTML = temperature;
          document.getElementById("power").innerHTML = power;
          onoff = onoff ? "ON" : "OFF";
          $("#ONOFF").text(onoff);

          if (data.alerte && data.alerte.trim() != "") {
            const alertContainer = document.getElementById("alertContainer");
            alertContainer.textContent = "Alerte : " + data.alerte;
            $("#alertBox").fadeIn();
          } else {
            $("#alertBox").fadeOut();
          }

                   // affichage des dallas et les Température ( boucle sur les dallas )
          var dallasData = {}; // Objet pour stocker les données des capteurs Dallas
          var dallasNumber;
          // Extraction des données des capteurs Dallas du JSON
          for (var key in data) {
            if (key.startsWith("dallas")) {
              dallasNumber = key.substring(6); // Récupérer le numéro du capteur Dallas
              var dallasTemperature = data[key];
              var dallasAddressKey = "addr" + dallasNumber;
              var dallasAddress = data[dallasAddressKey];
              dallasData[dallasNumber] = {
                temperature: dallasTemperature,
                address: dallasAddress,
              };
            }
          }
          // Affichage des données des capteurs Dallas dans la page HTML
          var dallasHtml = "";
          for (dallasNumber in dallasData) {
            dallasHtml +=
              "<p>Dallas sensor " +
              dallasNumber +
              ": " +
              dallasData[dallasNumber].temperature +
              "°C <br>Address: " +
              dallasData[dallasNumber].address +
              "</p>";
          }
          document.getElementById("dallas").innerHTML = dallasHtml;

        });
      }
      // Lancer après 500ms pour laisser l'ESP8266 charger
        setTimeout(refreshvalue, 500); 
      // Puis, continuer toutes les 5 secondes
        setInterval(refreshvalue, 5000); // Rafraîchir les données toutes les 5 secondes

      function sendmode(mode) {
        $.get("/get", { send: mode }).done(function (data) {
          document.getElementById("sendmode").innerHTML = "Request sent";
          document.getElementById("sendmode2").innerHTML = "Request sent";
        });
      }

      function save() {
        $.get("/get", { save: "yes" }).done(function (data) {});
      }

      // Sauvegarde de la configuration
      $("#save").click(function () {
        $.get("/get", { save: "yes" }).done(function (data) {
          $("#savemsg").text("Configuration sauvegardée").show().fadeOut(5000);
        });
      });

      $("#ONOFF").click(function () {
        $.get("/onoff").done(function (data) {
          // si la data est 1 alors on affiche ON sinon OFF
          if (data == 1) {
            data = "ON";
          } else {
            data = "OFF";
          }
          $("#ONOFF").text(data);
        });
      });

      $("#formulaire").submit(function () {
        var data = {
          hostname: $("#hostname").val(),
          port: $("#port").val(),
          Publish: $("#Publish").val(),
          maxtemp: $("#maxtemp").val(),
          startingpow: $("#startingpow").val(),
          minpow: $("#minpow").val(),
          maxpow: $("#maxpow").val(),
          child: $("#child").val(),
          SubscribePV: $("#SubscribePV").val(),
          SubscribeTEMP: $("#SubscribeTEMP").val(),
          mode: $("#delester").val(),
          charge1: $("#charge1").val(),
          charge2: $("#charge2").val(),
          charge3: $("#charge3").val(),
          DALLAS: $("#DALLAS").val(),
          dimmername: $("#dimmername").val(),
          trigger: $("#trigger").val(),
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

      $(document).ready(function () {
        function generateLink(dimmerValue) {
          if (dimmerValue !== "none") {
            var dimmerURL = "http://" + dimmerValue;
            $("#dimmer-url").html(
              `<a href="${dimmerURL}" target="_blank" rel="noopener">${dimmerURL}</a>`,
            );
          }
        }

        function generateLinkName(dimmerName) {
          var formattedName = dimmerName.replace(/ /g, "-") + ".local"; // Replace spaces with hyphens and add .local
          $("#dimmer-mDNS").html(formattedName); // Update the HTML content
        }

        var recupconfig = $.getJSON("/config", function (data) {
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

          // Une fois la requête terminée
          recupconfig.done(function () {
            // Générer le lien avec la valeur initiale
            var dimmerValue = $("#child").val();
            generateLink(dimmerValue);
            var dimmerValue = $("#dimmername").val();
            generateLinkName(dimmerValue);
          });

          $("#child").on("input", function () {
            var dimmerValue = $(this).val();
            generateLink(dimmerValue);
          });

          $("#dimmer-mDNS").on("input", function () {
            var dimmerValue = $(this).val();
            generateLinkName(dimmerValue);
          });
        });
      });
    </script>
</%text>
</%block>

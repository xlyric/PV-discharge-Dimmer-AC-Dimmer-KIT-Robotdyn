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
                              <img
                                height="40"
                                alt="dimmer"
                                src="data:image/png;base64,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"
                              />
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
                              <img
                                height="40"
                                alt="Jotta"
                                src="data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAJwAAABpCAYAAADP50rnAAAAAXNSR0IArs4c6QAAAARnQU1BAACxjwv8YQUAAAAJcEhZcwAADsMAAA7DAcdvqGQAAAVHSURBVHhe7Zy9btRAEMcTRGiIhCh4AhpSUYDyCPRQ0FISJETPE9AjJGhpKaDnESIoqFJBixBICAkaUkDG8SiTvf3+mNtb/3/S6nxee87n/WXW3j1n++KNg39bHo6PXs5LfbCz92heGo/d/bvT6+/Dd9PriFyYXwFQ4STDPTzJcNvz21U4w42cWXphQRnO26sCUA3RpZJ0EA+0xXINB/FAOzw3DRAP1Ad3qUCVBOGQ7UA5iRkO3SwoI7NLhXggj8JrOIgH0phnGtwcH72alwAoB3epQJVghivDPUcLVsGvRYrBNR44j1KXCvHAKcrXcBBv6azppoHFg3xLo4O7VIi3JDoQjoF4S6Aj4RgWD/KNSIfCSSDeaDQe+K1N2kCy7RFH+TCQ6wEhud73mGTsg0WuGOb+cuDXdWyE75gI3z6xx9yKzjOcCWe88N+Iq1FCjZVCaSyNYyRqxythw4STuOWTJ5j+orkwZgPENIiMI2OlYNs/9Nm2elsc2zobMd+1JRssnMQunnny+b2tUbQbIiTGqAwiHGMXT1KroTlObVF/vr4zvdY6Tj6+XgQfTDiTsIBEK3lKqX1cPXzPwYU7hU6wWVz46jQJHWcMcv8a8WqwCOF29g7mJcn5zJfa5XDj1eyquDu10YMsNRhaOPnzeLt0zKl8NeUJ4ROIjkOWUmzx1iXwkMJJuUg6Low747mv96iBZMnBtn8rATgex++FYTOcK6P5M51Z77/hKG1M2/5XH7yfl85Yd1aqyYZNbfVAu+c08EwDsMBdLxeQAoQrBgKmAOGqAwF9QLjmQEAJhFPHFHBZEkK4LjDFG1fCoYdFUv8RT2iMjkiNmYP7OM4PyaSMy/UyAIwMl0iMlO3gTMhl81hEhluvJPHs7t+bXn8fvp1ea3B2DpDhgCpmdjSLDpja6ogWGa4deVN8yHAgEzNDxhUIB1SBcEAVCAdUgXBAFQgHVIFwQJWhhaNRdo25z57p7RwgwwFVMJeawJtnl+alM+4//TsvlUGxb1/+Pi1/+HOtWtza56AUCBeBlMFGriCt4kp6Ew5daoAvz395pSCo3pb9fIRkI6iePn8kIJyHlMZOkS5GNslI0kE4BzZ5dm6+WCmSWInM7UJxidQM2ivVhKNrBb5eiMW3T068mtiksGGuD4lh1sfGTcmIPYMM54B/UEO4pLBRU4xUmTcBCGdBNmzMLXyukCn7jUJT4bhbtHWP8r1Z76tjeL0soH+aCWcToIYUvcolu+BYjj89npeWQxPhpBA04CgHHblOrjO38dUxvN5WV4o52Po3IIasD0lHg7m51Jp9WCfZwsVkGSlDCzFa8jFSDFPG0H6mNL4sN2IGzBJuXV2aLTu2wpbluMj3EpItJgtRluMumArFkXLRsilbSWbsiaIuVVs8Fk3rc13ZyhSNie3yaDszNotHRcpI1JhT7YXsyXtXo4ekMLOT3E7WmfvHyOaKba5P5XNgaik2s5nQ8MutwLhdbmym1jmoRdWbhtIv5ZJPso4Td/3JlanhzaxE76kuVwjaj2NL+LNKYvcKfp40OENnOABCQDigCoQDqkA4oAqEA6pAOKAKhAOqQDigCoQDqkC4hUMzEWbxEbONDwgHVnBJVSIaA+HABM21cmGkYDVkIyAcWKHlRD+EA1GY2S8XCAessFy1ulIGwgFVIBywwpmt9vUchAOqQDiwQu3rNgmEAxMkGRemxfDIIh6iAW55XOcotH2ujBBuIbTIVjmgSwWqDJ3hQH8gwwFVtr9++4EMB9RAhgOqQDigCoQDqkA4oAqEA6pAOKAKhAOKbG39B/7N2dFj5M6PAAAAAElFTkSuQmCC"
                              />
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
                              <img
                                height="40"
                                alt="relay2"
                                src="data:image/png;base64,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"
                              />
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
            alertContainer.innerHTML = "Alerte : " + data.alerte;
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

      <!--- sauvegarde de la configuration -->
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

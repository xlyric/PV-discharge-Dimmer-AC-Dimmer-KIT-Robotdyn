<%inherit file="index.tpl"/>
<%block name="topbar_content">
            <ul class="navbar-nav ml-auto">
              <div class="topbar-divider d-none d-sm-block"></div>
            </ul>
</%block>
<%block name="content">
            <!-- Page Heading -->
            <div class="d-sm-flex align-items-center justify-content-between mb-4">
              <h1 class="h3 mb-0 text-gray-800">Dashboard-AP</h1>
            </div>

            <!-- Content Row -->

            <div class="row">
              <div class="col-xl-3 col-md-6 mb-4">
                <div class="card border-left-primary shadow h-100 py-2">
                  <div class="card-body">
                    <div class="row no-gutters align-items-center">
                      <div class="col mr-2">
                        <div class="text-xs font-weight-bold text-primary text-uppercase mb-1">
                          Puissance
                        </div>
                        <div class="h5 mb-0 font-weight-bold text-gray-800">
                          <span id="state">%STATE%</span>
                        </div>
                      </div>
                      <div class="col-auto">
                        <i class="fas fa-calendar fa-2x text-gray-300"></i>
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
                          <span id="sigma">%SIGMA%</span>
                        </div>
                      </div>
                      <div class="col-auto">
                        <i class="fas fa-dollar-sign fa-2x text-gray-300"></i>
                      </div>
                    </div>
                  </div>
                </div>
              </div>
            </div>
</%block>
<%block name="pagescript">
<%text>
    <script type="text/javascript">
      setInterval(function () {
        $.getJSON("/state", function (data) {
          // Récupérer les données du JSON

          document.getElementById("state").innerHTML = data.power;
          document.getElementById("sigma").innerHTML = data.temperature;
        });
      }, 2000);
    </script>
</%text>
</%block>
#ifndef HA_FUNCTIONS
#define HA_FUNCTIONS

  #include <PubSubClient.h>

extern PubSubClient client;
extern Config config; // NOSONAR
extern Mqtt mqtt_config; // NOSONAR
extern System sysvar;
extern DeviceAddress addr[MAX_DALLAS];  // array of (up to) 15 temperature sensors
extern String devAddrNames[MAX_DALLAS];  // array of (up to) 15 temperature sensors
extern int deviceCount;  // nombre de sonde(s) dallas détectée(s)

String stringBool(bool mybool);

/// @brief déclaration des configurations HA et MQTT
struct HA
{
private: int MQTT_INTERVAL = 60;
  /* MQTT */
private: String name="none";
public: void Set_name(String setter) {
    name=setter;
  }

private: String object_id;
public: void Set_object_id(String setter) {
    object_id=setter;
  }

private: String dev_cla;
public: void Set_dev_cla(String setter) {
    dev_cla=setter;
  }

private: String unit_of_meas;
public: void Set_unit_of_meas(String setter) {
    unit_of_meas=setter;
  }

private: String stat_cla;
public: void Set_stat_cla(String setter) {
    stat_cla=setter;
  }

private: String entity_category;
public: void Set_entity_category(String setter) {
    entity_category=setter;
  }

private: String entity_type="sensor";
public: void Set_entity_type(String setter) {
    entity_type=setter;
  }

private: String icon;
public: void Set_icon(String setter) {
    icon=R"("ic": ")" + setter + R"(",)";
  }
  // {icon="\"ic\": \""+ setter +"\", "; }

private: String min;
public: void Set_entity_valuemin(String setter) {
    min=setter;
  }

private: String max;
public: void Set_entity_valuemax(String setter) {
    max=setter;
  }

private: String step;
public: void Set_entity_valuestep(String setter) {
    step=setter;
  }

private: String entity_option;
public: void Set_entity_option(String setter) {
    entity_option=setter;
  }

private: bool retain_flag;
public: void Set_retain_flag(bool setter) {
    retain_flag=setter;
  }

private: int qos;
public: void Set_entity_qos(int setter) {
    qos=setter;
  }

private: String retain;
public: void Set_retain(bool setter) {
    if (setter) {retain="\"ret\":true,"; }
  }

private: String expire_after;
public: void Set_expire_after(bool setter) {
    if (setter) {expire_after=R"("exp_aft": ")" + String(MQTT_INTERVAL) + R"(", )"; }
  }

private: String HA_sensor_type() {
    String topic = "homeassistant/"+ entity_type +"/"+ node_id +"/";
    String topic_Xlyric = "Xlyric/"+ node_id +"/";
    String info;
    if (entity_type == "sensor") {
      info = "\"dev_cla\": \""+dev_cla+"\","
             "\"unit_of_meas\": \""+unit_of_meas+"\","
             "\"stat_cla\": \""+stat_cla+"\","
             "\"value_template\": \"{{ value_json."+ object_id +" }}\",";
    }
    else if (entity_type == "switch") {
      info = R"(
          "value_template": "{{ value_json.)" + object_id + R"( }}",
          "payload_on": "{ \")" + object_id + R"(\" : 1 }",
          "payload_off": "{ \")" + object_id + R"(\" : 0 }",
          "stat_on": "1",
          "stat_off": "0",
          "qos": 1,
          "command_topic": ")" + topic_Xlyric + R"(command/)" + entity_type + R"(/)" + object_id + R"(",
          )";
    }
    else if (entity_type == "number") {
      info = "\"val_tpl\": \"{{ value_json."+ object_id +" }}\","
             "\"cmd_t\": \""+ topic_Xlyric + "command/" +  entity_type + "/" + object_id + "\","
             "\"cmd_tpl\": \"{ \\\""+object_id+"\\\" : {{ value }} } \","
             "\"entity_category\": \""+ entity_category + "\","
             "\"max\": \""+max+"\","
             "\"min\": \""+min+"\","
             "\"step\": \""+step+"\",";
    }
    else if (entity_type == "select") {
      info = "\"val_tpl\": \"{{ value_json."+ object_id +" }}\","
             "\"cmd_t\": \""+ topic_Xlyric + "command/" +  entity_type + "/" + object_id + "\","
             "\"cmd_tpl\": \"{ \\\""+object_id+"\\\" : \\\"{{ value }}\\\" } \","
             "\"entity_category\": \""+ entity_category + "\","
             "\"options\": ["+ entity_option + "],";
    }
    else if (entity_type == "binary_sensor") {
      info = "\"dev_cla\": \""+dev_cla+"\","
             "\"pl_on\":\"true\","
             "\"pl_off\":\"false\","
             "\"val_tpl\": \"{{ value_json."+ object_id +" }}\",";
    }
    else if (entity_type == "button") {
      info = "\"entity_category\": \""+ entity_category + "\","
             "\"cmd_t\": \""+ topic_Xlyric + "command/" +  entity_type + "/" + object_id + "\","
             "\"pl_prs\": \"{ \\\""+object_id+"\\\" : \\\"1\\\"  } \",";
    }
    return info;
  }

// setter mod_mac
public: void Set_node_mac(String setter) {
    node_mac=setter;
  }

private: String IPaddress = WiFi.localIP().toString();
         String node_mac = WiFi.macAddress().substring(12,14)+ WiFi.macAddress().substring(15,17);


         String node_id = String("dimmer-") + node_mac;
         String topic_switch = "homeassistant/switch/"+ node_id +"/";
         String topic_switch_state = "homeassistant/switch/";
         String HA_device_declare() {
         String IPaddress = WiFi.localIP().toString();
    String info = R"(
            "dev": {
                "ids": ")" + node_id + R"(",
                "name": ")" + node_id + R"(",
                "sw": "Dimmer )" + String(VERSION) + R"(",
                "mdl": "ESP8266 )" + IPaddress +
            R"(",
                "mf": "Cyril Poissonnier",
                "cu": "http://)" + IPaddress + R"("
            }
              )";
    return info;
  }


public: void HA_discovery(){

    // protection contre les variables non définies
    if (name == "none") {return; }

    String topic = "homeassistant/"+ entity_type +"/"+ node_id +"/";
    String topic_Xlyric = "Xlyric/"+ node_id +"/";

    String device = R"(
      {
        "name": ")" + name + R"(",
        "obj_id": "dimmer-)" + object_id + "-" + node_mac + R"(",
        "uniq_id": ")" + node_mac + "-" + object_id + R"(",
        "stat_t": ")" + topic_Xlyric + "sensors/" + object_id + "/state" + R"(",
        "avty_t": ")" + topic_Xlyric + "status\","
                    + HA_sensor_type()
                    + icon
                    + retain
                    + expire_after
                    + HA_device_declare() +
                    "}";

    if (object_id.length() != 0) {
      client.publish(String(topic+object_id+"/config").c_str(), device.c_str(),true);  // déclaration autoconf dimmer
    }
    else {
      client.publish(String(topic+"config").c_str(), device.c_str(),true);   // déclaration autoconf dimmer
    }

  }

public: void send(String value){
    if (config.JEEDOM || config.HA) {
      String topic = "Xlyric/"+ node_id +"/sensors/";
      String message = "  { \""+object_id+"\" : \"" + value.c_str() + "\"  } ";
      client.publish(String(topic + object_id + "/state").c_str(), message.c_str(), retain_flag);
    }
  }
};

/// création des sensors
HA device_dimmer;
HA device_temp[MAX_DALLAS];  // NOSONAR
HA device_temp_master;

/// création des switchs
HA device_relay1;
HA device_relay2;
HA device_dimmer_on_off;

/// création des button
HA device_dimmer_save;

/// création number
HA device_dimmer_starting_pow;
HA device_dimmer_maxtemp;
HA device_dimmer_minpow;
HA device_dimmer_maxpow;
HA device_dimmer_send_power;

/// création select
HA device_dimmer_child_mode;

/// création binary_sensor
HA device_dimmer_alarm_temp;
HA device_cooler;
HA device_dimmer_alarm_temp_clear;

// creation remonté de puissance
HA device_dimmer_power;
HA device_dimmer_total_power;

// création du boost
HA device_dimmer_boost;

void devices_init(){
  /// création des sensors
  device_dimmer.Set_name("Puissance");
  device_dimmer.Set_object_id("power");
  device_dimmer.Set_unit_of_meas("%");
  device_dimmer.Set_stat_cla("measurement");
  // fix is using native unit of measurement '%' which is not a valid unit for the device class ('power') it is using
  device_dimmer.Set_dev_cla("power_factor");
  device_dimmer.Set_icon("mdi:percent");
  device_dimmer.Set_entity_type("sensor");
  device_dimmer.Set_retain_flag(true);

  device_dimmer_power.Set_name("Watt");
  device_dimmer_power.Set_object_id("watt");
  device_dimmer_power.Set_unit_of_meas("W");
  device_dimmer_power.Set_stat_cla("measurement");
  // fix is using native unit of measurement '%' which is not a valid unit for the device class ('power') it is using
  device_dimmer_power.Set_dev_cla("power");
  device_dimmer_power.Set_icon("mdi:home-lightning-bolt-outline");
  device_dimmer_power.Set_entity_type("sensor");
  device_dimmer_power.Set_retain_flag(true);

  device_dimmer_total_power.Set_name("Watt total");
  device_dimmer_total_power.Set_object_id("watt_total");
  device_dimmer_total_power.Set_unit_of_meas("W");
  device_dimmer_total_power.Set_stat_cla("measurement");
  // fix is using native unit of measurement '%' which is not a valid unit for the device class ('power') it is using
  device_dimmer_total_power.Set_dev_cla("power");
  device_dimmer_total_power.Set_icon("mdi:home-lightning-bolt-outline");
  device_dimmer_total_power.Set_entity_type("sensor");
  device_dimmer_total_power.Set_retain_flag(true);

  for (int i = 0; i < deviceCount; i++) {
    device_temp[i].Set_name("Température" + String(i+1) );
    device_temp[i].Set_object_id("temperature_"+ devAddrNames[i]);
    device_temp[i].Set_unit_of_meas("°C");
    device_temp[i].Set_stat_cla("measurement");
    device_temp[i].Set_dev_cla("temperature");
    device_temp[i].Set_entity_type("sensor");
    device_temp[i].Set_entity_qos(1);
    device_temp[i].Set_retain_flag(true);
  }
  /// temp master
  device_temp_master.Set_name("Température master");
  device_temp_master.Set_object_id("temperature");
  device_temp_master.Set_unit_of_meas("°C");
  device_temp_master.Set_stat_cla("measurement");
  device_temp_master.Set_dev_cla("temperature");
  device_temp_master.Set_entity_type("sensor");
  device_temp_master.Set_entity_qos(1);
  device_temp_master.Set_retain_flag(true);

  /// création des switch
  device_relay1.Set_name("Relais 1");
  device_relay1.Set_object_id("relay1");
  device_relay1.Set_entity_type("switch");
  device_relay1.Set_retain_flag(true);

  device_relay2.Set_name("Relais 2");
  device_relay2.Set_object_id("relay2");
  device_relay2.Set_entity_type("switch");
  device_relay2.Set_retain_flag(true);

  device_dimmer_on_off.Set_name("Dimmer");
  device_dimmer_on_off.Set_object_id("on_off");
  device_dimmer_on_off.Set_entity_type("switch");
  device_dimmer_on_off.Set_retain_flag(true);

  device_dimmer_boost.Set_name("Boost");
  device_dimmer_boost.Set_object_id("boost");
  device_dimmer_boost.Set_entity_type("switch");
  device_dimmer_boost.Set_retain_flag(true);

  /// création des button
  device_dimmer_save.Set_name("Sauvegarder");
  device_dimmer_save.Set_object_id("save");
  device_dimmer_save.Set_entity_type("button");
  device_dimmer_save.Set_entity_category("config");
  device_dimmer_save.Set_retain_flag(false);

  /// création des number
  device_dimmer_starting_pow.Set_name("Puissance de démarrage");
  device_dimmer_starting_pow.Set_object_id("starting_power");
  device_dimmer_starting_pow.Set_entity_type("number");
  device_dimmer_starting_pow.Set_entity_category("config");
  device_dimmer_starting_pow.Set_entity_valuemin("-100");
  device_dimmer_starting_pow.Set_entity_valuemax("500");
  device_dimmer_starting_pow.Set_entity_valuestep("1");
  device_dimmer_starting_pow.Set_retain_flag(true);

  device_dimmer_minpow.Set_name("Puissance mini");
  device_dimmer_minpow.Set_object_id("minpow");
  device_dimmer_minpow.Set_entity_type("number");
  device_dimmer_minpow.Set_entity_category("config");
  device_dimmer_minpow.Set_entity_valuemin("0");
  device_dimmer_minpow.Set_entity_valuemax("100");
  device_dimmer_minpow.Set_entity_valuestep("1");
  device_dimmer_minpow.Set_retain_flag(true);

  device_dimmer_maxpow.Set_name("Puissance maxi");
  device_dimmer_maxpow.Set_object_id("maxpow");
  device_dimmer_maxpow.Set_entity_type("number");
  device_dimmer_maxpow.Set_entity_category("config");
  device_dimmer_maxpow.Set_entity_valuemin("0");
  device_dimmer_maxpow.Set_entity_valuemax("100");
  device_dimmer_maxpow.Set_entity_valuestep("1");
  device_dimmer_maxpow.Set_retain_flag(true);

  device_dimmer_send_power.Set_name("dimmer power");
  device_dimmer_send_power.Set_object_id("powdimmer");
  device_dimmer_send_power.Set_entity_type("number");
  device_dimmer_send_power.Set_entity_category("config");
  device_dimmer_send_power.Set_entity_valuemin("0");
  device_dimmer_send_power.Set_entity_valuemax("100");
  device_dimmer_send_power.Set_entity_valuestep("1");
  device_dimmer_send_power.Set_retain_flag(true);

  device_dimmer_maxtemp.Set_name("Température maxi");
  device_dimmer_maxtemp.Set_object_id("maxtemp");
  device_dimmer_maxtemp.Set_entity_type("number");
  device_dimmer_maxtemp.Set_entity_category("config");
  device_dimmer_maxtemp.Set_entity_valuemin("0");
  device_dimmer_maxtemp.Set_entity_valuemax("75");
  device_dimmer_maxtemp.Set_entity_valuestep("1");
  device_dimmer_maxtemp.Set_retain_flag(true);

  /// création des select
  device_dimmer_child_mode.Set_name("Mode");
  device_dimmer_child_mode.Set_object_id("child_mode");
  device_dimmer_child_mode.Set_entity_type("select");
  device_dimmer_child_mode.Set_entity_category("config");
  device_dimmer_child_mode.Set_entity_option(R"( "off","delester","equal" )");
  device_dimmer_child_mode.Set_retain_flag(true);

  // création des binary_sensor
  device_dimmer_alarm_temp.Set_name("Surchauffe");
  device_dimmer_alarm_temp.Set_object_id("alarm_temp");
  device_dimmer_alarm_temp.Set_entity_type("binary_sensor");
  device_dimmer_alarm_temp.Set_entity_category("diagnostic");
  device_dimmer_alarm_temp.Set_dev_cla("problem");
  device_dimmer_alarm_temp.Set_retain_flag(true);

  device_cooler.Set_name("Ventilateur");
  device_cooler.Set_object_id("cooler");
  device_cooler.Set_entity_type("binary_sensor");
  device_cooler.Set_entity_category("diagnostic");
  device_cooler.Set_dev_cla("running");
  device_cooler.Set_retain_flag(true);

  device_dimmer_alarm_temp_clear.Set_name("Reset alarme");
  device_dimmer_alarm_temp_clear.Set_object_id("reset_alarm");
  device_dimmer_alarm_temp_clear.Set_entity_type("button");
  device_dimmer_alarm_temp_clear.Set_entity_category("config");
  device_dimmer_alarm_temp_clear.Set_entity_qos(0);
  device_dimmer_alarm_temp_clear.Set_retain_flag(false);
}

void HA_discover(){
  if (config.HA) {
    Serial.println("HA discovery" );
    /// création des binary_sensor et enregistrement sous HA
    device_dimmer_on_off.HA_discovery();
    device_dimmer_on_off.send(String(config.dimmer_on_off));

    device_dimmer.HA_discovery();
    device_dimmer.send(String(sysvar.puissance));

    device_dimmer_power.HA_discovery();
    device_dimmer_power.send(String(sysvar.puissance* config.charge/100));

    device_dimmer_total_power.HA_discovery();
    device_dimmer_total_power.send(String(sysvar.puissance_cumul + (sysvar.puissance * config.charge/100)));

    device_cooler.HA_discovery();
    device_cooler.send(stringBool(false));

    device_temp_master.HA_discovery();  // discovery fait à la 1ere réception sonde ou mqtt.
    device_temp_master.send(String(0));

    #ifdef RELAY1
    device_relay1.HA_discovery();
    device_relay1.send(String(0));
    #endif
    #ifdef RELAY2
    device_relay2.HA_discovery();
    device_relay2.send(String(0));
    #endif

    // boost
    device_dimmer_boost.HA_discovery();
    device_dimmer_boost.send("0");

    device_dimmer_starting_pow.HA_discovery();
    device_dimmer_starting_pow.send(String(config.startingpow));

    device_dimmer_minpow.HA_discovery();
    device_dimmer_minpow.send(String(config.minpow));

    device_dimmer_maxpow.HA_discovery();
    device_dimmer_maxpow.send(String(config.maxpow));

    device_dimmer_send_power.HA_discovery();
    device_dimmer_send_power.send(String(sysvar.puissance));

    device_dimmer_child_mode.HA_discovery();
    device_dimmer_child_mode.send(String(config.mode));

    device_dimmer_save.HA_discovery();
  }
}
#endif
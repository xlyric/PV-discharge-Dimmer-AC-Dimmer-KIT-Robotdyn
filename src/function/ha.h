#ifndef HA_FUNCTIONS
#define HA_FUNCTIONS

//#include <PubSubClient.h>
#include <AsyncMqttClient.h>

extern AsyncMqttClient  client;
extern Config config;
extern Mqtt mqtt_config;
extern System sysvar;

//extern dimmerLamp dimmer;
String stringbool(bool mybool);

struct HA
{
    private:int MQTT_INTERVAL = 60;
      /* HA */
    private:String name; 
    public:void Set_name(String setter) {name=setter; }

    private:String object_id; 
    public:void Set_object_id(String setter) {object_id=setter; }

    private:String dev_cla; 
    public:void Set_dev_cla(String setter) {dev_cla=setter; }

    private:String unit_of_meas; 
    public:void Set_unit_of_meas(String setter) {unit_of_meas=setter; }

    private:String stat_cla; 
    public:void Set_stat_cla(String setter) {stat_cla=setter; }

    private:String entity_category; 
    public:void Set_entity_category(String setter) {entity_category=setter; }
    
    private:String entity_type; 
    public:void Set_entity_type(String setter) {entity_type=setter; }

    private:String icon; 
    public:void Set_icon(String setter) {icon="\"ic\": \""+ setter +"\", "; }

    private:String min; 
    public:void Set_entity_valuemin(String setter) {min=setter; }

    private:String max; 
    public:void Set_entity_valuemax(String setter) {max=setter; }

    private:String step; 
    public:void Set_entity_valuestep(String setter) {step=setter; }

    private:String entity_option; 
    public:void Set_entity_option(String setter) {entity_option=setter; }

    private:bool retain_flag; 
    public:void Set_retain_flag(bool setter) {retain_flag=setter; }

    private:String expire_after; 
    public:void Set_expire_after(bool setter) {
      if (setter) {expire_after="\"exp_aft\": \""+ String(MQTT_INTERVAL) +"\", "; }
    }

    private:String sensor_type() {
      String topic = "homeassistant/"+ entity_type +"/"+ node_id +"/";
      String info;
      if (entity_type == "sensor") {
              info =         "\"dev_cla\": \""+dev_cla+"\","
            "\"unit_of_meas\": \""+unit_of_meas+"\","
            "\"stat_cla\": \""+stat_cla+"\"," 
           // "\"avty_t\": \""+ topic + "status\","
            "\"value_template\": \"{{ value_json."+ object_id +" }}\","; 
      }
      else if (entity_type == "switch") { 
              info =         "\"val_tpl\": \"{{ value_json."+ object_id +" }}\","
            "\"pl\":  \"{{ value_json."+ object_id +" }}\","
            "\"pl_on\": \"{ \\\""+object_id+"\\\" : \\\"1\\\"  } \","
            "\"pl_off\": \"{ \\\""+object_id+"\\\" : \\\"0\\\"  } \","
            "\"stat_on\":1,"
            "\"stat_off\":0,"
            "\"cmd_t\": \""+ topic + "command\"," ;
           // "\"avty_t\": \"homeassistant/sensor/"+ node_id +"/" + "status\",";
      } 
      else if (entity_type == "number") { 
              info =         "\"val_tpl\": \"{{ value_json."+ object_id +" }}\"," //
            "\"cmd_t\": \""+ topic + "command\"," //
            "\"cmd_tpl\": \"{ \\\""+object_id+"\\\" : {{ value }} } \"," 
            "\"entity_category\": \""+ entity_category + "\"," //
            "\"max\": \""+max+"\"," //
            "\"min\": \""+min+"\"," //
            "\"step\": \""+step+"\"," ;//
          //  "\"avty_t\": \"homeassistant/sensor/"+ node_id +"/" + "status\","; //
      } 
      else if (entity_type == "select") { 
              info =         "\"val_tpl\": \"{{ value_json."+ object_id +" }}\"," //
            "\"cmd_t\": \""+ topic + "command\"," //
            "\"cmd_tpl\": \"{ \\\""+object_id+"\\\" : \\\"{{ value }}\\\" } \"," 
            "\"entity_category\": \""+ entity_category + "\"," //
			      "\"options\": ["+ entity_option + "]," ;//
          //  "\"avty_t\": \"homeassistant/sensor/"+ node_id +"/" + "status\","; //
      } 
      else if (entity_type == "binary_sensor") { 
              info =         "\"dev_cla\": \""+dev_cla+"\","
            "\"pl_on\":\"true\","
            "\"pl_off\":\"false\","
            "\"val_tpl\": \"{{ value_json."+ object_id +" }}\",";
         //   "\"avty_t\": \"homeassistant/sensor/"+ node_id +"/" + "status\",";
      }
      else if (entity_type == "button") { 
              info =            "\"entity_category\": \""+ entity_category + "\"," //
            "\"cmd_t\": \""+ topic + "command\"," //
            "\"pl_prs\": \"{ \\\""+object_id+"\\\" : \\\"1\\\"  } \",";
//"\"avty_t\": \"homeassistant/sensor/"+ node_id +"/" + "status\",";
      }
      return info;
    }

    //bool cmd_t; 



    //private:String state_topic; 
    //private:String stat_t; 
    //private:String avty_t;

    
    private:String node_mac = WiFi.macAddress().substring(12,14)+ WiFi.macAddress().substring(15,17);
    //private:String node_ids = WiFi.macAddress().substring(0,2)+ WiFi.macAddress().substring(4,6)+ WiFi.macAddress().substring(8,10) + WiFi.macAddress().substring(12,14)+ WiFi.macAddress().substring(15,17);
    private:String node_id = String("dimmer-") + node_mac; 
    //private:String topic = "homeassistant/sensor/"+ node_id +"/";
    private:String topic_switch = "homeassistant/switch/"+ node_id +"/";
    private:String topic_switch_state = "homeassistant/switch/";
    private:String device_declare() { 
              String IPaddress = WiFi.localIP().toString();
              String info =         "\"dev\": {"
              "\"ids\": \""+ node_id + "\","
              "\"name\": \""+ node_id + "\","
              "\"sw\": \"Dimmer "+ String(VERSION) +"\","
              "\"mdl\": \"ESP8266 " + IPaddress + "\","
              "\"mf\": \"Cyril Poissonnier\","
              "\"cu\": \"http://"+ IPaddress +"\""
            "}"; 
            return info;
            }

    public:void discovery(){
      String topic = "homeassistant/"+ entity_type +"/"+ node_id +"/";
      String device= "{\"name\": \""+ name + "\"," 
            "\"obj_id\": \"dimmer-"+ object_id +"-"+ node_mac + "\"," 
            "\"uniq_id\": \""+ node_mac + "-" + object_id +"\","
            "\"stat_t\": \""+ topic + object_id + "/state\"," 
            + sensor_type()
            + icon
            + expire_after
            + device_declare() + 
            "}";

      client.publish(String(topic+object_id+"/config").c_str() ,1,true, device.c_str()); // déclaration autoconf dimmer
      //send("0");
     
    }

    public:void send(String value){
      String topic = "homeassistant/"+ entity_type +"/"+ node_id +"/";
      String message = "  { \""+object_id+"\" : \"" + value.c_str() + "\"  } ";
      client.publish(String(topic + object_id + "/state").c_str() ,1,true, message.c_str());
      //client.loop();
    } 


};

/// création des sensors
HA device_dimmer; 
HA device_temp; 

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
HA device_dimmer_charge;

/// création select
HA device_dimmer_child_mode;

/// création binary_sensor
HA device_dimmer_alarm_temp;
HA device_cooler;

// creation remonté de puissance 
HA device_dimmer_power;
HA device_dimmer_total_power;

void devices_init(){
  /// création des sensors
  device_dimmer.Set_name("Puissance");
  device_dimmer.Set_object_id("power");
  device_dimmer.Set_unit_of_meas("%");
  device_dimmer.Set_stat_cla("measurement");
  device_dimmer.Set_dev_cla("power_factor"); // fix is using native unit of measurement '%' which is not a valid unit for the device class ('power') it is using
  device_dimmer.Set_icon("mdi:percent");
  device_dimmer.Set_entity_type("sensor");
  device_dimmer.Set_retain_flag(true);
  // device_dimmer.Set_expire_after(true);

  device_dimmer_power.Set_name("Watt");
  device_dimmer_power.Set_object_id("watt");
  device_dimmer_power.Set_unit_of_meas("W");
  device_dimmer_power.Set_stat_cla("measurement");
  device_dimmer_power.Set_dev_cla("power"); // fix is using native unit of measurement '%' which is not a valid unit for the device class ('power') it is using
  device_dimmer_power.Set_icon("mdi:home-lightning-bolt-outline");
  device_dimmer_power.Set_entity_type("sensor");
  device_dimmer_power.Set_retain_flag(true);

  device_dimmer_total_power.Set_name("Watt total");
  device_dimmer_total_power.Set_object_id("watt_total");
  device_dimmer_total_power.Set_unit_of_meas("W");
  device_dimmer_total_power.Set_stat_cla("measurement");
  device_dimmer_total_power.Set_dev_cla("power"); // fix is using native unit of measurement '%' which is not a valid unit for the device class ('power') it is using
  device_dimmer_total_power.Set_icon("mdi:home-lightning-bolt-outline");
  device_dimmer_total_power.Set_entity_type("sensor");
  device_dimmer_total_power.Set_retain_flag(true);


  device_temp.Set_name("Température");
  device_temp.Set_object_id("temperature");
  device_temp.Set_unit_of_meas("°C");
  device_temp.Set_stat_cla("measurement");
  device_temp.Set_dev_cla("temperature");
  device_temp.Set_entity_type("sensor");
  device_temp.Set_retain_flag(true);
  // device_dimmer.Set_expire_after(true);

  
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
  device_dimmer_starting_pow.Set_entity_valuemax("500"); // trop? pas assez? TODO : test sans valeur max?
  device_dimmer_starting_pow.Set_entity_valuestep("1");
  device_dimmer_starting_pow.Set_retain_flag(true);

  device_dimmer_minpow.Set_name("Puissance mini");
  device_dimmer_minpow.Set_object_id("minpow");
  device_dimmer_minpow.Set_entity_type("number");
  device_dimmer_minpow.Set_entity_category("config");
  device_dimmer_minpow.Set_entity_valuemin("0");
  device_dimmer_minpow.Set_entity_valuemax("100"); // trop? pas assez? TODO : test sans valeur max?
  device_dimmer_minpow.Set_entity_valuestep("1");
  device_dimmer_minpow.Set_retain_flag(true);

  device_dimmer_maxpow.Set_name("Puissance maxi");
  device_dimmer_maxpow.Set_object_id("maxpow");
  device_dimmer_maxpow.Set_entity_type("number");
  device_dimmer_maxpow.Set_entity_category("config");
  device_dimmer_maxpow.Set_entity_valuemin("0");
  device_dimmer_maxpow.Set_entity_valuemax("100"); // trop? pas assez? TODO : test sans valeur max?
  device_dimmer_maxpow.Set_entity_valuestep("1");
  device_dimmer_maxpow.Set_retain_flag(true);

  device_dimmer_send_power.Set_name("dimmer power");
  device_dimmer_send_power.Set_object_id("powdimmer");
  device_dimmer_send_power.Set_entity_type("number");
  device_dimmer_send_power.Set_entity_category("config");
  device_dimmer_send_power.Set_entity_valuemin("0");
  device_dimmer_send_power.Set_entity_valuemax("100"); // trop? pas assez? TODO : test sans valeur max?
  device_dimmer_send_power.Set_entity_valuestep("1");
  device_dimmer_send_power.Set_retain_flag(true);

  device_dimmer_maxtemp.Set_name("Température maxi");
  device_dimmer_maxtemp.Set_object_id("maxtemp");
  device_dimmer_maxtemp.Set_entity_type("number");
  device_dimmer_maxtemp.Set_entity_category("config");
  device_dimmer_maxtemp.Set_entity_valuemin("0");
  device_dimmer_maxtemp.Set_entity_valuemax("75"); // trop? pas assez? TODO : test sans valeur max?
  device_dimmer_maxtemp.Set_entity_valuestep("1");
  device_dimmer_maxtemp.Set_retain_flag(true);

  device_dimmer_charge.Set_name("Charge");
  device_dimmer_charge.Set_object_id("charge");
  device_dimmer_charge.Set_entity_type("number");
  device_dimmer_charge.Set_entity_category("config");
  device_dimmer_charge.Set_entity_valuemin("0");
  device_dimmer_charge.Set_entity_valuemax("3000");
  device_dimmer_charge.Set_entity_valuestep("50");
  device_dimmer_charge.Set_retain_flag(true);

  /// création des select
  device_dimmer_child_mode.Set_name("Mode");
  device_dimmer_child_mode.Set_object_id("child_mode");
  device_dimmer_child_mode.Set_entity_type("select");
  device_dimmer_child_mode.Set_entity_category("config");
  device_dimmer_child_mode.Set_entity_option("\"off\",\"delester\",\"equal\"");
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
}

void HA_discover(){
  if (mqtt_config.HA){
          Serial.println("HA discovery" );
        /// création des binary_sensor et enregistrement sous HA  
        device_dimmer_on_off.discovery();
        device_dimmer_on_off.send(String(config.dimmer_on_off));

        device_dimmer.discovery();
        device_dimmer.send(String(sysvar.puissance));

        device_dimmer_power.discovery();
        device_dimmer_power.send(String(sysvar.puissance* config.charge/100));

        device_dimmer_total_power.discovery();
        device_dimmer_total_power.send(String(sysvar.puissance_cumul + (sysvar.puissance * config.charge/100)));

        device_cooler.discovery();
        device_cooler.send(stringbool(false));

        device_temp.discovery(); // discovery fait à la 1ere réception sonde ou mqtt.


        #ifdef RELAY1
          device_relay1.discovery();
          device_relay1.send(String(0));
        #endif
        #ifdef RELAY2
          device_relay2.discovery();
          device_relay2.send(String(0));
        #endif
        device_dimmer_starting_pow.discovery();
        device_dimmer_starting_pow.send(String(config.startingpow));

        device_dimmer_minpow.discovery();
        device_dimmer_minpow.send(String(config.minpow));

        device_dimmer_maxpow.discovery();
        device_dimmer_maxpow.send(String(config.maxpow));

        device_dimmer_charge.discovery();
        device_dimmer_charge.send(String(config.charge));

        device_dimmer_send_power.discovery();
        device_dimmer_send_power.send(String(sysvar.puissance));

        device_dimmer_child_mode.discovery();
        device_dimmer_child_mode.send(String(config.mode));

        device_dimmer_save.discovery();
        }
}
#endif
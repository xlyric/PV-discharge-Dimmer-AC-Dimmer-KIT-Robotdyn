#ifndef HA_FUNCTIONS
#define HA_FUNCTIONS

#include <PubSubClient.h>

extern PubSubClient client;
extern Config config; 

struct MQTT
{

  private:int MQTT_INTERVAL = 60;
  // private:String IPaddress = String(WiFi.localIP().toString());

    /* MQTT */
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

  private:String retain; 
  public:void Set_retain(bool setter) {
    if (setter) {retain="\"ret\":true,"; }
  }

  private:String expire_after; 
  public:void Set_expire_after(bool setter) {
    if (setter) {expire_after="\"exp_aft\": \""+ String(MQTT_INTERVAL) +"\", "; }
  }

  private:String HA_sensor_type() {
    String topic = "homeassistant/"+ entity_type +"/"+ node_id +"/";
    String topic_Xlyric = "Xlyric/"+ node_id +"/";
    String info;
    if (entity_type == "sensor") {
            info =         "\"dev_cla\": \""+dev_cla+"\","
          "\"unit_of_meas\": \""+unit_of_meas+"\","
          "\"stat_cla\": \""+stat_cla+"\"," 
          "\"value_template\": \"{{ value_json."+ object_id +" }}\","; 
    }
    else if (entity_type == "switch") { 
            info =         "\"val_tpl\": \"{{ value_json."+ object_id +" }}\","
          "\"pl\":  \"{{ value_json."+ object_id +" }}\","
          "\"pl_on\": \"{ \\\""+object_id+"\\\" : \\\"1\\\"  } \","
          "\"pl_off\": \"{ \\\""+object_id+"\\\" : \\\"0\\\"  } \","
          "\"stat_on\":1,"
          "\"stat_off\":0,"
          "\"cmd_t\": \""+ topic_Xlyric + entity_type + "/command\",";
    } 
    else if (entity_type == "number") { 
            info =         "\"val_tpl\": \"{{ value_json."+ object_id +" }}\","
          "\"cmd_t\": \""+ topic_Xlyric + entity_type + "/command\","
          "\"cmd_tpl\": \"{ \\\""+object_id+"\\\" : {{ value }} } \"," 
          "\"entity_category\": \""+ entity_category + "\","
          "\"max\": \""+max+"\","
          "\"min\": \""+min+"\","
          "\"step\": \""+step+"\",";
    } 
    else if (entity_type == "select") { 
            info =         "\"val_tpl\": \"{{ value_json."+ object_id +" }}\","
          "\"cmd_t\": \""+ topic_Xlyric + entity_type + "/command\","
          "\"cmd_tpl\": \"{ \\\""+object_id+"\\\" : \\\"{{ value }}\\\" } \","
          "\"entity_category\": \""+ entity_category + "\","
          "\"options\": ["+ entity_option + "],";
    } 
    else if (entity_type == "binary_sensor") { 
            info =         "\"dev_cla\": \""+dev_cla+"\","
          "\"pl_on\":\"true\","
          "\"pl_off\":\"false\","
          "\"val_tpl\": \"{{ value_json."+ object_id +" }}\",";
    }
    else if (entity_type == "button") { 
            info =            "\"entity_category\": \""+ entity_category + "\","
          "\"cmd_t\": \""+ topic_Xlyric + entity_type + "/command\","
          "\"pl_prs\": \"{ \\\""+object_id+"\\\" : \\\"1\\\"  } \",";
    }
    return info;
  }

  //bool cmd_t; 

  private:String IPaddress = WiFi.localIP().toString();

  //private:String state_topic; 
  //private:String stat_t; 
  //private:String avty_t;


  private:String node_mac = WiFi.macAddress().substring(12,14)+ WiFi.macAddress().substring(15,17);
  private:String node_id = String("Dimmer-") + node_mac; 
  private:String HA_device_declare() { 
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
  

  public:void HA_discovery(){
    String topic = "homeassistant/"+ entity_type +"/"+ node_id +"/";
    String topic_Xlyric = "Xlyric/"+ node_id +"/";

    String device= "{\"name\": \""+ name + "\"," 
          "\"obj_id\": \"Dimmer-"+ object_id +"-"+ node_mac + "\"," 
          "\"uniq_id\": \""+ node_mac + "-" + object_id +"\","
          "\"stat_t\": \""+ topic_Xlyric + "sensors/" + object_id +"/state\"," 
          "\"avty_t\": \""+ topic_Xlyric + "status\","
          + HA_sensor_type()
          + icon
          + retain
          + expire_after
          + HA_device_declare() + 
          "}";
    client.publish((topic+object_id+"/config").c_str() , device.c_str(), true); // déclaration autoconf PvRouter
    // Serial.println(device.c_str());    /// sérial pour debug

  }

  public:void send(String value){
    if (config.JEEDOM || config.HA) {
      String topic = "Xlyric/"+ node_id +"/sensors/";
      String message = "  { \""+object_id+"\" : \"" + value.c_str() + "\"  } ";
      client.publish((topic + object_id + "/state").c_str() , message.c_str(), retain_flag);
    }
  } 
};

#endif
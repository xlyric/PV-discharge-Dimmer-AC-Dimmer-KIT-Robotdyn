#ifndef HA_FUNCTIONS
#define HA_FUNCTIONS

#include <PubSubClient.h>

extern PubSubClient client;


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
            "\"avty_t\": \""+ topic + "status\","
            "\"value_template\": \"{{ value_json."+ object_id +" }}\","; 
      }
      else if (entity_type == "switch") { 
              info =         "\"val_tpl\": \"{{ value_json."+ object_id +" }}\","
            "\"pl\":  \"{{ value_json."+ object_id +" }}\","
            "\"pl_on\": \"{ \\\""+object_id+"\\\" : \\\"1\\\"  } \","
            "\"pl_off\": \"{ \\\""+object_id+"\\\" : \\\"0\\\"  } \","
            "\"stat_on\":1,"
            "\"stat_off\":0,"
            "\"cmd_t\": \""+ topic + "command\"," 
            "\"avty_t\": \"homeassistant/sensor/"+ node_id +"/" + "status\",";
      } 
      else if (entity_type == "number") { 
              info =         "\"val_tpl\": \"{{ value_json."+ object_id +" }}\"," //
            "\"cmd_t\": \""+ topic + "command\"," //
            "\"cmd_tpl\": \"{ \\\""+object_id+"\\\" : {{ value }} } \"," 
            "\"entity_category\": \""+ entity_category + "\"," //
            "\"max\": \""+max+"\"," //
            "\"min\": \""+min+"\"," //
            "\"step\": \""+step+"\"," //
            "\"avty_t\": \"homeassistant/sensor/"+ node_id +"/" + "status\","; //
      } 
      else if (entity_type == "select") { 
              info =         "\"val_tpl\": \"{{ value_json."+ object_id +" }}\"," //
            "\"cmd_t\": \""+ topic + "command\"," //
            "\"cmd_tpl\": \"{ \\\""+object_id+"\\\" : \\\"{{ value }}\\\" } \"," 
            "\"entity_category\": \""+ entity_category + "\"," //
			      "\"options\": ["+ entity_option + "]," //
            "\"avty_t\": \"homeassistant/sensor/"+ node_id +"/" + "status\","; //
      } 
      else if (entity_type == "binary_sensor") { 
              info =         "\"dev_cla\": \""+dev_cla+"\","
            "\"pl_on\":\"true\","
            "\"pl_off\":\"false\","
            "\"val_tpl\": \"{{ value_json."+ object_id +" }}\","
            "\"avty_t\": \"homeassistant/sensor/"+ node_id +"/" + "status\",";
      }
      else if (entity_type == "button") { 
              info =            "\"entity_category\": \""+ entity_category + "\"," //
            "\"cmd_t\": \""+ topic + "command\"," //
            "\"pl_prs\": \"{ \\\""+object_id+"\\\" : \\\"1\\\"  } \","
            "\"avty_t\": \"homeassistant/sensor/"+ node_id +"/" + "status\",";
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
    //private:String uniq_id; 
    //private:String value_template; 

    /*public:void discovery(){
      IPaddress =   WiFi.localIP().toString() ;
      String device= "{ \"dev_cla\": \""+dev_cla+"\","
            "\"unit_of_meas\": \""+unit_of_meas+"\","
            "\"stat_cla\": \""+stat_cla+"\"," 
            "\"name\": \""+ name +"-"+ node_mac + "\"," 
            "\"state_topic\": \""+ topic +  "state\","
            "\"stat_t\": \""+ topic  + "state\","
            "\"avty_t\": \""+ topic + "status\","
            "\"uniq_id\": \""+ node_mac + "-" + name +"\", "
            "\"value_template\": \"{{ value_json."+name +" }}\", "
            "\"cmd_t\": \""+ topic +"command\","
            "\"cmd_tpl\": \"{{ value_json."+name +" }}\", "
            + icon
            + device_declare() + 
          "}";
          if (dev_cla =="" ) { dev_cla = name; }
          client.publish((topic+dev_cla+"/config").c_str() , device.c_str() , true); // déclaration autoconf dimmer
          Serial.println(device.c_str());
    }*/

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

      client.publish(String(topic+object_id+"/config").c_str() , device.c_str(), true); // déclaration autoconf dimmer
     // Serial.println(device.c_str());   /// sérial pour debug
    }

   /* public:void discovery_switch(){
      IPaddress =   WiFi.localIP().toString() ;
      String device= "{"
            "\"name\": \""+ name +"-"+ node_mac + "\"," 
            "\"payload_off\": \"OFF\","
            "\"payload_on\": \"ON\","
            "\"state_topic\": \""+ topic_switch_state +  name + "_" + node_mac + "/state\","
            "\"uniq_id\": \""+ node_mac + "-" + name +"\", "
            "\"value_template\": \"{{ value_json."+name +" }}\", "
            "\"cmd_t\": \""+ topic_switch + name +"/set\","
            + icon
            + device_declare() + 
          "}";
          if (dev_cla =="" ) { dev_cla = name; }
          client.publish((topic_switch+dev_cla+"/config").c_str() , device.c_str() , true); // déclaration autoconf dimmer
          Serial.println(device.c_str());
          Serial.println(ESP.getFreeHeap());
    }

    public:void send2(String value){
       //String message = "  { \""+name+"\" : \"" + value.c_str() + "\"  } ";
       client.publish((topic + "state").c_str() , value.c_str(), true);
    }

    public:void switch_send(String value){
       client.publish((topic_switch_state + name + "_" + node_mac + "/state").c_str() , value.c_str(), true);
    }*/
 
    public:void send(String value){
      String topic = "homeassistant/"+ entity_type +"/"+ node_id +"/";
      String message = "  { \""+object_id+"\" : \"" + value.c_str() + "\"  } ";
      client.publish(String(topic + object_id + "/state").c_str() , message.c_str(), retain_flag);
    } 


};


#endif
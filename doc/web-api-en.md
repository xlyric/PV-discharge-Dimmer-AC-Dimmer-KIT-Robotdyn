# Web API Reference

## Power Control

### Set Power Level
```
GET /?POWER=<value>
```
- **Description**: Set the dimmer power level
- **Parameters**: 
  - `POWER`: Power level (0-100% or 0-200% for child dimmer configurations)
  - `puissance`: Available power in Watts (optional, takes priority over POWER)
- **Response**: Current state in JSON
- **Example**: `http://192.168.1.100/?POWER=75`

### Set Initial Power Offset
```
GET /?OFFSET=<value>
```
- **Description**: Set the starting power offset in Watts
- **Parameters**: `OFFSET`: Starting power offset value
- **Response**: Current state in JSON

## Status and Monitoring

### Get System State
```
GET /state
```
- **Description**: Get complete system status
- **Response**: JSON containing:
  - `dimmer`: Current dimmer power level (%)
  - `commande`: Commanded power level (%)
  - `temperature`: Main Dallas temperature sensor (°C)
  - `power`: Current power consumption (W)
  - `Ptotal`: Total accumulated power (W)
  - `relay1`, `relay2`: Relay states
  - `minuteur`: Timer status
  - `onoff`: Dimmer on/off state
  - `alerte`: Current alerts
  - `boost`: Boost mode status
  - `dallas0`, `dallas1`, etc.: Individual temperature sensors

### Get Temperature Sensors
```
GET /state_dallas
```
- **Description**: Get detailed temperature sensor information
- **Response**: JSON with all Dallas sensor temperatures and addresses

### Get Configuration
```
GET /config
```
- **Description**: Get current device configuration
- **Response**: JSON with all configuration parameters

## Configuration Management

### Update Configuration
```
GET /get?<parameters>
```
- **Description**: Update device configuration
- **Parameters**:
  - `hostname`: MQTT server hostname
  - `port`: MQTT server port
  - `maxtemp`: Maximum temperature limit (°C)
  - `charge1`, `charge2`, `charge3`: Power ratings for each channel (W)
  - `startingpow`: Starting power offset (W)
  - `minpow`, `maxpow`: Minimum/maximum power limits (%)
  - `child`: Child dimmer IP address
  - `mode`: Child dimmer mode ("equal", "delest", "off")
  - `dimmername`: Device name
  - `dimmer_on_off`: Enable/disable dimmer (0/1)
  - `mqttuser`, `mqttpassword`: MQTT credentials
  - `DALLAS`: Master Dallas sensor address
  - `trigger`: Trigger configuration
  - `save`: Save configuration to flash ("1")
- **Example**: `http://192.168.1.100/get?maxtemp=65&save=1`

### Toggle Service Modes
```
GET /get?servermode=<MODE>
```
- **Modes**: `MQTT`, `HA`, `JEEDOM`, `DOMOTICZ`, `ONOFF`
- **Description**: Toggle integration services on/off

## Timer and Scheduling

### Get Timer Status
```
GET /getminuteur[?dimmer|relay1|relay2]
```
- **Description**: Get timer configuration and current time
- **Parameters**: Specify which timer (dimmer, relay1, or relay2)
- **Response**: Timer settings and current system time

### Set Timer Configuration
```
GET /setminuteur?<parameters>
```
- **Parameters**:
  - `dimmer|relay1|relay2`: Target device
  - `heure_demarrage`: Start time (HH:MM format)
  - `heure_arret`: Stop time (HH:MM format)
  - `puissance`: Power level for dimmer timer
  - `temperature`: Temperature limit (°C)

### Get/Set Relay Thresholds
```
GET /getseuil[?relay1|relay2]
GET /setseuil?<parameters>
```
- **Parameters for setseuil**:
  - `relay1|relay2`: Target relay
  - `seuil_demarrage`: Start threshold
  - `seuil_arret`: Stop threshold
  - `temperature`: Temperature threshold

## Boost and Special Functions

### Activate Boost Mode
```
GET /boost
```
- **Description**: Activate 2-hour boost mode
- **Response**: Boost timer status in JSON

### Relay Control
```
GET /get?relay1=<state>
GET /get?relay2=<state>
```
- **Parameters**: 
  - `state`: 0 (off), 1 (on), 2 (toggle)
- **Response**: Current relay state (0/1)

## System Management

### Device Control
```
GET /reboot          # Restart device
GET /reset           # Reset and restart
GET /disconnect      # Disconnect WiFi
GET /resetwifi       # Reset WiFi settings and restart
GET /ping            # Health check (returns "pong")
```

### On/Off Control
```
GET /onoff
```
- **Description**: Toggle main dimmer on/off state
- **Response**: Current state ("1" or "0")

### Get System Logs
```
GET /cs
```
- **Description**: Get system logs and clear log buffer
- **Response**: Plain text log entries

## MQTT Integration

### Get MQTT Configuration
```
GET /getmqtt
```
- **Response**: JSON with MQTT server settings and integration status

### MQTT Backup/Restore
```
GET /readmqtt
```
- **Description**: Subscribe to MQTT restore topic and redirect to config page

## File Downloads

### Configuration Backup
```
GET /save
```
- **Description**: Download current configuration as JSON file

### Static Resources
```
GET /config.json     # Configuration file
GET /mqtt.json       # MQTT settings
GET /wifi.json       # WiFi settings
GET /programme.json  # Timer programs
GET /lang.json       # Language file
```

## Integration Examples

### Home Assistant
```yaml
# configuration.yaml
sensor:
  - platform: rest
    resource: "http://192.168.1.100/state"
    name: "PV Dimmer"
    json_attributes:
      - dimmer
      - temperature  
      - power
    value_template: "{{ value_json.dimmer }}"
```

### Domoticz
```
# Power control
http://192.168.1.100/?POWER=75

# Status monitoring
http://192.168.1.100/state
```

### Node-RED
```javascript
// Set power level
msg.url = "http://192.168.1.100/?POWER=" + msg.payload;
return msg;

// Get status
msg.url = "http://192.168.1.100/state";
return msg;
```

**Important Note**: All API endpoints support both GET and POST methods. For security in production environments, consider implementing authentication and using HTTPS.
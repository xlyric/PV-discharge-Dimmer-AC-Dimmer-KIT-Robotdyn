# Web API Reference

## Power Control

### Set Power Level
```
GET /?POWER=<value>
```
- **Description**: Définir le niveau de puissance du variateur
- **Paramètres**: 
  - `POWER`: Niveau de puissance (0-100% ou 0-200% pour configurations de variateur enfant)
  - `puissance`: Puissance disponible en Watts (optionnel, prioritaire sur POWER)
- **Réponse**: État actuel en JSON
- **Exemple**: `http://192.168.1.100/?POWER=75`

### Définir un décalage de puissance initial
```
GET /?OFFSET=<value>
```
- **Description**: Définir le décalage de puissance initial en Watts
- **Paramètres**: `OFFSET`: Valeur du décalage de puissance initial
- **Réponse**: État actuel en JSON

## Statut et Surveillance

### Obtenir l'état du système
```
GET /state
```
- **Description**: Obtenir le statut complet du système
- **Réponse**: JSON contenant :
  - `dimmer`: Niveau de puissance actuel du variateur (%)
  - `commande`: Niveau de puissance commandé (%)
  - `temperature`: Température principale du capteur Dallas (°C)
  - `power`: Consommation de puissance actuelle (W)
  - `Ptotal`: Puissance totale accumulée (W)
  - `relay1`, `relay2`: États des relais
  - `minuteur`: Statut du minuteur
  - `onoff`: État marche/arrêt du variateur
  - `alerte`: Alertes actuelles
  - `boost`: Statut du mode boost
  - `dallas0`, `dallas1`, etc.: Capteurs de température individuels

### Obtenir les informations des capteurs de température
```
GET /state_dallas
```
- **Description**: Obtenir des informations détaillées sur les capteurs de température
- **Réponse**: JSON avec les températures et adresses de tous les capteurs Dallas

### Obtenir la configuration
```
GET /config
```
- **Description**: Obtenir la configuration actuelle de l'appareil
- **Réponse**: JSON avec tous les paramètres de configuration

## Gestion de la Configuration

### Mettre à jour la configuration
```
GET /get?<parameters>
```
- **Description**: Mettre à jour la configuration de l'appareil
- **Paramètres**:
  - `hostname`: Nom d'hôte du serveur MQTT
  - `port`: Port du serveur MQTT
  - `maxtemp`: Limite de température maximale (°C)
  - `charge1`, `charge2`, `charge3`: Puissances pour chaque canal (W)
  - `startingpow`: Décalage de puissance initial (W)
  - `minpow`, `maxpow`: Limites de puissance minimale/maximale (%)
  - `child`: Adresse IP du variateur enfant
  - `mode`: Mode du variateur enfant ("equal", "delest", "off")
  - `dimmername`: Nom de l'appareil
  - `dimmer_on_off`: Activer/désactiver le variateur (0/1)
  - `mqttuser`, `mqttpassword`: Identifiants MQTT
  - `DALLAS`: Adresse du capteur Dallas maître
  - `trigger`: Configuration du déclencheur
  - `save`: Sauvegarder la configuration en flash ("1")
- **Exemple**: `http://192.168.1.100/get?maxtemp=65&save=1`

### Basculer les modes de service
```
GET /get?servermode=<MODE>
```
- **Modes**: `MQTT`, `HA`, `JEEDOM`, `DOMOTICZ`, `ONOFF`
- **Description**: Activer/désactiver les services d'intégration

## Minuterie et Programmation

### Obtenir le statut du minuteur
```
GET /getminuteur[?dimmer|relay1|relay2]
```
- **Description**: Obtenir la configuration du minuteur et l'heure actuelle
- **Paramètres**: Spécifier le minuteur (dimmer, relay1, or relay2)
- **Réponse**: Paramètres du minuteur et heure système actuelle

### Configurer le minuteur
```
GET /setminuteur?<parameters>
```
- **Paramètres**:
  - `dimmer|relay1|relay2`: Appareil cible
  - `heure_demarrage`: Heure de début (format HH:MM)
  - `heure_arret`: Heure d'arrêt (format HH:MM)
  - `puissance`: Niveau de puissance pour le minuteur
  - `temperature`: Limite de température (°C)

### Obtenir/Définir les seuils de relais
```
GET /getseuil[?relay1|relay2]
GET /setseuil?<parameters>
```
- **Paramètres pour setseuil**:
  - `relay1|relay2`: Relais cible
  - `seuil_demarrage`: Seuil de démarrage
  - `seuil_arret`: Seuil d'arrêt
  - `temperature`: Seuil de température

## Fonctions Boost et Spéciales

### Activer le mode Boost
```
GET /boost
```
- **Description**: Activer le mode boost de 2 heures
- **Réponse**: État du minuteur boost en JSON

### Contrôle des Relais
```
GET /get?relay1=<state>
GET /get?relay2=<state>
```
- **Paramètres**: 
  - `state`: 0 (éteint), 1 (allumé), 2 (basculer)
- **Réponse**: État actuel du relais (0/1)

## Gestion du Système

### Contrôle de l'Appareil
```
GET /reboot          # Redémarrer l'appareil
GET /reset           # Réinitialiser et redémarrer
GET /disconnect      # Déconnecter le WiFi
GET /resetwifi       # Réinitialiser les paramètres WiFi et redémarrer
GET /ping            # Vérification de santé (retourne "pong")
```

### Contrôle Marche/Arrêt
```
GET /onoff
```
- **Description**: Basculer l'état marche/arrêt du variateur principal
- **Réponse**: État actuel ("1" ou "0")

### Obtenir les Logs Système
```
GET /cs
```
- **Description**: Obtenir les entrées de log système et effacer le tampon de log
- **Réponse**: Entrées de log en texte brut

## Intégration MQTT

### Obtenir la Configuration MQTT
```
GET /getmqtt
```
- **Réponse**: Paramètres du serveur MQTT et statut d'intégration

### Sauvegarde/Restauration MQTT
```
GET /readmqtt
```
- **Description**: S'abonner au topic de restauration MQTT et rediriger vers la page de configuration

## Téléchargement de Fichiers

### Sauvegarde de Configuration
```
GET /save
```
- **Description**: Télécharger la configuration actuelle en tant que fichier JSON

### Ressources Statiques
```
GET /config.json     # Fichier de configuration
GET /mqtt.json       # Paramètres MQTT
GET /wifi.json       # Paramètres WiFi
GET /programme.json  # Programmes de minuterie
GET /lang.json       # Fichier de langue
```

## Exemples d'Intégration

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
# Contrôle de puissance
http://192.168.1.100/?POWER=75

# Surveillance de l'état
http://192.168.1.100/state
```

### Node-RED
```javascript
// Définir le niveau de puissance
msg.url = "http://192.168.1.100/?POWER=" + msg.payload;
return msg;

// Obtenir l'état
msg.url = "http://192.168.1.100/state";
return msg;
```

**Note importante**: Tous les points de terminaison de l'API prennent en charge les méthodes GET et POST. Pour des raisons de sécurité dans les environnements de production, pensez à implémenter une authentification et à utiliser HTTPS.
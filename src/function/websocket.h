#ifndef WEBSOCKET_FUNCTIONS
#define WEBSOCKET_FUNCTIONS

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include "config/config.h"
#include "config/enums.h"
#include "function/unified_dimmer.h"
#include <ArduinoWebsockets.h>

extern System sysvar;
extern gestion_puissance unified_dimmer;
extern AsyncWebServer server;
extern Config config;

// Création de l'instance AsyncWebSocket
static AsyncWebSocket ws("/ws");

String getWebSocketData();
void updateWebSocketClients();

using namespace websockets;
WebsocketsClient clientws;
void clientWebSocket();
void onMessageCallback(WebsocketsMessage message);
void clientWebSocketLoop();
void clientWebSocketSend(String message);

// Variable pour stocker le timestamp de la dernière mise à jour
unsigned long lastUpdateTime = 0;
int stored_power = 0;
float stored_temp = 0;
// Intervalle de mise à jour en millisecondes (1 seconde par défaut)
const unsigned long updateInterval = 1000;

/**
 * @brief Fonction appelée lorsqu'un événement WebSocket se produit
 * 
 * @param client Client WebSocket
 * @param type Type d'événement
 * @param arg Arguments supplémentaires
 * @param data Données reçues
 * @param len Longueur des données
 */
void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("[WebSocket] Client #%u connecté depuis %s\n", client->id(), client->remoteIP().toString().c_str());
      // Envoyer immédiatement les données actuelles au client qui vient de se connecter
      client->text(getWebSocketData());
      // il faut toute les connexions se ferment si plus de 2 clients
      if (server->count() > 2) {
        Serial.println("[WebSocket] Trop de clients connectés, fermeture de la connexion");
        server->closeAll();
        server->cleanupClients(1);
      }      
      break;
    
    case WS_EVT_DISCONNECT:
      Serial.printf("[WebSocket] Client #%u déconnecté\n", client->id());
      client->close(); // Fermer la connexion
      break;
    
    case WS_EVT_DATA:
      // On pourrait traiter les messages reçus ici si nécessaire
      Serial.printf("[WebSocket] Message reçu du client #%u\n", client->id());
      // afficher les données reçues
      Serial.write(data, len);
      Serial.println();

      break;
    
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      // Autres types d'événements non traités
      break;
  }
}

/**
 * @brief Génère les données JSON à envoyer aux clients WebSocket
 * 
 * @return String Données au format JSON
 */
String getWebSocketData() {
  JsonDocument doc;
  
  // Ajouter la température
  char tempBuffer[10];
  dtostrf(sysvar.celsius[sysvar.dallas_maitre], 2, 1, tempBuffer);
  doc["temperature"] = tempBuffer;
  stored_temp = sysvar.celsius[sysvar.dallas_maitre];
  
  // Ajouter la puissance
  int instant_power = unified_dimmer.get_power();
  stored_power = sysvar.puissance_cumul + (instant_power * config.charge/100);
  doc["Ptotal"] = stored_power;

  // Timestamp pour suivre les mises à jour
  doc["timestamp"] = millis();
  
  String jsonString;
  serializeJson(doc, jsonString);
  return jsonString;
}

/**
 * @brief Met à jour les clients WebSocket avec les dernières données
 * Cette fonction doit être appelée régulièrement dans la boucle principale
 */
void updateWebSocketClients() {
int instant_power = unified_dimmer.get_power();
int temp_power = sysvar.puissance_cumul + (instant_power * config.charge/100);
  if (temp_power != stored_power || stored_temp != sysvar.celsius[sysvar.dallas_maitre]) {
  //vérifie d'un changement de puissance ou température

  // Générer les données JSON
    String json = getWebSocketData();
    
    // Envoyer les données à tous les clients connectés
    ws.textAll(json);
  }
}

/**
 * @brief Configure et démarre le serveur WebSocket
 * Cette fonction doit être appelée dans la fonction setup()
 */
void setupWebSocket() {
  // Configurer le gestionnaire d'événements WebSocket
  ws.onEvent(onWebSocketEvent);
 
  // Ajouter le gestionnaire WebSocket au serveur
  server.addHandler(&ws);
  server.begin();
  
  Serial.println("[WebSocket] Serveur WebSocket démarré sur /ws");
}

const char* websocket_server_host = "192.168.1.45"; // IP par défaut du serveur en mode AP
const int websocket_server_port = 80;



// récupération du message du serveur
void onMessageCallback(WebsocketsMessage message) {
  Serial.print("Message du serveur : ");
  Serial.println(message.data());
}

void clientWebSocket() {
  
  clientws.onMessage(onMessageCallback);
  if (clientws.connect(websocket_server_host, websocket_server_port, "/ws")) {
      Serial.println("Connecté au WebSocket !");
      clientws.send("Hello Server!");
  } else {
      Serial.println("Échec de connexion WebSocket.");
  }
  
}

void clientWebSocketSend(String message) {
  clientws.send(message);
}

void clientWebSocketLoop() {
  clientws.poll();
  // test de connexion avec le serveur 
  if (!clientws.ping()) {
    clientWebSocket();
  }

}

#endif // WEBSOCKET_FUNCTIONS

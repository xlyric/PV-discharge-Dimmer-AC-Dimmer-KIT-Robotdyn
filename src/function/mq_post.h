#ifndef MQ_POST_H
#define MQ_POST_H

// Émission MQTT vérifiée : taille, retour de publish(), déconnexion propre.

#include <Arduino.h>
#include <PubSubClient.h>

extern PubSubClient client;

bool mq_post(const char* topic, const char* payload, bool retained) {
  if (topic == nullptr) { return false; }
  if (payload == nullptr) { return false; }
  if (!client.connected()) { return false; }

  // +5 : en-tête fixe (1 octet) + longueur restante (jusqu'à 4 octets)
  const auto bufSize = client.getBufferSize();
  const auto needed = strnlen(topic, bufSize) + strnlen(payload, bufSize) + 5;
  if (needed > bufSize) {
    Serial.printf("MQTT: message trop long (%u > %u), ignore: %s\n",
                  (unsigned)needed, (unsigned)bufSize, topic);
    return false;
  }

  if (!client.publish(topic, payload, retained)) {
    Serial.printf("MQTT: echec publish (%s) -> deconnexion propre\n", topic);
    client.disconnect();
    return false;
  }

  delay(2);   // laisse la pile WiFi vider la socket avant le message suivant
  return true;
}

bool mq_post(const String& topic, const String& payload, bool retained) {
  return mq_post(topic.c_str(), payload.c_str(), retained);
}

#endif

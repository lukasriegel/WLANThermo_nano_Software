
/***************************************************
  Copyright (C) 2017  Steffen Ochs, Holger Imbery

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

  HISTORY: Please refer Github History

 ****************************************************/

//#define MQTT_DEBUG              // ENABLE SERIAL MQTT DEBUG MESSAGES

#ifdef MQTT_DEBUG
  #define MQPRINT(...)    Serial.print(__VA_ARGS__)
  #define MQPRINTLN(...)  Serial.println(__VA_ARGS__)
  #define MQPRINTP(...)   Serial.print(F(__VA_ARGS__))
  #define MQPRINTPLN(...) Serial.println(F(__VA_ARGS__))
  #define MQPRINTF(...)   Serial.printf(__VA_ARGS__)
  
#else
  #define MQPRINT(...)     //blank line
  #define MQPRINTLN(...)   //blank line 
  #define MQPRINTP(...)    //blank line
  #define MQPRINTPLN(...)  //blank line
  #define MQPRINTF(...)    //blank line
#endif
 
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Start MQTT
void connectToMqtt() {
  if (iot.P_MQTT_on) pmqttClient.connect();
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++
// MQTT Handler
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  IPRINTPLN("d:MQTT");
  if (WiFi.isConnected()) connectToMqtt;
}

void onMqttConnect(bool sessionPresent) {
  IPRINTPLN("c:MQTT");
  MQPRINTP("[MQTT]\tSession present: ");
  MQPRINTLN(sessionPresent);
  String adress = F("WLanThermo/");
  adress += sys.host;
  adress += F("/set/#");
  uint16_t packetIdSub = pmqttClient.subscribe(adress.c_str(), 2);
  MQPRINTP("[MQTT]\tSubscribing at QoS 2, packetId: ");
  MQPRINTLN(packetIdSub);
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  MQPRINTPLN("[MQTT]\tSubscribe acknowledged.");
  MQPRINTP("packetId: ");
  MQPRINTLN(packetId);
  MQPRINTP("qos: ");
  MQPRINTLN(qos);
}

void onMqttUnsubscribe(uint16_t packetId) {
  MQPRINTPLN("[MQTT]\tUnsubscribe acknowledged.");
  MQPRINTP("packetId: ");
  MQPRINTLN(packetId);
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  String topic_prefix = F("WLanThermo/");
  topic_prefix += sys.host;
  topic_prefix += F("/set/");
  int topic_prefix_length = topic_prefix.length();
  String topic_short = String(topic);
  topic_short.remove(0, topic_prefix_length);
  // temp - min/max
  if (topic_short.startsWith("temp")) {
    float new_payload = atof(payload);
    for (int i = 0; i < 8; i++) {
      String test1 = "temp" + String(i) + "/min";
      String test2 = "temp" + String(i) + "/max";
      if (test1 == topic_short) {
        ch[i-1].min = new_payload;
      }
      else if (test2 == topic_short) {
        ch[i-1].max = new_payload;
      }
      else {
      }
    }
    setconfig(eCHANNEL, {});
  }
  // skeleton
  if (topic_short.startsWith("alarm")) {
    bool newb_payload = (char)atoi(payload);
    for (int i = 0; i < 8; i++) {
      String test3 = "alarm" + String(i);
      if (test3 == topic_short) {
        ch[i].alarm = newb_payload;
      }
      
      else {
      }
    }
    setconfig(eCHANNEL, {});
    loadconfig(eCHANNEL);
  } 
}
  
  // skeleton
  // if (topic_short.startsWith("dummy")) {
  // }


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Configuration MQTT
void set_pmqtt() {
  pmqttClient.onConnect(onMqttConnect);
  pmqttClient.onDisconnect(onMqttDisconnect);
  pmqttClient.onSubscribe(onMqttSubscribe);
  pmqttClient.onUnsubscribe(onMqttUnsubscribe);
  pmqttClient.onMessage(onMqttMessage);
  pmqttClient.setServer(iot.P_MQTT_HOST.c_str(), iot.P_MQTT_PORT);
  pmqttClient.setCredentials(iot.P_MQTT_USER.c_str(), iot.P_MQTT_PASS.c_str());
}


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Send data to private MQTT Broker
void sendpmqtt() {

  if (pmqttClient.connected()) {

    unsigned long vorher = millis();
    String prefix = F("WLanThermo/");
    prefix += sys.host;
    prefix += F("/status/");

    
    for (int i = 0; i < 8; i++)  {
      if (ch[i].temp != INACTIVEVALUE) {
        String temp_adress = prefix + "temp";
        temp_adress += String(i + 1);
        String posttempStr = String(ch[i].temp, 1);
        pmqttClient.publish(temp_adress.c_str(), iot.P_MQTT_QoS, false, posttempStr.c_str());
      }
    }
    for (int i = 0; i < 8; i++)  {
      if (ch[i].max != INACTIVEVALUE) {
        String max_adress = prefix + "temp";
        max_adress += String(i + 1);
        max_adress += "/max";
        String posttempStr = String(ch[i].max, 1);
        pmqttClient.publish(max_adress.c_str(), iot.P_MQTT_QoS, false, posttempStr.c_str());
      }
    }
    for (int i = 0; i < 8; i++)  {
      if (ch[i].min != INACTIVEVALUE) {
        String min_adress = prefix + "temp";
        min_adress += String(i + 1);
        min_adress += "/min";
        String posttempStr = String(ch[i].min, 1);
        pmqttClient.publish(min_adress.c_str(), iot.P_MQTT_QoS, false, posttempStr.c_str());
      }
    }
 
    String volt_adress = prefix + "voltage";
    String postvoltStr = String(battery.percentage);
    pmqttClient.publish(volt_adress.c_str(), iot.P_MQTT_QoS, false, postvoltStr.c_str());

 
    String wlan_adress = prefix + "wlan";
    String postwlanStr = String(wifi.rssi);
    pmqttClient.publish(wlan_adress.c_str(), iot.P_MQTT_QoS, false, postwlanStr.c_str());


    MQPRINTF("[MQTT]\tp: %ums\r\n", millis() - vorher);   // Published to MQTT Broker

  } else {
    MQPRINTPLN("[MQTT]\tf:");        // not connect to MQTT Broker
    pmqttClient.connect();
  }
}


#include <ArduinoJson.h>
#include "FS.h"

JsonObject* loadJSONFile(char* fileName) {
  File configFile = SPIFFS.open(fileName, "r");
  if (!configFile) {
    Serial.println("Failed to open config file");
    return NULL;
  }

  size_t size = configFile.size();
  if (size > 1024) {
    Serial.println("Config file size is too large");
    return NULL;
  }

  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable. If you don't use ArduinoJson, you may as well
  // use configFile.readString instead.
  configFile.readBytes(buf.get(), size);

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(buf.get());

  if (!json.success()) {
    Serial.println("Failed to parse config file");
    return NULL;
  }
  return &json;
}

bool loadWifiSettings() {
  JsonObject* jsonp = loadJSONFile("/wifi.json");
  if (jsonp == NULL) {
    return false;
  }
  JsonObject& json = *jsonp;
  
  wifiEnabled = json["wifiEnabled"];
  
  return true;
}
bool loadConfig() {
  JsonObject* jsonp = loadJSONFile("/config.json");
  if (jsonp == NULL) {
    return false;
  }
  JsonObject& json = *jsonp;
  // Now get the settings from the json

  main_color.red = json["red"];
  main_color.green = json["green"];
  main_color.blue = json["blue"];
  ws2812fx_speed = json["speed"];
  brightness = json["brightness"];
  ws2812fx_mode = json["mode"]; 

  Serial.println("Settings loaded from json");
  return true;
}

bool saveJSONFile(char* fileName, JsonObject& json) {
  File configFile = SPIFFS.open(fileName, "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return false;
  }

  json.printTo(configFile);
 
  Serial.println("Settings saved to json");
  return true;
}

bool saveConfig() {
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  
  json["red"] = main_color.red;
  json["green"] = main_color.green;
  json["blue"] = main_color.blue;
  json["speed"] = ws2812fx_speed;
  json["brightness"] = brightness;
  json["mode"] = ws2812fx_mode;
  
  return saveJSONFile("/config.json", json);
}

bool saveWifiSettings() {
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  
  json["wifiEnabled"] = wifiEnabled;
  
  return saveJSONFile("/wifi.json", json);
}

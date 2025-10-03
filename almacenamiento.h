/**
 * almacenamiento.h
 * Funciones para cargar y guardar datos en SPIFFS
 */

#ifndef ALMACENAMIENTO_H
#define ALMACENAMIENTO_H

// ========= FUNCIONES DE GUARDADO Y CARGA DE DATOS ===========
void loadConfig() {
  File file = SPIFFS.open("/config.json", "r");
  if (!file) {
    Serial.println("No hay archivo de configuración, usando valores por defecto");
    return;
  }
  
  DynamicJsonDocument doc(512);
  DeserializationError error = deserializeJson(doc, file);
  if (error) {
    Serial.println("Error al leer configuración");
    return;
  }

  deviceId = doc["deviceId"] | 0x00010001;
  
  // Leer configuración básica
  strlcpy(basicConfig.firmwareVersion, doc["firmware"] | FIRMWARE_VERSION, 9);
  
  JsonArray pwd = doc["password"];
  for (int i = 0; i < 3 && i < pwd.size(); i++) {
    basicConfig.password[i] = pwd[i];
  }
  
  basicConfig.sleepTime = doc["sleep"] | 0;
  basicConfig.volume = doc["volume"] | 5;
  basicConfig.language = doc["language"] | 2;
  basicConfig.dateFormat = doc["dateformat"] | 0x02;
  basicConfig.machineStatus = doc["status"] | 0;
  basicConfig.languageFlag = doc["langflag"] | 0x10;
  basicConfig.cmdVersion = doc["cmdver"] | 0x02;

  // Cargar pines GPIO, con valores por defecto si no existen
  basicConfig.pin_d0 = doc["pin_d0"] | D7;
  basicConfig.pin_d1 = doc["pin_d1"] | D6;
  basicConfig.pin_relay = doc["pin_relay"] | D1;
  basicConfig.pin_led = doc["pin_led"] | D0;
  
  file.close();
}

void saveConfig() {
  DynamicJsonDocument doc(512);

  doc["deviceId"] = deviceId;
  
  doc["firmware"] = basicConfig.firmwareVersion;
  
  JsonArray pwd = doc.createNestedArray("password");
  for (int i = 0; i < 3; i++) {
    pwd.add(basicConfig.password[i]);
  }
  
  doc["sleep"] = basicConfig.sleepTime;
  doc["volume"] = basicConfig.volume;
  doc["language"] = basicConfig.language;
  doc["dateformat"] = basicConfig.dateFormat;
  doc["status"] = basicConfig.machineStatus;
  doc["langflag"] = basicConfig.languageFlag;
  doc["cmdver"] = basicConfig.cmdVersion;

  // Guardar pines GPIO
  doc["pin_d0"] = basicConfig.pin_d0;
  doc["pin_d1"] = basicConfig.pin_d1;
  doc["pin_relay"] = basicConfig.pin_relay;
  doc["pin_led"] = basicConfig.pin_led;
  
  File file = SPIFFS.open("/config.json", "w");
  if (!file) {
    Serial.println("Error al crear archivo de configuración");
    return;
  }
  
  serializeJson(doc, file);
  file.close();
}

void loadWebAuth() {
  File file = SPIFFS.open("/webauth.json", "r");
  if (!file) {
    Serial.println("No hay archivo de autenticación web, usando valores por defecto.");
    return;
  }

  DynamicJsonDocument doc(256);
  DeserializationError error = deserializeJson(doc, file);
  if (error) {
    Serial.println("Error al leer autenticación web.");
    file.close();
    return;
  }

  strlcpy(web_user, doc["user"] | "admin", sizeof(web_user));
  strlcpy(web_pass, doc["pass"] | "admin", sizeof(web_pass));

  file.close();
}

void saveWebAuth() {
  DynamicJsonDocument doc(256);
  doc["user"] = web_user;
  doc["pass"] = web_pass;

  File file = SPIFFS.open("/webauth.json", "w");
  if (!file) {
    Serial.println("Error al crear archivo de autenticación web.");
    return;
  }

  serializeJson(doc, file);
  file.close();
}

void loadUsers() {
  File file = SPIFFS.open("/users.json", "r");
  if (!file) {
    Serial.println("No hay archivo de usuarios");
    return;
  }
  
  DynamicJsonDocument doc(10000);
  DeserializationError error = deserializeJson(doc, file);
  if (error) {
    Serial.println("Error al leer usuarios");
    return;
  }
  
  userCount = doc["count"] | 0;
  JsonArray array = doc["users"];
  
  for (int i = 0; i < userCount && i < 100 && i < array.size(); i++) {
    JsonArray id = array[i]["id"];
    for (int j = 0; j < 5 && j < id.size(); j++) {
      users[i].id[j] = id[j];
    }
    
    JsonArray pwd = array[i]["pwd"];
    for (int j = 0; j < 3 && j < pwd.size(); j++) {
      users[i].password[j] = pwd[j];
    }
    
    users[i].cardId = array[i]["card"] | 0;
    
    const char* name = array[i]["name"];
    strncpy(users[i].name, name, 10);
    users[i].name[10] = 0; // Asegurar terminador null
    
    users[i].department = array[i]["dept"] | 0;
    users[i].group = array[i]["group"] | 0;
    users[i].mode = array[i]["mode"] | 0;
    
    JsonArray fp = array[i]["fp"];
    for (int j = 0; j < 2 && j < fp.size(); j++) {
      users[i].fpStatus[j] = fp[j];
    }
    
    users[i].special = array[i]["special"] | 0;
    users[i].isActive = array[i]["active"] | true;
  }
  
  file.close();
}

void saveUsers() {
  DynamicJsonDocument doc(10000);
  doc["count"] = userCount;
  JsonArray array = doc.createNestedArray("users");
  
  for (int i = 0; i < userCount; i++) {
    JsonObject obj = array.createNestedObject();
    
    JsonArray id = obj.createNestedArray("id");
    for (int j = 0; j < 5; j++) {
      id.add(users[i].id[j]);
    }
    
    JsonArray pwd = obj.createNestedArray("pwd");
    for (int j = 0; j < 3; j++) {
      pwd.add(users[i].password[j]);
    }
    
    obj["card"] = users[i].cardId;
    obj["name"] = users[i].name;
    obj["dept"] = users[i].department;
    obj["group"] = users[i].group;
    obj["mode"] = users[i].mode;
    
    JsonArray fp = obj.createNestedArray("fp");
    for (int j = 0; j < 2; j++) {
      fp.add(users[i].fpStatus[j]);
    }
    
    obj["special"] = users[i].special;
    obj["active"] = users[i].isActive;
  }
  
  File file = SPIFFS.open("/users.json", "w");
  if (!file) {
    Serial.println("Error al crear archivo de usuarios");
    return;
  }
  
  serializeJson(doc, file);
  file.close();
}

void loadRecords() {
  File file = SPIFFS.open("/records.json", "r");
  if (!file) {
    Serial.println("No hay archivo de registros");
    return;
  }
  
  DynamicJsonDocument doc(20000);
  DeserializationError error = deserializeJson(doc, file);
  if (error) {
    Serial.println("Error al leer registros");
    return;
  }
  
  recordCount = doc["count"] | 0;
  newRecordCount = doc["new"] | 0;
  JsonArray array = doc["records"];
  
  for (int i = 0; i < recordCount && i < 500 && i < array.size(); i++) {
    JsonArray id = array[i]["id"];
    for (int j = 0; j < 5 && j < id.size(); j++) {
      records[i].id[j] = id[j];
    }
    
    records[i].timestamp = array[i]["time"] | 0;
    records[i].backup = array[i]["backup"] | 0;
    records[i].recordType = array[i]["type"] | 0;
    
    JsonArray work = array[i]["work"];
    for (int j = 0; j < 3 && j < work.size(); j++) {
      records[i].workCode[j] = work[j];
    }
  }
  
  file.close();
}

void saveRecords() {
  DynamicJsonDocument doc(20000);
  doc["count"] = recordCount;
  doc["new"] = newRecordCount;
  JsonArray array = doc.createNestedArray("records");
  
  for (int i = 0; i < recordCount; i++) {
    JsonObject obj = array.createNestedObject();
    
    JsonArray id = obj.createNestedArray("id");
    for (int j = 0; j < 5; j++) {
      id.add(records[i].id[j]);
    }
    
    obj["time"] = records[i].timestamp;
    obj["backup"] = records[i].backup;
    obj["type"] = records[i].recordType;
    
    JsonArray work = obj.createNestedArray("work");
    for (int j = 0; j < 3; j++) {
      work.add(records[i].workCode[j]);
    }
  }
  
  File file = SPIFFS.open("/records.json", "w");
  if (!file) {
    Serial.println("Error al crear archivo de registros");
    return;
  }
  
  serializeJson(doc, file);
  file.close();
}

#endif // ALMACENAMIENTO_H
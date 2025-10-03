/**
 * web.h
 * Funciones para el servidor web del emulador Anviz
 * Con soporte UTF-8 para caracteres internacionales
 */


// Prototipos de funciones de utilidad
String uint64ToString(uint64_t input);
int findUserById(uint8_t* id);

// Variable para manejo de subida de archivos
File uploadFile;

// Declaracion de funciones externas necesarias de utilidades.h
extern String getFormattedDateTime();
extern String formatTimestamp(uint32_t timestamp);
extern void saveWebAuth();
extern void saveRecords();

// ========= FUNCIONES DE UTILIDAD ===========

// Funcion para convertir uint64_t a String
String uint64ToString(uint64_t input) {
  String result = "";
  if (input == 0) {
    return "0";
  }
  while (input > 0) {
    result = String(input % 10) + result;
    input /= 10;
  }
  return result;
}

// Funcion para buscar un usuario por su ID de 5 bytes
int findUserById(uint8_t* id) {
  for (int i = 0; i < userCount; i++) {
    if (memcmp(users[i].id, id, 5) == 0) {
      return i;
    }
  }
  return -1;
}

// Funcion para verificar la autenticacion
bool isAuthenticated() {
  if (webServer.hasHeader("Authorization")) {
    if (webServer.authenticate(web_user, web_pass)) {
      return true;
    }
  }
  webServer.requestAuthentication();
  return false;
}

// ========= FUNCIONES DEL SERVIDOR WEB ===========

// Pagina principal
void handleRoot() {
  if (!isAuthenticated()) return;
  String html = "<!DOCTYPE html><html><head><title>Emulador Anviz</title>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; margin: 0; padding: 20px; color: #333; }";
  html += "h1 { color: #0066cc; }";
  html += ".menu { background-color: #f0f0f0; padding: 10px; margin-bottom: 20px; }";
  html += ".menu a { margin-right: 10px; color: #0066cc; text-decoration: none; padding: 5px; }";
  html += ".menu a:hover { background-color: #e0e0e0; }";
  html += ".status { background-color: #e0f7fa; padding: 15px; border-radius: 5px; }";
  html += "table { border-collapse: collapse; width: 100%; }";
  html += "th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }";
  html += "th { background-color: #f2f2f2; }";
  html += "</style>";
  html += "</head><body>";
  html += "<h1>Emulador de Dispositivo Anviz</h1>";
  html += "<div class='menu'>";
  html += "<a href='/'>Inicio</a>";
  html += "<a href='/users'>Usuarios</a>";
  html += "<a href='/records'>Registros</a>";
  html += "<a href='/settings'>Configuracion</a>";
  html += "<a href='/auth'>Seguridad</a>";
  html += "</div>";
  html += "<div class='status'";
  html += "<h2>Estado del Sistema</h2>";
  html += "<p>Direccion IP: " + WiFi.localIP().toString() + "</p>";
  html += "<p>Fuerza de senal WiFi: " + String(WiFi.RSSI()) + " dBm</p>";
  html += "<p>Usuarios registrados: " + String(userCount) + "</p>";
  html += "<p>Registros de acceso: " + String(recordCount) + " (nuevos: " + String(newRecordCount) + ")</p>";
  html += "<p>Fecha y hora: " + getFormattedDateTime() + "</p>";
  html += "</div>";
  html += "<div><h2>Operaciones</h2>";
  html += "<p><a href='/changewifi' onclick='return confirm(\"Esta seguro de querer cambiar la red WiFi? Se borrara la configuracion actual y el dispositivo se reiniciara en modo de configuracion.\");'>Cambiar Red WiFi</a></p>";
  html += "<p><a href='/clearlogs' onclick='return confirm(\"Esta seguro de borrar todos los registros?\");'>Borrar registros</a></p>";
  html += "<p><a href='/reset' onclick='return confirm(\"Esta seguro de reiniciar el dispositivo?\");'>Reiniciar dispositivo</a></p>";
  html += "</div>";
  html += "</body></html>";
  
  webServer.sendHeader("Content-Type", "text/html; charset=UTF-8");
  webServer.send(200, "text/html", html);
}

// Pagina de usuarios
void handleUsers() {
  if (!isAuthenticated()) return;
  String html = "<!DOCTYPE html><html><head><title>Usuarios - Emulador Anviz</title>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; margin: 0; padding: 20px; color: #333; }";
  html += "h1 { color: #0066cc; }";
  html += ".menu { background-color: #f0f0f0; padding: 10px; margin-bottom: 20px; }";
  html += ".menu a { margin-right: 10px; color: #0066cc; text-decoration: none; padding: 5px; }";
  html += ".menu a:hover { background-color: #e0e0e0; }";
  html += "table { border-collapse: collapse; width: 100%; }";
  html += "th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }";
  html += "th { background-color: #f2f2f2; }";
  html += "</style>";
  html += "</head><body>";
  html += "<h1>Usuarios Registrados</h1>";
  html += "<div class='menu'>";
  html += "<a href='/'>Inicio</a>";
  html += "<a href='/users'>Usuarios</a>";
  html += "<a href='/records'>Registros</a>";
  html += "<a href='/settings'>Configuracion</a>";
  html += "<a href='/auth'>Seguridad</a>";
  html += "</div>";
  
  html += "<table>";
  html += "<tr><th>ID</th><th>Nombre</th><th>ID de Tarjeta</th><th>Departamento</th><th>Estado</th></tr>";
  
  for (int i = 0; i < userCount; i++) {
    html += "<tr>";
    
    uint64_t userId_dec = 0;
    for (int j = 0; j < 5; j++) {
      userId_dec = (userId_dec << 8) | users[i].id[j];
    }
    html += "<td>" + uint64ToString(userId_dec) + "</td>";
    
    String userName = "";
    for (int k = 0; k < 10 && users[i].name[k] != '\0'; k++) {
      if (users[i].name[k] >= 32 && users[i].name[k] < 127) {
        userName += users[i].name[k];
      }
    }
    html += "<td>" + userName + "</td>";
    
    html += "<td>" + String(users[i].cardId) + "</td>";
    
    html += "<td>" + String(users[i].department) + "</td>";
    
    html += "<td>" + String(users[i].isActive ? "Activo" : "Inactivo") + "</td>";
    
    html += "</tr>";
  }
  
  html += "</table>";
  
  if (userCount == 0) {
    html += "<p>No hay usuarios registrados. Utilice el software Anviz CrossChex para anadir usuarios.</p>";
  }
  
  html += "</body></html>";
  
  webServer.sendHeader("Content-Type", "text/html; charset=UTF-8");
  webServer.send(200, "text/html", html);
}

// Pagina de registros de acceso
void handleRecords() {
  if (!isAuthenticated()) return;
  String html = "<!DOCTYPE html><html><head><title>Registros - Emulador Anviz</title>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; margin: 0; padding: 20px; color: #333; }";
  html += "h1 { color: #0066cc; }";
  html += ".menu { background-color: #f0f0f0; padding: 10px; margin-bottom: 20px; }";
  html += ".menu a { margin-right: 10px; color: #0066cc; text-decoration: none; padding: 5px; }";
  html += ".menu a:hover { background-color: #e0e0e0; }";
  html += "table { border-collapse: collapse; width: 100%; }";
  html += "th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }";
  html += "th { background-color: #f2f2f2; }";
  html += "</style>";
  html += "</head><body>";
  html += "<h1>Registros de Acceso</h1>";
  html += "<div class='menu'>";
  html += "<a href='/'>Inicio</a>";
  html += "<a href='/users'>Usuarios</a>";
  html += "<a href='/records'>Registros</a>";
  html += "<a href='/settings'>Configuracion</a>";
  html += "<a href='/auth'>Seguridad</a>";
  html += "</div>";
  
  html += "<table>";
  html += "<tr><th>ID Usuario</th><th>Nombre</th><th>Fecha/Hora</th><th>Tipo</th><th>Metodo</th></tr>";
  
  int startIdx = (recordCount > 50) ? (recordCount - 50) : 0;
  int count = (recordCount > 50) ? 50 : recordCount;
  
  for (int i = 0; i < count; i++) {
    int idx = startIdx + i;
    html += "<tr>";
    
    uint64_t userId_dec = 0;
    for (int j = 0; j < 5; j++) {
      userId_dec = (userId_dec << 8) | records[idx].id[j];
    }
    html += "<td>" + uint64ToString(userId_dec) + "</td>";

    int userIndex = findUserById(records[idx].id);
    String userName = (userIndex != -1) ? String(users[userIndex].name) : "Desconocido";
    String sanitizedUserName = "";
    for (int k = 0; k < 10 && userName[k] != '\0'; k++) {
      if (userName[k] >= 32 && userName[k] < 127) {
        sanitizedUserName += userName[k];
      }
    }
    html += "<td>" + sanitizedUserName + "</td>";
    
    html += "<td>" + formatTimestamp(records[idx].timestamp) + "</td>";
    
    String recordType;
    if (records[idx].recordType & 0x80) {
      recordType = "Entrada";
    } else {
      recordType = "Salida";
    }
    html += "<td>" + recordType + "</td>";
    
    String method;
    switch (records[idx].backup) {
      case 0x01: method = "Contrasena"; break;
      case 0x02: method = "Huella digital"; break;
      case 0x08: method = "Tarjeta"; break;
      default: method = "Otro"; break;
    }
    html += "<td>" + method + "</td>";
    
    html += "</tr>";
  }
  
  html += "</table>";
  
  if (recordCount == 0) {
    html += "<p>No hay registros de acceso. Los registros se generaran cuando los usuarios utilicen sus tarjetas.</p>";
  } else if (recordCount > 50) {
    html += "<p>Mostrando los ultimos 50 registros de un total de " + String(recordCount) + ".</p>";
  }
  
  html += "<p><a href='/clearlogs' onclick='return confirm(\"Esta seguro de borrar todos los registros?\");'>Borrar todos los registros</a></p>";
  
  html += "</body></html>";
  
  webServer.sendHeader("Content-Type", "text/html; charset=UTF-8");
  webServer.send(200, "text/html", html);
}

// Pagina de configuracion
void handleSettings() {
  if (!isAuthenticated()) return;
  String html = "<!DOCTYPE html><html><head><title>Configuracion - Emulador Anviz</title>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; margin: 0; padding: 20px; color: #333; }";
  html += "h1 { color: #0066cc; }";
  html += ".menu { background-color: #f0f0f0; padding: 10px; margin-bottom: 20px; }";
  html += ".menu a { margin-right: 10px; color: #0066cc; text-decoration: none; padding: 5px; }";
  html += ".menu a:hover { background-color: #e0e0e0; }";
  html += ".config { background-color: #f9f9f9; padding: 15px; border-radius: 5px; }";
  html += "table { border-collapse: collapse; width: 100%; }";
  html += "th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }";
  html += "th { background-color: #f2f2f2; width: 40%; }";
  html += "</style>";
  html += "</head><body>";
  html += "<h1>Configuracion del Emulador</h1>";
  html += "<div class='menu'>";
  html += "<a href='/'>Inicio</a>";
  html += "<a href='/users'>Usuarios</a>";
  html += "<a href='/records'>Registros</a>";
  html += "<a href='/settings'>Configuracion</a>";
  html += "<a href='/auth'>Seguridad</a>";
  html += "</div>";
  html += "<div class='config'";
  
  html += "<form action='/savesettings' method='post'>";
  html += "<table>";
  html += "<tr><th>Parametro</th><th>Valor</th></tr>";
  html += "<tr><td>ID del dispositivo</td><td><input type='text' name='deviceId' value='" + String(deviceId) + "'></td></tr>";
  html += "<tr><td>Version de firmware</td><td>" + String(basicConfig.firmwareVersion) + "</td></tr>";
  html += "<tr><td colspan='2' style='background-color:#e0f7fa;'><b>Configuracion de Pines GPIO</b></td></tr>";
  html += "<tr><td>Pin Wiegand D0</td><td><input type='text' name='pin_d0' value='" + String(basicConfig.pin_d0) + "'></td></tr>";
  html += "<tr><td>Pin Wiegand D1</td><td><input type='text' name='pin_d1' value='" + String(basicConfig.pin_d1) + "'></td></tr>";
  html += "<tr><td>Pin Rele</td><td><input type='text' name='pin_relay' value='" + String(basicConfig.pin_relay) + "'></td></tr>";
  html += "<tr><td>Pin LED de Estado</td><td><input type='text' name='pin_led' value='" + String(basicConfig.pin_led) + "'></td></tr>";
  html += "<tr><td colspan='2' style='background-color:#e0f7fa;'><b>Otros Parametros</b></td></tr>";
  html += "<tr><td>Numero de serie</td><td>" + String(serialNumber) + "</td></tr>";
  html += "<tr><td>Volumen</td><td>" + String(basicConfig.volume) + "</td></tr>";
  
  String languageName;
  switch(basicConfig.language) {
    case 0: languageName = "Chino Simplificado"; break;
    case 1: languageName = "Chino Tradicional"; break;
    case 2: languageName = "Ingles"; break;
    case 3: languageName = "Frances"; break;
    case 4: languageName = "Espanol"; break;
    case 5: languageName = "Portugues"; break;
    default: languageName = String(basicConfig.language); break;
  }
  html += "<tr><td>Idioma</td><td>" + languageName + "</td></tr>";
  
  String dateFormat;
  uint8_t dateFormatType = (basicConfig.dateFormat >> 4) & 0x0F;
  uint8_t timeFormatType = basicConfig.dateFormat & 0x0F;
  
  switch(dateFormatType) {
    case 0: dateFormat = "Formato Chino (YYYY-MM-DD)"; break;
    case 1: dateFormat = "Formato Americano (MM/DD/YYYY)"; break;
    case 2: dateFormat = "Formato Europeo (DD/MM/YYYY)"; break;
    default: dateFormat = "Desconocido"; break;
  }
  
  dateFormat += ", ";
  dateFormat += (timeFormatType == 0) ? "24 horas" : "12 horas (AM/PM)";
  
  html += "<tr><td>Formato de fecha</td><td>" + dateFormat + "</td></tr>";
  
  FSInfo fsInfo;
  if (SPIFFS.info(fsInfo)) {
    html += "<tr><td>Espacio SPIFFS utilizado</td><td>" + String(fsInfo.usedBytes) + " / " + String(fsInfo.totalBytes) + " bytes</td></tr>";
  }
  
  html += "</table>";
  html += "<br><input type='submit' value='Guardar y Reiniciar'>";
  html += "</form>";
  html += "<hr><p><a href='/changewifi' onclick='return confirm(\"Esta seguro de querer cambiar la red WiFi? Se borrara la configuracion actual y el dispositivo se reiniciara en modo de configuracion.\");'>Cambiar Red WiFi</a></p>";
  
  html += "</div>";
  html += "</body></html>";
  
  webServer.sendHeader("Content-Type", "text/html; charset=UTF-8");
  webServer.send(200, "text/html", html);
}

// Página para cambiar la autenticación
void handleAuth() {
  if (!isAuthenticated()) return;
  String html = "<!DOCTYPE html><html><head><title>Seguridad - Emulador Anviz</title>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; margin: 0; padding: 20px; color: #333; }";
  html += "h1 { color: #0066cc; }";
  html += ".menu { background-color: #f0f0f0; padding: 10px; margin-bottom: 20px; }";
  html += ".menu a { margin-right: 10px; color: #0066cc; text-decoration: none; padding: 5px; }";
  html += ".menu a:hover { background-color: #e0e0e0; }";
  html += ".config { background-color: #f9f9f9; padding: 15px; border-radius: 5px; }";
  html += "input[type=text], input[type=password] { width: 100%; padding: 8px; margin: 4px 0; display: inline-block; border: 1px solid #ccc; border-radius: 4px; box-sizing: border-box; }";
  html += "input[type=submit] { background-color: #0066cc; color: white; padding: 10px 15px; border: none; border-radius: 4px; cursor: pointer; }";
  html += "</style>";
  html += "</head><body>";
  html += "<h1>Configuración de Seguridad</h1>";
  html += "<div class='menu'>";
  html += "<a href='/'>Inicio</a>";
  html += "<a href='/users'>Usuarios</a>";
  html += "<a href='/records'>Registros</a>";
  html += "<a href='/settings'>Configuracion</a>";
  html += "<a href='/auth'>Seguridad</a>";
  html += "</div>";
  html += "<div class='config'>";
  html += "<h2>Cambiar Credenciales de Acceso Web</h2>";
  html += "<form action='/saveauth' method='post'>";
  html += "<label for='user'>Usuario:</label>";
  html += "<input type='text' id='user' name='user' value='" + String(web_user) + "' required>";
  html += "<label for='pass'>Nueva Contraseña:</label>";
  html += "<input type='password' id='pass' name='pass' required>";
  html += "<br><br><input type='submit' value='Guardar Credenciales'>";
  html += "</form>";
  html += "</div>";
  html += "</body></html>";

  webServer.sendHeader("Content-Type", "text/html; charset=UTF-8");
  webServer.send(200, "text/html", html);
}

// Manejar guardado de configuracion
void handleSaveSettings() {
  if (!isAuthenticated()) return;
  if (webServer.hasArg("deviceId")) {
    deviceId = webServer.arg("deviceId").toInt();
    saveConfig();
  }
  if (webServer.hasArg("pin_d0")) {
    basicConfig.pin_d0 = webServer.arg("pin_d0").toInt();
    basicConfig.pin_d1 = webServer.arg("pin_d1").toInt();
    basicConfig.pin_relay = webServer.arg("pin_relay").toInt();
    basicConfig.pin_led = webServer.arg("pin_led").toInt();
    saveConfig();
  } else {
    // Si no se envían los pines, es posible que sea un guardado desde una versión anterior.
  }
  webServer.sendHeader("Location", "/settings");
  webServer.send(303);
  delay(1000);
  ESP.restart();
}

// Manejar guardado de credenciales
void handleSaveAuth() {
  if (!isAuthenticated()) return;
  if (webServer.hasArg("user") && webServer.hasArg("pass")) {
    String user = webServer.arg("user");
    String pass = webServer.arg("pass");
    if (user.length() > 0 && user.length() < sizeof(web_user) && pass.length() > 0 && pass.length() < sizeof(web_pass)) {
      user.toCharArray(web_user, sizeof(web_user));
      pass.toCharArray(web_pass, sizeof(web_pass));
      saveWebAuth();
      webServer.send(200, "text/plain", "Credenciales guardadas. Se recomienda reiniciar el dispositivo.");
    } else {
      webServer.send(400, "text/plain", "Usuario o contraseña demasiado largos.");
    }
  }
}

// Manejar cambio de WiFi
void handleWifiChange() {
  if (!isAuthenticated()) return;
  WiFiManager wifiManager;
  wifiManager.resetSettings();
  ESP.restart();
}

// Manejar borrado de registros
void handleClearLogs() {
  if (!isAuthenticated()) return;
  recordCount = 0;
  newRecordCount = 0;
  saveRecords();
  
  webServer.sendHeader("Location", "/records");
  webServer.send(303);
}

// Manejar reinicio del dispositivo
void handleReset() {
  if (!isAuthenticated()) return;
  webServer.sendHeader("Content-Type", "text/html; charset=UTF-8");
  webServer.send(200, "text/html", "<!DOCTYPE html><html><head><meta charset='UTF-8'><meta http-equiv='refresh' content='5;url=/'></head><body><h2>Reiniciando el dispositivo...</h2><p>Volviendo al inicio en 5 segundos.</p></body></html>");
  delay(1000);
  ESP.restart();
}

// Manejar carga de archivos
void handleFileUpload() {
  if (!isAuthenticated()) return;
  HTTPUpload& upload = webServer.upload();
  
  if (upload.status == UPLOAD_FILE_START) {
    String filename = "/upload_" + upload.filename;
    Serial.println("Iniciando carga: " + filename);
    
    File file = SPIFFS.open(filename, "w");
    if (!file) {
      Serial.println("Error al abrir el archivo para escritura");
      return;
    }
    
    uploadFile = file;
  }
  else if (upload.status == UPLOAD_FILE_WRITE) {
    if (uploadFile) {
      uploadFile.write(upload.buf, upload.currentSize);
    }
  }
  else if (upload.status == UPLOAD_FILE_END) {
    if (uploadFile) {
      uploadFile.close();
      Serial.println("Carga completada: " + String(upload.totalSize) + " bytes");
    }
  }
  else if (upload.status == UPLOAD_FILE_ABORTED) {
    Serial.println("Carga abortada");
    if (uploadFile) {
      uploadFile.close();
    }
  }
}
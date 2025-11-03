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

  webServer.sendHeader("Content-Type", "text/html; charset=UTF-8");
  webServer.setContentLength(CONTENT_LENGTH_UNKNOWN);

  // Send header
  webServer.sendContent_P(PSTR("<!DOCTYPE html><html><head><title>Emulador Anviz</title>"));
  webServer.sendContent_P(PSTR("<meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1'>"));
  webServer.sendContent_P(PSTR("<style>body{font-family:Arial,sans-serif;margin:0;padding:20px;color:#333}h1{color:#0066cc}.menu{background-color:#f0f0f0;padding:10px;margin-bottom:20px}.menu a{margin-right:10px;color:#0066cc;text-decoration:none;padding:5px}.menu a:hover{background-color:#e0e0e0}.status{background-color:#e0f7fa;padding:15px;border-radius:5px}table{border-collapse:collapse;width:100%}th,td{border:1px solid #ddd;padding:8px;text-align:left}th{background-color:#f2f2f2}</style>"));
  webServer.sendContent_P(PSTR("</head><body><h1>Emulador de Dispositivo Anviz</h1><div class='menu'><a href='/'>Inicio</a><a href='/users'>Usuarios</a><a href='/records'>Registros</a><a href='/settings'>Configuracion</a><a href='/auth'>Seguridad</a></div>"));

  // Send status
  char buffer[256];
  webServer.sendContent_P(PSTR("<div class='status'><h2>Estado del Sistema</h2>"));
  snprintf_P(buffer, sizeof(buffer), PSTR("<p>Direccion IP: %s</p>"), WiFi.localIP().toString().c_str());
  webServer.sendContent(buffer);
  snprintf_P(buffer, sizeof(buffer), PSTR("<p>Fuerza de senal WiFi: %d dBm</p>"), WiFi.RSSI());
  webServer.sendContent(buffer);
  snprintf_P(buffer, sizeof(buffer), PSTR("<p>SSID: %s</p>"), WiFi.SSID().c_str());
  webServer.sendContent(buffer);
  snprintf_P(buffer, sizeof(buffer), PSTR("<p>RAM Libre: %.2f KB</p>"), (float)ESP.getFreeHeap() / 1024.0);
  webServer.sendContent(buffer);
  FSInfo fsInfo;
  SPIFFS.info(fsInfo);
  snprintf_P(buffer, sizeof(buffer), PSTR("<p>Flash (SPIFFS) Libre: %.2f / %.2f MB</p>"), (float)(fsInfo.totalBytes - fsInfo.usedBytes) / (1024.0 * 1024.0), (float)fsInfo.totalBytes / (1024.0 * 1024.0));
  webServer.sendContent(buffer);
  snprintf_P(buffer, sizeof(buffer), PSTR("<p>Usuarios registrados: %d</p>"), userCount);
  webServer.sendContent(buffer);
  snprintf_P(buffer, sizeof(buffer), PSTR("<p>Registros de acceso: %d (nuevos: %d)</p>"), recordCount, newRecordCount);
  webServer.sendContent(buffer);
  snprintf_P(buffer, sizeof(buffer), PSTR("<p>Fecha y hora: %s</p>"), getFormattedDateTime().c_str());
  webServer.sendContent(buffer);
  webServer.sendContent_P(PSTR("</div>"));

  // Send operations
  webServer.sendContent_P(PSTR("<div><h2>Operaciones</h2><p><a href='/changewifi' onclick='return confirm(\"Esta seguro de querer cambiar la red WiFi? Se borrara la configuracion actual y el dispositivo se reiniciara en modo de configuracion.\");'>Cambiar Red WiFi</a></p>"));
  webServer.sendContent_P(PSTR("<p><a href='/clearlogs' onclick='return confirm(\"Esta seguro de borrar todos los registros?\");'>Borrar registros</a></p>"));
  webServer.sendContent_P(PSTR("<p><a href='/reset' onclick='return confirm(\"Esta seguro de reiniciar el dispositivo?\");'>Reiniciar dispositivo</a></p></div>"));

  // Send footer
  webServer.sendContent_P(PSTR("</body></html>"));
  webServer.sendContent(""); // Terminate the connection
}

// Pagina de usuarios
void handleUsers() {
  if (!isAuthenticated()) return;

  webServer.sendHeader("Content-Type", "text/html; charset=UTF-8");
  webServer.setContentLength(CONTENT_LENGTH_UNKNOWN);

  // Send header
  webServer.sendContent_P(PSTR("<!DOCTYPE html><html><head><title>Usuarios - Emulador Anviz</title>"));
  webServer.sendContent_P(PSTR("<meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1'>"));
  webServer.sendContent_P(PSTR("<style>body{font-family:Arial,sans-serif;margin:0;padding:20px;color:#333}h1{color:#0066cc}.menu{background-color:#f0f0f0;padding:10px;margin-bottom:20px}.menu a{margin-right:10px;color:#0066cc;text-decoration:none;padding:5px}.menu a:hover{background-color:#e0e0e0}table{border-collapse:collapse;width:100%}th,td{border:1px solid #ddd;padding:8px;text-align:left}th{background-color:#f2f2f2}</style>"));
  webServer.sendContent_P(PSTR("</head><body><h1>Usuarios Registrados</h1><div class='menu'><a href='/'>Inicio</a><a href='/users'>Usuarios</a><a href='/records'>Registros</a><a href='/settings'>Configuracion</a><a href='/auth'>Seguridad</a></div>"));

  // Send table header
  webServer.sendContent_P(PSTR("<table><tr><th>ID</th><th>Nombre</th><th>ID de Tarjeta</th><th>Departamento</th><th>Estado</th></tr>"));

  // Send table rows
  char buffer[256];
  for (int i = 0; i < userCount; i++) {
    webServer.sendContent_P(PSTR("<tr>"));

    uint64_t userId_dec = 0;
    for (int j = 0; j < 5; j++) {
      userId_dec = (userId_dec << 8) | users[i].id[j];
    }
    snprintf_P(buffer, sizeof(buffer), PSTR("<td>%s</td>"), uint64ToString(userId_dec).c_str());
    webServer.sendContent(buffer);

    snprintf_P(buffer, sizeof(buffer), PSTR("<td>%.10s</td>"), users[i].name);
    webServer.sendContent(buffer);

    snprintf_P(buffer, sizeof(buffer), PSTR("<td>%u</td>"), users[i].cardId);
    webServer.sendContent(buffer);

    snprintf_P(buffer, sizeof(buffer), PSTR("<td>%d</td>"), users[i].department);
    webServer.sendContent(buffer);

    snprintf_P(buffer, sizeof(buffer), PSTR("<td>%s</td>"), users[i].isActive ? "Activo" : "Inactivo");
    webServer.sendContent(buffer);

    webServer.sendContent_P(PSTR("</tr>"));
  }

  webServer.sendContent_P(PSTR("</table>"));

  if (userCount == 0) {
    webServer.sendContent_P(PSTR("<p>No hay usuarios registrados. Utilice el software Anviz CrossChex para anadir usuarios.</p>"));
  }

  // Send footer
  webServer.sendContent_P(PSTR("</body></html>"));
  webServer.sendContent(""); // Terminate the connection
}

// Pagina de registros de acceso
void handleRecords() {
  if (!isAuthenticated()) return;

  webServer.sendHeader("Content-Type", "text/html; charset=UTF-8");
  webServer.setContentLength(CONTENT_LENGTH_UNKNOWN);

  // Send header
  webServer.sendContent_P(PSTR("<!DOCTYPE html><html><head><title>Registros - Emulador Anviz</title>"));
  webServer.sendContent_P(PSTR("<meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1'>"));
  webServer.sendContent_P(PSTR("<style>body{font-family:Arial,sans-serif;margin:0;padding:20px;color:#333}h1{color:#0066cc}.menu{background-color:#f0f0f0;padding:10px;margin-bottom:20px}.menu a{margin-right:10px;color:#0066cc;text-decoration:none;padding:5px}.menu a:hover{background-color:#e0e0e0}table{border-collapse:collapse;width:100%}th,td{border:1px solid #ddd;padding:8px;text-align:left}th{background-color:#f2f2f2}</style>"));
  webServer.sendContent_P(PSTR("</head><body><h1>Registros de Acceso</h1><div class='menu'><a href='/'>Inicio</a><a href='/users'>Usuarios</a><a href='/records'>Registros</a><a href='/settings'>Configuracion</a><a href='/auth'>Seguridad</a></div>"));

  // Send table header
  webServer.sendContent_P(PSTR("<table><tr><th>ID Usuario</th><th>Nombre</th><th>Fecha/Hora</th><th>Tipo</th><th>Metodo</th></tr>"));

  // Send table rows
  char buffer[256];
  int startIdx = (recordCount > 50) ? (recordCount - 50) : 0;
  int count = (recordCount > 50) ? 50 : recordCount;

  for (int i = 0; i < count; i++) {
    int idx = startIdx + i;
    webServer.sendContent_P(PSTR("<tr>"));

    uint64_t userId_dec = 0;
    for (int j = 0; j < 5; j++) {
      userId_dec = (userId_dec << 8) | records[idx].id[j];
    }
    snprintf_P(buffer, sizeof(buffer), PSTR("<td>%s</td>"), uint64ToString(userId_dec).c_str());
    webServer.sendContent(buffer);

    int userIndex = findUserById(records[idx].id);
    const char* userName = (userIndex != -1) ? users[userIndex].name : "Desconocido";
    snprintf_P(buffer, sizeof(buffer), PSTR("<td>%.10s</td>"), userName);
    webServer.sendContent(buffer);

    snprintf_P(buffer, sizeof(buffer), PSTR("<td>%s</td>"), formatTimestamp(records[idx].timestamp).c_str());
    webServer.sendContent(buffer);

    const char* recordType = (records[idx].recordType & 0x80) ? "Entrada" : "Salida";
    snprintf_P(buffer, sizeof(buffer), PSTR("<td>%s</td>"), recordType);
    webServer.sendContent(buffer);

    const char* method;
    switch (records[idx].backup) {
      case 0x01: method = "Contrasena"; break;
      case 0x02: method = "Huella digital"; break;
      case 0x08: method = "Tarjeta"; break;
      default: method = "Otro"; break;
    }
    snprintf_P(buffer, sizeof(buffer), PSTR("<td>%s</td>"), method);
    webServer.sendContent(buffer);

    webServer.sendContent_P(PSTR("</tr>"));
  }

  webServer.sendContent_P(PSTR("</table>"));

  if (recordCount == 0) {
    webServer.sendContent_P(PSTR("<p>No hay registros de acceso. Los registros se generaran cuando los usuarios utilicen sus tarjetas.</p>"));
  } else if (recordCount > 50) {
    snprintf_P(buffer, sizeof(buffer), PSTR("<p>Mostrando los ultimos 50 registros de un total de %d.</p>"), recordCount);
    webServer.sendContent(buffer);
  }

  webServer.sendContent_P(PSTR("<p><a href='/clearlogs' onclick='return confirm(\"Esta seguro de borrar todos los registros?\");'>Borrar todos los registros</a></p>"));

  // Send footer
  webServer.sendContent_P(PSTR("</body></html>"));
  webServer.sendContent(""); // Terminate the connection
}

// Pagina de configuracion
void handleSettings() {
  if (!isAuthenticated()) return;

  webServer.sendHeader("Content-Type", "text/html; charset=UTF-8");
  webServer.setContentLength(CONTENT_LENGTH_UNKNOWN);

  // Send header
  webServer.sendContent_P(PSTR("<!DOCTYPE html><html><head><title>Configuracion - Emulador Anviz</title>"));
  webServer.sendContent_P(PSTR("<meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1'>"));
  webServer.sendContent_P(PSTR("<style>body{font-family:Arial,sans-serif;margin:0;padding:20px;color:#333}h1{color:#0066cc}.menu{background-color:#f0f0f0;padding:10px;margin-bottom:20px}.menu a{margin-right:10px;color:#0066cc;text-decoration:none;padding:5px}.menu a:hover{background-color:#e0e0e0}.config{background-color:#f9f9f9;padding:15px;border-radius:5px}table{border-collapse:collapse;width:100%}th,td{border:1px solid #ddd;padding:8px;text-align:left}th{background-color:#f2f2f2;width:40%}</style>"));
  webServer.sendContent_P(PSTR("</head><body><h1>Configuracion del Emulador</h1><div class='menu'><a href='/'>Inicio</a><a href='/users'>Usuarios</a><a href='/records'>Registros</a><a href='/settings'>Configuracion</a><a href='/auth'>Seguridad</a></div>"));

  // Send form
  webServer.sendContent_P(PSTR("<div class='config'><form action='/savesettings' method='post'><table>"));
  webServer.sendContent_P(PSTR("<tr><th>Parametro</th><th>Valor</th></tr>"));

  char buffer[256];
  snprintf_P(buffer, sizeof(buffer), PSTR("<tr><td>ID del dispositivo</td><td><input type='text' name='deviceId' value='%u'></td></tr>"), deviceId);
  webServer.sendContent(buffer);
  snprintf_P(buffer, sizeof(buffer), PSTR("<tr><td>Version de firmware</td><td>%s</td></tr>"), basicConfig.firmwareVersion);
  webServer.sendContent(buffer);

  webServer.sendContent_P(PSTR("<tr><td colspan='2' style='background-color:#e0f7fa;'><b>Configuracion de Pines GPIO</b></td></tr>"));
  snprintf_P(buffer, sizeof(buffer), PSTR("<tr><td>Pin Wiegand D0</td><td><input type='text' name='pin_d0' value='%d'></td></tr>"), basicConfig.pin_d0);
  webServer.sendContent(buffer);
  snprintf_P(buffer, sizeof(buffer), PSTR("<tr><td>Pin Wiegand D1</td><td><input type='text' name='pin_d1' value='%d'></td></tr>"), basicConfig.pin_d1);
  webServer.sendContent(buffer);
  snprintf_P(buffer, sizeof(buffer), PSTR("<tr><td>Pin Rele</td><td><input type='text' name='pin_relay' value='%d'></td></tr>"), basicConfig.pin_relay);
  webServer.sendContent(buffer);
  snprintf_P(buffer, sizeof(buffer), PSTR("<tr><td>Pin LED de Estado</td><td><input type='text' name='pin_led' value='%d'></td></tr>"), basicConfig.pin_led);
  webServer.sendContent(buffer);
  snprintf_P(buffer, sizeof(buffer), PSTR("<tr><td>Tiempo de Activacion Rele (ms)</td><td><input type='number' name='relayOnDuration' min='500' max='10000' value='%d'></td></tr>"), basicConfig.relayOnDuration);
  webServer.sendContent(buffer);

  webServer.sendContent_P(PSTR("<tr><td colspan='2' style='background-color:#e0f7fa;'><b>Reinicio Automático</b></td></tr>"));
  snprintf_P(buffer, sizeof(buffer), PSTR("<tr><td>Habilitar</td><td><input type='checkbox' name='rebootEnabled' %s></td></tr>"), basicConfig.rebootEnabled ? "checked" : "");
  webServer.sendContent(buffer);
  snprintf_P(buffer, sizeof(buffer), PSTR("<tr><td>Hora de Reinicio (HH:MM)</td><td><input type='number' name='rebootHour' min='0' max='23' value='%d'>:<input type='number' name='rebootMinute' min='0' max='59' value='%d'></td></tr>"), basicConfig.rebootHour, basicConfig.rebootMinute);
  webServer.sendContent(buffer);

  webServer.sendContent_P(PSTR("<tr><td colspan='2' style='background-color:#e0f7fa;'><b>Otros Parametros</b></td></tr>"));
  snprintf_P(buffer, sizeof(buffer), PSTR("<tr><td>Numero de serie</td><td>%s</td></tr>"), serialNumber);
  webServer.sendContent(buffer);
  snprintf_P(buffer, sizeof(buffer), PSTR("<tr><td>Volumen</td><td>%d</td></tr>"), basicConfig.volume);
  webServer.sendContent(buffer);

  const char* languageName;
  switch(basicConfig.language) {
    case 0: languageName = "Chino Simplificado"; break;
    case 1: languageName = "Chino Tradicional"; break;
    case 2: languageName = "Ingles"; break;
    case 3: languageName = "Frances"; break;
    case 4: languageName = "Espanol"; break;
    case 5: languageName = "Portugues"; break;
    default: languageName = "Desconocido"; break;
  }
  snprintf_P(buffer, sizeof(buffer), PSTR("<tr><td>Idioma</td><td>%s</td></tr>"), languageName);
  webServer.sendContent(buffer);

  const char* dateFormatStr;
  uint8_t dateFormatType = (basicConfig.dateFormat >> 4) & 0x0F;
  uint8_t timeFormatType = basicConfig.dateFormat & 0x0F;
  switch(dateFormatType) {
    case 0: dateFormatStr = "Formato Chino (YYYY-MM-DD)"; break;
    case 1: dateFormatStr = "Formato Americano (MM/DD/YYYY)"; break;
    case 2: dateFormatStr = "Formato Europeo (DD/MM/YYYY)"; break;
    default: dateFormatStr = "Desconocido"; break;
  }
  char fullDateFormat[50];
  snprintf_P(fullDateFormat, sizeof(fullDateFormat), PSTR("%s, %s"), dateFormatStr, (timeFormatType == 0) ? "24 horas" : "12 horas (AM/PM)");
  snprintf_P(buffer, sizeof(buffer), PSTR("<tr><td>Formato de fecha</td><td>%s</td></tr>"), fullDateFormat);
  webServer.sendContent(buffer);

  FSInfo fsInfo;
  if (SPIFFS.info(fsInfo)) {
    snprintf_P(buffer, sizeof(buffer), PSTR("<tr><td>Espacio SPIFFS utilizado</td><td>%d / %d bytes</td></tr>"), fsInfo.usedBytes, fsInfo.totalBytes);
    webServer.sendContent(buffer);
  }

  webServer.sendContent_P(PSTR("</table><br><input type='submit' value='Guardar y Reiniciar'></form>"));
  webServer.sendContent_P(PSTR("<hr><p><a href='/changewifi' onclick='return confirm(\"Esta seguro de querer cambiar la red WiFi? Se borrara la configuracion actual y el dispositivo se reiniciara en modo de configuracion.\");'>Cambiar Red WiFi</a></p></div>"));

  // Send footer
  webServer.sendContent_P(PSTR("</body></html>"));
  webServer.sendContent(""); // Terminate the connection
}

// Página para cambiar la autenticación
void handleAuth() {
  if (!isAuthenticated()) return;

  webServer.sendHeader("Content-Type", "text/html; charset=UTF-8");
  webServer.setContentLength(CONTENT_LENGTH_UNKNOWN);

  // Send header
  webServer.sendContent_P(PSTR("<!DOCTYPE html><html><head><title>Seguridad - Emulador Anviz</title>"));
  webServer.sendContent_P(PSTR("<meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1'>"));
  webServer.sendContent_P(PSTR("<style>body{font-family:Arial,sans-serif;margin:0;padding:20px;color:#333}h1{color:#0066cc}.menu{background-color:#f0f0f0;padding:10px;margin-bottom:20px}.menu a{margin-right:10px;color:#0066cc;text-decoration:none;padding:5px}.menu a:hover{background-color:#e0e0e0}.config{background-color:#f9f9f9;padding:15px;border-radius:5px}input[type=text],input[type=password]{width:100%;padding:8px;margin:4px 0;display:inline-block;border:1px solid #ccc;border-radius:4px;box-sizing:border-box}input[type=submit]{background-color:#0066cc;color:white;padding:10px 15px;border:none;border-radius:4px;cursor:pointer}</style>"));
  webServer.sendContent_P(PSTR("</head><body><h1>Configuración de Seguridad</h1><div class='menu'><a href='/'>Inicio</a><a href='/users'>Usuarios</a><a href='/records'>Registros</a><a href='/settings'>Configuracion</a><a href='/auth'>Seguridad</a></div>"));

  // Send form
  char buffer[256];
  webServer.sendContent_P(PSTR("<div class='config'><h2>Cambiar Credenciales de Acceso Web</h2><form action='/saveauth' method='post'>"));
  webServer.sendContent_P(PSTR("<label for='user'>Usuario:</label>"));
  snprintf_P(buffer, sizeof(buffer), PSTR("<input type='text' id='user' name='user' value='%s' required>"), web_user);
  webServer.sendContent(buffer);
  webServer.sendContent_P(PSTR("<label for='pass'>Nueva Contraseña:</label><input type='password' id='pass' name='pass' required><br><br><input type='submit' value='Guardar Credenciales'></form></div>"));

  // Send footer
  webServer.sendContent_P(PSTR("</body></html>"));
  webServer.sendContent(""); // Terminate the connection
}

// Manejar guardado de configuracion
void handleSaveSettings() {
  if (!isAuthenticated()) return;
  if (webServer.hasArg("deviceId")) {
    deviceId = webServer.arg("deviceId").toInt();
  }
  if (webServer.hasArg("pin_d0")) {
    basicConfig.pin_d0 = webServer.arg("pin_d0").toInt();
    basicConfig.pin_d1 = webServer.arg("pin_d1").toInt();
    basicConfig.pin_relay = webServer.arg("pin_relay").toInt();
    basicConfig.pin_led = webServer.arg("pin_led").toInt();
  }
  if (webServer.hasArg("relayOnDuration")) {
    basicConfig.relayOnDuration = webServer.arg("relayOnDuration").toInt();
  }

  // Guardar configuración de reinicio
  basicConfig.rebootEnabled = webServer.hasArg("rebootEnabled");
  if (webServer.hasArg("rebootHour")) {
    basicConfig.rebootHour = webServer.arg("rebootHour").toInt();
  }
  if (webServer.hasArg("rebootMinute")) {
    basicConfig.rebootMinute = webServer.arg("rebootMinute").toInt();
  }

  saveConfig();
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
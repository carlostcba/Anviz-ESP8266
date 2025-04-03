/**
 * web.h
 * Funciones para el servidor web del emulador Anviz
 * Con soporte UTF-8 para caracteres internacionales
 */

#ifndef WEB_H
#define WEB_H

// Variable para manejo de subida de archivos
File uploadFile;

// Declaración de funciones externas necesarias de utilidades.h
extern String getFormattedDateTime();
extern String formatTimestamp(uint32_t timestamp);
extern void saveRecords();

// ========= FUNCIONES DEL SERVIDOR WEB ===========

// Página principal
void handleRoot() {
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
  html += "<a href='/settings'>Configuración</a>";
  html += "</div>";
  html += "<div class='status'>";
  html += "<h2>Estado del Sistema</h2>";
  html += "<p>Dirección IP: " + WiFi.localIP().toString() + "</p>";
  html += "<p>Fuerza de señal WiFi: " + String(WiFi.RSSI()) + " dBm</p>";
  html += "<p>Usuarios registrados: " + String(userCount) + "</p>";
  html += "<p>Registros de acceso: " + String(recordCount) + " (nuevos: " + String(newRecordCount) + ")</p>";
  html += "<p>Fecha y hora: " + getFormattedDateTime() + "</p>";
  html += "</div>";
  html += "<div><h2>Operaciones</h2>";
  html += "<p><a href='/clearlogs' onclick='return confirm(\"¿Está seguro de borrar todos los registros?\");'>Borrar registros</a></p>";
  html += "<p><a href='/reset' onclick='return confirm(\"¿Está seguro de reiniciar el dispositivo?\");'>Reiniciar dispositivo</a></p>";
  html += "</div>";
  html += "</body></html>";
  
  webServer.sendHeader("Content-Type", "text/html; charset=UTF-8");
  webServer.send(200, "text/html", html);
}

// Página de usuarios
void handleUsers() {
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
  html += "<a href='/settings'>Configuración</a>";
  html += "</div>";
  
  // Tabla de usuarios
  html += "<table>";
  html += "<tr><th>ID</th><th>Nombre</th><th>ID de Tarjeta</th><th>Departamento</th><th>Estado</th></tr>";
  
  for (int i = 0; i < userCount; i++) {
    html += "<tr>";
    
    // ID de usuario (formatear en hexadecimal)
    html += "<td>";
    for (int j = 0; j < 5; j++) {
      char hexVal[3];
      sprintf(hexVal, "%02X", users[i].id[j]);
      html += String(hexVal);
      if (j < 4) html += ":";
    }
    html += "</td>";
    
    // Nombre
    html += "<td>" + String(users[i].name) + "</td>";
    
    // ID de Tarjeta (formato hexadecimal)
    char cardHex[10];
    sprintf(cardHex, "0x%08X", users[i].cardId);
    html += "<td>" + String(cardHex) + "</td>";
    
    // Departamento
    html += "<td>" + String(users[i].department) + "</td>";
    
    // Estado
    html += "<td>" + String(users[i].isActive ? "Activo" : "Inactivo") + "</td>";
    
    html += "</tr>";
  }
  
  html += "</table>";
  
  if (userCount == 0) {
    html += "<p>No hay usuarios registrados. Utilice el software Anviz CrossChex para añadir usuarios.</p>";
  }
  
  html += "</body></html>";
  
  webServer.sendHeader("Content-Type", "text/html; charset=UTF-8");
  webServer.send(200, "text/html", html);
}

// Página de registros de acceso
void handleRecords() {
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
  html += "<a href='/settings'>Configuración</a>";
  html += "</div>";
  
  // Tabla de registros (mostramos solo los últimos 50 para evitar sobrecarga)
  html += "<table>";
  html += "<tr><th>ID Usuario</th><th>Fecha/Hora</th><th>Tipo</th><th>Método</th></tr>";
  
  int startIdx = (recordCount > 50) ? (recordCount - 50) : 0;
  int count = (recordCount > 50) ? 50 : recordCount;
  
  for (int i = 0; i < count; i++) {
    int idx = startIdx + i;
    html += "<tr>";
    
    // ID de usuario
    html += "<td>";
    for (int j = 0; j < 5; j++) {
      char hexVal[3];
      sprintf(hexVal, "%02X", records[idx].id[j]);
      html += String(hexVal);
      if (j < 4) html += ":";
    }
    html += "</td>";
    
    // Fecha y hora
    html += "<td>" + formatTimestamp(records[idx].timestamp) + "</td>";
    
    // Tipo de registro
    String recordType;
    if (records[idx].recordType & 0x80) {
      recordType = "Entrada"; // Bit 7 = 1
    } else {
      recordType = "Salida";  // Bit 7 = 0
    }
    html += "<td>" + recordType + "</td>";
    
    // Método de verificación
    String method;
    switch (records[idx].backup) {
      case 0x01: method = "Contraseña"; break;
      case 0x02: method = "Huella digital"; break;
      case 0x08: method = "Tarjeta"; break;
      default: method = "Otro"; break;
    }
    html += "<td>" + method + "</td>";
    
    html += "</tr>";
  }
  
  html += "</table>";
  
  if (recordCount == 0) {
    html += "<p>No hay registros de acceso. Los registros se generarán cuando los usuarios utilicen sus tarjetas.</p>";
  } else if (recordCount > 50) {
    html += "<p>Mostrando los últimos 50 registros de un total de " + String(recordCount) + ".</p>";
  }
  
  html += "<p><a href='/clearlogs' onclick='return confirm(\"¿Está seguro de borrar todos los registros?\");'>Borrar todos los registros</a></p>";
  
  html += "</body></html>";
  
  webServer.sendHeader("Content-Type", "text/html; charset=UTF-8");
  webServer.send(200, "text/html", html);
}

// Página de configuración
void handleSettings() {
  String html = "<!DOCTYPE html><html><head><title>Configuración - Emulador Anviz</title>";
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
  html += "<h1>Configuración del Emulador</h1>";
  html += "<div class='menu'>";
  html += "<a href='/'>Inicio</a>";
  html += "<a href='/users'>Usuarios</a>";
  html += "<a href='/records'>Registros</a>";
  html += "<a href='/settings'>Configuración</a>";
  html += "</div>";
  html += "<div class='config'>";
  
  // Tabla de configuración
  html += "<table>";
  html += "<tr><th>Parámetro</th><th>Valor</th></tr>";
  html += "<tr><td>ID del dispositivo</td><td>0x" + String(DEVICE_ID, HEX) + "</td></tr>";
  html += "<tr><td>Versión de firmware</td><td>" + String(basicConfig.firmwareVersion) + "</td></tr>";
  html += "<tr><td>Número de serie</td><td>" + String(serialNumber) + "</td></tr>";
  html += "<tr><td>Volumen</td><td>" + String(basicConfig.volume) + "</td></tr>";
  
  // Idioma con nombres en lugar de números
  String languageName;
  switch(basicConfig.language) {
    case 0: languageName = "Chino Simplificado"; break;
    case 1: languageName = "Chino Tradicional"; break;
    case 2: languageName = "Inglés"; break;
    case 3: languageName = "Francés"; break;
    case 4: languageName = "Español"; break;
    case 5: languageName = "Portugués"; break;
    default: languageName = String(basicConfig.language); break;
  }
  html += "<tr><td>Idioma</td><td>" + languageName + "</td></tr>";
  
  // Formato de fecha con descripción
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
  
  // Comprobar si SPIFFS está disponible
  FSInfo fsInfo;
  if (SPIFFS.info(fsInfo)) {
    html += "<tr><td>Espacio SPIFFS utilizado</td><td>" + String(fsInfo.usedBytes) + " / " + String(fsInfo.totalBytes) + " bytes</td></tr>";
  }
  
  html += "</table>";
  
  html += "<p>Nota: Para modificar la configuración del dispositivo, utilice el software Anviz CrossChex.</p>";
  html += "</div>";
  html += "</body></html>";
  
  webServer.sendHeader("Content-Type", "text/html; charset=UTF-8");
  webServer.send(200, "text/html", html);
}

// Manejar borrado de registros
void handleClearLogs() {
  recordCount = 0;
  newRecordCount = 0;
  saveRecords();
  
  webServer.sendHeader("Location", "/records");
  webServer.send(303);
}

// Manejar reinicio del dispositivo
void handleReset() {
  webServer.sendHeader("Content-Type", "text/html; charset=UTF-8");
  webServer.send(200, "text/html", "<!DOCTYPE html><html><head><meta charset='UTF-8'><meta http-equiv='refresh' content='5;url=/'></head><body><h2>Reiniciando el dispositivo...</h2><p>Volviendo al inicio en 5 segundos.</p></body></html>");
  delay(1000);
  ESP.restart();
}

// Manejar carga de archivos
void handleFileUpload() {
  HTTPUpload& upload = webServer.upload();
  
  if (upload.status == UPLOAD_FILE_START) {
    String filename = "/upload_" + upload.filename;
    Serial.println("Iniciando carga: " + filename);
    
    // Abrir archivo para escritura
    File file = SPIFFS.open(filename, "w");
    if (!file) {
      Serial.println("Error al abrir el archivo para escritura");
      return;
    }
    
    // Almacenar handle del archivo para escrituras posteriores
    uploadFile = file;
  }
  else if (upload.status == UPLOAD_FILE_WRITE) {
    // Escribir datos recibidos al archivo
    if (uploadFile) {
      uploadFile.write(upload.buf, upload.currentSize);
    }
  }
  else if (upload.status == UPLOAD_FILE_END) {
    // Cerrar archivo
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

#endif // WEB_H

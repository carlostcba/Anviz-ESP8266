/**
 * protocolo.h
 * Implementación del protocolo Anviz TC_B V2.1
 */

#ifndef PROTOCOLO_H
#define PROTOCOLO_H

// Declaración de funciones
void handleGetDeviceInfo();
void handleGetRecordInfo();
void handleDownloadRecords(uint8_t* data, uint16_t dataLen);
void handleDownloadStaffInfo(uint8_t* data, uint16_t dataLen);
void handleUploadStaffInfo(uint8_t* data, uint16_t dataLen);
void handleDeleteUser(uint8_t* data, uint16_t dataLen);
void handleGetDeviceId();
void handleSetDeviceInfo(uint8_t* data, uint16_t dataLen);
void handleGetTime();
void handleSetTime(uint8_t* data, uint16_t dataLen);
void handleUploadRecord(uint8_t* data, uint16_t dataLen);
void handleDeleteRecords(uint8_t* data, uint16_t dataLen);
void handleForcedUnlock();
void handleSetDeviceId(uint8_t* data, uint16_t dataLen);
void handleGetDeviceTypeCode();
void sendSimpleResponse(uint8_t cmd, uint8_t ret);
void handleGetDeviceTypeCode();
void handleUploadStaffInfoExtended(uint8_t* data, uint16_t dataLen);
uint16_t calculateCRC16(uint8_t* data, int length);

// Funciones externas del módulo de almacenamiento
extern void saveConfig();
extern void saveUsers();
extern void saveRecords();

// ========= PROCESAMIENTO DE COMANDOS ANVIZ ===========
void processAnvizCommand() {
  uint8_t buffer[512];
  int bytesRead = 0;
  
  // Leer el encabezado (8 bytes)
  while (client.available() && bytesRead < 8) {
    buffer[bytesRead++] = client.read();
  }
  
  // Verificar STX
  if (buffer[0] != STX) {
    Serial.println("Error: STX incorrecto");
    return;
  }
  
  // Extraer CH, CMD y LEN
  uint32_t receivedDeviceId = ((uint32_t)buffer[1] << 24) | 
                      ((uint32_t)buffer[2] << 16) | 
                      ((uint32_t)buffer[3] << 8) | 
                      buffer[4];
  uint8_t cmd = buffer[5];
  uint16_t dataLen = ((uint16_t)buffer[6] << 8) | buffer[7];
  
  // Leer el resto del comando
  int expectedBytes = dataLen + 2; // Datos + CRC16
  int timeout = 1000; // 1 segundo de timeout
  unsigned long startTime = millis();
  
  while (bytesRead < 8 + expectedBytes && millis() - startTime < timeout) {
    if (client.available()) {
      buffer[bytesRead++] = client.read();
    }
  }
  
  // Verificar si recibimos todos los datos esperados
  if (bytesRead != 8 + expectedBytes) {
    Serial.println("Error: Datos incompletos");
    return;
  }
  
  // Verificar CRC16
uint16_t receivedCRC = ((uint16_t)buffer[bytesRead-2] << 8) | buffer[bytesRead-1];
uint16_t calculatedCRC = calculateCRC16(buffer, bytesRead - 2);

// Después de leer el mensaje completo
Serial.println("Mensaje recibido:");
for (int i = 0; i < bytesRead; i++) {
  Serial.print(buffer[i], HEX);
  Serial.print(" ");
  if ((i + 1) % 16 == 0) Serial.println();
}
Serial.println();
Serial.print("CRC recibido: 0x");
Serial.println(receivedCRC, HEX);
Serial.print("CRC calculado: 0x");
Serial.println(calculatedCRC, HEX);

if (receivedCRC != calculatedCRC) {
  Serial.println("Error: CRC incorrecto");
  return;
}
  
  Serial.print("Comando recibido: 0x");
  Serial.println(cmd, HEX);
  
  // Procesar comandos específicos
  switch (cmd) {
    // Comandos Críticos
    case 0x30: // Obtener información del dispositivo T&A 1
      handleGetDeviceInfo();
      break;
    case 0x3C: // Obtener información de registros
      handleGetRecordInfo();
      break;
    case 0x40: // Descargar registros de acceso
      handleDownloadRecords(&buffer[8], dataLen);
      break;
    case 0x42: // Descargar información de personal
      handleDownloadStaffInfo(&buffer[8], dataLen);
      break;
    case 0x43: // Cargar información de personal
      handleUploadStaffInfo(&buffer[8], dataLen);
      break;
    case 0x4C: // Eliminar datos de usuario
      handleDeleteUser(&buffer[8], dataLen);
      break;
    case 0x74: // Obtener ID del dispositivo
      handleGetDeviceId();
      break;
      
    // Comandos Importantes
    case 0x31: // Configurar información de T&A 1
      handleSetDeviceInfo(&buffer[8], dataLen);
      break;
    case 0x38: // Obtener fecha y hora del dispositivo
      handleGetTime();
      break;
    case 0x39: // Configurar fecha y hora
      handleSetTime(&buffer[8], dataLen);
      break;
    case 0x41: // Cargar registros de T&A
      handleUploadRecord(&buffer[8], dataLen);
      break;
    case 0x4E: // Borrar registros/Borrar marcas de nuevos registros
      handleDeleteRecords(&buffer[8], dataLen);
      break;
      
    // Comandos Adicionales
    case 0x5E: // Abrir cerradura sin verificar usuario
      handleForcedUnlock();
      break;
    case 0x75: // Modificar ID de dispositivo de comunicación
      handleSetDeviceId(&buffer[8], dataLen);
      break;
    case 0x48: // Obtener código de tipo de dispositivo
      handleGetDeviceTypeCode();
      break;
    case 0x73: // Cargar información de personal (versión extendida)
      handleUploadStaffInfoExtended(&buffer[8], dataLen);
      break;
      
    default:
      // Comando no soportado
      Serial.print("Comando no soportado: 0x");
      Serial.println(cmd, HEX);
      sendSimpleResponse(cmd, ACK_FAIL);
      break;
  }
}

// CMD 0x30: Obtener información del dispositivo
void handleGetDeviceInfo() {
  uint8_t response[29];
  
  // STX
  response[0] = STX;
  
  // CH (Device ID)
  response[1] = (deviceId >> 24) & 0xFF;
  response[2] = (deviceId >> 16) & 0xFF;
  response[3] = (deviceId >> 8) & 0xFF;
  response[4] = deviceId & 0xFF;
  
  // ACK
  response[5] = 0xB0; // CMD (0x30) + 0x80
  
  // RET
  response[6] = ACK_SUCCESS;
  
  // LEN
  response[7] = 0x00;
  response[8] = 0x12; // 18 bytes de datos
  
  // DATA (18 bytes)
  // Firmware version (8 bytes)
  for (int i = 0; i < 8; i++) {
    response[9+i] = (i < strlen(basicConfig.firmwareVersion)) ? basicConfig.firmwareVersion[i] : 0;
  }
  
  // Communication password (3 bytes)
  response[17] = basicConfig.password[0];
  response[18] = basicConfig.password[1];
  response[19] = basicConfig.password[2];
  
  // Sleep time (1 byte)
  response[20] = basicConfig.sleepTime;
  
  // Volume (1 byte)
  response[21] = basicConfig.volume;
  
  // Language (1 byte)
  response[22] = basicConfig.language;
  
  // Date/Time format (1 byte)
  response[23] = basicConfig.dateFormat;
  
  // Attendance state (1 byte)
  response[24] = basicConfig.machineStatus;
  
  // Language setting flag (1 byte)
  response[25] = basicConfig.languageFlag;
  
  // Command version (1 byte)
  response[26] = basicConfig.cmdVersion;
  
  // Calcular CRC16
  uint16_t crc = calculateCRC16(response, 27);
  response[27] = (crc >> 8) & 0xFF;
  response[28] = crc & 0xFF;
  
  // Enviar respuesta
  client.write(response, 29);
}

// CMD 0x3C: Obtener información de registros
void handleGetRecordInfo() {
  uint8_t response[29];
  
  // STX
  response[0] = STX;
  
  // CH (Device ID)
  response[1] = (deviceId >> 24) & 0xFF;
  response[2] = (deviceId >> 16) & 0xFF;
  response[3] = (deviceId >> 8) & 0xFF;
  response[4] = deviceId & 0xFF;
  
  // ACK
  response[5] = 0xBC; // CMD (0x3C) + 0x80
  
  // RET
  response[6] = ACK_SUCCESS;
  
  // LEN
  response[7] = 0x00;
  response[8] = 0x12; // 18 bytes de datos
  
  // DATA (18 bytes)
  // User Amount (3 bytes)
  response[9] = 0;
  response[10] = 0;
  response[11] = userCount;
  
  // FP Amount (3 bytes)
  response[12] = 0;
  response[13] = 0;
  response[14] = 0; // No soportamos huellas
  
  // Password Amount (3 bytes)
  response[15] = 0;
  response[16] = 0;
  response[17] = 0; // No usamos contraseñas
  
  // Card Amount (3 bytes)
  response[18] = 0;
  response[19] = 0;
  response[20] = userCount; // Asumimos que cada usuario tiene una tarjeta
  
  // All Record Amount (3 bytes)
  response[21] = (recordCount >> 16) & 0xFF;
  response[22] = (recordCount >> 8) & 0xFF;
  response[23] = recordCount & 0xFF;
  
  // New Record Amount (3 bytes)
  response[24] = (newRecordCount >> 16) & 0xFF;
  response[25] = (newRecordCount >> 8) & 0xFF;
  response[26] = newRecordCount & 0xFF;
  
  // Calcular CRC16
  uint16_t crc = calculateCRC16(response, 27);
  response[27] = (crc >> 8) & 0xFF;
  response[28] = crc & 0xFF;
  
  // Enviar respuesta
  client.write(response, 29);
}

// CMD 0x40: Descargar registros de acceso
void handleDownloadRecords(uint8_t* data, uint16_t dataLen) {
  if (dataLen < 2) {
    sendSimpleResponse(0x40, ACK_FAIL);
    return;
  }
  
  // Extraer parámetros
  uint8_t parameter = data[0];
  uint8_t requestedCount = data[1];
  
  // Limitar a 25 registros por solicitud como indica el protocolo
  if (requestedCount > 25) {
    requestedCount = 25;
  }
  
  // Determinar qué registros enviar
  int startIndex = 0;
  int count = 0;
  
  if (parameter == 1) {
    // Reiniciar y enviar todos los registros
    lastDownloadRecordIndex = 0;
    startIndex = 0;
    count = (requestedCount < recordCount) ? requestedCount : recordCount;
  } else if (parameter == 2) {
    // Reiniciar y enviar nuevos registros
    lastDownloadRecordIndex = (recordCount > newRecordCount) ? (recordCount - newRecordCount) : 0;
    startIndex = lastDownloadRecordIndex;
    count = (requestedCount < newRecordCount) ? requestedCount : newRecordCount;
  } else if (parameter == 0) {
    // Continuar descarga normal
    startIndex = lastDownloadRecordIndex;
    count = (requestedCount < (recordCount - lastDownloadRecordIndex)) ? 
            requestedCount : (recordCount - lastDownloadRecordIndex);
    lastDownloadRecordIndex += count;
  }
  
  // Si no hay registros para enviar, envía una respuesta vacía pero exitosa.
  if (count == 0) {
    sendSimpleResponse(0x40, ACK_SUCCESS);
    return;
  }
  
  // Preparar respuesta
  // Usar un buffer estático para evitar la asignación dinámica y posible fragmentación.
  // El tamaño máximo es 12 bytes de cabecera + 25 registros * 18 bytes/registro = 462 bytes.
  uint8_t response[12 + 25 * 18];
  // STX
  response[0] = STX;
  
  // CH (Device ID)
  response[1] = (deviceId >> 24) & 0xFF;
  response[2] = (deviceId >> 16) & 0xFF;
  response[3] = (deviceId >> 8) & 0xFF;
  response[4] = deviceId & 0xFF;
  
  // ACK
  response[5] = 0xC0; // CMD (0x40) + 0x80
  
  // RET
  response[6] = ACK_SUCCESS;
  
  // LEN
  uint16_t responseLen = 1 + count * 18; // Cada registro ahora ocupa 18 bytes
  response[7] = (responseLen >> 8) & 0xFF;
  response[8] = responseLen & 0xFF;
  
  // DATA
  // Valid records count
  response[9] = count;
  
  // Records data
  for (int i = 0; i < count; i++) {
    int idx = startIndex + i;
    int recordOffset = 10 + i * 18;

    // User ID (5 bytes)
    memcpy(&response[recordOffset], records[idx].id, 5);
    
    // Date & Time (4 bytes)
    uint32_t timestamp = records[idx].timestamp;
    response[recordOffset + 5] = (timestamp >> 24) & 0xFF;
    response[recordOffset + 6] = (timestamp >> 16) & 0xFF;
    response[recordOffset + 7] = (timestamp >> 8) & 0xFF;
    response[recordOffset + 8] = timestamp & 0xFF;
    
    // Backup code (1 byte)
    response[recordOffset + 9] = records[idx].backup;
    
    // Record type (1 byte)
    response[recordOffset + 10] = records[idx].recordType;
    
    // Work code (3 bytes)
    memcpy(&response[recordOffset + 11], records[idx].workCode, 3);

    // Device ID (4 bytes) - ¡ESTO ES LO QUE FALTABA!
    response[recordOffset + 14] = (deviceId >> 24) & 0xFF;
    response[recordOffset + 15] = (deviceId >> 16) & 0xFF;
    response[recordOffset + 16] = (deviceId >> 8) & 0xFF;
    response[recordOffset + 17] = deviceId & 0xFF;
  }
  
  // Calcular CRC16
  uint16_t crc = calculateCRC16(response, 9 + responseLen);
  response[9 + responseLen] = (crc >> 8) & 0xFF;
  response[9 + responseLen + 1] = crc & 0xFF;
  
  // Enviar respuesta
  client.write(response, 9 + responseLen + 2);
  
  // Si hemos enviado registros nuevos, actualizamos el contador
  if (parameter == 2 && count > 0) {
    newRecordCount = 0;
    saveRecords();
  }
}

// CMD 0x42: Descargar información de personal
void handleDownloadStaffInfo(uint8_t* data, uint16_t dataLen) {
  if (dataLen < 2) {
    sendSimpleResponse(0x42, ACK_FAIL);
    return;
  }
  
  // Extraer parámetros
  uint8_t parameter = data[0];
  uint8_t requestedCount = data[1];
  
  // Limitar a 12 usuarios por solicitud como indica el protocolo
  if (requestedCount > 12) {
    requestedCount = 12;
  }
  
  // Determinar qué usuarios enviar
  int startIndex = 0;
  int count = 0;
  
  if (parameter == 1) {
    // Reiniciar y enviar todos los usuarios
    lastDownloadUserIndex = 0;
    startIndex = 0;
    count = (requestedCount < userCount) ? requestedCount : userCount;
  } else if (parameter == 0) {
    // Continuar descarga normal
    startIndex = lastDownloadUserIndex;
    count = (requestedCount < (userCount - lastDownloadUserIndex)) ? 
            requestedCount : (userCount - lastDownloadUserIndex);
    lastDownloadUserIndex += count;
    
    // Si hemos llegado al final, reiniciar
    if (lastDownloadUserIndex >= userCount) {
      lastDownloadUserIndex = 0;
    }
  }
  
  // Preparar respuesta
  // Usar un buffer estático para evitar la asignación dinámica y posible fragmentación.
  // El tamaño máximo es 12 bytes de cabecera + 12 usuarios * 27 bytes/usuario = 336 bytes.
  uint8_t response[12 + 12 * 27];
  
  // STX
  response[0] = STX;
  
  
  // CH (Device ID)
  response[1] = (deviceId >> 24) & 0xFF;
  response[2] = (deviceId >> 16) & 0xFF;
  response[3] = (deviceId >> 8) & 0xFF;
  response[4] = deviceId & 0xFF;
  
  // ACK
  response[5] = 0xC2; // CMD (0x42) + 0x80
  
  // RET
  response[6] = ACK_SUCCESS;
  
  // LEN
  uint16_t responseLen = 1 + count * 27;
  response[7] = (responseLen >> 8) & 0xFF;
  response[8] = responseLen & 0xFF;
  
  // DATA
  // Valid users count
  response[9] = count;
  
  // Users data
  for (int i = 0; i < count; i++) {
    int idx = startIndex + i;
    
    // User ID (5 bytes)
    memcpy(&response[10 + i*27], users[idx].id, 5);
    
    // Password (3 bytes)
    memcpy(&response[10 + i*27 + 5], users[idx].password, 3);
    
    // Card ID (3 bytes)
    uint32_t cardId = users[idx].cardId;
    response[10 + i*27 + 8] = (cardId >> 16) & 0xFF;
    response[10 + i*27 + 9] = (cardId >> 8) & 0xFF;
    response[10 + i*27 + 10] = cardId & 0xFF;
    
    // Name (10 bytes)
    memcpy(&response[10 + i*27 + 11], users[idx].name, 10);
    
    // Department (1 byte)
    response[10 + i*27 + 21] = users[idx].department;
    
    // Group (1 byte)
    response[10 + i*27 + 22] = users[idx].group;
    
    // Attendance mode (1 byte)
    response[10 + i*27 + 23] = users[idx].mode;
    
    // FP Status (2 bytes)
    memcpy(&response[10 + i*27 + 24], users[idx].fpStatus, 2);
    
    // Special info (1 byte)
    response[10 + i*27 + 26] = users[idx].special;
  }
  
  // Calcular CRC16
  uint16_t crc = calculateCRC16(response, 9 + responseLen);
  response[9 + responseLen] = (crc >> 8) & 0xFF;
  response[9 + responseLen + 1] = crc & 0xFF;
  
  // Enviar respuesta
  client.write(response, 12 + count * 27);
}

// CMD 0x43: Cargar información de personal
void handleUploadStaffInfo(uint8_t* data, uint16_t dataLen) {
  if (dataLen < 1) {
    sendSimpleResponse(0x43, ACK_FAIL);
    return;
  }
  
  // Extraer el número de usuarios
  uint8_t count = data[0];
  
  // Verificar que hay suficientes datos
  if (dataLen < 1 + count * 27) {
    sendSimpleResponse(0x43, ACK_FAIL);
    return;
  }
  
  // Procesar cada usuario
  uint16_t result = 0; // Bits indicando éxito/fallo para cada usuario
  
  for (int i = 0; i < count; i++) {
    uint8_t* userData = &data[1 + i*27];
    
    // Buscar si el usuario ya existe por ID
    int existingIndex = -1;
    for (int j = 0; j < userCount; j++) {
      if (memcmp(users[j].id, userData, 5) == 0) {
        existingIndex = j;
        break;
      }
    }
    
    if (existingIndex >= 0) {
      // Actualizar usuario existente
      memcpy(users[existingIndex].password, &userData[5], 3);
      
      // Extraer Card ID
      uint32_t cardId = ((uint32_t)userData[8] << 16) | 
                        ((uint32_t)userData[9] << 8) | 
                        userData[10];
      users[existingIndex].cardId = cardId;
      
      // Copiar resto de datos
      memcpy(users[existingIndex].name, &userData[11], 10);
      users[existingIndex].department = userData[21];
      users[existingIndex].group = userData[22];
      users[existingIndex].mode = userData[23];
      memcpy(users[existingIndex].fpStatus, &userData[24], 2);
      users[existingIndex].special = userData[26];
      users[existingIndex].isActive = true;
      
      // Marcar como exitoso
      result |= (1 << i);
    } else if (userCount < 100) {
      // Añadir nuevo usuario
      User newUser;
      
      memcpy(newUser.id, userData, 5);
      memcpy(newUser.password, &userData[5], 3);
      
      // Extraer Card ID
      uint32_t cardId = ((uint32_t)userData[8] << 16) | 
                        ((uint32_t)userData[9] << 8) | 
                        userData[10];
      newUser.cardId = cardId;
      
      // Copiar resto de datos
      memcpy(newUser.name, &userData[11], 10);
      newUser.department = userData[21];
      newUser.group = userData[22];
      newUser.mode = userData[23];
      memcpy(newUser.fpStatus, &userData[24], 2);
      newUser.special = userData[26];
      newUser.isActive = true;
      
      // Añadir a la lista
      users[userCount++] = newUser;
      
      // Marcar como exitoso
      result |= (1 << i);
    }
  }
  
  // Guardar usuarios
  saveUsers();
  
  // Preparar respuesta
  uint8_t response[13];
  
  // STX
  response[0] = STX;
  
  // CH (Device ID)
  response[1] = (deviceId >> 24) & 0xFF;
  response[2] = (deviceId >> 16) & 0xFF;
  response[3] = (deviceId >> 8) & 0xFF;
  response[4] = deviceId & 0xFF;
  
  // ACK
  response[5] = 0xC3; // CMD (0x43) + 0x80
  
  // RET
  response[6] = ACK_SUCCESS;
  
  // LEN
  response[7] = 0x00;
  response[8] = 0x02;
  
  // DATA (2 bytes)
  response[9] = result & 0xFF;
  response[10] = (result >> 8) & 0xFF;
  
  // Calcular CRC16
  uint16_t crc = calculateCRC16(response, 11);
  response[11] = (crc >> 8) & 0xFF;
  response[12] = crc & 0xFF;
  
  // Enviar respuesta
  client.write(response, 13);
}

// CMD 0x4C: Eliminar datos de usuario
void handleDeleteUser(uint8_t* data, uint16_t dataLen) {
  if (dataLen < 6) {
    sendSimpleResponse(0x4C, ACK_FAIL);
    return;
  }
  
  // Extraer ID y tipo de borrado
  uint8_t userId[5];
  memcpy(userId, data, 5);
  uint8_t backupCode = data[5];
  
  // Buscar usuario por ID
  int userIndex = -1;
  for (int i = 0; i < userCount; i++) {
    if (memcmp(users[i].id, userId, 5) == 0) {
      userIndex = i;
      break;
    }
  }
  
  if (userIndex < 0) {
    // Usuario no encontrado
    sendSimpleResponse(0x4C, ACK_NO_USER);
    return;
  }
  
  // Procesar según tipo de borrado
  if (backupCode == 0xFF) {
    // Borrar completamente
    // Mover todos los usuarios una posición hacia atrás
    for (int i = userIndex; i < userCount - 1; i++) {
      users[i] = users[i + 1];
    }
    userCount--;
  } else {
    // Borrar selectivamente
    if (backupCode & 0x08) { // Borrar tarjeta
      users[userIndex].cardId = 0;
    }
    if (backupCode & 0x04) { // Borrar contraseña
      memset(users[userIndex].password, 0xFF, 3);
    }
    // No implementamos borrado de huellas digitales porque no las usamos
  }
  
  // Guardar usuarios
  saveUsers();
  
  // Enviar respuesta
  sendSimpleResponse(0x4C, ACK_SUCCESS);
}

// CMD 0x74: Obtener ID del dispositivo
void handleGetDeviceId() {
  uint8_t response[15];
  
  // STX
  response[0] = STX;
  
  // CH (Device ID)
  response[1] = (deviceId >> 24) & 0xFF;
  response[2] = (deviceId >> 16) & 0xFF;
  response[3] = (deviceId >> 8) & 0xFF;
  response[4] = deviceId & 0xFF;
  
  // ACK
  response[5] = 0xF4; // CMD (0x74) + 0x80
  
  // RET
  response[6] = ACK_SUCCESS;
  
  // LEN
  response[7] = 0x00;
  response[8] = 0x04;
  
  // DATA (4 bytes) - El ID del dispositivo
  response[9] = (deviceId >> 24) & 0xFF;
  response[10] = (deviceId >> 16) & 0xFF;
  response[11] = (deviceId >> 8) & 0xFF;
  response[12] = deviceId & 0xFF;
  
  // Calcular CRC16
  uint16_t crc = calculateCRC16(response, 13);
  response[13] = (crc >> 8) & 0xFF;
  response[14] = crc & 0xFF;
  
  // Enviar respuesta
  client.write(response, 15);
}

// CMD 0x31: Configurar información de T&A 1
void handleSetDeviceInfo(uint8_t* data, uint16_t dataLen) {
  if (dataLen < 18) {
    sendSimpleResponse(0x31, ACK_FAIL);
    return;
  }
  
  // Extraer datos
  for (int i = 0; i < 8; i++) {
    basicConfig.firmwareVersion[i] = data[i];
  }
  basicConfig.firmwareVersion[8] = 0; // Asegurar terminador null
  
  memcpy(basicConfig.password, &data[8], 3);
  basicConfig.sleepTime = data[11];
  basicConfig.volume = data[12];
  basicConfig.language = data[13];
  basicConfig.dateFormat = data[14];
  basicConfig.machineStatus = data[15];
  basicConfig.languageFlag = data[16];
  basicConfig.cmdVersion = data[17];
  
  // Guardar configuración
  saveConfig();
  
  // Enviar respuesta
  sendSimpleResponse(0x31, ACK_SUCCESS);
}

// CMD 0x38: Obtener fecha y hora del dispositivo
void handleGetTime() {
  uint8_t response[16];
  
  // STX
  response[0] = STX;
  
  // CH (Device ID)
  response[1] = (deviceId >> 24) & 0xFF;
  response[2] = (deviceId >> 16) & 0xFF;
  response[3] = (deviceId >> 8) & 0xFF;
  response[4] = deviceId & 0xFF;
  
  // ACK
  response[5] = 0xB8; // CMD (0x38) + 0x80
  
  // RET
  response[6] = ACK_SUCCESS;
  
  // LEN
  response[7] = 0x00;
  response[8] = 0x05;
  
  // DATA (5 bytes) - Fecha y hora actuales
  response[9] = year() % 100;  // Últimos 2 dígitos del año
  response[10] = month();      // Mes (1-12)
  response[11] = day();        // Día (1-31)
  response[12] = hour();       // Hora (0-23)
  response[13] = minute();     // Minutos (0-59)
  
  // Calcular CRC16
  uint16_t crc = calculateCRC16(response, 14);
  response[14] = (crc >> 8) & 0xFF;
  response[15] = crc & 0xFF;
  
  // Enviar respuesta
  client.write(response, 16);
}

// CMD 0x39: Configurar fecha y hora
void handleSetTime(uint8_t* data, uint16_t dataLen) {
  if (dataLen < 5) {
    sendSimpleResponse(0x39, ACK_FAIL);
    return;
  }
  
  // Extraer fecha y hora
  int yr = 2000 + data[0];  // Año (2000-2099)
  int mo = data[1];         // Mes (1-12)
  int dy = data[2];         // Día (1-31)
  int hr = data[3];         // Hora (0-23)
  int mn = data[4];         // Minutos (0-59)
  
  // Configurar tiempo interno
  setTime(hr, mn, 0, dy, mo, yr);
  
  // Enviar respuesta
  sendSimpleResponse(0x39, ACK_SUCCESS);
}

// CMD 0x41: Cargar registros de T&A
void handleUploadRecord(uint8_t* data, uint16_t dataLen) {
  if (dataLen < 1 || data[0] == 0) {
    sendSimpleResponse(0x41, ACK_FAIL);
    return;
  }
  
  uint8_t count = data[0];
  
  if (dataLen < 1 + count * 14) {
    sendSimpleResponse(0x41, ACK_FAIL);
    return;
  }
  
  // Procesar cada registro
  for (int i = 0; i < count; i++) {
    if (recordCount >= 500) {
      // Buffer lleno, sobrescribir el más antiguo
      for (int j = 0; j < 499; j++) {
        records[j] = records[j+1];
      }
      recordCount = 499;
    }
    
    AccessRecord record;
    
    // Copiar ID de usuario (5 bytes)
    memcpy(record.id, &data[1 + i*14], 5);
    
    // Copiar timestamp (4 bytes)
    record.timestamp = ((uint32_t)data[1 + i*14 + 5] << 24) | 
                       ((uint32_t)data[1 + i*14 + 6] << 16) |
                       ((uint32_t)data[1 + i*14 + 7] << 8) |
                       data[1 + i*14 + 8];
    
    // Backup code (1 byte)
    record.backup = data[1 + i*14 + 9];
    
    // Record type (1 byte)
    record.recordType = data[1 + i*14 + 10];
    
    // Work code (3 bytes)
    memcpy(record.workCode, &data[1 + i*14 + 11], 3);
    
    // Añadir registro
    records[recordCount++] = record;
    newRecordCount++;
  }
  
  // Guardar registros
  saveRecords();
  
  // Enviar respuesta
  sendSimpleResponse(0x41, ACK_SUCCESS);
}

// CMD 0x4E: Borrar registros
void handleDeleteRecords(uint8_t* data, uint16_t dataLen) {
  if (dataLen < 1) {
    sendSimpleResponse(0x4E, ACK_FAIL);
    return;
  }
  
  uint8_t parameter = data[0];
  
  if (parameter == 1) {
    // Borrar todos los registros
    recordCount = 0;
    newRecordCount = 0;
  } else if (parameter == 2) {
    // Borrar marca de nuevos registros
    newRecordCount = 0;
  }
  
  // Guardar cambios
  saveRecords();
  
  // Enviar respuesta
  sendSimpleResponse(0x4E, ACK_SUCCESS);
}

// CMD 0x5E: Abrir cerradura sin verificar usuario
void handleForcedUnlock() {
  // Activar relé para abrir la puerta
  digitalWrite(basicConfig.pin_relay, HIGH);
  digitalWrite(basicConfig.pin_led, HIGH);
  
  // Preparar respuesta
  sendSimpleResponse(0x5E, ACK_SUCCESS);
  
  // Esperar y desactivar el relé
  delay(2000);
  digitalWrite(basicConfig.pin_relay, LOW);
  digitalWrite(basicConfig.pin_led, LOW);
}

// CMD 0x75: Modificar ID de dispositivo de comunicación
void handleSetDeviceId(uint8_t* data, uint16_t dataLen) {
  if (dataLen < 4) {
    sendSimpleResponse(0x75, ACK_FAIL);
    return;
  }
  
  // No cambiamos realmente el ID ya que está definido como constante,
  // pero simulamos una respuesta exitosa para mantener la compatibilidad
  sendSimpleResponse(0x75, ACK_SUCCESS);
}

// CMD 0x48: Obtener código de tipo de dispositivo
void handleGetDeviceTypeCode() {
    // Esta función estaba causando un crash por acceso ilegal a memoria.
    // Se deshabilita su contenido y se envía una respuesta simple para mantener la compatibilidad.
    sendSimpleResponse(0x48, ACK_SUCCESS);
}

// CMD 0x73: Cargar información de personal (extendido)
void handleUploadStaffInfoExtended(uint8_t* data, uint16_t dataLen) {
    Serial.println("Iniciando carga de usuarios extendida");
    Serial.print("Longitud de datos: ");
    Serial.println(dataLen);

    if (dataLen < 1) {
        Serial.println("Error: Datos insuficientes");
        sendSimpleResponse(0x73, ACK_FAIL);
        return;
    }
    
    // Extraer el número de usuarios
    uint8_t count = data[0];
    Serial.print("Número de usuarios a cargar: ");
    Serial.println(count);
    
    // Verificar que hay suficientes datos (cada usuario ocupa 30 bytes)
    if (dataLen < 1 + count * 30) {
        Serial.println("Error: Longitud de datos incorrecta");
        sendSimpleResponse(0x73, ACK_FAIL);
        return;
    }
    
    // Procesar cada usuario
    uint16_t result = 0; // Bits indicando éxito/fallo para cada usuario
    
    for (int i = 0; i < count; i++) {
        uint8_t* userData = &data[1 + i*30];
        
        // Buscar si el usuario ya existe por ID
        int existingIndex = -1;
        for (int j = 0; j < userCount; j++) {
            if (memcmp(users[j].id, userData, 5) == 0) {
                existingIndex = j;
                break;
            }
        }
        
        if (existingIndex >= 0) {
            // Actualizar usuario existente
            memcpy(users[existingIndex].id, userData, 5);
            
            // Extraer número de contraseña + contraseña
            memcpy(users[existingIndex].password, &userData[5], 3);
            
            // Extraer Card ID (4 bytes) - Corregido
            uint32_t cardId = 
                ((uint32_t)userData[8] << 24) | 
                ((uint32_t)userData[9] << 16) | 
                ((uint32_t)userData[10] << 8) | 
                userData[11];
            
            users[existingIndex].cardId = cardId;
            
            // Copiar nombre 
            memcpy(users[existingIndex].name, &userData[12], 10);
            users[existingIndex].name[10] = '\0'; // Asegurar terminador null
            
            users[existingIndex].department = userData[22];
            users[existingIndex].group = userData[23];
            users[existingIndex].mode = userData[24];
            
            // Estado de huella digital
            memcpy(users[existingIndex].fpStatus, &userData[25], 2);
            
            users[existingIndex].special = userData[27];
            users[existingIndex].isActive = true;
            
            // Marcar como exitoso
            result |= (1 << i);
        } 
        else if (userCount < 100) {
            // Añadir nuevo usuario
            User newUser = {0}; // Inicializar a cero
            
            memcpy(newUser.id, userData, 5);
            memcpy(newUser.password, &userData[5], 3);
            
            // Extraer Card ID (4 bytes) - Corregido
            uint32_t cardId = 
                ((uint32_t)userData[8] << 24) | 
                ((uint32_t)userData[9] << 16) | 
                ((uint32_t)userData[10] << 8) | 
                userData[11];
            
            newUser.cardId = cardId;
            
            // Copiar nombre
            memcpy(newUser.name, &userData[12], 10);
            newUser.name[10] = '\0'; // Asegurar terminador null
            
            newUser.department = userData[22];
            newUser.group = userData[23];
            newUser.mode = userData[24];
            
            memcpy(newUser.fpStatus, &userData[25], 2);
            newUser.special = userData[27];
            newUser.isActive = true;
            
            // Añadir a la lista
            users[userCount++] = newUser;
            
            // Marcar como exitoso
            result |= (1 << i);
        }
    }
    
    // Guardar usuarios
    saveUsers();
    
    // Preparar respuesta
    uint8_t response[13];
    
    // STX
    response[0] = STX;
    
    // CH (Device ID)
    response[1] = (deviceId >> 24) & 0xFF;
    response[2] = (deviceId >> 16) & 0xFF;
    response[3] = (deviceId >> 8) & 0xFF;
    response[4] = deviceId & 0xFF;
    
    // ACK
    response[5] = 0xF3; // CMD (0x73) + 0x80
    
    // RET
    response[6] = ACK_SUCCESS;
    
    // LEN
    response[7] = 0x00;
    response[8] = 0x02;
    
    // DATA (2 bytes)
    response[9] = result & 0xFF;
    response[10] = (result >> 8) & 0xFF;
    
    // Calcular CRC16
    uint16_t crc = calculateCRC16(response, 11);
    response[11] = (crc >> 8) & 0xFF;
    response[12] = crc & 0xFF;
    
    // Enviar respuesta
    client.write(response, 13);
    
    Serial.print("Carga de usuarios completada. Total usuarios: ");
    Serial.println(userCount);
}

// Función genérica para enviar respuestas simples
void sendSimpleResponse(uint8_t cmd, uint8_t ret) {
  uint8_t response[11];
  
  // STX
  response[0] = STX;
  
  // CH (Device ID)
  response[1] = (deviceId >> 24) & 0xFF;
  response[2] = (deviceId >> 16) & 0xFF;
  response[3] = (deviceId >> 8) & 0xFF;
  response[4] = deviceId & 0xFF;
  
  // ACK
  response[5] = cmd + 0x80;
  
  // RET
  response[6] = ret;
  
  // LEN
  response[7] = 0x00;
  response[8] = 0x00;
  
  // Calcular CRC16
  uint16_t crc = calculateCRC16(response, 9);
  response[9] = (crc >> 8) & 0xFF;
  response[10] = crc & 0xFF;
  
  // Enviar respuesta
  client.write(response, 11);
}

// Función para calcular CRC16
// Tabla precalculada para CRC16-CCITT
static const uint16_t crc16_table[256] = {
    0x0000, 0x1189, 0x2312, 0x329B, 0x4624, 0x57AD, 0x6536, 0x74BF,
    0x8C48, 0x9DC1, 0xAF5A, 0xBED3, 0xCA6C, 0xDBE5, 0xE97E, 0xF8F7,
    0x1081, 0x0108, 0x3393, 0x221A, 0x56A5, 0x472C, 0x75B7, 0x643E,
    0x9CC9, 0x8D40, 0xBFDB, 0xAE52, 0xDAED, 0xCB64, 0xF9FF, 0xE876,
    0x2102, 0x308B, 0x0210, 0x1399, 0x6726, 0x76AF, 0x4434, 0x55BD,
    0xAD4A, 0xBCC3, 0x8E58, 0x9FD1, 0xEB6E, 0xFAE7, 0xC87C, 0xD9F5,
    0x3183, 0x200A, 0x1291, 0x0318, 0x77A7, 0x662E, 0x54B5, 0x453C,
    0xBDCB, 0xAC42, 0x9ED9, 0x8F50, 0xFBEF, 0xEA66, 0xD8FD, 0xC974,
    0x4204, 0x538D, 0x6116, 0x709F, 0x0420, 0x15A9, 0x2732, 0x36BB,
    0xCE4C, 0xDFC5, 0xED5E, 0xFCD7, 0x8868, 0x99E1, 0xAB7A, 0xBAF3,
    0x5285, 0x430C, 0x7197, 0x601E, 0x14A1, 0x0528, 0x37B3, 0x263A,
    0xDECD, 0xCF44, 0xFDDF, 0xEC56, 0x98E9, 0x8960, 0xBBFB, 0xAA72,
    0x6306, 0x728F, 0x4014, 0x519D, 0x2522, 0x34AB, 0x0630, 0x17B9,
    0xEF4E, 0xFEC7, 0xCC5C, 0xDDD5, 0xA96A, 0xB8E3, 0x8A78, 0x9BF1,
    0x7387, 0x620E, 0x5095, 0x411C, 0x35A3, 0x242A, 0x16B1, 0x0738,
    0xFFCF, 0xEE46, 0xDCDD, 0xCD54, 0xB9EB, 0xA862, 0x9AF9, 0x8B70,
    0x8408, 0x9581, 0xA71A, 0xB693, 0xC22C, 0xD3A5, 0xE13E, 0xF0B7,
    0x0840, 0x19C9, 0x2B52, 0x3ADB, 0x4E64, 0x5FED, 0x6D76, 0x7CFF,
    0x9489, 0x8500, 0xB79B, 0xA612, 0xD2AD, 0xC324, 0xF1BF, 0xE036,
    0x18C1, 0x0948, 0x3BD3, 0x2A5A, 0x5EE5, 0x4F6C, 0x7DF7, 0x6C7E,
    0xA50A, 0xB483, 0x8618, 0x9791, 0xE32E, 0xF2A7, 0xC03C, 0xD1B5,
    0x2942, 0x38CB, 0x0A50, 0x1BD9, 0x6F66, 0x7EEF, 0x4C74, 0x5DFD,
    0xB58B, 0xA402, 0x9699, 0x8710, 0xF3AF, 0xE226, 0xD0BD, 0xC134,
    0x39C3, 0x284A, 0x1AD1, 0x0B58, 0x7FE7, 0x6E6E, 0x5CF5, 0x4D7C,
    0xC60C, 0xD785, 0xE51E, 0xF497, 0x8028, 0x91A1, 0xA33A, 0xB2B3,
    0x4A44, 0x5BCD, 0x6956, 0x78DF, 0x0C60, 0x1DE9, 0x2F72, 0x3EFB,
    0xD68D, 0xC704, 0xF59F, 0xE416, 0x90A9, 0x8120, 0xB3BB, 0xA232,
    0x5AC5, 0x4B4C, 0x79D7, 0x685E, 0x1CE1, 0x0D68, 0x3FF3, 0x2E7A,
    0xE70E, 0xF687, 0xC41C, 0xD595, 0xA12A, 0xB0A3, 0x8238, 0x93B1,
    0x6B46, 0x7ACF, 0x4854, 0x59DD, 0x2D62, 0x3CEB, 0x0E70, 0x1FF9,
    0xF78F, 0xE606, 0xD49D, 0xC514, 0xB1AB, 0xA022, 0x92B9, 0x8330,
    0x7BC7, 0x6A4E, 0x58D5, 0x495C, 0x3DE3, 0x2C6A, 0x1EF1, 0x0F78
};

uint16_t calculateCRC16(uint8_t* data, int length) {
    uint16_t crc = 0xFFFF;
    
    for (int i = 0; i < length; i++) {
        crc = (crc >> 8) ^ crc16_table[(crc ^ data[i]) & 0xFF];
    }
    
    // Intercambiar los bytes del resultado para obtener el formato correcto
    return ((crc & 0xFF) << 8) | ((crc >> 8) & 0xFF);
}

#endif // PROTOCOLO_H
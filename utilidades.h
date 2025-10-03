/**
 * utilidades.h
 * Funciones de utilidad para el emulador Anviz
 */

#ifndef UTILIDADES_H
#define UTILIDADES_H

// ========= FUNCIONES DE UTILIDAD PARA LA INTERFAZ WEB ===========

// Obtener fecha y hora formateada
String getFormattedDateTime() {
  char buffer[30];
  sprintf(buffer, "%04d-%02d-%02d %02d:%02d:%02d", 
    year(), month(), day(), hour(), minute(), second());
  return String(buffer);
}

// Formatear timestamp Anviz a fecha/hora legible
String formatTimestamp(uint32_t timestamp) {
  // El timestamp Anviz es segundos desde 2000-01-01
  // Ajustamos a Unix timestamp (segundos desde 1970-01-01)
  time_t unixTime = timestamp + 946684800;
  
  // Convertir a año, mes, día, etc.
  int yr = year(unixTime);
  int mo = month(unixTime);
  int dy = day(unixTime);
  int hr = hour(unixTime);
  int mn = minute(unixTime);
  int sc = second(unixTime);
  
  char buffer[30];
  sprintf(buffer, "%04d-%02d-%02d %02d:%02d:%02d", yr, mo, dy, hr, mn, sc);
  return String(buffer);
}

// Hacer parpadear el LED para indicar un error
void blinkError(int count) {
  for (int i = 0; i < count; i++) {
    digitalWrite(basicConfig.pin_led, HIGH);
    delay(200);
    digitalWrite(basicConfig.pin_led, LOW);
    delay(200);
  }
}

// ========= FUNCIONES DE UTILIDAD PARA MANEJAR USUARIOS Y REGISTROS ===========
int findUserByCardId(uint32_t cardId) {
  for (int i = 0; i < userCount; i++) {
    if (users[i].cardId == cardId && users[i].isActive) {
      return i;
    }
  }
  return -1; // No encontrado
}

#endif // UTILIDADES_H
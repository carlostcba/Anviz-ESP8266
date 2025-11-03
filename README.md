# Anviz-ESP8266: Emulador de Control de Acceso Anviz para ESP8266

Este proyecto convierte un microcontrolador ESP8266 en un emulador de dispositivo de control de acceso Anviz, totalmente compatible con el software **Anviz CrossChex**. Permite construir un sistema de control de acceso funcional y económico, utilizando un lector de tarjetas RFID con interfaz Wiegand.

## ✨ Características Principales

-   **Emulación de Protocolo Anviz:** Se comunica vía TCP (puerto 5010) para ser detectado y gestionado por CrossChex como si fuera un dispositivo nativo.
-   **Compatibilidad con CrossChex:** Permite la gestión remota de usuarios (alta, baja, modificación) y la descarga de registros de asistencia directamente desde el software oficial.
-   **Lector RFID Wiegand:** Compatible con lectores de tarjetas estándar Wiegand 26 y Wiegand 34.
-   **Interfaz Web de Administración:** Incluye un servidor web para la configuración y monitorización del dispositivo:
    -   **Dashboard:** Muestra el estado del sistema en tiempo real (IP, WiFi, contadores, hora, memoria).
    -   **Gestión de Usuarios:** Lista los usuarios almacenados en el dispositivo.
    -   **Visualizador de Registros:** Muestra los últimos 50 eventos de acceso con el nombre del usuario.
    -   **Configuración del Dispositivo:** Permite cambiar en caliente los pines GPIO, el ID del dispositivo, la duración del relé y programar reinicios automáticos.
    -   **Seguridad:** Protegido con autenticación (usuario y contraseña), con la posibilidad de cambiar las credenciales.
    -   **Mantenimiento:** Funciones para reiniciar el dispositivo, borrar todos los registros y resetear la configuración WiFi.
-   **Persistencia de Datos:** Almacena la configuración, la lista de usuarios y los registros de acceso en la memoria flash (SPIFFS), resistiendo reinicios y cortes de energía.
-   **Control de Acceso Físico:** Activa un relé para controlar una cerradura eléctrica, con duración de apertura configurable.
-   **Sincronización de Hora (NTP):** Mantiene el reloj interno sincronizado con un servidor NTP para asegurar la precisión de los registros de asistencia.
-   **Configuración WiFi Sencilla:** Utiliza **WiFiManager** para una configuración inicial de la red fácil y rápida a través de un portal cautivo.
-   **Manejo No Bloqueante:** El control del LED de estado y el relé se gestiona de forma asíncrona para no interferir con las operaciones principales.
-   **Corrección de Protocolo de Registros:** Se ha implementado una corrección para el desfase de un día en los registros de asistencia al ser descargados por CrossChex, asegurando que las fechas se muestren correctamente.

## ⚙️ Requisitos de Hardware

-   **Microcontrolador:** Placa de desarrollo ESP8266 (ej. NodeMCU, Wemos D1 Mini).
-   **Lector RFID:** Cualquier lector de tarjetas con salida de datos Wiegand (26 o 34 bits).
-   **Módulo de Relé:** Un relé de 5V o 3.3V compatible con los niveles lógicos del ESP8266.
-   **Fuente de Alimentación:** Una fuente de 5V con suficiente corriente para alimentar el ESP8266, el lector y el relé.

## 🔌 Conexiones de Hardware

Los pines por defecto se pueden cambiar desde la interfaz web en la sección "Configuración".

| Componente      | Pin ESP8266 (por defecto) |
| --------------- | ------------------------- |
| Lector Wiegand D0 | `D7`                      |
| Lector Wiegand D1 | `D6`                      |
| Módulo de Relé    | `D1`                      |
| LED de Estado     | `D0`                      |

## 📚 Dependencias de Software

Asegúrate de instalar las siguientes librerías a través del Gestor de Librerías del Arduino IDE:

-   `ArduinoJson` by Benoit Blanchon (v6.x recomendada)
-   `NTPClient` by Fabrice Weinberg
-   `WiFiManager` by tzapu

## 🚀 Instalación y Uso

1.  **Hardware:** Realiza las conexiones de hardware como se indica en la tabla anterior.
2.  **Software:**
    *   Abre el archivo `Anviz-ESP8266.ino` en el Arduino IDE.
    *   Instala las librerías requeridas desde el gestor de librerías.
    *   Selecciona tu placa ESP8266 y el puerto COM correspondiente.
3.  **Carga:** Sube el código a tu placa.
4.  **Configuración WiFi (Primera Vez):**
    *   Al arrancar por primera vez, el dispositivo creará un punto de acceso WiFi llamado **"Anviz-ESP8266-XXYY"**.
    *   Conéctate a esta red desde un teléfono o un ordenador. Se abrirá automáticamente un portal cautivo.
    *   Selecciona tu red WiFi local, introduce la contraseña y haz clic en "Guardar".
    *   El dispositivo se reiniciará y se conectará a la red WiFi proporcionada.
5.  **Uso y Gestión:**
    *   Abre el Monitor Serie (a 115200 baudios) en el Arduino IDE para ver la dirección IP asignada al dispositivo.
    *   Accede a la interfaz web desde un navegador usando la IP del dispositivo.
    *   Se te solicitarán credenciales. Por defecto son **usuario:** `admin`, **contraseña:** `admin`.
    *   En **Anviz CrossChex**, añade un nuevo dispositivo. Introduce su dirección IP y utiliza el puerto por defecto `5010`. Ahora puedes sincronizar usuarios y descargar registros como si fuera un terminal Anviz estándar.

## 📂 Estructura del Proyecto

-   `Anviz-ESP8266.ino`: Lógica principal del programa, `setup()` y `loop()`.
-   `protocolo.h`: Implementación del protocolo de comunicación TCP de Anviz, incluyendo el manejo de comandos y respuestas.
-   `web.h`: Código del servidor web, incluyendo el HTML de todas las páginas y la lógica para la interfaz de administración.
-   `almacenamiento.h`: Funciones para guardar y cargar datos (configuración, usuarios, registros) de forma persistente en la memoria flash (SPIFFS).
-   `estructuras.h`: Definiciones de las estructuras de datos (`User`, `AccessRecord`, `BasicConfig`) utilizadas en el proyecto.
-   `variables.h`: Declaración de todas las variables globales y externas.
-   `utilidades.h`: Funciones auxiliares para tareas comunes como formateo de fecha/hora, búsqueda de usuarios, y manejo de LEDs/relés.

## 💡 Mejoras Futuras / Ideas

Aquí hay algunas ideas para futuras mejoras y expansiones del proyecto:

-   **Soporte Extendido de Comandos Anviz:** Implementar más comandos del protocolo Anviz para una compatibilidad más completa, incluyendo la gestión de huellas dactilares si se integra hardware biométrico.
-   **Actualizaciones OTA (Over-The-Air):** Permitir la actualización del firmware del ESP8266 de forma inalámbrica, facilitando el mantenimiento y la implementación de nuevas características.
-   **Interfaz Web Mejorada:** Desarrollar una interfaz de usuario más dinámica y moderna para la administración web, posiblemente utilizando frameworks frontend ligeros o técnicas de AJAX para una mejor experiencia de usuario.
-   **Integración con Sistemas de Automatización:** Explorar la integración con plataformas de domótica (ej. Home Assistant, MQTT) para permitir el control de acceso y la monitorización desde un sistema centralizado.
-   **Manejo de Errores y Logging Avanzado:** Implementar un sistema de logging más robusto y configurable para facilitar la depuración y el monitoreo del dispositivo en producción.
-   **Opciones de Sincronización de Hora:** Además de NTP, considerar la opción de configurar la hora manualmente a través de la interfaz web o mediante comandos específicos.
-   **Seguridad de la Interfaz Web:** Implementar HTTPS para la interfaz de administración web, protegiendo las credenciales y los datos transmitidos.
-   **Soporte para Múltiples Lectores Wiegand:** Permitir la conexión y gestión de varios lectores Wiegand para escenarios de control de acceso más complejos.
-   **Respaldo de Batería para RTC:** Si la precisión del tiempo es crítica y el dispositivo puede sufrir cortes de energía, considerar la adición de un módulo RTC con batería de respaldo.
-   **Optimización de Memoria y Rendimiento:** Continuar optimizando el uso de memoria y el rendimiento del ESP8266, especialmente si se añaden más características.

## 📝 Autor

-   **Autor:** Oemspot
-   **Fecha de última actualización:** 03-11-2025
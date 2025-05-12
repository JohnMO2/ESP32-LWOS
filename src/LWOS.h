#ifndef LWOS_H
#define LWOS_H

#include <WiFi.h>
#include <ArduinoOTA.h>
#include <esp_task_wdt.h>
#include <EEPROM.h>

// EEPROM storage locations
#define EEPROM_ADDR_FAIL_COUNT 0  // Address for storing failure count
#define EEPROM_ADDR_SSID 1        // Address for SSID storage
#define EEPROM_ADDR_PASS 32       // Address for password storage (offset from SSID)

/**
 * LWOS (Lightweight OS) class for managing system recovery, OTA updates, and watchdog timers.
 */
class LWOS {
public:
    /**
     * Constructor for LWOS.
     * @param failsafeTriggerCount Number of failures before entering failsafe mode (Default: 3)
     * @param watchdogTimeout Watchdog timeout in milliseconds (Default: 1000 ms)
     */
    LWOS(int failsafeTriggerCount = 3, int watchdogTimeout = 1000);

    /**
     * Begins LWOS operation, initializes Wi-Fi, watchdog, and OTA functionality.
     * @param ssid Wi-Fi SSID
     * @param password Wi-Fi Password
     */
    void begin(const char* ssid, const char* password);

    /**
     * Handles routine tasks such as OTA updates and refreshing the watchdog timer.
     * Should be called inside the main loop.
     */
    void handle();

    /**
     * Forces the system into failsafe mode on the next reboot.
     * Sets failure count to max and restarts.
     */
    void panic();

    /**
     * Performs a safe system reboot while resetting the failure count.
     */
    void restart();

    /**
     * Dynamically changes the watchdog timeout value.
     * @param timeoutMs New timeout duration in milliseconds.
     */
    void changeWatchdog(int timeoutMs);

    /**
     * Enables or disables automatic failsafe mode when Wi-Fi fails to connect.
     * @param enable If true, enters failsafe mode; otherwise, attempts fallback AP.
     */
    void UseWifiFailoverPanic(bool enable);
	/**
     * Resets failure count stored in EEPROM.
     * WARNING: Only use this if you fully understand the risk!
     * If used incorrectly, this can bypass the protection against infinite reboot cycles.
     */
    void resetStartCount();
private:
    int failsafeTriggerCount;  // Maximum failures before entering failsafe mode
    int watchdogTimeout;       // Watchdog timeout duration
    bool useWifiFailoverPanic = false;  // Determines Wi-Fi failover behavior

    void initWiFi();           // Initializes Wi-Fi connection
    void startFailsafeMode();  // Triggers failsafe mode with OTA updates only
    void setWatchdog(int coreMask, int timeoutMs); // Initializes watchdog timer
};

#endif

#include "LWOS.h"

LWOS::LWOS(int failsafeTriggerCount, int watchdogTimeout)
    : failsafeTriggerCount(failsafeTriggerCount), watchdogTimeout(watchdogTimeout) {
    
}

void LWOS::begin(const char* ssid, const char* password) {
	EEPROM.begin(64);
	
    int startCount = EEPROM.read(EEPROM_ADDR_FAIL_COUNT);
    EEPROM.write(EEPROM_ADDR_FAIL_COUNT, startCount + 1);

    // Ensure proper storage with fixed-length buffers and null termination
    char ssidBuffer[31] = {0};  
    char passBuffer[31] = {0};
    strncpy(ssidBuffer, ssid, sizeof(ssidBuffer) - 1);
    strncpy(passBuffer, password, sizeof(passBuffer) - 1);

    EEPROM.put(EEPROM_ADDR_SSID, ssidBuffer);
    EEPROM.put(EEPROM_ADDR_PASS, passBuffer);
    EEPROM.commit();

    if (startCount >= failsafeTriggerCount) {
        startFailsafeMode();
    }
	
    initWiFi();
	setWatchdog(0, watchdogTimeout);
    
	
    ArduinoOTA.onEnd([this]() {
        Serial.println("OTA update complete, resetting failure count...");
        this->resetStartCount();
    });
	ArduinoOTA.begin();
    
}

void LWOS::UseWifiFailoverPanic(bool enable) {
    useWifiFailoverPanic = enable;
    Serial.printf("WiFi Failover Panic Mode set to: %s\n", enable ? "ENABLED" : "DISABLED");
}

void LWOS::initWiFi() {
    char ssid[31] = {0}, password[31] = {0};
    EEPROM.get(EEPROM_ADDR_SSID, ssid);
    EEPROM.get(EEPROM_ADDR_PASS, password);

    WiFi.begin(ssid, password);

    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.println("Failed to connect to primary SSID.");

        if (useWifiFailoverPanic) {
            Serial.println("Entering OTA-only failsafe mode...");
            startFailsafeMode();
        } else {
            Serial.println("Attempting fallback Wi-Fi...");
            WiFi.softAP("LWOS_OTA");
        }
    }
}



void LWOS::startFailsafeMode() {
    Serial.println("Entering failsafe mode... OTA updates only.");
	useWifiFailoverPanic = false;
    resetStartCount();
    initWiFi();
    ArduinoOTA.begin();

    while (true) { 
        ArduinoOTA.handle();
        delay(500);
    }
}

void LWOS::setWatchdog(int coreMask, int timeoutMs) {
	Serial.println("Starting Watchdog");
    if (esp_task_wdt_status(NULL) == 261) {
        esp_task_wdt_deinit();
    }

    esp_task_wdt_config_t twdt_config = { timeoutMs, coreMask, true };
    esp_task_wdt_init(&twdt_config);
    esp_task_wdt_add(NULL);
}

void LWOS::changeWatchdog(int timeoutMs) {

    // Remove all subscribed tasks before deinitialization
    esp_task_wdt_delete(NULL);

    // Ensure watchdog is properly deinitialized
    esp_err_t deinitResult = esp_task_wdt_deinit();
    if (deinitResult != ESP_OK) {
        Serial.printf("Watchdog deinitialization failed: %d\n", deinitResult);
    }

    // Define new TWDT configuration with the updated timeout
    esp_task_wdt_config_t twdt_config = {
        .timeout_ms = timeoutMs,
        .idle_core_mask = 0,   // Monitor only Core 0
        .trigger_panic = true  // Enable panic on timeout
    };

    // Reinitialize the watchdog timer
    esp_err_t initResult = esp_task_wdt_init(&twdt_config);
    if (initResult != ESP_OK) {
        Serial.printf("Watchdog update failed: %d\n", initResult);
    }

    // Re-register the current task to the watchdog
    esp_task_wdt_add(NULL);
}


void LWOS::resetStartCount() {
    EEPROM.write(EEPROM_ADDR_FAIL_COUNT, 0);
    EEPROM.commit();
}

void LWOS::panic() {
    EEPROM.write(EEPROM_ADDR_FAIL_COUNT, 254);
    EEPROM.commit();
    ESP.restart();
}

void LWOS::restart() {
    resetStartCount();
    ESP.restart();
}

void LWOS::handle() {
    esp_task_wdt_reset();
    ArduinoOTA.handle();
}

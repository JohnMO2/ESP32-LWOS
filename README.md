# ESP32-LWOS
A simple library I made for my esp32 projects where I am only able to use Arduino OTA. This library ensures you don't accidently lock yourself out of OTA and provides simple watchdog capabilities

Install instructions:
Unzip the folder and add to your libraries folder for Arduino IDE located In your sketch folder (Ussually Documents/ArduinoIDE/libraries)

Example Code:
```
#include <LWOS.h>

LWOS lwos(3, 1000); // this sets number of crashes before booting into OTA only mode to 3, and the watchdog timer to 1000ms

void setup() {
    Serial.begin(115200);
    lwos.UseWifiFailoverPanic(true); // Use if you want it to failover to OTA only mode if it fails to connect to Wifi. This needs to be called before LWOS.begin()
    lwos.begin("SSID", "PASSWORD"); // Set wifi SSID and Password, if it fails to connect to this wifi, it will start its own named "LWOS_OTA". This needs to be at the start of setup() as any code before it is unprotected.
    Serial.println("Hello World!");
}

void loop() {
    lwos.handle(); // Put LWOS.handle() at the start of your main loop
    if (random(0,1000) == 1){ // 1 in 1000 chance to reboot into OTA only mode.
      Serial.println("AHHHH");
      lwos.panic();
      ESP.restart();
    }
    if (random(0,100) == 2){ // 1 in 100 chance to simulate a watchdog timer expiration.
     Serial.println("Simulating Crash"); 
     delay(10000);
    }
    if (random(0,100) == 3){ // 1 in 100 chance to simulate a process taking a long time
      Serial.println("Long processing Time");
      lwos.changeWatchdog(10000); // Changes watchdog timer to a longer time temporarily
      delay(5000);
      lwos.changeWatchdog(1000); // Changes watchdog timer to 1000ms
    }
    if (random(1,400) == 1){ // 1 in 400 chance to restart, a sucsesfull restart will clear the crash count.
      Serial.println("Restarting!");
      lwos.restart(); // Use this instead of ESP.Restart(). ESP.Restart() will count as a crash and will incriment the crash count.
    }
    delay(100);
}
```

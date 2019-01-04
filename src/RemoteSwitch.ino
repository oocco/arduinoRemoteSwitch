#ifndef CONFIG_H
#define CONFIG_H
#include "config.h"
#endif
#ifndef DEVICE_H
#define DEVICE_H
#include "device.h"
#endif
#ifndef HTTPSERVER_H
#define HTTPSERVER_H
#include "httpserver.h"
#endif
#include <WiFiManager.h>

void setup() {
    espio_class = NULL;
    web_interface = NULL;
    Serial.begin(115200);

    //clear eeprom data, enable when first upload
    #define RESET_EEPROM
    #ifdef RESET_EEPROM
    String reset_status;
    EEPMEM::read(reset_status, EP_RESET_MODE, EP_RESET_MODE_MAX);
    Serial.print("reset_status:\t"); Serial.println(reset_status);
    if(reset_status != "1"){
        Serial.println("clear eeprom data");
        EEPMEM::reset();
        EEPMEM::write("1", EP_RESET_MODE);
        //EEPMEM::write("abc", EP_ADMIN_PASSWD);
    }
    #endif

    //Initializing Device
    Serial.println("Initializing device...");
    espio_class = new INIT_ESPIO();

    //Initializing Wifi
    WiFiManager wifiManager;
    Serial.println("Connecting Wifi...");
    wifiManager.autoConnect("ESP_AUTO_AP");

    //File Operation Loading
    SPIFFS.begin();

    //Initializing Webserver
    Serial.println("MDNS.begin(HOST_NAME)");
    MDNS.begin(HOST_NAME);
    
    Serial.println("web_server.begin()");
    web_interface = new WEBINTERFACE(80);
    web_interface->web_server.begin();
    
}

void loop() {
    //web requests
    web_interface->web_server.handleClient();
}

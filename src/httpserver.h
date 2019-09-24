#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <FS.h>

#ifndef CONFIG_H
#define CONFIG_H
#include "config.h"
#endif

#ifndef DEVICE_H
#define DEVICE_H
#include "device.h"
#endif

//#include <Ed25519.h>

class WEBINTERFACE
{
    public:
        WEBINTERFACE(int port);
        ESP8266WebServer web_server;

        static String getContentType(String filename);
        static String getFileType(String filename);

        static String formatBytes(size_t bytes);

        File fsUploadFile;
        static String upload_status;

        static void autoTemperature();

    private:
        static void handleFile(String path);
        static void handleIndex();
        static void handleFileCommand();
        static void handleFileUpload();
        static void handleNotFound();
        static bool getFileList(String & file_json);

        static void handleDeviceCommand();

        static void sendJsonData(String json_in);

        static bool handleValidatePage();
        static void handleValidateCommand();
        static void handleTemperature();
        static bool loginValidate(String key_in);
        static bool setKey(String key_in, String key_old_in);
        static String getKey();
        static String key;
};
/*
class CRYPTO_SIGIN
{
    
};
*/
extern WEBINTERFACE * web_interface;
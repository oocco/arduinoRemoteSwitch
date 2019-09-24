#include <Arduino.h>
#include <EEPROM.h>

#define EEPROM_SIZE 1024 //max is 1024

//Serial send message
#define LOG(String){ Serial.println(String);}

//eeprom save pos
#define EP_RESET_MODE			0    //1 byte = flag
#define EP_STA_SSID				2    //33 bytes 32+1 = string
#define EP_STA_PASSWORD			35   //65 bytes 64 +1 = string
#define EP_AP_SSID				100  //33 bytes 32+1 = string
#define EP_AP_PASSWORD			133  //65 bytes 64 +1 = string
#define EP_ADMIN_NAME           198  //65 bytes 64 +1 = string
#define EP_ADMIN_PASSWD         263  //65 bytes 64 +1 = string
#define DEVICE_ID               328  //65 bytes 64 +1 = string
//393-511
#define DEVICE_TEMPERATURE      393  //65 bytes 64 +1 = string
//empty 458-511
/*
/*DEVICE_IO_STATUS example: 
/*id,status,type,name|id,status,type,name
/* Table: "!" = 33; "|" = 124;
*/
#define DEVICE_IO_STATUS        512  //340 bytes 369 +1 = string
//empty 852-1024

//eeprom save size
#define EP_RESET_MODE_MAX		1
#define EP_STA_SSID_MAX		    33
#define EP_STA_PASSWORD_MAX	    65
#define EP_AP_SSID_MAX			33
#define EP_AP_PASSWORD_MAX		65

#define EP_ADMIN_NAME_MAX       65
#define EP_ADMIN_PASSWD_MAX     65

#define DEVICE_ID_MAX           65
#define DEVICE_IO_STATUS_MAX    340
#define ESPIO_MAX               9
#define DEVICE_TEMPERATURE_MAX  65

#define DEVICE_PWMIO_MAX  5


//web interface information
const char HOST_NAME [] PROGMEM = "RemoteSwitch";

//device information
const int IODEFINE[ESPIO_MAX] = {16, 5, 4, 0, 2, 14, 12, 13, 15}; //device id to gpio


class EEPMEM
{
    public:
        static bool read(String & data, int pos, int size_max);
        static bool write(String data, int pos);
        static bool reset();
};

#include "config.h"

bool EEPMEM::read(String & data, int pos, int size_max)
{
    if((0>size_max>EEPROM_SIZE) || (pos>=EEPROM_SIZE)){
        return false;
    }
    data = "";
    byte b =13;
    int i = 0;
    EEPROM.begin(EEPROM_SIZE);
    while(i<size_max || i<EEPROM_SIZE){
        b = EEPROM.read(pos+i);
        if(b != 0){
            data += (char)b;
        }else{
            break;
        }
        i++;
    }
    EEPROM.end();
    return true;
}

bool EEPMEM::write(String data, int pos)
{
    int data_size;
    int size_max;
    const char * data_char = data.c_str();
    data_size = strlen(data_char);
    //check data type
    switch(pos){
        case EP_RESET_MODE:
            size_max = EP_RESET_MODE_MAX;
            break;
        case EP_STA_SSID:
        case EP_AP_SSID:
            size_max = EP_STA_SSID_MAX;
            break;
        case EP_STA_PASSWORD:
        case EP_AP_PASSWORD:
        case EP_ADMIN_NAME:
        case EP_ADMIN_PASSWD:
        case DEVICE_ID:
            size_max = EP_STA_PASSWORD_MAX;
            break;
        default:
            size_max = EEPROM_SIZE;
            break;
    }
    if(pos+data_size+1 > EEPROM_SIZE || data_size > size_max){
        return false;
    }
    EEPROM.begin(EEPROM_SIZE);
    for(int i=0; i<data_size; i++){
        EEPROM.write(pos+i, data_char[i]);
    }
    EEPROM.write(pos + data_size, 0x00);
    EEPROM.commit();
    EEPROM.end();
    return true;
}

bool EEPMEM::reset()
{
    EEPROM.begin(EEPROM_SIZE);
    for(int i=0; i<EEPROM_SIZE; i++){
        EEPROM.write(i, 0x00);
    }
    EEPROM.commit();
    EEPROM.end();
    return true;
}
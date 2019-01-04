#include "device.h"
#include <ArduinoJson.h>

const size_t bufferSize = JSON_OBJECT_SIZE(5) + 50;
DynamicJsonBuffer jsonBuffer(bufferSize);

//device io function
int inching(int pin){
    //pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
    delay(500);
    digitalWrite(pin, LOW);
    return 0;
}

int selflock(int pin, int & status){
    //pinMode(pin, OUTPUT);
    if(status){
        digitalWrite(pin, HIGH);
    }else{
        digitalWrite(pin, LOW);
    }
    //digitalRead(pin) ? digitalWrite(pin, LOW) : digitalWrite(pin, HIGH);
    return digitalRead(pin);
}

bool pwmWrite(int pin, int & status){
    if(-1<status<1001){
        analogWrite(pin, status);
        return true;
    }
    return false;
}

//class ESPIO method
ESPIO::ESPIO()
{
    id = -1;
    type = -1;
    pin = -1;
}

bool ESPIO::initialize(int id_in, int status_in, int type_in, String name_in)
{
    if((0<=id_in<9) && (espio_class->all_espio_id[id_in] == -1)){
        id = id_in;
        pin = IODEFINE[id];
        type = -1;
        setDevice(status_in, type_in, name_in);
        espio_class->all_espio_id[id_in] = id_in;
        return true;
    }
    return false;
}

bool ESPIO::setDevice(int status_in, int type_in, String name_in)
{
    name = name_in;
    setType(type_in);
    setStatus(status_in);
    return true;
}

bool ESPIO::setId(int id_in){
    if(id_in == -1){
        if(0<=id<ESPIO_MAX){
            espio_class->all_espio_id[id] = -1;
        }
        setStatus(0);
        pinMode(pin, INPUT);
        id = id_in;
        pin = id_in;
        status = 0;
        type = 0;
        name = "";
    }
}

bool ESPIO::setType(int type_in)
{
    if(type == type_in){
        return true;
    }else{
        switch (type_in){
            case INCHING:
            case SELFLOCK:
                pinMode(pin, OUTPUT);
                break;
            case PWMWRITE:
                pinMode(pin, OUTPUT);
                //change pwm range, default: 1023
                analogWriteRange(1000);
                break;
            default:
                break;
        }
    }
    type = type_in;
    return true;
}

bool ESPIO::setStatus(int status_in)
{
    if(status_in == status){
        return true;
    }
    switch (type)
    {
        case INCHING:
            status = inching(pin);
            return true;
            break;
        case SELFLOCK:
            status = selflock(pin, status_in);
            return true;
            break;
        case PWMWRITE:
            if(pwmWrite(pin, status_in)){
                status = status_in;
            }
            return true;
            break;
        default:
            break;
    }
    return false;
}

bool ESPIO::setStatus(int status_in, String & msg_in)
{
    msg_in = "not yet";
    return true;
}

String ESPIO::getJson()
{
    String json;
    json = "{\"id\":";
    json += id;
    json += ",\"name\":\"";
    json += name;
    json += "\",\"pin\":";
    json += pin;
    json += ",\"status\":";
    json += status;
    json += ",\"type\":";
    json += type;
    json += "}";
    return json;
}

//class INIT_ESPIO method
INIT_ESPIO::INIT_ESPIO()
{
    espio_class->initializeDevice();
}

//read device information from eeprom, then initializing all device objects(ESPIO);
//save for espio_class->all_device[];
bool INIT_ESPIO::initializeDevice()
{
    for(int i=0; i<ESPIO_MAX; i++){
        all_espio_id[i] = -1;
    }
    String status;
    if(EEPMEM::read(status, DEVICE_IO_STATUS, DEVICE_IO_STATUS_MAX)){
        //data format: 1!13!0!1!name|2!14!0!1!name
        int temp_id;
        int temp_type;
        int temp_status;
        String temp_name;
        int temp_count = 0;
        
        String temp_str;
        int device_count = 0;

        const char * status_char = status.c_str();
        int data_size = strlen(status_char);
        
        for(int i=0; i<=data_size; i++){
            if((byte)status_char[i] == 124 || i == data_size){
                temp_name = temp_str;
                temp_count = 0;
                temp_str = "";
                if(device_count<ESPIO_MAX){
                    espio_class->all_device[device_count].initialize(temp_id, temp_status, temp_type, temp_name);
                    device_count++;
                }
                continue;
            }
            if((byte)status_char[i] == 33){
                switch(temp_count){
                    case 0:
                        temp_id = temp_str.toInt();
                        break;
                    case 2:
                        temp_type = temp_str.toInt();
                        break;
                    case 3:
                        temp_status = temp_str.toInt();
                        break;
                    default:
                    break;
                }
                temp_count++;
                temp_str = "";
                continue;
            }
            temp_str += status_char[i];
        }
        return true;
    }
    return false;
}

bool INIT_ESPIO::saveDevice()
{
    String eeprom_format;

    for(int i=0; i<ESPIO_MAX; i++){
        eeprom_format += espio_class->all_device[i].getId();
        eeprom_format += "!";
        eeprom_format += espio_class->all_device[i].getPin();
        eeprom_format += "!";
        eeprom_format += espio_class->all_device[i].getType();
        eeprom_format += "!";
        eeprom_format += espio_class->all_device[i].getStatus();
        eeprom_format += "!";
        eeprom_format += espio_class->all_device[i].getName();
        if(i+1!=ESPIO_MAX){
            eeprom_format += "|";
        }
    }
    Serial.print("eeprom_format:\t");
    Serial.println(eeprom_format);
    if(EEPMEM::write(eeprom_format, DEVICE_IO_STATUS)){
        return true;
    }

    return false;
}

bool INIT_ESPIO::delete_device(int id_in)
{
    if(0 <= id_in < ESPIO_MAX){
        for(int i=0; i<ESPIO_MAX; i++){
            if(espio_class->all_device[i].getId() == id_in){
                espio_class->all_device[i].setId(-1);
                return true;
            }
        }
    }
    return false;
}

String INIT_ESPIO::getJson()
{
    String all_json;
    all_json += "[";
    for(int i=0; i<ESPIO_MAX; i++){
        all_json += espio_class->all_device[i].getJson();
        if(i+1!=ESPIO_MAX){
            all_json += ",";
        }
    }
    all_json += "]";
    return all_json;
}

bool INIT_ESPIO::setJsonSingle(String json_in)
{
    JsonObject& root = jsonBuffer.parseObject(json_in);

    int id_in = root["id"];
    String name_in = root["name"];
    int pin_in = root["pin"];
    int status_in = root["status"];
    int type_in = root["type"];

    if(id_in==0 && name_in=="" && pin_in==0 && type_in==0){
        return false;
    }
    //If device already setting
    for(int i = 0; i<ESPIO_MAX; i++){
        if(espio_class->all_device[i].getId() == id_in){
            if(espio_class->all_device[i].setDevice(status_in, type_in, name_in)){
                return true;
            }
        }
    }
    //Set a new device
    for(int i = 0; i<ESPIO_MAX; i++){
        if(espio_class->all_device[i].getId() == -1){
            if(espio_class->all_device[i].initialize(id_in, status_in, type_in, name_in)){
                return true;
            }
        }
    }
    return false;
}


bool INIT_ESPIO::setJson(String json_in)
{
    String temp_single;
    json_in.replace("[","");
    json_in.replace("]","");
    const char * data_char = json_in.c_str();
    int data_size = strlen(data_char);
    String temp_data;
    int count_id = 0;


    for(int i=0; i<=data_size; i++){
        if(((byte)data_char[i] == 44 && (byte)data_char[i-1] == 125) || i==data_size){
            espio_class->setJsonSingle(temp_single);
            count_id++;
            temp_single = "";
            continue;
        }
        temp_single += data_char[i];
    }

    return true;
}

bool INIT_ESPIO::setStatus(int id_in, int & status_in)
{
    for(int i=0; i<ESPIO_MAX; i++){
        if(espio_class->all_device[i].getId() == id_in){
            bool set_status = false;
            set_status = espio_class->all_device[i].setStatus(status_in);
            status_in = espio_class->all_device[i].getStatus();
            return set_status;
        }
    }
    return false;
}

bool INIT_ESPIO::setStatus(int id_in, int & status_in, String & msg_in)
{
    for(int i=0; i<ESPIO_MAX; i++){
        if(espio_class->all_device[i].getId() == id_in){
            bool set_status = false;
            set_status = espio_class->all_device[i].setStatus(status_in, msg_in);
            status_in = espio_class->all_device[i].getStatus();
            return set_status;
        }
    }
    return false;
}

int INIT_ESPIO::all_espio_id[ESPIO_MAX] = {};
ESPIO INIT_ESPIO::all_device[ESPIO_MAX] = {};

INIT_ESPIO * espio_class;

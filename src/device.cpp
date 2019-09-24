#include "device.h"
#include <ArduinoJson.h>

const size_t bufferSize = JSON_OBJECT_SIZE(5) + 50;
DynamicJsonBuffer jsonBuffer(bufferSize);

float NTC_A = 1.11492089e-3;
float NTC_B = 2.372075385e-4;
float NTC_C = 6.954079529e-8;
double NTC_Vcc = 3.225;
unsigned int NTC_Rs = 2000;

int AnalogRead(){
  int val = 0;
  for(int i=0; i<20; i++){
    val +=  analogRead(A0);
    delay(1);
  }
  val = val/20;
  return val;
}

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

int analogTemperature(){
    
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
    if((0<=id_in<ESPIO_MAX) && (espio_class->all_espio_id[id_in] == -1)){
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
        Serial.print("device_eeprom_read: \t");
        Serial.println(status);

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

    if (!root.success()) {
        Serial.println("auto_tempereture_class->setJson: failed");
        return false;
    }
    if(!root.containsKey("id") || !root.containsKey("name") || !root.containsKey("pin") || !root.containsKey("status") || !root.containsKey("type")){
        return false;
    }

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

int INIT_ESPIO::getType(int id_in)
{
    for(int i=0; i<ESPIO_MAX; i++){
        if(espio_class->all_device[i].getId() == id_in){
            return espio_class->all_device[i].getType();
        }
    }
    return -1;
}




//class AUTO_TEMPERATURE method
AUTO_TEMPERATURE::AUTO_TEMPERATURE()
{
    for(int i=0; i<DEVICE_PWMIO_MAX; i++){
        auto_tempereture_class->auto_espio[i] = -1;
    }

    auto_tempereture_class->initializeTemperatureAuto();
}

bool AUTO_TEMPERATURE::resetEEROM()
{
    String json_auto = "{\"temperature\":[40,50,60,65],\"speed\":[0,10,30,100],\"status\":1,\"espid\":[-1,-1,-1,-1,-1]}";
    auto_tempereture_class->setJson(json_auto);
    return true;
}

bool AUTO_TEMPERATURE::initializeTemperatureAuto()
{
    String status;
    if(EEPMEM::read(status, DEVICE_TEMPERATURE, DEVICE_TEMPERATURE_MAX)){
        //data format: 40!50!60!70    |       0!30!50!100   |       0       !   1!2!3!4!5      |
        //temperature  A  B  C  D       speed A  B  C  D        status    espid  A B C D E    end

        Serial.print("temperature_eeprom_read: \t");
        Serial.println(status);

        int temp_count = 0;
        int temp_type = 0;
        const char * status_char = status.c_str();
        int data_size = strlen(status_char);

        String temp_str = "";
        
        for(int i=0; i<=data_size; i++){
            if((byte)status_char[i] == 33 || (byte)status_char[i] == 124){
                switch (temp_type){
                    case 0:
                        auto_tempereture_class->temperature_value[temp_count] = temp_str.toInt();
                        break;
                    case 1:
                        auto_tempereture_class->temperature_speed[temp_count] = temp_str.toInt();
                        break;
                    case 2:
                        if(temp_str.toInt() == 1){
                            auto_tempereture_class->auto_status = true;
                        }else{
                            auto_tempereture_class->auto_status = false;
                        }
                        break;
                    case 3:
                        auto_tempereture_class->setTemperatureAutoEspio(temp_str.toInt());
                        
                        break;
                    default:
                        break;
                }
                temp_str = "";
            }

            if((byte)status_char[i] == 33){
                temp_count++;
                continue;

            }else if((byte)status_char[i] == 124){
                temp_count = 0;
                temp_type++;
                continue;
            }

            temp_str += status_char[i];
            
        }
        auto_tempereture_class->last_temperature = -1;
        auto_tempereture_class->reloadTemperatureAuto();
        return true;
    }
    return false;
}

bool AUTO_TEMPERATURE::reloadTemperatureAuto()
{
    //get device temperature
    int temp_temperature = auto_tempereture_class->get_temperature();
    //Serial.print("temp_temperature:\t");
    //Serial.println(temp_temperature);

    //set temperature = 100 when value error
    if(temp_temperature == -1){
        temp_temperature = 100;
    }
    //jump program command when temperature equal last value
    if(temp_temperature == auto_tempereture_class->last_temperature){
        return true;
    }
    auto_tempereture_class->last_temperature = temp_temperature;

    double temp_speed = 0;
    int temp_point = 0;

    int step_temperature = 0;
    double step_speed = 0;

    //get speed value
    for(int i=0; i<4; i++){
        if(auto_tempereture_class->temperature_value[i]>temp_temperature){
            temp_point = i;
            break;
        }else if(i == 3){
            temp_point = 4;
        }
        temp_speed = auto_tempereture_class->temperature_speed[i];
    }
    //Serial.print("temp_point:\t");
    //Serial.println(temp_point);
    if(temp_point==0){
        temp_speed = 0;
    }else if(temp_point != 4){
        step_temperature = auto_tempereture_class->temperature_value[temp_point]-auto_tempereture_class->temperature_value[(temp_point-1)];
        step_speed = auto_tempereture_class->temperature_speed[temp_point]-auto_tempereture_class->temperature_speed[(temp_point-1)];
        step_speed = step_speed/step_temperature;

        step_temperature = temp_temperature - auto_tempereture_class->temperature_value[(temp_point-1)];

        temp_speed = temp_speed + (step_speed*step_temperature);
    }
    //Serial.print("step_speed:\t");
    //Serial.println(step_speed);

    //set speed = 100 when value error
    if(temp_speed<0 || temp_speed>100){
        temp_speed = 100;
    }

    int temp_speed_int = temp_speed*10;
    auto_tempereture_class->last_speed = temp_speed_int;
    //Serial.print("temp_speed:\t");
    //Serial.println(temp_speed_int);
    //Serial.println("");

    //set espid status
    if(auto_tempereture_class->auto_status){
        for(int i=0; i<DEVICE_PWMIO_MAX; i++){
            if(auto_tempereture_class->auto_espio[i] == -1){
                continue;
            }
            espio_class->setStatus(auto_tempereture_class->auto_espio[i], temp_speed_int);
        }
        return true;
    }
    return false;
}

bool AUTO_TEMPERATURE::saveTemperatureAuto()
{
    String eeprom_format = "";
    //40!50!60!70|0!30!50!100|0|1!2!3!4!5|
    for(int i=0; i<4; i++){
        eeprom_format += auto_tempereture_class->temperature_value[i];
        if(i==3){break;}
        eeprom_format += "!";
    }
    eeprom_format += "|";
    for(int i=0; i<4; i++){
        eeprom_format += auto_tempereture_class->temperature_speed[i];
        if(i==3){break;}
        eeprom_format += "!";
    }
    eeprom_format += "|";
    if(auto_tempereture_class->auto_status){
        eeprom_format += "1";
    }else{
        eeprom_format += "0";
    }
    
    eeprom_format += "|";
    for(int i=0; i<DEVICE_PWMIO_MAX; i++){
        eeprom_format += auto_tempereture_class->auto_espio[i];
        if((i+1)==DEVICE_PWMIO_MAX){break;}
        eeprom_format += "!";
    }

    eeprom_format += "|";

    Serial.print("temperature_eeprom_format:\t");
    Serial.println(eeprom_format);
    if(EEPMEM::write(eeprom_format, DEVICE_TEMPERATURE)){
        return true;
    }
    return false;
}

bool AUTO_TEMPERATURE::setTemperatureAutoEspio(int id_in)
{
    if(id_in == -1){
        return false;
    }

    for(int i=0; i<DEVICE_PWMIO_MAX; i++){
        if(id_in == auto_tempereture_class->auto_espio[i]){
            return false;
            break;
        }
    }

    for(int i=0; i<DEVICE_PWMIO_MAX; i++){
        if(auto_tempereture_class->auto_espio[i] == -1){
            if(espio_class->getType(id_in) == PWMWRITE){
                auto_tempereture_class->auto_espio[i] = id_in;
                return true;
                //break;
            }
        }
    }
    return false;
}

bool AUTO_TEMPERATURE::setJson(String json_in)
{
    JsonObject& root = jsonBuffer.parseObject(json_in);

    if (!root.success()) {
        Serial.println("auto_tempereture_class->setJson: failed");
        return false;
    }
    if(root.containsKey("temperature")){
        for(int i=0; i<4; i++){
            auto_tempereture_class->temperature_value[i] = root["temperature"][i];
        }
    }

    if(root.containsKey("speed")){
        for(int i=0; i<4; i++){
            auto_tempereture_class->temperature_speed[i] = root["speed"][i];
        }
    }

    if(root.containsKey("status")){
        if(root["status"] == 1){
            auto_tempereture_class->auto_status = true;
        }else{
            auto_tempereture_class->auto_status = false;
        }
    }

    if(root.containsKey("espid")){
        //clear all espid
        for(int i=0; i<DEVICE_PWMIO_MAX; i++){
            auto_tempereture_class->auto_espio[i] = -1;
        }
        for(int i=0; i<DEVICE_PWMIO_MAX; i++){
            auto_tempereture_class->setTemperatureAutoEspio(root["espid"][i]);
        }
    }

    auto_tempereture_class->last_temperature = -1;
    auto_tempereture_class->reloadTemperatureAuto();
    
    return true;
}

int AUTO_TEMPERATURE::get_temperature()
{
    double V_NTC = ((double)AnalogRead()*3.1)/1024;
    double R_NTC = (NTC_Rs*V_NTC)/(NTC_Vcc-V_NTC);

    R_NTC = log(R_NTC);
    double Temp = 1/(NTC_A+(NTC_B+(NTC_C*R_NTC*R_NTC))* R_NTC);
    Temp = Temp-273.15;

    if(0<Temp<120){
        return (int)Temp;
    }
    return -1;
    //return rand() % 100;
}

String AUTO_TEMPERATURE::getJson()
{
    String json;
    json = "{\"temperature\":";
    json += "[";
    for(int i=0; i<4; i++){
        json += auto_tempereture_class->temperature_value[i];
        if(i==3){break;}
        json += ",";
    }
    json += "]";

    json += ",\"speed\":";
    json += "[";
    for(int i=0; i<4; i++){
        json += auto_tempereture_class->temperature_speed[i];
        if(i==3){break;}
        json += ",";
    }
    json += "]";

    json += ",\"status\":";
    if(auto_tempereture_class->auto_status){
        json += "1";
    }else{
        json += "0";
    }

    json += ",\"espid\":";
    json += "[";
    for(int i=0; i<DEVICE_PWMIO_MAX; i++){
        json += auto_tempereture_class->auto_espio[i];
        if((i+1)==DEVICE_PWMIO_MAX){break;}
        json += ",";
    }

    json += "]";
    json += "}";

    Serial.print("temperature_json_format:\t");
    Serial.println(json);

    return json;
}

int INIT_ESPIO::all_espio_id[ESPIO_MAX] = {};
ESPIO INIT_ESPIO::all_device[ESPIO_MAX] = {};

int AUTO_TEMPERATURE::temperature_value[4] = {};
int AUTO_TEMPERATURE::temperature_speed[4] = {};
int AUTO_TEMPERATURE::auto_espio[DEVICE_PWMIO_MAX] = {};

bool AUTO_TEMPERATURE::auto_status = false;
int AUTO_TEMPERATURE::last_temperature = -1;
int AUTO_TEMPERATURE::last_speed = -1;

INIT_ESPIO * espio_class;
AUTO_TEMPERATURE * auto_tempereture_class;
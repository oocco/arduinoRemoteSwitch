#ifndef CONFIG_H
#define CONFIG_H
#include "config.h"
#endif

#define INCHING 1
#define SELFLOCK 2
#define PWMWRITE 3

class ESPIO
{
    public:
        ESPIO();
        int getId(){return id;};
        int getPin(){return pin;};
        String getName(){return name;};
        int getType(){return type;};
        int getStatus(){return status;};

        bool setId(int id_in);
        void setName(String name_in){name=name_in;return;};
        bool setType(int type_in);
        bool setStatus(int status_in);
        bool setStatus(int status_in, String & msg_in);

        String getJson();

        bool initialize(int id_in, int status_in, int type_in, String name_in);
        bool setDevice(int status_in, int type_in, String name_in);
    private:
        int id;
        int pin;
        String name;
        int type;
        int status;
};

class INIT_ESPIO
{
    public:
        INIT_ESPIO();
        static ESPIO all_device[ESPIO_MAX];
        static int all_espio_id[ESPIO_MAX];

        static bool initializeDevice();
        static bool saveDevice();
        static bool reloadDevice();
        static bool delete_device(int id_in);

        static bool setJsonSingle(String json_in);
        static bool setJson(String json_in);
        static bool setStatus(int id_in, int & status_in);
        static bool setStatus(int id_in, int & status_in, String & msg_in);

        static int getType(int id_in);

        String getJson();
};

class AUTO_TEMPERATURE
{
    public:
        AUTO_TEMPERATURE();
        static int temperature_value[4];
        static int temperature_speed[4];
        static int auto_espio[DEVICE_PWMIO_MAX];
        static bool auto_status;
        static int last_temperature;
        static int last_speed;

        static bool resetEEROM();
        static bool initializeTemperatureAuto();
        static bool reloadTemperatureAuto();
        static bool saveTemperatureAuto();
        static bool setTemperatureAutoEspio(int id_in);

        static bool setJson(String json_in);

        static int get_temperature();

        String getJson();
};

extern INIT_ESPIO * espio_class;
extern AUTO_TEMPERATURE * auto_tempereture_class;
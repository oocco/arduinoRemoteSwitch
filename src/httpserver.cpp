#include "httpserver.h"

const char* indexUpload = "\
    <form method='POST' action='/files' enctype='multipart/form-data'>\
        <input type='text' name='validate' value='' placeholder='Password, if have'>\
        <input type='file' name='upload'>\
        <input type='submit' value='Upload'>\
    </form>";

WEBINTERFACE::WEBINTERFACE(int port):web_server(port)
{
    key = getKey();
    web_server.on("/", HTTP_ANY, handleIndex);
    web_server.on("/files", HTTP_ANY, handleFileCommand, handleFileUpload);
    web_server.on("/device", HTTP_ANY, handleDeviceCommand);
    web_server.on("/validate", HTTP_ANY, handleValidateCommand);
    web_server.onNotFound(handleNotFound);
    fsUploadFile = (File)0;
    upload_status = "";
}

bool WEBINTERFACE::handleValidatePage()
{
    if(web_interface->web_server.hasArg("validate")){
        String key_in = web_interface->web_server.arg("validate");
        if(loginValidate(key_in)){
            return true;
        }
    }
    if(loginValidate("")){return true;};
    //sendJsonData("{\"login\": 0}");
    web_interface->web_server.sendHeader("Cache-Control", "no-cache");
    web_interface->web_server.sendHeader("Access-Control-Allow-Origin", "*");
    web_interface->web_server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    web_interface->web_server.sendHeader("Access-Control-Allow-Methods", "GET,POST");
    web_interface->web_server.send(401, "text/plain", "401 Unauthorized");
    return false;
}

void WEBINTERFACE::handleValidateCommand()
{
    if(!handleValidatePage()){return;};

    String status;
    String result = "false";
    String key_org = key;
    if(web_interface->web_server.hasArg("action")){
        String action = web_interface->web_server.arg("action");
        if(action == "set_password"){
            if(web_interface->web_server.hasArg("password")){
                String password = web_interface->web_server.arg("password");
                if(setKey(password, key_org)){
                    status = "Password Setting";
                    result = "true";
                }else{
                    status = "Setting Failed";
                }
            }else{
                if(setKey("", key_org)){
                    status = "Password Already Deleted";
                    result = "true";
                }else{
                    status = "Delete Failed";
                }
            }
        }
    }
    String validate_json = "{\"validate\":";
    validate_json += result;
    validate_json += ",\"status\":\"";
    validate_json += status;
    validate_json += "\"}";
    sendJsonData(validate_json);
}

void WEBINTERFACE::handleFile(String path)
{
    String contentType = getContentType(path);
    String pathWithGz = path + ".gz";
    if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){
        if(SPIFFS.exists(pathWithGz)){
            path = pathWithGz;
        }
        File file = SPIFFS.open(path, "r");
        web_interface->web_server.streamFile(file, contentType);
        file.close();
        return;
    }else if(!(SPIFFS.exists("/index.html") || SPIFFS.exists("/index.html.gz"))){
        //if no index file, jump to upload page
        web_interface->web_server.sendHeader("Connection", "close");
        web_interface->web_server.send(200, "text/html", indexUpload);
        return;
    }
    //Default, return 404 found
    web_interface->web_server.send(404, "text/plain", "404 Not Found");
    return;
}

void WEBINTERFACE::handleNotFound()
{
    //if(!handleValidatePage()){return;};
    String path = web_interface->web_server.urlDecode(web_interface->web_server.uri());
    handleFile(path);
    return;
}

void WEBINTERFACE::handleIndex()
{
    //if(!handleValidatePage()){return;};
    //SPIFFS.begin();
    handleFile("/index.html");
    return;
}

void WEBINTERFACE::sendJsonData(String json_in)
{
    web_interface->web_server.sendHeader("Cache-Control", "no-cache");
    web_interface->web_server.sendHeader("Access-Control-Allow-Origin", "*");
    web_interface->web_server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    web_interface->web_server.sendHeader("Access-Control-Allow-Methods", "GET,POST");

    web_interface->web_server.send(200, "application/json", json_in);
}

void WEBINTERFACE::handleFileCommand()
{
    if(!handleValidatePage()){return;};
    String status = "Refresh Files Success";

    //Check upload status first
    if(upload_status != ""){
        status = upload_status;
    }
    
    if(web_interface->web_server.hasArg("action")){
        String action = web_interface->web_server.arg("action");
        if(action == "delete"){
            if(web_interface->web_server.hasArg("filename")){
                String filename = web_interface->web_server.arg("filename");
                if(SPIFFS.exists(filename)){
                    SPIFFS.remove(filename);
                    status = filename + " deleted";
                }else{
                    status = filename + " does not exists!";
                }
            }else{
                status = action + ": Must provide a filename";
            }
        }else{
            status = action + " not realize";
        }
    }
    
    //Get SPIFFS file list
    String file_json;
    getFileList(file_json);
    //Add status
    file_json = file_json.substring(0,file_json.length()-1);
    file_json += ",\"status\":\"";
    file_json += status;
    file_json += "\"}";

    //send jsonfile to the client
    sendJsonData(file_json);
    upload_status = "";
    return;
}

bool WEBINTERFACE::getFileList(String & file_json)
{
    Dir dir = SPIFFS.openDir("/");
    int count = 0;
    String filename;
    size_t file_size;
    String file_type;

    String file_json_list;

    file_json_list = "[";

    while(dir.next()){
        File entry = dir.openFile("r");

        filename = dir.fileName();
        file_size = dir.fileSize();

        if (file_json_list != "["){
            file_json_list += ",";
        }
        file_json_list += "{\"id\":\"";
        file_json_list += count;
        file_json_list += "\",\"type\":\"";
        file_json_list += getFileType(filename);
        file_json_list += "\",\"name\":\"";
        file_json_list += filename;
        file_json_list += "\",\"size\":\"";
        file_json_list += formatBytes(file_size);
        file_json_list += "\"}";

        entry.close();
        count ++;
    }
    file_json_list += "]";

    FSInfo info;
    SPIFFS.info(info);

    size_t totalBytes = info.totalBytes;
    size_t usedBytes = info.usedBytes;

    file_json = "{\"files\":" + file_json_list +",";
    file_json += "\"total\":\"" + formatBytes(totalBytes) + "\",";
    file_json += "\"used\":\"" + formatBytes(usedBytes) + "\",";
    file_json.concat(F("\"occupation\":\""));
    file_json += 100 * usedBytes / totalBytes;
    file_json += "\"";
    file_json += "}";
    return true;
}

void WEBINTERFACE::handleFileUpload()
{
    if(!handleValidatePage()){return;};
    String filename;
    HTTPUpload& upload = (web_interface->web_server).upload();
    if(upload.status == UPLOAD_FILE_START){
        filename = upload.filename;
        if(!filename.startsWith("/")){
            filename = "/" + filename;
        }
        web_interface->fsUploadFile = SPIFFS.open(filename, "w");
        //filename = String();
        upload_status = "Start Upload";
    }else if(upload.status == UPLOAD_FILE_WRITE){
        upload_status = "ing";
        //Serial.print("handleFileUpload Data: "); Serial.println(upload.currentSize);
        if(web_interface->fsUploadFile){
            web_interface->fsUploadFile.write(upload.buf, upload.currentSize);
        }
    }else if(upload.status == UPLOAD_FILE_END){
        if(web_interface->fsUploadFile){
            filename = web_interface->fsUploadFile.name();
            web_interface->fsUploadFile.close();
        }
        if(SPIFFS.exists(filename)){
            File file = SPIFFS.open(filename, "r");
            if(upload.totalSize == file.size()){
                upload_status = "Upload Completed";
                file.close();
            }else{
                upload_status = "Failed: File Size Error";
                file.close();
                SPIFFS.remove(filename);
            }
        }else{
            upload_status = "Upload Failed";
        }
        //Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);
    }
}

void WEBINTERFACE::handleDeviceCommand()
{
    if(!handleValidatePage()){return;};
    String device_command_json;
    String msg;

    //Send all deivce json to client
    bool send_device = false;

    if(web_interface->web_server.hasArg("list")){
        send_device = true;
    }

    device_command_json = "{\"deviceList\":";
    //INIT_ESPIO::setStatus(id_in, status_in);
    //INIT_ESPIO::setStatus(id_in, status_in, msg_in);
    if(web_interface->web_server.hasArg("action")){
        String action = web_interface->web_server.arg("action");
        int id_in;
        int status_in;
        String json_in;
        String msg_in;
        
        //Set device status
        if((action == "status_single")){
            if(web_interface->web_server.hasArg("id") && web_interface->web_server.hasArg("status")){
                id_in = web_interface->web_server.arg("id").toInt();
                status_in = web_interface->web_server.arg("status").toInt();
                if(web_interface->web_server.hasArg("msg")){
                    msg_in = web_interface->web_server.arg("msg");
                    if(!(espio_class->setStatus(id_in, status_in, msg_in))){
                        msg == "Set Device Failed";
                    };
                }else{
                    if(!(espio_class->setStatus(id_in, status_in))){
                        msg == "Set Device Failed";
                    };
                }
                device_command_json.replace("deviceList", "deviceStatus");
                msg = "Set Device Success";
                device_command_json += "{\"id\":";
                device_command_json += id_in;
                device_command_json += ",\"status\":";
                device_command_json += status_in;
                device_command_json += ",\"msg\":\"";
                device_command_json += msg_in;
                device_command_json += "\"}";
            }else{
                msg = "Must have Device ID and Status";
            }
            send_device = false;

        //Set device json like {"id":1,"name":"name01","pin":5,"status":0,"type":1}
        }else if(action == "device_single"){
            if(web_interface->web_server.hasArg("device")){
                json_in = web_interface->web_server.arg("device");
                json_in.replace("!", "");
                json_in.replace("|", "");
                if(espio_class->setJsonSingle(json_in)){
                    msg = "Set Device Success";
                }else{
                    msg = "Set Device have Error, Please Check Json";
                }
            }else{
                msg = "Must have Device Json";
            }
            send_device = true;

        //Set a device json list
        }else if(action == "device_list"){
            if(web_interface->web_server.hasArg("device")){
                json_in = web_interface->web_server.arg("device");
                json_in.replace("!", "");
                json_in.replace("|", "");
                if(espio_class->setJson(json_in)){
                    msg = "Set Device Success";
                }else{
                    msg = "Set Device have Error, Please Check Json";
                }
            }else{
                msg = "Must have Device ID and Status";
            }
            send_device = true;
        
        //Delete a device
        }else if(action == "delete_device"){
            if(web_interface->web_server.hasArg("id")){
                id_in = web_interface->web_server.arg("id").toInt();
                if(espio_class->delete_device(id_in)){
                    msg = "Delete Success, ID: ";
                    msg += id_in;
                }else{
                    msg = "Delete Failed, ID: ";
                    msg += id_in;
                }
                send_device = true;
            }else{
                msg = "Must have Device ID";
                device_command_json += "\"\"";
                send_device = false;
            }
        }else if(action == "save_eeprom"){
            device_command_json.replace("deviceList", "deviceSave");
            if(espio_class->saveDevice()){
                device_command_json += "true";
                msg = "Save Device Success";
            }else{
                device_command_json += "false";
                msg = "Save Failed";
            }
            send_device = false;
        }else if(action == "clear_eeprom"){
            EEPMEM::reset();
            msg = "Reset Success, Please Reboot";
            device_command_json += "\"\"";
        //Default
        }else{
            msg = "Request Error";
            device_command_json += "\"\"";
        }
    }
    if(send_device){
        device_command_json += espio_class->getJson();
    }
    device_command_json += ",\"status\":\"";
    device_command_json += msg;
    device_command_json += "\"}";

    sendJsonData(device_command_json);
}

String WEBINTERFACE::getContentType(String filename)
{
    if(filename.endsWith(".htm")) { return "text/html"; } 
    else if(filename.endsWith(".html")) { return "text/html"; } 
    else if(filename.endsWith(".css")) { return "text/css"; } 
    else if(filename.endsWith(".js")) { return "application/javascript"; } 
    else if(filename.endsWith(".png")) { return "image/png"; } 
    else if(filename.endsWith(".gif")) { return "image/gif"; } 
    else if(filename.endsWith(".jpeg")) { return "image/jpeg"; } 
    else if(filename.endsWith(".jpg")) { return "image/jpeg"; } 
    else if(filename.endsWith(".ico")) { return "image/x-icon"; } 
    else if(filename.endsWith(".xml")) { return "text/xml"; } 
    else if(filename.endsWith(".pdf")) { return "application/x-pdf"; } 
    else if(filename.endsWith(".zip")) { return "application/x-zip"; } 
    else if(filename.endsWith(".gz")) { return "application/x-gzip"; } 
    else if(filename.endsWith(".tpl")) { return "text/plain"; } 
    else if(filename.endsWith(".inc")) { return "text/plain"; }
    else if(filename.endsWith(".txt")) { return "text/plain"; }
    return "application/octet-stream";
}

String WEBINTERFACE::getFileType(String filename)
{
    if(filename.endsWith(".htm")) { return "text/html"; } 
    else if(filename.endsWith(".html")) { return ".html"; } 
    else if(filename.endsWith(".css")) { return ".css"; } 
    else if(filename.endsWith(".js")) { return ".js"; } 
    else if(filename.endsWith(".png")) { return ".png"; } 
    else if(filename.endsWith(".gif")) { return ".gif"; } 
    else if(filename.endsWith(".jpg") || filename.endsWith(".jpeg")) { return ".jpg"; } 
    else if(filename.endsWith(".xml")) { return ".xml"; } 
    else if(filename.endsWith(".pdf")) { return ".pdf"; } 
    else if(filename.endsWith(".zip")) { return ".zip"; } 
    else if(filename.endsWith(".gz")) { return ".gz"; } 
    else if(filename.endsWith(".txt")) { return ".txt"; }
    return "file";
}

String WEBINTERFACE::formatBytes(size_t bytes) {
  if (bytes < 1024) {
    return String(bytes) + "B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + "KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  } else {
    return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
  }
}

String WEBINTERFACE::getKey()
{
    String key_str;
    EEPMEM::read(key_str, EP_ADMIN_PASSWD, EP_ADMIN_PASSWD_MAX);
    Serial.print("Password: "); Serial.println(key_str);
    return key_str;
}

bool WEBINTERFACE::loginValidate(String key_in)
{
    if(key == key_in){
        return true;
    }
    return false;
}

bool WEBINTERFACE::setKey(String key_in, String key_in_old)
{
    if(loginValidate(key_in_old)){
        if(EEPMEM::write(key_in, EP_ADMIN_PASSWD)){
            key = key_in;
            return true;
        }
    }
    return false;
}

String WEBINTERFACE::key;

String WEBINTERFACE::upload_status = "";

WEBINTERFACE * web_interface;
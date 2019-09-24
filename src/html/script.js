Vue.component('file-item', {
    props: ['item'],
    template: `
        <tr><td><svg height="24" width="24" viewBox="0 0 24 24"><path 
        d="M1,2 L1,21 L2,22 L16,22 L17,21 L17,6 L12,6 L12,1  L2,1 z" 
        stroke="black" fill="white"></path><line x1="12" y1="1" x2="17" y2="6" 
        stroke="black" stroke-width="1"></line><line x1="5" y1="10" x2="13" y2="10" 
        stroke="black" stroke-width="1"></line><line x1="5" y1="14" x2="13" y2="14" 
        stroke="black" stroke-width="1"></line><line x1="5" y1="18" x2="13" y2="18" 
        stroke="black" stroke-width="1"></line></svg></td><td>{{item.name}}</td><td>
        {{item.size}}</td><td><div v-on:click="$emit(\'delfile\')"><svg width="24" 
        height="24" viewBox="0 0 128 128"><rect x="52" y="12" rx="6" ry="6" width="25" 
        height="7" style="fill:red;"></rect><rect x="52" y="16" width="25" height="2" 
        style="fill:white;"></rect><rect x="30" y="18" rx="6" ry="6" width="67" 
        height="100" style="fill:red;"></rect><rect x="20" y="18" rx="10" ry="10" 
        width="87" height="14" style="fill:red;"></rect><rect x="20" y="29" width="87" 
        height="3" style="fill:white;"></rect><rect x="40" y="43" rx="7" ry="7" width="7" 
        height="63" style="fill:white;"></rect><rect x="60" y="43" rx="7" ry="7" width="7" 
        height="63" style="fill:white;"></rect><rect x="80" y="43" rx="7" ry="7" 
        width="7" height="63" style="fill:white;"></rect></svg></div></td></tr>
    `
});

var vuePanel = new Vue({
    el: '#vuePanel',
    data: {
        //panel value
        panel: 'home', //Switch panel status

        //file panel value
        path: '/', //Folder path
        fileList: [
            //{id: 1, type: 'file', name: '404.html', size: '2 KB'}
        ],

        //temperature value
        temperatureList: {
            temperature: [-1,-1,-1,-1],
            speed: [-1,-1,-1,-1],
            status: -1,
            espid: [-1,-1,-1,-1,-1]
        },
        temperatureValueLast: -1,
        temperatureValue: -1,
        temperatureSpeed: -1,
        temperatureSpeedLast: -1,
        
        temperatureAutoStatus: false, //enable auto temperature control

        temperatureRefreshTime: 3,
        timeSender: null,

        flashInformation: {status: '', total: '', used: '', occupation: 0}, //File status

        serverUri: '', //Url setting
        
        serialNumber: '', //device id

        //Device type
        deviceType: [
            {type: 0, name: "Unknown Device"},
            {type: 1, name: "Inching Switch"},
            {type: 2, name: "Selflock Switch"},
            {type: 3, name: "PWM AnalogWrite"}
        ],

        //Pin setting
        deviceList: [
            //{id:1, name: 'D5', pin: 14, type: 1, status: 0}
        ],

        //add device temp object
        deviceTempObject: {id: null, name: "",pin: -1 , type: 0, status: 0},

        deviceRefreshTime: 12,
        deviceTimeSender: null,
        
        //tips valus
        msg: '', //Alert message
        msgStatus: false, //Show or hide alert box

        //password
        password: '',
        tempPassword: '',
        loginValidate: false,
        ajaxCount: 0,
    },
    computed: {
        fileUri: function(){
            if(this.serverUri){
                return this.serverUri + '/files'
            }
            return '/files';
        }, //files ajax url path
        deviceUri: function(){
            if(this.serverUri){
                return this.serverUri + '/device'
            }
            return '/device';
        }, //command ajax url path
        validateUri: function(){
            if(this.serverUri){
                return this.serverUri + '/validate'
            }
            return '/validate';
        }, //validate ajax url path
        temperatureUri: function(){
            if(this.serverUri){
                return this.serverUri + '/temperature'
            }
            return '/temperature';
        } //temperature ajax url path
        
    },
    created: function(){
        let _this = this;
        _this.initializeLcoalStorge();
        
        //Get device information from Device
        _this.msg = "Refresh List ... ";
        _this.getDevicesList();
        _this.autoRefreshData();

        //Get device information from Device
        let n = new FormData;
        n.append("list", "list");
        _this.ajaxRequest(_this.fileUri, n);

        //_this.temperatureRefresh();
        _this.setAutoRefreshTime();

        _this.getTemperatureList();
    },
    watch: {
        //Show alert message when msg has change
        msg: function(){
            _this = this;
            if(_this.msg != ""){
                _this.msgStatus = true;
                setTimeout(() =>{
                    _this.msg = "";
                    _this.msgStatus = false;
                }, 5000);
            }
        },
        temperatureAutoStatus: function(){
            let _this = this;
            let change = false;
            if(_this.temperatureAutoStatus){
                if(_this.temperatureList.status != 1){change = true;}
                _this.temperatureList.status = 1;
            }else{
                if(_this.temperatureList.status == 1){change = true;}
                _this.temperatureList.status = 0;
            }
            if(change){
                _this.setTemperatureStatus();
            }
        }
    },
    methods: {
        initializeLcoalStorge: function(){
            //Get device information when loading
            let _this = this;
            _this.password = _this.getLocalStorage("password");
            _this.serverUri = _this.getLocalStorage("serverUri");
            _this.serialNumber = _this.getLocalStorage("serialNumber");
            if(_this.getLocalStorage("temperatureRefreshTime") != null){
                _this.temperatureRefreshTime = _this.getLocalStorage("temperatureRefreshTime");
            }
            if(_this.getLocalStorage("deviceRefreshTime") != null){
                _this.deviceRefreshTime = _this.getLocalStorage("deviceRefreshTime");
            }
        },
        //add password and send request
        ajaxRequest: function(uri, n, msg){
            let _this = this;
            n.append("validate", _this.password);
            axios.post(uri, n)
            .then(function(response){
                //refresh device list
                if('temperatureValue' in response.data){
                    _this.temperatureValue = response.data.temperatureValue;

                    if(response.data.temperatureSpeed != _this.temperatureSpeedLast){
                        _this.temperatureSpeed = response.data.temperatureSpeed/10;
                        _this.temperatureSpeedLast = response.data.temperatureSpeed;

                        for(let i=0; i<_this.temperatureList.espid.length; i++){

                            for(let v=0; v<_this.deviceList.length; v++){
                                if(_this.temperatureList.espid[i] == _this.deviceList[v].id){
                                    _this.deviceList[v].status = _this.temperatureSpeedLast;
                                }
                            }

                        }
                        //_this.deviceRefreshDate();
                    }

                //refresh single device status, msg = id
                }else if('deviceStatus' in response.data){
                    _this.setIdStatus(response.data.deviceStatus, msg);
                
                //refresh files list
                }else if('deviceList' in response.data){
                    _this.deviceList = response.data.deviceList;
                    _this.delEmptyDevice();
                    if(response.data.status != ""){
                        _this.msg = response.data.status;
                    }else{
                        _this.msgStatus = false;
                    }

                //refresh temperature list
                }else if('temperatureList' in response.data){
                    _this.temperatureList = response.data.temperatureList;
                    if(_this.temperatureList.status == 1){
                        _this.temperatureAutoStatus = true;
                    }else{
                        _this.temperatureAutoStatus = false;
                    }
                    if(response.data.status != ""){
                        _this.msg = response.data.status;
                    }

                //refresh files list
                
                }else if('files' in response.data){
                    _this.fileList = response.data.files;
                    _this.flashInformation.status = response.data.status;
                    _this.flashInformation.total = response.data.total;
                    _this.flashInformation.used = response.data.used;
                    _this.flashInformation.occupation = response.data.occupation / 100;
                    _this.msg = response.data.status;

                //Apply New Password
                }else if('validate' in response.data){
                    if(response.data.validate === true){
                        //save to localStorage
                        _this.setLocalStorage("password", _this.password);
                    }
                    _this.msg = response.data.status;

                //default, display alert box
                }else{
                    _this.msg = response.data.status;
                }

                setTimeout(function(){_this.ajaxCount = 0;}, 500);
                
            })
            .catch(function(error){
                _this.msg = "Send Failed! ";
                if(error.status === 401){
                    clearInterval(_this.timeSender);
                    clearInterval(_this.deviceTimeSender);
                    _this.loginValidate = true;
                    //_this.loginAuth();
                }else{
                    console.log(error);
                }
                _this.ajaxCount = 0;
            })
        },

        //device methods
        getDevicesList: function (){
            let _this = this;
            //_this.msg = "Get Device List ... ";
            let n = new FormData;
            n.append("list", "list");
            _this.ajaxRequest(_this.deviceUri, n);
        },
        delEmptyDevice: function(){
            let _this = this;
            let device_list_temp = [];
            let z = 0;
            // delete empty device and sort by id
            for(let i=0; i < _this.deviceList.length; i++){
                if(_this.deviceList[i].id === -1){
                    continue;
                }else{
                    z++;
                    for(let x=0; x < z; x++){
                        if(!(device_list_temp[x])){
                            device_list_temp.push(_this.deviceList[i]);
                            break;
                        }else if(_this.deviceList[i].id < device_list_temp[x].id){
                            device_list_temp.splice(x, 0, _this.deviceList[i]);
                            break;
                        }
                    }
                }
            }
            _this.deviceList = device_list_temp;
        },
        deviceChange: function(){
            let _this = this;
            if(confirm("Confirm Change and Save?")){
                let n = new FormData;
                _this.msg = "Setting Device ... ";
                //console.log(JSON.stringify(_this.deviceList));
                n.append("action", "device_list");
                n.append("device", JSON.stringify(_this.deviceList));
                _this.ajaxRequest(_this.deviceUri, n);
            }            
        },
        addDevice: function(){
            let _this = this;
            for(let i=0; i < _this.deviceList.length; i++){
                if(_this.deviceList[i].id === _this.deviceTempObject.id){
                    _this.msg = "Failed, Device have Used";
                    return false;
                }
            }
            if(_this.deviceTempObject.id >= 0 && _this.deviceTempObject.id != null){
                if(confirm("Confirm Add Device? ID: " +  _this.deviceTempObject.id)){
                    let n = new FormData;
                    _this.msg = "Setting Device ... ";
                    n.append("action", "device_single");
                    n.append("device", JSON.stringify(_this.deviceTempObject));
                    console.log(JSON.stringify(_this.deviceTempObject))
                    _this.ajaxRequest(_this.deviceUri, n);
                }
            }else{
                _this.msg = "ID must be greater than or equal to 0";
            }
        },
        delDevice: function(id){
            let _this = this;
            if(confirm("Confirm Delete Device? ID: " + id)){
                let n = new FormData;
                _this.msg = "Setting Device ... ";
                //console.log(JSON.stringify(_this.deviceList));
                n.append("action", "delete_device");
                n.append("id", id);
                _this.ajaxRequest(_this.deviceUri, n);
            }
        },
        deviceSave: function(){
            let _this = this;
            let n = new FormData;
            n.append("action", "save_eeprom");
            _this.ajaxRequest(_this.deviceUri, n, 2);
        },
        deviceCommand: function(id, status){
            //_this.msg = "Send Command ... ";
            let n = new FormData;
            let _this = this;
            n.append("action", "status_single");
            n.append("id", id);
            n.append("status", status);
            _this.ajaxRequest(_this.deviceUri, n, id)
        },
        deviceInputCommand: function(e){
            let _this = this;
            //control ajax send times
            _this.ajaxCount ++;
            if(_this.ajaxCount<2){
                _this.deviceCommand(e.target.name, e.target.value);
            }else{
                //make sure send last status
                setTimeout(function(){
                    _this.ajaxCount ++;
                    if(_this.ajaxCount<2){
                        _this.deviceCommand(e.target.name, e.target.value);
                    }
                },600);
            }
            
        },
        isDeviceType: function (type, self) {
            if(type === self){
                return true;
            }else{
                return false;
            }
        },
        setIdStatus: function(data, id){
            for(let i=0; i < _this.deviceList.length; i++){
                if(_this.deviceList[i].id === id){
                    _this.deviceList[i].status = data.status;
                    //console.log(data.status);
                }
            }
        },

        getDeviceName: function(id_in){
            let _this = this;
            for(let i=0; i<_this.deviceList.length; i++){
                if(_this.deviceList[i].id == id_in){
                    return _this.deviceList[i].name;
                    break;
                }
            }
            return "Empty";
        },

        //temperature methods
        temperatureRefresh: function(){
            let n = new FormData;
            let _this = this;
            n.append("get_temperature", "1");
            _this.ajaxRequest(_this.temperatureUri, n);
        },
        getTemperatureList: function(){
            let n = new FormData;
            let _this = this;
            n.append("action", "get_list");
            _this.ajaxRequest(_this.temperatureUri, n);
        },
        setTemperatureList: function(){
            let n = new FormData;
            let _this = this;

            let t = _this.temperatureList;
            delete t.espid;

            n.append("action", "set_list");
            n.append("json", JSON.stringify(t));
            _this.ajaxRequest(_this.temperatureUri, n);
        },
        setTemperatureId: function(){
            let n = new FormData;
            let _this = this;

            let t = "{\"espid\":"+ JSON.stringify(_this.temperatureList.espid) +"}";
            n.append("action", "set_espid");
            n.append("json", t);
            _this.ajaxRequest(_this.temperatureUri, n);
        },
        setTemperatureStatus: function(){
            let n = new FormData;
            let _this = this;

            let t = "{\"status\":"+ JSON.stringify(_this.temperatureList.status) +"}";
            n.append("action", "set_status");
            n.append("json", t);
            _this.ajaxRequest(_this.temperatureUri, n);
        },
        saveTemperatureList: function(){
            let n = new FormData;
            let _this = this;
            _this.setTemperatureList();

            n.append("action", "save");
            _this.ajaxRequest(_this.temperatureUri, n);
        },
        resetTemperatureList: function(){
            let n = new FormData;
            let _this = this;
            n.append("action", "reset");
            _this.ajaxRequest(_this.temperatureUri, n);
        },
        setTemperatureEspid: function(id_in, index){
            let _this = this;
            if(id_in != -1){
                _this.$set(_this.temperatureList.espid, index, -1);
            }else{
                let get_id = -1;
                for(let i=0; i<_this.deviceList.length; i++){
                    if (_this.deviceList[i].type == 3) {
                        get_id = _this.deviceList[i].id;
                        for(let v=0; v<_this.temperatureList.espid.length; v++){
                            if(_this.deviceList[i].id == _this.temperatureList.espid[v]){
                                get_id = -1;
                                break;
                            }
                        }
                    }
                    if(get_id != -1){
                        _this.$set(_this.temperatureList.espid, index, get_id);
                        break;
                    }
                }
            }
        },

        displayDeviceList: function(id_in){
            let _this = this;
            if(_this.temperatureAutoStatus){
                for(let i=0; i<_this.temperatureList.espid.length; i++){
                    if(_this.temperatureList.espid[i] == id_in){
                        return false;
                        break;
                    }
                }
            }
            return true;
        },//disable PWM control when auto temperature
        
        //file methods
        delFile: function(file){
            let _this = this;
            if(confirm("Confirm deletion of file: " + file)){
                _this.msg = "Delete \' " + file + " \' ... ";
                let n = new FormData;
                n.append("action", "delete");
                n.append("filename", file);
                _this.ajaxRequest(_this.fileUri, n);
            }
        },
        sendFile: function(){
            let _this = this;
            let t = document.getElementById("fileSelect").files;
            if (0 != t.length) {
                let n = new FormData;
                n.append("validate", _this.password);
                n.append("path", "/");
                for (let i = 0; i < t.length; i++) {
                    let l = t[i];
                    n.append("myfiles[]", l, "/" + l.name)
                }
                _this.ajaxRequest(_this.fileUri, n);
            }else{
                _this.msg = "Please select file! ";
            }
        },

        //get data from local
        setLocalStorage: function(name_in, value_in){
            //console.log("setLocalStorage");
            localStorage.setItem(name_in, value_in);
        },
        getLocalStorage: function(name_in){
            return localStorage.getItem(name_in);
        },

        //validate methods
        setPassword: function(){
            let _this = this;
            let n = new FormData;
            n.append("action", "set_password");
            n.append("password", _this.tempPassword);
            _this.ajaxRequest(_this.validateUri, n);
            _this.password = _this.tempPassword;
        },
        loginAuth: function(){
            let _this = this;
            _this.password = _this.tempPassword;
            _this.loginValidate = false;
            let n = new FormData;
            n.append("list", "list");
            _this.ajaxRequest(_this.deviceUri, n);
            _this.ajaxRequest(_this.fileUri, n);
            _this.setLocalStorage("password", _this.password);
            _this.setAutoRefreshTime();
            _this.getTemperatureList();
        },

        //auto refresh data
        autoRefreshData: function(){
            this.temperatureRefresh();
        },
        toggleAutoRefreshDate: function(){
            let _this = this;
            if(_this.temperatureRefreshTime == 0){
                _this.temperatureRefreshTime = 3;
            }else{
                _this.temperatureRefreshTime = 0;
            }
            _this.setAutoRefreshTime();
        },

        deviceRefreshDate: function(){
            this.getDevicesList();
        },
        toggleDeviceRefreshDate: function(){
            let _this = this;
            if(_this.deviceRefreshTime == 0){
                _this.deviceRefreshTime = 12;
            }else{
                _this.deviceRefreshTime = 0;
            }
            _this.setAutoRefreshTime();
        },

        //change refresh time
        setAutoRefreshTime: function(toggle = false){
            let _this = this;
            _this.setLocalStorage("temperatureRefreshTime", _this.temperatureRefreshTime);
            _this.setLocalStorage("deviceRefreshTime", _this.deviceRefreshTime);
            clearInterval(_this.timeSender);
            clearInterval(_this.deviceTimeSender);
            if(_this.temperatureRefreshTime != 0){
                _this.timeSender = setInterval(function(){_this.autoRefreshData();}, _this.temperatureRefreshTime*1000);
            }
            if(_this.deviceRefreshTime != 0){
                _this.deviceTimeSender = setInterval(function(){_this.deviceRefreshDate();}, _this.deviceRefreshTime*1000);
            }
        }
    }
})
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
        path: '/',  //Folder path
        fileList: [
            //{id: 1, type: 'file', name: '404.html', size: '2 KB'}
        ],
        flashInformation: {status: '', total: '', used: '', occupation: 0}, //File status

        //Url setting
        serverUri: '',

        //device id
        serialNumber: '',

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
    },
    created: function(){
        let _this = this;
        _this.initializeLcoalStorge();
        //Get device information from Device
        _this.msg = "Refresh List ... ";
        let n = new FormData;
        n.append("list", "list");
        _this.ajaxRequest(_this.deviceUri, n);
        _this.ajaxRequest(_this.fileUri, n);
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
        }
    },
    methods: {
        initializeLcoalStorge: function(){
            //Get device information when loading
            let _this = this;
            _this.password = _this.getLocalStorage("password");
            _this.serverUri = _this.getLocalStorage("serverUri");
            _this.serialNumber = _this.getLocalStorage("serialNumber");
        },
        //add password and send request
        ajaxRequest: function(uri, n, msg){
            let _this = this;
            n.append("validate", _this.password);
            axios.post(uri, n)
            .then(function(response){
                //refresh device list
                if('deviceList' in response.data){
                    _this.deviceList = response.data.deviceList;
                    _this.delEmptyDevice();
                    _this.msg = response.data.status;

                //refresh single device status, msg = id
                }else if('deviceStatus' in response.data){
                    _this.setIdStatus(response.data.deviceStatus, msg);

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
                _this.ajaxCount = 0;
            })
            .catch(function(error){
                _this.msg = "Send Failed! ";
                if(error.status === 401){
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
            _this.msg = "Get Device List ... ";
            let n = new FormData;
            n.append("list", "list");
            ajaxRequest(_this.deviceUri, n);
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
            _this = this;
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
            _this = this;
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
                },1000);
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
        }
    }
})
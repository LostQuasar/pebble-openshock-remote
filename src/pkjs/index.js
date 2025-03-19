var Clay = require("pebble-clay");
var clayConfig = require("./config.json");
var clay = new Clay(clayConfig);
var keys = require('message_keys');

Pebble.addEventListener('ready', function() {
    // PebbleKit JS is ready!
    var dict = {}
    dict[keys.js_ready] = 1
    Pebble.sendAppMessage(dict, function () {
    }, function (e) {
        console.log('Message failed: ' + JSON.stringify(e));
    });

  });

Pebble.addEventListener('appmessage', function (e) {
    var dict = e.payload;

    if (dict["shock_str"]) {
        sendShock(dict["shock_str"]);
    }
    if (dict["check_online"]) {
        checkOnlineStatus();
    }

});

function checkOnlineStatus() {
    var dict = {}
    dict[keys.online_status] = checkAPI();

    Pebble.sendAppMessage(dict, function () {
    }, function (e) {
        console.log('Message failed: ' + JSON.stringify(e));
    });
}

function checkAPI() {
    var xhr = new XMLHttpRequest();
    xhr.open(
        "GET",
        "https://api.openshock.app/1",
        false
    );
    xhr.setRequestHeader("accept", "application/json");
    xhr.setRequestHeader("Content-Type", "application/json");
    xhr.send();
    var response = JSON.parse(xhr.responseText);
    if (response["message"] == "OpenShock") {
        return 1
    }
    else {
        return 0
    }
}


function sendShock(shock_str) {
    var settings = JSON.parse(localStorage.getItem("clay-settings"));
    if (settings && settings.api_key && settings.shocker_id && settings.shock_max && settings.shock_dur) {
        api_key = settings.api_key;
        shocker_id = settings.shocker_id;
        shock_max = settings.shock_max;
        shock_dur = settings.shock_dur;
    }
    var xhr = new XMLHttpRequest();
    xhr.open(
        "POST",
        "https://api.openshock.app/2/shockers/control",
        false
    );
    xhr.setRequestHeader("OpenShockToken", api_key);
    xhr.setRequestHeader("accept", "application/json");
    xhr.setRequestHeader("Content-Type", "application/json");
    xhr.send(JSON.stringify({
        shocks: [
            {
                id: shocker_id,
                type: "Shock",
                intensity: shock_str,
                duration: shock_dur,
                exclusive: true
            }
        ],
        customName: "Pebble App"
    }));
    var response = JSON.parse(xhr.responseText);
    if (response["message"] != "Successfully sent control messages") {
        console.log("API Error: ", response["message"])
    } 
};


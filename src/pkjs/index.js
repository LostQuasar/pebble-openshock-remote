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

    if (dict["shock_str"] && dict["shock_dur"]) {
        sendShock(dict["shock_str"], dict["shock_dur"]);
    }
});

function sendShock(shock_str, shock_dur) {
    var settings = JSON.parse(localStorage.getItem("clay-settings"));
    if (settings && settings.api_key && settings.shocker_id) {
        api_key = settings.api_key;
        shocker_id = settings.shocker_id;
    }
    else {
        console.log("VARIABLES UNSET IN CONFIG")
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


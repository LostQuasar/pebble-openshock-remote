var Clay = require("pebble-clay");
var clayConfig = require("./config.json");
var clay = new Clay(clayConfig);


Pebble.addEventListener('appmessage', function (e) {
    var dict = e.payload;

    if (dict["shock_str"]) {
        sendShock(dict["shock_str"]);
    }
});


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
    console.log("Sending Shock: at " + shock_str + " for " + shock_dur);
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
    console.log("API Response: ", response["message"])
};


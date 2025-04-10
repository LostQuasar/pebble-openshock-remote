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
        sendShock(dict["shock_str"], dict["shock_dur"], dict["ID"], dict["mode"]);
    }
});

function sendShock(shock_str, shock_dur, ID, mode) {
    var settings = JSON.parse(localStorage.getItem("clay-settings"));
    if (settings && settings.api_key && settings.shocker_id) {
        api_key = settings.api_key;

		if (ID == 3){
		shocker_id = settings.shocker_id3;
		}
		else if (ID == 2){
		shocker_id = settings.shocker_id2;
		}
		else {
        shocker_id = settings.shocker_id;
		}
		
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
	if (mode == 1) {
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
	}
		else
		{
		xhr.send(JSON.stringify({
			shocks: [
				{
					id: shocker_id,
					type: "Vibrate",
					intensity: shock_str,
					duration: shock_dur,
					exclusive: true
				}
			],
			customName: "Pebble App"
    }));
	}
    var response = JSON.parse(xhr.responseText);
    if (response["message"] != "Successfully sent control messages") {
        console.log("API Error: ", response["message"])
    } 
};


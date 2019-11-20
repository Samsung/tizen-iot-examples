function httpRequestAsync(httpMethod, path, callback) {
	var xmlHttp = new XMLHttpRequest();
	xmlHttp.open(httpMethod, path, true);

	xmlHttp.onload = function (e) {
		if (xmlHttp.readyState === 4) {
			if (xmlHttp.status === 200) {
				callback(xmlHttp.responseText);
			} else {
				console.log("failed to get message : " + xmlHttp.statusText);
			}
		}
	};

	xmlHttp.onerror = function (e) {
		console.log("failed to get message : " + xmlHttp.statusText);
	};

	xmlHttp.send(null);
}

// System Info
function setSystemInfo(jsonStr) {
	var json = JSON.parse(jsonStr);
	var parentElm = document.getElementById("system-information-content");

	var strPreKey = "<div class='col-xl-5 col-lg-5 col-md-10 col-sm-10 col-10 my-3'><div class='card'><div class='card-body'><p class='card-text'>";
	var strPreValue = "</p><h5 class='card-title'>";
	var strEnd = "</h5></div></div></div>";

	while (parentElm.hasChildNodes()) {
		parentElm.removeChild(parentElm.firstChild);
	}

	for (x in json) {
		parentElm.insertAdjacentHTML('beforeend', strPreKey + x + strPreValue + json[x] + strEnd);
	}
}

function fetchSystemInfo() {
	httpRequestAsync("GET", "/api/systemInfo", setSystemInfo);
}

// Storage Info
function drawStoragePieChart(valueUsed, valueAvailable, ctxId) {
	var config = {
		type: 'pie',
		data: {
			datasets: [{
				data: [
					valueUsed,
					valueAvailable,
				],
				backgroundColor: [
					"#fd625e",
					"#01b8aa",
				],
				label: 'Dataset 1'
			}],
			labels: [
				'Used',
				'Available',
			]
		},
		options: {
			responsive: true
		}
	};

	var ctx = document.getElementById(ctxId).getContext('2d');
	window.myPie = new Chart(ctx, config);
}

function storageInfoForeach(value, index, array) {
	var valueTotal = (value.totalSpace / 1024).toFixed(1);
	var valueAvailable = (value.availSpace / 1024).toFixed(1);
	var valueUsed = (valueTotal - valueAvailable).toFixed(1);

	var strPreType = "<div class='col-xl-5 col-lg-7 col-md-10 col-sm-10 col-10 my-3'><div class='card'><h5 class='storage-type card-header'>";
	var strPreInfo = "</h5><div class='card-body'><ul class='storage-info'>";
	var strPreChartId = "</ul><div id='storage-canvas-holder' style='width:100%;height:100%;'><canvas id='storage-chart-area-";
	var strEnd = "'></canvas></div></div></div></div>";

	var parentElm = document.getElementById("storage-content");

	var storageInfoStr = strPreType + value.path + strPreInfo +
		'<li>Type: ' + value.type + ' [' + value.state + ']' + '</li>' +
		'<li>Total Size: ' + valueTotal + ' MB</li>' +
		'<li>Available Size: ' + valueAvailable + ' MB</li>' +
		strPreChartId + (index + 1) + strEnd;

	parentElm.insertAdjacentHTML('beforeend', storageInfoStr);

	drawStoragePieChart(valueUsed, valueAvailable, 'storage-chart-area-' + (index + 1));
}

function setStorageInfo(jsonStr) {
	var parentElm = document.getElementById("storage-content");

	while (parentElm.hasChildNodes()) {
		parentElm.removeChild(parentElm.firstChild);
	}

	var json = JSON.parse(jsonStr);
	var storageInfoList = json.storageInfoList;

	if (storageInfoList) {
		storageInfoList.forEach(storageInfoForeach);
	}
}

function fetchStorageInfo() {
	httpRequestAsync("GET", "/api/storageInfo", setStorageInfo);
}

// connection status
function setConnectionInfo(jsonStr) {
	var json = JSON.parse(jsonStr);
	// document.getElementById("connection-type-value").innerHTML = json.connection_type;
	document.getElementById("wifi-value").innerHTML = json.wifi;
	document.getElementById("eth-value").innerHTML = json.ethernet;
	document.getElementById("bt-value").innerHTML = json.bluetooth;
}

function fetchConnectionStatus() {
	httpRequestAsync("GET", "/api/connection", setConnectionInfo);
}

//wifi ap list
function apListForeach(value, index, array) {
	var tBodyElm = document.getElementById("wifi-ap-list");
	var apInfoStr = "";

	if (value.favorite) {
		apInfoStr = "<tr class='table-success'><td>" + (index + 1) + "</td><td>" + value.essid + "</td><td>" + value.rssi + " dBm</td><td>Connected</td></tr>";
	} else {
		apInfoStr = "<tr><td>" + (index + 1) + "</td><td>" + value.essid + "</td><td>" + value.rssi + " dBm</td><td>Available</td></tr>";
	}

	tBodyElm.insertAdjacentHTML('beforeend', apInfoStr);
}

function rssiSort(a, b) {
	return (b.rssi - a.rssi);
}

function setWifiApList(jsonStr) {
	var json = JSON.parse(jsonStr);
	var tBodyElm = document.getElementById("wifi-ap-list");

	while (tBodyElm.hasChildNodes()) {
		tBodyElm.removeChild(tBodyElm.firstChild);
	}

	var apList = json.apList;
	apList.sort(rssiSort);
	if (apList) {
		apList.forEach(apListForeach);
	}

	var btn = document.getElementById('wifi-ap-refresh');
	btn.disabled = false;
	btn.textContent = 'Refresh';

}

function fetchApList() {
	httpRequestAsync("GET", "/api/connection/wifiScan", setWifiApList);
	var btn = document.getElementById('wifi-ap-refresh');
	btn.disabled = true;
	btn.textContent = 'Pending';
}

// application list
function appListForeach(value, index, array) {
	var tBodyElm = document.getElementById("app-list");
	var appStateStr = "";

	if (value.appPid) {
		appStateStr = "<tr><td>" + (index + 1) + "</td><td>" + value.appId + "</td><td>" + value.appState + "</td><td>" + value.appPid + "</td></tr>";
	} else {
		appStateStr = "<tr><td>" + (index + 1) + "</td><td>" + value.appId + "</td><td>" + value.appState + "</td><td>-</td></tr>";
	}

	tBodyElm.insertAdjacentHTML('beforeend', appStateStr);
}

function pidiSort(a, b) {
	if (a.appPid == b.appPid)
		return 0;

	if (a.appPid == undefined)
		return 1;

	if (b.appPid == undefined)
		return -1;

	return (b.appPid - a.appPid);
}

function setAppliationList(jsonStr) {
	var json = JSON.parse(jsonStr);
	var tBodyElm = document.getElementById("app-list");

	while (tBodyElm.hasChildNodes()) {
		tBodyElm.removeChild(tBodyElm.firstChild);
	}

	var appList = json.installedAppList;
	appList.sort(pidiSort);
	if (appList) {
		appList.forEach(appListForeach);
	}
}

function fetchApplicationList() {
	httpRequestAsync("GET", "/api/applicationList", setAppliationList);
}

(() => {
	document.addEventListener("DOMContentLoaded", function () {
		fetchSystemInfo();
	});

	document.getElementById("storage-tab").addEventListener("click", function () {
		fetchStorageInfo();
	});

	document.getElementById("connection-tab").addEventListener("click", function () {
		fetchConnectionStatus();
	});


	document.getElementById("wifi-ap-refresh").addEventListener("click", function () {
		fetchApList();
	});

	document.getElementById("application-tab").addEventListener("click", function () {
		fetchApplicationList();
	});
})();

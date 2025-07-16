#ifndef WEBPAGES_H
#define WEBPAGES_H

const char wifiConfigPage[] PROGMEM = R"===(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>WiFi Configuration</title>
  <style>
    body { font-family: Arial; margin: 20px; background: #f0f0f0; }
    .container { max-width: 400px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }
    h1 { color: #333; text-align: center; }
    .wifi-list { margin: 20px 0; max-height: 200px; overflow-y: auto; border: 1px solid #ddd; border-radius: 5px; }
    .wifi-item { padding: 10px; border-bottom: 1px solid #eee; cursor: pointer; display: flex; justify-content: space-between; align-items: center; }
    .wifi-item:hover { background: #f5f5f5; }
    .wifi-item:last-child { border-bottom: none; }
    .signal { font-size: 12px; color: #666; }
    .signal-strong { color: #4CAF50; }
    .signal-medium { color: #FF9800; }
    .signal-weak { color: #F44336; }
    input, button { width: 100%; padding: 10px; margin: 10px 0; box-sizing: border-box; }
    input { border: 1px solid #ddd; border-radius: 5px; }
    button { background: #4CAF50; color: white; border: none; border-radius: 5px; cursor: pointer; }
    button:hover { background: #45a049; }
    .scan-btn { background: #2196F3; }
    .scan-btn:hover { background: #1976D2; }
    .status { margin-top: 20px; padding: 10px; border-radius: 5px; }
    .success { background: #d4edda; color: #155724; }
    .error { background: #f8d7da; color: #721c24; }
    .loading { text-align: center; color: #666; }
  </style>
</head>
<body>
  <div class="container">
    <h1>WiFi Configuration</h1>
    <button class="scan-btn" onclick="scanWiFi()">Scan WiFi</button>
    <div id="wifiList" class="wifi-list">
      <div class="loading">Click button above to scan WiFi</div>
    </div>
    <form id="wifiForm">
      <input type="text" id="ssid" placeholder="WiFi Name" required>
      <input type="password" id="password" placeholder="WiFi Password">
      <button type="submit">Connect</button>
    </form>
    <div id="status"></div>
  </div>
  <script>
    var currentNetworks = [];
    
    // Add click handlers to WiFi items
    document.addEventListener("click", function(e) {
      if (e.target.closest(".wifi-item")) {
        var item = e.target.closest(".wifi-item");
        var idx = parseInt(item.getAttribute("data-idx"));
        if (currentNetworks[idx]) {
          document.getElementById("ssid").value = currentNetworks[idx].ssid;
          document.getElementById("password").focus();
        }
      }
    });
    
    function scanWiFi() {
      document.getElementById("wifiList").innerHTML = '<div class="loading">Scanning...</div>';
      
      fetch("/scan")
        .then(response => response.json())
        .then(networks => {
          currentNetworks = networks;
          const listDiv = document.getElementById("wifiList");
          if (networks.length === 0) {
            listDiv.innerHTML = '<div class="loading">No WiFi networks found</div>';
          } else {
            listDiv.innerHTML = networks.map(net => {
              let signalClass = "signal-weak";
              if (net.rssi > -60) signalClass = "signal-strong";
              else if (net.rssi > -70) signalClass = "signal-medium";
              
              // Create WiFi item with index instead of SSID in attribute
              var idx = networks.indexOf(net);
              return '<div class="wifi-item" data-idx="' + idx + '">' +
                '<span>' + net.ssid + (net.secure ? ' [LOCK]' : '') + '</span>' +
                '<span class="signal ' + signalClass + '">' + net.rssi + ' dBm</span>' +
              '</div>';
            }).join("");
          }
        })
        .catch(err => {
          document.getElementById("wifiList").innerHTML = '<div class="loading">Scan failed</div>';
        });
    }
    
    document.getElementById("wifiForm").onsubmit = function(e) {
      e.preventDefault();
      const data = {
        ssid: document.getElementById("ssid").value,
        password: document.getElementById("password").value
      };
      
      document.getElementById("status").innerHTML = '<div class="status">Connecting...</div>';
      
      fetch("/wifi/connect", {
        method: "POST",
        headers: {"Content-Type": "application/json"},
        body: JSON.stringify(data)
      })
      .then(response => response.json())
      .then(result => {
        const status = document.getElementById("status");
        if(result.success) {
          status.innerHTML = '<div class="status success">WiFi connected! Redirecting...</div>';
          setTimeout(() => {
            window.location.href = result.redirect || "/";
          }, 2000);
        } else {
          status.innerHTML = '<div class="status error">Connection failed: ' + result.message + '</div>';
        }
      });
    };
  </script>
</body>
</html>
)===";

const char wsConfigPage[] PROGMEM = R"===(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>WebSocket Configuration</title>
  <style>
    body { font-family: Arial; margin: 20px; background: #f0f0f0; }
    .container { max-width: 500px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }
    h1 { color: #333; text-align: center; }
    .info { background: #e3f2fd; padding: 15px; border-radius: 5px; margin-bottom: 20px; }
    .info h3 { margin-top: 0; color: #1976d2; }
    .example { background: #f5f5f5; padding: 10px; border-radius: 3px; font-family: monospace; margin: 5px 0; }
    input, button { width: 100%; padding: 10px; margin: 10px 0; box-sizing: border-box; }
    input { border: 1px solid #ddd; border-radius: 5px; }
    button { background: #4CAF50; color: white; border: none; border-radius: 5px; cursor: pointer; }
    button:hover { background: #45a049; }
    .status { margin-top: 20px; padding: 10px; border-radius: 5px; }
    .success { background: #d4edda; color: #155724; }
    .error { background: #f8d7da; color: #721c24; }
    .current-config { background: #fff9c4; padding: 10px; border-radius: 5px; margin-bottom: 20px; }
  </style>
</head>
<body>
  <div class="container">
    <h1>WebSocket Configuration</h1>
    
    <div class="current-config" id="currentConfig">
      <strong>Current Configuration:</strong>
      <span id="currentUrl">Not configured</span>
    </div>
    
    <div class="info">
      <h3>Configuration Guide</h3>
      <p>Please enter the complete WebSocket server address in the following format:</p>
      <div class="example">ws://192.168.1.100:8080/ws</div>
      <div class="example">ws://example.com:8080/websocket</div>
      <div class="example">wss://secure.example.com/ws</div>
    </div>
    
    <form id="wsForm">
      <input type="text" id="wsUrl" placeholder="WebSocket Server URL" required pattern="wss?://.+">
      <input type="text" id="wsToken" placeholder="Authentication Token" required>
      <button type="submit">Save Configuration</button>
    </form>
    
    <div id="status"></div>
    
    <button style="background: #ff9800; margin-top: 20px;" onclick="testConnection()">Test Connection</button>
  </div>
  
  <script>
    fetch("/ws/current")
      .then(response => response.json())
      .then(config => {
        if (config.wsUrl) {
          document.getElementById("currentUrl").textContent = config.wsUrl;
          document.getElementById("wsUrl").value = config.wsUrl;
          document.getElementById("wsToken").value = config.wsToken || "";
        }
      });
    
    document.getElementById("wsForm").onsubmit = function(e) {
      e.preventDefault();
      const data = {
        wsUrl: document.getElementById("wsUrl").value,
        wsToken: document.getElementById("wsToken").value
      };
      
      fetch("/ws/config", {
        method: "POST",
        headers: {"Content-Type": "application/json"},
        body: JSON.stringify(data)
      })
      .then(response => response.json())
      .then(result => {
        const status = document.getElementById("status");
        if(result.success) {
          status.innerHTML = '<div class="status success">Configuration saved!</div>';
          document.getElementById("currentUrl").textContent = data.wsUrl;
        } else {
          status.innerHTML = '<div class="status error">Save failed: ' + result.message + '</div>';
        }
      });
    };
    
    function testConnection() {
      document.getElementById("status").innerHTML = '<div class="status">Testing connection...</div>';
      
      fetch("/ws/test", { method: "POST" })
        .then(response => response.json())
        .then(result => {
          const status = document.getElementById("status");
          if(result.success) {
            status.innerHTML = '<div class="status success">Connection test successful!</div>';
          } else {
            status.innerHTML = '<div class="status error">Connection test failed: ' + result.message + '</div>';
          }
        });
    }
  </script>
</body>
</html>
)===";

const char statusPage[] PROGMEM = R"===(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP8266 Status</title>
  <style>
    body { font-family: Arial; margin: 20px; background: #f0f0f0; }
    .container { max-width: 600px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }
    h1 { color: #333; text-align: center; }
    .section { margin: 20px 0; padding: 15px; background: #f8f9fa; border-radius: 5px; }
    .section h2 { margin-top: 0; color: #495057; font-size: 18px; }
    .info { margin: 10px 0; display: flex; justify-content: space-between; }
    .label { font-weight: bold; color: #666; }
    .value { color: #333; }
    .status { display: inline-block; width: 10px; height: 10px; border-radius: 50%; margin-right: 10px; }
    .online { background: #28a745; }
    .offline { background: #dc3545; }
    button { background: #007bff; color: white; border: none; padding: 10px 20px; border-radius: 5px; cursor: pointer; margin: 5px; }
    button:hover { background: #0056b3; }
    .button-group { text-align: center; margin-top: 20px; }
  </style>
</head>
<body>
  <div class="container">
    <h1>Device Status</h1>
    
    <div class="section">
      <h2>WiFi Connection</h2>
      <div class="info">
        <span class="label">Status:</span>
        <span class="value">
          <span class="status" id="wifiStatus"></span>
          <span id="wifiText"></span>
        </span>
      </div>
      <div class="info">
        <span class="label">SSID:</span>
        <span class="value" id="ssid"></span>
      </div>
      <div class="info">
        <span class="label">IP Address:</span>
        <span class="value" id="ipAddress"></span>
      </div>
      <div class="info">
        <span class="label">Signal Strength:</span>
        <span class="value" id="rssi"></span>
      </div>
    </div>
    
    <div class="section">
      <h2>WebSocket Connection</h2>
      <div class="info">
        <span class="label">Status:</span>
        <span class="value">
          <span class="status" id="wsStatus"></span>
          <span id="wsText"></span>
        </span>
      </div>
      <div class="info">
        <span class="label">Server:</span>
        <span class="value" id="wsServer"></span>
      </div>
    </div>
    
    <div class="section">
      <h2>System Information</h2>
      <div class="info">
        <span class="label">Free Memory:</span>
        <span class="value" id="freeHeap"></span>
      </div>
      <div class="info">
        <span class="label">Uptime:</span>
        <span class="value" id="uptime"></span>
      </div>
    </div>
    
    <div class="button-group">
      <button onclick="location.href='/wifi/config'">WiFi Configuration</button>
      <button onclick="location.href='/ws/config'">WebSocket Configuration</button>
      <button onclick="location.href='/reset'" style="background: #dc3545;">Reset Device</button>
    </div>
  </div>
  
  <script>
    function updateStatus() {
      fetch("/status")
        .then(response => response.json())
        .then(data => {
          document.getElementById("wifiStatus").className = "status " + (data.wifiConnected ? "online" : "offline");
          document.getElementById("wifiText").textContent = data.wifiConnected ? "Connected" : "Disconnected";
          document.getElementById("ssid").textContent = data.ssid || "-";
          document.getElementById("ipAddress").textContent = data.ip || "-";
          document.getElementById("rssi").textContent = data.rssi ? data.rssi + " dBm" : "-";
          
          document.getElementById("wsStatus").className = "status " + (data.wsConnected ? "online" : "offline");
          document.getElementById("wsText").textContent = data.wsConnected ? "Connected" : "Disconnected";
          document.getElementById("wsServer").textContent = data.wsUrl || "Not configured";
          
          document.getElementById("freeHeap").textContent = data.freeHeap ? (data.freeHeap / 1024).toFixed(1) + " KB" : "-";
          document.getElementById("uptime").textContent = data.uptime ? formatUptime(data.uptime) : "-";
        });
    }
    
    function formatUptime(seconds) {
      const days = Math.floor(seconds / 86400);
      const hours = Math.floor((seconds % 86400) / 3600);
      const minutes = Math.floor((seconds % 3600) / 60);
      
      if (days > 0) return days + " days " + hours + " hours";
      if (hours > 0) return hours + " hours " + minutes + " minutes";
      return minutes + " minutes";
    }
    
    updateStatus();
    setInterval(updateStatus, 5000);
  </script>
</body>
</html>
)===";

#endif // WEBPAGES_H
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>

const char* ssid = "WifiChat 1.0";
const char* password = ""; // Set a secure password

AsyncWebServer server(80);
DNSServer dnsServer;

const int maxMessages = 5;
struct Message {
  String sender;
  String content;
  String ip;
};
Message messages[maxMessages];
int messageIndex = 0;
int currentMessageCount = 0; // Tracks the actual number of messages

std::vector<String> blockedIPs;

const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<style>
  body { font-family: Arial, sans-serif; margin: 0; padding: 0; text-align: center; }
  h1 { color: #333; }
  form { margin: 20px auto; max-width: 500px; }
  input[type=text], input[type=submit] { width: calc(100% - 22px); padding: 10px; margin: 10px 0; box-sizing: border-box; }
  input[type=submit] { background-color: #007BFF; color: white; border: none; border-radius: 5px; cursor: pointer; }
  input[type=submit]:hover { background-color: #0056b3; }
  ul { list-style-type: none; padding: 0; margin: 20px auto; max-width: 500px; }
  li { background-color: #f9f9f9; margin: 5px 0; padding: 10px; border-radius: 5px; word-wrap: break-word; overflow-wrap: break-word; white-space: pre-wrap; }
  #deviceCount { margin: 20px auto; max-width: 500px; }
  .warning { color: red; margin-bottom: 20px; }
  .link { color: #007BFF; text-decoration: none; }
  .link:hover { text-decoration: underline; }
</style>
<script>
function fetchData() {
  fetch('/messages').then(response => response.json()).then(data => {
    const ul = document.getElementById('messageList');
    ul.innerHTML = data.messages.map(msg => `<li>${msg.sender}: ${msg.message}</li>`).join('');
  });
  updateDeviceCount();
}

function updateDeviceCount() {
  fetch('/deviceCount').then(response => response.json()).then(data => {
    document.getElementById('deviceCount').textContent = 'Users Online: ' + data.count;
  });
}

function saveName() {
  const nameInput = document.getElementById('nameInput');
  localStorage.setItem('userName', nameInput.value);
}

function loadName() {
  const savedName = localStorage.getItem('userName');
  if (savedName) {
    document.getElementById('nameInput').value = savedName;
  }
}

window.onload = function() {
  loadName();
  fetchData();
  setInterval(fetchData, 5000);
  setInterval(updateDeviceCount, 5000);
};
</script>
</head>
<body>
<h2>WiFiChat 1.0</h2>
<div class='warning'>For your safety, do not share your location or any personal information!</div>
<form action="/update" method="POST">
  <input type="text" id="nameInput" name="sender" placeholder="Enter your name" required oninput="saveName()" />
  <input type="text" name="msg" placeholder="Enter your message" required />
  <input type="submit" value="Send" />
</form>
<div id='deviceCount'>Users Online: 0</div>
<ul id='messageList'></ul>
</body>
</html>
)rawliteral";

const char devicesPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<style>
  body { font-family: Arial, sans-serif; margin: 0; padding: 0; text-align: center; }
  h2 { color: #333; }
  ul { list-style-type: none; padding: 0; margin: 20px auto; max-width: 500px; }
  li { background-color: #f9f9f9; margin: 5px 0; padding: 10px; border-radius: 5px; word-wrap: break-word; overflow-wrap: break-word; white-space: pre-wrap; display: flex; flex-direction: column; align-items: center; }
  .button-container { display: flex; gap: 10px; margin-top: 10px; }
  button { padding: 5px 10px; background-color: #f44336; color: white; border: none; border-radius: 5px; cursor: pointer; }
  button:hover { background-color: #d32f2f; }
  .block-button { background-color: #f44336; }
  .unblock-button { background-color: #4CAF50; }
</style>
<script>
function fetchDeviceData() {
  fetch('/messages').then(response => response.json()).then(data => {
    const ul = document.getElementById('deviceList');
    ul.innerHTML = data.messages.map((msg, index) => 
      `<li>${msg.sender} (${msg.ip}): ${msg.message}
      <div class="button-container">
        <button onclick="deleteMessage(${index})" class="block-button">Delete</button>
        <button onclick="blockDevice('${msg.ip}')" class="block-button">Block</button>
      </div></li>`).join('');
  });
  fetchBlockedIPs();
  updateDeviceCount();
}

function fetchBlockedIPs() {
  fetch('/blockedIPs').then(response => response.json()).then(data => {
    const blockedList = document.getElementById('blockedList');
    blockedList.innerHTML = data.blockedIPs.map(ip => 
      `<li>${ip} <button onclick="unblockDevice('${ip}')" class="unblock-button">Unblock</button></li>`
    ).join('');
  });
}

function deleteMessage(index) {
  fetch(`/deleteMessage?index=${index}`, { method: 'GET' })
  .then(response => response.json())
  .then(data => {
    if (data.success) {
      fetchDeviceData();  // Refresh the list after deletion
    } else {
      alert('Failed to delete message: ' + data.error);
    }
  });
}

function blockDevice(ip) {
  fetch(`/blockDevice?ip=${ip}`, { method: 'GET' })
  .then(response => response.json())
  .then(data => {
    if (data.success) {
      alert('Device blocked');
      fetchDeviceData();  // Refresh the list after blocking
    } else {
      alert('Failed to block device');
    }
  });
}

function unblockDevice(ip) {
  fetch(`/unblockDevice?ip=${ip}`, { method: 'GET' })
  .then(response => response.json())
  .then(data => {
    if (data.success) {
      alert('Device unblocked');
      fetchDeviceData();  // Refresh the list after unblocking
    } else {
      alert('Failed to unblock device');
    }
  });
}

function updateDeviceCount() {
  fetch('/deviceCount').then(response => response.json()).then(data => {
    document.getElementById('deviceCount').textContent = 'Users Online: ' + data.count;
  });
}

window.onload = function() {
  fetchDeviceData();
  setInterval(fetchDeviceData, 5000);
  setInterval(updateDeviceCount, 5000);
};
</script>
</head>
<body>
<h2>Connected Devices and Messages</h2>
<ul id='deviceList'></ul>
<h2>Blocked IPs</h2>
<ul id='blockedList'></ul>
<p><a href="/">Back to Chat</a></p>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);
  WiFi.softAP(ssid, password);
  Serial.println("Access Point started");

  dnsServer.start(53, "*", WiFi.softAPIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", htmlPage);
  });

  server.on("/devices", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", devicesPage);
  });

  server.on("/messages", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = "[";
    bool first = true;
    for (int i = 0; i < currentMessageCount; i++) {
      if (!messages[i].content.isEmpty()) {
        if (!first) json += ",";
        json += "{\"sender\":\"" + messages[i].sender + "\",\"message\":\"" + messages[i].content + "\",\"ip\":\"" + messages[i].ip + "\"}";
        first = false;
      }
    }
    json += "]";
    request->send(200, "application/json", "{\"messages\":" + json + "}");
  });

  server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request) {
    String newMessage, senderName;
    if (request->hasParam("msg", true) && request->hasParam("sender", true)) {
      newMessage = request->getParam("msg", true)->value();
      senderName = request->getParam("sender", true)->value();
      String clientIP = request->client()->remoteIP().toString();

      // Check if the IP is blocked
      if (std::find(blockedIPs.begin(), blockedIPs.end(), clientIP) != blockedIPs.end()) {
        // Send a styled HTML response for blocked users
        String blockMessage = R"rawliteral(
          <!DOCTYPE html>
          <html>
          <head>
          <meta name="viewport" content="width=device-width, initial-scale=1.0">
          <style>
            body { font-family: Arial, sans-serif; margin: 0; padding: 20px; text-align: center; background-color: #f8d7da; }
            .block-warning { color: #721c24; background-color: #f8d7da; border: 1px solid #f5c6cb; padding: 20px; border-radius: 5px; margin-top: 50px; }
            h1 { color: #721c24; }
          </style>
          </head>
          <body>
            <div class="block-warning">
              <h1>Access Denied</h1>
              <p>Your IP address has been blocked due to inappropriate activity.</p>
              <p>Please contact the administrator if you believe this is a mistake.</p>
            </div>
          </body>
          </html>
        )rawliteral";
        request->send(403, "text/html", blockMessage);
        return;
      }

      // Normal message processing
      messages[messageIndex].content = newMessage;
      messages[messageIndex].sender = senderName;
      messages[messageIndex].ip = clientIP;
      messageIndex = (messageIndex + 1) % maxMessages;
      if (currentMessageCount < maxMessages) currentMessageCount++;
    }
    request->redirect("/");
  });

  server.on("/deleteMessage", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("index")) {
      int index = request->getParam("index")->value().toInt();
      Serial.print("Attempting to delete message at index: ");
      Serial.println(index);
      if (index >= 0 && index < currentMessageCount) {
        // Shift messages to maintain order after deletion
        for (int i = index; i < currentMessageCount - 1; i++) {
          messages[i] = messages[i + 1];
        }
        // Clear the last message now that everything is shifted
        messages[currentMessageCount - 1] = {"", "", ""};
        currentMessageCount--;
        Serial.println("Message deleted successfully.");
        request->send(200, "application/json", "{\"success\":true}");
      } else {
        Serial.println("Invalid index.");
        request->send(200, "application/json", "{\"success\":false, \"error\":\"Invalid index.\"}");
      }
    } else {
      Serial.println("Index parameter missing.");
      request->send(200, "application/json", "{\"success\":false, \"error\":\"Index parameter missing.\"}");
    }
  });

  server.on("/blockDevice", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("ip")) {
      String ip = request->getParam("ip")->value();

      // Check if the IP is already in the blocked list
      if (std::find(blockedIPs.begin(), blockedIPs.end(), ip) == blockedIPs.end()) {
        blockedIPs.push_back(ip); // Block the IP address if not already blocked
        request->send(200, "application/json", "{\"success\":true}");
      } else {
        request->send(200, "application/json", "{\"success\":false, \"error\":\"IP already blocked.\"}");
      }
    } else {
      request->send(200, "application/json", "{\"success\":false, \"error\":\"IP parameter missing.\"}");
    }
  });

  server.on("/unblockDevice", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("ip")) {
      String ip = request->getParam("ip")->value();
      auto it = std::find(blockedIPs.begin(), blockedIPs.end(), ip);
      if (it != blockedIPs.end()) {
        blockedIPs.erase(it); // Unblock the IP address
        request->send(200, "application/json", "{\"success\":true}");
      } else {
        request->send(200, "application/json", "{\"success\":false}");
      }
    } else {
      request->send(200, "application/json", "{\"success\":false}");
    }
  });

  server.on("/blockedIPs", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = "[";
    for (size_t i = 0; i < blockedIPs.size(); i++) {
      if (i > 0) json += ",";
      json += "\"" + blockedIPs[i] + "\"";
    }
    json += "]";
    request->send(200, "application/json", "{\"blockedIPs\":" + json + "}");
  });

  server.on("/deviceCount", HTTP_GET, [](AsyncWebServerRequest *request){
    int deviceCount = WiFi.softAPgetStationNum();
    request->send(200, "application/json", "{\"count\":" + String(deviceCount) + "}");
  });

  server.begin();
}

void loop() {
  dnsServer.processNextRequest();
}

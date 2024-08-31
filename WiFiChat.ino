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
};
Message messages[maxMessages];
int messageIndex = 0;

const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<style>
  body { font-family: Arial, sans-serif; margin: 0; padding: 0; text-align: center; }
  h1 { color: #333; }
  form { margin: 20px auto; max-width: 500px; }
  input[type=text] { width: calc(100% - 22px); padding: 10px; margin: 10px 0; box-sizing: border-box; }
  input[type=submit] { padding: 10px 20px; background-color: #007BFF; color: white; border: none; border-radius: 5px; cursor: pointer; }
  input[type=submit]:hover { background-color: #0056b3; }
  ul { list-style-type: none; padding: 0; margin: 20px auto; max-width: 500px; }
  li { background-color: #f9f9f9; margin: 5px 0; padding: 10px; border-radius: 5px; word-wrap: break-word; overflow-wrap: break-word; white-space: pre-wrap; }
  #deviceCount { margin: 20px auto; max-width: 500px; }
  .warning { color: red; margin-bottom: 20px; }
</style>
<script>
function fetchData() {
  fetch('/messages').then(response => response.json()).then(data => {
    const ul = document.getElementById('messageList');
    ul.innerHTML = data.messages.map(msg => `<li>${msg.sender}: ${msg.message}</li>`).join('');
  });
  fetch('/deviceCount').then(response => response.json()).then(data => {
    document.getElementById('deviceCount').textContent = 'Users Online: ' + data.count;
  });
}
window.onload = function() {
  loadName();
  fetchData();
  setInterval(fetchData, 10000); // Fetch data every 10 seconds
};
function saveName() {
  const nameInput = document.getElementById('nameInput');
  localStorage.setItem('username', nameInput.value);
}
function loadName() {
  const savedName = localStorage.getItem('username');
  if (savedName) {
    document.getElementById('nameInput').value = savedName;
  }
}
</script>
</head>
<body>
<h2>WiFiChat 1.0</h2>
<div class='warning'>For your safety, do not share your location or any personal information!</div>
<form action="/update" method="POST" onsubmit="saveName()">
  <input type="text" id="nameInput" name="sender" placeholder="Enter your name" required />
  <input type="text" name="msg" placeholder="Enter your message" required />
  <input type="submit" value="Send" />
</form>
<div id='deviceCount'>Users Online: 0</div>
<ul id='messageList'></ul>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);

  WiFi.softAP(ssid, password);
  WiFi.setTxPower(WIFI_POWER_19_5dBm); // Adjust power as needed to conserve energy
  Serial.println("Access Point started with full power");

  dnsServer.start(53, "*", WiFi.softAPIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", htmlPage);
  });

  server.on("/messages", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = "[";
    bool first = true;
    for (int i = 0; i < maxMessages; i++) {
      int index = (messageIndex + i) % maxMessages;
      if (messages[index].content != "") {
        if (!first) json += ",";
        json += "{\"sender\":\"" + messages[index].sender + "\",\"message\":\"" + messages[index].content + "\"}";
        first = false;
      }
    }
    json += "]";
    request->send(200, "application/json", "{\"messages\":" + json + "}");
  });

  server.on("/deviceCount", HTTP_GET, [](AsyncWebServerRequest *request){
    int deviceCount = WiFi.softAPgetStationNum();
    request->send(200, "application/json", "{\"count\":" + String(deviceCount) + "}");
  });

  server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request){
    String newMessage = "";
    String senderName = "";
    if (request->hasParam("msg", true)) {
      newMessage = request->getParam("msg", true)->value();
    }
    if (request->hasParam("sender", true)) {
      senderName = request->getParam("sender", true)->value();
    }

    // Sanitize user input
    newMessage.replace("<", "&lt;");
    newMessage.replace(">", "&gt;");
    senderName.replace("<", "&lt;");
    senderName.replace(">", "&gt;");

    messages[messageIndex].content = newMessage;
    messages[messageIndex].sender = senderName;
    messageIndex = (messageIndex + 1) % maxMessages;

    request->redirect("/");
  });

  server.begin();
}

void loop() {
  dnsServer.processNextRequest();
}

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>

const char* ssid = "WifiChat 1.0";
const char* password = ""; // No password

AsyncWebServer server(80);
DNSServer dnsServer;

// Define the number of messages to keep
const int maxMessages = 5;
struct Message {
  String sender;
  String content;
};
Message messages[maxMessages];
int messageIndex = 0;

// Captive portal redirect URL
const String portalRedirectURL = "/";

void setup() {
  Serial.begin(115200);

  // Set up Wi-Fi access point with maximum power
  WiFi.softAP(ssid, password);
  WiFi.setTxPower(WIFI_POWER_19_5dBm); // Set Wi-Fi power to maximum
  Serial.println("Access Point started with full power");

  // Set up DNS server to redirect all requests to the captive portal
  dnsServer.start(53, "*", WiFi.softAPIP());

  // Serve the main page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = "<!DOCTYPE html><html><head>";
    html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
    html += "<style>";
    html += "body { font-family: Arial, sans-serif; margin: 0; padding: 0; text-align: center; }";
    html += "h1 { color: #333; }";
    html += "form { margin: 20px auto; max-width: 500px; }";
    html += "input[type=text] { width: calc(100% - 22px); padding: 10px; margin: 10px 0; box-sizing: border-box; }";
    html += "input[type=submit] { padding: 10px 20px; background-color: #007BFF; color: white; border: none; border-radius: 5px; cursor: pointer; }";
    html += "input[type=submit]:hover { background-color: #0056b3; }";
    html += "ul { list-style-type: none; padding: 0; margin: 20px auto; max-width: 500px; }";
    html += "li {";
    html += "  background-color: #f9f9f9;";
    html += "  margin: 5px 0;";
    html += "  padding: 10px;";
    html += "  border-radius: 5px;";
    html += "  word-wrap: break-word;"; // Break long words to fit within the container
    html += "  overflow-wrap: break-word;"; // For handling overflow in long words
    html += "  white-space: pre-wrap;"; // Preserve spaces and line breaks
    html += "}";
    html += "#deviceCount { margin: 20px auto; max-width: 500px; }";
    html += ".warning { color: red; margin-bottom: 20px; }"; // Removed font-weight: bold
    html += "</style>";
    html += "<script>";
    html += "function fetchMessages() {";
    html += "  fetch('/messages').then(response => response.json()).then(data => {";
    html += "    const ul = document.getElementById('messageList');";
    html += "    ul.innerHTML = '';";
    html += "    data.messages.forEach(msg => {";
    html += "      const li = document.createElement('li');";
    html += "      li.textContent = msg.sender + ': ' + msg.message;";
    html += "      ul.appendChild(li);";
    html += "    });";
    html += "  });";
    html += "}";
    html += "function fetchDeviceCount() {";
    html += "  fetch('/deviceCount').then(response => response.json()).then(data => {";
    html += "    document.getElementById('deviceCount').textContent = 'Users Online: ' + data.count;";
    html += "  });";
    html += "}";
    html += "function saveName() {";
    html += "  const nameInput = document.getElementById('nameInput');";
    html += "  localStorage.setItem('username', nameInput.value);";
    html += "}";
    html += "function loadName() {";
    html += "  const savedName = localStorage.getItem('username');";
    html += "  if (savedName) {";
    html += "    document.getElementById('nameInput').value = savedName;";
    html += "  }";
    html += "}";
    html += "window.onload = function() {";
    html += "  loadName();";
    html += "  fetchMessages();";
    html += "  fetchDeviceCount();";
    html += "  setInterval(fetchMessages, 5000);"; // Refresh messages every 5 seconds
    html += "  setInterval(fetchDeviceCount, 5000);"; // Refresh device count every 5 seconds
    html += "};";
    html += "</script>";
    html += "</head><body>";
    html += "<h2>WiFiChat 1.0</h2>";
    html += "<div class='warning'>For your safety, do not share your location or any personal information!</div>";
    html += "<form action=\"/update\" method=\"POST\" onsubmit=\"saveName()\">";
    html += "<input type=\"text\" id=\"nameInput\" name=\"sender\" placeholder=\"Enter your name\" required />";
    html += "<input type=\"text\" name=\"msg\" placeholder=\"Enter your message\" required />";
    html += "<input type=\"submit\" value=\"Send\" />";
    html += "</form>";
    html += "<div id='deviceCount'>Users Online: 0</div>";
    html += "<ul id='messageList'></ul>"; // Messages shown below the form
    html += "</body></html>";
    request->send(200, "text/html", html);
  });

  // Serve messages as JSON
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

  // Serve connected device count as JSON
  server.on("/deviceCount", HTTP_GET, [](AsyncWebServerRequest *request){
    int deviceCount = WiFi.softAPgetStationNum();
    request->send(200, "application/json", "{\"count\":" + String(deviceCount) + "}");
  });

  // Handle message update
  server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request){
    String newMessage = "";
    String senderName = "";
    if (request->hasParam("msg", true)) {
      newMessage = request->getParam("msg", true)->value();
    }
    if (request->hasParam("sender", true)) {
      senderName = request->getParam("sender", true)->value();
    }
    
    // Update messages array
    messages[messageIndex].content = newMessage;
    messages[messageIndex].sender = senderName;
    messageIndex = (messageIndex + 1) % maxMessages;
    
    request->redirect("/");
  });

  // Start the server
  server.begin();
}

void loop() {
  dnsServer.processNextRequest();
}


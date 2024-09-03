A simple Esp32 project that acts as a wifi portal for connected users to exchange messages via html webpage.
To access web chat when conected to esp32 wifi just input http://anything to your browser after connecting to the esp32 wifi and it will bring up the web chat. (do not use https://)
you can type http://anything/devices to access admin tools.
Wifi chat currently has no password to connect the wifi but you can add one if you wish this to be private.

Instructions for Running WiFiChat 1.0 and Accessing the Chat
This code sets up a simple WiFi chat application using an ESP32 microcontroller. It hosts a web-based chat interface where users can send messages, view connected devices, and block or unblock devices based on their IP addresses.

Setup Instructions:
Install Required Libraries:

Ensure you have the latest ESP32 board package installed in the Arduino IDE.
Required libraries:
ESPAsyncWebServer: Install from the Arduino Library Manager by searching for "ESPAsyncWebServer".
AsyncTCP: Required dependency for ESPAsyncWebServer, also available via the Arduino Library Manager.
DNSServer: Install from the Arduino Library Manager.
Flash the Code to the ESP32:

Copy and paste the code into your Arduino IDE.
Select the correct board (e.g., "ESP32 Dev Module") and the correct COM port.
Click "Upload" to flash the code to the ESP32.

Connect to the WiFi Network:

After flashing the code, the ESP32 will create a WiFi Access Point (AP) named WifiChat 1.0.
Connect your computer, smartphone, or tablet to this WiFi network. No password is set, but you can configure one by editing the password variable in the code.

Access the Chat Interface:

Open a web browser (e.g., Chrome, Firefox) on your connected device.
Navigate to the IP address 192.168.4.1 OR http://anything.This is the default IP address for the ESP32 in AP mode.
You will see the chat interface where you can enter your name and message to participate in the chat.
Using the Chat Interface:

Sending Messages: Enter your name and message in the form and click "Send." Messages will be displayed in the chat area.
View Device Count: The number of connected users is displayed at the top of the page.
Blocking Users: Administrators can view the list of connected devices and block or unblock users based on their IP addresses by visiting the /devices URL (e.g., 192.168.4.1/devices , http://anything/devices).
Blocking and Unblocking Devices:

Blocked devices will see a warning message if they attempt to send messages.
You can manage blocked IPs through the "Blocked IPs" section on the devices page.
Security Note:

This application does not use encryption, so avoid sending sensitive or personal information.
This setup allows for a simple local chat application that operates entirely within the created WiFi network, making it a great project for learning basic web server functionality and network communication using ESP32.

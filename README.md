# CheapRemote

CheapRemote is a project that should allow you to read essential information about your GoPro in real time, such as battery charge status and recording status.

I run this code on a M5StickCPlus with ESP32 Pico, but it can run on any board having WiFi.

The project is still raw and I believe it will never reach the stable stage.

# Bugs or Problems
This is the list of problems encountered:

- [ ] After a GoPro reboot the WiFi AP may not start, so it is impossible to connect to the GoPro WiFi network.
- [ ] After a GoPro restart it may happen that the HTTP service fails to start or crash, thus it would be impossible to perform querys.

# Attempts made to resolve problems

- [X] Using the mode() and disconnect() methods in Setup()
- [X] Using delay() method
- [X] Verified network parameters
- [X] DNS configured
- [X] Manually set network parameters (No DHCP)
- [X] Use 'bool HTTPClient::begin(String host, uint16_t port, String uri)' instead of 'bool HTTPClient::begin(String url)'
- [X] Infinite while loop on pressing the shutdown key to make sure the TCP socket is closed
- [X] Disconnect at each cycle and then connect again later

# Solutions to problems
None, the problem is in GoPro firmware.

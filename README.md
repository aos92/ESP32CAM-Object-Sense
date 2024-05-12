# ESP32CAM Object Sense

This script is an implementation of object tracking using TensorFlow.js COCO-SSD on ESP32-CAM. The script allows you to perform real-time object tracking using the ESP32-CAM camera.

### Components:

- ESP32-CAM Wrover Module
- 5mm White LED
- Resistor 220 Ohm

### Board Configuration:

- **PSRAM:** Enable
- **Partition Scheme:** hugo APP (3MB No OTA)
- **CPU Frequency:** 240MHz
- **Flash Mode:** QIO
- **Flash Frequency:** 80MHz
- **Flash Size:** 4MB
- **Upload Speed:** 115200
- **Core Debug Level:** None

### Pin Configuration

#### Camera Pins:
- **XCLK:** GPIO 21
- **SIOD:** GPIO 26
- **SIOC:** GPIO 27
- **Y9:** GPIO 35
- **Y8:** GPIO 34
- **Y7:** GPIO 39
- **Y6:** GPIO 36
- **Y5:** GPIO 19
- **Y4:** GPIO 18
- **Y3:** GPIO 5
- **Y2:** GPIO 4
- **VSYNC:** GPIO 25
- **HREF:** GPIO 23
- **PCLK:** GPIO 22

#### LED Flash Pins:
- **LED Flash:** GPIO 14

### Network Connection
You can use AP (Access Point) mode or STA (Station Association) mode or both. To configure the connection, you can use the specified WiFi network name and password.

### Object Classes
You can refer to [tfjs-models/classes.ts](https://github.com/tensorflow/tfjs-models/blob/master/coco-ssd/src/classes.ts) to see the recognized object classes.

The available object models encompass a variety of items, ranging from humans, animals, vehicles, household items, and much more. Some of them include bicycles, cars, airplanes, buses, trains, trucks, boats, traffic lights, as well as various types of food and drinks like pizza, donuts, or wine glasses. Additionally, there are objects such as chairs, sofas, potted plants, and others. In total, there are over fifty object models that can be recognized by the system.

### Custom Command Format
You can control the system's behavior by sending custom commands via HTTP.

### Common Commands
- `ip`: Get and display the IP address of both AP and STA modes.
- `mac`: Get and display the MAC address of the STA.
- `resetwifi`: Reset WIFI connection with the provided SSID and password.
- `restart`: Restart the ESP32 device.
- `flash`: Set LED light intensity.
- `framesize`: Set camera frame size.
- `quality`: Set camera image quality.
- `contrast`: Set camera image contrast.
- `brightness`: Set camera image brightness.

### Example Usage of Custom Commands
- `http://<IP>/control?cmd=flash=100`: Set LED light intensity to 100.
- `http://<IP>/control?cmd=framesize=VGA`: Set camera frame size to VGA.

### Note
Make sure you have selected the ESP32 Wrover Module or another board with PSRAM enabled.


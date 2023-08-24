#include "esp_camera.h"
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <iostream>
#include <sstream>

#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27
#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

const char *ssid = "UTCMiniCar";
const char *password = "12345678";

AsyncWebServer server(80);
AsyncWebSocket wsCamera("/Camera");
AsyncWebSocket wsCarInput("/CarInput");
uint32_t cameraClientId = 0;

const char *htmlHomePage PROGMEM = R"HTMLHOMEPAGE(
<!DOCTYPE html>
<html>

<head>
    <meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no">
    <style>
        .noselect {
            -webkit-touch-callout: none;
            /* iOS Safari */
            -webkit-user-select: none;
            /* Safari */
            -khtml-user-select: none;
            /* Konqueror HTML */
            -moz-user-select: none;
            /* Firefox */
            -ms-user-select: none;
            /* Internet Explorer/Edge */
            user-select: none;
            /* Non-prefixed version, currently
                                      supported by Chrome and Opera */
        }

        .slidecontainer {
            width: 100%;
        }

        .slider {
            -webkit-appearance: none;
            width: 100%;
            height: 15px;
            border-radius: 5px;
            background: #d3d3d3;
            outline: none;
            opacity: 0.7;
            -webkit-transition: .2s;
            transition: opacity .2s;
        }

        .slider:hover {
            opacity: 1;
        }

        .slider::-webkit-slider-thumb {
            -webkit-appearance: none;
            appearance: none;
            width: 25px;
            height: 25px;
            border-radius: 50%;
            background: red;
            cursor: pointer;
        }

        .slider::-moz-range-thumb {
            width: 25px;
            height: 25px;
            border-radius: 50%;
            background: red;
            cursor: pointer;
        }

        td {
            display: inline-block;
            vertical-align: middle;
        }

        .button {
            border-radius: 50%;
        }

        .button:active .top {
            border: 0.5em solid #e74c3c;
        }

        .button:active .top:after {
            border-top: 0.5em solid #e74c3c;
            border-right: 0.5em solid #e74c3c;
        }

        .top {
            display: inline-block;
            width: 2em;
            height: 2em;
            border: 0.5em solid #333;
            border-radius: 50%;
        }

        .top:after {
            content: '';
            display: inline-block;
            margin-top: 0.5em;
            width: 0.8em;
            height: 0.8em;
            border-top: 0.4em solid #333;
            border-right: 0.4em solid #333;
            -moz-transform: rotate(-45deg);
            -webkit-transform: rotate(-45deg);
            transform: rotate(-45deg);
        }
    </style>

</head>

<body class="noselect" style="background-color:white">
    <div style="display:flex;align-items: center;justify-content: center;">
        <div>
            <table style="min-width: 160px;text-align: center;">
                <tr>
                    <td class="button" ontouchstart='sendButtonInput("MoveCar","4")'
                        ontouchend='sendButtonInput("MoveCar","0")' style="transform: rotate(-50deg);"><span
                            class="top"></span></td>
                    <td class="button" ontouchstart='sendButtonInput("MoveCar","1")'
                        ontouchend='sendButtonInput("MoveCar","0")'><span class="top"></span></td>
                    <td class="button" ontouchstart='sendButtonInput("MoveCar","7")'
                        ontouchend='sendButtonInput("MoveCar","0")'>
                        <span class="top" style="transform: rotate(50deg);"></span>
                    </td>
                </tr>
                <tr>
                    <td class="button" ontouchstart='sendButtonInput("MoveCar","2")'
                        ontouchend='sendButtonInput("MoveCar","0")'><span class="top"
                            style="transform: rotate(-90deg);"></span>
                    </td>
                    <td style="display: inline-block;width: 3.2em;height: 3.2em;border-radius: 50%;"></td>
                    <td class="button" ontouchstart='sendButtonInput("MoveCar","5")'
                        ontouchend='sendButtonInput("MoveCar","0")'><span class="top"
                            style="transform: rotate(90deg);"></span>
                    </td>
                </tr>
                <tr>
                    <td class="button" ontouchstart='sendButtonInput("MoveCar","9")'
                        ontouchend='sendButtonInput("MoveCar","0")'><span class="top"
                            style="transform: rotate(-145deg);"></span>
                    </td>
                    <td class="button" ontouchstart='sendButtonInput("MoveCar","8")'
                        ontouchend='sendButtonInput("MoveCar","0")'><span class="top"
                            style="transform: rotate(180deg);"></span>
                    </td>
                    <td class="button" ontouchstart='sendButtonInput("MoveCar","10")'
                        ontouchend='sendButtonInput("MoveCar","0")'><span class="top"
                            style="transform: rotate(145deg);"></span>
                    </td>
                </tr>
            </table>
        </div>

        <img id="cameraImage" src="" style="width:100%;height:250px;margin: 10px;"></td>
        <div style="width: 30%;">
            <div class="slidecontainer" style="margin-bottom: 10px;">
                <input type="range" min="0" max="255" value="150" class="slider" id="Speed"
                    oninput='sendButtonInput("Speed",value)'>
            </div>

            <div class="slidecontainer" style="margin-bottom: 10px;">
                <input type="range" min="0" max="255" value="0" class="slider" id="Light"
                    oninput='sendButtonInput("Light",value)'>
            </div>

            <div class="slidecontainer" style="margin-bottom: 10px;">
                <input type="range" min="0" max="180" value="90" class="slider" id="Pan"
                    oninput='sendButtonInput("Pan",value)'>
            </div>

            <div class="slidecontainer" style="margin-bottom: 10px;">
                <input type="range" min="0" max="180" value="90" class="slider" id="Tilt"
                    oninput='sendButtonInput("Tilt",value)'>
            </div>
            <div>
                <table style="min-width: 160px;text-align: center;">
                    <tr>
                        <td class="button" ontouchstart='sendButtonInput("MoveCar","3")'
                            ontouchend='sendButtonInput("MoveCar","0")'
                            style="transform: rotate(-90deg); margin-right: 17%;"><span class="top"></span></td>
                        <td class="button" ontouchstart='sendButtonInput("MoveCar","6")'
                            ontouchend='sendButtonInput("MoveCar","0")'>
                            <span class="top" style="transform: rotate(90deg);"></span>
                        </td>
                    </tr>
                </table>
            </div>
        </div>
    </div>
    <script>
        var webSocketCameraUrl = "ws:\/\/" + window.location.hostname + "/Camera";
        var webSocketCarInputUrl = "ws:\/\/" + window.location.hostname + "/CarInput";
        var websocketCamera;
        var websocketCarInput;

        function initCameraWebSocket() {
            websocketCamera = new WebSocket(webSocketCameraUrl);
            websocketCamera.binaryType = 'blob';
            websocketCamera.onopen = function (event) { };
            websocketCamera.onclose = function (event) { setTimeout(initCameraWebSocket, 2000); };
            websocketCamera.onmessage = function (event) {
                var imageId = document.getElementById("cameraImage");
                imageId.src = URL.createObjectURL(event.data);
            };
        }

        function initCarInputWebSocket() {
            websocketCarInput = new WebSocket(webSocketCarInputUrl);
            websocketCarInput.onopen = function (event) {
                sendButtonInput("Speed", document.getElementById("Speed").value);
                sendButtonInput("Light", document.getElementById("Light").value);
                sendButtonInput("Pan", document.getElementById("Pan").value);
                sendButtonInput("Tilt", document.getElementById("Tilt").value);
            };
            websocketCarInput.onclose = function (event) { setTimeout(initCarInputWebSocket, 2000); };
            websocketCarInput.onmessage = function (event) { };
        }

        function initWebSocket() {
            initCameraWebSocket();
            initCarInputWebSocket();
        }

        function sendButtonInput(key, value) {
            var data = key + "," + value;
            websocketCarInput.send(data);
        }

        window.onload = initWebSocket;
        document.getElementById("mainTable").addEventListener("touchend", function (event) {
            event.preventDefault()
        });      
    </script>
</body>

</html>
)HTMLHOMEPAGE";


void handleRoot(AsyncWebServerRequest *request) {
  request->send_P(200, "text/html", htmlHomePage);
}

void handleNotFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "File Not Found");
}

void onCarInputWebSocketEvent(AsyncWebSocket *server,
                              AsyncWebSocketClient *client,
                              AwsEventType type,
                              void *arg,
                              uint8_t *data,
                              size_t len) {
  static byte controlMoveStatus = 0;
  static byte speedMoveStatus = 0;
  static byte spectatorViewX = 90;
  static byte spectatorViewY = 90;
  static String dataTransmitter = String(controlMoveStatus) + "," + String(speedMoveStatus) + "," + String(spectatorViewX) + "," + String(spectatorViewY) + "&";
  switch (type) {
    case WS_EVT_CONNECT:
      // Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      // Serial.printf("WebSocket client #%u disconnected\n", client->id());
      controlMoveStatus = 0;
      speedMoveStatus = 0;
      spectatorViewX = 90;
      spectatorViewY = 90;
      dataTransmitter = String(controlMoveStatus) + "," + String(speedMoveStatus) + "," + String(spectatorViewX) + "," + String(spectatorViewY) + "&";
      Serial.print(dataTransmitter);
      break;
    case WS_EVT_DATA:
      AwsFrameInfo *info;
      info = (AwsFrameInfo *)arg;
      if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        std::string myData = "";
        myData.assign((char *)data, len);
        std::istringstream ss(myData);
        std::string key, value;
        std::getline(ss, key, ',');
        std::getline(ss, value, ',');
        // Serial.printf("Key [%s] Value[%s]\n", key.c_str(), value.c_str());
        int valueInt = atoi(value.c_str());
        if (key == "MoveCar") {
          controlMoveStatus = valueInt;
        } else if (key == "Speed") {
          speedMoveStatus = valueInt;
        } else if (key == "Light") {
          analogWrite(4, valueInt);
          delay(30);
        } else if (key == "Pan") {
          spectatorViewX = valueInt;
        } else if (key == "Tilt") {
          spectatorViewY = valueInt;
        }
      }
      dataTransmitter = String(controlMoveStatus) + "," + String(speedMoveStatus) + "," + String(spectatorViewX) + "," + String(spectatorViewY) + "&";
      Serial.print(dataTransmitter);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
    default:
      break;
  }
}

void onCameraWebSocketEvent(AsyncWebSocket *server,
                            AsyncWebSocketClient *client,
                            AwsEventType type,
                            void *arg,
                            uint8_t *data,
                            size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      // Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      cameraClientId = client->id();
      break;
    case WS_EVT_DISCONNECT:
      // Serial.printf("WebSocket client #%u disconnected\n", client->id());
      cameraClientId = 0;
      break;
    case WS_EVT_DATA:
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
    default:
      break;
  }
}

void setupCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_4;
  config.ledc_timer = LEDC_TIMER_2;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  config.frame_size = FRAMESIZE_VGA;
  config.jpeg_quality = 10;
  config.fb_count = 1;

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    // Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  if (psramFound()) {
    heap_caps_malloc_extmem_enable(20000);
    // Serial.printf("PSRAM initialized. malloc to take memory from psram above this size");
  }
}

void sendCameraPicture() {
  if (cameraClientId == 0) {
    return;
  }
  unsigned long startTime1 = millis();
  //capture a frame
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    // Serial.println("Frame buffer could not be acquired");
    return;
  }

  unsigned long startTime2 = millis();
  wsCamera.binary(cameraClientId, fb->buf, fb->len);
  esp_camera_fb_return(fb);

  //Wait for message to be delivered
  while (true) {
    AsyncWebSocketClient *clientPointer = wsCamera.client(cameraClientId);
    if (!clientPointer || !(clientPointer->queueIsFull())) {
      break;
    }
    delay(1);
  }

  unsigned long startTime3 = millis();
  // Serial.printf("Time taken Total: %d|%d|%d\n",startTime3 - startTime1, startTime2 - startTime1, startTime3-startTime2 );
}

void setup() {
  Serial.begin(115200, SERIAL_8N1, 3, 1);

  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  // Serial.print("AP IP address: ");
  // Serial.println(IP);

  server.on("/", HTTP_GET, handleRoot);
  server.onNotFound(handleNotFound);

  wsCamera.onEvent(onCameraWebSocketEvent);
  server.addHandler(&wsCamera);

  wsCarInput.onEvent(onCarInputWebSocketEvent);
  server.addHandler(&wsCarInput);

  server.begin();

  setupCamera();
}
void loop() {
  wsCamera.cleanupClients();
  wsCarInput.cleanupClients();
  sendCameraPicture();
  // Serial.print("7549,678,10,333");
  // delay(2000);
}
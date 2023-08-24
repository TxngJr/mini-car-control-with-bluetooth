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
        * {
            padding: 0;
            margin: 0;
            box-sizing: border-box;
        }

        @media all and (orientation: landscape) {
            .arrows {
                font-size: 30px;
                color: red;
            }

            td.button {
                background-color: black;
                border-radius: 25%;
                box-shadow: 5px 5px #888888;
            }

            td.button:active {
                transform: translate(5px, 5px);
                box-shadow: none;
            }

            .noselect {
                -webkit-touch-callout: none;
                -webkit-user-select: none;
                -khtml-user-select: none;
                -moz-user-select: none;
                -ms-user-select: none;
                user-select: none;
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
        }
    </style>
</head>

<body class="noselect" style="background-color:red;position: relative;display: flex;">
    <img id="cameraImage" src="" style="left: 0;margin: 0;position: absolute;width: 100%;">
    <table style="position: absolute;right: 10px;top: 10px;height: 150px;">
        <tr style="position: relative;margin-bottom: 20px;">
            <td style="text-align:left"><b>Light:</b></td>
            <td colspan=2>
                <div class="slidecontainer">
                    <input type="range" min="0" max="255" value="0" class="slider" id="Light"
                        oninput='sendButtonInput("Light",value)'>
                </div>
            </td>
        </tr>
        <tr style="position: relative;margin-bottom: 20px;">
            <td style="text-align:left"><b>Pan:</b></td>
            <td colspan=2>
                <div class="slidecontainer">
                    <input type="range" min="0" max="180" value="90" class="slider" id="servoX"
                        oninput='sendButtonInput("servoX",value)'>
                </div>
            </td>
        </tr>
        <tr style="position: relative;margin-bottom: 20px;">
            <td style="text-align:left"><b>Tilt:</b></td>
            <td colspan=2>
                <div class="slidecontainer">
                    <input type="range" min="0" max="180" value="90" class="slider" id="servoY"
                        oninput='sendButtonInput("servoY",value)'>
                </div>
            </td>
        </tr>
    </table>
    <canvas id="canvas" name="game" style="position: absolute;right: 280px;top: 100px;"></canvas>

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
                sendButtonInput("Light", document.getElementById("Light").value);
                sendButtonInput("servoX", document.getElementById("servoX").value);
                sendButtonInput("servoY", document.getElementById("servoY").value);
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
    </script>
    <script>
        var canvas, ctx;

        window.addEventListener('load', () => {

            canvas = document.getElementById('canvas');
            ctx = canvas.getContext('2d');
            resize();

            document.addEventListener('mousedown', startDrawing);
            document.addEventListener('mouseup', stopDrawing);
            document.addEventListener('mousemove', Draw);

            document.addEventListener('touchstart', startDrawing);
            document.addEventListener('touchend', stopDrawing);
            document.addEventListener('touchcancel', stopDrawing);
            document.addEventListener('touchmove', Draw);
            window.addEventListener('resize', resize);

            document.getElementById("x_coordinate").innerText = 0;
            document.getElementById("y_coordinate").innerText = 0;
            document.getElementById("speed").innerText = 0;
            document.getElementById("angle").innerText = 0;
        });




        var width, height, radius, x_orig, y_orig;
        function resize() {
            width = window.innerWidth;
            radius = 70;
            height = radius * 6.5;
            ctx.canvas.width = width;
            ctx.canvas.height = height;
            background();
            joystick(width / 2, height / 3);
        }

        function background() {
            x_orig = width / 2;
            y_orig = height / 3;

            ctx.beginPath();
            ctx.arc(x_orig, y_orig, radius + 20, 0, Math.PI * 2, true);
            ctx.fillStyle = '#ECE5E5';
            ctx.fill();
        }

        function joystick(width, height) {
            ctx.beginPath();
            ctx.arc(width, height, radius, 0, Math.PI * 2, true);
            ctx.fillStyle = '#F08080';
            ctx.fill();
            ctx.strokeStyle = '#F6ABAB';
            ctx.lineWidth = 8;
            ctx.stroke();
        }

        let coord = { x: 0, y: 0 };
        let paint = false;

        function getPosition(event) {
            var mouse_x = event.clientX || event.touches[0].clientX;
            var mouse_y = event.clientY || event.touches[0].clientY;
            coord.x = mouse_x - canvas.offsetLeft;
            coord.y = mouse_y - canvas.offsetTop;
        }

        function is_it_in_the_circle() {
            var current_radius = Math.sqrt(Math.pow(coord.x - x_orig, 2) + Math.pow(coord.y - y_orig, 2));
            if (radius >= current_radius) return true
            else return false
        }


        function startDrawing(event) {
            paint = true;
            getPosition(event);
            if (is_it_in_the_circle()) {
                ctx.clearRect(0, 0, canvas.width, canvas.height);
                background();
                joystick(coord.x, coord.y);
                Draw();
            }
        }


        function stopDrawing() {
            paint = false;
            ctx.clearRect(0, 0, canvas.width, canvas.height);
            background();
            joystick(width / 2, height / 3);
            document.getElementById("x_coordinate").innerText = 0;
            document.getElementById("y_coordinate").innerText = 0;
            document.getElementById("speed").innerText = 0;
            document.getElementById("angle").innerText = 0;
            sendButtonInput("MoveCar", "0");
        }

        function Draw(event) {

            if (paint) {
                ctx.clearRect(0, 0, canvas.width, canvas.height);
                background();
                var angle_in_degrees, x, y, speed;
                var angle = Math.atan2((coord.y - y_orig), (coord.x - x_orig));

                if (Math.sign(angle) == -1) {
                    angle_in_degrees = Math.round(-angle * 180 / Math.PI);
                }
                else {
                    angle_in_degrees = Math.round(360 - angle * 180 / Math.PI);
                }


                if (is_it_in_the_circle()) {
                    joystick(coord.x, coord.y);
                    x = coord.x;
                    y = coord.y;
                }
                else {
                    x = radius * Math.cos(angle) + x_orig;
                    y = radius * Math.sin(angle) + y_orig;
                    joystick(x, y);
                }

                getPosition(event);

                var speed = Math.round(100 * Math.sqrt(Math.pow(x - x_orig, 2) + Math.pow(y - y_orig, 2)) / radius);

                var x_relative = Math.round(x - x_orig);
                var y_relative = Math.round(y - y_orig);


                document.getElementById("x_coordinate").innerText = x_relative;
                document.getElementById("y_coordinate").innerText = y_relative;
                document.getElementById("speed").innerText = speed;
                document.getElementById("angle").innerText = angle_in_degrees;
                sendButtonInput("joyStickX", x_relative);
                sendButtonInput("joyStickY", y_relative);
                sendButtonInput("Speed", speed);
            }
        } 
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
  static byte joyStickX = 0;
  static byte joyStickY = 0;
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
        if (key == "joyStickX") {
          joyStickX = valueInt;
          Serial.println("joyStickX :"+joyStickX);
        } else if (key == "joyStickY") {
          joyStickY = valueInt;
          Serial.println("joyStickY :"+joyStickY);
        } else if (key == "Speed") {
          speedMoveStatus = map(valueInt, 0, 100, 0, 255);
          Serial.println("speedMoveStatus :"+speedMoveStatus);
        } else if (key == "Light") {
          analogWrite(4, valueInt);
          delay(30);
        } else if (key == "servoX") {
          spectatorViewX = valueInt;
          Serial.println("spectatorViewX :"+spectatorViewX);
        } else if (key == "servoY") {
          spectatorViewY = valueInt;
          Serial.println("spectatorViewY :"+spectatorViewY);
        }
      }
      dataTransmitter = moveControl(joyStickX, joyStickY) + "," + String(speedMoveStatus) + "," + String(spectatorViewX) + "," + String(spectatorViewY) + "&";
      Serial.print(dataTransmitter);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
    default:
      break;
  }
}


byte moveControl(int joyStickX, int joyStickY) {
  byte status = 0;
  if (joyStickX < 200 && joyStickY > 823) {
    //forwardLeft
    status = 1;
  } else if (joyStickX < 200 && joyStickY < 200) {
    //forwardRight
    status = 2;
  } else if (joyStickX > 823 && joyStickY > 823) {
    //backwardLeft
    status = 3;
  } else if (joyStickX > 823 && joyStickY < 200) {
    //backwardRight
    status = 4;
  } else if (joyStickX < 200) {
    //forward
    status = 5;
  } else if (joyStickX > 823) {
    //backward
    status = 6;
  } else if (joyStickY > 823) {
    //left
    status = 7;
  } else if (joyStickY < 200) {
    //right
    status = 8;
  }
  return status;
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
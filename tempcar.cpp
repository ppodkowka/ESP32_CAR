#include "esp_camera.h"
#include <WiFi.h>
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "fb_gfx.h"
#include "soc/soc.h"             // disable brownout problems
#include "soc/rtc_cntl_reg.h"    // disable brownout problems
#include "esp_http_server.h"
#include <PCF8574.h>

// setting car wifi credentials
const char* ssid = "MyCar";
const char* password = "123456789";

#define PART_BOUNDARY "123456789000000000000987654321"

#define CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// defining expander and pins SDA=12 and SCL=13
PCF8574 expander(0x20,12,13); 



// defining motor 1 and 2
const int motor1Pin1 = 0; // bylo 14 teraz 0 bo expander
const int motor1Pin2 = 1; // bylo 15 teraz 1 bo expander
const int motor2Pin1 = 2; // bylo 13 teraz 2 bo expander
const int motor2Pin2 = 3; // bylo 12 teraz 3 bo expander
const int enablePin = 2; // pin 2 esp 

// defining led pin
const int ledPin = 4;
int ledState = 0;   // variable to store the read value

//Setting PWM properties
const int freq = 30000;
const int pwmChannel = 0;
const int pwmChannelLED = 1;
const int pwmResolution = 8;
int speed = 250;
int ledBrightness = 0;
int ledFlag = 0;


static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

httpd_handle_t camera_httpd = NULL;
httpd_handle_t stream_httpd = NULL;

static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<html>
  <head>
    <title>ESP CAR</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
      body { font-family: Verdana; text-align: center; margin:0px auto; padding-top: 30px;}
      table { margin-left: auto; margin-right: auto; }
      td { padding: 8 px; }
      .button {
        background-color: #2E4057;
        border: none;
        border-radius: 10px;
        color: #c9d6e0;
        padding: 10px 20px;
        text-align: center;
        text-decoration: none;
        display: inline-block;
        font-size: 18px;
        margin: 6px 3px;
        cursor: pointer;
        -webkit-touch-callout: none;
        -webkit-user-select: none;
        -khtml-user-select: none;
        -moz-user-select: none;
        -ms-user-select: none;
        user-select: none;
        -webkit-tap-highlight-color: rgba(0,0,0,0);
        box-shadow: 7px 6px 28px 1px rgba(0, 0, 0, 0.24);
      }
      .button:active {
        background-color: #24232b8e;
        transform: scale(0.98);
        box-shadow: 3px 2px 22px 1px rgba(0, 0, 0, 0.24);
        }
      .button_stop {
        background-color: #922D50;
        }
      img {  width: auto ;
        max-width: 100% ;
        height: auto ; 
      }
      .slider { 
        -webkit-appearance: none; 
        margin: 14px; 
        width: 160px; 
        height: 25px; 
        border-radius: 10px; 
        background: #FFD65C;
        outline: none; 
        -webkit-transition: .2s; 
        transition: opacity .2s;
        }

       .slider:hover {
        opacity: 1; 
        }

       .slider::-webkit-slider-thumb {
        -webkit-appearance: none; 
        appearance: none; 
        border-radius: 10px; 
        width: 35px; 
        height: 35px; 
        background: #003249; 
        cursor: pointer;
        }
       .slider::-moz-range-thumb { 
        width: 35px; 
        height: 35px; 
        background: #003249; 
        cursor: pointer; 
        } 
       .switch {
        position: relative;
        display: inline-block;
        width: 60px;
        height: 34px;
      }

      .switch input { 
        opacity: 0;
        width: 0;
        height: 0;
      }

      .slider1 {
        position: absolute;
        cursor: pointer;
        top: 0;
        left: 0;
        right: 0;
        bottom: 0;
        background-color: #ccc;
        -webkit-transition: .4s;
        transition: .4s;
      }

      .slider1:before {
        position: absolute;
        content: "";
        height: 26px;
        width: 26px;
        left: 4px;
        bottom: 4px;
        background-color: white;
        -webkit-transition: .4s;
        transition: .4s;
      }

      input:checked + .slider1 {
        background-color: #2196F3;
      }

      input:focus + .slider1 {
        box-shadow: 0 0 1px #2196F3;
      }

      input:checked + .slider1:before {
        -webkit-transform: translateX(26px);
        -ms-transform: translateX(26px);
        transform: translateX(26px);
      }

      /* Rounded sliders */
      .slider1.round {
        border-radius: 34px;
      }

      .slider1.round:before {
        border-radius: 50%;
      }
       
    </style>
  </head>
  <body>
    <h1>ESP CAR</h1>
    <img src="" id="photo" >
    <table>
      <tr><td colspan="3" align="center"><button class="button" ontouchstart="toggleCheckbox('forward');" ontouchend="toggleCheckbox('stop');">&#8593;</button></td></tr>
      <tr><td align="center"><button class="button" ontouchstart="toggleCheckbox('left');" ontouchend="toggleCheckbox('stop');">&#8592;</button></td><td align="center"><button class="button button_stop" ontouchstart="toggleCheckbox('stop');">Stop</button></td><td align="center"><button class="button" ontouchstart="toggleCheckbox('right');" ontouchend="toggleCheckbox('stop');">&#8594;</button></td></tr>
      <tr><td colspan="3" align="center"><button class="button" ontouchstart="toggleCheckbox('backward');" ontouchend="toggleCheckbox('stop');">&#8595;</button></td></tr>                   
    </table>
    <table>                
      <tr><td colspan="3" align="center">CAR SPEED:<span id='speedSliderValue'>MEDIUM</span></td><td colspan="3" align="center">LED</td></tr>
      <tr><td colspan="3" align="center"><input type="range" onchange="toggleSlider(this)" id="speed" min="190" max="250" value="220" step="30" class="slider"></td><td colspan="3" align="center"><label class="switch"><input type="checkbox" onclick="toggleCheckbox('led')"><span class="slider1 round"></span></label></td></tr>
    </table>
   <script>
   function toggleCheckbox(x) {
    var HttpRequest = new XMLHttpRequest();
    HttpRequest.open("GET", "/action?go=" + x +"&val=" , true);
    HttpRequest.send();
   }
   function toggleSlider(element) {
    var carSpeed = document.getElementById("speed").value; // gets current value of slider by its ID and saves it in "carSpeed" variable
    if(carSpeed == 190){
      document.getElementById("speedSliderValue").innerHTML = "LOW"; // sets label with id "speedSliderValue" with correct string acording to carSpeed
    }
    else if(carSpeed == 220){
      document.getElementById("speedSliderValue").innerHTML = "MEDIUM";
    }
    else if(carSpeed == 250){
      document.getElementById("speedSliderValue").innerHTML = "HIGH";
    }
    var HttpRequest1 = new XMLHttpRequest();
    HttpRequest1.open("GET", "/action?go=speed&val=" +carSpeed, true);
    HttpRequest1.send();
   }
   window.onload = document.getElementById("photo").src = window.location.href.slice(0, -1) + ":81/stream";
  </script>
  </body>
</html>
)rawliteral";


static esp_err_t index_handler(httpd_req_t *req){
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, (const char *)INDEX_HTML, strlen(INDEX_HTML));
}

// stream handler
static esp_err_t stream_handler(httpd_req_t *req){
  camera_fb_t * fb = NULL;
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t * _jpg_buf = NULL;
  char * part_buf[64];

  res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  if(res != ESP_OK){
    return res;
  }

  while(true){
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      res = ESP_FAIL;
    } else {
      if(fb->width > 400){
        if(fb->format != PIXFORMAT_JPEG){
          bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
          esp_camera_fb_return(fb);
          fb = NULL;
          if(!jpeg_converted){
            Serial.println("JPEG compression failed");
            res = ESP_FAIL;
          }
        } else {
          _jpg_buf_len = fb->len;
          _jpg_buf = fb->buf;
        }
      }
    }
    if(res == ESP_OK){
      size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);
      res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
    }
    if(res == ESP_OK){
      res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
    }
    if(res == ESP_OK){
      res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
    }
    if(fb){
      esp_camera_fb_return(fb);
      fb = NULL;
      _jpg_buf = NULL;
    } else if(_jpg_buf){
      free(_jpg_buf);
      _jpg_buf = NULL;
    }
    if(res != ESP_OK){
      break;
    }
    //Serial.printf("MJPG: %uB\n",(uint32_t)(_jpg_buf_len));
  }
  return res;
}

// command handler
static esp_err_t cmd_handler(httpd_req_t *req){
  char*  buf;
  size_t buf_len;
  char variable[32] = {0,};
  char value[32] = {0,};

  buf_len = httpd_req_get_url_query_len(req) + 1;
  if (buf_len > 1) {
    buf = (char*)malloc(buf_len);
    if(!buf){
      httpd_resp_send_500(req);
      return ESP_FAIL;
    }
    if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
      if (httpd_query_key_value(buf, "go", variable, sizeof(variable)) == ESP_OK && httpd_query_key_value(buf, "val", value, sizeof(value)) == ESP_OK) {
      } else {
        free(buf);
        httpd_resp_send_404(req);
        return ESP_FAIL;
      }
    } else {
      free(buf);
      httpd_resp_send_404(req);
      return ESP_FAIL;
    }
    free(buf);
  } else {
    httpd_resp_send_404(req);
    return ESP_FAIL;
  }

  int Value = atoi(value);
  sensor_t * s = esp_camera_sensor_get();
  int res = 0;
  
  if(!strcmp(variable, "forward")) {
    Serial.println("FORWARD");
    expander.digitalWrite(motor1Pin1, LOW); 
    expander.digitalWrite(motor1Pin2, HIGH);
    expander.digitalWrite(motor2Pin1, LOW); 
    expander.digitalWrite(motor2Pin2, HIGH);
  }
  else if(!strcmp(variable, "left")) {
    Serial.println("LEFT");
    expander.digitalWrite(motor1Pin1, LOW);
    expander.digitalWrite(motor1Pin2, HIGH);
    expander.digitalWrite(motor2Pin1, HIGH);
    expander.digitalWrite(motor2Pin2, LOW);
  }
  else if(!strcmp(variable, "right")) {
    Serial.println("Right");
    expander.digitalWrite(motor1Pin1, HIGH);
    expander.digitalWrite(motor1Pin2, LOW);
    expander.digitalWrite(motor2Pin1, LOW);
    expander.digitalWrite(motor2Pin2, HIGH);
  }
  else if(!strcmp(variable, "backward")) {
    Serial.println("Backward");
    expander.digitalWrite(motor1Pin1, HIGH);
    expander.digitalWrite(motor1Pin2, LOW);
    expander.digitalWrite(motor2Pin1, HIGH);
    expander.digitalWrite(motor2Pin2, LOW);
  }
  else if(!strcmp(variable, "stop")) {
    Serial.println("STOP");
    expander.digitalWrite(motor1Pin1, LOW);
    expander.digitalWrite(motor1Pin2, LOW);
    expander.digitalWrite(motor2Pin1, LOW);
    expander.digitalWrite(motor2Pin2, LOW);
  }
  else if(!strcmp(variable, "led")) {
    Serial.println("LED");

    if(ledFlag == 0){
      ledFlag = 1;
      ledBrightness = 250;
      ledcWrite(pwmChannelLED, ledBrightness);
      digitalWrite(ledPin, HIGH);
      Serial.println("h");
    }
    else if(ledFlag == 1){
      ledFlag = 0;
      ledBrightness = 0;
      ledcWrite(pwmChannelLED, ledBrightness);
      digitalWrite(ledPin, LOW);
      Serial.println("l");
    }
  }
  else if(!strcmp(variable, "speed")) {
    Serial.println("Slider");
    if      (Value > 250) Value = 250;
    else if (Value <   100) Value = 100;
    speed = Value;
    ledcWrite(pwmChannel, speed);
  }
  else {
    res = -1;
  }

  if(res){
    return httpd_resp_send_500(req);
  }

  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  return httpd_resp_send(req, NULL, 0);
}

void startCameraServer(){
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;
  httpd_uri_t index_uri = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = index_handler,
    .user_ctx  = NULL
  };

  httpd_uri_t cmd_uri = {
    .uri       = "/action",
    .method    = HTTP_GET,
    .handler   = cmd_handler,
    .user_ctx  = NULL
  };
  httpd_uri_t stream_uri = {
    .uri       = "/stream",
    .method    = HTTP_GET,
    .handler   = stream_handler,
    .user_ctx  = NULL
  };
  if (httpd_start(&camera_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(camera_httpd, &index_uri);
    httpd_register_uri_handler(camera_httpd, &cmd_uri);
  }
  config.server_port += 1;
  config.ctrl_port += 1;
  if (httpd_start(&stream_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(stream_httpd, &stream_uri);
  }
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector

  
  expander.pinMode(motor1Pin1, OUTPUT); // P0 is motor1Pin1
  expander.pinMode(motor1Pin2, OUTPUT);
  expander.pinMode(motor2Pin1, OUTPUT);
  expander.pinMode(motor2Pin2, OUTPUT);
  pinMode(enablePin, OUTPUT);
  pinMode(ledPin, OUTPUT);

  // configurating PWM
  ledcSetup(pwmChannel, freq, pwmResolution);
  ledcSetup(pwmChannelLED, freq, pwmResolution);

  // attach the channel to the GPIO 
  ledcAttachPin(enablePin, pwmChannel);
  ledcAttachPin(ledPin, pwmChannelLED);

  // setting speed of PWM
  ledcWrite(pwmChannel, speed);
  ledcWrite(pwmChannelLED, ledBrightness);

  // setting all pins of motor 1 and 2 to low at the beggining
  expander.digitalWrite(motor1Pin1, LOW);
  expander.digitalWrite(motor1Pin2, LOW);
  expander.digitalWrite(motor2Pin1, LOW);
  expander.digitalWrite(motor2Pin2, LOW);
  digitalWrite(ledPin, LOW);


  
  Serial.begin(115200);
  Serial.setDebugOutput(false);
  
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
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
  
  if(psramFound()){
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  
  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  // Wi-Fi connection
  Serial.print("Setting AP (Access Point)…\n");
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("Camera Stream Ready! Go to: http:// ");
  Serial.println(IP);

  
  // Start streaming web server
  startCameraServer();
}


void loop() {
  
}

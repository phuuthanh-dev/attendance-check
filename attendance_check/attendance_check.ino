// pin #2 is IN from sensor
// pin #3 is OUT from arduino
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal.h>
#include <Arduino.h>
#if defined(ESP32) || defined(ARDUINO_RASPBERRY_PI_PICO_W)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include <ESPAsyncWebServer.h>
#include <ESP8266mDNS.h>

MDNSResponder mdns;
AsyncWebServer server(80);

// #include <Firebase_ESP_Client.h>

// Provide the token generation process info.
// #include <addons/TokenHelper.h>

// Provide the RTDB payload printing info and other helper functions.
// #include <addons/RTDBHelper.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "Long Châu"
#define WIFI_PASSWORD "ptlc210703"

// For the following credentials, see examples/Authentications/SignInAsUser/EmailPassword/EmailPassword.ino

/* 2. Define the API Key */
// #define API_KEY "AIzaSyAmSco6lJUfrpfU0Rq6LBh-DLkyunHKMjQ"

// /* 3. Define the RTDB URL */
// #define DATABASE_URL "https://fingerprint-53a7d-default-rtdb.asia-southeast1.firebasedatabase.app/"  //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

// /* 4. Define the user Email and password that alreadey registerd or added in your project */
// #define USER_EMAIL "phuuthanh2003@gmail.com"
// #define USER_PASSWORD "thanhthanh1"

// Define Firebase Data object
SoftwareSerial mySerial(5, 16);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
// FirebaseData fbdo;

// FirebaseAuth auth;
// FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;

const char *input_parameter1 = "fingerid";
String inputString = "";
bool stringComplete = false;  // whether the string is complete
int functionState = 0;
uint8_t id;
uint8_t checkTask = -1;  //for check task
int previousButtonState = 0;
const int RS = D2, EN = D3, d4 = D5, d5 = D6, d6 = D7, d7 = D8;
int numberFinger = 0;
LiquidCrystal lcd(RS, EN, d4, d5, d6, d7);
String input_message;

String enrollPage = {
  "<!DOCTYPE html>"
  "<html>"
  "<head>"
  "    <meta http-equiv='Content-Type' content='text/html; charset=utf-8'>"
  "    <title>Đăng kí vân tay</title>"
  "    <meta name='viewport' content='width=device-width, initial-scale=1'>"
  "    <style>"
  "        .b{width: 120px;"
  "            height: 40px;"
  "            font-size: 21px;"
  "            font-weight: bold;"
  "            color: #fff;"
  "            background-color:#4caf50;"
  "            border-radius: 15px;"
  "            border: none;"
  "            padding: 0 10px;"
  "            transition: 0,2s;"
  "            cursor: pointer;"
  "        }"
  "        .b:hover {"
  "            scale: 0.95;"
  "            opacity: 0.9;"
  "        }"
  "        .t{width: 100px;height: 40px;font-size: 21px;color: #FFF;background-color:#f44336;border-radius: 10px;}"
  "        .a {"
  "        margin-top: 40px;"
  "        display: inline-block;"
  "        text-align: center;"
  "        width: 150px;"
  "        height: 30px;"
  "        line-height: 30px;"
  "        font-size: 21px;"
  "        font-weight: bold;"
  "        background-color: #6495ED;"
  "        text-decoration: none;"
  "        color: white;"
  "        border-radius: 10px;"
  "        padding: 10px;"
  "        transition: 0.5s;"
  "      }"
  ""
  "      .a:hover {"
  "       scale: 0.95;"
  "       opacity: 0.9;"
  "       color: #FFBF00;"
  "      }"
  "    </style>"
  "</head>"
  "<body>"
  "<div style='height: auto; text-align: center; margin-top: 70px'>"
  "    <h1 align='center'>Please type in the ID # (from 1 to 127) you want to save this finger as...</h1>"
  "    <form action='/enrollFG'>"
  "        <input style='height: 40px; width: 100px;' name='fingerid' type='number' value='' min='1' max='127' />"
  "        <button type='submit' class='b'>Đăng kí</button>"
  "    </form>"
  "    <a class='a' href=\"/\">Back to home</a>"
  "</div>"
  "</body>"
  "</html>"

};

String checkId[127];
String attend() {
  Serial.println(checkId[1]);
  String attendHTML = "";
  for (int i = 1; i < 127; i++) {
    if (checkId[i] == "Đã điểm danh") {
      attendHTML += "<p class='b'>User " + String(i) + ": " + checkId[i] + "</p>";
    } else if (checkId[i] == "Đã xóa") {
      continue;
    } else if (checkId[i] == "Chưa điểm danh") {
      attendHTML += "<p class='t'>User " + String(i) + ": " + checkId[i] + "</p>";
    }
  }
  String attend = {
    "<!DOCTYPE html>"
    "<html>"
    "<head>"
    "   <meta http-equiv='Content-Type' content='text/html; charset=utf-8'>"
    "  <title>Xác minh vân tay</title>"
    "  <meta name='viewport' content='width=device-width, initial-scale=1'>"
    "  <style> "
    " h1 {"
    "   font-size: 30px;"
    "   font-weight: bold;"
    "   text-transform: capitalize;"
    " }"
    " .b {"
    "   text-align: center;"
    "   max-width: 300px;"
    "   height: 40px;"
    "   font-size: 21px;"
    "   font-weight: bold;"
    "   line-height: 40px;"
    "   color: #fff;"
    "   margin-right:10px;"
    "   background-color: #4caf50;"
    "   border-radius: 15px;"
    "   padding: 10px 5px;"
    "   transition: 0.2s;"
    "   cursor: pointer;"
    " }"
    " .b:hover {"
    "   scale: 1.05;"
    "   opacity: 0.9;"
    "   color: #DBFF33;"
    " }"
    " .t {"
    "   max-width: 300px;"
    "   height: 40px;"
    "   font-weight: bold;"
    "   font-size: 21px;"
    "   line-height: 40px;"
    "   color: #fff;"
    "   margin-right:10px;"
    "   padding: 10px 5px;"
    "   background-color: #f44336;"
    "   border-radius: 15px;"
    " }"
    " .a {"
    "   display: inline-block;"
    "   text-align: center;"
    "   width: 150px;"
    "   height: 30px;"
    "   line-height: 30px;"
    "   font-size: 21px;"
    "   font-weight: bold;"
    "   background-color: #6495ED;"
    "   text-decoration: none;"
    "   color: white;"
    "   border-radius: 10px;"
    "   padding: 10px;"
    "   transition: 0.5s;"
    " }"
    " .a:hover {"
    "   scale: 0.95;"
    "   opacity: 0.9;"
    "   color: #FFBF00;"
    " }"
    "  </style>"
    "</head>"
    "<body>"
    "<div style='height: auto; text-align: center; margin-top: 70px'>"
    "<h1 align='center'>Xác minh vân tay</h1>"
    "<a class='a' href=\"/\">Back to home</a>"
    "<div style='display: flex; flex-wrap: wrap'>"
    + attendHTML + "</div>"
                   "</div>"
                   "</body>"
                   "</html>"
  };
  return attend;
}

String deleteFG = {
  "<!DOCTYPE html>"
  "<html>"
  "<head>"
  "    <meta http-equiv='Content-Type' content='text/html; charset=utf-8'>"
  "    <title>Xóa vân tay</title>"
  "    <meta name='viewport' content='width=device-width, initial-scale=1'>"
  "    <style>"
  "        .b{width: 100px;"
  "            height: 40px;"
  "            font-size: 21px;"
  "            font-weight: bold;"
  "            color: #fff;"
  "            background-color:#E74C3C;"
  "            border-radius: 15px;"
  "            border: none;"
  "            transition: 0,2s;"
  "            cursor: pointer;"
  "        }"
  "        .b:hover {"
  "            scale: 0.95;"
  "            opacity: 0.9;"
  "        }"
  "        .t{width: 100px;height: 40px;font-size: 21px;color: #FFF;background-color:#f44336;border-radius: 10px;}"
  "        .a {"
  "        margin-top: 40px;"
  "        display: inline-block;"
  "        text-align: center;"
  "        width: 150px;"
  "        height: 30px;"
  "        line-height: 30px;"
  "        font-size: 21px;"
  "        font-weight: bold;"
  "        background-color: #6495ED;"
  "        text-decoration: none;"
  "        color: white;"
  "        border-radius: 10px;"
  "        padding: 10px;"
  "        transition: 0.5s;"
  "      }"
  ""
  "      .a:hover {"
  "       scale: 0.95;"
  "       opacity: 0.9;"
  "       color: #FFBF00;"
  "      }"
  "    </style>"
  "</head>"
  "<body>"
  "    <div style='height: auto; text-align: center; margin-top: 70px'>"
  "        <h1 align='center'>Xóa vân tay (id = 0 để xóa hết)</h1>"
  "        <form action='/deleteFG'>"
  "            <input style='height: 40px; width: 100px;' name='fingerid' type='number' value='' min='0' max='127' />"
  "            <button type='submit' class='b'>Xóa</button>"
  "        </form>"
  "        <a class='a' href=\"/\">Back to home</a>"
  "    </div>"
  "</body>"
  "</html>"

};

String checkExist = {
  "<!DOCTYPE html>"
  "<html>"
  "<head>"
  "    <meta http-equiv='Content-Type' content='text/html; charset=utf-8'>"
  "    <title>Kiểm tra vân tay</title>"
  "    <meta name='viewport' content='width=device-width, initial-scale=1'>"
  "    <style>"
  "        .b{width: 120px;"
  "            height: 40px;"
  "            font-size: 21px;"
  "            font-weight: bold;"
  "            color: #fff;"
  "            background-color:#4caf50;"
  "            border-radius: 15px;"
  "            border: none;"
  "            padding: 0 10px;"
  "            transition: 0,2s;"
  "            cursor: pointer;"
  "        }"
  "        .b:hover {"
  "            scale: 0.95;"
  "            opacity: 0.9;"
  "        }"
  "        .t{width: 100px;height: 40px;font-size: 21px;color: #FFF;background-color:#f44336;border-radius: 10px;}"
  "        .a {"
  "        margin-top: 40px;"
  "        display: inline-block;"
  "        text-align: center;"
  "        width: 150px;"
  "        height: 30px;"
  "        line-height: 30px;"
  "        font-size: 21px;"
  "        font-weight: bold;"
  "        background-color: #6495ED;"
  "        text-decoration: none;"
  "        color: white;"
  "        border-radius: 10px;"
  "        padding: 10px;"
  "        transition: 0.5s;"
  "      }"
  ""
  "      .a:hover {"
  "       scale: 0.95;"
  "       opacity: 0.9;"
  "       color: #FFBF00;"
  "      }"
  "    </style>"
  "</head>"
  "<body>"
  "    <div style='height: auto; text-align: center; margin-top: 70px'>"
  "        <h1 align='center'>Kiểm tra vân tay</h1>"
  "        <form action='/checkFG'>"
  "            <input style='height: 40px; width: 100px;' name='fingerid' type='number' value='' min='1' max='127' />"
  "            <button type='submit' class='b'>Kiểm tra</button>"
  "        </form>"
  "        <a class='a' href=\"/\">Back to home</a>"
  "    </div>"
  "</body>"
  "</html>"

};

String webPage = "<!DOCTYPE html>"
                 "<html>"
                 "<head>"
                 "    <meta http-equiv='Content-Type' content='text/html; charset=utf-8'>"
                 "    <title>Điểm danh</title>"
                 "    <meta name='viewport' content='width=device-width, initial-scale=1'>"
                 "    <style>"
                 "        h1 {"
                 "            font-size: 30px;"
                 "            font-weight: bold;"
                 "            text-transform: capitalize;"
                 "        }"
                 "        .b {"
                 "            text-align: center;"
                 "            width: 110px;"
                 "            height: 40px;"
                 "            font-size: 18px;"
                 "            font-weight: bold;"
                 "            color: #fff;"
                 "            background-color: #4caf50;"
                 "            border-radius: 15px;"
                 "            padding: 10px 5px;"
                 "            transition: 0.2s;"
                 "            cursor: pointer;"
                 "        }"
                 "        .b:hover {"
                 "            scale: 1.05;"
                 "            opacity: 0.9;"
                 "            color: #DBFF33;"
                 "        }"
                 "        .t {"
                 "            width: 100px;"
                 "            height: 40px;"
                 "            font-size: 21px;"
                 "            color: #fff;"
                 "            background-color: #f44336;"
                 "            border-radius: 10px;"
                 "        }"
                 "        .a {"
                 "            display: inline-block;"
                 "            text-align: center;"
                 "            max-width: 300px;"
                 "            height: 30px;"
                 "            line-height: 30px;"
                 "            font-size: 21px;"
                 "            font-weight: bold;"
                 "            background-color: #229954 ;"
                 "            text-decoration: none;"
                 "            color: white;"
                 "            border-radius: 10px;"
                 "            padding: 10px;"
                 "            transition: 0.5s;"
                 "        }"
                 "        .a:hover {"
                 "            scale: 0.95;"
                 "            opacity: 0.9;"
                 "            color: #FFBF00;"
                 "        }"
                 "    </style>"
                 "</head>"
                 "<body>"
                 "<div style='height: auto;margin-top: 70px'>"
                 "    <h1 style='text-align: center'>Điều khiển thiết bị qua WIFI</h1>"
                 "    <table align='center'>"
                 "        <tr>"
                 "            <td>"
                 "                <a class='a' href='/attend'>Điểm danh</a>"
                 "                <a class='a' href='/enroll'>Đăng kí vân tay</a>"
                 "                <a class='a' href='/checkexist'>Kiểm tra vân tay</a>"
                 "                <a class='a' href='/delete'>Xóa vân tay</a>"
                 "            </td>"
                 "        </tr>"
                 "    </table>"
                 "</div>"
                 "</body>"
                 "</html>";

String rsCheck(int idOk) {
  String rs = {
    "<!DOCTYPE html>"
    "<html>"
    "<head>"
    "   <meta http-equiv='Content-Type' content='text/html; charset=utf-8'>"
    "  <title>Kiểm tra vân tay</title>"
    "  <meta name='viewport' content='width=device-width, initial-scale=1'>"
    "  <style>"
    "    .b{width: 100px;height: 40px;font-size: 21px;color: #FFF;background-color:#4caf50;border-radius: 10px;}"
    "    .t{width: 100px;height: 40px;font-size: 21px;color: #FFF;background-color:#f44336;border-radius: 10px;}"
    "    .a{display: inline-block; text-align: center;width: 150px;height: 30px;font-size: 21px;background-color: blue;text-decoration: none;color: white;border-radius: 10px;border: 1px solid black;}"
    "  </style>"
    "</head>"
    "<body>"
    "<div style='height: auto; text-align: center; margin-top: 70px'>"
    "<h1 align='center'>Vân tay với id "
    + String(idOk) + " đã tồn tại</h1>"
                     "<a class='a' href='/checkexist'>Back to check</a>"
                     "</div>"
                     "</body>"
                     "</html>"
  };
  return rs;
}

String rsCheckNot(int idNotOk) {
  String rs = {
    "<!DOCTYPE html>"
    "<html>"
    "<head>"
    "   <meta http-equiv='Content-Type' content='text/html; charset=utf-8'>"
    "  <title>Kiểm tra vân tay</title>"
    "  <meta name='viewport' content='width=device-width, initial-scale=1'>"
    "  <style>"
    "    .b{width: 100px;height: 40px;font-size: 21px;color: #FFF;background-color:#4caf50;border-radius: 10px;}"
    "    .t{width: 100px;height: 40px;font-size: 21px;color: #FFF;background-color:#f44336;border-radius: 10px;}"
    "    .a{display: inline-block; text-align: center;width: 150px;height: 30px;font-size: 21px;background-color: blue;text-decoration: none;color: white;border-radius: 10px;border: 1px solid black;}"
    "  </style>"
    "</head>"
    "<body>"
    "<div style='height: auto; text-align: center; margin-top: 70px'>"
    "<h1 align='center'>Vân tay với id "
    + String(idNotOk) + " không tồn tại</h1>"
                        "<a class='a' href='/checkexist'>Back to check</a>"
                        "</div>"
                        "</body>"
                        "</html>"
  };
  return rs;
}

String xoaThanhCong(int id) {
  String rs = "";
  if (id != 0) {
    rs = {
      "<!DOCTYPE html>"
      "<html>"
      "<head>"
      "   <meta http-equiv='Content-Type' content='text/html; charset=utf-8'>"
      "  <title>Kiểm tra vân tay</title>"
      "  <meta name='viewport' content='width=device-width, initial-scale=1'>"
      "  <style>"
      "    .a{display: inline-block; text-align: center;width: 150px;height: 30px;font-size: 21px;background-color: blue;text-decoration: none;color: white;border-radius: 10px;border: 1px solid black;}"
      "  </style>"
      "</head>"
      "<body>"
      "<div style='height: auto; text-align: center; margin-top: 70px'>"
      "<h1 align='center'>Xóa thành công vân tay với id "
      + String(id) + "</h1>"
                     "<a class='a' href='/delete'>Back to delete</a>"
                     "</div>"
                     "</body>"
                     "</html>"
    };
  } else {
    rs = {
      "<!DOCTYPE html>"
      "<html>"
      "<head>"
      "   <meta http-equiv='Content-Type' content='text/html; charset=utf-8'>"
      "  <title>Kiểm tra vân tay</title>"
      "  <meta name='viewport' content='width=device-width, initial-scale=1'>"
      "  <style>"
      "    .a{display: inline-block; text-align: center;width: 150px;height: 30px;font-size: 21px;background-color: blue;text-decoration: none;color: white;border-radius: 10px;border: 1px solid black;}"
      "  </style>"
      "</head>"
      "<body>"
      "<div style='height: auto; text-align: center; margin-top: 70px'>"
      "<h1 align='center'>Xóa hết vân tay thành công!</h1>"
      "<a class='a' href='/delete'>Back to delete</a>"
      "</div>"
      "</body>"
      "</html>"
    };
  }
  return rs;
}

String daTonTai(String inputmsg) {
  String rs = {
    "<!DOCTYPE html>"
    "<html>"
    "<head>"
    "   <meta http-equiv='Content-Type' content='text/html; charset=utf-8'>"
    "  <title>Kiểm tra vân tay</title>"
    "  <meta name='viewport' content='width=device-width, initial-scale=1'>"
    "  <style>"
    "    .a{display: inline-block; text-align: center;width: 150px;height: 30px;font-size: 21px;background-color: blue;text-decoration: none;color: white;border-radius: 10px;border: 1px solid black;}"
    "  </style>"
    "</head>"
    "<body>"
    "<div style='height: auto; text-align: center; margin-top: 70px'>"
    "<h1 align='center'>Vân tay với id đã tồn tại"
    + inputmsg + "</h1>"
                 "<a class='a' href='/enroll'>Back to enroll</a>"
                 "</div>"
                 "</body>"
                 "</html>"
  };
  return rs;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  if (mdns.begin("esp8266", WiFi.localIP()))
    Serial.println("MDNS responder started");

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    checkTask = -1;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Vui long chon");
    lcd.setCursor(0, 1);
    lcd.print("che do");
    request->send(200, "text/html", webPage);
  });

  server.on("/attend", HTTP_GET, [](AsyncWebServerRequest *request) {
    checkTask = 0;
    request->send(200, "text/html", attend());
  });

  server.on("/enroll", HTTP_GET, [](AsyncWebServerRequest *request) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Nhap id de");
    lcd.setCursor(0, 1);
    lcd.print("dang ki");
    request->send(200, "text/html", enrollPage);
  });

  server.on("/checkexist", HTTP_GET, [](AsyncWebServerRequest *request) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Nhap id de check");
    request->send(200, "text/html", checkExist);
  });

  server.on("/delete", HTTP_GET, [](AsyncWebServerRequest *request) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Nhap id de xoa");
    request->send(200, "text/html", deleteFG);
  });

  server.on("/enrollFG", HTTP_GET, [](AsyncWebServerRequest *request) {
    String input_parameter;
    if (request->hasParam(input_parameter1)) {
      input_message = request->getParam(input_parameter1)->value();
      input_parameter = input_parameter1;
      if (checkId[input_message.toInt()] == "Chưa điểm danh" || checkId[input_message.toInt()] == "Đã điểm danh") {
        request->send(200, "text/html", daTonTai(input_message));
      }
      checkTask = 1;
    } else {
      input_message = "No message sent";
      input_parameter = "none";
    }
    request->send(200, "text/html", enrollPage);
  });

  server.on("/checkFG", HTTP_GET, [](AsyncWebServerRequest *request) {
    String input_parameter;
    if (request->hasParam(input_parameter1)) {
      input_message = request->getParam(input_parameter1)->value();
      input_parameter = input_parameter1;
      int isOk = checkFinger();
      if (isOk) {
        request->send(200, "text/html", rsCheck(id));
      } else {
        request->send(200, "text/html", rsCheckNot(id));
      }
    } else {
      input_message = "No message sent";
      input_parameter = "none";
    }
  });

  server.on("/deleteFG", HTTP_GET, [](AsyncWebServerRequest *request) {
    String input_parameter;
    if (request->hasParam(input_parameter1)) {
      input_message = request->getParam(input_parameter1)->value();
      input_parameter = input_parameter1;
      int parse_id = input_message.toInt();
      deleteFingerPrintWithId(input_message, parse_id);
      request->send(200, "text/html", xoaThanhCong(id));
    } else {
      input_message = "No message sent";
      input_parameter = "none";
    }
  });

  server.begin();

  // Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  // /* Assign the api key (required) */
  // config.api_key = API_KEY;

  // /* Assign the user sign in credentials */
  // auth.user.email = USER_EMAIL;
  // auth.user.password = USER_PASSWORD;

  // /* Assign the RTDB URL (required) */
  // config.database_url = DATABASE_URL;

  // /* Assign the callback function for the long running token generation task */
  // config.token_status_callback = tokenStatusCallback;  // see addons/TokenHelper.h

  // // Since Firebase v4.4.x, BearSSL engine was used, the SSL buffer need to be set.
  // // Large data transmission may require larger RX buffer, otherwise connection issue or data read time out can be occurred.
  // fbdo.setBSSLBufferSize(4096, 1024);

  // Or use legacy authenticate method
  // config.database_url = DATABASE_URL;
  // config.signer.tokens.legacy_token = "<database secret>";

  // Firebase.begin(&config, &auth);

  // Comment or pass false value when WiFi reconnection will control by your code or third party library e.g. WiFiManager
  // Firebase.reconnectNetwork(true);

  lcd.begin(16, 2);
  while (!Serial)
    ;  // For Yun/Leo/Micro/Zero/...
  delay(100);
  Serial.println("\n\nAdafruit Fingerprint sensor enrollment");
  // set the data rate for the sensor serial port
  finger.begin(57600);
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }
  lcd.print("Hello there!");
  Serial.println(F("Reading sensor parameters"));
  finger.getParameters();
  Serial.print(F("Status: 0x"));
  Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x"));
  Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: "));
  Serial.println(finger.capacity);
  Serial.print(F("Security level: "));
  Serial.println(finger.security_level);
  Serial.print(F("Device address: "));
  Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: "));
  Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: "));
  Serial.println(finger.baud_rate);
  finger.getTemplateCount();
  Serial.print(F("Sensor contains: "));
  Serial.print(finger.templateCount);
  Serial.println(" templates");
}

uint8_t stringToUint8(String str) {
  int intValue = str.toInt();            // Convert the string to an integer
  if (intValue < 0 || intValue > 255) {  // Check if the integer value is within the range of uint8_t
    // Handle the case where the value is out of range
    Serial.println("Invalid value. Must be between 0 and 255.");
    return 0;  // Return 0 or handle appropriately based on your requirements
  }
  return static_cast<uint8_t>(intValue);  // Convert the integer to uint8_t
}

void loop() {
  if (checkTask == -1) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Vui long chon");
    lcd.setCursor(0, 1);
    lcd.print("che do");
  } else if (checkTask == 0) {
    getFingerprintID();
  } else if (checkTask == 1)  //Add template fingerprint
  {
    Serial.println("Ready to enroll a fingerprint!");
    Serial.println("Please type in the ID # (from 1 to 127) you want to save this finger as...");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Nhap ID");
    int parse_id = input_message.toInt();
    id = stringToUint8(input_message);
    Serial.println(input_message);
    //    Serial.println(getUser(parse_id));
    if (parse_id == 0) {  // ID #0 not allowed, try again!
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Khong hop le");
      delay(1000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Thu lai");
    }
    Serial.print("Enrolling ID #");
    Serial.println(id);
    while (!getFingerprintEnroll(parse_id)) {
    }
    checkTask = -1;
  } else if (checkTask == 2)  //check template
  {
    checkFinger();
  } else if (checkTask == 3)  //delete template
  {
    id = stringToUint8(input_message);
    int parse_id = input_message.toInt();
    if (id == 0)  //delete all template
    {
      finger.emptyDatabase();
      lcd.print("Empty database");
      Serial.println("Now database is empty");
    } else {
      deleteFingerprint(id, parse_id);
      lcd.print("Delete user id" + input_message);
      Serial.println("Delete user id " + input_message);
    }
    checkTask = -1;
  }
}

bool checkFinger() {
  bool isOk = false;
  // Parse input_message to uint8_t
  id = stringToUint8(input_message);

  // Load fingerprint model by id
  uint8_t p = finger.loadModel(id);

  // Display result based on the loaded model
  Serial.print("ID ");
  Serial.print(id);
  if (p == FINGERPRINT_OK) {
    isOk = true;
    Serial.println(" is exist");
  } else {
    Serial.println(" is NOT exist");
  }

  // Reset checkTask flag
  Serial.println(isOk);
  return isOk;
}

void deleteFingerPrintWithId(String input_message, int parse_id) {
  id = stringToUint8(input_message);
  if (id == 0)  //delete all template
  {
    finger.emptyDatabase();
    for (int i = 1; i < 127; i++) {
      checkId[i] = "Đã xóa";
    }
    lcd.print("Empty database");
    Serial.println("Now database is empty");
  } else {
    deleteFingerprint(id, parse_id);
    lcd.print("Delete user id" + input_message);
    Serial.println("Delete user id " + input_message);
  }
  checkTask = -1;
}

// uint8_t readnumber(void) {
//   uint8_t num = 0;
//   while (num == 0) {
//     while (!Serial.available())
//       ;
//     num = Serial.parseInt();
//   }
//   return num;
// }

uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Diem danh");
      delay(500);
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  // OK success!
  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  // OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Thu lai");
    delay(2000);
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  // found a match!
  Serial.print("Found ID #");
  Serial.print(finger.fingerID);
  Serial.print(" with confidence of ");
  Serial.println(finger.confidence);
  // updateUser(finger.fingerID, "Đã điểm danh");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Da diem danh");
  delay(2000);
  checkId[finger.fingerID] = "Đã điểm danh";
  return finger.fingerID;
}

uint8_t getFingerprintEnroll(int parse_id) {
  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Dau van tay");
  Serial.println(id);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(".");
        delay(500);
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
  }

  // OK success!
  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  Serial.println("Remove finger");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Bo tay");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID ");
  Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Xac nhan");
  lcd.setCursor(0, 1);
  lcd.print("Dau van tay");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
  }

  // OK success!
  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  Serial.print("Creating model for #");
  Serial.println(id);
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    lcd.setCursor(0, 0);
    lcd.print("Van tay");
    lcd.setCursor(0, 1);
    lcd.print("Khong khop");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  Serial.print("ID ");
  Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
    checkId[parse_id] = "Chưa điểm danh";
    // updateUser(id, "Chưa điểm danh");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Luu van tay");
    lcd.setCursor(0, 1);
    lcd.print("Thanh cong");
    delay(5000);
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }
  return true;
}

uint8_t deleteFingerprint(uint8_t id, int parse_id) {
  uint8_t p = -1;
  p = finger.deleteModel(id);
  if (p == FINGERPRINT_OK) {
    // int parse_id = input_message.toInt();
    // updateUser(id, "Đã xóa");
    checkId[parse_id] = "Đã xóa";
    Serial.println("Deleted!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not delete in that location");
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
  } else {
    Serial.print("Unknown error: 0x");
    Serial.println(p, HEX);
  }
  return p;
}

// int updateUser(int user, String value) {
//   String userIdString = "user" + String(user);
//   if (Firebase.RTDB.setString(&fbdo, "user/" + userIdString, value)) {
//     return 1;
//   } else {
//     Serial.println("FAILED");
//     Serial.println("REASON: " + fbdo.errorReason());
//     return 0;
//   }
// }

// int getUser(int user) {
//   String userIdString = "user";
//   userIdString.concat(user);
//   Serial.println(userIdString);
//   if (Firebase.RTDB.getString(&fbdo, userIdString.c_str())) {
//     String retrievedValue = String(fbdo.to<float>()).c_str();
//     return (retrievedValue != "Đã xóa") ? 1 : 0;
//   } else {
//     Serial.println("FAILED");
//     Serial.println("REASON: " + fbdo.errorReason());
//     return 0;
//   }
// }

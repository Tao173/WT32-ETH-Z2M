#include <Arduino.h>
#include <ArduinoJson.h>
#include <ETH.h>
#include "WiFi.h"
#include <WebServer.h>
#include "FS.h"
#include "LITTLEFS.h"
#include "web.h"
#include "config.h"
#include "log.h"
#include "etc.h"
#include <Update.h>
#include "html.h"
#include "zigbee.h"

#include "webh/glyphicons.woff.gz.h"
#include "webh/required.css.gz.h"
#include "webh/bootstrap.min.js.gz.h"
#include "webh/functions.js.gz.h"
#include "webh/jquery-min.js.gz.h"
#include "webh/logo.png.gz.h"
#include "webh/nok.png.gz.h"
#include "webh/ok.png.gz.h"
#include "webh/wait.gif.gz.h"

extern struct ConfigSettingsStruct ConfigSettings;
extern unsigned long timeLog;

WebServer serverWeb(80);

void webServerHandleClient()
{
  serverWeb.handleClient();
}

void initWebServer()
{
  serverWeb.on("/js/bootstrap.min.js", handle_bootstrap_js);
  serverWeb.on("/js/functions.js", handle_functions_js);
  serverWeb.on("/js/jquery-min.js", handle_jquery_js);
  serverWeb.on("/css/required.css", handle_required_css);
  serverWeb.on("/fonts/glyphicons.woff", handle_glyphicons_woff);
  serverWeb.on("/img/logo.png", handle_logo_png);
  serverWeb.on("/img/wait.gif", handle_wait_gif);
  serverWeb.on("/img/nok.png", handle_nok_png);
  serverWeb.on("/img/ok.png", handle_ok_png);
  serverWeb.on("/", handleRoot);
  serverWeb.on("/general", handleGeneral);
  serverWeb.on("/wifi", handleWifi);
  serverWeb.on("/ethernet", handleEther);
  serverWeb.on("/serial", handleSerial);
  serverWeb.on("/mqtt", handleMqtt);
  serverWeb.on("/saveGeneral", HTTP_POST, handleSaveGeneral);
  serverWeb.on("/saveSerial", HTTP_POST, handleSaveSerial);
  serverWeb.on("/saveWifi", HTTP_POST, handleSaveWifi);
  serverWeb.on("/saveEther", HTTP_POST, handleSaveEther);
  serverWeb.on("/saveMqtt", HTTP_POST, handleSaveMqtt);
  serverWeb.on("/fsbrowser", handleFSbrowser);
  serverWeb.on("/logs", handleLogs);
  serverWeb.on("/reboot", handleReboot);
  serverWeb.on("/updates", handleUpdate);
  serverWeb.on("/readFile", handleReadfile);
  serverWeb.on("/saveFile", handleSavefile);
  serverWeb.on("/getLogBuffer", handleLogBuffer);
  serverWeb.on("/scanNetwork", handleScanNetwork);
  serverWeb.on("/cmdClearConsole", handleClearConsole);
  serverWeb.on("/cmdGetVersion", handleGetVersion);
  serverWeb.on("/cmdZigRestart", handleZigRestart);
  serverWeb.on("/cmdZigRST", handleZigbeeRestart);
  serverWeb.on("/cmdZigBSL", handleZigbeeBSL);
  serverWeb.on("/switch/firmware_update/toggle", handleZigbeeBSL); //for cc-2538.py ESPHome edition back compatibility | will be disabled on 1.0.0
  /*serverWeb.on("/help", handleHelp);*/
  serverWeb.on("/esp_update", handleESPUpdate);
  serverWeb.on("/logged-out", handleLoggedOut);
  serverWeb.onNotFound(handleNotFound);

  serverWeb.on("/logout", []()
               { serverWeb.send(401); });

  /*handling uploading firmware file */
  serverWeb.on(
      "/update", HTTP_POST, []()
      {
        serverWeb.sendHeader("Connection", "close");
        serverWeb.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
        ESP.restart();
      },
      []()
      {
        if (checkAuth())
        {
          HTTPUpload &upload = serverWeb.upload();
          if (upload.status == UPLOAD_FILE_START)
          {
            Serial.printf("Update: %s\n", upload.filename.c_str());
            if (!Update.begin(UPDATE_SIZE_UNKNOWN))
            { //start with max available size
              Update.printError(Serial);
            }
          }
          else if (upload.status == UPLOAD_FILE_WRITE)
          {
            /* flashing firmware to ESP*/
            if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
            {
              Update.printError(Serial);
            }
          }
          else if (upload.status == UPLOAD_FILE_END)
          {
            if (Update.end(true))
            { //true to set the size to the current progress
              Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
            }
            else
            {
              Update.printError(Serial);
            }
          }
        }
      });
  serverWeb.begin();
}

void handle_functions_js()
{
  const char *dataType = "text/javascript";
  serverWeb.sendHeader(F("Content-Encoding"), F("gzip"));
  serverWeb.send_P(200, dataType, (const char *)functions_js_gz, functions_js_gz_len);
}

void handle_bootstrap_js()
{
  const char *dataType = "text/javascript";
  serverWeb.sendHeader(F("Content-Encoding"), F("gzip"));
  serverWeb.send_P(200, dataType, (const char *)bootstrap_min_js_gz, bootstrap_min_js_gz_len);
}

void handle_jquery_js()
{
  const char *dataType = "text/javascript";
  serverWeb.sendHeader(F("Content-Encoding"), F("gzip"));
  serverWeb.send_P(200, dataType, (const char *)jquery_min_js_gz, jquery_min_js_gz_len);
}

void handle_required_css()
{
  const char *dataType = "text/css";
  serverWeb.sendHeader(F("Content-Encoding"), F("gzip"));
  serverWeb.send_P(200, dataType, (const char *)required_css_gz, required_css_gz_len);
}

void handle_glyphicons_woff()
{
  const char *dataType = "font/woff";
  serverWeb.sendHeader(F("Content-Encoding"), F("gzip"));
  serverWeb.send_P(200, dataType, (const char *)glyphicons_woff_gz, glyphicons_woff_gz_len);
}

void handle_logo_png()
{
  const char *dataType = "img/png";
  serverWeb.sendHeader(F("Content-Encoding"), F("gzip"));
  serverWeb.send_P(200, dataType, (const char *)logo_png_gz, logo_png_gz_len);
}

void handle_wait_gif()
{
  const char *dataType = "img/gif";
  serverWeb.sendHeader(F("Content-Encoding"), F("gzip"));
  serverWeb.send_P(200, dataType, (const char *)wait_gif_gz, wait_gif_gz_len);
}

void handle_nok_png()
{
  const char *dataType = "img/png";
  serverWeb.sendHeader(F("Content-Encoding"), F("gzip"));
  serverWeb.send_P(200, dataType, (const char *)nok_png_gz, nok_png_gz_len);
}

void handle_ok_png()
{
  const char *dataType = "img/png";
  serverWeb.sendHeader(F("Content-Encoding"), F("gzip"));
  serverWeb.send_P(200, dataType, (const char *)ok_png_gz, ok_png_gz_len);
}

void handleLoggedOut()
{
  String result;
  result += F("<html>");
  result += FPSTR(HTTP_HEADER);
  result.replace("{{logoutLink}}", "");
  result += FPSTR(HTTP_ERROR);
  result += F("</html>");
  result.replace("{{pageName}}", "&#x767B;&#x51FA;");

  serverWeb.send(200, F("text/html"), result);
}

void handleNotFound()
{
  if (checkAuth())
  {
    String result;
    result += F("<html>");
    result += FPSTR(HTTP_HEADER);
    if (ConfigSettings.webAuth)
    {
      result.replace("{{logoutLink}}", LOGOUT_LINK);
    }
    else
    {
      result.replace("{{logoutLink}}", "");
    }
    result += FPSTR(HTTP_ERROR);
    result += F("</html>");
    result.replace("{{pageName}}", "Not found - 404");

    serverWeb.send(404, F("text/html"), result);
  }
}

bool checkAuth()
{
  String result;
  result += F("<html>");
  result += FPSTR(HTTP_HEADER);
  result.replace("{{logoutLink}}", "");

  result += FPSTR(HTTP_ERROR);
  result += F("</html>");
  result.replace("{{pageName}}", "&#x8EAB;&#x4EFD;&#x9A8C;&#x8BC1;&#x5931;&#x8D25;");

  const char *www_realm = "Login Required";

  if (ConfigSettings.webAuth && !serverWeb.authenticate(ConfigSettings.webUser, ConfigSettings.webPass))
  {
    serverWeb.requestAuthentication(DIGEST_AUTH, www_realm, result);
    return false;
  }
  else
  {
    return true;
  }
}

    /*
void handleHelp()
{
  if (checkAuth())
  {
    String result;
    result += F("<html>");
    result += FPSTR(HTTP_HEADER);
    if (ConfigSettings.webAuth)
    {
      result.replace("{{logoutLink}}", LOGOUT_LINK);
    }
    else
    {
      result.replace("{{logoutLink}}", "");
    }
    result += FPSTR(HTTP_HELP);
    result += F("</html>");
    result.replace("{{pageName}}", "&#x5E2E;&#x52A9;");
    if (ConfigSettings.webAuth)
    {
      result.replace("{{logoutLink}}", LOGOUT_LINK);
    }
    else
    {
      result.replace("{{logoutLink}}", "");
    }
    serverWeb.send(200, "text/html", result);
  }
}
*/

void handleGeneral()
{
  if (checkAuth())
  {
    String result;
    result += F("<html>");
    result += FPSTR(HTTP_HEADER);
    if (ConfigSettings.webAuth)
    {
      result.replace("{{logoutLink}}", LOGOUT_LINK);
    }
    else
    {
      result.replace("{{logoutLink}}", "");
    }
    result += FPSTR(HTTP_GENERAL);
    result += F("</html>");

    result.replace("{{pageName}}", "&#x901A;&#x7528;");

    if (ConfigSettings.disableWeb)
    {
      result.replace("{{disableWeb}}", "checked");
    }
    else
    {
      result.replace("{{disableWeb}}", "");
    }

    result.replace("{{refreshLogs}}", (String)ConfigSettings.refreshLogs);
    result.replace("{{hostname}}", (String)ConfigSettings.hostname);

    if (ConfigSettings.webAuth)
    {
      result.replace("{{webAuth}}", "checked");
    }
    else
    {
      result.replace("{{webAuth}}", "");
    }
    result.replace("{{webUser}}", (String)ConfigSettings.webUser);
    result.replace("{{webPass}}", (String)ConfigSettings.webPass);

    serverWeb.send(200, "text/html", result);
  }
}

void handleSaveSucces(String msg)
{
  if (checkAuth())
  {
    String result;
    result += F("<html>");
    result += FPSTR(HTTP_HEADER);
    if (ConfigSettings.webAuth)
    {
      result.replace("{{logoutLink}}", LOGOUT_LINK);
    }
    else
    {
      result.replace("{{logoutLink}}", "");
    }
    result += F("<h2>{{pageName}}</h2>");
    result += F("<div id='main' class='col-sm-12'>");
    result += F("<div id='main' class='col-sm-6'>");
    result += F("<form method='GET' action='reboot' id='upload_form'>");
    result += F("<label>&#x4FDD;&#x5B58; ");
    result += msg;
    result += F(" &#x6210;&#x529F;</label><br><br><br>");
    result += F("<button type='submit' class='btn btn-warning mb-2'>&#x91CD;&#x542F;</button>");
    result += F("</form></div></div>");
    result += F("</html>");
    result.replace("{{pageName}}", "&#x5DF2;&#x4FDD;&#x5B58;");

    serverWeb.send(200, "text/html", result);
  }
}

void handleWifi()
{
  if (checkAuth())
  {
    String result;
    result += F("<html>");
    result += FPSTR(HTTP_HEADER);
    if (ConfigSettings.webAuth)
    {
      result.replace("{{logoutLink}}", LOGOUT_LINK);
    }
    else
    {
      result.replace("{{logoutLink}}", "");
    }
    result += FPSTR(HTTP_WIFI);
    result += F("</html>");

    result.replace("{{pageName}}", "&#x65E0;&#x7EBF;&#x7F51;&#x7EDC;");

    DEBUG_PRINTLN(ConfigSettings.enableWiFi);
    if (ConfigSettings.enableWiFi)
    {

      result.replace("{{checkedWiFi}}", "Checked");
    }
    else
    {
      result.replace("{{checkedWiFi}}", "");
    }
    result.replace("{{ssid}}", String(ConfigSettings.ssid));
    result.replace("{{passWifi}}", String(ConfigSettings.password));
    if (ConfigSettings.dhcpWiFi)
    {
      result.replace("{{modeWiFi}}", "Checked");
    }
    else
    {
      result.replace("{{modeWiFi}}", "");
    }
    result.replace("{{ip}}", ConfigSettings.ipAddressWiFi);
    result.replace("{{mask}}", ConfigSettings.ipMaskWiFi);
    result.replace("{{gw}}", ConfigSettings.ipGWWiFi);

    serverWeb.send(200, "text/html", result);
  }
}

void handleSerial()
{
  if (checkAuth())
  {
    String result;
    result += F("<html>");
    result += FPSTR(HTTP_HEADER);
    if (ConfigSettings.webAuth)
    {
      result.replace("{{logoutLink}}", LOGOUT_LINK);
    }
    else
    {
      result.replace("{{logoutLink}}", "");
    }
    result += FPSTR(HTTP_SERIAL);
    result += F("</html>");

    result.replace("{{pageName}}", "&#x4E32;&#x53E3;");

    if (ConfigSettings.serialSpeed == 9600)
    {
      result.replace("{{selected9600}}", "Selected");
    }
    else if (ConfigSettings.serialSpeed == 19200)
    {
      result.replace("{{selected19200}}", "Selected");
    }
    else if (ConfigSettings.serialSpeed == 38400)
    {
      result.replace("{{selected38400}}", "Selected");
    }
    else if (ConfigSettings.serialSpeed == 57600)
    {
      result.replace("{{selected57600}}", "Selected");
    }
    else if (ConfigSettings.serialSpeed == 115200)
    {
      result.replace("{{selected115200}}", "Selected");
    }
    else
    {
      result.replace("{{selected115200}}", "Selected");
    }
    result.replace("{{socketPort}}", String(ConfigSettings.socketPort));

    serverWeb.send(200, "text/html", result);
  }
}

void handleEther()
{
  if (checkAuth())
  {
    String result;
    result += F("<html>");
    result += FPSTR(HTTP_HEADER);
    if (ConfigSettings.webAuth)
    {
      result.replace("{{logoutLink}}", LOGOUT_LINK);
    }
    else
    {
      result.replace("{{logoutLink}}", "");
    }
    result += FPSTR(HTTP_ETHERNET);
    result += F("</html>");

    result.replace("{{pageName}}", "&#x6709;&#x7EBF;&#x7F51;&#x7EDC;");

    if (ConfigSettings.dhcp)
    {
      result.replace("{{modeEther}}", "Checked");
    }
    else
    {
      result.replace("{{modeEther}}", "");
    }
    result.replace("{{ipEther}}", ConfigSettings.ipAddress);
    result.replace("{{maskEther}}", ConfigSettings.ipMask);
    result.replace("{{GWEther}}", ConfigSettings.ipGW);

    serverWeb.send(200, "text/html", result);
  }
}

void handleMqtt()
{
  if (checkAuth())
  {
    String result;
    result += F("<html>");
    result += FPSTR(HTTP_HEADER);
    if (ConfigSettings.webAuth)
    {
      result.replace("{{logoutLink}}", LOGOUT_LINK);
    }
    else
    {
      result.replace("{{logoutLink}}", "");
    }
    result += FPSTR(HTTP_MQTT);
    result += F("</html>");

    result.replace("{{pageName}}", "MQTT&#x914D;&#x7F6E;");

    if (ConfigSettings.mqttEnable)
    {
      result.replace("{{mqttEnable}}", "Checked");
    }
    else
    {
      result.replace("{{mqttEnable}}", "");
    }
    result.replace("{{mqttServer}}", String(ConfigSettings.mqttServer));
    result.replace("{{mqttPort}}", String(ConfigSettings.mqttPort));
    result.replace("{{mqttUser}}", String(ConfigSettings.mqttUser));
    result.replace("{{mqttPass}}", String(ConfigSettings.mqttPass));
    result.replace("{{mqttTopic}}", String(ConfigSettings.mqttTopic));
    /*
  if (ConfigSettings.mqttRetain)
  {
    result.replace("{{mqttRetain}}", "Checked");
  }
  else
  {
    result.replace("{{mqttRetain}}", "");
  }
  */
    result.replace("{{mqttInterval}}", String(ConfigSettings.mqttInterval));
    if (ConfigSettings.mqttDiscovery)
    {
      result.replace("{{mqttDiscovery}}", "Checked");
    }
    else
    {
      result.replace("{{mqttDiscovery}}", "");
    }

    serverWeb.send(200, "text/html", result);
  }
}

void handleRoot()
{
  if (checkAuth())
  {
    String result;
    result += F("<html>");
    result += FPSTR(HTTP_HEADER);
    if (ConfigSettings.webAuth)
    {
      result.replace("{{logoutLink}}", LOGOUT_LINK);
    }
    else
    {
      result.replace("{{logoutLink}}", "");
    }
    result += FPSTR(HTTP_ROOT);
    result += F("</html>");

    result.replace("{{pageName}}", "&#x72B6;&#x6001;");

    String socketStatus;
    String readableTime;
    getReadableTime(readableTime, ConfigSettings.socketTime);
    if (ConfigSettings.connectedClients > 0)
    {
      socketStatus = "<img src='/img/ok.png'>";
      socketStatus = socketStatus + " " + readableTime + " (" + ConfigSettings.connectedClients;
      if (ConfigSettings.connectedClients > 1)
      {
        socketStatus = socketStatus + " clients)";
      }
      else
      {
        socketStatus = socketStatus + " client)";
      }
    }
    else
    {
      socketStatus = "<img src='/img/nok.png'>";
      socketStatus = socketStatus + " " + readableTime;
    }

    result.replace("{{connectedSocket}}", socketStatus);

    getReadableTime(readableTime, 0);
    result.replace("{{uptime}}", readableTime);

    String CPUtemp;
    getCPUtemp(CPUtemp);
    result.replace("{{deviceTemp}}", CPUtemp);

    result.replace("{{hwRev}}", ConfigSettings.boardName);

    result.replace("{{espModel}}", String(ESP.getChipModel()));
    result.replace("{{espCores}}", String(ESP.getChipCores()));
    result.replace("{{espFreq}}", String(ESP.getCpuFreqMHz()));

    result.replace("{{espHeapFree}}", String(ESP.getFreeHeap() / 1024));
    result.replace("{{espHeapSize}}", String(ESP.getHeapSize() / 1024));

    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    if (chip_info.features & CHIP_FEATURE_EMB_FLASH)
    {
      result.replace("{{espFlashType}}", "embedded");
    }
    else
    {
      result.replace("{{espFlashType}}", "external");
    }

    result.replace("{{espFlashSize}}", String(ESP.getFlashChipSize() / (1024 * 1024)));

    String ethState = "<strong>&#x5DF2;&#x8FDE;&#x63A5;: </strong>";
    if (ConfigSettings.connectedEther)
    {
      int speed = ETH.linkSpeed();
      String SpeedEth = String(speed) + String(" Mbps");
      if (ETH.fullDuplex())
      {
        SpeedEth += String(", FULL DUPLEX");
      }
      else
      {
        SpeedEth += String(", HALF DUPLEX");
      }
      ethState += "<img src='/img/ok.png'>";
      ethState += "<br><strong>MAC&#x5730;&#x5740; : </strong>" + ETH.macAddress();
      ethState += "<br><strong>WAN&#x53E3;&#x901F;&#x7387; : </strong> " + SpeedEth;
      ethState += "<br><strong>&#x6A21;&#x5F0F; : </strong>";
      if (ConfigSettings.dhcp)
      {
        ethState += "DHCP<br><strong>IP : </strong>" + ETH.localIP().toString();
        ethState += "<br><strong>&#x5B50;&#x7F51;&#x63A9;&#x7801; : </strong>" + ETH.subnetMask().toString();
        ethState += "<br><strong>&#x7F51;&#x5173; : </strong>" + ETH.gatewayIP().toString();
      }
      else
      {
        ethState = ethState + "STATIC<br><strong>IP : </strong>" + ConfigSettings.ipAddress;
        ethState = ethState + "<br><strong>&#x5B50;&#x7F51;&#x63A9;&#x7801; : </strong>" + ConfigSettings.ipMask;
        ethState = ethState + "<br><strong>&#x7F51;&#x5173; : </strong>" + ConfigSettings.ipGW;
      }
    }
    else
    {
      ethState += "<img src='/img/nok.png'>";
    }
    result.replace("{{stateEther}}", ethState);

    String wifiState = "<strong>&#x542F;&#x7528; : </strong>";
    if (ConfigSettings.enableWiFi || ConfigSettings.emergencyWifi)
    {
      wifiState += "<img src='/img/ok.png'>";
      if (ConfigSettings.emergencyWifi)
      {
        wifiState += "<strong> WIFI&#x6A21;&#x5F0F;</strong>";
      }
      wifiState += "<br><strong>MAC&#x5730;&#x5740; : </strong>" + WiFi.softAPmacAddress();
      wifiState += "<br><strong>&#x6A21;&#x5F0F; : </strong> ";
      if (ConfigSettings.radioModeWiFi)
      {
        String AP_NameString;
        getDeviceID(AP_NameString);
        wifiState += "AP <br><strong>WIFI&#x540D;&#x79F0; : </strong>" + AP_NameString;
        //wifiState += "<br>&#x65E0;&#x5BC6;&#x7801;";
       // wifiState += "<br><strong>&#x5BC6;&#x7801; : </strong>ZigStar1";
        wifiState += "<br><strong>IP : </strong>192.168.4.1";
      }
      else
      {
        int rssi = WiFi.RSSI();
        String rssiWifi = String(rssi) + String(" dBm");
        wifiState = wifiState + "STA <br><strong>WIFI&#x540D;&#x79F0; : </strong>" + ConfigSettings.ssid;
        wifiState += "<br><strong>&#x4FE1;&#x53F7;&#x5F3A;&#x5EA6; : </strong>" + rssiWifi;
        wifiState += "<br><strong>&#x6A21;&#x5F0F; : </strong>";
        if (ConfigSettings.dhcpWiFi)
        {
          wifiState += "DHCP<br><strong>IP : </strong>" + WiFi.localIP().toString();
          wifiState += "<br><strong>&#x5B50;&#x7F51;&#x63A9;&#x7801; : </strong>" + WiFi.subnetMask().toString();
          wifiState += "<br><strong>&#x7F51;&#x5173; : </strong>" + WiFi.gatewayIP().toString();
        }
        else
        {
          wifiState = wifiState + "STATIC<br><strong>IP : </strong>" + ConfigSettings.ipAddressWiFi;
          wifiState = wifiState + "<br><strong>&#x5B50;&#x7F51;&#x63A9;&#x7801; : </strong>" + ConfigSettings.ipMaskWiFi;
          wifiState = wifiState + "<br><strong>&#x7F51;&#x5173; : </strong>" + ConfigSettings.ipGWWiFi;
        }
      }
    }
    else
    {
      wifiState += "<img src='/img/nok.png'>";
    }
    result.replace("{{stateWifi}}", wifiState);

    String mqttState = "<strong>&#x542F;&#x7528; : </strong>";
    if (ConfigSettings.mqttEnable)
    {
      mqttState += "<img src='/img/ok.png'>";
      mqttState = mqttState + "<br><strong>&#x670D;&#x52A1;&#x5668;&#x5730;&#x5740; : </strong>" + ConfigSettings.mqttServer;
      mqttState += "<br><strong>&#x5DF2;&#x8FDE;&#x63A5; : </strong>";
      if (ConfigSettings.mqttReconnectTime == 0)
      {
        mqttState += "<img src='/img/ok.png'>";
      }
      else
      {
        mqttState += "<img src='/img/nok.png'>";
      }
    }
    else
    {
      mqttState += "<img src='/img/nok.png'>";
    }
    result.replace("{{stateMqtt}}", mqttState);

    serverWeb.send(200, "text/html", result);
  }
}

void handleSaveGeneral()
{
  if (checkAuth())
  {
    String StringConfig;
    String disableWeb;
    String refreshLogs;
    String hostname;
    String webAuth;
    String webUser;
    String webPass;

    if (serverWeb.arg("disableWeb") == "on")
    {
      disableWeb = "1";
    }
    else
    {
      disableWeb = "0";
    }

    if (serverWeb.arg("refreshLogs").toDouble() < 1000)
    {
      refreshLogs = "1000";
    }
    else
    {
      refreshLogs = serverWeb.arg("refreshLogs");
    }

    if (serverWeb.arg("hostname") != "")
    {
      hostname = serverWeb.arg("hostname");
    }
    else
    {
      hostname = "Zigbee2Mqtt";
    }

    if (serverWeb.arg("webAuth") == "on")
    {
      webAuth = "1";
    }
    else
    {
      webAuth = "0";
    }

    if (serverWeb.arg("webUser") != "")
    {
      webUser = serverWeb.arg("webUser");
    }
    else
    {
      webUser = "admin";
    }

    if (serverWeb.arg("webPass") != "")
    {
      webPass = serverWeb.arg("webPass");
    }
    else
    {
      webPass = "admin";
    }

    //DEBUG_PRINTLN(hostname);
    const char *path = "/config/configGeneral.json";

    StringConfig = "{\"disableWeb\":" + disableWeb + ",\"refreshLogs\":" + refreshLogs + ",\"webAuth\":" + webAuth + ",\"webUser\":\"" + webUser + "\",\"webPass\":\"" + webPass + "\",\"hostname\":\"" + hostname + "\"}";
    DEBUG_PRINTLN(StringConfig);
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, StringConfig);

    File configFile = LITTLEFS.open(path, FILE_WRITE);
    if (!configFile)
    {
      DEBUG_PRINTLN(F("failed open"));
    }
    else
    {
      serializeJson(doc, configFile);
    }
    handleSaveSucces("&#x914D;&#x7F6E;");
  }
}

void handleSaveWifi()
{
  if (checkAuth())
  {
    if (!serverWeb.hasArg("WIFISSID"))
    {
      serverWeb.send(500, "text/plain", "BAD ARGS");
      return;
    }

    String StringConfig;
    String enableWiFi;
    if (serverWeb.arg("wifiEnable") == "on")
    {
      enableWiFi = "1";
    }
    else
    {
      enableWiFi = "0";
    }
    String ssid = serverWeb.arg("WIFISSID");
    String pass = serverWeb.arg("WIFIpassword");

    String dhcpWiFi;
    if (serverWeb.arg("dhcpWiFi") == "on")
    {
      dhcpWiFi = "1";
    }
    else
    {
      dhcpWiFi = "0";
    }

    String ipAddress = serverWeb.arg("ipAddress");
    String ipMask = serverWeb.arg("ipMask");
    String ipGW = serverWeb.arg("ipGW");

    const char *path = "/config/configWifi.json";

    StringConfig = "{\"enableWiFi\":" + enableWiFi + ",\"ssid\":\"" + ssid + "\",\"pass\":\"" + pass + "\",\"dhcpWiFi\":" + dhcpWiFi + ",\"ip\":\"" + ipAddress + "\",\"mask\":\"" + ipMask + "\",\"gw\":\"" + ipGW + "\"}";
    DEBUG_PRINTLN(StringConfig);
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, StringConfig);

    File configFile = LITTLEFS.open(path, FILE_WRITE);
    if (!configFile)
    {
      DEBUG_PRINTLN(F("failed open"));
    }
    else
    {
      serializeJson(doc, configFile);
    }
    handleSaveSucces("&#x914D;&#x7F6E;");
  }
}

void handleSaveSerial()
{
  if (checkAuth())
  {
    String StringConfig;
    String serialSpeed = serverWeb.arg("baud");
    String socketPort = serverWeb.arg("port");
    const char *path = "/config/configSerial.json";

    StringConfig = "{\"baud\":" + serialSpeed + ", \"port\":" + socketPort + "}";
    DEBUG_PRINTLN(StringConfig);
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, StringConfig);

    File configFile = LITTLEFS.open(path, FILE_WRITE);
    if (!configFile)
    {
      DEBUG_PRINTLN(F("failed open"));
    }
    else
    {
      serializeJson(doc, configFile);
    }
    handleSaveSucces("&#x914D;&#x7F6E;");
  }
}

void handleSaveEther()
{
  if (checkAuth())
  {
    if (!serverWeb.hasArg("ipAddress"))
    {
      serverWeb.send(500, "text/plain", "BAD ARGS");
      return;
    }

    String StringConfig;
    String dhcp;
    if (serverWeb.arg("dhcp") == "on")
    {
      dhcp = "1";
    }
    else
    {
      dhcp = "0";
    }
    String ipAddress = serverWeb.arg("ipAddress");
    String ipMask = serverWeb.arg("ipMask");
    String ipGW = serverWeb.arg("ipGW");

    const char *path = "/config/configEther.json";

    StringConfig = "{\"dhcp\":" + dhcp + ",\"ip\":\"" + ipAddress + "\",\"mask\":\"" + ipMask + "\",\"gw\":\"" + ipGW + "\"}";
    DEBUG_PRINTLN(StringConfig);
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, StringConfig);

    File configFile = LITTLEFS.open(path, FILE_WRITE);
    if (!configFile)
    {
      DEBUG_PRINTLN(F("failed open"));
    }
    else
    {
      serializeJson(doc, configFile);
    }
    handleSaveSucces("&#x914D;&#x7F6E;");
  }
}

void handleSaveMqtt()
{
  if (checkAuth())
  {
    String StringConfig;
    String enable;
    if (serverWeb.arg("enable") == "on")
    {
      enable = "1";
    }
    else
    {
      enable = "0";
    }

    String server = serverWeb.arg("server");
    String port = serverWeb.arg("port");
    String user = serverWeb.arg("user");
    String pass = serverWeb.arg("pass");
    String topic = serverWeb.arg("topic");
    /*
  String retain;
  if (serverWeb.arg("retain") == "on")
  {
    retain = "1";
  }
  else
  {
    retain = "0";
  }
  */
    String interval = serverWeb.arg("interval");
    String discovery;
    if (serverWeb.arg("discovery") == "on")
    {
      discovery = "1";
    }
    else
    {
      discovery = "0";
    }
    const char *path = "/config/configMqtt.json";

    StringConfig = "{\"enable\":" + enable + ",\"server\":\"" + server + "\",\"port\":" + port + ",\"user\":\"" + user + "\",\"pass\":\"" + pass + "\",\"topic\":\"" + topic + "\",\"interval\":" + interval + ",\"discovery\":" + discovery + "}";

    DEBUG_PRINTLN(StringConfig);
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, StringConfig);

    File configFile = LITTLEFS.open(path, FILE_WRITE);
    if (!configFile)
    {
      DEBUG_PRINTLN(F("failed open"));
    }
    else
    {
      serializeJson(doc, configFile);
    }
    handleSaveSucces("&#x914D;&#x7F6E;");
  }
}

void handleLogs()
{
  if (checkAuth())
  {
    String result;

    result += F("<html>");
    result += FPSTR(HTTP_HEADER);
    if (ConfigSettings.webAuth)
    {
      result.replace("{{logoutLink}}", LOGOUT_LINK);
    }
    else
    {
      result.replace("{{logoutLink}}", "");
    }
    result += F("<h2>{{pageName}}</h2>");
    result += F("<div id='main' class='col-sm-12'>");
    result += F("<div id='help_btns' class='col-sm-8'>");
    result += F("<button type='button' onclick='cmd(\"ClearConsole\");document.getElementById(\"console\").value=\"\";' class='btn btn-secondary'>&#x5237;&#x65B0;&#x63A7;&#x5236;&#x53F0;</button> ");
#ifdef DEBUG
    result += F("<button type='button' onclick='cmd(\"GetVersion\");' class='btn btn-success'>Get Version</button> ");
    result += F("<button type='button' onclick='cmd(\"ZigRestart\");' class='btn btn-danger'>Zig Restart</button> ");
#endif
    result += F("<button type='button' onclick='cmd(\"ZigRST\");' class='btn btn-primary'>&#x91CD;&#x542F;Zigbee</button> ");
    result += F("<button type='button' onclick='cmd(\"ZigBSL\");' class='btn btn-warning'>Zigbee&#x66F4;&#x65B0;&#x6A21;&#x5F0F;</button> ");
    result += F("</div></div>");
    result += F("<div id='main' class='col-sm-8'>");
    result += F("<div class='col-md-12'>&#x539F;&#x59CB;&#x6570;&#x636E;:</div>");
    result += F("<textarea class='col-md-12' id='console' rows='16' ></textarea>");
    result += F("</div>");
    result += F("</body>");
    result += F("<script language='javascript'>");
    result += F("logRefresh({{refreshLogs}});");
    result += F("</script>");
    result += F("</html>");

    result.replace("{{pageName}}", "&#x63A7;&#x5236;&#x53F0;");
    result.replace("{{refreshLogs}}", (String)ConfigSettings.refreshLogs);

    serverWeb.send(200, F("text/html"), result);
  }
}

void handleReboot()
{
  if (checkAuth())
  {
    String result;

    result += F("<html>");
    result += F("<meta http-equiv='refresh' content='1; URL=/'>");
    result += FPSTR(HTTP_HEADER);
    if (ConfigSettings.webAuth)
    {
      result.replace("{{logoutLink}}", LOGOUT_LINK);
    }
    else
    {
      result.replace("{{logoutLink}}", "");
    }
    result += F("<h2>{{pageName}}</h2>");
    result = result + F("</body></html>");
    result.replace("{{pageName}}", "&#x5DF2;&#x91CD;&#x542F;");

    serverWeb.send(200, F("text/html"), result);

    ESP.restart();
  }
}

void handleUpdate()
{
  if (checkAuth())
  {
    String result;
    result += F("<html>");
    result += FPSTR(HTTP_HEADER);
    if (ConfigSettings.webAuth)
    {
      result.replace("{{logoutLink}}", LOGOUT_LINK);
    }
    else
    {
      result.replace("{{logoutLink}}", "");
    }
    result += F("<h2>{{pageName}}</h2>");
    result += F("<div class='btn-group-vertical'>");
    result += F("<a href='/setchipid' class='btn btn-primary mb-2'>setChipId</button>");
    result += F("<a href='/setmodeprod' class='btn btn-primary mb-2'>setModeProd</button>");
    result += F("</div>");

    result = result + F("</body></html>");
    result.replace("{{pageName}}", "&#x5347;&#x7EA7;Zigbee");

    serverWeb.send(200, F("text/html"), result);
  }
}

void handleESPUpdate()
{
  if (checkAuth())
  {
    String result;
    result += F("<html>");
    result += FPSTR(HTTP_HEADER);
    if (ConfigSettings.webAuth)
    {
      result.replace("{{logoutLink}}", LOGOUT_LINK);
    }
    else
    {
      result.replace("{{logoutLink}}", "");
    }
    result += F("<h2>{{pageName}}</h2>");
    result += FPSTR(HTTP_UPDATE);
    result = result + F("</body></html>");
    result.replace("{{pageName}}", "&#x5347;&#x7EA7;ESP32");
    serverWeb.sendHeader("Connection", "close");

    serverWeb.send(200, "text/html", result);
  }
}

void handleFSbrowser()
{
  if (checkAuth())
  {
    String result;
    result += F("<html>");
    result += FPSTR(HTTP_HEADER);
    if (ConfigSettings.webAuth)
    {
      result.replace("{{logoutLink}}", LOGOUT_LINK);
    }
    else
    {
      result.replace("{{logoutLink}}", "");
    }
    result += F("<h2>{{pageName}}</h2>");
    result += F("<div id='main' class='col-sm-12'>");
    result += F("<div id='help_btns' class='col-md-11'>");
    result.replace("{{pageName}}", "&#x6587;&#x4EF6;");

    String str = "";
    File root = LITTLEFS.open("/config");
    File file = root.openNextFile();
    while (file)
    {
      String tmp = file.name();
      tmp = tmp.substring(8);
      result += F("<a href='#' onClick=\"readfile('");
      result += tmp;
      result += F("');\">");
      result += tmp;
      result += F("<br>(");
      result += file.size();
      result += F(" B)</a>");
      file = root.openNextFile();
    }
    result += F("</div>");
    result += F("<div id='main' class='col-md-9'>");
    result += F("<div class='app-main-content'>");
    result += F("<form method='POST' action='saveFile'>");
    result += F("<div class='form-group'>");
    result += F("<div><label for='file'>&#x6587;&#x4EF6;&#xFF1A; <span id='title'></span></label>");
    result += F("<input type='hidden' name='filename' id='filename' value=''></div>");
    result += F("<textarea class='form-control' id='file' name='file' rows='10'>");
    result += F("</textarea>");
    result += F("</div>");
    result += F("<button type='submit' class='btn btn-primary mb-2'>&#x4FDD;&#x5B58;</button>");
    result += F("</form>");
    result += F("</div>");
    result += F("</div>");
    result += F("</div>");
    result += F("</body></html>");

    serverWeb.send(200, F("text/html"), result);
  }
}

void handleReadfile()
{
  if (checkAuth())
  {
    String result;
    String filename = "/config/" + serverWeb.arg(0);
    File file = LITTLEFS.open(filename, "r");

    if (!file)
    {
      return;
    }

    while (file.available())
    {
      result += (char)file.read();
    }
    file.close();

    serverWeb.send(200, F("text/html"), result);
  }
}

void handleSavefile()
{
  if (checkAuth())
  {
    if (serverWeb.method() != HTTP_POST)
    {
      serverWeb.send(405, F("text/plain"), F("Method Not Allowed"));
    }
    else
    {
      String filename = "/config/" + serverWeb.arg(0);
      String content = serverWeb.arg(1);
      File file = LITTLEFS.open(filename, "w");
      if (!file)
      {
        DEBUG_PRINT(F("Failed to open file for reading\r\n"));
        return;
      }

      int bytesWritten = file.print(content);

      if (bytesWritten > 0)
      {
        DEBUG_PRINTLN(F("File was written"));
        DEBUG_PRINTLN(bytesWritten);
      }
      else
      {
        DEBUG_PRINTLN(F("File write failed"));
      }

      file.close();
      serverWeb.sendHeader(F("Location"), F("/fsbrowser"));
      serverWeb.send(303);
    }
  }
}

void handleLogBuffer()
{
  if (checkAuth())
  {
    String result;
    result = logPrint();

    serverWeb.send(200, F("text/html"), result);
  }
}

void handleScanNetwork()
{
  if (checkAuth())
  {
    String result = "";
    int n = WiFi.scanNetworks();
    if (n == 0)
    {
      result = " <label for='ssid'>WIFI&#x540D;&#x79F0;</label>";
      result += "<input class='form-control' id='ssid' type='text' name='WIFISSID' value='{{ssid}}'> <a onclick='scanNetwork();' class='btn btn-primary mb-2'>Scan</a><div id='networks'></div>";
    }
    else
    {

      result = "<select name='WIFISSID' onChange='updateSSID(this.value);'>";
      result += "<OPTION value=''>&#x9009;&#x62E9;WIFI</OPTION>";
      for (int i = 0; i < n; ++i)
      {
        result += "<OPTION value='";
        result += WiFi.SSID(i);
        result += "'>";
        result += WiFi.SSID(i) + " (" + WiFi.RSSI(i) + ")";
        result += "</OPTION>";
      }
      result += "</select>";
    }

    serverWeb.send(200, F("text/html"), result);
  }
}
void handleClearConsole()
{
  if (checkAuth())
  {
    logClear();

    serverWeb.send(200, F("text/html"), "");
  }
}

void handleGetVersion()
{
  cmdGetZigVersion();
}

void handleZigRestart()
{
  cmdZigRestart();
}

void handleZigbeeRestart()
{
  if (checkAuth())
  {
    serverWeb.send(200, F("text/html"), "");

    zigbeeRestart();
  }
}

void handleZigbeeBSL()
{
  if (checkAuth())
  {
    serverWeb.send(200, F("text/html"), "");

    zigbeeEnableBSL();
  }
}

void printLogTime()
{
  String tmpTime;
  timeLog = millis();
  tmpTime = String(timeLog, DEC);
  logPush('[');
  for (int j = 0; j < tmpTime.length(); j++)
  {
    logPush(tmpTime[j]);
  }
  logPush(']');
}

void printLogMsg(String msg)
{
  printLogTime();
  logPush(' ');
  logPush('|');
  logPush(' ');
  for (int j = 0; j < msg.length(); j++)
  {
    logPush(msg[j]);
  }
  logPush('\n');
}

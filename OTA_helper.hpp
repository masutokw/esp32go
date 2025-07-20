void InitOTA(String mhost) {
  byte i = mhost.length() + 1;
 
  if (i == 1)
    ArduinoOTA.setHostname("ESP32go") ;else {
   char buf[i];
      mhost.toCharArray(buf, i);
      ArduinoOTA.setHostname(buf);
    }

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else {  // U_SPIFFS
      type = "filesystem";
    }
  });
  ArduinoOTA.onEnd([]() {});
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {});
  ArduinoOTA.onError([](ota_error_t error) {});
  ArduinoOTA.begin();
}

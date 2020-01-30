#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <SimpleTimer.h>
#include <vector>

char auth[] = "X8DbT0UKnGgQjUI0oHAGDNEbCtOCuOuM";  //authentication string from Blynk app.
int mins = 30;
const long MillesInMin = 60000;
const int num_sections = 7;
const byte pins[] = {5, 4, 2, 15, 14, 12, 13 };
bool sectionSelected[] = {false, false, false, false, false, false, false};
bool updated = false;
bool started = false;
std::vector<int> sections;

SimpleTimer timer;
int timerId;

char ssid[] = "####";   //SSID
char pass[] = "####";      //Password

std::vector<int> getActiveSections();

BLYNK_WRITE(V10) {
  if (param.asInt()) {
    sections = getActiveSections();
    digitalWrite(sections.back(), LOW);
    timerId = timer.setTimer(MillesInMin * mins, startSection, sections.size());
    started = true;
  } else {
    timer.disable(timerId);
    digitalWrite(sections.back(), HIGH);
    std::vector<int> empty;
    std::swap(sections, empty);
  }
}

BLYNK_WRITE_DEFAULT() {
  int sectionPin = request.pin;
  sectionSelected[sectionPin - 1] = !sectionSelected[sectionPin - 1];
}

BLYNK_WRITE(V15) {
  mins = param.asInt();
}

void startSection() {
  int pinNum = sections.back();
  digitalWrite(pinNum, HIGH);
  for (int i = 0; i < num_sections; i++) {
    if (pins[i] == pinNum) {
      Blynk.virtualWrite(i + 1, 0);
    }
  }
  sections.pop_back();
  if (!sections.empty()) {
    digitalWrite(sections.back(), LOW);
  }
}

std::vector<int> getActiveSections() {
  std::vector<int> sections;
  for (int i = num_sections - 1; i >= 0; i--) {
    if (sectionSelected[i]) {
      sections.push_back(pins[i]);
    }
  }
  if (sections.empty()) {
    for (int i = num_sections - 1; i >= 0; i--) {
      sections.push_back(pins[i]);
    }
  }
  return sections;
}

void setup()
{
  for (int pin : pins) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
  }
  // Debug console
  Serial.begin(115200); //ESP8266 boards

  //Blynk.begin(auth, ssid, pass);
  // You can also specify server:
  Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 8080);
  //Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8080);
}

void loop() {
  Blynk.run();
  timer.run();

  if (Blynk.connected() && !updated) {
    for (int i = 0; i < num_sections; i++) {
      Blynk.virtualWrite(i + 1, sectionSelected[i]);
    }
    Blynk.virtualWrite(V10, 0);
    Blynk.virtualWrite(V15, mins);
    updated = true;
  }
  if (started && !timer.isEnabled(timerId)) {
    finish();
  }
}

void finish() {
  started = false;
  digitalWrite(sections.back(), HIGH);
  std::vector<int> empty;
  std::swap(sections, empty);
  Blynk.virtualWrite(V10, 0);
  for (int i = 0; i < num_sections; i++) {
    Blynk.virtualWrite(i + 1, 0);
    sectionSelected[i] = false;
  }
}

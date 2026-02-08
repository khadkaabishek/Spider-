#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

// ===== Servo objects =====
Servo s1, s2, s3, s4;

// ===== Pins =====
#define SERVO1 13
#define SERVO2 12
#define SERVO3 14
#define SERVO4 27

#define TRIG 5
#define ECHO 18

// ===== WiFi =====
const char* ssid = "SPYDER_ROBOT";
const char* password = "12345678";

WebServer server(80);

// ===== Variables =====
bool moveForward = false;

// ===== Distance Function =====
long getDistance() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  long duration = pulseIn(ECHO, HIGH, 30000);
  return duration * 0.034 / 2;
}

// ===== Movement =====
void stopRobot() {
  moveForward = false;
}

void forwardStep() {
  s1.write(60);
  s3.write(60);
  s2.write(120);
  s4.write(120);
  delay(300);

  s1.write(120);
  s3.write(120);
  s2.write(60);
  s4.write(60);
  delay(300);
}

// ===== Web Page =====
void handleRoot() {
  server.send(200, "text/html",
    "<h2>Spyder Robot Control</h2>"
    "<a href='/forward'><button style='height:50px;width:150px;'>FORWARD</button></a><br><br>"
    "<a href='/stop'><button style='height:50px;width:150px;'>STOP</button></a>"
  );
}

void handleForward() {
  moveForward = true;
  server.send(200, "text/plain", "Moving Forward");
}

void handleStop() {
  stopRobot();
  server.send(200, "text/plain", "Stopped");
}

void setup() {
  Serial.begin(115200);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  s1.attach(SERVO1);
  s2.attach(SERVO2);
  s3.attach(SERVO3);
  s4.attach(SERVO4);

  // Neutral position
  s1.write(90);
  s2.write(90);
  s3.write(90);
  s4.write(90);

  // WiFi AP
  WiFi.softAP(ssid, password);
  Serial.println("WiFi Started");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/forward", handleForward);
  server.on("/stop", handleStop);
  server.begin();
}

void loop() {
  server.handleClient();

  if (moveForward) {
    long distance = getDistance();

    if (distance > 20 || distance == 0) {
      forwardStep();
    } else {
      stopRobot();
    }
  }
}
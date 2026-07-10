// ▣ 핀 설정
int soilPin = A5;     // 토양수분센서 AO
int IN1 = 5;          // 모터드라이버 A_IA
int IN2 = 6;          // 모터드라이버 A_1B

int blueLED  = 11;    // 파란 LED (촉촉 상태)
int greenLED = 10;    // 초록 LED (적당 상태)
int redLED   = 9;     // 빨간 LED (건조 상태)

int buzzer = 13;      // 부저 +극 / -극은 GND

// ▣ 기본 임계값 (센서 측정 기준)
int wetThreshold  = 400;   // 이 값 이하 → 촉촉
int dryThreshold  = 900;   // 이 값 이상 → 건조

// ▣ 히스테리시스 완충 영역
int wetRelease  = 460;   // 촉촉 → 적당으로 변하려면 이 값 이상이어야 함
int dryRelease  = 840;   // 건조 → 적당으로 내려오려면 이 값 이하이어야 함

// ▣ 현재 토양상태 (0=촉촉, 1=적당, 2=건조)
int currentState = 1;   // 초기상태를 적당으로 설정(안전)

// ▣ 타이머(쿨다운)
unsigned long lastPumpTime = 0;      // 마지막 펌프 작동 시간
unsigned long pumpCooldown = 60000;  // 60초(1분) 대기

void setup() {
  Serial.begin(9600);

  pinMode(soilPin, INPUT);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  pinMode(blueLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);

  pinMode(buzzer, OUTPUT);
}

void loop() {

  int soilValue = analogRead(soilPin);
  Serial.print("Soil Moisture Value: ");
  Serial.println(soilValue);

  unsigned long currentTime = millis();
  bool pumpIsAllowed = (currentTime - lastPumpTime >= pumpCooldown);

  // ▣ 현재 상태 결정 (히스테리시스 적용)

  // ● 현재 촉촉 상태인데, 촉촉을 벗어났을 때만 변경
  if (currentState == 0 && soilValue > wetRelease) {
    currentState = 1;   // 촉촉 → 적당
  }
  // ● 현재 적당 상태 → 촉촉 또는 건조로 변경
  else if (currentState == 1) {
    if (soilValue <= wetThreshold) currentState = 0;    // 적당 → 촉촉
    else if (soilValue >= dryThreshold) currentState = 2; // 적당 → 건조
  }
  // ● 현재 건조 상태인데, 건조를 벗어났을 때만 변경
  else if (currentState == 2 && soilValue < dryRelease) {
    currentState = 1;   // 건조 → 적당
  }

  // ============================================================
  // ▣ 상태별 동작
  // ============================================================

  // ■ 촉촉 상태
  if (currentState == 0) {
    Serial.println("State = Very Wet");

    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);

    digitalWrite(blueLED, HIGH);
    digitalWrite(greenLED, LOW);
    digitalWrite(redLED, LOW);

    noTone(buzzer);
  }

  // ■ 적당 상태
  else if (currentState == 1) {
    Serial.println("State = Moderate");

    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);

    digitalWrite(blueLED, LOW);
    digitalWrite(greenLED, HIGH);
    digitalWrite(redLED, LOW);

    noTone(buzzer);
  }

  // ■ 건조 상태
  else if (currentState == 2) {
    Serial.println("State = Dry");

    // 쿨다운이 끝난 경우만 펌프 ON
    if (pumpIsAllowed) {
      Serial.println("Pump ON for 3 seconds");

      digitalWrite(redLED, HIGH);
      digitalWrite(blueLED, LOW);
      digitalWrite(greenLED, LOW);

      tone(buzzer, 1500, 300);

      // 펌프 동작
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);

      delay(3000);

      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);

      lastPumpTime = millis();
    }
    else {
      Serial.println("Cooldown active → Pump OFF");

      digitalWrite(redLED, HIGH);
      digitalWrite(blueLED, LOW);
      digitalWrite(greenLED, LOW);

      noTone(buzzer);
    }
  }

  delay(1000);  // 1초마다 측정 (원하는대로 조절)
}

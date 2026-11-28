#include <Arduino.h>

#define LIDAR_RX 16
#define FR_IN1 26
#define FR_IN2 25
#define FL_IN1 19
#define FL_IN2 27
#define RR_IN1 23
#define RR_IN2 12
#define RL_IN1 0
#define RL_IN2 15

const uint8_t motorPins[] = {
    FR_IN1, FR_IN2, FL_IN1, FL_IN2,
    RR_IN1, RR_IN2, RL_IN1, RL_IN2
};

// 바퀴가 반대로 돌면 해당 자리만 true로 바꾼다. 순서는 FR, FL, RR, RL.
const bool reverseMotor[] = {false, false, false, false};
const int MOTOR_PWM = 100;
const int FRONT_START = 145;  // 정면 180도
const int FRONT_END = 215;
const int STOP_MM = 350;
const int RESUME_MM = 450;

const int PACKET_SIZE = 47;
uint8_t packet[PACKET_SIZE];
bool moving = false;
float lastAngle = 0;  // 0이면 아직 전방을 읽고 있지 않다.
int frontMin = 0;    // 0이면 전방 데이터가 부족하다.
uint32_t lastPacketMs = 0;
uint32_t lastFrontMs = 0;

void setMotors(bool forward) {
    if (moving == forward) return;

    for (int i = 0; i < 4; i++) {
        uint8_t pin = motorPins[2 * i];
        if (reverseMotor[i]) pin = motorPins[2 * i + 1];
        analogWrite(pin, forward ? MOTOR_PWM : 0);
    }
    moving = forward;
    Serial.println(moving ? "FORWARD" : "STOP");
}

uint16_t readUInt16(uint8_t low, uint8_t high) {
    return low | (uint16_t(high) << 8);
}

bool readPacket() {
    // 47바이트가 모였을 때만 읽는다.
    if (Serial2.available() < PACKET_SIZE) return false;
    if (Serial2.read() != 0x54 || Serial2.peek() != 0x2C) return false;

    packet[0] = 0x54;
    for (int i = 1; i < PACKET_SIZE; i++) {
        packet[i] = uint8_t(Serial2.read());
    }

    // 마지막 바이트와 CRC를 비교해서 깨진 패킷을 걸러낸다.
    uint8_t crc = 0;
    for (int i = 0; i < PACKET_SIZE - 1; i++) {
        crc ^= packet[i];
        for (int bit = 0; bit < 8; bit++) {
            crc = uint8_t((crc & 0x80) ? (crc << 1) ^ 0x4D : crc << 1);
        }
    }
    return crc == packet[PACKET_SIZE - 1];
}

void checkFront(float angle, int distance) {
    if (angle >= FRONT_START && angle <= FRONT_END) {
        if (distance == 0 || distance > 12000) return;
        if (lastAngle == 0) {
            lastAngle = FRONT_START;
            frontMin = distance;
        }

        // 6도 넘게 비면 이번 스캔으로 전방이 비었는지 판단할 수 없다.
        if (angle < lastAngle || angle - lastAngle > 6) {
            frontMin = 0;
            setMotors(false);
        } else if (distance < frontMin) {
            frontMin = distance;
        }
        lastAngle = angle;

        if (distance <= STOP_MM) setMotors(false);
    } else if (lastAngle != 0) {
        // 전방 구간을 끝까지 읽은 뒤 재출발 여부를 결정한다.
        if (angle > FRONT_END && lastAngle >= FRONT_END - 6 && frontMin > 0) {
            lastFrontMs = millis();
            if (frontMin > RESUME_MM) setMotors(true);
        } else {
            setMotors(false);
        }
        lastAngle = 0;
    }
}

void setup() {
    for (int i = 0; i < 8; i++) {
        pinMode(motorPins[i], OUTPUT);
        digitalWrite(motorPins[i], LOW);
    }
    Serial.begin(115200);
    Serial2.begin(230400, SERIAL_8N1, LIDAR_RX, 17);  // GPIO17은 연결하지 않는다.
    Serial.println("STOP");
}

void loop() {
    uint32_t now = millis();
    if (now - lastPacketMs >= 150 || (moving && now - lastFrontMs >= 300)) {
        setMotors(false);
        lastAngle = 0;
    }

    if (!readPacket()) return;

    float start = readUInt16(packet[4], packet[5]) / 100.0f;
    float end = readUInt16(packet[42], packet[43]) / 100.0f;
    if (start >= 360 || end >= 360) return;
    if (end < start) end += 360;
    if (end <= start || end - start > 30) return;
    lastPacketMs = now;

    // 한 패킷의 거리 12개를 차례대로 확인한다.
    float step = (end - start) / 11;
    for (int i = 0; i < 12; i++) {
        int index = 6 + 3 * i;
        int distance = readUInt16(packet[index], packet[index + 1]);
        float angle = start + step * i;
        if (angle >= 360) angle -= 360;
        checkFront(angle, distance);
    }
}

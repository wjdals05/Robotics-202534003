#include <Arduino.h>

// ==================================================
// 1. Pin Configuration
// ==================================================

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
    FR_IN1, FR_IN2,
    FL_IN1, FL_IN2,
    RR_IN1, RR_IN2,
    RL_IN1, RL_IN2
};

const bool reverseMotor[] = {
    false, false, false, false
};


// ==================================================
// 2. Parameters
// ==================================================

const int MOTOR_PWM = 100;
const int REVERSE_PWM = 100;
const int TURN_PWM = 100;

const int STOP_MM = 350;
const int RESUME_MM = 450;
const int SIDE_CLEAR_MM = 450;

const int REAR_STOP_MM = 350;
const int REAR_CLEAR_MM = 450;

const int TURN_GUARD_MM = 220;

const uint32_t REVERSE_MS = 400;
const uint32_t TURN_MS = 600;

const uint32_t PACKET_TIMEOUT = 150;
const uint32_t SCAN_TIMEOUT = 300;
const uint32_t FORWARD_RESET_MS = 1000;

const int MAX_TURN_ATTEMPTS = 2;
const int MAX_DISTANCE = 12000;

const float SECTOR_WIDTH = 70.0f;
const float MAX_ANGLE_GAP = 6.0f;


// ==================================================
// 3. LiDAR Sector Configuration
// ==================================================

struct Sector {
    float start;

    bool active = false;
    bool hasPoint = false;
    bool bad = false;
    bool valid = false;

    float lastOffset = 0;

    int scanMin = MAX_DISTANCE + 1;
    int distance = MAX_DISTANCE + 1;

    uint32_t stamp = 0;

    Sector(float startAngle) : start(startAngle) {}
};

Sector front(145.0f);  // 145 ~ 215
Sector left(55.0f);    //  55 ~ 125
Sector right(235.0f);  // 235 ~ 305
Sector rear(325.0f);   // 325 ~ 35

Sector* sectors[] = {
    &front, &left, &right, &rear
};


// ==================================================
// 4. Robot State
// ==================================================

enum class Mode {
    STOPPED,
    FORWARD,
    BACKWARD,
    WAIT_TURN,
    TURN_LEFT,
    TURN_RIGHT
};

Mode mode = Mode::STOPPED;

uint32_t stateSince = 0;
uint32_t lastPacketMs = 0;

int turnAttempts = 0;
bool lastTurnLeft = false;

bool nearSeen = false;
uint32_t lastNearMs = 0;


// ==================================================
// 5. LiDAR Packet Configuration
// ==================================================

const int PACKET_SIZE = 47;

uint8_t packet[PACKET_SIZE];


// ==================================================
// 6. Motor Control
// ==================================================

void setDrive(int leftPWM, int rightPWM) {

    // Stop all motor outputs before changing direction.
    for (int i = 0; i < 8; i++) {
        analogWrite(motorPins[i], 0);
    }

    for (int i = 0; i < 4; i++) {

        // FR, RR: right motors
        // FL, RL: left motors
        int speed = (i % 2 == 0)
                    ? rightPWM
                    : leftPWM;

        if (reverseMotor[i]) {
            speed = -speed;
        }

        uint8_t in1 = motorPins[2 * i];
        uint8_t in2 = motorPins[2 * i + 1];

        if (speed > 0) {
            analogWrite(in1, speed);
        }
        else if (speed < 0) {
            analogWrite(in2, -speed);
        }
    }
}


// ==================================================
// 7. State Transition
// ==================================================

void setMode(Mode next, uint32_t now) {

    if (mode == next) return;

    mode = next;
    stateSince = now;

    switch (mode) {

        case Mode::FORWARD:
            setDrive(MOTOR_PWM, MOTOR_PWM);
            Serial.println("FORWARD");
            break;

        case Mode::BACKWARD:
            setDrive(-REVERSE_PWM, -REVERSE_PWM);
            Serial.println("BACKWARD");
            break;

        case Mode::TURN_LEFT:
            setDrive(-TURN_PWM, TURN_PWM);
            Serial.println("TURN LEFT");
            break;

        case Mode::TURN_RIGHT:
            setDrive(TURN_PWM, -TURN_PWM);
            Serial.println("TURN RIGHT");
            break;

        case Mode::WAIT_TURN:
            setDrive(0, 0);
            Serial.println("WAIT FOR NEW SCAN");
            break;

        case Mode::STOPPED:
            setDrive(0, 0);
            Serial.println("STOP");
            break;
    }
}


// ==================================================
// 8. LiDAR Packet Reception
// ==================================================

uint16_t readUInt16(uint8_t low, uint8_t high) {
    return low | (uint16_t(high) << 8);
}

bool readPacket() {

    if (Serial2.available() < PACKET_SIZE) {
        return false;
    }

    if (Serial2.read() != 0x54 ||
        Serial2.peek() != 0x2C) {
        return false;
    }

    packet[0] = 0x54;

    for (int i = 1; i < PACKET_SIZE; i++) {
        packet[i] = uint8_t(Serial2.read());
    }

    uint8_t crc = 0;

    for (int i = 0; i < PACKET_SIZE - 1; i++) {

        crc ^= packet[i];

        for (int bit = 0; bit < 8; bit++) {

            crc = uint8_t(
                (crc & 0x80)
                ? (crc << 1) ^ 0x4D
                : crc << 1
            );
        }
    }

    return crc == packet[PACKET_SIZE - 1];
}


// ==================================================
// 9. Sector Distance Calculation
// ==================================================

void updateSector(
    Sector &s,
    float angle,
    int distance,
    uint32_t now
) {

    // Convert the angle to a sector-relative angle.
    float offset = angle - s.start;

    if (offset < 0) {
        offset += 360.0f;
    }

    if (offset <= SECTOR_WIDTH) {

        // Start a new sector scan.
        if (!s.active) {

            s.active = true;
            s.hasPoint = false;
            s.bad = false;
            s.lastOffset = 0;
            s.scanMin = MAX_DISTANCE + 1;
        }

        // Ignore invalid distance measurements.
        if (distance == 0 ||
            distance > MAX_DISTANCE) {
            return;
        }

        // Check scan continuity.
        if (offset < s.lastOffset ||
            offset - s.lastOffset > MAX_ANGLE_GAP) {

            s.bad = true;
            s.valid = false;
        }

        if (distance < s.scanMin) {
            s.scanMin = distance;
        }

        s.lastOffset = offset;
        s.hasPoint = true;
    }

    // The LiDAR has exited this sector.
    else if (s.active) {

        s.valid =
            s.hasPoint &&
            !s.bad &&
            s.lastOffset >= SECTOR_WIDTH - MAX_ANGLE_GAP;

        if (s.valid) {

            s.distance = s.scanMin;
            s.stamp = now;
        }

        s.active = false;
    }
}


// ==================================================
// 10. Sector Validation
// ==================================================

bool isFresh(const Sector &s, uint32_t now) {

    return s.valid &&
           now - s.stamp < SCAN_TIMEOUT;
}

bool freshAfterStop(const Sector &s, uint32_t now) {

    return isFresh(s, now) &&
           now - s.stamp < now - stateSince;
}

bool allSectorsFresh(uint32_t now) {

    for (Sector* s : sectors) {

        if (!isFresh(*s, now)) {
            return false;
        }
    }

    return true;
}

bool allSectorsFreshAfterStop(uint32_t now) {

    for (Sector* s : sectors) {

        if (!freshAfterStop(*s, now)) {
            return false;
        }
    }

    return true;
}

int getDistance(const Sector &s) {

    int d = s.distance;

    // Also consider measurements from the ongoing scan.
    if (s.active && s.hasPoint && s.scanMin < d) {
        d = s.scanMin;
    }

    return d;
}

void resetSectors() {

    for (Sector* s : sectors) {

        s->active = false;
        s->hasPoint = false;
        s->bad = false;
        s->valid = false;

        s->scanMin = MAX_DISTANCE + 1;
        s->distance = MAX_DISTANCE + 1;
        s->lastOffset = 0;
        s->stamp = 0;
    }
}


// ==================================================
// 11. Rotation Safety
// ==================================================

bool canRotate(uint32_t now) {

    if (!allSectorsFresh(now)) {
        return false;
    }

    for (Sector* s : sectors) {

        if (getDistance(*s) <= TURN_GUARD_MM) {
            return false;
        }
    }

    // Check recently detected close obstacles,
    // including points outside the four sectors.
    if (nearSeen &&
        now - lastNearMs < SCAN_TIMEOUT) {

        return false;
    }

    return true;
}


// ==================================================
// 12. Point Processing and Emergency Stop
// ==================================================

void processPoint(
    float angle,
    int distance,
    uint32_t now
) {

    for (Sector* s : sectors) {
        updateSector(*s, angle, distance, now);
    }

    if (distance == 0 ||
        distance > MAX_DISTANCE) {
        return;
    }

    bool inFront =
        angle >= 145 && angle <= 215;

    bool inRear =
        angle >= 325 || angle <= 35;

    if (distance <= TURN_GUARD_MM) {

        nearSeen = true;
        lastNearMs = now;
    }

    // Immediate stop during forward motion.
    if (mode == Mode::FORWARD &&
        inFront &&
        distance <= STOP_MM) {

        setMode(Mode::STOPPED, now);
    }

    // Immediate stop during backward motion.
    else if (mode == Mode::BACKWARD &&
             inRear &&
             distance <= REAR_STOP_MM) {

        setMode(Mode::WAIT_TURN, now);
    }

    // Stop if a close obstacle is detected
    // during rotation.
    else if (
        (mode == Mode::TURN_LEFT ||
         mode == Mode::TURN_RIGHT) &&
        distance <= TURN_GUARD_MM
    ) {

        setMode(Mode::STOPPED, now);
    }
}


// ==================================================
// 13. LiDAR Packet Processing
// ==================================================

void processPacket(uint32_t now) {

    float start =
        readUInt16(packet[4], packet[5]) / 100.0f;

    float end =
        readUInt16(packet[42], packet[43]) / 100.0f;

    if (start >= 360 || end >= 360) {
        return;
    }

    if (end < start) {
        end += 360;
    }

    if (end <= start || end - start > 30) {
        return;
    }

    lastPacketMs = now;

    float step = (end - start) / 11.0f;

    for (int i = 0; i < 12; i++) {

        int index = 6 + 3 * i;

        int distance = readUInt16(
            packet[index],
            packet[index + 1]
        );

        float angle = start + step * i;

        if (angle >= 360) {
            angle -= 360;
        }

        processPoint(angle, distance, now);
    }
}


// ==================================================
// 14. Select Rotation Direction
// ==================================================

void selectTurn(uint32_t now) {

    int dL = getDistance(left);
    int dR = getDistance(right);

    bool turnLeft;

    // Alternate directions if the two sides
    // have approximately equal clearance.
    if (abs(dL - dR) < 100) {

        turnLeft = !lastTurnLeft;
    }
    else {

        turnLeft = dL > dR;
    }

    lastTurnLeft = turnLeft;

    if (turnAttempts < 255) {
        turnAttempts++;
    }

    if (turnLeft) {
        setMode(Mode::TURN_LEFT, now);
    }
    else {
        setMode(Mode::TURN_RIGHT, now);
    }
}


// ==================================================
// 15. Obstacle Avoidance and Corner Escape
// ==================================================

void updateMotion(uint32_t now) {

    switch (mode) {

        // ------------------------------------------
        // Forward
        // ------------------------------------------

        case Mode::FORWARD:

            if (!isFresh(front, now) ||
                getDistance(front) <= STOP_MM) {

                setMode(Mode::STOPPED, now);
            }
            else if (
                now - stateSince >= FORWARD_RESET_MS
            ) {

                turnAttempts = 0;
            }

            break;


        // ------------------------------------------
        // Stop and analyze the environment
        // ------------------------------------------

        case Mode::STOPPED: {

            // Wait for a new front scan after stopping.
            if (!freshAfterStop(front, now)) {
                break;
            }

            int dF = getDistance(front);

            // Resume forward motion if the path is clear.
            if (dF > RESUME_MM) {

                setMode(Mode::FORWARD, now);
                break;
            }

            // All directions must be updated before
            // making an escape decision.
            if (!allSectorsFreshAfterStop(now)) {
                break;
            }

            int dL = getDistance(left);
            int dR = getDistance(right);
            int dB = getDistance(rear);

            bool cornerDetected =
                dF <= RESUME_MM &&
                dL <= SIDE_CLEAR_MM &&
                dR <= SIDE_CLEAR_MM;

            bool repeatedTurn =
                turnAttempts >= MAX_TURN_ATTEMPTS;

            bool rotationSafe = canRotate(now);

            Serial.printf(
                "F:%d L:%d R:%d B:%d Attempts:%d\n",
                dF, dL, dR, dB, turnAttempts
            );

            // Corner, repeated turning, or insufficient
            // rotation clearance.
            if (cornerDetected ||
                repeatedTurn ||
                !rotationSafe) {

                if (dB > REAR_CLEAR_MM) {

                    Serial.println("ESCAPE REQUIRED");

                    setMode(Mode::BACKWARD, now);
                }
                // Otherwise remain stopped.
            }

            // Normal obstacle avoidance.
            else {

                selectTurn(now);
            }

            break;
        }


        // ------------------------------------------
        // Reverse
        // ------------------------------------------

        case Mode::BACKWARD:

            if (!isFresh(rear, now)) {

                setMode(Mode::STOPPED, now);
            }
            else if (
                getDistance(rear) <= REAR_STOP_MM ||
                now - stateSince >= REVERSE_MS
            ) {

                setMode(Mode::WAIT_TURN, now);
            }

            break;


        // ------------------------------------------
        // Re-scan after reversing
        // ------------------------------------------

        case Mode::WAIT_TURN:

            if (allSectorsFreshAfterStop(now) &&
                canRotate(now)) {

                selectTurn(now);
            }

            break;


        // ------------------------------------------
        // Rotate
        // ------------------------------------------

        case Mode::TURN_LEFT:
        case Mode::TURN_RIGHT:

            if (!canRotate(now)) {

                setMode(Mode::STOPPED, now);
            }
            else if (
                now - stateSince >= TURN_MS
            ) {

                setMode(Mode::STOPPED, now);
            }

            break;
    }
}


// ==================================================
// 16. Setup
// ==================================================

void setup() {

    for (int i = 0; i < 8; i++) {

        pinMode(motorPins[i], OUTPUT);
        digitalWrite(motorPins[i], LOW);
    }

    Serial.begin(115200);

    Serial2.begin(
        230400,
        SERIAL_8N1,
        LIDAR_RX,
        17
    );

    stateSince = millis();

    Serial.println("STOP");
}


// ==================================================
// 17. Main Loop
// ==================================================

void loop() {

    uint32_t now = millis();

    // Stop immediately if LiDAR communication fails.
    if (now - lastPacketMs >= PACKET_TIMEOUT) {

        setMode(Mode::STOPPED, now);
        resetSectors();
    }

    // LiDAR reception continues in every motion state.
    if (readPacket()) {

        processPacket(now);
    }

    // Non-blocking robot motion control.
    updateMotion(millis());
}

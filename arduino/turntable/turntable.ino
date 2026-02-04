#include <libb64.h>
// --------------------- Timer Interrupt stuff -------------------- //

/* Set Timer1 use, note, this will only work on Arduino UNO at the moment.
 * And this sketch CANNOT use the Servo Library as that would use Timer1.
 */
#define USE_TIMER_1 true
// #define TIMER1_FREQ_HZ 1000000.0
#include <TimerInterrupt.h>
// --------------------------------------------------------------//

// ------------------ Serial Comms Related -------------//
// static const char start_tx = '\x01';   // SOH (start of heading)
// static const char start_data = '\x02'; // STX (start of text)
// static const char end_data = '\x03';   // ETX (end of text)
// static const char end_tx = '\x04';     // EOT (end of transmission)
// ALTERNATE for reading
static const char start_tx = '#';   // SOH (start of heading)
static const char start_data = '$'; // STX (start of text)
static const char end_data = '%';   // ETX (end of text)
static const char end_tx = '&';     // EOT (end of transmission)

#define SERIAL_RX_BUFFER_SIZE 256
#define SERIAL_TX_BUFFER_SIZE 256
// --------------------------------------------------------------//

// -------------- State Machine Related --------------------//
enum State
{
    BYTES_AVAILABLE,
    READ_INTO_ARRAY,
    SEND_TO_DECODE,
    RESET_FOR_NEW_MSG,
    RESET_FOR_CONTINUING_MSG

};

struct StateMachine
{
    State state;
    int bytes_available = 0;
    char incoming[500];
    uint16_t index = 0;
    bool start_tx_found = false;
    int bytes_read_this_cycle = 0;
};
StateMachine sm;
// --------------------------------------------------------------//

// ------------------ Comms Message Related -------------------//
#define SERIALIZE_BUF_LEN 17
#define B64_BUF_LEN 33
#define PACKET_BUF_LEN B64_BUF_LEN * 5
struct MessageInfo
{

    int start_tx_loc = 0;
    int start_data_loc = 0;
    int end_data_loc = 0;
    int end_tx_loc = 0;
    // const int id_len = 4;
    // const int data_len = 4;
    // const int crc_len = 4;
    char id_b64[B64_BUF_LEN] = {0};
    char data_b64[B64_BUF_LEN] = {0};
    char crc[B64_BUF_LEN] = {0};
};
MessageInfo mi;

struct MessagePrototype
{
    int32_t id;
    int32_t data;
};

enum MessageTypes
{
    MoveBy = 7,
    Position = 9,
    IncomingMessageLengthError = 1,
    EStop = 4,
    MotorEnable = 5,
    Ack = 2,
    MotionComplete = 3
};

const static uint16_t queue_length = 16;
struct OutBoundMessageQueue
{
    MessagePrototype queue[queue_length];
    int16_t index = 0;
};

// union char_to_uint32{
//     uint32_t value;
//     char buffer[5];
// };

// union int8_to_int32{
//     int32_t value;
//     int8_t buffer[4];
// };

union num_to_bytes
{
    int32_t num[2];
    char bytes[4];
};

void serialize_int32(uint32_t value, char* out)
{

    out[0] = (char)( value        & 0xFF);
    out[1] = (char)((value >> 8)  & 0xFF);
    out[2] = (char)((value >> 16) & 0xFF);
    out[3] = (char)((value >> 24) & 0xFF);
    return;
}

uint32_t deserialize_int32(const char in[4])
{
    return ((uint32_t)(char)in[0])        |
           ((uint32_t)(char)in[1] << 8)   |
           ((uint32_t)(char)in[2] << 16)  |
           ((uint32_t)(char)in[3] << 24);
}

void send_message(uint32_t id, uint32_t data)
{
    // Serialize id
    char id_bytes[SERIALIZE_BUF_LEN];
    serialize_int32(id, id_bytes);

    // Serialize data
    char data_bytes[SERIALIZE_BUF_LEN];
    serialize_int32(data, data_bytes);

    // uint32_t deserial_data = deserialize_int32(data_bytes);
    //  Serial.print(" id dec: ");
    //  Serial.print(id);
    //  Serial.print(" | id bytes: ");
    //  for(int i=0; i<4; i++){Serial.print(id_bytes[i], DEC); Serial.print(" ");}
    //  Serial.print(" | data dec: ");
    //  Serial.print((int32_t)data);
    //  Serial.print(" | data bytes: ");
    //  for(int i=0; i<4; i++){Serial.print(data_bytes[i], DEC); Serial.print(" ");}
    //  Serial.println();

    // b64 encode id
    char id_b64[B64_BUF_LEN] = {0};
    encode(id_bytes, id_b64, 4);

    // b64 encode data
    char data_b64[B64_BUF_LEN] = {0};
    encode(data_bytes, data_b64, 4);

    // Print debug
    //  Serial.print(" id dec: ");
    //  Serial.print(id);
    //  Serial.print(" | id b64: ");
    //  for(int i=0; i<strlen(id_b64); i++){Serial.print(id_b64[i]);}
    //  Serial.print(" | data dec: ");
    //  Serial.print((int32_t)data);
    //  Serial.print(" | data b64: ");
    //  for(int i=0; i<strlen(data_b64); i++){Serial.print(data_b64[i]);}
    //  Serial.println();

    // Packetize info
    char packet_out[PACKET_BUF_LEN];
    char *pack_ptr = packet_out;

    // start tx
    *pack_ptr = start_tx;
    pack_ptr++;
    // id
    for (int i = 0; i < strlen(id_b64); i++)
    {
        *pack_ptr = id_b64[i];
        pack_ptr++;
    }
    // start data
    *pack_ptr = start_data;
    pack_ptr++;
    // data
    for (int i = 0; i < strlen(data_b64); i++)
    {
        *pack_ptr = data_b64[i];
        pack_ptr++;
    }
    // end data
    *pack_ptr = end_data;
    pack_ptr++;
    // data as CRC
    for (int i = 0; i < strlen(data_b64); i++)
    {
        *pack_ptr = data_b64[i];
        pack_ptr++;
    }
    // end tx
    *pack_ptr = end_tx;
    pack_ptr++;
    // null terminate
    *pack_ptr = '\0';
    pack_ptr++;
    size_t num_bytes = pack_ptr - packet_out;

    // Write to serial port
    for (int i = 0; i < num_bytes; i++)
    {
        Serial.write(packet_out[i]);
    }
    Serial.flush();
}

MessagePrototype recv_message(char *input)
{
    MessageInfo mi;
    // char input[] = "#AAAACQ==$AAAACg==%1bb7&\0";
    for (int i = 0; i <= strlen(input); i++)
    {
        if (input[i] == start_tx)
        {
            mi.start_tx_loc = i;
        }
        if (input[i] == start_data)
        {
            mi.start_data_loc = i;
        }
        if (input[i] == end_data)
        {
            mi.end_data_loc = i;
        }
        if (input[i] == end_tx)
        {
            mi.end_tx_loc = i;
        }
    }

    // Get bytes between start_tx and start_data for msg id
    int id_index = 0;
    for (int i = mi.start_tx_loc + 1; i < mi.start_data_loc; i++)
    {
        // Serial.print(input[i]);
        mi.id_b64[id_index] = input[i];
        id_index++;
    }

    // // Get bytes between start_data and end_data for msg data
    int data_index = 0;
    for (int i = mi.start_data_loc + 1; i < mi.end_data_loc; i++)
    {
        // Serial.print(input[i]);
        mi.data_b64[data_index] = input[i];
        data_index++;
    }

    char id_bytes[SERIALIZE_BUF_LEN] = {0};
    char data_bytes[SERIALIZE_BUF_LEN] = {0};
    decode(mi.id_b64, id_bytes);
    decode(mi.data_b64, data_bytes);
    // for(int i=0; i<SERIALIZE_BUF_LEN; i++)
    // {
    //     Serial.print(id_bytes[i], BIN);
    // }
    int32_t id = deserialize_int32(id_bytes);
    int32_t data = deserialize_int32(data_bytes);

    // Serial.print(" id: ");
    // Serial.print(id);
    // Serial.print(" | data: ");
    // Serial.print(data);
    // Serial.println();

    MessagePrototype mp;
    mp.id = id;
    mp.data = data;

    return mp;
}
// --------------------------------------------------------------//

// --------------- Motor control information ---------------- //
struct MotorConfig
{
    uint8_t step_pin = 5;
    uint8_t dir_pin = 6;
    uint8_t ena_pin = 7;
    uint32_t steps_per_degree = 10;   // 10 steps per degree of rotation of the turntable
    unsigned long step_interval = 20; // ms
};

MotorConfig motor_config;

struct MotorControlState
{
    volatile bool enabled = false;
    volatile int32_t current_steps = 0; // in steps
    volatile int32_t current_deg = 0;   // in degrees
    volatile int32_t desired_deg = 0;   // in degrees
    volatile bool estop = false;
};

volatile MotorControlState motor_state;

void motor_setup()
{
    pinMode(motor_config.step_pin, OUTPUT);
    pinMode(motor_config.dir_pin, OUTPUT);
}

void cycle_motor_control(); // Forward declaration
// ---------------------------------------------------------- //

// ----------------------- Standard Arduino Stuff ----------------//
void setup()
{
    Serial.begin(115200, SERIAL_8N1);
    motor_setup();

    sm.state = BYTES_AVAILABLE;
    sm.index = 0;

    ITimer1.init();
    if (ITimer1.attachInterruptInterval(motor_config.step_interval, cycle_motor_control))
    {
        Serial.print(F("Starting  ITimer1 OK, millis() = "));
        Serial.println(millis());
    }
    else
        Serial.println(F("Can't set ITimer1. Select another freq. or timer"));
}

void loop()
{
    cycle_comms_state_machine();
    // delay(10);
    // Serial.println("LOOP")
    // send_message(Ack, 256);
    // char in[B64_BUF_LEN];
    // recv_message(in);
    // delay(100);
}

// ---------------------------------------------------------- //

void cycle_motor_control()
{
    // Get motor error, (desired - current)
    /* Note that we do control in the steps domain, so desired deg is first converted
     * into steps using the steps_per_degree config parameter.
     * Then an error_steps is calculated and used to determine which way to step the motor
     * The enable pin is set before any control, and control only happens if motor is enabled
     */
    int32_t desired_steps = motor_state.desired_deg * motor_config.steps_per_degree;
    int32_t error_steps = desired_steps - motor_state.current_steps;

    if (motor_state.enabled)
    {
        // Enable Motor
        digitalWrite(motor_config.ena_pin, HIGH);

        if (error_steps > 0)
        {
            // Set dir pin
            digitalWrite(motor_config.dir_pin, LOW);
            delayMicroseconds(100);
            // Step one step
            digitalWrite(motor_config.step_pin, LOW);
            delayMicroseconds(100);
            digitalWrite(motor_config.step_pin, HIGH);
            delayMicroseconds(100);
            motor_state.current_steps++;
        }
        else if (error_steps < 0)
        {
            digitalWrite(motor_config.dir_pin, HIGH);
            delayMicroseconds(100);
            // Step one step
            digitalWrite(motor_config.step_pin, LOW);
            delayMicroseconds(100);
            digitalWrite(motor_config.step_pin, HIGH);
            delayMicroseconds(100);
            motor_state.current_steps--;
        }
        else
        {
            return;
        }
    }
    else if (!motor_state.enabled)
    {
        // Disable Motor
        digitalWrite(motor_config.ena_pin, LOW);
    }
}

void cycle_comms_state_machine()
{
    switch (sm.state)
    {
    case BYTES_AVAILABLE:

        sm.bytes_available = Serial.available();
        // Serial.write("BYTES AVAIL: ");
        // Serial.write(sm.bytes_available);
        // Serial.flush();
        if (sm.bytes_available == 0)
        {
            break;
        }
        sm.state = READ_INTO_ARRAY;
        break;

    case READ_INTO_ARRAY:
        // Serial.write("READ INTO ARRAY\n");
        // Serial.flush();
        // Read # of bytes available into incoming char array, increment as needed
        for (int i = 0; i < sm.bytes_available; i++)
        {
            sm.incoming[sm.index] = Serial.read();
            // Serial.write(sm.incoming[sm.index]);
            // Serial.flush();

            // Found end of message, decode
            if (sm.incoming[sm.index] == end_tx)
            {
                sm.index++;
                sm.bytes_read_this_cycle++;

                sm.state = SEND_TO_DECODE;
                // Serial.write("FOUND END TX");
                // Serial.flush();
                return;
            }

            sm.index++;
            sm.bytes_read_this_cycle++;
        }

        if (sm.bytes_read_this_cycle == sm.bytes_available)
        {
            // Serial.write("Read all bytes in buffer at that moment");
            // Serial.flush();
            sm.state = RESET_FOR_CONTINUING_MSG;
            return;
        }
        return;

    case SEND_TO_DECODE:
    {
        MessagePrototype mp;
        mp = recv_message(sm.incoming);

        // Check message type and act accordingly
        switch (mp.id)
        {
        case MoveBy:
            // Check if motor is enabled, if not, ignore command and send error message back
            if (motor_state.enabled == false)
            {
                // Write ACK with fail
                send_message(Ack, 0);
                break;
            }
            if (motor_state.enabled == true)
            {
                // Update desired position
                motor_state.desired_deg = motor_state.desired_deg + mp.data;
                // Write ACK with success
                send_message(Ack, 1);
            }

            break;

        case Position:
        {
            // Write ACK with success
            send_message(Ack, 1);
            // Send motor position (as steps for now) TODO: Change to deg, update deg in motor control cycle
            // TODO: Fix negative values for base64 encoding issue
            ITimer1.disableTimer();
            int32_t* pos = malloc(sizeof(int32_t));
            *pos = motor_state.current_steps;
            send_message(Position, *pos);
            free(pos);
            ITimer1.enableTimer();
            break;
        }
        case EStop:
            // Write ACK with success
            send_message(Ack, 1);
            motor_state.estop = true;
            break;

        case MotorEnable:
            if (mp.data == 1)
            {
                motor_state.enabled = true;
            }
            else
            {
                motor_state.enabled = false;
            }

            // Write ACK with success
            send_message(Ack, 1);
            break;

        case Ack:
            // Write ACK with success
            send_message(Ack, 1);
            break;
        }

        sm.state = RESET_FOR_NEW_MSG;
    }
    break;

    case RESET_FOR_NEW_MSG:
        // Serial.write("RESET FOR NEW MSG");
        sm.index = 0;
        sm.bytes_available = 0;
        sm.bytes_read_this_cycle = 0;
        // Clear incoming message char array to all NULL so when populated makes a valid cstr
        memset(sm.incoming, byte('\0'), sizeof(sm.incoming) * sizeof(sm.incoming[0]));
        // Reset packet control characters
        mi.start_tx_loc = 0;
        mi.start_data_loc = 0;
        mi.end_data_loc = 0;
        mi.end_tx_loc = 0;
        sm.state = BYTES_AVAILABLE;
        break;

    case RESET_FOR_CONTINUING_MSG:
        // Serial.write("RESET CONTINUE MSG");
        // Serial.flush();
        // Don't reset index, as we need to continue appending incoming chars to get full msg
        sm.bytes_read_this_cycle = 0;
        sm.state = BYTES_AVAILABLE;
        return;

    default:
        break;
    }
}
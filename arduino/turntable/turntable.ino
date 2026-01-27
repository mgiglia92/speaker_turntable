// --------------------- Timer Interrupt stuff -------------------- //

/* Set Timer1 use, note, this will only work on Arduino UNO at the moment. 
 * And this sketch CANNOT use the Servo Library as that would use Timer1.
 */
#define USE_TIMER_1 true
// #define TIMER1_FREQ_HZ 1000000.0
#include <TimerInterrupt.h>
// --------------------------------------------------------------//

// ------------------ Serial Comms Related -------------//
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
struct MessageInfo
{

    int start_tx_loc = 0;
    int start_data_loc = 0;
    int end_data_loc = 0;
    int end_tx_loc = 0;
    char id[4];
    char data[4];
    char crc[4];

};
MessageInfo mi;

struct MessagePrototype
{
    uint32_t id;
    int32_t data;
};

enum MessageTypes
{
    MoveBy                      = 7,
    Position                    = 9,
    IncomingMessageLengthError  = 1,
    EStop                       = 4,
    MotorEnable                 = 5,
    Ack                         = 2,
    MotionComplete              = 3
};

const static uint16_t queue_length = 16;
struct OutBoundMessageQueue
{
    MessagePrototype queue[queue_length];
    int16_t index=0;
};


union uint8_to_uint32{
    uint32_t value;
    char buffer[4];
};

union int8_to_int32{
    int32_t value;
    int8_t buffer[4];
};


void send_message(uint32_t id, int32_t data)
{
    // Place id into union
    uint8_to_uint32 id_union;
    id_union.value = id;

    // Place data into union
    int8_to_int32 data_union;
    data_union.value = data;

    // buffer byte array
    const size_t out_buf_size = 16;
    char out_buf[out_buf_size];

    // Set values in buffer
    out_buf[0] = start_tx;
    // id
    out_buf[1] = id_union.buffer[0];
    out_buf[2] = id_union.buffer[1];
    out_buf[3] = id_union.buffer[2];
    out_buf[4] = id_union.buffer[3];
    
    out_buf[5] = start_data;
    // data
    out_buf[6] = data_union.buffer[0];
    out_buf[7] = data_union.buffer[1];
    out_buf[8] = data_union.buffer[2];
    out_buf[9] = data_union.buffer[3];

    out_buf[10] = end_data;
    // data again as crc (proper crc not implemented yet)
    out_buf[11] = data_union.buffer[0];
    out_buf[12] = data_union.buffer[1];
    out_buf[13] = data_union.buffer[2];
    out_buf[14] = data_union.buffer[3];
    
    out_buf[15] = end_tx;

    // Print the out buffer to the serial port
    for(int i=0; i<out_buf_size; i++)
    {
        Serial.write(out_buf[i]);
    }
    Serial.flush();

}
// --------------------------------------------------------------//

// --------------- Motor control information ---------------- //
struct MotorConfig
{
    uint8_t step_pin = 5;
    uint8_t dir_pin = 6;
    uint8_t ena_pin = 7;
    uint32_t steps_per_degree = 10; // 10 steps per degree of rotation of the turntable
    unsigned long step_interval = 20; // ms
};

MotorConfig motor_config;

struct MotorControlState
{
    volatile bool enabled = false;
    volatile int32_t current_steps = 0; // in steps
    volatile int32_t current_deg = 0; // in degrees
    volatile int32_t desired_deg = 0; // in degrees
    volatile bool estop = false;
};

volatile MotorControlState motor_state;

void motor_setup()
{
    pinMode(motor_config.step_pin, OUTPUT);
    pinMode(motor_config.dir_pin, OUTPUT);
}

void cycle_motor_control(); //Forward declaration
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
        Serial.print(F("Starting  ITimer1 OK, millis() = ")); Serial.println(millis());
    }
    else
        Serial.println(F("Can't set ITimer1. Select another freq. or timer"));
}


void loop()
{
    cycle_comms_state_machine();
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
    int32_t error_steps = desired_steps - motor_state.current_steps ;
    
    if(motor_state.enabled)
    {
        // Enable Motor
        digitalWrite(motor_config.ena_pin, HIGH);

        if(error_steps > 0)
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
        else if(error_steps < 0)
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
        else { return; }
    }
    else if(!motor_state.enabled)
    {
        // Disable Motor
        digitalWrite(motor_config.ena_pin, LOW);
    }
}

extern "C"{
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
            // Serial.write("SEND TO DECODE");
            // Serial.flush();
            // // // Do decoding
            // // Serial.write(sm.incoming, strlen(sm.incoming));
            // // Serial.flush();
            // // // Serial.write("IN DECODING");
            // for (int i = 0; i <= 16; i++)
            // {
            //     Serial.write(sm.incoming[i]);
            // }

            // Get locations of control chars
            for (int i = 0; i <= 16; i++)
            {
                if (sm.incoming[i] == start_tx)     {mi.start_tx_loc    = i;}
                if (sm.incoming[i] == start_data)   {mi.start_data_loc  = i;}
                if (sm.incoming[i] == end_data)     {mi.end_data_loc    = i;}
                if (sm.incoming[i] == end_tx)       {mi.end_tx_loc      = i;}
            }

            // Get bytes between start_tx and start_data for msg id
            int id_index=0;
            for(int i = mi.start_tx_loc + 1; i <= mi.start_data_loc; i++)
            {
                mi.id[id_index] = sm.incoming[i];
                id_index++;
            }

            // // Get bytes between start_data and end_data for msg data
            int data_index=0;
            for(int i=mi.start_data_loc + 1; i <= mi.end_data_loc; i++)
            {
                mi.data[data_index] = sm.incoming[i];
                data_index++;
            }

            sm.state = RESET_FOR_NEW_MSG;

            //Decoding now
            //Get 4 bytes for id, push into uint32_t
            uint8_to_uint32 id_union;
            for(int i =0; i <4; i++)
            {
                id_union.buffer[i]=mi.id[i];
            }
            // Serial.print("Union for id as int32: ");
            // Serial.println(id_union.value);

            //get 4 bytes for data, push into int32_t (notice signed not unsigned)
            int8_to_int32 data_union;
            for(int i =0; i <4; i++)
            {
                data_union.buffer[i]=mi.data[i];
            }

            //get 4 bytes for checksum, push into int32_t
            int8_to_int32 crc_union;
            for(int i =0; i <4; i++)
            {
                crc_union.buffer[i]=mi.data[i];
            }

            // Verify checksum (not implemented yet)

            // Check message type and act accordingly
            switch (id_union.value)
            {
                case MoveBy:
                    // Check if motor is enabled, if not, ignore command and send error message back
                    if(motor_state.enabled == false)
                    {
                        // Write ACK with fail
                        send_message(Ack, 0);
                        break;
                    }
                    if(motor_state.enabled == true)
                    {
                        // Update desired position
                        motor_state.desired_deg = motor_state.desired_deg + data_union.value;
                        // Write ACK with success
                        send_message(Ack, 1);
                    }



                    break;
                
                case Position:
                    // Write ACK with success
                    send_message(Ack, 1);
                    // Send motor position (as steps for now) TODO: Change to deg, update deg in motor control cycle
                    send_message(Position, motor_state.current_steps);
                    break;

                case EStop:
                    // Write ACK with success
                    send_message(Ack, 1);
                    motor_state.estop = true;
                    break;

                case MotorEnable:
                    if(data_union.value == 1)
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
        }
        break;

    case RESET_FOR_NEW_MSG:
        // Serial.write("RESET FOR NEW MSG");
        sm.index = 0;
        sm.bytes_available = 0;
        sm.bytes_read_this_cycle = 0;
        // Clear incoming message char array to all NULL so when populated makes a valid cstr
        memset(sm.incoming, byte('\0'), sizeof(sm.incoming) * sizeof(sm.incoming[0]));
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
}
#include <base64.h>

static const char start_tx = '#';   // SOH (start of heading)
static const char start_data = '$'; // STX (start of text)
static const char end_data = '%';   // ETX (end of text)
static const char end_tx = '&';     // EOT (end of transmission)

#define SERIAL_RX_BUFFER_SIZE 256
#define SERIAL_TX_BUFFER_SIZE 256

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

union uint8_to_uint32{
    uint32_t value;
    char buffer[4];
};

union int8_to_int32{
    int32_t value;
    int8_t buffer[4];
};


MessageInfo mi;
StateMachine sm;

void setup()
{
    pinMode(13, OUTPUT);
    Serial.begin(115200, SERIAL_8N1);
    sm.state = BYTES_AVAILABLE;
    sm.index = 0;
}

void loop()
{
    // while(Serial.available())
    // {
    //     Serial.write(Serial.read());
    //     Serial.flush();
    // }    
    // digitalWrite(13, !digitalRead(13));
    // Serial.write("LOOP\n");
    // Serial.println(sm.state);
    // Serial.flush();
    loop_comms_state_machine();

    // delay(100);
    // Serial.println("hello world");
    // Serial.write('\x04');
    // Serial.write('\x04');
    // Serial.write('\x04');
    // Serial.write('\x04');
    // Serial.write('\x04');
    // Serial.write('\x04');
    // Serial.write('\x04');
}

extern "C"{
void loop_comms_state_machine()
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
            for (int i = 0; i <= 16; i++)
            {
                Serial.write(sm.incoming[i]);
            }

            // Get locations of control chars
            for (int i = 0; i <= 16; i++)
            {
                if (sm.incoming[i] == start_tx)
                {
                    mi.start_tx_loc = i;
                    // Serial.print("FOUND TX LOC\n strlen: ");
                    Serial.println(strlen(sm.incoming));
                }
                if (sm.incoming[i] == start_data)
                {
                    
                    // Serial.print("FOUND DATA START LOC");
                    mi.start_data_loc = i;
                }
                if (sm.incoming[i] == end_data)
                {
                    
                    // Serial.print("FOUND data end LOC");
                    mi.end_data_loc = i;
                }
                if (sm.incoming[i] == end_tx)
                {

                    // Serial.print("FOUND end/ LOC");
                    mi.end_tx_loc = i;
                }
            }
            // // Get bytes between start_tx and start_data for msg id
            
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

            Serial.print("ID: ");
            Serial.write(mi.id, 4);
            Serial.print(" | DATA: ");
            Serial.write(mi.data, 4);
            Serial.println();
            sm.state = RESET_FOR_NEW_MSG;

            //Decoding now
            //Get 4 bytes for id, push into uint32_t
            uint8_to_uint32 id_union;
            for(int i =0; i <4; i++)
            {
                id_union.buffer[i]=mi.id[i];
            }
            Serial.print("Union for id as int32: ");
            Serial.println(id_union.value);

            //get 4 bytes for data, push into int32_t (notice signed not unsigned)
            int8_to_int32 data_union;
            for(int i =0; i <4; i++)
            {
                data_union.buffer[i]=mi.data[i];
            }
            Serial.print("Union for data as int32: ");
            Serial.println(data_union.value);

            //get 4 bytes for checksum, push into int32_t
            int8_to_int32 crc_union;
            for(int i =0; i <4; i++)
            {
                crc_union.buffer[i]=mi.data[i];
            }
            Serial.print("Union for crc as int32: ");
            Serial.println(crc_union.value);

            // Verify checksum (not implemented yet)

            // Check message type and act accordingly
        }
        break;

    case RESET_FOR_NEW_MSG:
        // Serial.write("RESET FOR NEW MSG");
        sm.index = 0;
        sm.bytes_available = 0;
        sm.bytes_read_this_cycle = 0;
        // sm.incoming[0] = '\0';
        memset(sm.incoming, byte('\0'), sizeof(sm.incoming) * sizeof(sm.incoming[0]));
        sm.state = BYTES_AVAILABLE;
        break;

    case RESET_FOR_CONTINUING_MSG:
        // Serial.write("RESET CONTINUE MSG");
        // Serial.flush();
        sm.bytes_read_this_cycle = 0;
        sm.state = BYTES_AVAILABLE;
        return;

    default:
        break;
    }
}
}
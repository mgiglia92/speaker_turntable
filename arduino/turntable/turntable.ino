
static const char start_tx = '\x01';   // SOH (start of heading)
static const char start_data = '\x02'; // STX (start of text)
static const char end_data = '\x03';   // ETX (end of text)
static const char end_tx = '\x04';     // EOT (end of transmission)

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
            Serial.write(sm.incoming[sm.index]);
            // Serial.flush();
            sm.index++;
            sm.bytes_read_this_cycle++;
            // Found end of message, decode
            if (sm.incoming[sm.index] == end_tx)
            {
                sm.state = SEND_TO_DECODE;
                // Serial.write("FOUND END TX");
                return;
            }

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

        // Serial.write("SEND TO DECODE");
        // // Do decoding
        // Serial.write(sm.incoming, strlen(sm.incoming));
        // Serial.flush();
        // // Serial.write("IN DECODING");
        // int start_tx_loc = 0;
        // int start_data_loc = 0;
        // int end_data_loc = 0;
        // int end_tx_loc = 0;
        // // Get locations of control chars
        // for (int i = 0; i < strlen(sm.incoming); i++)
        // {
        //     if (sm.incoming[i] == start_tx)
        //     {
        //         start_tx_loc = i;
        //     }
        //     if (sm.incoming[i] == start_data)
        //     {
        //         start_data_loc = i;
        //     }
        //     if (sm.incoming[i] == end_data)
        //     {
        //         end_data_loc = i;
        //     }
        //     if (sm.incoming[i] == end_tx)
        //     {
        //         end_tx_loc = i;
        //     }
        // }

        // Serial.write(start_tx_loc);
        // // Get bytes between start_tx and start_data for msg id
        // byte id[10];
        // int id_index=0;
        // for(int i=start_tx_loc; i<start_data_loc; i++)
        // {
        //     id[id_index] = sm.incoming[i];
        //     id_index++;
        // }

        // // Get bytes between start_data and end_data for msg data
        // byte data[32];
        // int data_index=0;
        // for(int i=start_data_loc; i<end_data_loc; i++)
        // {
        //     data[data_index] = sm.incoming[i];
        //     data_index++;
        // }

        // Serial.print('ID: ');
        // Serial.write(id, strlen(id));
        // Serial.print(" | DATA: ");
        // Serial.write(data, strlen(data));

        sm.state = RESET_FOR_NEW_MSG;
        return;



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
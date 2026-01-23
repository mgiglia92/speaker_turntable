
static const char start_tx   = '\x01'; // SOH (start of heading)
static const char start_data = '\x02'; // STX (start of text)
static const char end_data   = '\x03'; // ETX (end of text)
static const char end_tx     = '\x04'; // EOT (end of transmission)

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
    int bytes_available=0;
    char incoming[100];
    int index=0;
    bool start_tx_found=false;
    int bytes_read_this_cycle=0;
};

StateMachine sm;

void setup()
{
    Serial.begin(115200);
    sm.state=BYTES_AVAILABLE;
    sm.index = 0;
}

void loop()
{
    loop_comms_state_machine();
    // Serial.println("hello world");
    delay(100);
}


void loop_comms_state_machine()
{
    switch(sm.state)
    {
        case BYTES_AVAILABLE:
            Serial.println("BYTES AVAIL");
            if(Serial.available()==0)
            { 
                break;
            }
            sm.bytes_available = Serial.available();     
            sm.state = READ_INTO_ARRAY;
            break;

        case READ_INTO_ARRAY:
            Serial.println("READ INTO ARRAY");
            // Read # of bytes available into incoming char array, increment as needed
            for (int i = 0; i<sm.bytes_available; i++)
            {
                sm.incoming[sm.index] = Serial.read();
                Serial.write(sm.incoming[sm.index]);
                sm.index++;
                sm.bytes_read_this_cycle++;
            }
            // Found end of message, decode
            if (sm.incoming[sm.index-1] == end_tx)
            {
                sm.state = SEND_TO_DECODE;
                break;
            }

            // All bytes read, end of msg not found
            if (sm.bytes_read_this_cycle == sm.bytes_available)
            {
                sm.state = RESET_FOR_CONTINUING_MSG;
                break;
            }

            break;

        case SEND_TO_DECODE:
        
            Serial.println("SEND TO DECODE");
            // Do decoding
            Serial.print("IN DECODING");
            sm.state = RESET_FOR_NEW_MSG;
            break;

        case RESET_FOR_CONTINUING_MSG:
            Serial.println("RESET CONTINUE MSG");
            sm.bytes_read_this_cycle=0;
            sm.state=BYTES_AVAILABLE;
            break;
        
        case RESET_FOR_NEW_MSG:
            Serial.println("RESET FOR NEW MSG");
            sm.index=0;
            sm.bytes_available=0;
            sm.bytes_read_this_cycle=0;
            sm.incoming[0] = '\0';
            sm.state=BYTES_AVAILABLE;
            break;

        default:
            break;
    }
}
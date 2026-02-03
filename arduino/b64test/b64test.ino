#include <libb64.h>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>

/* arbitrary buffer size */
#define SIZE 100

char* encode(const char* input, char* output)
{
  /* set up a destination buffer large enough to hold the encoded data */
//  char* output = (char*)malloc(SIZE);
  /* keep track of our encoded position */
  char* c = output;
  /* store the number of bytes encoded by a single call */
  int cnt = 0;
  /* we need an encoder state */
  base64_encodestate s;
  
  /*---------- START ENCODING ----------*/
  /* initialise the encoder state */
  base64_init_encodestate(&s);
  /* gather data from the input and send it to the output */
  cnt = base64_encode_block(input, strlen(input), c, &s);
  c += cnt;
  /* since we have encoded the entire input string, we know that 
     there is no more input data; finalise the encoding */
  cnt = base64_encode_blockend(c, &s);
  c += cnt;
  /*---------- STOP ENCODING  ----------*/
  
  /* we want to print the encoded data, so null-terminate it: */
  *c = 0;
  
  return output;
}

char* decode(const char* input, char* output)
{
  /* set up a destination buffer large enough to hold the encoded data */
//  char* output = (char*)malloc(SIZE);
  /* keep track of our decoded position */
  char* c = output;
  /* store the number of bytes decoded by a single call */
  int cnt = 0;
  /* we need a decoder state */
  base64_decodestate s;
  
  /*---------- START DECODING ----------*/
  /* initialise the decoder state */
  base64_init_decodestate(&s);
  /* decode the input data */
  cnt = base64_decode_block(input, strlen(input), c, &s);
  c += cnt;
  /* note: there is no base64_decode_blockend! */
  /*---------- STOP DECODING  ----------*/
  
  /* we want to print the decoded data, so null-terminate it: */
  *c = 0;
  
  return output;
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("RESTART");
}

union num_to_bytes{
  int32_t num;
  char bytes[16];
};

void loop() {
  const char* input = "AAAACQ==\0";
  char* encoded;
  char out_bytes[32]={0};
  char* decoded;
  
  /* encode the data */
//  encoded = encode(input);
//  printf("encoded: %s", encoded); /* encoded data has a trailing newline */

  /* decode the data */
  decoded = decode(input, out_bytes);
  int32_t data=0;
  for(int i = 0; i<4; i++)
  {
      Serial.print(out_bytes[i], HEX);
      data |= (int32_t)((int8_t)out_bytes[i] << (24 - i*8));
  }
  Serial.print(" | data: ");
  Serial.println(data);
  /* compare the original and decoded data */
//  Serial.println(strcmp(input, decoded) == 0);
//  Serial.println(getPSTR("Old way to force String to Flash"));
//  Serial.println(F("New way to force String to Flash"));
//  Serial.print(F("Free RAM = "));
//  Serial.println(freeMemory());
//  free(encoded);
//  free(decoded);
  return 0;
}

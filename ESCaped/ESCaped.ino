#include <mcp_can.h>
#include <mcp_can_dfs.h>
#include <SPI.h>
#include <time.h>


//CAN-CH bus ID definitions
#define TCESC_CONTROL       0x384
#define SUSPENSION_CONTROL  0x1FC

//CAN-CH bus data definitions
#define SUSP_SOFT           0x50
#define SUSP_MID            0x40
#define SUSP_FIRM           0x60

//DNA Mode definitions
#define DNA_RACE                  0x31
#define DNA_DYNAMIC               0x9
#define DNA_NEUTRAL               0x1
#define DNA_ADVANCED_EFFICENCY_QV 0x11
#define DNA_ADVANCED_EFFICENCY_TB 0x17

#define CAN_5KBPS    1
#define CAN_10KBPS   2
#define CAN_20KBPS   3
#define CAN_25KBPS   4
#define CAN_31K25BPS 5
#define CAN_33KBPS   6
#define CAN_40KBPS   7
#define CAN_50KBPS   8
#define CAN_80KBPS   9
#define CAN_83K3BPS  10
#define CAN_95KBPS   11
#define CAN_100KBPS  12
#define CAN_125KBPS  13
#define CAN_200KBPS  14
#define CAN_250KBPS  15
#define CAN_500KBPS  16
#define CAN_666kbps  17
#define CAN_1000KBPS 18

#define SPI_CS_CAN  9
MCP_CAN CAN(SPI_CS_CAN);     

byte len = 0;
byte buf[8] = {0};
byte tcesc_buf[8] = {0};

byte last_dna_mode = 0;
byte left_stalk_count = 0;
byte tc_disable = 0;

void print_can_buf(int id, unsigned char len) {
    Serial.print(id, HEX);
    Serial.print(",");
    Serial.print(len);
    Serial.print(",");
    for(int i = 0; i < len; i++) {
      Serial.print(buf[i], HEX);
      Serial.print(",");
    }
    Serial.println();
}

void setup()
{
  Serial.begin(115200);
  int i = 0;
  while (CAN_OK != CAN.begin(CAN_500KBPS))
  {
    Serial.println("CAN BUS Shield init fail");
    Serial.println(i++);
  }
  Serial.println("CAN BUS Shield init ok!");
  CAN.init_Mask(0, 0, 0x7ff);//did you add the delay(100) to the library per the readme?
  CAN.init_Mask(1, 0, 0x7ff);//did you RTFM?

  CAN.init_Filt(0, 0, TCESC_CONTROL);
  CAN.init_Filt(1, 0, SUSPENSION_CONTROL);
}

void handle_tcesc_control() {
  if(tcesc_buf[1] != DNA_NEUTRAL && tcesc_buf[1] != DNA_DYNAMIC && tcesc_buf[1] != DNA_ADVANCED_EFFICENCY_TB && tcesc_buf[1] != DNA_RACE)
    return;//sanity check, don't want to send random shit to module if buffer corrupt/incorrect
  else if (tcesc_buf[1] == DNA_DYNAMIC) {
    Serial.println("SEND MESSAGE TO TCESC control!");
    CAN.sendMsgBuf(TCESC_CONTROL,0, 8, tcesc_buf);
  }
}


void loop()
{
  unsigned long id = 0;
  if (CAN.checkReceive() == CAN_MSGAVAIL) {//consider switch to interrupt mode
                                          
    CAN.readMsgBufID(&id, &len, buf);
    Serial.println("Got id from buffer: ");
    Serial.println(id);
    
    
    if(id == TCESC_CONTROL) {
    // if (id == DNA_DYNAMIC) {
      memcpy(tcesc_buf, buf, 8);

      Serial.println("print buffer 1: ");
      Serial.println(tcesc_buf[1]);
      
      Serial.println("print buffer 3: ");
      Serial.println(tcesc_buf[3]);

      // TODO: need to use another switch for example paddles!
      //if(tcesc_buf[3] & 0x40) {//driver pressing left stalk button
      if(tcesc_buf[3] == 8) {
        left_stalk_count++;
        if(left_stalk_count > 8) {
          tc_disable ^= 1; //toggle tc_disable between 0 and 1
          left_stalk_count = 0;//reset stalk count for next press event
          Serial.println("tc_disable: ");
          Serial.println(tc_disable);
        }
      }
      else
        left_stalk_count = 0;
      /*
       * let's detect real DNA mode change to/from race for QV cars and change to expected TC/ESC setting when that happens
       */
      if (last_dna_mode == DNA_DYNAMIC && tcesc_buf[1] != DNA_DYNAMIC) {
        Serial.println("TCESC enabled");
        tc_disable = 0;
      } else if (last_dna_mode != DNA_DYNAMIC && tcesc_buf[1] == DNA_DYNAMIC) {
        Serial.println("TCESC disabled");
        tc_disable = 1;
      }

      Serial.println("Will set last DNA Mode: ");
      Serial.println(tcesc_buf[1]);
      last_dna_mode = tcesc_buf[1];
    }
    
    //if (id == TCESC_CONTROL || id == SUSPENSION_CONTROL) {
    if (id == TCESC_CONTROL) {
      handle_tcesc_control();//added extra check to deal with library/filter bug, don't spam the bus
    }
  }
}

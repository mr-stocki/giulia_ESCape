#include <mcp_can.h>
#include <mcp_can_dfs.h>
#include <SPI.h>
#include <time.h>


//CAN-CH bus ID definitions
#define TCESC_CONTROL       0x384 // 900
#define SUSPENSION_CONTROL  0x1FC // 508

//CAN-CH bus data definitions
#define SUSP_SOFT           0x50
#define SUSP_MID            0x40
#define SUSP_FIRM           0x60

//DNA Mode definitions
#define DNA_RACE                  0x31 // 49
#define DNA_DYNAMIC               0x9 // 9
#define DNA_NEUTRAL               0x1 // 1
#define DNA_ADVANCED_EFFICENCY    0x11 // 17

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
  if(tcesc_buf[1] != DNA_NEUTRAL && 
      tcesc_buf[1] != DNA_DYNAMIC && 
      tcesc_buf[1] != DNA_ADVANCED_EFFICENCY && 
      tcesc_buf[1] != DNA_RACE) {
    Serial.println("RETURN - DO NTH.");
    Serial.println(tcesc_buf[1]);
    return;
  } else if (tcesc_buf[1] == DNA_DYNAMIC) {
    Serial.println("buffer equals DNA_DYNAMIC, set to DNA_RACE");
    tcesc_buf[1] = DNA_RACE;
  }

  Serial.println("SEND MESSAGE TO TCESC control!");
  Serial.println(tcesc_buf[1]);
  CAN.sendMsgBuf(TCESC_CONTROL,0, 8, tcesc_buf);
}


void loop()
{
  unsigned long id = 0;
  if (CAN.checkReceive() == CAN_MSGAVAIL) {//consider switch to interrupt mode
                                          
    CAN.readMsgBufID(&id, &len, buf);
    Serial.println("Got id from buffer: ");
    Serial.println(id);
    
    if(id == TCESC_CONTROL) {
      Serial.println("ID seems to be TCESC_CONTROL");
      memcpy(tcesc_buf, buf, 8);
      Serial.println("print buffer 1: ");   
      Serial.println(tcesc_buf[1]);   
           
      handle_tcesc_control();
    }
  }
}

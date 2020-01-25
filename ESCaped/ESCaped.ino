#include <df_can.h>
#include <df_candfs.h>
#include <time.h>

//CAN-CH bus ID definitions
#define TCESC_CONTROL 0x384      // 900
#define SUSPENSION_CONTROL 0x1FC // 508

//CAN-CH bus data definitions
#define SUSP_SOFT 0x50 // 80
#define SUSP_MID 0x40  // 64
#define SUSP_FIRM 0x60 // 96

//DNA Mode definitions
#define DNA_RACE 0x31               // 49
#define DNA_DYNAMIC 0x9             // 9
#define DNA_NEUTRAL 0x1             // 1
#define DNA_ADVANCED_EFFICENCY 0x11 // 17

//should be pin 10, according to the data sheet
#define SPI_CS_CAN 10

MCPCAN CAN(SPI_CS_CAN);

byte len = 0;
byte buf[8] = {0};
byte tcesc_buf[8] = {0};
byte last_dna_mode = 0x9;

int setupMasksAndFilters()
{
  // (up to) 2 mask-registers (0 and 1)
  // mask defines which bits of the filter are to be checked
  // 0x7ff -> max. of 11-bit address -> check all bits
  int mask0InitSuccess = CAN.init_Mask(MCP_RXM0, 0, 0x7ff);
  int mask1InitSuccess = CAN.init_Mask(MCP_RXM1, 0, 0x7ff);

  // (up to) 5 filter-registers
  // mask 0 applies to filters 0 and 1; mask 1 applies to filters 2-5
  // filter defines which message ids are accepted
  int filter0InitSuccess = CAN.init_Filter(MCP_RXF0, 0, TCESC_CONTROL);
  int filter1InitSuccess = CAN.init_Filter(MCP_RXF1, 0, SUSPENSION_CONTROL);

  int overallSuccess = mask0InitSuccess || mask1InitSuccess || filter0InitSuccess || filter1InitSuccess;

  if (overallSuccess != MCP_OK)
  {
    Serial.println("mask and/or filter setup failed!");
    Serial.print("mask0InitSuccess: ");
    Serial.println(mask0InitSuccess);
    Serial.print("mask1InitSuccess: ");
    Serial.println(mask1InitSuccess);
    Serial.print("filter0InitSuccess: ");
    Serial.println(filter0InitSuccess);
    Serial.print("filter1InitSuccess: ");
    Serial.println(filter1InitSuccess);
  }

  return overallSuccess;
}

void setup()
{
  Serial.begin(115200);

  int beginSuccess = CAN_FAIL;
  do
  {
    Serial.println("CAN begin...");
    CAN.init();
    beginSuccess = CAN.begin(CAN_500KBPS);
    Serial.print("result: ");
    Serial.println(beginSuccess);
  } while (beginSuccess != CAN_OK);
  Serial.println("CAN BUS Shield init ok!");

  int maskAndFilterSuccess = MCP_FAIL;
  do
  {
    Serial.println("setting up masks and filters...");
    maskAndFilterSuccess = setupMasksAndFilters();
  } while(maskAndFilterSuccess != MCP_OK);
  Serial.println("mask and filter setup ok!");

  Serial.println("setup done.");
}

void handle_tcesc_control()
{
  if (tcesc_buf[1] != DNA_NEUTRAL &&
      tcesc_buf[1] != DNA_DYNAMIC &&
      tcesc_buf[1] != DNA_ADVANCED_EFFICENCY &&
      tcesc_buf[1] != DNA_RACE)
  {
    Serial.println("RETURN - DO NTH.");
    Serial.println(tcesc_buf[1]);
    return;
  }

  Serial.println("SEND MESSAGE TO TCESC control!");
  Serial.println(tcesc_buf[1]);
  CAN.sendMsgBuf(TCESC_CONTROL, 0, 8, tcesc_buf);
}

void convertBufToStr(byte string)
{
  switch (string)
  {
  case DNA_NEUTRAL:
  {
    Serial.println("DNA_NEUTRAL");
    break;
  }
  case DNA_ADVANCED_EFFICENCY:
  {
    Serial.println("DNA_ADVANCED_EFFICENCY");
    break;
  }
  case DNA_DYNAMIC:
  {
    Serial.println("DNA_DYNAMIC");
    break;
  }
  case DNA_RACE:
  {
    Serial.println("DNA_RACE");
    break;
  }
  }
}

void loop()
{
  unsigned long id = 0;
  if (CAN.checkReceive() == CAN_MSGAVAIL)
  { //consider switch to interrupt mode
    CAN.readMsgBufID(&id, &len, buf);
    memcpy(tcesc_buf, buf, 8);

    Serial.println("Got id from buffer: ");
    Serial.println(id);

    Serial.println(last_dna_mode);
    Serial.println(tcesc_buf[1]);

    // check if dna mode changed and if it is a request for TCESC control
    if (id == TCESC_CONTROL && last_dna_mode != tcesc_buf[1])
    {
      Serial.println("ID seems to be TCESC_CONTROL");
      Serial.println("print buffer 1 / DNA mode: ");
      Serial.println(tcesc_buf[1]);

      convertBufToStr(tcesc_buf[1]);
      handle_tcesc_control();
      Serial.println("last dna mode");
      Serial.println(last_dna_mode);
      last_dna_mode = tcesc_buf[1];
      Serial.println("NEW last dna mode");
      Serial.println(tcesc_buf[1]);
    }
  }
}

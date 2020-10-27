#include <df_can.h>
#include <df_candfs.h>
#include <time.h>

// CAN-CH bus message id definitions
#define TCESC_CONTROL 0x384      // 900
#define SUSPENSION_CONTROL 0x1FC // 508

// CAN-CH bus data definitions
// messages with id TCESC_CONTROL, values at idx 6
#define TC_SUSP_SOFTEN_1_STEP 0x80 // 128
#define TC_SUSP_DEFAULT 0x0         // 0

// messages with id SUSPENSION_CONTROL, values at idx 0
#define SUSP_NORMAL 0x10    // 16 - default value in A and N mode; D mode with "soft" pressed
#define SUSP_MEDIUM 0x0     // 0 - default value in D mode
#define SUSP_HARD_RACE 0x60 // 96 - default value in Race mode
#define SUSP_MID_RACE 0x40  // 64 - value after pressing the "soft" button

// DNA Mode definitions
// messages with id TCESC_CONTROL, values at idx 1
#define DNA_RACE 0x31               // 49
#define DNA_DYNAMIC 0x9             // 9
#define DNA_NEUTRAL 0x1             // 1
#define DNA_ADVANCED_EFFICENCY 0x11 // 17

// should be pin 10, according to the data sheet
#define SPI_CS_CAN 10

MCPCAN CAN(SPI_CS_CAN);

byte len = 0;
byte readBuffer[8] = {0};
/* tcesc_buf: data to be written to the bus
    idx 0:  ???
    idx 1:  DNA-Mode
    idx 2:  ???
    idx 3:  buttons (LDW)?!
    idx ...
    idx 6:  suspension-button (dna-knob)
*/
byte tcesc_buf[8] = {0};

byte last_dna_mode = 0;
byte left_stalk_count = 0;
byte tc_disable = 0;

int setupMasksAndFilters()
{
  // (up to) 2 mask-registers (0 and 1)
  // mask defines which bits of the filter/id are to be checked
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
  } while (maskAndFilterSuccess != MCP_OK);
  Serial.println("mask and filter setup ok!");

  Serial.println("setup done.");
}

void handle_tcesc_control()
{
  // sanity check, don't want to send random shit to module if buffer corrupt/incorrect
  if (tcesc_buf[1] != DNA_NEUTRAL &&
      tcesc_buf[1] != DNA_DYNAMIC &&
      tcesc_buf[1] != DNA_ADVANCED_EFFICENCY &&
      tcesc_buf[1] != DNA_RACE)
  {
    return;
  }

  // TCESC disable in race mode
  if (tc_disable && tcesc_buf[1] != DNA_RACE)
  {
    tcesc_buf[1] = DNA_RACE;
  }
  // car in race mode wants TC, put TCESC in Dynamic mode
  else if (!tc_disable && tcesc_buf[1] == DNA_RACE)
  {
    tcesc_buf[1] = DNA_DYNAMIC;
  }
  // nothing to do if driver wants TCESC on in non-race mode or off in race-mode. Car can manage itself
  else
  {
    return;
  }
  CAN.sendMsgBuf(TCESC_CONTROL, 0, 8, tcesc_buf);
}

void printReadBuffer(unsigned long messageIdPrefix)
{
  switch (messageIdPrefix)
  {
  case TCESC_CONTROL:
    Serial.print("TCESC:\t");
    break;
  case SUSPENSION_CONTROL:
    Serial.print("SUSP:\t");
    break;
  }

  for (int i = 0; i < 8; i++)
  {
    Serial.print(readBuffer[i]);
    Serial.print("\t");
  }
  Serial.println("");
}

void loop()
{
  unsigned long id = 0;
  if (CAN.checkReceive() == CAN_MSGAVAIL)
  {
    CAN.readMsgBufID(&id, &len, readBuffer);

    if (id == TCESC_CONTROL)
    {
      memcpy(tcesc_buf, readBuffer, 8);

      // driver pressing left stalk button
      if (tcesc_buf[3] & 0x40)
      {
        left_stalk_count++;
        if (left_stalk_count > 8)
        {
          tc_disable ^= 1;      // toggle tc_disable between 0 and 1
          left_stalk_count = 0; // reset stalk count for next press event
        }
      }
      else
      {
        left_stalk_count = 0;
      }

      /*
       * let's detect real DNA mode change to/from race for QV cars and change to expected TC/ESC setting when that happens
       */
      if (last_dna_mode == DNA_RACE && tcesc_buf[1] != DNA_RACE)
      {
        tc_disable = 0;
      }
      else if (last_dna_mode != DNA_RACE && tcesc_buf[1] == DNA_RACE)
      {
        tc_disable = 1;
      }
      last_dna_mode = tcesc_buf[1];
    }

    // added extra check to deal with library/filter bug, don't spam the bus
    if (id == TCESC_CONTROL || id == SUSPENSION_CONTROL)
    {
      printReadBuffer(id);
      handle_tcesc_control();
    }
  }
}
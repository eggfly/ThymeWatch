#include "IT7259Driver.h"

/*
  The IT7259 chip uses the I2C address: 1000110

  The format for this driver is little endian
*/
#define DELAY_BETWEEN_TRANSMISSIONS 1

#define DEBUG_LEVEL 1
#define INFO 0
#define DEBUG 1
#define ERROR 2

#define SCREEN_OFFSET_X 0
#define SCREEN_OFFSET_Y 0

#define DEVICE_ADDRESS 0x46 // (1000110 is 46 in hex)

#define BURST_WRITE_COMMAND_BUFFER 0x20
#define BURST_READ_COMMAND_BUFFER 0xA0
#define POINT_INFORMATION_BUFFER 0xE0
#define QUERY_BUFFER 0x80

#define DEVICE_NAME_LENGTH 0x0A

// COMMANDS
#define DEVICE_NAME 0x00
#define GET_PANEL_INFO 0x01
#define SET_PANEL_INFO 0x02
#define SET_POWER_MODE 0x04
#define GET_VAR_VALUE 0x05
#define SET_VAR_VALUE 0x06
#define RESET_QUEUE 0x07
#define ENTER_OR_EXIT_PURE_COMMAND_MODE 0x08
#define SET_START_OFFSET_OF_FLASH 0x09
#define READ_FLASH 0x0B
#define REINITIALIZE_FIRMWARE 0x0C
#define WRITE_MEM_OR_REG 0x0D
#define READ_MEM_OR_REG 0x0E
#define SET_CHARGE_MODE 0x0F
#define SET_GSM_MODE 0x10
#define GET_ALGORITHM_PARAM 0x14
#define SET_AGLORITHM_PARAM 0x15
#define WRITE_START 0x16
#define WRITE_CONTINUE 0x17
#define READ_START 0x18
#define READ_CONTINUE 0x19
#define FUNCTION_TEST 0x1A
#define AUTO_TUNE_CDC 0x1C

// SUB-COMMANDS

// Get cap sensor information
#define FIRMWARE_INFO 0x00
#define PANEL_RES 0x02
#define FLASH_SIZE 0x03
#define INT_NOTIFICATION_STATUS 0x04 // The same command for set cap sensor information
#define GESTURE_INFO 0x05
#define CONFIG_VERSION 0x06
// Get algorithm parameter
#define CDC_TUNE_LEVEL 0x5A



// GESTURES
#define TAP 0x20
#define PRESS 0x21
#define FLICK 0x22
#define DOUBLE_TAP 0x23
#define TAP_AND_SLIDE 0x24
#define DRAG 0x25
#define DIRECTION 0x26
#define TURN 0x27
#define CLOCKWISE 0x28
#define DIR_4WAY 0x29




/*
					COMMANDS:
  ---------------------------------------------------
  Command 0x00: Device Name

  COMMAND FORMAT:
	Command = 0x00
	sub command = None
	Parameter = None

  DATA FORMAT:
	Data Length: 0x0A
	Vendor ID: ASCII "ITE"
	Device ID: ASCII "7259"
	Device Version: 0x30, 0x30 for AX IC
					0x31, 0x30 for TC IC
  ---------------------------------------------------
  Command 0x18: Read Start

  COMMAND FORMAT:
	Command = 0x18
	Sub command = 0x04 (Reserved)
				  0x05 (Command Response Buffer)
				  0x06 (Reserved)
				  0x07 (Point Buffer)
	Paramter = None

  DATA FORMAT:
	Command Status: 0x0000 (Success)
  ---------------------------------------------------
  Command 0x19: Read Continue

  COMMAND FORMAT:
	Command = 0x19
	Sub command = 0x00 (Not final read command)
				  0x01 (Final read command)
	Paramter = Size (Unit: Byte)

  DATA FORMAT:
	Data Buffer: Data
  ---------------------------------------------------
*/



/*
	Write using burst to command buffer.

	Order of operations:
*	* start transmission
*	* write into mode-register
*	* write command
*	* write sub command if provided
*/
void writeToCommandBuffer(uint8_t deviceAddress, bool repeatedStartBit, uint8_t buffer, int8_t command = -1, int8_t subCommand = -1, int8_t parameter = -1, int8_t extraParameter = -1)
{
  Wire.beginTransmission(deviceAddress);
  Wire.write(buffer);

  if (command != -1)
  {
    Serial.println("DAFUQ");
    Wire.write(command);
    if (subCommand != -1)
      Wire.write(subCommand);
    if (parameter != -1)
      Wire.write(parameter);
    if (extraParameter != -1)
      Wire.write(extraParameter);
  }

  uint16_t error = Wire.endTransmission(!repeatedStartBit); // repeated start bit should send false here

  if (error != 0)
  {
    Serial.print("Failed to write command: ");
    Serial.print(command);
    Serial.print(" with sub command: ");
    Serial.print(subCommand);
    if (parameter != -1)
    {
      Serial.print(" and parameter: ");
      Serial.println(parameter);
    }
    else
      Serial.println("");

    Serial.print("The error is: ");
    Serial.print(error);
    switch (error)
    {
      case 0x0001:
        Serial.println(" (Unknown command)");
        break;
      case 0x0002:
        Serial.println(" (Unknown sub command)");
        break;
      case 0x0003:
        Serial.println(" (Incorrect data length or count)");
        break;
      case 0x0004:
        Serial.println(" (When read/write memory, data type is not valid)");
        break;
      case 0x0005:
        Serial.println(" (In memory or flash, offset + size > boundary)");
        break;
      case 0x0006:
        Serial.println(" (Incorrect parameter value)");
        break;
      case 0x0010:
        Serial.println(" (Invalid firmware upgrade key)");
        break;
      case 0x0011:
        Serial.println(" (Write flash operation failed since it is not in firmware upgrade mode)");
        break;
      case 0x0012:
        Serial.println(" (Write flash operation failed since the offset is not in page boundary)");
        break;
      case 0x0013:
        Serial.println(" (Read/Write flash data length is incorrect)");
        break;
      case 0x0014:
        Serial.println(" (Incorrect configuration ID)");
        break;
      case 0x0015:
        Serial.println(" (Information is not available since program is running in bootloader)");
        break;
      case 0x0016:
        Serial.println(" (Information is not available since program is running in flash)");
        break;
      case 0x0017:
        Serial.println(" (Operation is not supported)");
        break;
      case 0xFFFF:
        Serial.println(" (Panel reported Unknown error)");
        break;

      default:
        Serial.println(" (Unknown)");
        break;
    }
  }
}

/*
	Reads using burst read from the command response buffer.
*/
void readFromCommandResponseBuffer(uint8_t deviceAddress, int8_t buffer, uint8_t lengthToRead, uint8_t *response)
{
  if (buffer != -1)
  {
    Wire.beginTransmission(deviceAddress);
    Wire.write(buffer);
    uint16_t error = Wire.endTransmission(false);

    delay(DELAY_BETWEEN_TRANSMISSIONS); // Delay before sending request to device (it cannot handle too fast transmission)

    if (error != 0)
    {
      Serial.print("Failed to write command with error: ");
      Serial.println(error);
    }
  }

  Wire.requestFrom(deviceAddress, lengthToRead);

  uint8_t i = 0;
  while (Wire.available())
  {
    response[i++] = Wire.read();
  }

  Wire.endTransmission(true);
  delay(DELAY_BETWEEN_TRANSMISSIONS);
}

int8_t getTypeOfEvent(byte formatData)
{
  if (formatData >> 4 == 8)
  {
    if (DEBUG_LEVEL >= DEBUG)
      Serial.println("Event type is: Gesture information");

    return 0;
  }
  else if (formatData >> 4 == 4)
  {
    if (DEBUG_LEVEL >= DEBUG)
      Serial.println("Event type is: Touch Event");

    return 1;
  }
  else if (formatData >> 4 == 1)
  {
    if (DEBUG_LEVEL >= DEBUG)
      Serial.println("Event type is: Wakeup");

    return 2;
  }
  else if (formatData >> 4 == 0)
  {
    if (DEBUG_LEVEL >= DEBUG)
      Serial.println("Event type is: Point information");

    return 3;
  }

  if (DEBUG_LEVEL >= DEBUG)
  {
    Serial.print("Event type is unknown with code: ");
    Serial.println(formatData >> 4);
  }

  return -1;
}

void getGesture(byte *responseBuffer, struct TouchPointData *responseData)
{
  if (DEBUG_LEVEL >= DEBUG)
    Serial.println("Need to implement classes extending a super-class Gesture");
  switch (responseBuffer[1])
  {
    case TAP:
      if (DEBUG_LEVEL >= DEBUG)
        Serial.println("Gesture is: Tap");

      responseData->xPos = responseBuffer[2] + SCREEN_OFFSET_X;
      responseData->yPos = responseBuffer[4] + SCREEN_OFFSET_Y;
      responseData->isNull = false;
      break;
    case PRESS:
      if (DEBUG_LEVEL >= DEBUG)
        Serial.println("Gesture is: Press");
      break;
    case FLICK:
      if (DEBUG_LEVEL >= DEBUG)
        Serial.println("Gesture is: Flick");
      break;
    case DOUBLE_TAP:
      if (DEBUG_LEVEL >= DEBUG)
        Serial.println("Gesture is: Double-Tap");
      break;
    case TAP_AND_SLIDE:
      if (DEBUG_LEVEL >= DEBUG)
        Serial.println("Gesture is: Tap-and-Slide");
      break;
    case DRAG:
      if (DEBUG_LEVEL >= DEBUG)
        Serial.println("Gesture is: Drag");
      break;
    case DIRECTION:
      if (DEBUG_LEVEL >= DEBUG)
        Serial.println("Gesture is: Direction");
      break;
    case TURN:
      if (DEBUG_LEVEL >= DEBUG)
        Serial.println("Gesture is: Turn");
      break;
    case CLOCKWISE:
      if (DEBUG_LEVEL >= DEBUG)
        Serial.println("Gesture is: Clockwise");
      break;
    case DIR_4WAY:
      if (DEBUG_LEVEL >= DEBUG)
        Serial.println("Gesture is: Dir_4Way");
      break;

    default:
      if (DEBUG_LEVEL >= DEBUG)
        Serial.println("Gesture is: Unknown");
      break;
  }
}

/*
	The same as readFromCommandResponseBuffer just other register to read from
	Read point information from point information buffer
*/
void readPointInformation(uint8_t deviceAddress, uint8_t lengthToRead, TouchPointData* pointData)
{
  writeToCommandBuffer(deviceAddress, true, POINT_INFORMATION_BUFFER);

  delay(DELAY_BETWEEN_TRANSMISSIONS); // Delay before sending request to device (it cannot handle too fast transmission)

  uint8_t responseBuffer[lengthToRead];
  readFromCommandResponseBuffer(deviceAddress, -1, lengthToRead, responseBuffer);

  int8_t eventType = getTypeOfEvent(responseBuffer[0]);

  if (eventType == 0) // Gesture event
  {
    getGesture(responseBuffer, pointData);
  }
  else if (eventType == 1) // Touch event
  {
    
  }
  else if (eventType == 2) // Wakeup event
  {

  }
  else if (eventType == 3) // Point information event
  {
    pointData->xPos = responseBuffer[2] + SCREEN_OFFSET_X;
    pointData->yPos = responseBuffer[4] + SCREEN_OFFSET_Y;
    pointData->isNull = false;
    Serial.printf("%d,%d\n", pointData->xPos, pointData->yPos);
  }
  else // Unknown event
  {

  }
}

/*
	Reads the next in queue touch event of the panel
*/
void readTouchEvent(struct TouchPointData *dataBuffer)
{
  writeToCommandBuffer(DEVICE_ADDRESS, true, QUERY_BUFFER);

  delay(DELAY_BETWEEN_TRANSMISSIONS);

  uint8_t packetInformationStatus;
  readFromCommandResponseBuffer(DEVICE_ADDRESS, -1, 1, &packetInformationStatus);
  packetInformationStatus = packetInformationStatus >> 6;

  if (packetInformationStatus >> 1 == 1) // change to == 3 without bit-shift
  {
    readPointInformation(DEVICE_ADDRESS, 14, dataBuffer);
    return;
  }
  else if (packetInformationStatus == 1)
  {
    dataBuffer->isNull = true;
    return;
  }

  delay(DELAY_BETWEEN_TRANSMISSIONS);

  dataBuffer->isNull = true;
  return;
}



/*
  Needs to be reworked
*/
void getDeviceName()
{
  writeToCommandBuffer(DEVICE_ADDRESS, false, BURST_WRITE_COMMAND_BUFFER, DEVICE_NAME);

  uint8_t response[10]; // 10 = 0x0A
  readFromCommandResponseBuffer(DEVICE_ADDRESS, BURST_READ_COMMAND_BUFFER, 10, response); // 10 = 0x0A
}

/*
	Get the firmware version of the panel
*/
void getFirmwareInfo(struct PanelCapabilities *panelCap, bool readVendorFirmwareVersion = false)
{
  writeToCommandBuffer(DEVICE_ADDRESS, false, BURST_WRITE_COMMAND_BUFFER, GET_PANEL_INFO, FIRMWARE_INFO);

  uint8_t length = 0;
  if (readVendorFirmwareVersion)
    length = 10; // 10 = 0x0A
  else
    length = 9; // 9 = 0x09

  uint8_t response[length];
  readFromCommandResponseBuffer(DEVICE_ADDRESS, BURST_READ_COMMAND_BUFFER, length, response);

  // The response from the touch panel is added to user provided buffer below
  for (int i = 0; i < 4; i++)
    panelCap->romVersion[i] = response[i + 1]; // Skip the first byte as it contains the size of the response

  for (int i = 0; i < 4; i++)
    panelCap->flashFirmwareVersion[i] = response[i + 5];

  if (readVendorFirmwareVersion)
    panelCap->vendorFirmwareVersion = response[length - 1];
}

/*
	Get panel specific details such as resolution
*/
void getPanelResolutions(struct PanelCapabilities *panelCap)
{
  writeToCommandBuffer(DEVICE_ADDRESS, false, BURST_WRITE_COMMAND_BUFFER, GET_PANEL_INFO, PANEL_RES);

  uint8_t response[12]; // 12 = 0x0C
  readFromCommandResponseBuffer(DEVICE_ADDRESS, BURST_READ_COMMAND_BUFFER, 12, response); // 12 = 0x0C

  // The response from the touch panel is added to user provided buffer below
  panelCap->xRes = response[3] << 8 | response[2];
  panelCap->yRes = response[5] << 8 | response[4];
  panelCap->scale = response[6];
  panelCap->connectionType = response[7];
  panelCap->stageA = response[8];
  panelCap->stageB = response[9];
  panelCap->stageC = response[10];
  panelCap->stageD = response[11];
}

/*
	Get the code size on the flash memory
*/
void getFlashSize(struct PanelCapabilities *panelCap)
{
  writeToCommandBuffer(DEVICE_ADDRESS, false, BURST_WRITE_COMMAND_BUFFER, GET_PANEL_INFO, FLASH_SIZE);

  uint8_t response[3]; // 3 = 0x03
  readFromCommandResponseBuffer(DEVICE_ADDRESS, BURST_READ_COMMAND_BUFFER, 3, response); // 3 = 0x03

  panelCap->flashCodeSize = response[2] << 8 | response[1]; // LSB and first byte is size of response
}

/*
	Get interrupt status and what triggers an interrupt
*/
void getInterrupNotificationStatus(struct PanelCapabilities *panelCap)
{
  writeToCommandBuffer(DEVICE_ADDRESS, false, BURST_WRITE_COMMAND_BUFFER, GET_PANEL_INFO, INT_NOTIFICATION_STATUS);

  uint8_t response[2];
  readFromCommandResponseBuffer(DEVICE_ADDRESS, BURST_READ_COMMAND_BUFFER, 2, response);

  panelCap->interruptNotificationStatus = response[0];
  panelCap->interruptType = response[1];
}

/*
	Count number of bits set to 1 in a byte
*/
uint8_t countNumberOfBitsSetToOne(uint8_t byteToCheck)
{
  uint8_t currentByte = byteToCheck;
  uint8_t counter = 0;
  while (currentByte) {
    counter += currentByte & 1;
    currentByte >>= 1;
  }

  return counter;
}

/*
	Get the gesture capabilities
	Will save the number of gestures supported
*/
void getGestureInfo(struct PanelCapabilities *panelCap)
{
  writeToCommandBuffer(DEVICE_ADDRESS, false, BURST_WRITE_COMMAND_BUFFER, GET_PANEL_INFO, GESTURE_INFO);

  uint8_t response[14]; // 14 = 0x0E
  readFromCommandResponseBuffer(DEVICE_ADDRESS, BURST_READ_COMMAND_BUFFER, 14, response); // 14 = 0x0E

  panelCap->gestureSupport = response[1]; // Skip first byte which is the size of the response

  uint8_t counter = 0;
  for (int i = 2; i < 6; i++)
    counter += countNumberOfBitsSetToOne(response[i]);

  panelCap->oneFingerGesture = counter;

  counter = 0;
  for (int i = 6; i < 10; i++)
    counter += countNumberOfBitsSetToOne(response[i]);

  panelCap->twoFingerGesture = counter;

  counter = 0;
  for (int i = 10; i < 14; i++)
    counter += countNumberOfBitsSetToOne(response[i]);

  panelCap->threeFingerGesture = counter;
}

/*
	Get the version of the configuration
*/
void getConfigurationVersion(struct PanelCapabilities *panelCap)
{
  writeToCommandBuffer(DEVICE_ADDRESS, false, BURST_WRITE_COMMAND_BUFFER, GET_PANEL_INFO, CONFIG_VERSION);

  uint8_t response[7]; // 7 = 0x07
  readFromCommandResponseBuffer(DEVICE_ADDRESS, BURST_READ_COMMAND_BUFFER, 7, response); // 7 = 0x07

  for (int i = 1; i < 5; i++) // Skip first byte which is the size of the response
    panelCap->configVersion[i - 1] = response[i];
}

/*
  Get Cap Sensor Information
*/
void getPanelCapabilities(struct PanelCapabilities *panelCap)
{
  getFirmwareInfo(panelCap, true);
  getPanelResolutions(panelCap);
  getFlashSize(panelCap);
  // getInterruptNotificationStatus(panelCap);
  getGestureInfo(panelCap);
  getConfigurationVersion(panelCap);
}

/*	PARAM DESCRIPTION
	status:
	0x00 - Disable
	0x01 - Enable

	type:
	0x00 - Low level trigger
	0x01 - High level trigger
	0x10 - Falling edge trigger
	0x11 - Rising edge trigger
*/
void setInterruptNotificationBehaviour(uint8_t status, uint8_t type)
{
  writeToCommandBuffer(DEVICE_ADDRESS, false, BURST_WRITE_COMMAND_BUFFER, SET_PANEL_INFO, INT_NOTIFICATION_STATUS, status, type);

  uint8_t response[2]; // 0x0000 is two bytes
  readFromCommandResponseBuffer(DEVICE_ADDRESS, BURST_READ_COMMAND_BUFFER, 2, response); // 0x0000 is two bytes

  if ((response[1] << 8 | response[0]) == 0)
  {
    if (DEBUG_LEVEL >= DEBUG)
      Serial.println("Successfully set interrupt notification");
  }
  else
  {
    if (DEBUG_LEVEL >= DEBUG)
    {
      Serial.println("Failed to set interrupt notification");
    }
  }
}

/*
  When setting the power mode to idle, the power consumption goes from 1.5mA (active) to 25uA (idle).
  In idle mode the device no longer report packets of points and gestures.
  The device will go into active mode once the user touch it.
*/
void setPowerModeIdle()
{
  writeToCommandBuffer(DEVICE_ADDRESS, false, BURST_WRITE_COMMAND_BUFFER, SET_POWER_MODE, 0, 1);

  // Response is not defined in the datasheet
}

/*
	Reset the queue of point/gesture/event information of the ITE Cap Sensor device
*/
void resetEventQueue()
{
  writeToCommandBuffer(DEVICE_ADDRESS, false, BURST_WRITE_COMMAND_BUFFER, RESET_QUEUE);

  uint8_t response[2]; // 0x0000 is two bytes
  readFromCommandResponseBuffer(DEVICE_ADDRESS, BURST_READ_COMMAND_BUFFER, 2, response); // 0x0000 is two bytes

  if (DEBUG_LEVEL >= DEBUG)
  {
    if ((response[1] << 8 | response[0]) == 0)
      Serial.println("Successfully reset queue");
    else
      Serial.println("Failed to reset queue");
  }
}

/*
	Reinitialize the firmware (Reset)
	The capacitive touch panel will be reset and requires 100ms to be in a ready state again
*/
void resetFirmware()
{
  writeToCommandBuffer(DEVICE_ADDRESS, false, BURST_WRITE_COMMAND_BUFFER, REINITIALIZE_FIRMWARE);

  uint8_t response[2]; // 0x0000 is two bytes
  readFromCommandResponseBuffer(DEVICE_ADDRESS, BURST_READ_COMMAND_BUFFER, 2, response); // 0x0000 is two bytes

  if (DEBUG_LEVEL >= DEBUG)
  {
    if ((response[1] << 8 | response[1]) == 0)
      Serial.println("Successfully reset the firmware");
    else
      Serial.println("Failed to reset the firmware");
  }
}

/*
	Enter or exit charge mode
*/
void setChargeMode(bool status)
{
  if (status)
    writeToCommandBuffer(DEVICE_ADDRESS, false, BURST_WRITE_COMMAND_BUFFER, SET_CHARGE_MODE, 1); // Enter charge mode
  else
    writeToCommandBuffer(DEVICE_ADDRESS, false, BURST_WRITE_COMMAND_BUFFER, SET_CHARGE_MODE, 0); // Enter normal mode

  uint8_t response[2]; // 0x0000 is two bytes
  readFromCommandResponseBuffer(DEVICE_ADDRESS, BURST_READ_COMMAND_BUFFER, 2, response); // 0x0000 is two bytes

  if (DEBUG_LEVEL >= DEBUG)
  {
    if ((response[1] << 8 | response[1]) == 0)
      Serial.println("Successfully set charge mode");
    else
      Serial.println("Failed to set charge mode");
  }
}

/*
	Enter or exit GSM mode
*/
void setGsmMode(bool status)
{
  if (status)
    writeToCommandBuffer(DEVICE_ADDRESS, false, BURST_WRITE_COMMAND_BUFFER, SET_GSM_MODE, 1); // Enter GSM mode
  else
    writeToCommandBuffer(DEVICE_ADDRESS, false, BURST_WRITE_COMMAND_BUFFER, SET_GSM_MODE, 0); // Exit GSM mode

  uint8_t response[2]; // 0x0000 is two bytes
  readFromCommandResponseBuffer(DEVICE_ADDRESS, BURST_READ_COMMAND_BUFFER, 2, response); // 0x0000 is two bytes

  if (DEBUG_LEVEL >= DEBUG)
  {
    if ((response[1] << 8 | response[1]) == 0)
      Serial.println("Successfully set GSM mode");
    else
      Serial.println("Failed to set GSM mode");
  }
}

/*
	Gets the algorithm parameters from the ITE Cap Sensor device
*/
void getAlgorithmParam(uint16_t *cdcTuneLevel)
{
  writeToCommandBuffer(DEVICE_ADDRESS, false, BURST_WRITE_COMMAND_BUFFER, GET_ALGORITHM_PARAM, CDC_TUNE_LEVEL);

  uint8_t response[4]; // 4 = 0x04 which is one more than 0x03
  readFromCommandResponseBuffer(DEVICE_ADDRESS, BURST_READ_COMMAND_BUFFER, 3, response); // 4 = 0x04 which is more than 0x03

  cdcTuneLevel[0] = response[3] << 8 | response[2];
}




/*
	Sets the algorithm parameters
	(Not implemented)
*/
void setAlgorithmParams()
{

}

#ifndef _IT7259_Driver_H_
#define _IT7259_Driver_H_

#include "Arduino.h"
#include "Wire.h"

struct TouchPointData
{
	bool isNull;
	uint8_t xPos;
	uint8_t yPos;
};


struct PanelCapabilities
{
	// Firmware Information
	uint8_t romVersion[4]; // A.B.C.D
	uint8_t flashFirmwareVersion[4]; // A.B.C.D
	uint8_t vendorFirmwareVersion; // A

	// 2D Resolutions
	uint16_t xRes;
	uint16_t yRes;
	uint8_t scale;
	uint8_t connectionType;
	uint8_t stageA; // Channel of X-axis
	uint8_t stageB; // Channel of Y-axis
	uint8_t stageC; // Additional Channel
	uint8_t stageD; // Button Number

	// Flash Size
	uint16_t flashCodeSize;

	// Interrupt Notification Status
	uint8_t interruptNotificationStatus; // 0 = disabled, 1 = enabled
	uint8_t interruptType; // 0 = Low level trigger, 1 = High level trigger, 2 = Falling edge trigger, 3 = Rising edge trigger

	// Gesture Information
	uint8_t gestureSupport; // 0 = Unsupported, 1 = Supported
	uint32_t oneFingerGesture; // number of gestures supported
	uint32_t twoFingerGesture; // number of gestures supported
	uint32_t threeFingerGesture; // number of gestures supported

	// Configuration Version
	uint8_t configVersion[4]; // A.B.C.D
};

void getDeviceName();
// void readTouchEvent(TouchPointData* data = NULL);
void readTouchEvent(struct TouchPointData *dataBuffer);

void getPanelCapabilities(struct PanelCapabilities *panelCap);

void setInterruptNotificationBehaviour(uint8_t status, uint8_t type);




void getAlgorithmParam(uint16_t *cdcTuneLevel);

void setAlgorithmParams(); // Not implemented


#endif

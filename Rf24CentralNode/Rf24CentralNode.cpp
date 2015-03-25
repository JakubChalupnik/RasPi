//*******************************************************************************
//*
//* RasPi based router for RF24Network
//* Based on "helloworld" from RF24Network library by J. Coliz <maniacbug@ymail.com> / TMRh20
//*
//*******************************************************************************
//* Processor:  RasPi
//* Author      Date       Comment
//*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//* Kubik       19.1.2015 First version, just basic code for HW tests
//* Kubik       10.2.2015 Switched over to ncurses and abandoning the T6963C 
//* Kubik       19.3.2015 Migrated to RF24Radio for sensors 
//******************************************************************************* 

//*******************************************************************************
//*                            HW details                                       *
//*******************************************************************************
// Standard NRF24L01+ module (eBay)
//
// Used pins:
//  NRF24L01+       SPI + CS0, GPIO22
//

//*******************************************************************************
//*                           Includes and defines                              *
//*******************************************************************************  
#include <cstdlib>
#include <iostream>
#include <RF24/RF24.h>
#include <RF24Network/RF24Network.h>
#include <ctime>
#include <stdio.h>
#include <time.h>
#include <ncurses.h>
#include "Rf24PacketDefine.h"

#define MAX_SENSORS 32
  
typedef struct {
  SensorPayload_t Payload;
  time_t LastReport;
} SensorNode_t;
   
//*******************************************************************************
//*                               Static variables                              *
//*******************************************************************************
 
//
// RF24 variables
//

RF24 Radio (RPI_V2_GPIO_P1_22, BCM2835_SPI_CS0, BCM2835_SPI_SPEED_8MHZ);  

// RF24Network Network (Radio);    // Network uses that radio
SensorNode_t Sensors [MAX_SENSORS];
uint8_t SensorsCount = 0; 

int ScrRow, ScrCol;

//*******************************************************************************
//*                               Sensor node methods                           *
//*******************************************************************************
 
// const char *NodeBatteryType (Node_t *Node) {
	
	// switch (Node->Flags & F_BATTERY_MASK) {
		// case F_BATTERY_NONE:
			// return "None  ";
		// case F_BATTERY_CR2032:
			// return "CR2032";
		// case F_BATTERY_LIION:
			// return "LiIon ";
		// case F_BATTERY_SOLAR:
			// return "Solar ";
		// default:
			// return "Other ";
	// }
// }

// const char *NodeTempSensorType (Node_t *Node) {
	
	// switch (Node->Flags & F_SENSOR_MASK) {
		// case F_SENSOR_NONE:
			// return "None  ";
		// case F_SENSOR_DS1820:
			// return "DS1820";
		// case F_SENSOR_DS1822:
			// return "DS1822";
		// default:
			// return "Other ";
	// }
// }

//*******************************************************************************
//*                            Debug methods                                    *
//******************************************************************************* 

char *PrintBuffer (uint8_t *Buffer, int BufferSize) {
  static char buff[256];
	char *b = buff;

	while (BufferSize > 0) {
		BufferSize--;
		b += sprintf (b, "%2.2X ", *Buffer);
		Buffer++;
	}
	
	return buff;
}

//*******************************************************************************
//*                            Screen output helpers                            *
//******************************************************************************* 

char *PayloadToString (int Index) {
  static char Buffer[64];
  char *b;
  SensorPayload_t *p = &Sensors[Index].Payload;

  b = Buffer + sprintf (Buffer, "%c%c ", (char) (p->SensorId >> 8), (char) (p->SensorId & 0xFF));
  
  if (p->BattLevel == 255) {
    b += sprintf (b, "NoBat  ");
  } else if (p->BattLevel == 0) {
    b += sprintf (b, "BatLow ");
  } else {
    b += sprintf (b, "%4.2fV  ", p->BattLevel * 0.010 + 2.0);
  }
  
  switch (p->PacketType) {
    case RF24_SENSOR_TYPE_TEMP:
      b += sprintf (b, "%5.1fC ", (p->TemperatureInt + 0.05) / 10.0); 
      if (p->TemperatureExt < 65535) {
        b += sprintf (b, "%5.1fC ", (p->TemperatureExt + 0.05) / 10.0); 
      }
      break;
      
    case RF24_SENSOR_TYPE_METEO:
      b += sprintf (b, "%5.1fC %4dhPa %2d%%", (p->Temperature + 0.05) / 10.0, p->Pressure, p->Humidity); 
      break;
      
    case RF24_SENSOR_TYPE_SOLAR:
      b += sprintf (b, "%5.2fV %5.2fW", p->SolarVoltage / 1000.0, p->SolarPower / 100.0); 
      break;
      
    default:
      break;
  }
  
  return Buffer;
}

FILE *LogFile;

//*******************************************************************************
//*                            Arduino setup method                             *
//******************************************************************************* 

void setup (void) {
  
  //
  // RF24Network init
  //
  
  Radio.begin ();
	
	Radio.setPayloadSize (sizeof (SensorPayloadTemperature_t));
  Radio.setAutoAck (false);
  Radio.setPALevel (RF24_PA_HIGH);
  Radio.setDataRate (RF24_250KBPS);
  Radio.setChannel (RF24_RADIO_CHANNEL);
  Radio.openReadingPipe (1, RF24_SENSOR_PIPE);
  Radio.openWritingPipe (RF24_BROADCAST_PIPE);
  Radio.printDetails ();
  Radio.startListening();
  
  //
  // Setup other stuff
  //
  
  memset (Sensors, 0, sizeof (Sensors));
  
  //
  // ncurses stuff
  //
  
  initscr ();
  getmaxyx (stdscr, ScrRow, ScrCol);		// get the number of rows and columns 
	
	LogFile = fopen ("logfile.txt", "at");
	if (LogFile == NULL) {
		printf ("Logfile could not be opened, aborting\n");
		exit (1);
	}
} 

//*******************************************************************************
//*                              Main program loop                              *
//******************************************************************************* 

void loop (void) {
  SensorPayload_t Payload;
  bool ScreenNeedsUpdate = false;
	int i;

  if (Radio.available ()) {
    ScreenNeedsUpdate = true;
    
    Radio.read (&Payload, sizeof (SensorPayload_t));
    
    //
    // Search through all the known sensors and see if this one has reported already.
    // If found, copy the new values over the old one.
    // If not found, add the new entry into the array of known sensors
    //
    
    for (i = 0; i < SensorsCount; i++) {
      if (Payload.SensorId == Sensors[i].Payload.SensorId) {
        break;
      }
    }
    
    memcpy (&Sensors[i].Payload, &Payload, sizeof (Payload));
    Sensors[i].LastReport = time (NULL);
		
		// fprintf (LogFile, "%d %s (%s)\n", Sensors[i].LastReport, PayloadToString (i), PrintBuffer ((uint8_t *) &Payload, sizeof (Payload)));
		fprintf (LogFile, "%d %s\n", Sensors[i].LastReport, PayloadToString (i));
		fflush (LogFile);
		
    if (i == SensorsCount) {
      SensorsCount++;
    }
  }

	if (ScreenNeedsUpdate) {
		for (i = 0; i < SensorsCount; i++) {
			move (i + 1, 0);
			printw ("%s\n", PayloadToString (i));
		}
		refresh ();
	}
}
	

//*******************************************************************************
//*                              Main C function                                *
//******************************************************************************* 

int main (int argc, char** argv) {
	
	setup ();
	
	delay (5);
	
	while (1) {
		loop ();
		usleep (100);
	}

	return 0;
}


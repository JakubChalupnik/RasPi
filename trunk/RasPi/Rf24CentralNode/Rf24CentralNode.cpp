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

#define THIS_NODE 00    // Address of our node in Octal format. This is main router, thus 00
#define MAX_NODE_COUNT 32
  

typedef struct {
  uint16_t Address;
  uint8_t BattLevel;
  uint16_t Temperature[2];
  char Id[NODE_ID_SIZE];
  uint16_t Flags;
} Node_t;
   
//*******************************************************************************
//*                               Static variables                              *
//*******************************************************************************
 
//
// RF24 variables
//

RF24 Radio (RPI_V2_GPIO_P1_22, BCM2835_SPI_CS0, BCM2835_SPI_SPEED_8MHZ);  

RF24Network Network (Radio);    // Network uses that radio
Node_t Nodes [MAX_NODE_COUNT];
uint8_t NodeCount = 0; 

int ScrRow, ScrCol;

//*******************************************************************************
//*                               Sensor node methods                           *
//*******************************************************************************
 
const char *NodeBatteryType (Node_t *Node) {
	
	switch (Node->Flags & F_BATTERY_MASK) {
		case F_BATTERY_NONE:
			return "None  ";
		case F_BATTERY_CR2032:
			return "CR2032";
		case F_BATTERY_LIION:
			return "LiIon ";
		case F_BATTERY_SOLAR:
			return "Solar ";
		default:
			return "Other ";
	}
}

const char *NodeTempSensorType (Node_t *Node) {
	
	switch (Node->Flags & F_SENSOR_MASK) {
		case F_SENSOR_NONE:
			return "None  ";
		case F_SENSOR_DS1820:
			return "DS1820";
		case F_SENSOR_DS1822:
			return "DS1822";
		default:
			return "Other ";
	}
}

//*******************************************************************************
//*                            Debug methods                                    *
//******************************************************************************* 

void PrintBuffer (uint8_t *Buffer, int BufferSize) {

	while (BufferSize > 0) {
		BufferSize--;
		// printw ("%2.2X ", *Buffer);
		Buffer++;
	}
}

//*******************************************************************************
//*                            Arduino setup method                             *
//******************************************************************************* 

void setup (void) {
  
  //
  // RF24Network init
  //
  
  Radio.begin ();
  Network.begin (RF24_CHANNEL, THIS_NODE);
  Radio.printDetails();
  
  //
  // Setup other stuff
  //
  
  memset (Nodes, 0, sizeof (Nodes));
  
  //
  // ncurses stuff
  //
  
  initscr ();
  getmaxyx (stdscr, ScrRow, ScrCol);		// get the number of rows and columns 
} 

//*******************************************************************************
//*                              Main program loop                              *
//******************************************************************************* 

void loop (void) {
  static uint32_t PacketCounter = 0;
  uint32_t Fails, Oks;
  RF24NetworkHeader Header;        // If so, grab it and print it out
  PayloadTemperature_t *Payload;
  uint8_t i;
  char s[256];
  
  PayloadId_t *PayloadId;
  uint8_t Buffer[32];
  
  Payload = (PayloadTemperature_t *) Buffer;
  PayloadId = (PayloadId_t *) Buffer;

  //
  // Things that have to be done every loop pass
  //
  
  Network.update ();                 // Check the network regularly

  while (Network.available ()) {     // Is there anything ready for us?
    Network.read (Header, &Buffer, sizeof (Buffer));
    PacketCounter++;
	mvprintw (0, 0, "Packets: %d\n", PacketCounter);
	PrintBuffer (Buffer, sizeof (Buffer));
    
    for (i = 0; i < NodeCount; i++) {
      if (Header.from_node == Nodes [i].Address) {
        break;
      }
    }
    
    Nodes [i].Address = Header.from_node;
    Nodes [i].BattLevel = Payload->BattLevel;
    switch (Header.type) {
      case RF24_TYPE_TEMP:
        Nodes [i].Temperature [0] = Payload->Temperature [0];
        Nodes [i].Temperature [1] = Payload->Temperature [1];
        break;
        
      case RF24_TYPE_ID:
        memcpy (Nodes [i].Id, PayloadId->Id, NODE_ID_SIZE);
        Nodes [i].Flags = PayloadId->Flags;
        break;
        
      default:
        break;
    }

    if ((i == NodeCount) && (NodeCount < (MAX_NODE_COUNT - 1)))  {        // New node reporting itself
      NodeCount++;
    }

	// LcdGotoXY (0, 0);
	// sprintf (s, "Packets: %d nodesize %d\n", PacketCounter, NODE_ID_SIZE);
	// lprint (s);
	
	for (i = 0; i < NodeCount; i++) {
		sprintf (s, "%3.3o %*s %4.2fV %+3dC %s %s", Nodes [i].Address, NODE_ID_SIZE, Nodes [i].Id, (Nodes [i].BattLevel * 10 + 2000) / 1000.0, (((int16_t) Nodes [i].Temperature [0]) + 5) / 10, NodeBatteryType (&Nodes [i]), NodeTempSensorType (&Nodes [i]));
		move (i + 1, 0);
		printw ("%s\n", s);
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


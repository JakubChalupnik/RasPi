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
#include "Rf24PacketDefine.h"
#include "T6963Lcd.h"

#define THIS_NODE 00    // Address of our node in Octal format. This is main router, thus 00
#define MAX_NODE_COUNT 32
  

typedef struct {
  uint16_t Address;
  uint8_t BattLevel;
  uint8_t Temperature[4];
  char Id[8];
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

//*******************************************************************************
//*                            Arduino setup method                             *
//******************************************************************************* 

void setup (void) {
  
  printf ("[RF24RouterNode]\n");
 
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
  // LCD stuff
  //
  
  MainLcdInit ();
  LcdGotoXY (0, 0);
  lprint ("Hello world");
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
	printf ("Packets: %d\n", PacketCounter);
    
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
        Nodes [i].Temperature [2] = Payload->Temperature [2];
        Nodes [i].Temperature [3] = Payload->Temperature [3];
        break;
        
      case RF24_TYPE_ID:
        memcpy (Nodes [i].Id, PayloadId->Id, 8);
        Nodes [i].Flags = PayloadId->Flags;
        break;
        
      default:
        break;
    }

    if ((i == NodeCount) && (NodeCount < (MAX_NODE_COUNT - 1)))  {        // New node reporting itself
      NodeCount++;
    }

	LcdGotoXY (0, 0);
	sprintf (s, "Packets: %d\n", PacketCounter);
	lprint (s);
	
	for (i = 0; i < NodeCount; i++) {
		printf ("%3.3o: %8s %4dmV %4dC\n", Nodes [i].Address, Nodes [i].Id, Nodes [i].BattLevel + 2, Nodes [i].Temperature [0]);
		sprintf (s, "%3.3o: %8s %4dmV %4dC\n", Nodes [i].Address, Nodes [i].Id, Nodes [i].BattLevel + 2, Nodes [i].Temperature [0]);
		LcdGotoXY (0, i + 1);
		lprint (s);
	}
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


/***********************************/
/* MIDI_LEDuino Example            */
/***********************************/
/* Copyright uBld Electronics, LLC */
/***********************************/

/* Tested on an Arduino Leonardo */

#include <FastLED.h>
#define LED_PIN   7  
#define NUM_LEDS  8
CRGB leds[NUM_LEDS];

/* Max Brightness of LEDs */
#define BRIGHTNESS 20

/* Lenght of notes */
#define BASE      25      /* Milliseconds */
#define WHOLE     BASE*16
#define HALF      BASE*8
#define QUARTER   BASE*4
#define EIGHTH    BASE*2
#define SIXTEENTH BASE

/* Variables for MIDI */
byte MIDIcommand, MIDIdata1, MIDIdata2;
byte MIDIdataBytes=1;
byte MIDIbyteCounter=0;
byte MIDIpacketSize=3;

/* Variables for LEDs */
int x=0;
int randNumber1, randNumber2, randNumber3;

void setup() {
  
  /* Set up WS2812B LEDs */
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);

  /* Red */
  for(int temp=0;temp<BRIGHTNESS;temp++)
  {
    for (x=0;x<8;x++)
    {
      /* Flash the LEDs randombly when we get a MIDI packet */
      leds[x] = CRGB(temp,0,0);
    }
    FastLED.show();
    delay(150);
  }
  
  /* Green */
  for(int temp=0;temp<BRIGHTNESS;temp++)
  {
    for (x=0;x<8;x++)
    {
      /* Flash the LEDs randombly when we get a MIDI packet */
      leds[x] = CRGB(0,temp,0);
    }
    FastLED.show();
    delay(150);
  }
  
  /* Blue */
  for(int temp=0;temp<BRIGHTNESS;temp++)
  {
    for (x=0;x<8;x++)
    {
      /* Flash the LEDs randombly when we get a MIDI packet */
      leds[x] = CRGB(0,0,temp);
    }
    FastLED.show();
    delay(150);
  }
   
  /* Set up serial port */
  /* On the Leonardo, Serial1 is on pins 0 & 1 */
  /* For other Arduino, you may need to set up the port differently */
  Serial1.begin(31250);

  /* Set up USB serial montior on Leonardo */
  Serial.begin(31250);

  /* Outputs notes to MIDI out at startup */
  for (int notes = 36; notes < 72; notes ++) 
    playNote(notes,0x7f,SIXTEENTH);

  delay(500);

  /* Shave and a Haircut */
  playNote(60,0x7f,QUARTER);
  playNote(55,0x7f,EIGHTH);
  playNote(55,0x7f,EIGHTH);
  playNote(56,0x7f,QUARTER);
  playNote(55,0x7f,QUARTER);
  playNote(00,00,QUARTER);
  playNote(59,0x7f,QUARTER);
  playNote(60,0x7f,QUARTER);
  
  /* Clears out any junk that may be in the receive buffer so we start clean */
  Serial1.flush();
}

void playNote(byte note, byte velocity, byte notetime)
{
    MIDIcommand=0x90; /* Note On */
    MIDIdata1=note;  /* Pitch */
    MIDIdata2=velocity;     /* Velocity */
    sendMIDI(3);      /* Send MIDI Bytes */
    delay(notetime);
    MIDIdata2=0;     /* Velocity */
    sendMIDI(3);      /* Send MIDI Bytes */
    delay(notetime);
}

/* Send MIDI Message */
/* From byte variables */
void sendMIDI(byte sendBytes) 
{
  if(sendBytes>=1)
    Serial1.write(MIDIcommand);
  if(sendBytes>=2)
    Serial1.write(MIDIdata1);
  if(sendBytes>=3)
    Serial1.write(MIDIdata2);
}

/* Send MIDI Message */
/* From byte variables */
void printMIDI(byte sendBytes) 
{
  Serial.print("MIDI: 0x");
  if(sendBytes>=1)
  {
    Serial.print(MIDIcommand,HEX);
  }
  if(sendBytes>=2)
  {
    Serial.print(" : 0x");
    Serial.print(MIDIdata1,HEX);
  }
  if(sendBytes>=3)
  {
    Serial.print(" : 0x");
    Serial.print(MIDIdata2,HEX);
  }
  Serial.print("\n");
}

/* Receive MIDI Message */
/* Returns packet size received (1 to 3) */
int recvMIDI(void) 
{
  byte tempByte;

  /* While we have a byte in the serial receive buffer */
  if(Serial1.available())
  { 
    /* Get a byte from the serial port buffer */
    tempByte=Serial1.read();

    /* If this is a command byte */
    if(tempByte & 0x80)
    {
      /* Store command */
      MIDIcommand=tempByte;
      MIDIbyteCounter=1;

      switch(MIDIcommand)
      {
        /* Note Off */
        case 0x80: 
          MIDIdataBytes=2;
          break;

        /* Note On */
        case 0x90: 
          MIDIdataBytes=2; 
          break;

        /* Polymorphic key pressure / Aftertouch */
        case 0xA0: 
          MIDIdataBytes=2; 
          break;

        /* Control change */
        case 0xB0: 
          MIDIdataBytes=2; 
          break;

        /* Program Change */
        case 0xC0: 
          MIDIdataBytes=1;
          break;

        /* Channel pressure / Aftertouch */
        case 0xD0: 
          MIDIdataBytes=1;
          break;

        /* Pitch bend change */
        case 0xE0: 
          MIDIdataBytes=2; 
          break;

        /* System Messages */
        case 0xF0: 

          /* System Real Time */
          if(tempByte&0x08)
          {
            MIDIdataBytes=0; 
            return 1;
          }
          else
          {
            /* System Exclusive */
            /* This needs to be fixed if your system uses this command */
            MIDIdataBytes=2; 
            /* return 1; */
            /* System Common */
          }
          break;
      }
    }
    /* This is a data byte */
    else
    {
      if(MIDIbyteCounter <= MIDIdataBytes)
      {
        /* If one byte has been received (command byte) */
        if(MIDIbyteCounter==1)
        {
          MIDIdata1=tempByte;

          if(MIDIbyteCounter==MIDIdataBytes)
            return 2;
        }
    
        /* If two bytes have been received (command and one data) */
        if(MIDIbyteCounter==2)
        {
          MIDIdata2=tempByte;

          if(MIDIbyteCounter==MIDIdataBytes)
            return 3;
        }
      }
      MIDIbyteCounter++;
    }
  }
  
  /* We don't have a message ready */
  return false;
}

void loop() {
  int temp,temp2;
  temp = recvMIDI();

  /* If we recevied a MIDI packet */
  if(temp>0)
  {
    temp2=MIDIcommand & 0xf0;

    /* Change Middle-C to D */
    /* Example of modifying the stream */
    if ((temp2==0x80)||(temp2==0x90))
      if(MIDIdata1==0x3C)
        MIDIdata1=0x3E;

    /* Send the MIDI and print it on the USB monitor port */
    sendMIDI(temp);
    printMIDI(temp);

    /* Flash the LEDs randombly when we get a MIDI packet */
    for (x=0;x<8;x++)
    {
      randNumber1=random(BRIGHTNESS>>1);
      randNumber2=random(BRIGHTNESS>>1);
      randNumber3=random(BRIGHTNESS>>1);
      leds[x] = CRGB(randNumber1, randNumber2, randNumber3);
    }
    FastLED.show();
  }
}

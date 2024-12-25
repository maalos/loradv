# LoRa Digital Voice Project

## Current hardware
PCB: yet to be released  
MCU: WeMos Lolin32 Lite  
LoRa: Ebyte E220-400M30S  
Battery connector: 3P right angle battery connector  
Battery: BL-5C style  
Display: 1.54" ST7789  
I2S amp: MAX98357(A)  
Speaker: 2W 8Ohm 40mm dia.  
Microphone: INMP441 module  
Rotary encoder: EC11 horizontal, 15mm plum shaft with pushbutton  
SMA connector: "SMA male half"/"SMA-JE 1.6mm" 1.6mm spacing  

## Current software
The software is based on [sh123's library and project](https://github.com/sh123/esp32_codec2_arduino), aiming to become a fully featured LoRa digital voice radio. At it's core it's using David Rowe's Codec2, an awesome, open source, low bitrate voice codec. Currently, the radio supports all codec bitrates, however presets and settings are not yet implemented.

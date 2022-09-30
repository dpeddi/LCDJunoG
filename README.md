
Roland Juno G LCD Emulator

International Display Works  PN: 5485SGPABNC

It was really difficult for me to get working with dma and rpi pico.

To get a quick start i've started trying every example of pio+dma..

I've tested and played with:
rp2040-logic-analyzer.c.txt (Modified by Mark Komus 2021)
picampinos-main.zip (pi-cám-pinos)
pico-tflmicro-main\include\arducam_mic\arducam_mic.c 
code_dmd-master.zip 
Pico-DMX-main.zip 

Finally i've started with pico-dmx, adapted to my needs to get a basic working example, fixed some dynamic dma buffering by taking some idea from arducam_mic.c

Moving liquidsensor

<img src="https://github.com/gtmans/movingliquidsensor/blob/main/waterpinion1.png" width="300" align="left" />
To measure the water level in my dehumidifier’s tank without walking over to check manually, I came up with a solution using a liquid sensor that moves vertically along the outside of the container. I found a useful rack-and-pinion design on Thingiverse, which I adapted for my needs. To achieve a stroke of about 8 cm, I initially used a 360-degree servo, but it proved to be unreliable. I tried enhancing accuracy with an optical angle sensor, but the results weren’t satisfactory.

<img src="https://github.com/gtmans/movingliquidsensor/blob/main/fritzing.png" width="300" align="right" />

<BR><BR><BR><BR><BR><BR><BR><BR>To solve these issues, I switched to a 28BYJ-48 stepper motor controlled with a ULN2003 driver. For processing and display, I used an M5 Atom Lite and an SSD1306 OLED screen. This setup works well but is rather bulky. My next goal is to create a more compact version with a smaller stepper motor, processor, display, and driver.
<BR><BR>
parts:
- 28BYJ-48 steppenmotor with ULN2003 driver
- M5 Atom light
- 0,9" SSD1306 display (128x64)
- microswitch 8x8mm
- watersensor (DFrobot Gravity 5V contacless liquidlevel sensor or DFrobot 3-5v flexible capacative sensor)
- 3D printer parts
  <BR><BR>
  video:
https://youtube.com/shorts/d_gbJdMp8hM?feature=share
  <BR><BR>


I already have a working prototype using a 15mm stepper motor, ESP32-C3 with integrated OLED, and a DRV8833 motor driver. More details on this compact version will be shared later.  <BR><BR> 
<img src="https://github.com/gtmans/movingliquidsensor/blob/main/mini-version.png" width="300" align="left" />

<img src="https://github.com/gtmans/movingliquidsensor/blob/main/mini-version.photo.png" width="300" />



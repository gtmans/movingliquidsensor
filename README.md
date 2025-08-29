Moving liquidsensor

To measure the water level in my dehumidifier’s tank without walking over to check manually, I came up with a solution using a liquid sensor that moves vertically along the outside of the container. I found a useful rack-and-pinion design on Thingiverse, which I adapted for my needs. To achieve a stroke of about 8 cm, I initially used a 360-degree servo, but it proved to be unreliable. I tried enhancing accuracy with an optical angle sensor, but the results weren’t satisfactory.

To solve these issues, I switched to a 28BYJ-48 stepper motor controlled with a ULN2003 driver. For processing and display, I used an M5 Atom Lite and an SSD1306 OLED screen. This setup worked well but was rather bulky. My next goal is to create a more compact version with a smaller stepper motor, processor, display, and driver.

I already have a working prototype using a 15mm stepper motor, ESP32-C3 with integrated OLED, and a DRV8833 motor driver. More details on this compact version will be shared soon.

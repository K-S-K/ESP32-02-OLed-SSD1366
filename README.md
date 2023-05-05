# OLED Display SSD1306 128x32 Example

This project communicates with the SSD1306 128x32 OLED Display ([pdf](docs/SSD1306.pdf)):<br>
![ 128x32 OLED Display view.](docs/display.png)


#### It were some interesting tasks during the project implementation.
- It was challenging to print data to the 128x32 display in the same way we expected after reading about printing to the 128x64 display. If you are interested, please look at the [description of the task and solution](docs/Ch01_LostBits.md).<br>
- Also, it was predictable, but it wasn't expected to face a situation where we couldn't apply the "big" 16x16 font we found on the internet "as is" for our application. It was necessary to transpose every quadrant of every letter in the proper sequence. We even wrote a special program in C# to convert the font to be used for our purposes.

#### The project *in vivo*:<br>
![](docs/oled.gif)


#### The ESP32 Microcontroller pinout:<br>

![The device description.](docs/pinout.png)



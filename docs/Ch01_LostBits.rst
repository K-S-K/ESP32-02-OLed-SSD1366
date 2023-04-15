# Printing the text on the Display

Based on the ([display documentation.pdf](docs/SSD1306.pdf)) we have lines (named "pages") in the screen. Each "page" represented by line with 8 pixel height and 128 pixel width.

We have a [font file](../main/font08x08.h) containing eight bytes for every symbol that our software can display on the screen. It looks like that's enough to prepare a bit array [128x8] bits, that is, an array with 128 bytes, filled with bytes in the places we want to see symbols. Looks good and is understandable. Let's fill the display with white points (just for fun) and then put two lines: " + - / . 0 " over them.

![Half - lines](Ch01_fig01_TwoLines.png)

The result we received is far from expected.
- We have the suspicious narrow lines with a height of 4 pixels instead of 8.
- It looks like the screen contains 8 lines with 4-pixel height instead of 4 lines with 8-pixel height.
- The symbols we have on the screen are corrupted.


Let's take a look onto the font.<br>
![Crazy lines](Ch01_fig02.png)

All looks good. 

Let's make another experiment. Try to print two lines:<br>
"1234"<br>"ABCD":<br>
![Crazy lines](Ch01_fig03.png)

That looks more understandable. Let's take a deeper look at the pixels.:<br>
![Crazy lines](Ch01_fig04.png)

It looks cleaner now. Remember the documentation. It writes only about an 8-line (or 8-page) screen. Not a word about the 4-line screen. It seems like the controller developers developed it for the 8-line screens only, but later, display developers found out how to employ it for the different types of screens.

The controller when works with a 4-line screen, connected to the even points only. It takes not every bit from the byte sent to it, but even ones. And the controller thinks he is still working with 8 line screen, so why not.

So, when we deal with a narrow display, we need to split every byte into even and not even bits and send one half-byte as a byte for the first line and the second half-byte to the second line. We must do it for every couple of screen's half lines, and every couple of half lines on the screen represents one whole display line. So, the controller thinks that it sends two lines to the 8-line screen, but actually, it sends two modified half-lines to the two half-lines of the screen. 

That's how they use one type of controller for the two types of screens. Great decision, isn't it?

Next step, we divide every byte of our 128-byte line of bits, imprinted there according to our input string and font, and, finally, we have a text on the screen that does not look crazy, but it looks exactly as we wanted.

![Expected lines finaly here](Ch01_fig05.png)



<!-- 
rst markup description
https://gist.github.com/javiertejero/4585196
https://gist.github.com/ionelmc/e876b73e2001acd2140f
https://docs.espressif.com/projects/esp-docs/en/latest/introduction/restructuredtext-vs-markdown.html
-->

| Useful links |
| Sub   | Headings | Too  |
+===+=======+==========+======+
| 1 | ESP32 manufacturer community forum about OLED display connections | [Espressif forum brunch: OLED & Graphics Libraries](https://www.esp32.com/viewtopic.php?f=13&t=18656&hilit=128x32)  |
| 2 | Useful example of the dealing with the OLED display I learned from  | [Sample code for driving 128x64 OLED display by Yanbe](https://github.com/yanbe/ssd1306-esp-idf-i2c)    |
| cells | * paragraphs    |
| too   |                 |
+-------+-----------------+

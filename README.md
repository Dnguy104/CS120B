# CS120B

###Authors
Daniel Nguyen
https://github.com/Dnguy104/


Components- 8x8 led matrix, 4 shift registers, 2 joysticks

https://youtu.be/xr-YOfQQzMk 

============

My custom lab project is a 2 player  bomber game. When the game starts, each player is initialized at separate corners of the map. Each player can move up, down, left, and right by use of their joystick, and plant at most 2 bombs by pressing the joystick. The bombs will flash for a few seconds, then explode along the x and y axis. The goal of the game is to get the other player to die before you do. One rule is that players cannot go through bombs, wall, or other players. When a person is hit by a bomb, they will die and the winners color will flash on the screen. After that, the game will reset. 


![Alt text](/pictures/bomber.jpg?raw=true "Optional Title")

===========
## Materials

So tools  that I have borrowed are the code for ADC conversion, and the code for using shift registers.
This project uses 2 joysticks that each have 3 input lines, 1 for the button, and 1 for each axis of the joystick. PortA is used soley for input and ADC conversion. PortB is split into 2 sides that both lead into shift registers. The shift registers allow more to be outputted while using less pins. The shiftregisters on PortB are connected to the blue and red colors on the led matrix. PortC is similar to PortB, in that it leads to 2 shift registers that control the color green, and dataline.



![Alt text](/pictures/diagram.png?raw=true "Optional Title")



To explain the bomb function, If there is less than 2 bombs in play, when the player presses the button, the bombfunction will start and record the location of the player. It will then go through a slow blinking pwm timer, then a fast pwm for the explosion.










##License 
Please read the terms specified in the [LICENSE] file.

[LICENSE]: /LICENSE


##About
This custom shell is made as an assignment for academic reasons.
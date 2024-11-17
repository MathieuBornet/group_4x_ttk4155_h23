GIT :


The code for the two nodes are in each correspondant folder. The node 1 represents the circuit mounted on the breadboard and the node 2 represents the Arduino.

The system has been divided in two with a CAN communication in between. The Arduino monitor the motors/servo with the received data and send a message when the ball cut the IR. The breadbord manages the modes of the state machine (which runs the game). To do this, it manages the joystick and buttons, and displays the menus and messages for the various states of the state machine on the screen.
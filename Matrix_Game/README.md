# BATTLESHIPS   
  
The game is a replica of the well-known classic game battleships, played by two players, where each player  
places some ships of a different size (2/3/4-long) on a map and then they take turns trying to find the other's  
player ships. The ships have to be at least one square away in any direction from each other.  
  
The game is implemented on a 8x8 LED matrix, with an LCD screen used as menu and also to help the steps in game.  
Initially, there will be a menu on the LCD where the players can choose to play, change settings, or see highscores.
  
Link to video showing a demo of the game:  
https://drive.google.com/open?id=1xm-Mvgz8Bos5ng_9Ut5R4y8xqUxfPRsV
  
Link to photo showing the fully assembled hardware:  
https://drive.google.com/open?id=1n0pFE15fYdXcSv68PPpez7jvBPA1W5FM
  
## HARDWARE COMPONENTS  
  
- Arduino UNO board  
- wires  
- resistors  
- capacitors  
- 2 8x8 LED matrices  
- 2 MAX7219 LED control drivers
- 2 joysticks  
- LCD display  
- potentiometer for LCD contrast  
  
## HOW TO PLAY  
  
### MAIN MENU  
  
  The main menu contains 4 options: Play, Options, Highscore and Info.  
  All controls in the main menu are associated to player 1. Player 1 is the one closest to the Arduino board.
  
### OPTIONS

  The options menu allows the players to change the LED matrices brightness (0-15) and to change the game level.  
  The difference between levels consists of a shorter time when shooting (level 0 - 120 seconds, level 1 - 20 seconds,  
  level 2 - 10 seconds) and in a different number of boats that have to be placed (level 0: 2 x 2-long, 2 x 3-long, 1 x 4-long;  
  level 1:3 x 2-long, 1 x 3-long, 1 x 4-long; level 1:4 x 2-long, 1 x 3-long, 0 x 4-long).
  
### HIGHSCORE

  The highscore menu shows the current highscore. It is saved in the eeprom so that it is not deleted when the game is closed.  
  
### INFO
  
  The info menu shows the name of the game, the creator's name and the github link. It also states that the project that the   
  project was made with the help of @unibuc robotics.
  
### PLAY

  Selecting the play starts the game with the selected options. The game is split into two stages, preparation and playing.  
  During one player's turn the other joystick has no effect. While important messages are shown on the LCD (like boat placed  
  or boat positioning incorrect) none of the joysticks have effects.
  
#### PREPARATION

  During the preparation phase each player place their ships in order. Player 1 starts placing his ships first. Moving around the  
  matrix is done using the joystick and selecting a space is done by pressing the joystick button. Spaces can only be selected  
  if they were not already selected and are not unavailable. The LCD shows the current player and says how many ships are left  
  to be placed of the current length. When enough spaces have been selected, the selected positions are checked to see if they can  
  form a boat and an appropriate message is shown. Boats have to be at least 1 space one from another in any direction. While  
  player 2 is placing ships the player 1 matrix shows all his placed ships. At the end of the preparation phase both matrices show  
  all placed boats and the LCD prints a message that says that the game is about to start.  
    
  Preparation phase LED legend:
  - on = occupied(part of a boat)  
  - off = empty
  - blinking fast = cursor
  - blinking slow = unavailable(places around a placed boat)
  
#### PLAYING
  
  The players take turns trying to find each other's boats. Navigating around the matrix is done using the joystick and shooting  
  is done using the joystick buttons. The LCD shows the current player and how many ships the other player has left. When a boat is  
  destroyed the space around it is filled automatically because there cannot be any boats there. If a player hits a part of a boat of  
  that belongs to the other player his timer is reset and he shoots again. The turns end when the timer is over or if the current  
  player misses. A player cannot select a space that was automatically filled or that he already shot.
    
  The game ends when one player remains without any boats and the ending screen is shown.
  
  Game phase LED legend:
  - on = empty/part of a destroyed boat and the spaces around it  
  - off = not yet shot
  - blinking fast = cursor
  - blinking slow = part of a boat hit but not yet destroyed
  
#### ENDING SCREEN

  The ending screen shows the winner and the score that he achieved.  
  Score is calculated depending on total time spent shooting and on game level.  
  If the score is a highscore, an appropriate message is shown and the score is stored as highscore.  
  In the end, the players are presented the option to play again or to go back to main menu. The cursor for selecting one of    
  thes two options will appear after a few seconds so that players do not press the joystick buttons by mistake. Also, only  
  player 1 can control the choice in this menu using joystick, the other joystick having no effect.
  
  
  
  
  
  
  

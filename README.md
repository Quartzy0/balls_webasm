# Balls

Balls is a simple 2D game made in C, compiled to WebASM, and WebGL. The goal of the game is to dodge as many balls as possible.
The game can be player at [https://quartzy.me/balls](https://quartzy.me/balls).
To view a tutorial describing how to play the game, visit [its website](https://quartzy.me/balls) and click on the tutorial button on the main menu.

## Technical details
### Balls spawn rate
The first ball will spawn 0.5 seconds after the player presses the start button.
This point in time is know as the `start_time`.
The equation used to work out how much time should pass until the next ball spawns is as follows:
```
              0.7  
  - start_time     
e             
```

### Ball spawn properties
A ball's radius is minimum 10 pixels and maximum 70.
The position is random and is not closer to the cursor than 100 pixels + the ball's radius.
The speed of a ball is minimum 150 and maximum 550. The ball will always travel in the directon of the mouse cursor at the time when it is spawned.

### Power ups
The chance of a ball becoming a power up is 2%
#### Clearer
The clearer powerup will expand to a radius of 400 pixels at a rate of 10 pixels per frame

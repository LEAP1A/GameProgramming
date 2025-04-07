# CS3113: Intro to Game Programming - James Zhang

Welcome to my CS3113 Intro to Game Programming repository! This repo contains assignments and projects in this class.

## HW1
This project includes loading texture and 2D animation. My program should have met all the requirements of the assignment including extra credits.
A red jet orbits around a stationary blue jet, rotating counterclockwise. Also, a blue jet orbits around a red jet, rotating clockwise. Both moving jets repeatedly scale up and down out of
the extra-credit requirement. The following screenshot shows the pattern.
<img width="1203" alt="image" src="https://github.com/user-attachments/assets/80ffa0f7-fb1c-4a59-afa6-157b4e86290f" />

## HW2
This project is a completed Pong game. All requirements are satisfied including extra credits.
- Start the game by pressing "SPACE". The ball will then be ejected in a random direction. The default game mode is two-player. The left paddle is controlled by "W" and "S", and the right paddle is controlled by "UP" and "DOWN.
  <img width="955" alt="image" src="https://github.com/user-attachments/assets/96767ed5-edd5-469b-aa83-6c013af877f8" />

- By pressing "t", the game will be switched to single-player mode. The left paddle will be uncontrollable and make a simple up and down motion. The right paddle can still be controlled.
- As the ball goes out of the screen, the player on the opposite side will be announced as the winner. A message will be printed on the screen.
  <img width="952" alt="image" src="https://github.com/user-attachments/assets/d2440566-99d6-4aec-b397-adb9ec30f3c1" />

- The number of balls can be changed, ranging from 1 to 3, simply by pressing 1/2/3. However, the winner will be the first player who loses a ball. The winner will not be updated for the second/third ball.
  <img width="959" alt="image" src="https://github.com/user-attachments/assets/ca4a3b23-3825-4d1f-8c21-77c9618882c0" />

## HW3
This project is a simplified lunar lander game. All requirements are satisfied including extra credits.
- By pressing "SPACE", the game will start, and the rocket will start being attracted by the ground (i.e. free fall).
- "W", "A", and "D" are used to ignite your engine while burning fuel. The fuel is initially set to be 2000. After burning out, you cannot control it anymore. Check the remaining fuel in the top right corner frequently!
- Even if there is gravity in this world, there is no air resistance. That means all horizontal acceleration needs to be counteracted by an equal amount of reverse thrust (fuel!!!).
- It is extremely difficult to land on the moving platform, so try to land on the static platform and just consider the moving one as an obstacle you need to avoid...
- Too fast vertical speed / horizontal speed will be an unstable approach, causing failure!
<img width="950" alt="Screenshot 2025-03-18 at 9 23 10 PM" src="https://github.com/user-attachments/assets/6331c527-735b-41da-9cf1-68b441a0284f" />

## HW4
This project is a platformer game containing three levels. All requirements are satisfied except for the extra credit. 

- There is a simple menu page.
- The player has a total of three lives to complete three levels.
- A single wooden box is used as the target
- Jump to hit the bottom of the target to win this level/go to the next level
- The levels are increasing in difficulty as more enemies are included with more tricky hunting logic.
  - The first level simply includes a walker-type enemy doing cycling motions.
  - The second level includes two enemies in guard type. Once the player gets close, they will start chasing the player.
  - The third level includes three enemies, one for flyer type and two for guard type.
- All enemies and the player have moving animations.
- The higher level has a more complicated map.
- The enemies have different textures.
- There is an audio system in this game.
  - A BGM is played once the game starts.
  - Three sound effects are included - jump, death, and win.
<img width="633" alt="image" src="https://github.com/user-attachments/assets/060a8b8d-60a1-4dc4-9c7f-db6ba50aec5d" />
<img width="635" alt="image" src="https://github.com/user-attachments/assets/8bd3f201-d192-41dd-8e64-35cb685fc633" />





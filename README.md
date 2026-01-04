**Touhou Arcade Game**
A high-performance, bare-metal implementation of a bullet-hell style arcade game developed for the Texas Instruments MSP432. This project demonstrates low-level hardware-software integration by managing real-time graphics and complex projectile logic.

**Key Technical Features**
Non-Blocking Engine: Developed a custom C engine that utilizes hardware timers instead of delay loops, ensuring the system remains responsive to user inputs at all times.
Finite State Machine (HSM): Architected an FSM to manage complex game transitions (Menu, Active Gameplay, Collision, Game Over) and deterministic logic updates.
Real-Time Graphics: Synchronized data transfer to the LCD via SPI to maintain fluid frame rates while processing "bullet-hell" projectile patterns.
Bare-Metal Hardware Control: Implemented direct interfacing with the ARM Cortex-M4 peripherals, including ADCs for joystick tracking and PWM for feedback.

**Touhou Arcade Game**
A high-performance, bare-metal implementation of a bullet-hell style arcade game developed for the Texas Instruments MSP432. This project demonstrates low-level hardware-software integration by managing real-time graphics and complex projectile logic without an operating system.

**Key Technical Features**
Non-Blocking Engine: Developed a custom C engine in main.c that utilizes hardware timers instead of delay loops, ensuring the system remains responsive to user inputs at all times.

Finite State Machine (FSM): Architected an FSM to manage complex game transitions (Menu, Active Gameplay, Collision, Game Over) and deterministic logic updates like the player or enemy getting hit by a rocket.

Real-Time Graphics Synchronization: Synchronized data transfer to the LCD via SPI to maintain fluid frame rates while processing various projectile patterns.

Hardware Abstraction Layer (HAL): Implemented a dedicated hal/ directory to isolate peripheral drivers and clock initialization from high-level game logic.

**Software Architecture**
The project follows a modular design to ensure code maintainability and portability:

1. main.c: The core of the application. Contains the primary execution loop, all game logic, collision detection algorithms, and the Finite State Machine.
2. hal/: The Hardware Abstraction Layer. This directory contains all code responsible for initializing hardware clocks and managing low-level drivers for the SPI LCD and ADC-based joystick.
3. applications.h: The central configuration hub. Houses all global variable declarations, function prototypes, and state definitions used throughout the game engine.

**Hardware Stack**
1. Microcontroller: Texas Instruments MSP432 (ARM Cortex-M4).
2. Display: SPI TFT LCD.
3. Input: 2-Axis Analog Joystick.
4. Timing: Internal System Tick Timers for deterministic frame-rate regulation.

**Learning Process:**
I learned to configure the ADC for multi-channel sampling to capture both X and Y axes simultaneously.

Implementation: By setting up the ADC in a repeated-sequence mode and using a hardware trigger, I was able to convert raw analog voltages into digital coordinates. These coordinates are then passed to main.c to update the character’s position within the game state.

Optimization: Partial Refresh vs. Full Screen Refresh
A major challenge in achieving a smooth performance on the MSP432 was the bottleneck of SPI communication with the LCD.

The Problem: Initially, redrawing the entire screen for every frame caused significant flickering and lag because the amount of data exceeded the bus speed during complex projectile patterns.

The Solution: I implemented a partial refresh strategy. Instead of clearing the whole screen, the engine identifies the previous coordinates of moving objects (bullets and player), "deletes" them by drawing background-colored pixels over their old positions, and then draws them at their new coordinates.

Result: This drastically reduced the number of pixels sent over SPI per frame, allowing for fluid motion even with dozens of active projectiles on screen.

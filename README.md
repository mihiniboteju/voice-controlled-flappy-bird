# Voice-Controlled Flappy Bird Game

An embedded, voice-controlled version of the classic Flappy Bird game built using the **STM32F767ZI** microcontroller. The project demonstrates **real-time processing**, **voice-based interaction**, and **graphical rendering** on an embedded platform. Created as part of our 3rd-year Microcontroller course project at King Mongkut's Institute of Technology Ladkrabang (KMITL).

---

## 🎮 Overview

This system enables players to control the “jump” action using **voice commands** captured through a MAX9814 microphone module, supplementing traditional touch input. The goal is to provide a hands-free, accessible gaming experience while showcasing human–machine interaction on STM32.

---

## 🛠️ Hardware Components

- **STM32F767ZI Nucleo Board**
- **MAX9814 Microphone AGC Module**
- **3.5” ILI9486 TFT LCD Display**
- Breadboard & jumper wires  
- 3D-printed casing (for enclosure)


---

## 💻 Software & Firmware

- **Languages:** C
- **IDE:** STM32CubeIDE  
- **Drivers:** STM32 HAL, STM32-ILI9341 driver
- **Protocols:** UART, I2C, SPI, ADC, EXTI  
- **Peripherals:** ADC (audio input), Timers, Interrupts  
- **Graphics:** Real-time rendering on ILI9486 TFT LCD

---

## Block Diagram

The following diagram illustrates the hardware and firmware flow of the project:

![Block Diagram](Docs/block_diagram.png)


## ⭐ Features

- Dual input modes: **Voice** or **Touch**
- Voice-activated jump using MAX9814 microphone
- Dynamic difficulty progression  
- Real-time graphics & smooth rendering  
- Game state handling: start, play, collision, score  
- Score tracking system  
- Audio-responsive gameplay  
- Designed for accessibility and user engagement  

---

## 📐 System Architecture

- MAX9814 audio → ADC sampling  
- Threshold detection triggers jump  
- Timers handle game updates & obstacle movement  
- EXTI for touch input  
- SPI handles LCD communication  
- Non-blocking update loop for smooth rendering  

---

## 🚀 How to Build & Run

1. Open **STM32CubeIDE**  
2. Import the project:  
   **File → Import → Existing Project into Workspace**  
3. Ensure clock configuration matches the STM32F767ZI Nucleo default  
4. Install required ST-Link drivers  
5. Build and flash to the board  
6. Connect the TFT display and microphone as per wiring diagram  
7. Reset board → Game starts automatically

---

## 📸 Demo

Photos and gameplay videos are available in the `/Docs` folder.

---

## 🎯 Purpose

This project was developed for **KMITL microcontroller assignment**, highlighting skills in:

- Embedded C programming  
- Real-time system design  
- Sensor integration
- LCD driver interfacing  
- Interrupt-based input handling  
- Game logic implementation on STM32  

---

## 📄 License

This project is released under the MIT License.


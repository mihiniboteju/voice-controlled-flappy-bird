/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "eth.h"
#include "i2c.h"
#include "rng.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "usb_otg.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ILI9341_STM32_Driver.h"
#include "ILI9341_GFX.h"
#include "ILI9341_Touchscreen.h"
#include "stdio.h"
#include "stdlib.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct {
    float x, y;
    float velocity;
    uint8_t alive;
} Bird;

typedef struct {
    int16_t x;
    uint16_t top_height;
    uint16_t bottom_y;
    uint8_t passed;
} Pipe;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// Game constants
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define BIRD_SIZE 12
#define PIPE_WIDTH 40
//#define PIPE_GAP 100
#define GRAVITY 1
#define JUMP_STRENGTH -4
//#define PIPE_SPEED 2
#define GROUND_HEIGHT 30

#define WINNING_SCORE 8

// Difficulty Level Thresholds
#define DIFFICULTY_LEVEL_2_SCORE 3
#define DIFFICULTY_LEVEL_3_SCORE 5

// Difficulty Parameters
#define LEVEL_1_SPEED 2
#define LEVEL_1_GAP   100
#define LEVEL_2_SPEED 3
#define LEVEL_2_GAP   90
#define LEVEL_3_SPEED 4
#define LEVEL_3_GAP   80
// Colors (RGB565 format)
//#define SKY_BLUE 0x87CE
#define SKY_BLUE 0x867F
#define BIRD_YELLOW 0xFFE0
#define PIPE_GREEN 0x07E0
#define GROUND_BROWN 0x8A22
#define SCORE_WHITE 0xFFFF
#define BLACK 0x0000
#define RED   0xF800
#define YOU_WIN_GREEN 0x07E0
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
// Game variables
Bird bird;
Pipe pipes[2];
uint16_t score = 0;
uint8_t game_state = 0; // 0=menu, 1=playing, 2=game_over, 3=win, 4=level_up
uint32_t last_update = 0;
static uint8_t game_over_drawn = 0;
volatile uint32_t adc_val = 0;
// Shared state for pipe drawing (moved to globals so ResetPipeDrawState can access)
int16_t _last_pipe_x[2] = { -1, -1 };
uint8_t _first_draw = 1;

static uint8_t win_screen_drawn = 0;
static uint8_t level_up_drawn = 0;
uint8_t difficulty_level = 1;
uint8_t current_pipe_speed = LEVEL_1_SPEED;
uint16_t current_pipe_gap = LEVEL_1_GAP;

volatile uint8_t g_touch_detected = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void FlappyBird_Init(void);
void FlappyBird_Update(void);
void FlappyBird_Draw(void);
void FlappyBird_HandleInput(void);
void DrawBird(void);
void DrawPipes(void);
void DrawGround(void);
// void DrawScore(void); // MODIFIED: This function is no longer needed
void DrawMenu(void);
void DrawGameOver(void);
void DrawWinScreen(void);
void DrawLevelUpScreen(void);
void ResetGame(void);
uint8_t CheckCollision(void);
void UpdatePipes(void);
void DrawBackground(uint8_t reset);
void ResetPipeDrawState(void);
// void DrawLevel(void); // MODIFIED: This function is no longer needed
void UpdateHUD(uint8_t force_redraw); // NEW: Efficient HUD update function
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint16_t RGB565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* Enable the CPU Cache */

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ETH_Init();
  MX_USART3_UART_Init();
  MX_USB_OTG_FS_PCD_Init();
  MX_RNG_Init();
  MX_SPI5_Init();
  MX_TIM1_Init();
  MX_I2C1_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
  FlappyBird_Init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
      FlappyBird_Update();
      HAL_Delay(1); // Small delay to prevent busy-waiting at 100% CPU
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 96;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void FlappyBird_Init(void) {
    // Initialize LCD
    ILI9341_Init();
    ILI9341_Set_Rotation(SCREEN_HORIZONTAL_2);

    // Initialize random seed
    srand(HAL_GetTick());

    // Set initial game state to menu
    game_state = 0;
    DrawMenu();
}

void FlappyBird_Update(void) {
    uint32_t current_time = HAL_GetTick();

    // Update at ~30 FPS
    if (current_time - last_update < 33) {
        return;
    }
    last_update = current_time;

    FlappyBird_HandleInput();

    if (game_state == 1) { // Playing
        // Continuous voice control - check ADC for sound input
        HAL_ADC_Start(&hadc1);
        if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) { // Shorter timeout for smoother gameplay
            adc_val = HAL_ADC_GetValue(&hadc1);

            // Voice-controlled jump
            if (adc_val > 3000) {
                bird.velocity = JUMP_STRENGTH; // Jump on voice input
            }
        }
        HAL_ADC_Stop(&hadc1);

        // Update bird physics
        bird.velocity += GRAVITY * 0.3f;
        bird.y += bird.velocity;

        // Update pipes (logic only)
        UpdatePipes();

        // Check collisions
        if (CheckCollision()) {
            game_state = 2; // Game over
        }

        // Check score
        for (int i = 0; i < 2; i++) {
            if (!pipes[i].passed && pipes[i].x + PIPE_WIDTH < bird.x) {
                pipes[i].passed = 1;
                score++;
                // Check for difficulty increase
                if (difficulty_level == 1 && score >= DIFFICULTY_LEVEL_2_SCORE) {
                        difficulty_level = 2;
                        current_pipe_speed = LEVEL_2_SPEED;
                        current_pipe_gap = LEVEL_2_GAP;
                        game_state = 4; // PAUSE for level up
                    }
                else if (difficulty_level == 2 && score >= DIFFICULTY_LEVEL_3_SCORE) {
                        difficulty_level = 3;
                        current_pipe_speed = LEVEL_3_SPEED;
                        current_pipe_gap = LEVEL_3_GAP;
                        game_state = 4; // PAUSE for level up
                    }
            }
        }
        if (score >= WINNING_SCORE) {
                    game_state = 3; // You Win!
                }
    }

    FlappyBird_Draw();
}

void FlappyBird_HandleInput(void) {
    static uint32_t last_touch_time = 0;

    // Check if the interrupt has set our flag
    if (g_touch_detected) {
        // Reset the flag immediately so we don't process the same touch again
        g_touch_detected = 0;

        // We can still use our time-based debounce logic
        uint32_t current_time = HAL_GetTick();
        if (current_time - last_touch_time < 200) {
            return; // Too soon since last touch, ignore
        }
        last_touch_time = current_time;

        uint16_t position_array[2];
        // We still need to read the coordinates to confirm the press
        if (TP_Read_Coordinates(position_array) == TOUCHPAD_DATA_OK) {
            switch (game_state) {
                case 0: // Menu
                    ResetGame();
                    game_state = 1;
                    break;
                case 1: // Playing
                    bird.velocity = JUMP_STRENGTH;
                    break;
                case 2: // Game over
                    game_state = 0;
                    DrawMenu();
                    break;
                case 3: // Win screen
                    game_state = 0;
                    DrawMenu();
                    break;
                case 4: // Level up screen
                    game_state = 1;
                    level_up_drawn = 0;
                    ILI9341_Fill_Screen(SKY_BLUE);
                    DrawGround();
                    UpdateHUD(1);
                    ResetPipeDrawState();
                    DrawBackground(1);
                    break;
            }
        }
    }
}

void FlappyBird_Draw(void) {
    if (game_state == 1) { // Playing
        DrawBackground(0);  // Clears bird's old position
        DrawPipes();        // Redraws moving pipes and clears trailing edges
        DrawBird();         // Draw bird at new position
        UpdateHUD(0);       // Call the efficient HUD update function
    } else if (game_state == 2) { // Game Over
        if (!game_over_drawn) {
            DrawGameOver();
            game_over_drawn = 1;
        }
    } else if (game_state == 3) { // You Win
        if (!win_screen_drawn) {
            DrawWinScreen();
            win_screen_drawn = 1;
        }
    }
    else if (game_state == 4) { // Level Up Screen
            if (!level_up_drawn) {
                DrawLevelUpScreen();
                level_up_drawn = 1;
            }
    }
}

void DrawBackground(uint8_t reset) {
    static int16_t last_bird_x = 50, last_bird_y = 120;
    static uint8_t first_frame = 1;

    if (reset) {
        first_frame = 1;
        return;
    }

    if (!first_frame) {
        // Only clear a small area around bird's previous position
        ILI9341_Draw_Filled_Rectangle_Coord(
            last_bird_x - (BIRD_SIZE / 2) - 2, last_bird_y - (BIRD_SIZE / 2) - 2,
            last_bird_x + (BIRD_SIZE / 2) + 2, last_bird_y + (BIRD_SIZE / 2) + 2,
            SKY_BLUE
        );
    } else {
        first_frame = 0;
    }

    last_bird_x = (int16_t)bird.x;
    last_bird_y = (int16_t)bird.y;
}

void DrawBird(void) {
    int16_t bird_x = (int16_t)bird.x;
    int16_t bird_y = (int16_t)bird.y;

    ILI9341_Draw_Filled_Circle(bird_x, bird_y, BIRD_SIZE/2, BIRD_YELLOW);
    ILI9341_Draw_Filled_Circle(bird_x + 2, bird_y - 1, 1, BLACK); // Eye
}

void DrawPipes(void) {
    for (int i = 0; i < 2; i++) {
        if (!_first_draw && _last_pipe_x[i] > pipes[i].x) {
            int16_t x0_ideal = pipes[i].x + PIPE_WIDTH;
            int16_t x1_ideal = _last_pipe_x[i] + PIPE_WIDTH;

            if (x1_ideal < 0) {
                 _last_pipe_x[i] = pipes[i].x;
                 continue;
            }
            if (x0_ideal < 0) x0_ideal = 0;
            if (x1_ideal > SCREEN_WIDTH) x1_ideal = SCREEN_WIDTH;

            int16_t width = x1_ideal - x0_ideal;

            if (width > 0 && (width % 2 != 0)) {
                if (x0_ideal > 0) {
                    x0_ideal--;
                }
            }

            if (x1_ideal > x0_ideal) {
                ILI9341_Draw_Filled_Rectangle_Coord(x0_ideal, 0, x1_ideal, pipes[i].top_height, SKY_BLUE);
                ILI9341_Draw_Filled_Rectangle_Coord(x0_ideal, pipes[i].bottom_y, x1_ideal, SCREEN_HEIGHT - GROUND_HEIGHT, SKY_BLUE);
            }
        }

        if (pipes[i].x + PIPE_WIDTH > 0 && pipes[i].x < SCREEN_WIDTH) {
            int16_t draw_x0 = pipes[i].x < 0 ? 0 : pipes[i].x;
            int16_t draw_x1 = (pipes[i].x + PIPE_WIDTH > SCREEN_WIDTH) ? SCREEN_WIDTH : (pipes[i].x + PIPE_WIDTH);

            ILI9341_Draw_Filled_Rectangle_Coord(draw_x0, 0, draw_x1, pipes[i].top_height, PIPE_GREEN);
            ILI9341_Draw_Filled_Rectangle_Coord(draw_x0, pipes[i].bottom_y, draw_x1, SCREEN_HEIGHT - GROUND_HEIGHT, PIPE_GREEN);
        }

        _last_pipe_x[i] = pipes[i].x;
    }
    _first_draw = 0;
}

// MODIFIED: This function now ONLY draws the static ground bar. Text is handled by UpdateHUD.
void DrawGround(void) {
    ILI9341_Draw_Filled_Rectangle_Coord(0, SCREEN_HEIGHT - GROUND_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT, GROUND_BROWN);
}

// DELETED: The old DrawScore function is no longer needed.

// NEW FUNCTION: Efficiently updates the HUD text only when values change.
void UpdateHUD(uint8_t force_redraw) {
    static int16_t last_score = -1; // Use -1 to guarantee the first draw
    static uint8_t last_level = 0;   // Use 0 to guarantee the first draw

    if (force_redraw || score != last_score || difficulty_level != last_level) {
        uint16_t ground_top = SCREEN_HEIGHT - GROUND_HEIGHT;

        char level_str[15];
        sprintf(level_str, "Level: %d", difficulty_level);
        char score_str[15];
        sprintf(score_str, "Score: %d", score);

        // Erase the old text by redrawing the ground bar area
        ILI9341_Draw_Filled_Rectangle_Coord(0, ground_top, SCREEN_WIDTH, SCREEN_HEIGHT, GROUND_BROWN);

        // Draw the new text
        ILI9341_Draw_Text(level_str, 10, ground_top + 8, SCORE_WHITE, 2, GROUND_BROWN);
        ILI9341_Draw_Text(score_str, SCREEN_WIDTH - 150, ground_top + 8, SCORE_WHITE, 2, GROUND_BROWN);

        // Update the last known values
        last_score = score;
        last_level = difficulty_level;
    }
}

void DrawMenu(void) {
    ILI9341_Fill_Screen(SKY_BLUE);
    ILI9341_Draw_Text("FLAPPY BIRD", 100, 60, SCORE_WHITE, 2, SKY_BLUE);
    ILI9341_Draw_Text("Touch to Start", 120, 120, SCORE_WHITE, 1, SKY_BLUE);
    ILI9341_Draw_Text("Touch or Scream to Flap", 95, 140, SCORE_WHITE, 1, SKY_BLUE);
    ILI9341_Draw_Filled_Circle(160, 100, BIRD_SIZE, BIRD_YELLOW);
    ILI9341_Draw_Filled_Circle(165, 97, 2, BLACK);
}

void DrawGameOver(void) {
    ILI9341_Draw_Filled_Rectangle_Coord(60, 80, 260, 160, BLACK);
    ILI9341_Draw_Text("GAME OVER!", 80, 90, RED, 3, BLACK);
    char score_str[20];
    sprintf(score_str, "Score: %d", score);
    ILI9341_Draw_Text(score_str, 115, 120, SCORE_WHITE, 2, BLACK);
    ILI9341_Draw_Text("Touch to Restart", 115, 145, SCORE_WHITE, 1, BLACK);
}

void DrawWinScreen(void) {
    ILI9341_Draw_Filled_Rectangle_Coord(60, 80, 260, 160, BLACK);
    ILI9341_Draw_Text("YOU WIN!", 100, 90, YOU_WIN_GREEN, 3, BLACK);
    char score_str[20];
    sprintf(score_str, "Score: %d", WINNING_SCORE);
    ILI9341_Draw_Text(score_str, 115, 120, SCORE_WHITE, 2, BLACK);
    ILI9341_Draw_Text("Touch to Continue", 105, 145, SCORE_WHITE, 1, BLACK);
}

void ResetPipeDrawState(void) {
    _last_pipe_x[0] = -1;
    _last_pipe_x[1] = -1;
    _first_draw = 1;
}

void ResetGame(void) {
	game_over_drawn = 0;
	win_screen_drawn = 0;
	level_up_drawn = 0;
    // Reset bird
    bird.x = 50;
    bird.y = SCREEN_HEIGHT / 2;
    bird.velocity = 0;
    bird.alive = 1;
    //Reset difficulty
    difficulty_level = 1;
	current_pipe_speed = LEVEL_1_SPEED;
	current_pipe_gap = LEVEL_1_GAP;
    // Reset pipes
    for (int i = 0; i < 2; i++) {
        pipes[i].x = SCREEN_WIDTH + (i * (SCREEN_WIDTH / 2 + PIPE_WIDTH / 2));
        pipes[i].top_height = 30 + (rand() % 90);
        pipes[i].bottom_y = pipes[i].top_height + current_pipe_gap;
        pipes[i].passed = 0;
    }

    // Reset score
    score = 0;

    // Reset draw history so no ghost pipes appear
    ResetPipeDrawState();

    // Clear and draw initial frame for a clean start
    ILI9341_Fill_Screen(SKY_BLUE);
    DrawPipes();
    DrawBird();
    // MODIFIED: Draw the static ground bar once, then force the initial HUD text draw.
    DrawGround();
    UpdateHUD(1);
}

void UpdatePipes(void) {
    for (int i = 0; i < 2; i++) {
        pipes[i].x -= current_pipe_speed;

        // Reset pipe when it goes off screen
        if (pipes[i].x < -PIPE_WIDTH) {
            pipes[i].x = SCREEN_WIDTH;
            pipes[i].top_height = 30 + (rand() % 90);
            pipes[i].bottom_y = pipes[i].top_height + current_pipe_gap;
            pipes[i].passed = 0;
        }
    }
}

uint8_t CheckCollision(void) {
    int16_t bird_x = (int16_t)bird.x;
    int16_t bird_y = (int16_t)bird.y;
    int16_t bird_radius = BIRD_SIZE / 2;

    // Ground and ceiling collision
    if (bird_y + bird_radius >= SCREEN_HEIGHT - GROUND_HEIGHT || bird_y - bird_radius <= 0) {
        return 1;
    }

    // Pipe collision
    for (int i = 0; i < 2; i++) {
        if (bird_x + bird_radius > pipes[i].x && bird_x - bird_radius < pipes[i].x + PIPE_WIDTH) {
            if (bird_y - bird_radius < pipes[i].top_height || bird_y + bird_radius > pipes[i].bottom_y) {
                return 1;
            }
        }
    }
    return 0;
}

// DELETED: The old DrawLevel function is no longer needed.

void DrawLevelUpScreen(void) {
    ILI9341_Draw_Filled_Rectangle_Coord(50, 80, 270, 160, BLACK);
    char level_msg[30];

    // Announce the level that was just completed
    sprintf(level_msg, "Level %d Complete!", difficulty_level - 1);
    ILI9341_Draw_Text(level_msg, 60, 95, YOU_WIN_GREEN, 2, BLACK);

    // Announce the next level
    sprintf(level_msg, "Get Ready for Level %d", difficulty_level);
    ILI9341_Draw_Text(level_msg, 95, 120, SCORE_WHITE, 1, BLACK);

    ILI9341_Draw_Text("Touch to Continue", 110, 145, SCORE_WHITE, 1, BLACK);
}
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    // Make sure the interrupt came from the correct pin.
    if (GPIO_Pin == GPIO_PIN_2)
    {
        // Set our global flag to signal that a touch has happened.
        g_touch_detected = 1;
    }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

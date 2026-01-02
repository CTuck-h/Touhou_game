/*
 * Application.h
 *
 *  Created on: Nov 11, 2025
 *      Author: Caleb Tucker
 *  Supervisor: Leyla Nazhand-Ali
 */

#ifndef APPLICATION_H_
#define APPLICATION_H_

#include <ti/grlib/grlib.h>

#include <HAL/HAL.h>
#include <HAL/Timer.h>

#define DIFFICULTY1 1000
#define DIFFICULTY2 2500

#define DEFAULT_NUM 0
#define CENTERSCREEN 64
#define OPTIONSNUM 3
#define CHARACTERSPACE 10
#define CHAR_SIZE 8
#define CHAR_DAMAGE 5

#define P1_ROCKET_NUM 16
#define MAX_ROCKET_NUM 10
#define LAUNCHPOINT 4
#define ROCKETSIZE_X 1
#define ROCKETSIZE_Y 2

#define ENEMYSTART_Y 32
#define PLAYERSTART_Y 116
#define PLAYER_HP 25

#define MAX_SCORE 10000

#define GAMEBOUNDS_LEFT 20
#define GAMEBOUNDS_RIGHT 108
#define LCD_EDGE 128

#define GAMESPEED1 1
#define GAMESPEED2 2
#define GAMESPEED3 3
#define SCORE_STORAGE 5

typedef enum{
    TITLE,
    HOME,
    INSTRUCTION,
    SCORE,
    GAME,
    GAMEOVER
}GameScreen;

struct _Character {

  uint8_t pos_x;
  uint8_t pos_y;
};
typedef struct _Character Character;

struct _Rocket{

    uint8_t x;
    uint8_t y;
    bool moving;

};
typedef struct _Rocket Rocket;


struct _Application {
  // Put your application members and FSM state variables here!
  // =========================================================================
  bool firstCall;
  Graphics_Context gfxContext;//come back

  GameScreen screen;

  unsigned int touhou_time;

  uint8_t cursor;
  uint8_t enemy_pattern;
  unsigned int high_scores[5];

  int player_HP;
  unsigned int player_score;
  int enemy_HP;

  SWTimer Title_timer;
  SWTimer game_timer;

  uint8_t gamespeed;

  Character p1;
  Character enemy;

  Rocket p1_rocket[MAX_ROCKET_NUM];
  Rocket enemy_rocket[MAX_ROCKET_NUM];
  bool pause_check;
  bool one_launch;
  bool one_collision;
  bool firstgame_call;
  bool enemy_left;
  bool enemy_right;

  bool gameover_firstcall;
};
typedef struct _Application Application;

// Called only a single time - inside of main(), where the application is
// constructed
Application Application_construct();

// Called once per super-loop of the main application.
void Application_loop(Application* app, HAL* hal);

// Called whenever the UART module needs to be updated
void Application_updateCommunications(Application* app, HAL* hal);

// Interprets an incoming character and echoes back to terminal what kind of
// character was received (number, letter, or other)
char Application_interpretIncomingChar(char);

// Generic circular increment function
uint32_t CircularIncrement(uint32_t value, uint32_t maximum);
uint32_t CircularDecrement(uint32_t value, uint32_t maximum);

//Displays

void Start_Screen(Application* app_p);
void Main_Menu(Application* app_p, HAL* hal_p);
void Instruction_Screen(Application* app_p, HAL* hal_p);
void Home_Screen(Application* app_p);
void Score_Screen(Application* app_p, HAL* hal_p);
void GameOver_Screen(Application* app_p, HAL* hal_p);
void Victory_Screen(Application* app_p, HAL* hal_p);
void Display_Game(Application* app_p, HAL* hal_p);

//enemy functions
void Display_Enemy_HP(Application* app_p);
void Display_enemy(Application* app_p);
void move_enemy(Application* app_p);

void enemy_vertical(Application* app_p);
void enemy_horizontal(Application* app_p);
void enemy_horizontal_both_sides(Application* app_p);
void enemy_horizontal_right(Application* app_p);
void enemy_horizontal_left(Application* app_p);
void enemy_diagonal(Application* app_p);

void enemy_collision_check(Application* app_p);
void enemy_rocket_launch(Application* app_p);
void delete_enemy_rockets(Application* app_p);
void delete_previous_enemy(Application* app_p);
void change_enemy_pattern(Application* app_p);
void choose_enemy_pattern( Application* app_p);

bool enemy_launch_status(Application* app_p);

//p1 functions
void Display_Player_HP(Application* app_p);
void Display_p1(Application* app_p);
void MoveSquare( bool moveToLeft, bool moveToRight, bool moveToUp, bool moveToDown, Application* app_p);

void P1_Rockets(Application* app_p, HAL* hal_p);
void P1_Rockets_move(Application* app_p);
void P1_Rockets_display(Application* app_p);
void delete_P1_rockets(Application* app_p);
void player_collision_check(Application* app_p);


void Erase_p1(Application* app_p);


void reset_game(Application* app_p, HAL* hal_p);
void update_score(Application* app_p);
void Game_Pause(Application* app_p, HAL* hal_p);
void update_difficulty(Application* app_p);


void initADC();
void JoystickCheck(Application* app_p);


#endif /* APPLICATION_H_ */

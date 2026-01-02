/* DriverLib Includes */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* Standard Includes */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <HAL/LcdDriver/Crystalfontz128x128_ST7735.h>
#include <ti/grlib/grlib.h>
#include <stdlib.h>

/* HAL and Application includes */
#include <Application.h>
#include <HAL/HAL.h>
#include <HAL/Timer.h>
#include <HAL/graphics_HAL.h>


// Non-blocking check. Whenever Launchpad S1 is pressed, LED1 turns on.
static void InitNonBlockingLED() {
  GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
  GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1);
}

// Non-blocking check. Whenever Launchpad S1 is pressed, LED1 turns on.
static void PollNonBlockingLED() {
  GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
  if (GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN1) == 0) {
    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
  }
}


/**
 * The main entry point of your project. The main function should immediately
 * stop the Watchdog timer, call the Application constructor, and then
 * repeatedly call the main super-loop function. The Application constructor
 * should be responsible for initializing all hardware components as well as all
 * other finite state machines you choose to use in this project.
 *
 * THIS FUNCTION IS ALREADY COMPLETE. Unless you want to temporarily experiment
 * with some behavior of a code snippet you may have, we DO NOT RECOMMEND
 * modifying this function in any way.
 */
int main(void) {
  // Stop Watchdog Timer - THIS SHOULD ALWAYS BE THE FIRST LINE OF YOUR MAIN
  WDT_A_holdTimer();

  // Initialize the system clock and background hardware timer, used to enable
  // software timers to time their measurements properly.
  InitSystemTiming();

  // Initialize the main Application object and HAL object
  HAL hal = HAL_construct();
  Application app = Application_construct();

  // Do not remove this line. This is your non-blocking check.
  InitNonBlockingLED();


  // Main super-loop! In a polling architecture, this function should call
  // your main FSM function over and over.
  while (true) {
    // Do not remove this line. This is your non-blocking check.
    PollNonBlockingLED();
    HAL_refresh(&hal);
    Application_loop(&app, &hal);
  }
}




/**
 * A helper function which increments a value with a maximum. If incrementing
 * the number causes the value to hit its maximum, the number wraps around
 * to 0.
 */
uint32_t CircularIncrement(uint32_t value, uint32_t maximum) {
  return (value + 1) % maximum;
}
/**
 * A helper function which Decrements a value with a maximum. If decrementing
 * the number causes the value to hit a negative number, the number wraps around
 * to 2.
 */
uint32_t CircularDecrement(uint32_t value, uint32_t maximum) {
    if(value == 0)
        return 2;
  return (value - 1) % maximum;
}
/**
 * The main constructor for your application. This function should initialize
 * each of the FSMs which implement the application logic of your project.
 *
 * @return a completely initialized Application object
 */
Application Application_construct() {
  Application app;

  // Initialize local application state variables here!
  app.firstCall = true;

  //Initialize game variables

  //home screen variables
  app.cursor = DEFAULT_NUM;//refers to the cursor

  app.screen = TITLE;
  app.Title_timer = SWTimer_construct(3000);
  SWTimer_start(&app.Title_timer);

  app.touhou_time = DEFAULT_NUM;

  app.enemy_HP = PLAYER_HP;
  app.player_HP = PLAYER_HP;
  app.player_score = DEFAULT_NUM;
  // initializes the high score variables and sets them as 0 until later updated
  int scorecount;

  for(scorecount = DEFAULT_NUM; scorecount < SCORE_STORAGE; scorecount++)
  app.high_scores[scorecount] = DEFAULT_NUM;

  app.game_timer = SWTimer_construct(DEFAULT_NUM);
  app.gamespeed = GAMESPEED1;
  app.pause_check = false;
  int rocketcount;
  for(rocketcount = DEFAULT_NUM; rocketcount < MAX_ROCKET_NUM; rocketcount++)
  app.p1_rocket[rocketcount].moving = false;

  app.p1.pos_x = CENTERSCREEN;
  app.p1.pos_y = PLAYERSTART_Y;

  for(rocketcount = DEFAULT_NUM; rocketcount < MAX_ROCKET_NUM; rocketcount++)
  app.p1_rocket[rocketcount].moving = false;

  app.enemy.pos_x = CENTERSCREEN;
  app.enemy.pos_y = ENEMYSTART_Y;

  app.enemy_pattern = DEFAULT_NUM;

  app.one_collision = false;
  app.one_launch = false;

  app.gameover_firstcall = true;
  app.firstgame_call = true;
  app.enemy_left = true;
  app.enemy_right = false;
  // Initialize the graphics context (g_sCrystalfontz128x128 is the LCD screen)
      Graphics_initContext(&app.gfxContext, &g_sCrystalfontz128x128, &g_sCrystalfontz128x128_funcs);

      // Set the initial appearance
      Graphics_setFont(&app.gfxContext, &g_sFontFixed6x8);
      Graphics_setForegroundColor(&app.gfxContext, GRAPHICS_COLOR_WHITE);
      Graphics_setBackgroundColor(&app.gfxContext, GRAPHICS_COLOR_BLACK);

      // Clear the screen once at startup
      Graphics_clearDisplay(&app.gfxContext);

       initADC();
       initJoyStick();
       startADC();

      Start_Screen(&app);

  return app;
}




/**
 * The main super-loop function of the application. We place this inside of a
 * single infinite loop in main. In this way, we can model a polling system of
 * FSMs. Every cycle of this loop function, we poll each of the FSMs one time,
 * followed by refreshing all inputs to the system through a convenient
 * [HAL_refresh()] call.
 *
 * @param app_p:  A pointer to the main Application object.
 * @param hal_p:  A pointer to the main HAL object
 */
void Application_loop(Application* app_p, HAL* hal_p) {
  // Restart/Update communications if either this is the first time the
  // application is run or if BoosterPack S2 is pressed (which means a new
  // baudrate is being set up)

    if (Button_isTapped(&hal_p->launchpadS2) || app_p->firstCall) {
      Application_updateCommunications(app_p, hal_p);
    }


    switch (app_p->screen) {
            case TITLE:
                if(SWTimer_expired(&app_p->Title_timer))
                {
                    app_p->screen = HOME;
                    Home_Screen(app_p);
                }
                else
                    Start_Screen(app_p);
                break;
            case HOME:
                Main_Menu(app_p, hal_p);
                break;
            case INSTRUCTION: //calls the instruction screen when
                Instruction_Screen(app_p, hal_p);
                break;
            case SCORE:
                Score_Screen(app_p, hal_p);
                break;
            case GAME:
                if(app_p->firstgame_call)
                {
                    Graphics_clearDisplay(&app_p->gfxContext);
                    reset_game(app_p, hal_p);
                    app_p->firstgame_call = false;
                    SWTimer_start(&app_p->game_timer);
                }
                Game_Pause(app_p, hal_p);
                break;
            case GAMEOVER:
                GameOver_Screen(app_p, hal_p);
                break;
        }
}

/**
 * @param app_p:  A pointer to the main Application object.
 * @param hal_p:  A pointer to the main HAL object
 */
void Application_updateCommunications(Application* app_p, HAL* hal_p) {
  // When this application first loops, the proper LEDs aren't lit. The
  // firstCall flag is used to ensure that the
  if (app_p->firstCall) {
    app_p->firstCall = false;
  }

}


void Main_Menu(Application* app_p, HAL* hal_p)
{
    //moves the cursor if BB2 is pressed in the homescreen
    if (Button_isTapped(&hal_p->boosterpackS2) && app_p->screen == HOME) {

        app_p->cursor = CircularIncrement(app_p->cursor, OPTIONSNUM);
        Home_Screen(app_p);
    }
    else if (Button_isTapped(&hal_p->boosterpackS1) && app_p->screen == HOME) {

        app_p->cursor = CircularDecrement(app_p->cursor, OPTIONSNUM);
        Home_Screen(app_p);
    }
    //changes the screen to the selected option from the home screen
    if (Button_isTapped(&hal_p->boosterpackJS)) {

        switch (app_p->cursor) {
        //starts the game and initializes the values of the in game variables
        case 0:
                SWTimer_start(&app_p->game_timer);
                // Draw the initial game screen UI (placeholder for now)
                Display_Game(app_p, hal_p);
                break;

            case 1: //displays the Instructions screen and changes t
                app_p->screen = INSTRUCTION;

                Instruction_Screen(app_p,hal_p);
                break;

            case 2: //displays the High Score screen
                app_p->screen = SCORE;

                Score_Screen(app_p,hal_p);
                break;
               }
        }
    }

//Displays the home screen of the game that holds the options start game, instructions, and scores with a cursor that points on
//the current option
void Home_Screen(Application* app_p)
{
    int8_t g_pcPrintStart[32];

          Graphics_clearDisplay(&app_p->gfxContext);

          //prints menu on the top left of the screen
          sprintf((char *)g_pcPrintStart, "MENU");
          Graphics_drawString(&app_p->gfxContext, (int8_t*) g_pcPrintStart, -1, 10, 10, true);

          //prints the options in the main menu
          sprintf((char *)g_pcPrintStart, "Play Game");
          Graphics_drawString(&app_p->gfxContext, (int8_t*) g_pcPrintStart, -1, 20, 50, true);

          sprintf((char *)g_pcPrintStart, "Instructions");
          Graphics_drawString(&app_p->gfxContext, (int8_t*) g_pcPrintStart, -1, 20, 60, true);

          sprintf((char *)g_pcPrintStart, "High Scores");
          Graphics_drawString(&app_p->gfxContext, (int8_t*) g_pcPrintStart, -1, 20, 70, true);


          //Redraws the cursor to point at the current option in the home screen
          if(app_p->cursor == 0){//points the cursor at play game
              Graphics_drawLine(&app_p->gfxContext, 10, 50, 13, 53);
              Graphics_drawLine(&app_p->gfxContext, 10, 56, 13, 53);
          }
          else if(app_p->cursor == 1){//points the cursor at instructions
              Graphics_drawLine(&app_p->gfxContext, 10, 60, 13, 63);
              Graphics_drawLine(&app_p->gfxContext, 10, 66, 13, 63);
          }
          else if(app_p->cursor == 2){//points the cursor at high scores
              Graphics_drawLine(&app_p->gfxContext, 10, 70, 13, 73);
              Graphics_drawLine(&app_p->gfxContext, 10, 76, 13, 73);
          }

          return;
}
//prints the Start screen that shows up for the first three seconds of the application
void Start_Screen(Application* app_p){

    int8_t g_pcPrintStart[32];

      sprintf((char *)g_pcPrintStart, "ECE2564");
      Graphics_drawStringCentered(&app_p->gfxContext, (int8_t*) g_pcPrintStart, -1, CENTERSCREEN, CHARACTERSPACE , true);

      sprintf((char *)g_pcPrintStart, "Project 2");
      Graphics_drawStringCentered(&app_p->gfxContext, (int8_t*) g_pcPrintStart, -1,  CENTERSCREEN, CHARACTERSPACE * 2, true);

      sprintf((char *)g_pcPrintStart, "Caleb Tucker");
      Graphics_drawStringCentered(&app_p->gfxContext, (int8_t*) g_pcPrintStart, -1,  CENTERSCREEN, CHARACTERSPACE * 3, true);

      sprintf((char *)g_pcPrintStart, "--------------");
      Graphics_drawStringCentered(&app_p->gfxContext, (int8_t*) g_pcPrintStart, -1,  CENTERSCREEN, CHARACTERSPACE * 4, true);

      sprintf((char *)g_pcPrintStart, "Touhou");
      Graphics_drawStringCentered(&app_p->gfxContext, (int8_t*) g_pcPrintStart, -1, CENTERSCREEN, CHARACTERSPACE * 5, true);

}

//displays the instruction screen
void Instruction_Screen(Application* app_p, HAL* hal_p){

    //if BB2 is pressed go back to the home screen
    if (Button_isTapped(&hal_p->boosterpackS2))
       {
        app_p->screen = HOME;
        Home_Screen(app_p);
       }

    if (Button_isTapped(&hal_p->boosterpackJS))
    {//makes sure that the screen is only changed when JS was pressed to select the option
        Graphics_setForegroundColor(&app_p->gfxContext, GRAPHICS_COLOR_WHITE);

        Graphics_clearDisplay(&app_p->gfxContext);

        //initializes the instructions to be displayed on screen
        const char* instructionLines[] = {
            "Move the character",
            "to avoid being hit by",
            "the bullets.",
            "Press BB1 to fire at",
            "the enemy.",
            "First one to run out",
            "health loses.",
        };

        const int instructionY[] = {20,30,40,56, 66, 81, 91 };
        const int numInstructions = sizeof(instructionLines) / sizeof(instructionLines[0]);

        Graphics_drawStringCentered(&app_p->gfxContext, (int8_t*) "Game Instructions", -1, 64, 10, true);

        int count = 0;
        // prints the instructions
        while(count < numInstructions) {
            Graphics_drawString(&app_p->gfxContext, (int8_t*)instructionLines[count], -1, 0, instructionY[count], true);
            count++;
        }

        // Back Instruction, displays how to go back to the home screen
        Graphics_drawString(&app_p->gfxContext, (int8_t*)"BB2: Go Back", -1, 0, 118, true);
    }
}

//displays the 5 highest scores from the game
// the 5 highest scores consist of the fasted completed games
void Score_Screen(Application* app_p, HAL* hal_p){
    //if BB2 is pressed go back to the home screen
    if (Button_isTapped(&hal_p->boosterpackS2))
       {
        app_p->screen = HOME;
        Home_Screen(app_p);
       }
    //makes sure that the screen is only changed when JS was pressed to select the option
    if (Button_isTapped(&hal_p->boosterpackJS))
    {
        char scores[32];

                Graphics_setForegroundColor(&app_p->gfxContext, GRAPHICS_COLOR_WHITE);

                Graphics_clearDisplay(&app_p->gfxContext);

                Graphics_drawString(&app_p->gfxContext, "High Scores", -1, 10, 10, OPAQUE_TEXT);

                Graphics_drawString(&app_p->gfxContext, (int8_t*) "--------------", -1, 0, 20, true);

                snprintf(scores, 20, "%d", app_p->high_scores[0]);
                Graphics_drawStringCentered(&app_p->gfxContext, (int8_t*)scores, -1, 15, 30, OPAQUE_TEXT);

                snprintf(scores, 20, "%d", app_p->high_scores[1]);
                Graphics_drawStringCentered(&app_p->gfxContext, (int8_t*)scores, -1, 15, 40, OPAQUE_TEXT);

                snprintf(scores, 20, "%d", app_p->high_scores[2]);
                Graphics_drawStringCentered(&app_p->gfxContext, (int8_t*)scores, -1, 15, 50, OPAQUE_TEXT);

                snprintf(scores, 20, "%d", app_p->high_scores[3]);
                Graphics_drawStringCentered(&app_p->gfxContext, (int8_t*)scores, -1, 15, 60, OPAQUE_TEXT);

                snprintf(scores, 20, "%d", app_p->high_scores[4]);
                Graphics_drawStringCentered(&app_p->gfxContext, (int8_t*)scores, -1, 15, 70, OPAQUE_TEXT);

                // Back Instruction
                Graphics_drawString(&app_p->gfxContext, (int8_t*)"Press BB2 to return", -1, 0, 118, true);
    }
}
void GameOver_Screen(Application* app_p, HAL* hal_p)
{
    //if BB2 is pressed go back to the home screen
    if (Button_isTapped(&hal_p->boosterpackJS))
       {
        Graphics_clearDisplay(&app_p->gfxContext);//change it so it only clears the play field

        reset_game(app_p, hal_p);
        Home_Screen(app_p);
        return;
       }
    //makes sure that the screen is only changed when BB1 is called the first time
    if (app_p->gameover_firstcall == true)
    {
        app_p->screen = GAMEOVER;
        app_p->gameover_firstcall = false;
        char overString[20];
        Graphics_setForegroundColor(&app_p->gfxContext, GRAPHICS_COLOR_WHITE);

        Graphics_drawString(&app_p->gfxContext, (int8_t*)"You Lose =(", -1, CENTERSCREEN - 20, CENTERSCREEN, true);


        snprintf(overString, 20, "Score: %d", app_p->player_score);
        Graphics_drawString(&app_p->gfxContext, (int8_t*)overString, -1,0, 110, OPAQUE_TEXT);
        Graphics_drawString(&app_p->gfxContext, (int8_t*)"Press JSB to continue", -1, 0, 118, true);

    }
}

void reset_game(Application* app_p, HAL* hal_p)
{
    app_p->player_HP = PLAYER_HP;
    app_p->p1.pos_y = PLAYERSTART_Y;
    app_p->p1.pos_x = CENTERSCREEN;

    app_p->enemy_HP = PLAYER_HP;
    app_p->enemy.pos_y = ENEMYSTART_Y;
    app_p->enemy.pos_x = CENTERSCREEN;

    app_p->game_timer = SWTimer_construct(DEFAULT_NUM);;
    app_p->player_score = DEFAULT_NUM;
    app_p->firstgame_call = true;
    int rocketcount;
    for(rocketcount = DEFAULT_NUM; rocketcount < MAX_ROCKET_NUM; rocketcount++)
        app_p->p1_rocket[rocketcount].moving = false;



    int bullet_index;
    for(bullet_index = DEFAULT_NUM; bullet_index < MAX_ROCKET_NUM; bullet_index++)
        app_p->enemy_rocket[bullet_index].moving = false;

    app_p->enemy_pattern = DEFAULT_NUM;

    app_p->gameover_firstcall = true;
    app_p->screen = HOME;
    return;
}
void update_score(Application* app_p)
{
    int i;
    int j;

    // 1. Find the correct insertion index (i)
    // Loop through the high scores to find the first score that is less than the player's new score.
    for(i = DEFAULT_NUM; i < SCORE_STORAGE; i++){
        if(app_p->high_scores[i] < app_p->player_score ){
            // Insertion point found at index i.

            // 2. SHIFT: Move all scores from index i downward by one position.
            // Start from the last index (SCORE_STORAGE - 1) and move backward to i + 1.
            // This ensures we move the data safely without duplication.
            for(j = SCORE_STORAGE - 1; j > i; j--)
                app_p->high_scores[j] = app_p->high_scores[j-1];

            // 3. INSERT: Place the new score in the now-open spot at index i.
            app_p->high_scores[i] = app_p->player_score;

            // 4. Stop: We found the spot and inserted the score, so exit the main loop.
            break;
        }
    }//end for

}
void Victory_Screen(Application* app_p, HAL* hal_p)
{
    //if BB2 is pressed go back to the home screen
    if (Button_isTapped(&hal_p->boosterpackJS))
       {
        Graphics_clearDisplay(&app_p->gfxContext);//change it so it only clears the play field
        reset_game(app_p, hal_p);
        Home_Screen(app_p);
        return;
       }
    //makes sure that the screen is only changed when BB1 is called the first time
    if (app_p->gameover_firstcall == true)
    {
        update_score(app_p);

        app_p->screen = GAMEOVER;
        app_p->gameover_firstcall = false;
        char overString[20];
        Graphics_setForegroundColor(&app_p->gfxContext, GRAPHICS_COLOR_WHITE);

        Graphics_drawString(&app_p->gfxContext, (int8_t*)"You Win =)", -1, CENTERSCREEN - 20, CENTERSCREEN, true);


        snprintf(overString, 20, "Score: %d", app_p->player_score);
        Graphics_drawString(&app_p->gfxContext, (int8_t*)overString, -1,0, 110, OPAQUE_TEXT);
        Graphics_drawString(&app_p->gfxContext, (int8_t*)"Press JSB to continue", -1, 0, 118, true);

    }
}
//pauses the game if bb2 has been pressed
void Game_Pause(Application* app_p, HAL* hal_p)
{
    if (Button_isTapped(&hal_p->boosterpackS2))
       {// pauses the game
        app_p->pause_check = !app_p->pause_check;

        Graphics_clearDisplay(&app_p->gfxContext);

       }

    if(app_p->pause_check == false)
        Display_Game(app_p, hal_p);
    else
    {//displays the pause screen
        Graphics_setForegroundColor(&app_p->gfxContext, GRAPHICS_COLOR_WHITE);
        Graphics_drawString(&app_p->gfxContext, (int8_t*)"Game Paused", -1, CENTERSCREEN - 20, CENTERSCREEN, true);
        Graphics_drawString(&app_p->gfxContext, (int8_t*)"Press BB2 to unpause", -1, 0, 118, true);
        return;
    }
}
//updates the gamespeed based on the amount of time passed, making it more difficult to play
void update_difficulty(Application* app_p)
{
    if(app_p->touhou_time < DIFFICULTY1)
        app_p->gamespeed = GAMESPEED1;
    else if(app_p->touhou_time < DIFFICULTY2 && app_p->touhou_time > DIFFICULTY1)
        app_p->gamespeed = GAMESPEED2;
    else
        app_p->gamespeed = GAMESPEED3;
}
void Display_Game(Application* app_p, HAL* hal_p)
{
    update_difficulty(app_p);

    app_p->screen = GAME;
    //displays the player and its HP as well as the enemy's HP
    Display_p1(app_p);
    Display_Player_HP(app_p);
    Display_Enemy_HP(app_p);

    app_p->touhou_time = SWTimer_elapsedCycles(&app_p->game_timer)/500000;

    if(app_p->player_HP <= DEFAULT_NUM)
    {
        GameOver_Screen(app_p, hal_p);
        return;
    }
    if(app_p->enemy_HP <= DEFAULT_NUM)
    {
        app_p->player_score = 10000-app_p->touhou_time ;
        Victory_Screen(app_p, hal_p);
        return;
    }

    char timerString[20];
    Graphics_setForegroundColor(&app_p->gfxContext, GRAPHICS_COLOR_WHITE);

    snprintf(timerString, 20, "Time: %d", app_p->touhou_time);
    Graphics_drawStringCentered(&app_p->gfxContext, (int8_t*)timerString, -1,30, 10, OPAQUE_TEXT);

    //checks to see if the joystick has been moved and if so moves the character
    JoystickCheck(app_p);
    //moves the enemy
    move_enemy(app_p);

    //checks to see if the rocket has been launched and if so moves it up the screen
    P1_Rockets(app_p, hal_p);
    P1_Rockets_move(app_p);
    P1_Rockets_display(app_p);

    choose_enemy_pattern( app_p);

    //checks if the player or the enemy has been hit by a rocker
    player_collision_check(app_p);
    enemy_collision_check(app_p);
}
//chooses the enemy's pattern on a cyclic basis
void choose_enemy_pattern( Application* app_p)
{
    if(app_p->enemy_pattern == 0)
        enemy_vertical(app_p);
    else if(app_p->enemy_pattern == 1)
        enemy_horizontal(app_p);
    else if(app_p->enemy_pattern == 2)
        enemy_horizontal_both_sides(app_p);
    else if(app_p->enemy_pattern == 3)
        enemy_horizontal_left(app_p);
    else if(app_p->enemy_pattern == 4)
        enemy_horizontal_right(app_p);
    else
        enemy_diagonal( app_p);
}
//displays the player's HP
void Display_Player_HP(Application* app_p)
{
   Graphics_setForegroundColor(&app_p->gfxContext, GRAPHICS_COLOR_WHITE);

   char healthString[20];
   snprintf(healthString, 20, "%d", app_p->player_HP);

   Graphics_setForegroundColor(&app_p->gfxContext, GRAPHICS_COLOR_BLUE);
   Graphics_fillCircle(&app_p->gfxContext, 10, 118, 8);

   Graphics_setForegroundColor(&app_p->gfxContext, GRAPHICS_COLOR_WHITE);
   Graphics_setBackgroundColor(&app_p->gfxContext, GRAPHICS_COLOR_BLACK);

   Graphics_drawStringCentered(&app_p->gfxContext, (int8_t*)healthString, -1, 11, 118, OPAQUE_TEXT);
   Graphics_setBackgroundColor(&app_p->gfxContext, GRAPHICS_COLOR_BLACK);

}
//displays the enemy's HP
void Display_Enemy_HP(Application* app_p)
{
    Graphics_setForegroundColor(&app_p->gfxContext, GRAPHICS_COLOR_WHITE);

    char healthString[20];
    snprintf(healthString, 20, "%d", app_p->enemy_HP);

    Graphics_setForegroundColor(&app_p->gfxContext, GRAPHICS_COLOR_RED);
    Graphics_fillCircle(&app_p->gfxContext, 118, 8, 8);

    Graphics_setForegroundColor(&app_p->gfxContext, GRAPHICS_COLOR_WHITE);
    Graphics_setBackgroundColor(&app_p->gfxContext, GRAPHICS_COLOR_BLACK);

    Graphics_drawStringCentered(&app_p->gfxContext, (int8_t*)healthString, -1, 118, 8, OPAQUE_TEXT);
    Graphics_setBackgroundColor(&app_p->gfxContext, GRAPHICS_COLOR_BLACK);

}
//displays the characters current frame
void Display_p1(Application* app_p)
{
    Graphics_Rectangle character;

    character.xMax = app_p->p1.pos_x;
    character.xMin = app_p->p1.pos_x - CHAR_SIZE;
    character.yMin = app_p->p1.pos_y - CHAR_SIZE;
    character.yMax = app_p->p1.pos_y;

    Graphics_setForegroundColor(&app_p->gfxContext, GRAPHICS_COLOR_BLUE);
    Graphics_fillRectangle(&app_p->gfxContext, &character);
}
//deletes the previous frame of the player
void Erase_p1(Application* app_p)
{
    Graphics_Rectangle oldcharacter;

    oldcharacter.xMax = app_p->p1.pos_x;
    oldcharacter.xMin = app_p->p1.pos_x - CHAR_SIZE;
    oldcharacter.yMin = app_p->p1.pos_y - CHAR_SIZE;
    oldcharacter.yMax = app_p->p1.pos_y;
    Graphics_setForegroundColor(&app_p->gfxContext, GRAPHICS_COLOR_BLACK);
    Graphics_fillRectangle(&app_p->gfxContext, &oldcharacter);
}

void JoystickCheck(Application* app_p)
{
    unsigned vx, vy;

           //checks the threshold of the joystick
           getSampleJoyStick(&vx, &vy);

           //checks for joystick movement in the x axis
           bool joyStickPushedtoRight = false;
           bool joyStickPushedtoLeft = false;
           if (vx < LEFT_THRESHOLD)
           {
               joyStickPushedtoLeft = true;
           }
           if (vx > RIGHT_THRESHOLD)
           {
               joyStickPushedtoRight = true;
           }

           //checks for joystick movement in the y axis
           bool joyStickPushedtoUp = false;
           bool joyStickPushedtoDown = false;
           if (vy > UP_THRESHOLD)
           {
               joyStickPushedtoUp = true;
           }
           if (vy < DOWN_THRESHOLD)
           {
               joyStickPushedtoDown = true;
           }
           // ------------------------
           MoveSquare(      joyStickPushedtoLeft,
                            joyStickPushedtoRight,
                            joyStickPushedtoUp,
                            joyStickPushedtoDown,
                            app_p);
           return;
}
void MoveSquare(bool moveToLeft, bool moveToRight, bool moveToUp, bool moveToDown, Application* app_p)
{

    bool moved = false; // checks if a move occurred

    // Check for X-axis movement and boundaries (20 < x < 110)
    if (moveToLeft && (app_p->p1.pos_x > 20))
    {
        // Erase old square
        Erase_p1(app_p);

        app_p->p1.pos_x -= app_p->gamespeed;
        moved = true;
    }
    else if (moveToRight && (app_p->p1.pos_x < 110))
    {
        // Erase old square
        Erase_p1(app_p);

        app_p->p1.pos_x += app_p->gamespeed;
        moved = true;
    }

    // Check for Y-axis movement and boundaries (0 < y < 128)
    if (moveToUp && (app_p->p1.pos_y - CHAR_SIZE > 0))
    {
        // Erase old square if
            Erase_p1(app_p);

            app_p->p1.pos_y -= app_p->gamespeed;
        moved = true;
    }

    else if (moveToDown && (app_p->p1.pos_y < 128)) // R.yMax (96) - circle radius (10) = 86
    {
        // Erase old square if not already erased
        if(!moved)
            Erase_p1(app_p);

            app_p->p1.pos_y += app_p->gamespeed;
        moved = true;
    }
    // Redraw the user in its new postion
    if (moved)
        Display_p1(app_p);

    return;
}
//fires an inactive rocket if the user prompts it
void P1_Rockets(Application* app_p, HAL* hal_p)
{
    if (Button_isTapped(&hal_p->boosterpackS1))
        {
            uint8_t i;
            for (i = 0; i < MAX_ROCKET_NUM; i++)
            {
                // Find the first available "inactive" bullet slot in the array
                if (!app_p->p1_rocket[i].moving)
                {
                    // Found an inactive bullet slot, activate it
                    app_p->p1_rocket[i].moving = true;

                    // Launch the bullet from the center of the player
                    app_p->p1_rocket[i].x = app_p->p1.pos_x - LAUNCHPOINT;
                    app_p->p1_rocket[i].y = app_p->p1.pos_y - LAUNCHPOINT;

                    return;
                }
            }
        }
}

//clears the rockets previous frame or deletes it from the screen
void delete_P1_rockets(Application* app_p)
{
    uint8_t i;

    for(i=0; i < MAX_ROCKET_NUM  ; i++)
    {
        if(app_p->p1_rocket[i].moving)
        {
            Graphics_Rectangle rocket;

            rocket.xMax = app_p->p1_rocket[i].x;
            rocket.xMin = app_p->p1_rocket[i].x - ROCKETSIZE_X;
            rocket.yMax = app_p->p1_rocket[i].y;
            rocket.yMin = app_p->p1_rocket[i].y + ROCKETSIZE_Y;

            Graphics_setForegroundColor(&app_p->gfxContext, GRAPHICS_COLOR_BLACK);
            Graphics_fillRectangle(&app_p->gfxContext, &rocket);
        }
    }
    return;
}
//changes the value of the players rocket so that it moves up the screen towards the enemy
void P1_Rockets_move(Application* app_p)
{
    int i;
    delete_P1_rockets(app_p);
    for(i=0; i < MAX_ROCKET_NUM; i++)
    {
        if(app_p->p1_rocket[i].moving)
        app_p->p1_rocket[i].y -= app_p->gamespeed;;

        if(app_p->p1_rocket[i].y < 3)
            app_p->p1_rocket[i].moving = false;
    }
}
//displays the players rockets
void P1_Rockets_display(Application* app_p)
{
    int i;

    for(i=0; i < MAX_ROCKET_NUM; i++)
    {
        if(app_p->p1_rocket[i].moving)
        {
            Graphics_Rectangle rocket;

            rocket.xMax = app_p->p1_rocket[i].x;
            rocket.xMin = app_p->p1_rocket[i].x - ROCKETSIZE_X;
            rocket.yMax = app_p->p1_rocket[i].y;
            rocket.yMin = app_p->p1_rocket[i].y + ROCKETSIZE_Y;

            Graphics_setForegroundColor(&app_p->gfxContext, GRAPHICS_COLOR_WHITE);
            Graphics_fillRectangle(&app_p->gfxContext, &rocket);
        }

    }
}

//displays the enemy's current position
void Display_enemy(Application* app_p)
{
    Graphics_Rectangle character;

    character.xMax = app_p->enemy.pos_x;
    character.xMin = app_p->enemy.pos_x - CHAR_SIZE;
    character.yMin = app_p->enemy.pos_y - CHAR_SIZE;
    character.yMax = app_p->enemy.pos_y;

    Graphics_setForegroundColor(&app_p->gfxContext, GRAPHICS_COLOR_RED);
    Graphics_fillRectangle(&app_p->gfxContext, &character);
}
//clears the enemy's previous frame
void delete_previous_enemy(Application* app_p)
{
    Graphics_Rectangle character;

    character.xMax = app_p->enemy.pos_x;
    character.xMin = app_p->enemy.pos_x - CHAR_SIZE;
    character.yMin = app_p->enemy.pos_y - CHAR_SIZE;
    character.yMax = app_p->enemy.pos_y;

    Graphics_setForegroundColor(&app_p->gfxContext, GRAPHICS_COLOR_BLACK);
    Graphics_fillRectangle(&app_p->gfxContext, &character);

    return;
}

//moves the enemy along the x axis of the screen and deletes the previous enemy's position for fluid movement
void move_enemy(Application* app_p)
{
    //deletes previous frame
    delete_previous_enemy(app_p);

    //checks if the enemy has hit the left barrier and if it has changes the trajectory to go to the right
    if(app_p->enemy.pos_x <= GAMEBOUNDS_LEFT && app_p->enemy_left )
    {
        app_p->enemy.pos_x = GAMEBOUNDS_LEFT;
        app_p->enemy_left = false;
        app_p->enemy_right = true;
    }
    //checks if the enemy has hit the right barrier and if it has changes the trajectory to go to the left
    if(app_p->enemy.pos_x >= GAMEBOUNDS_RIGHT && app_p->enemy_right)
    {
        app_p->enemy.pos_x = GAMEBOUNDS_RIGHT;
        app_p->enemy_left = true;
        app_p->enemy_right = false;
    }
    //moves the enemy to the left
    if(app_p->enemy_left)
    {
        app_p->enemy.pos_x -= app_p->gamespeed;
        Display_enemy( app_p);
    }
    //moves the enemy to the right
    if(app_p->enemy_right)
    {
        app_p->enemy.pos_x += app_p->gamespeed;
        Display_enemy( app_p);
    }
}

//checks to see if the enemy has been hit by the players rocket and if it has it clears the rocket and reduces the enemy's HP
void enemy_collision_check(Application* app_p)
{
    int i;
    for(i=0; i < MAX_ROCKET_NUM; i++)
    {
        if(app_p->p1_rocket[i].moving)//checks to see if the rocket is moving
        {
            if(app_p->p1_rocket[i].x >= (app_p->enemy.pos_x - CHAR_SIZE) && app_p->p1_rocket[i].x <= app_p->enemy.pos_x)
            {//if the rocket is in the x radius of the square
                if(app_p->p1_rocket[i].y >= (app_p->enemy.pos_y - CHAR_SIZE) && app_p->p1_rocket[i].y <= app_p->enemy.pos_y)
                {//if the rocket is in the y radius of the square and the x radius as well
                    //reduces the enemy's health and removes the rocket from the screen
                    app_p->enemy_HP -= CHAR_DAMAGE;
                    app_p->p1_rocket[i].moving = false;

                    Graphics_Rectangle rocket;

                    rocket.xMax = app_p->p1_rocket[i].x;
                    rocket.xMin = app_p->p1_rocket[i].x - ROCKETSIZE_X;
                    rocket.yMax = app_p->p1_rocket[i].y;
                    rocket.yMin = app_p->p1_rocket[i].y + ROCKETSIZE_Y;

                    Graphics_setForegroundColor(&app_p->gfxContext, GRAPHICS_COLOR_BLACK);
                    Graphics_fillRectangle(&app_p->gfxContext, &rocket);
                }
            }

        }
    }

}
//checks to see if the player has been hit by an enemy rocket and if it has it clears the rocket and reduces the players HP
void player_collision_check(Application* app_p)
{
    int i;
    for(i=0; i < MAX_ROCKET_NUM; i++)
    {
        if(app_p->enemy_rocket[i].moving)
        {
            if(app_p->enemy_rocket[i].x >= (app_p->p1.pos_x - CHAR_SIZE) && app_p->enemy_rocket[i].x <= app_p->p1.pos_x)
            {
                if(app_p->enemy_rocket[i].y >= (app_p->p1.pos_y - CHAR_SIZE) && app_p->enemy_rocket[i].y <= app_p->p1.pos_y)
                {   //if the rocket is in the players radius deals damage and clears the rocket
                    app_p->player_HP -= CHAR_DAMAGE;
                    app_p->enemy_rocket[i].moving = false;

                    Graphics_Rectangle rocket;

                    rocket.xMax = app_p->enemy_rocket[i].x;
                    rocket.xMin = app_p->enemy_rocket[i].x - ROCKETSIZE_X;
                    rocket.yMax = app_p->enemy_rocket[i].y;
                    rocket.yMin = app_p->enemy_rocket[i].y + ROCKETSIZE_Y;

                    Graphics_setForegroundColor(&app_p->gfxContext, GRAPHICS_COLOR_BLACK);
                    Graphics_fillRectangle(&app_p->gfxContext, &rocket);
                }
            }

        }
    }

}
void enemy_vertical(Application* app_p)
{
    uint8_t i;

    delete_enemy_rockets(app_p);

    for (i = 0; i < MAX_ROCKET_NUM ; i++)
    {
        // Find the first available "inactive" bullet slot in the array
        if (!app_p->enemy_rocket[i].moving && app_p->one_launch == false)
        {
            // Found an inactive bullet slot, activate it
            app_p->enemy_rocket[i].moving = true;

            // Launch the bullet from the center of the player
            app_p->enemy_rocket[i].x = 28 + (i * 8);
            app_p->enemy_rocket[i].y = app_p->enemy.pos_y;
        }
    }
    //moves the rocket down the screen or erases it off the screen if it is at the edge
    for(i=0; i < MAX_ROCKET_NUM ; i++)
    {
        if(app_p->enemy_rocket[i].moving)
        app_p->enemy_rocket[i].y += app_p->gamespeed;

        if(app_p->enemy_rocket[i].y > 125)
            app_p->enemy_rocket[i].moving = false;

    }
    //prints the rocket in its new position
    for(i=0; i < MAX_ROCKET_NUM ; i++)
    {
        if(app_p->enemy_rocket[i].moving)
        {
            Graphics_Rectangle rocket;

            rocket.xMax = app_p->enemy_rocket[i].x;
            rocket.xMin = app_p->enemy_rocket[i].x - ROCKETSIZE_X;
            rocket.yMax = app_p->enemy_rocket[i].y;
            rocket.yMin = app_p->enemy_rocket[i].y + ROCKETSIZE_Y;

            Graphics_setForegroundColor(&app_p->gfxContext, GRAPHICS_COLOR_YELLOW);
            Graphics_fillRectangle(&app_p->gfxContext, &rocket);
        }
    }

    change_enemy_pattern(app_p);

}
void enemy_diagonal(Application* app_p)
{
    uint8_t i;

    delete_enemy_rockets(app_p);

    for (i = 0; i < MAX_ROCKET_NUM ; i++)
    {
        // Find the first available "inactive" bullet slot in the array
        if (!app_p->enemy_rocket[i].moving && app_p->one_launch == false)
        {
            // Found an inactive bullet slot, activate it
            app_p->enemy_rocket[i].moving = true;

            // Launch the bullet from the center of the player
            app_p->enemy_rocket[i].x = 28 + (i * 8);
            app_p->enemy_rocket[i].y = app_p->enemy.pos_y;
        }
    }
    //moves the rocket down the square
    for(i=0; i < MAX_ROCKET_NUM ; i++)
    {
        if(app_p->enemy_rocket[i].moving)
        {
        app_p->enemy_rocket[i].y += app_p->gamespeed;
        app_p->enemy_rocket[i].x += app_p->gamespeed;

        }
        if(app_p->enemy_rocket[i].y > 125)
            app_p->enemy_rocket[i].moving = false;

    }
    //prints the rocket in its new position
    for(i=0; i < MAX_ROCKET_NUM ; i++) {
        if(app_p->enemy_rocket[i].moving){
            Graphics_Rectangle rocket;

            rocket.xMax = app_p->enemy_rocket[i].x;
            rocket.xMin = app_p->enemy_rocket[i].x - ROCKETSIZE_X;
            rocket.yMax = app_p->enemy_rocket[i].y;
            rocket.yMin = app_p->enemy_rocket[i].y + ROCKETSIZE_Y;

            Graphics_setForegroundColor(&app_p->gfxContext, GRAPHICS_COLOR_RED);
            Graphics_fillRectangle(&app_p->gfxContext, &rocket);
        }
    }

    change_enemy_pattern(app_p);
}
void enemy_horizontal(Application* app_p)
{
    uint8_t i;

    delete_enemy_rockets(app_p);
    //begins shooting the rockets if they are not currently moving
    for (i = 0; i < MAX_ROCKET_NUM ; i++)
    {
        // Find the first available "inactive" bullet slot in the array
        if (!app_p->enemy_rocket[i].moving && app_p->one_launch == false)
        {
            // Found an inactive bullet slot, activate it
            app_p->enemy_rocket[i].moving = true;

            app_p->enemy_rocket[i].x = 20;
            app_p->enemy_rocket[i].y = LCD_EDGE - (10 * i);

            break;
        }
    }
    //moves the value of the rocket to the right it is currently moving and performs a collision
    for(i=0; i < MAX_ROCKET_NUM ; i++)
    {
        //if the rocket is moving progesses it to the right
        if(app_p->enemy_rocket[i].moving)
        app_p->enemy_rocket[i].x += app_p->gamespeed;
        //if the rocket passes the x barrier it then disappears
        if(app_p->enemy_rocket[i].x > GAMEBOUNDS_RIGHT)
            app_p->enemy_rocket[i].moving = false;
        //if all the rockets have been fired and collided one_collision indicates that the pattern has run its course

    }
    //displays the rocket in its new position as it moves across the screen and checks to see if the rockets have run their course
    for(i=0; i < MAX_ROCKET_NUM  ; i++)
    {
        if(app_p->enemy_rocket[i].moving)
        {
            Graphics_Rectangle rocket;

            rocket.xMax = app_p->enemy_rocket[i].x;
            rocket.xMin = app_p->enemy_rocket[i].x - ROCKETSIZE_X;
            rocket.yMax = app_p->enemy_rocket[i].y;
            rocket.yMin = app_p->enemy_rocket[i].y + ROCKETSIZE_Y;

            Graphics_setForegroundColor(&app_p->gfxContext, GRAPHICS_COLOR_BLUE);
            Graphics_fillRectangle(&app_p->gfxContext, &rocket);
        }
    }
    change_enemy_pattern(app_p);
}
void enemy_horizontal_both_sides(Application* app_p)
{
    uint8_t i;

    delete_enemy_rockets(app_p);
    //begins shooting the rockets if they are not currently moving
    for (i = 0; i < MAX_ROCKET_NUM ; i++){
        // Find the first available "inactive" bullet slot in the array
        if (!app_p->enemy_rocket[i].moving && app_p->one_launch == false){
            // Found an inactive bullet slot, activate it
            app_p->enemy_rocket[i].moving = true;
            if(i%2 == 0){
            app_p->enemy_rocket[i].x = GAMEBOUNDS_RIGHT;
            app_p->enemy_rocket[i].y = LCD_EDGE - (10 * i);
            }//end if
            if(i%2 ==1){
                app_p->enemy_rocket[i].x = GAMEBOUNDS_LEFT;
                app_p->enemy_rocket[i].y = LCD_EDGE - (10 * i);
            }//end if

            break;
        }//end if
    }//end for
    //moves the value of the rocket to the right it is currently moving and performs a collision
    for(i=0; i < MAX_ROCKET_NUM ; i++){
        //if the rocket is moving progesses it to the right
        if(app_p->enemy_rocket[i].moving){
            if(i%2 == 0) app_p->enemy_rocket[i].x -= app_p->gamespeed;
            else app_p->enemy_rocket[i].x += app_p->gamespeed;
        }//end of
        //if the rocket passes the x barrier it then disappears
        if(app_p->enemy_rocket[i].x > GAMEBOUNDS_RIGHT || app_p->enemy_rocket[i].x < GAMEBOUNDS_LEFT)
            app_p->enemy_rocket[i].moving = false;
    }//end for
    //displays the rocket in its new position as it moves across the screen and checks to see if the rockets have run their course
    for(i=0; i < MAX_ROCKET_NUM  ; i++){
        if(app_p->enemy_rocket[i].moving){
            Graphics_Rectangle rocket;

            rocket.xMax = app_p->enemy_rocket[i].x;
            rocket.xMin = app_p->enemy_rocket[i].x - ROCKETSIZE_X;
            rocket.yMax = app_p->enemy_rocket[i].y;
            rocket.yMin = app_p->enemy_rocket[i].y + ROCKETSIZE_Y;

            Graphics_setForegroundColor(&app_p->gfxContext, GRAPHICS_COLOR_YELLOW);
            Graphics_fillRectangle(&app_p->gfxContext, &rocket);
        }
    }
    change_enemy_pattern(app_p);
}
//a pattern only shooting the rockets from the right of the screen with large gaps in between
void enemy_horizontal_left(Application* app_p)
{
    uint8_t i;

    delete_enemy_rockets(app_p);
    //begins shooting the rockets if they are not currently moving
    for (i = 0; i < MAX_ROCKET_NUM ; i++){
        // Find the first available "inactive" bullet slot in the array
        if (!app_p->enemy_rocket[i].moving && app_p->one_launch == false){
            // Found an inactive bullet slot, activate it
            app_p->enemy_rocket[i].moving = true;
            if(i%2 == 0){
            app_p->enemy_rocket[i].x = GAMEBOUNDS_RIGHT;
            app_p->enemy_rocket[i].y = LCD_EDGE - (10 * i);
            }//end if
            if(i%2 ==1){
                app_p->enemy_rocket[i].x = GAMEBOUNDS_LEFT;
                app_p->enemy_rocket[i].y = LCD_EDGE + (10 * i);
            }//end for

            break;
        }
    }
    //moves the value of the rocket to the left it is currently moving and performs a collision
    for(i=0; i < MAX_ROCKET_NUM ; i++){
        //if the rocket is moving progesses it to the left
        if(app_p->enemy_rocket[i].moving){
            if(i%2 == 0) app_p->enemy_rocket[i].x -= app_p->gamespeed;
            else app_p->enemy_rocket[i].x += app_p->gamespeed;
        }//end if
        //if the rocket passes the x barrier it then disappears
        if(app_p->enemy_rocket[i].x > GAMEBOUNDS_RIGHT || app_p->enemy_rocket[i].x < GAMEBOUNDS_LEFT)
            app_p->enemy_rocket[i].moving = false;
        //if all the rockets have been fired and collided one_collision indicates that the pattern has run its course
    }//end for
    //displays the rocket in its new position as it moves across the screen and checks to see if the rockets have run their course
    for(i=0; i < MAX_ROCKET_NUM  ; i++){
        if(app_p->enemy_rocket[i].moving){
            Graphics_Rectangle rocket;

            rocket.xMax = app_p->enemy_rocket[i].x;
            rocket.xMin = app_p->enemy_rocket[i].x - ROCKETSIZE_X;
            rocket.yMax = app_p->enemy_rocket[i].y;
            rocket.yMin = app_p->enemy_rocket[i].y + ROCKETSIZE_Y;

            Graphics_setForegroundColor(&app_p->gfxContext, GRAPHICS_COLOR_GREEN);
            Graphics_fillRectangle(&app_p->gfxContext, &rocket);
        }
    }
    change_enemy_pattern(app_p);
}
//a pattern only shooting the rockets from the left of the screen with large gaps in between
void enemy_horizontal_right(Application* app_p)
{
    uint8_t i;

    delete_enemy_rockets(app_p);
    //begins shooting the rockets if they are not currently moving
    for (i = 0; i < MAX_ROCKET_NUM ; i++){
        // Find the first available "inactive" bullet slot in the array
        if (!app_p->enemy_rocket[i].moving && app_p->one_launch == false){
            // Found an inactive bullet slot, activate it
            app_p->enemy_rocket[i].moving = true;
            if(i%2 == 0){
            app_p->enemy_rocket[i].x = GAMEBOUNDS_RIGHT;
            app_p->enemy_rocket[i].y = LCD_EDGE + (10 * i);
            }//end if
            if(i%2 ==1){
                app_p->enemy_rocket[i].x = GAMEBOUNDS_LEFT;
                app_p->enemy_rocket[i].y = LCD_EDGE - (10 * i);
            }//end if
            break;
        }// end if
    }//end for
    //moves the value of the rocket to the right it is currently moving and performs a collision
    for(i=0; i < MAX_ROCKET_NUM ; i++){
        //if the rocket is moving progresses it to the right
        if(app_p->enemy_rocket[i].moving){
            if(i%2 == 0) app_p->enemy_rocket[i].x -= app_p->gamespeed;
            else app_p->enemy_rocket[i].x += app_p->gamespeed;
        }//end if
        //if the rocket passes the x barrier it then disappears
        if(app_p->enemy_rocket[i].x > GAMEBOUNDS_RIGHT || app_p->enemy_rocket[i].x < GAMEBOUNDS_LEFT)
            app_p->enemy_rocket[i].moving = false;
        //if all the rockets have been fired and collided one_collision indicates that the pattern has run its course
    }
    //displays the rocket in its new position as it moves across the screen and checks to see if the rockets have run their course
    for(i=0; i < MAX_ROCKET_NUM  ; i++){
        if(app_p->enemy_rocket[i].moving){
            Graphics_Rectangle rocket;

            rocket.xMax = app_p->enemy_rocket[i].x;
            rocket.xMin = app_p->enemy_rocket[i].x - ROCKETSIZE_X;
            rocket.yMax = app_p->enemy_rocket[i].y;
            rocket.yMin = app_p->enemy_rocket[i].y + ROCKETSIZE_Y;

            Graphics_setForegroundColor(&app_p->gfxContext, GRAPHICS_COLOR_RED);
            Graphics_fillRectangle(&app_p->gfxContext, &rocket);
        }//end if
    }//end for
    change_enemy_pattern(app_p);
}

// deletes the previous frame of enemy rockets as it travels across the screen// deletes the previous frame of enemy rockets as it travels across the screen
void delete_enemy_rockets(Application* app_p)
{
    uint8_t i;
    //checks if the rocket is active
    for(i=0; i < MAX_ROCKET_NUM  ; i++)
    {
        if(app_p->enemy_rocket[i].moving)
        {   //gets the shape of the enemy's previous frame
            Graphics_Rectangle rocket;

            //x dimensions of the rocket
            rocket.xMax = app_p->enemy_rocket[i].x;
            rocket.xMin = app_p->enemy_rocket[i].x - ROCKETSIZE_X;
            //y dimensions of the rocket
            rocket.yMax = app_p->enemy_rocket[i].y;
            rocket.yMin = app_p->enemy_rocket[i].y + ROCKETSIZE_Y;
            //deletes the shape
            Graphics_setForegroundColor(&app_p->gfxContext, GRAPHICS_COLOR_BLACK);
            Graphics_fillRectangle(&app_p->gfxContext, &rocket);
        }
    }
    return;
}
// changes the enemy's launch pattern once one launch pattern is complete
void change_enemy_pattern(Application* app_p)
{
    if(enemy_launch_status(app_p))
    {
        app_p->enemy_pattern = CircularIncrement(app_p->enemy_pattern, 6);
        app_p->one_collision = false;
        app_p->one_launch = false;

    }
    return;
}
//checks if an enemy rocket lauch pattern has been completed
bool enemy_launch_status(Application* app_p)
{
    int i;
    int active_rockets = DEFAULT_NUM;
    int inactive_rockets = DEFAULT_NUM;

    for(i= 0 ; i< MAX_ROCKET_NUM; i++)
    {
        if(app_p->enemy_rocket[i].moving)
        {
            active_rockets++;
            if(active_rockets == MAX_ROCKET_NUM)
                app_p->one_launch = true;
        }
        if(app_p->one_launch && app_p->enemy_rocket[i].moving == false)
        {
            inactive_rockets++;
            if(inactive_rockets == MAX_ROCKET_NUM)
                app_p->one_collision = true;
        }
    }

    return app_p->one_collision && app_p->one_launch;
}

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

#include "raylib.h"
#include "raymath.h"

#define CLAGS_IMPLEMENTATION
#include "clags.h"

#define WIDTH 800
#define HEIGHT 600
#define PLAYER_SLIPPINESS 500
#define PLAYER_WIDTH 20
#define PLAYER_HEIGHT 128
#define BALL_SIZE 20
#define BALL_SPEEDUP_FACTOR 1.05f
#define FONT_SIZE 18
#define PAUSED_TEXT "Paused."
#define RANDOM_START_VEL 100

uint8_t max_score;
double player_speed;
bool singleplayer = false;
clags_range_t player_speed_range = clags_double_range(100, 1000);
clags_config_t *help = NULL;

Rectangle ball = {WIDTH/2-BALL_SIZE/2, HEIGHT/2-BALL_SIZE/2, BALL_SIZE, BALL_SIZE};
Vector2 ball_vel = {0};

Rectangle player1 = {10, HEIGHT/2-PLAYER_HEIGHT/2, PLAYER_WIDTH, PLAYER_HEIGHT};
Rectangle player2 = {WIDTH-PLAYER_WIDTH-10, HEIGHT/2-PLAYER_HEIGHT/2, PLAYER_WIDTH, PLAYER_HEIGHT};

size_t player1_score = 0;
size_t player1_wins = 0;
size_t player2_score = 0;
size_t player2_wins = 0;
char score1_buffer[64];
char score2_buffer[64];
bool paused = true;


clags_arg_t args[] = {
    clags_option('m', "max-score", &max_score, "SCORE", "the score to reach (0 means infinite)", .value_type=Clags_UInt8, .default_input="0"),
    clags_option('p', "player-speed", &player_speed, "SPEED", "the speed with which the players move", .value_type=Clags_Double, .range=&player_speed_range, .default_input="500"),
    clags_flag('s', "singleplayer", &singleplayer, "play against the computer"),
    clags_flag_help_config(&help),
};
clags_config_t config = clags_config(args);

void update_frame(void)
{
    
    float dt = GetFrameTime();

    if (IsKeyPressed(KEY_SPACE)) paused = !paused;

    // Player Movement
    if (IsKeyDown(KEY_S)) player1.y += player_speed * dt;
    if (IsKeyDown(KEY_W)) player1.y -= player_speed * dt;
    if (singleplayer){
        if (player2.y + player2.height < ball.y) player2.y += player_speed *dt;
        if (player2.y > ball.y) player2.y -= player_speed * dt;
    } else{
        if (IsKeyDown(KEY_DOWN)) player2.y += player_speed * dt;
        if (IsKeyDown(KEY_UP)) player2.y -= player_speed * dt;
    }
    if (player1.y < 0) player1.y = 0;
    if (player1.y > HEIGHT-player1.height) player1.y = HEIGHT-player1.height;
    if (player2.y < 0) player2.y = 0;
    if (player2.y > HEIGHT-player2.height) player2.y = HEIGHT-player2.height;
    if (!paused){


        // Ball Movement
        ball.x += ball_vel.x * dt;
        ball.y += ball_vel.y * dt;

        // Wall Collision
        if (ball.y <= 0){
            ball.y = 1;
            ball_vel.y *= -1;
        }
        if (ball.y >= HEIGHT-ball.height){
            ball.y = HEIGHT-ball.height - 1;
            ball_vel.y *= -1;
        }
            
        if (ball.x <= 0){
            ball.x = WIDTH/2-ball.width/2;
            ball.y = HEIGHT/2-ball.height/2;
            ball_vel = (Vector2) {800, (float)GetRandomValue(-RANDOM_START_VEL, RANDOM_START_VEL)};
            player2_score += 1;
            paused = true;
            if (max_score && player2_score >= max_score){
                player1_score = player2_score = 0;
                player2_wins += 1;
                paused = true;
            }
        }

        if (ball.x >= WIDTH-ball.width){
            ball.x = WIDTH/2-ball.width/2;
            ball.y = HEIGHT/2-ball.height/2;
            ball_vel = (Vector2) {-800, (float)GetRandomValue(-RANDOM_START_VEL, RANDOM_START_VEL)};
            player1_score += 1;
            paused = true;
            if (max_score && player1_score >= max_score){
                player1_score = player2_score = 0;
                player1_wins += 1;
                paused = true;
            }
        }

        // Player Collision
        if (CheckCollisionRecs(player1, ball)){
            if (ball_vel.x < 0){
                ball_vel.x = Clamp(ball_vel.x * -BALL_SPEEDUP_FACTOR, 0, 1200);

                float dy = (ball.y + ball.height/2) - (player1.y + player1.height/2);
                ball_vel.y = (dy/(player1.height/2))*PLAYER_SLIPPINESS;
                ball.x = player1.x + player1.width + 1;
            }
        }
        if (CheckCollisionRecs(player2, ball)){
            if (ball_vel.x > 0){
                ball_vel.x = -Clamp(ball_vel.x * BALL_SPEEDUP_FACTOR, 0, 1200);

                float dy = (ball.y + ball.height/2) - (player2.y + player2.height/2);
                ball_vel.y = (dy/(player2.height/2))*PLAYER_SLIPPINESS;
                ball.x = player2.x - 1;
            }
        }

    }
    if (max_score){
        snprintf(score1_buffer, sizeof(score1_buffer), "Wins: %zu Score: %zu", player1_wins, player1_score);
        snprintf(score2_buffer, sizeof(score2_buffer), "Wins: %zu Score: %zu", player2_wins, player2_score);
    } else{
        snprintf(score1_buffer, sizeof(score1_buffer), "Score: %zu", player1_score);
        snprintf(score2_buffer, sizeof(score2_buffer), "Score: %zu", player2_score);
    }
    float score1_x = 10;
    float score2_x = WIDTH - MeasureText(score2_buffer, FONT_SIZE) - 10;
    float paused_x = WIDTH/2 - MeasureText(PAUSED_TEXT, FONT_SIZE)/2;

    BeginDrawing();
    {
        ClearBackground(GetColor(0x181818FF));
            
        // Center Line
        for (int i=0; i<HEIGHT; i += 40) {
            DrawRectangle(WIDTH / 2 - 2, i+10, 4, 20, DARKGRAY);
        }
         
        DrawRectangleRec(ball, RED);
        DrawRectangleRec(player1, WHITE);
        DrawRectangleRec(player2, WHITE);

            
        DrawText(score1_buffer, score1_x, 10, FONT_SIZE, WHITE);
        DrawText(score2_buffer, score2_x, 10, FONT_SIZE, WHITE);
        if (paused){
            DrawText(PAUSED_TEXT, paused_x, 10, FONT_SIZE, ORANGE);
        }
    }
    EndDrawing();
}


int main(int argc, char *argv[])
{
    clags_config_t *failed = clags_parse(argc, argv, &config);
    if (failed){
        clags_usage(argv[0], failed);
        return 1;
    }
    if (help){
        clags_usage(argv[0], help);
        return 0;
    }
    
    InitWindow(WIDTH, HEIGHT, "Pong");
    SetTargetFPS(60);
    SetRandomSeed(time(NULL));

    ball_vel = (Vector2){-800, (float)GetRandomValue(-RANDOM_START_VEL, RANDOM_START_VEL)};

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(update_frame, 0, 1);
#else
    while (!WindowShouldClose()){
        update_frame();
    }
#endif // PLATFORM_WEB
    return 0;
}

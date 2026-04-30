#include <stdio.h>
#include <math.h>
#include <time.h>

#include "raylib.h"
#include "raymath.h"

#define WIDTH 800
#define HEIGHT 600
#define PLAYER_SPEED 500
#define PLAYER_SLIPPINESS 500
#define PLAYER_WIDTH 20
#define PLAYER_HEIGHT 128
#define BALL_SIZE 20
#define BALL_SPEEDUP_FACTOR 1.05f
#define FONT_SIZE 18
#define PAUSED_TEXT "Paused."
#define RANDOM_START_VEL 100

int main(void)
{
    InitWindow(WIDTH, HEIGHT, "Pong");
    SetTargetFPS(60);
    SetRandomSeed(time(NULL));

    Rectangle ball = {WIDTH/2-BALL_SIZE/2, HEIGHT/2-BALL_SIZE/2, BALL_SIZE, BALL_SIZE};
    Vector2 ball_vel = {-800, (float)GetRandomValue(-RANDOM_START_VEL, RANDOM_START_VEL)};

    Rectangle player = {10, HEIGHT/2-PLAYER_HEIGHT/2, PLAYER_WIDTH, PLAYER_HEIGHT};

    size_t score = 0;
    size_t best_score = 0;
    char score_buffer[64];
    bool paused = true;
    
    while (!WindowShouldClose()){

        float dt = GetFrameTime();

        if (IsKeyPressed(KEY_SPACE)) paused = !paused;

        if (!paused){
            // Player Movement
            if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) player.y += PLAYER_SPEED * dt;
            if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) player.y -= PLAYER_SPEED * dt;

            if (player.y < 0) player.y = 0;
            if (player.y > HEIGHT-player.height) player.y = HEIGHT-player.height;

            // Ball Movement
            ball.x += ball_vel.x * dt;
            ball.y += ball_vel.y * dt;

            // Wall Collision
            if (ball.x >= WIDTH-ball.width){
                ball_vel.x *= -1;
                ball.x = WIDTH-ball.width-1;
            }
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
                ball_vel = (Vector2) {-800, (float)GetRandomValue(-RANDOM_START_VEL, RANDOM_START_VEL)};
                if (score > best_score) best_score = score;
                score = 0;
            }


            // Player Collision
            if (CheckCollisionRecs(player, ball)){
                if (ball_vel.x < 0){
                    ball_vel.x = Clamp(ball_vel.x * -BALL_SPEEDUP_FACTOR, 0, 1200);

                    float dy = (ball.y + ball.height/2) - (player.y + player.height/2);
                    ball_vel.y = (dy/(player.height/2))*PLAYER_SLIPPINESS;
                    ball.x = player.x + player.width + 1;
                    score += 1;
                }
            }

        }
        snprintf(score_buffer, sizeof(score_buffer), "Score: %zu Best: %zu", score, best_score);
        float score_x = WIDTH - MeasureText(score_buffer, FONT_SIZE) - 10;
        float paused_x = WIDTH/2 - MeasureText(PAUSED_TEXT, FONT_SIZE)/2;

        BeginDrawing();
        {
            ClearBackground(GetColor(0x181818FF));
            
            // Center Line
            for (int i=0; i<HEIGHT; i += 40) {
                DrawRectangle(WIDTH / 2 - 2, i+10, 4, 20, DARKGRAY);
            }
         
            DrawRectangleRec(ball, RED);
            DrawRectangleRec(player, WHITE);

            
            DrawText(score_buffer, score_x, 10, FONT_SIZE, WHITE);
            if (paused){
                DrawText(PAUSED_TEXT, paused_x, 10, FONT_SIZE, ORANGE);
            }
        }
        EndDrawing();
    }
    return 0;
}

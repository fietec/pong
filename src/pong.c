#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

#include <raylib.h>
#include <raymath.h>

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

typedef enum{
    Singleplayer,
    Computer,
    Multiplayer,
} Mode;

static struct{
    Rectangle ball;
    Vector2 ball_vel;
    Rectangle player1;
    Rectangle player2;
    
    size_t player1_score;
    size_t player1_wins;
    size_t player2_score;
    size_t player2_wins;
    char score1_buffer[64];
    char score2_buffer[64];
    bool paused;
    float paused_x, score2_x;
    Mode mode;
    size_t max_score;
    double player_speed;
    bool text_changed;
} game =  {
    .ball = {WIDTH/2-BALL_SIZE/2, HEIGHT/2-BALL_SIZE/2, BALL_SIZE, BALL_SIZE},
    .player1 = {10, HEIGHT/2-PLAYER_HEIGHT/2, PLAYER_WIDTH, PLAYER_HEIGHT},
    .player2 = {WIDTH-PLAYER_WIDTH-10, HEIGHT/2-PLAYER_HEIGHT/2, PLAYER_WIDTH, PLAYER_HEIGHT},
    .paused = true,
    .text_changed = true,
};

void update_text(void)
{
    if (game.mode == Singleplayer){
        snprintf(game.score2_buffer, sizeof(game.score2_buffer), "Score: %zu Best: %zu", game.player1_score, game.player1_wins);
    } else{
        if (game.max_score){
            snprintf(game.score1_buffer, sizeof(game.score1_buffer), "Score: %zu Wins: %zu", game.player1_score, game.player1_wins);
            snprintf(game.score2_buffer, sizeof(game.score2_buffer), "Score: %zu Wins: %zu", game.player2_score, game.player2_wins); 
        } else{
            snprintf(game.score1_buffer, sizeof(game.score1_buffer), "Score: %zu", game.player1_score);
            snprintf(game.score2_buffer, sizeof(game.score2_buffer), "Score: %zu", game.player2_score); 
        }
    }
    game.score2_x = WIDTH - MeasureText(game.score2_buffer, FONT_SIZE) - 10;
    game.paused_x = WIDTH/2 - MeasureText(PAUSED_TEXT, FONT_SIZE) / 2;
    game.text_changed = false;
}

void update_frame(void)
{
    if (game.text_changed) update_text();
    float dt = GetFrameTime();

    if (IsKeyPressed(KEY_SPACE)){
        game.paused = !game.paused;
        game.text_changed = true;
    }

    // Player Movement
    if (IsKeyDown(KEY_S)) game.player1.y += game.player_speed * dt;
    if (IsKeyDown(KEY_W)) game.player1.y -= game.player_speed * dt;
    switch (game.mode){
        case Computer:{
            if (game.player2.y + game.player2.height < game.ball.y) game.player2.y += game.player_speed *dt;
            if (game.player2.y > game.ball.y) game.player2.y -= game.player_speed * dt;
        } break;
        case Multiplayer:{
            if (IsKeyDown(KEY_DOWN)) game.player2.y += game.player_speed * dt;
            if (IsKeyDown(KEY_UP)) game.player2.y -= game.player_speed * dt;
        } break;
        default: break;
    }

    // Player-Wall Collision
    if (game.player1.y < 0) game.player1.y = 0;
    if (game.player1.y > HEIGHT-game.player1.height) game.player1.y = HEIGHT-game.player1.height;
    if (game.player2.y < 0) game.player2.y = 0;    
    if (game.player2.y > HEIGHT-game.player2.height) game.player2.y = HEIGHT-game.player2.height;
    
    if (!game.paused){
        // Ball Movement
        game.ball.x += game.ball_vel.x * dt;
        game.ball.y += game.ball_vel.y * dt;

        // Wall Collision
        // Top Wall
        if (game.ball.y <= 0){
            game.ball.y = 1;
            game.ball_vel.y *= -1;
        }
        // Bottom Wall
        if (game.ball.y >= HEIGHT-game.ball.height){
            game.ball.y = HEIGHT-game.ball.height - 1;
            game.ball_vel.y *= -1;
        }
        // Left Wall   
        if (game.ball.x <= 0){
            game.ball.x = WIDTH/2-game.ball.width/2;
            game.ball.y = HEIGHT/2-game.ball.height/2;
            game.paused = true;

            // Update scores
            if (game.mode == Singleplayer){
                game.ball_vel = (Vector2) {-800, (float)GetRandomValue(-RANDOM_START_VEL, RANDOM_START_VEL)};
                if (game.player1_score > game.player1_wins) game.player1_wins = game.player1_score;
                game.player1_score = 0;
            } else{
                game.ball_vel = (Vector2) {800, (float)GetRandomValue(-RANDOM_START_VEL, RANDOM_START_VEL)};
                game.player2_score += 1;
                if (game.max_score && game.player2_score >= game.max_score){
                    game.player1_score = game.player2_score = 0;
                    game.player2_wins += 1;
                }
            }
            game.text_changed = true;
        }
        // Right Wall
        if (game.ball.x >= WIDTH-game.ball.width){
            if (game.mode == Singleplayer){
                game.ball.x = WIDTH-game.ball.width - 1;
                game.ball_vel.x *= -1;
            } else{
                game.ball.x = WIDTH/2-game.ball.width/2;
                game.ball.y = HEIGHT/2-game.ball.height/2;
                game.ball_vel = (Vector2) {-800, (float)GetRandomValue(-RANDOM_START_VEL, RANDOM_START_VEL)};
                game.player1_score += 1;
                game.paused = true;
                if (game.max_score && game.player1_score >= game.max_score){
                    game.player1_score = game.player2_score = 0;
                    game.player1_wins += 1;
                }
                game.text_changed = true;
            }
        }

        // Player Collision
        if (CheckCollisionRecs(game.player1, game.ball)){
            if (game.ball_vel.x < 0){
                game.ball_vel.x = Clamp(game.ball_vel.x * -BALL_SPEEDUP_FACTOR, 0, 1200);

                float dy = (game.ball.y + game.ball.height/2) - (game.player1.y + game.player1.height/2);
                game.ball_vel.y = (dy/(game.player1.height/2))*PLAYER_SLIPPINESS;
                game.ball.x = game.player1.x + game.player1.width + 1;
                if (game.mode == Singleplayer){
                    game.player1_score += 1;
                    game.text_changed = true;
                }
            }
        }
        if (game.mode != Singleplayer && CheckCollisionRecs(game.player2, game.ball)){
            if (game.ball_vel.x > 0){
                game.ball_vel.x = -Clamp(game.ball_vel.x * BALL_SPEEDUP_FACTOR, 0, 1200);

                float dy = (game.ball.y + game.ball.height/2) - (game.player2.y + game.player2.height/2);
                game.ball_vel.y = (dy/(game.player2.height/2))*PLAYER_SLIPPINESS;
                game.ball.x = game.player2.x - 1;
            }
        }
    }

    BeginDrawing();
    {
        ClearBackground(GetColor(0x181818FF));
            
        // Center Line
        for (int i=0; i<HEIGHT; i += 40) {
            DrawRectangle(WIDTH / 2 - 2, i+10, 4, 20, DARKGRAY);
        }
         
        DrawRectangleRec(game.ball, RED);
        DrawRectangleRec(game.player1, WHITE);
        if (game.mode != Singleplayer){
            DrawRectangleRec(game.player2, WHITE);
            DrawText(game.score1_buffer, 10, 10, FONT_SIZE, WHITE);
        }
        DrawText(game.score2_buffer, game.score2_x, 10, FONT_SIZE, WHITE);
        if (game.paused) DrawText(PAUSED_TEXT, game.paused_x, 10, FONT_SIZE, ORANGE);
    }
    EndDrawing();
}

int main(int argc, char *argv[])
{
    clags_choice_t *pmode = NULL;
    clags_config_t *help = NULL;

    clags_choice_t modes[] = {
        [Singleplayer] = {"single", "singleplayer with only one paddle"},
        [Computer] = {"computer", "singleplayer vs the computer"},
        [Multiplayer] = {"multi", "multiplayer for two players"},
    };
    clags_choices_t choices = clags_choices(modes);

    clags_range_t player_speed_range = clags_double_range(100, 1000);

    clags_arg_t args[] = {
        clags_option('s', "max-score", &game.max_score, "SCORE", "the score to reach (0 means infinite)", .value_type=Clags_SizeT, .default_input="0"),
        clags_option('p', "player-speed", &game.player_speed, "SPEED", "the speed with which the players move", .value_type=Clags_Double, .range=&player_speed_range, .default_input="500"),
        clags_option('m', "mode", &pmode, "MODE", "the game mode", .value_type=Clags_Choice, .choices=&choices, .default_input="single"),
        clags_flag_help_config(&help),
    };
    clags_config_t config = clags_config(args);
    
    clags_config_t *failed = clags_parse(argc, argv, &config);
    if (failed){
        clags_usage(argv[0], failed);
        return 1;
    }
    if (help){
        clags_usage(argv[0], help);
        return 0;
    }
    game.mode = clags_choice_index(&choices, pmode);
    
    InitWindow(WIDTH, HEIGHT, "Pong");
    SetTargetFPS(60);
    SetRandomSeed(time(NULL));

    game.ball_vel = (Vector2){-800, (float)GetRandomValue(-RANDOM_START_VEL, RANDOM_START_VEL)};

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(update_frame, 0, 1);
#else
    while (!WindowShouldClose()){
        update_frame();
    }
#endif // PLATFORM_WEB
    return 0;
}

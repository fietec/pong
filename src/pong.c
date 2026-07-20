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
#define PLAYER_WIDTH 20
#define PLAYER_HEIGHT 128
#define PLAYER_MAX_SPEED 2000
#define BALL_MAX_SPEED 2400
#define BALL_SIZE 20
#define FONT_SIZE 18
#define PAUSED_TEXT "Paused."
#define RANDOM_START_VEL 100

typedef enum{
    Singleplayer,
    Computer,
    Multiplayer,
} Mode;

typedef struct {
    Rectangle rect;
    float vel;
    float init_vel;
    float max_vel;
    float delta_vel;
    float slip_vel;
    size_t score;
    size_t wins;
    char score_buf[64];
} Player;

typedef struct {
    Rectangle rect;
    Vector2 vel;
    float init_x_vel;
    float max_x_vel;
    float delta_x_vel;
} Ball;

static struct{
    Ball ball;
    Player player1;
    Player player2;

    Mode mode;
    bool paused;
    float paused_x, score2_x;
    size_t max_score;
    bool text_changed;
} game =  {
    .ball.rect = {WIDTH/2-BALL_SIZE/2, HEIGHT/2-BALL_SIZE/2, BALL_SIZE, BALL_SIZE},
    .player1.rect = {10, HEIGHT/2-PLAYER_HEIGHT/2, PLAYER_WIDTH, PLAYER_HEIGHT},
    .player2.rect = {WIDTH-PLAYER_WIDTH-10, HEIGHT/2-PLAYER_HEIGHT/2, PLAYER_WIDTH, PLAYER_HEIGHT},
    .paused = true,
    .text_changed = true,
};

void update_text(void)
{
    if (game.mode == Singleplayer){
        snprintf(game.player2.score_buf, sizeof(game.player2.score_buf), "Score: %zu Best: %zu", game.player1.score, game.player1.wins);
    } else{
        if (game.max_score){
            snprintf(game.player1.score_buf, sizeof(game.player1.score_buf), "Score: %zu Wins: %zu", game.player1.score, game.player1.wins);
            snprintf(game.player2.score_buf, sizeof(game.player2.score_buf), "Score: %zu Wins: %zu", game.player2.score, game.player2.wins);
        } else{
            snprintf(game.player1.score_buf, sizeof(game.player1.score_buf), "Score: %zu", game.player1.score);
            snprintf(game.player2.score_buf, sizeof(game.player2.score_buf), "Score: %zu", game.player2.score);
        }
    }
    game.score2_x = WIDTH - MeasureText(game.player2.score_buf, FONT_SIZE) - 10;
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
    if (IsKeyDown(KEY_S)) game.player1.rect.y += game.player1.vel * dt;
    if (IsKeyDown(KEY_W)) game.player1.rect.y -= game.player1.vel * dt;
    switch (game.mode){
        case Computer:{
            if (game.player2.rect.y + game.player2.rect.height < game.ball.rect.y) game.player2.rect.y += game.player2.vel *dt;
            if (game.player2.rect.y > game.ball.rect.y) game.player2.rect.y -= game.player2.vel * dt;
        } break;
        case Multiplayer:{
            if (IsKeyDown(KEY_DOWN)) game.player2.rect.y += game.player2.vel * dt;
            if (IsKeyDown(KEY_UP)) game.player2.rect.y -= game.player2.vel * dt;
        } break;
        default: break;
    }

    // Player-Wall Collision
    if (game.player1.rect.y < 0) game.player1.rect.y = 0;
    if (game.player1.rect.y > HEIGHT-game.player1.rect.height) game.player1.rect.y = HEIGHT-game.player1.rect.height;
    if (game.player2.rect.y < 0) game.player2.rect.y = 0;
    if (game.player2.rect.y > HEIGHT-game.player2.rect.height) game.player2.rect.y = HEIGHT-game.player2.rect.height;

    if (!game.paused){
        // Ball Movement
        game.ball.rect.x += game.ball.vel.x * dt;
        game.ball.rect.y += game.ball.vel.y * dt;

        // Wall Collision
        // Top Wall
        if (game.ball.rect.y <= 0){
            game.ball.rect.y = 1;
            game.ball.vel.y *= -1;
        }
        // Bottom Wall
        if (game.ball.rect.y >= HEIGHT-game.ball.rect.height){
            game.ball.rect.y = HEIGHT-game.ball.rect.height - 1;
            game.ball.vel.y *= -1;
        }
        // Left Wall
        if (game.ball.rect.x <= 0){
            game.ball.rect.x = WIDTH/2-game.ball.rect.width/2;
            game.ball.rect.y = HEIGHT/2-game.ball.rect.height/2;
            game.paused = true;

            // Update scores
            if (game.mode == Singleplayer){
                game.ball.vel = (Vector2) {-game.ball.init_x_vel, (float)GetRandomValue(-RANDOM_START_VEL, RANDOM_START_VEL)};
                if (game.player1.score > game.player1.wins) game.player1.wins = game.player1.score;
                game.player1.score = 0;
            } else{
                game.ball.vel = (Vector2) {game.ball.init_x_vel, (float)GetRandomValue(-RANDOM_START_VEL, RANDOM_START_VEL)};
                game.player2.score += 1;
                if (game.max_score && game.player2.score >= game.max_score){
                    game.player1.score = game.player2.score = 0;
                    game.player2.wins += 1;
                }
            }
            game.player1.vel = game.player1.init_vel;
            game.text_changed = true;
        }
        // Right Wall
        if (game.ball.rect.x >= WIDTH-game.ball.rect.width){
            if (game.mode == Singleplayer){
                game.ball.rect.x = WIDTH-game.ball.rect.width - 1;
                game.ball.vel.x *= -1;
            } else{
                game.ball.rect.x = WIDTH/2-game.ball.rect.width/2;
                game.ball.rect.y = HEIGHT/2-game.ball.rect.height/2;
                game.ball.vel = (Vector2) {-game.ball.init_x_vel, (float)GetRandomValue(-RANDOM_START_VEL, RANDOM_START_VEL)};
                game.player1.score += 1;
                game.paused = true;
                if (game.max_score && game.player1.score >= game.max_score){
                    game.player1.score = game.player2.score = 0;
                    game.player1.wins += 1;
                }
                game.text_changed = true;
                game.player2.vel = game.player2.init_vel;
            }
        }

        // Player Collision
        if (CheckCollisionRecs(game.player1.rect, game.ball.rect)){
            if (game.ball.vel.x < 0){
                game.ball.vel.x = Clamp(game.ball.vel.x * -game.ball.delta_x_vel, 0, game.ball.max_x_vel);

                float dy = (game.ball.rect.y + game.ball.rect.height/2) - (game.player1.rect.y + game.player1.rect.height/2);
                game.ball.vel.y = (dy/(game.player1.rect.height/2))*game.player1.slip_vel;
                game.ball.rect.x = game.player1.rect.x + game.player1.rect.width + 1;
                game.player1.vel = Clamp(game.player1.vel * game.player1.delta_vel, 0, PLAYER_MAX_SPEED);
                if (game.mode == Singleplayer){
                    game.player1.score += 1;
                    game.text_changed = true;
                }
            }
        }
        if (game.mode != Singleplayer && CheckCollisionRecs(game.player2.rect, game.ball.rect)){
            if (game.ball.vel.x > 0){
                game.ball.vel.x = -Clamp(game.ball.vel.x * game.ball.delta_x_vel, 0, game.ball.max_x_vel);

                float dy = (game.ball.rect.y + game.ball.rect.height/2) - (game.player2.rect.y + game.player2.rect.height/2);
                game.ball.vel.y = (dy/(game.player2.rect.height/2))*game.player2.slip_vel;
                game.ball.rect.x = game.player2.rect.x - 1;
                game.player2.vel = Clamp(game.player2.vel * game.player2.delta_vel, 0, PLAYER_MAX_SPEED);
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

        DrawRectangleRec(game.ball.rect, RED);
        DrawRectangleRec(game.player1.rect, WHITE);
        if (game.mode != Singleplayer){
            DrawRectangleRec(game.player2.rect, WHITE);
            DrawText(game.player1.score_buf, 10, 10, FONT_SIZE, WHITE);
        }
        DrawText(game.player2.score_buf, game.score2_x, 10, FONT_SIZE, WHITE);
        if (game.paused) DrawText(PAUSED_TEXT, game.paused_x, 10, FONT_SIZE, ORANGE);
    }
    EndDrawing();
}

int main(int argc, char *argv[])
{
    uint64_t max_score;
    double ball_x_speed, ball_speedup, ball_max_speed, player_speed, player_slippiness, player_speedup, player_max_speed;
    clags_choice_t *pmode = NULL;
    clags_config_t *help = NULL;

    clags_choice_t modes[] = {
        [Singleplayer] = {"single", "singleplayer with only one paddle"},
        [Computer] = {"computer", "singleplayer vs the computer"},
        [Multiplayer] = {"multi", "multiplayer for two players"},
    };
    clags_choices_t choices = clags_choices(modes);

    clags_range_t player_speed_range = clags_real_range(100, PLAYER_MAX_SPEED);
    clags_range_t ball_speed_range   = clags_real_range(100, BALL_MAX_SPEED);
    clags_range_t speedup_range = clags_real_range(0, 2);

    clags_arg_t args[] = {
        clags_option('m', "mode", &pmode, "MODE", "the game mode", .value_type=Clags_Choice, .choices=&choices, .default_input="single"),
        clags_option('s', "max-score", &max_score, "SCORE", "the score to reach (0 means infinite)", .value_type=Clags_UInt, .default_input="0"),
        clags_option(0, "player-speed", &player_speed, "VEL", "the speed with which the players move", .value_type=Clags_Real, .range=&player_speed_range, .default_input="500"),
        clags_option(0, "player-speedup", &player_speedup, "FACT", "the factor by which the player changes every hit", .value_type=Clags_Real, .range=&speedup_range, .default_input="1"),
        clags_option(0, "player-max-speed", &player_max_speed, "VEL", "the maximum player velocity", .value_type=Clags_Real, .range=&player_speed_range, .default_input="1000"),
        clags_option(0, "player-slippiness", &player_slippiness, "VEL", "the maximum slippiness velocity of a player", .value_type=Clags_Real, .range=&player_speed_range, .default_input="500"),
        clags_option(0, "ball-speed", &ball_x_speed, "VEL", "the initial speed of the ball", .value_type=Clags_Real, .range=&ball_speed_range, .default_input="800"),
        clags_option(0, "ball-speedup", &ball_speedup, "FACT", "the factor by which the ball changes every hit", .value_type=Clags_Real, .range=&speedup_range, .default_input="1.05"),
        clags_option(0, "ball-max-speed", &ball_max_speed, "VEL", "the maximum ball velocity", .value_type=Clags_Real, .range=&ball_speed_range, .default_input="1200"),
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

    game.mode              = clags_choice_index(&choices, pmode);
    game.max_score         = (size_t) max_score;

    game.ball.vel          = (Vector2){-(float)ball_x_speed, (float)GetRandomValue(-RANDOM_START_VEL, RANDOM_START_VEL)};
    game.ball.init_x_vel   = (float) ball_x_speed;
    game.ball.delta_x_vel  = (float) ball_speedup;
    game.ball.max_x_vel    = (float) ball_max_speed;

    game.player1.vel       = game.player2.vel       = game.player1.init_vel = game.player2.init_vel =  (float) player_speed;
    game.player1.max_vel   = game.player2.max_vel   = (float) player_max_speed;
    game.player1.delta_vel = game.player2.delta_vel = (float) player_speedup;
    game.player1.slip_vel  = game.player2.slip_vel  = (float) player_slippiness;

    InitWindow(WIDTH, HEIGHT, "Pong");
    SetTargetFPS(60);
    SetRandomSeed(time(NULL));

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(update_frame, 0, 1);
#else
    while (!WindowShouldClose()){
        update_frame();
    }
#endif // PLATFORM_WEB
    return 0;
}

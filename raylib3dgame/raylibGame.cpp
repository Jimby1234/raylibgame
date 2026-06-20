#include "raylib.h"
#include "vector"
#include "iostream"
#include "string"

#define MAX_BULLETS 1000
#define MAX_ENEMIES 1000
#define MAX_POWERS 20

struct Bullet
{
    Vector2 pos;
    Vector2 vel;
    bool alive;
};

struct Enemy
{
    Vector2 pos;
    Vector2 vel;
    float speed;
    float health;
    float experience;
    bool alive;
    float dmg;
};

struct Player {
    Vector2 position;
    float health;
    float speed;
    float lvlExperience;
    float totalExperience;
    Vector2 size;
    float maxReloadTime;
    float reloadTime;
    float dmg;
    int powers[MAX_POWERS];
};

//bullet setup
Bullet bullets[MAX_BULLETS];
Enemy enemies[MAX_ENEMIES];

void shoot(Vector2 playerPosition, Vector2 size, Vector2 mousePos)
{
    float angle = atan2f(mousePos.y - playerPosition.y, mousePos.x - playerPosition.x); 

    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (!bullets[i].alive)
        {
            bullets[i].pos.x = playerPosition.x + size.x / 2;
            bullets[i].pos.y = playerPosition.y + size.y / 2;
            bullets[i].vel = { cosf(angle) * 500, sinf(angle) * 500 };
            bullets[i].alive = true;
            break;
        }
    }
}

float updateEnemies(Player &player, float dt)
{
    float xp = 0.0f;
    Rectangle playerRect = { player.position.x, player.position.y, player.size.x, player.size.y };
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (!enemies[i].alive) continue;

        float angle = atan2f(player.position.y - enemies[i].pos.y, player.position.x - enemies[i].pos.x);

        enemies[i].vel.x = cosf(angle) * enemies[i].speed;
        enemies[i].vel.y = sinf(angle) * enemies[i].speed;

        enemies[i].pos.x += enemies[i].vel.x * dt;
        enemies[i].pos.y += enemies[i].vel.y * dt;

        if (CheckCollisionCircleRec(enemies[i].pos, 10, playerRect))
        {
            enemies[i].alive = false;
            enemies[i].health = 0.0f;
            player.health -= enemies[i].dmg;
        }

        for (int k = 0; k < MAX_BULLETS; k++)
        {
            if (bullets[k].alive and CheckCollisionCircles(enemies[i].pos, 10, bullets[k].pos, 10))
            {
                enemies[i].health -= player.dmg;
                bullets[k].alive = false;

                if (enemies[i].health <= 0.0f)
                {
                    enemies[i].alive = false;
                    xp += enemies[i].experience;
                }
            }
        }
    }
    return xp;
}

void spawnEnemy(float difficulty)
{
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (!enemies[i].alive)
        {
            int side = GetRandomValue(0, 3);

            if (side == 0)
            {
                enemies[i].pos.x = GetRandomValue(0, GetScreenWidth());
                enemies[i].pos.y = -20;
            }
            else if (side == 1)
            {
                enemies[i].pos.x = GetRandomValue(0, GetScreenWidth());
                enemies[i].pos.y = GetScreenHeight() + 20;
            }
            else if (side == 2)
            {
                enemies[i].pos.x = -20;
                enemies[i].pos.y = GetRandomValue(0, GetScreenHeight());
            }
            else
            {
                enemies[i].pos.x = GetScreenWidth() + 20;
                enemies[i].pos.y = GetRandomValue(0, GetScreenHeight());
            }

            enemies[i].speed = 30.0f + difficulty * 5.0f;
            enemies[i].health = 3.0f + difficulty * 1.0f;
            enemies[i].dmg = 10.0f + difficulty * 2.0f;
            enemies[i].alive = true;
            enemies[i].experience = 5.0f;
            break;
        }
    }
}

void updateBullets(float dt)
{
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (!bullets[i].alive) continue;

        bullets[i].pos.x += bullets[i].vel.x * dt;
        bullets[i].pos.y += bullets[i].vel.y * dt;

        if (bullets[i].pos.x < 0 || bullets[i].pos.x > GetScreenWidth() ||
            bullets[i].pos.y < 0 || bullets[i].pos.y > GetScreenHeight())
        {
            bullets[i].alive = false;
        }
    }
}

void drawBullets()
{
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (bullets[i].alive)
        {
            DrawCircleV(bullets[i].pos, 10, BLUE);
        }
    }
}

void drawEnemies()
{
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (enemies[i].alive)
        {
            DrawCircleV(enemies[i].pos, 10, RED);
        }
    }
}

int main(void)
{
    //play = the actual game
    //lvlup = leveling up
    std::string state = "play";
    //window set up
    const int screenWidth = 700;
    const int screenHeight = 700;

    InitWindow(screenWidth, screenHeight, "run");
    SetTargetFPS(60);

    //some button rects
    Rectangle btn1 = { screenWidth / 3 - 60, screenHeight / 2 - 20, 120, 40 };
    Rectangle btn2 = { screenWidth / 3 * 2 - 60, screenHeight / 2 - 20, 120, 40 };

    //player variables
    Player player;
    player.speed = 150.0f;
    player.size = { 30,30 };
    player.position = { (float)screenWidth / 2 - player.size.x / 2, (float)screenHeight / 2 - player.size.y / 2 };
    player.lvlExperience = 0.0f;
    player.totalExperience = 0.0f;
    player.maxReloadTime = 0.3f;
    player.reloadTime = 0.0f;
    player.health = 100.0f;
    player.dmg = 1.0f;

    float spawnInterval = 2.0f;
    float totalTime = 0.0f;
    int spawnAmount = 1;
    // Main game loop
    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        
        

        Vector2 mousePos = GetMousePosition();

        //LOGIC
        if (state == "play")
        {
            player.position.x += (IsKeyDown(KEY_D) - IsKeyDown(KEY_A)) * player.speed * dt;
            player.position.y += (IsKeyDown(KEY_S) - IsKeyDown(KEY_W)) * player.speed * dt;
            player.reloadTime += dt;

            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) and player.reloadTime >= player.maxReloadTime)
            {
                shoot(player.position, player.size, mousePos);
                player.reloadTime = 0.0f;
            }

            updateBullets(dt);
            float xp = updateEnemies(player, dt);
            player.lvlExperience += xp;
            player.totalExperience += xp;

            totalTime += dt;

            if (totalTime >= spawnInterval)
            {
                for (int i = 0; i < spawnAmount; i++)
                {
                    spawnEnemy(player.totalExperience / 200.0f);
                }
                totalTime = 0;
                spawnInterval = fmaxf(0.3f, 2.0f - player.totalExperience / 500.0f);
                spawnAmount = 1 + (int)(player.totalExperience / 200.0f);
            }

            if (player.lvlExperience >= 50.0f)
            {
                state = "lvlup";
            }

            else if (player.health <= 0.0f)
            {
                state = "dead";
            }
        }
        else if(state == "lvlup")
        {
            if (CheckCollisionPointRec(mousePos, btn1) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                player.maxReloadTime *= 0.9;
                player.lvlExperience = 0.0f;
                state = "play";

            }
            else if(CheckCollisionPointRec(mousePos, btn2) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                player.dmg += 0.5;
                player.lvlExperience = 0.0f;
                state = "play";
            }
        }

        else if (state == "dead")
        {
            player.speed = 150.0f;
            player.size = { 30,30 };
            player.position = { GetScreenWidth() / 2 - player.size.x / 2, GetScreenHeight() / 2 - player.size.y / 2 };
            player.lvlExperience = 0.0f;
            player.totalExperience = 0.0f;
            player.maxReloadTime = 0.3f;
            player.reloadTime = 0.0f;
            player.health = 100.0f;

            // clear bullets
            for (int i = 0; i < MAX_BULLETS; i++)
            {
                bullets[i].alive = false;
            }

            // clear enemies
            for (int i = 0; i < MAX_ENEMIES; i++)
            {
                enemies[i].alive = false;
            }

            totalTime = 0.0f;
            state = "play";
        }
        

        //DRAWING
        BeginDrawing();
        ClearBackground(DARKGRAY);

        drawBullets();
        drawEnemies();

        DrawRectangleV(player.position, player.size, GREEN);

        DrawFPS(20, 20);
        std::string xpText = "xp: " + std::to_string(player.lvlExperience);
        DrawText(xpText.c_str(), screenWidth - 200, screenHeight-40, 20, BLACK);
        std::string hpText = "hp: " + std::to_string(player.health);
        DrawText(hpText.c_str(), 40, screenHeight - 40, 20, BLACK);

        if (state == "lvlup")
        {

            DrawRectangleRec(btn1, { 244, 0, 167, 255 });
            DrawRectangleRec(btn2, { 244, 0, 167, 255 });
            DrawText("less reload", screenWidth / 3 - 40, screenHeight / 2 - 10, 20, BLACK);
            DrawText("more damage", screenWidth / 3 * 2 - 40, screenHeight / 2 - 10, 20, BLACK);
        }


        EndDrawing();
    }

    CloseWindow();

    return 0;
}
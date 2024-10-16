#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <memory.h>
#include "Game.h"



bool CheckCollisionHitBox(HitBox* h1, HitBox* h2)
{
    // if the bounding boxes dont intersect, then neither will the finer boxes
    if(!CheckCollisionRecs(*(h1->bounding), *(h2->bounding)))
    {
        return false;
    }
    for(int i = 0; i < h1->numBoxes; i++)
    {
        for(int j = 0; j < h2->numBoxes; j++)
        {
            Rectangle absH1 = h1->boxes[i];
            absH1.x += h1->bounding->x;
            absH1.y += h1->bounding->y;
            Rectangle absH2 = h2->boxes[j];
            absH2.x += h2->bounding->x;
            absH2.y += h2->bounding->y;
            if(CheckCollisionRecs(absH1, absH2))
            {
                return true;
            }
        }
    }
    return false;
}


void drawHitbox(HitBox* h1)
{
    for(int i = 0; i < h1->numBoxes; i++)
    {

        Rectangle absH1 = h1->boxes[i];
        absH1.x += h1->bounding->x;
        absH1.y += h1->bounding->y;
        
        DrawRectangleRec(absH1, RED);
        
    }
}

void flipHitbox(HitBox* h1)
{
    for(int i = 0; i < h1->numBoxes; i++)
    {
        h1->boxes[i].y = h1->bounding->height - h1->boxes[i].y - h1->boxes[i].height;
    }
}



void game_reset(Game* game)
{
    game->gameOver = 0;
    // bat
    game->currSprite = game->bat_down_t;
    game->flapTime = GetTime();
    

    game->bat.x = -game->bat.width/2;
    game->bat.y = -game->bat.height/2;


    // obstacles
    game->lastObstacle = GetTime();
    game->screenHeight = GetScreenHeight();
    game->screenWidth = GetScreenWidth();


    game->worldWidth = ((UNITS)*(float)game->screenWidth)/(game->screenHeight);

    game->camera.zoom = 1;
    game->camera.offset.x = game->screenWidth/2;
    game->camera.offset.y = game->screenHeight/2;
    game->camera.target.x = 0;
    game->camera.target.y = 0;
    game->camera.rotation = 0;

    
    game->velocity = 0;




    game->lastProjectile = GetTime();

    game->score = 0;

    memset(&game->projectiles, 0, sizeof(game->projectiles));
    memset(&game->obstacles, 0, sizeof(game->obstacles));


}

int game_init(Game* game)
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    InitWindow(DEFAULT_WIDTH, DEFAULT_HEIGHT, "Flappy Bat");

    //SetRandomSeed(time(NULL));
    SetRandomSeed(0);



    game->monitorWidth = GetMonitorWidth(0);
    game->monitorHeight = GetMonitorHeight(0);

    SetTargetFPS(240);

    game->gameOver = false;

    // load sprites
    if(!(game->atlasInfo = TextureAtlas_parse("atlas.rtpa")))
    {
        return -1;
    }
    
    game->atlas = LoadTexture(game->atlasInfo->info.imagePath);
    TextureInfo* sprite;
    if(!(sprite = (TextureInfo*)hash_table_get(game->atlasInfo->textures, &(const char*){"small_rock_t"})))
    {
        fprintf(stderr, "Error could not find sprite: small_rock_t\n");
        return -1;
    }
    game->rocks[0].x = sprite->positionX;
    game->rocks[0].y = sprite->positionY;
    game->rocks[0].width = sprite->trimRecWidth;
    game->rocks[0].height = sprite->trimRecHeight;
    if(!(sprite = (TextureInfo*)hash_table_get(game->atlasInfo->textures, &(const char*){"medium_rock_t"})))
    {
        fprintf(stderr, "Error could not find sprite: medium_rock_t\n");
        return -1;
    }
    game->rocks[1].x = sprite->positionX;
    game->rocks[1].y = sprite->positionY;
    game->rocks[1].width = sprite->trimRecWidth;
    game->rocks[1].height = sprite->trimRecHeight;
    if(!(sprite = (TextureInfo*)hash_table_get(game->atlasInfo->textures, &(const char*){"big_rock_t"})))
    {
        fprintf(stderr, "Error could not find sprite: big_rock_t\n");
        return -1;
    }
    game->rocks[2].x = sprite->positionX;
    game->rocks[2].y = sprite->positionY;
    game->rocks[2].width = sprite->trimRecWidth;
    game->rocks[2].height = sprite->trimRecHeight;
    if(!(sprite = (TextureInfo*)hash_table_get(game->atlasInfo->textures, &(const char*){"bat_down_t"})))
    {
        fprintf(stderr, "Error could not find sprite: bat_down_t\n");
        return -1;
    }
    game->bat_down_t.x = sprite->positionX;
    game->bat_down_t.y = sprite->positionY;
    game->bat_down_t.width = sprite->trimRecWidth;
    game->bat_down_t.height = sprite->trimRecHeight;
    if(!(sprite = (TextureInfo*)hash_table_get(game->atlasInfo->textures, &(const char*){"bat_up_t"})))
    {
        fprintf(stderr, "Error could not find sprite: bat_up_t\n");
        return -1;
    }
    game->bat_up_t.x = sprite->positionX;
    game->bat_up_t.y = sprite->positionY;
    game->bat_up_t.width = sprite->trimRecWidth;
    game->bat_up_t.height = sprite->trimRecHeight;
    

    // bat
    game->currSprite = game->bat_down_t;
    game->flapTime = GetTime();
    
    game->bat.width = game->bat_up_t.width * (UNITS/((float)game->atlasInfo->info.width)) * 0.3;    
    game->bat.height = game->bat_up_t.height * (UNITS/((float)game->atlasInfo->info.height)) * 0.3;
    game->bat.x = -game->bat.width/2;
    game->bat.y = -game->bat.height/2;

    game->batHitbox.bounding = &game->bat;
    game->batHitbox.numBoxes = 4;
    //right wing
    game->batHitbox.boxes[0] = (Rectangle){.x = game->bat.width*0.73, . y = game->bat.height * 0.75, .width = game->bat.width*0.27, .height = game->bat.height*0.25};
    // left wing part 1
    game->batHitbox.boxes[1] = (Rectangle){.x = game->bat.width*0.25, . y = game->bat.height * 0.5, .width = game->bat.width*0.25, .height = game->bat.height*0.5};
    // left wing part 2
    game->batHitbox.boxes[2] = (Rectangle){.x = 0, . y = game->bat.height * 0.58, .width = game->bat.width*0.25, .height = game->bat.height*0.25};

    // body
    game->batHitbox.boxes[3] = (Rectangle){.x = game->bat.width*0.46, .y = 0, .width = game->bat.width*0.27, .height = game->bat.height};

    // obstacles
    game->lastObstacle = GetTime();
    game->rockWidth = 30;
    game->rockHeights[0] = 28;
    game->rockHeights[1] = 68;
    game->rockHeights[2] = 108;

    // hitboxes for small rock
    game->rockHitboxes[0].numBoxes = 3;
    game->rockHitboxes[0].boxes[0] = (Rectangle){.x = 0, .y = game->rockHeights[0]*0.66, .width = game->rockWidth, .height = game->rockHeights[0]*0.34};
    game->rockHitboxes[0].boxes[1] = (Rectangle){.x = game->rockWidth*0.08, .y = game->rockHeights[0]*0.33, .width = game->rockWidth*0.78, .height = game->rockHeights[0]*0.33};
    game->rockHitboxes[0].boxes[2] = (Rectangle){.x = game->rockWidth*0.27, .y = game->rockHeights[0]*0, .width = game->rockWidth*0.39, .height = game->rockHeights[0]*0.33};


    // hit boxes for medium rock
    game->rockHitboxes[1].numBoxes = 3;
    game->rockHitboxes[1].boxes[0] = (Rectangle){.x = game->rockWidth*0.04, .y = game->rockHeights[1]*0.66, .width = game->rockWidth*0.94, .height = game->rockHeights[1]*0.34};
    game->rockHitboxes[1].boxes[1] = (Rectangle){.x = game->rockWidth*0.08, .y = game->rockHeights[1]*0.33, .width = game->rockWidth*0.77, .height = game->rockHeights[1]*0.33};
    game->rockHitboxes[1].boxes[2] = (Rectangle){.x = game->rockWidth*0.25, .y = game->rockHeights[1]*0.0, .width = game->rockWidth*0.4, .height = game->rockHeights[1]*0.33};


    // hit boxes for big rock
    game->rockHitboxes[2].numBoxes = 4;
    game->rockHitboxes[2].boxes[0] = (Rectangle){.x = 0, .y = game->rockHeights[2]*0.66, .width = game->rockWidth, game->rockHeights[2]*0.34};
    game->rockHitboxes[2].boxes[1] = (Rectangle){.x = game->rockWidth*0.15, .y = game->rockHeights[2]*0.33, .width = game->rockWidth*0.77, game->rockHeights[2]*0.33};
    game->rockHitboxes[2].boxes[2] = (Rectangle){.x = game->rockWidth*0.21, .y = game->rockHeights[2]*0.18, .width = game->rockWidth*0.53, game->rockHeights[2]*0.15};
    game->rockHitboxes[2].boxes[3] = (Rectangle){.x = game->rockWidth*0.34, .y = game->rockHeights[2]*0.0, .width = game->rockWidth*0.28, game->rockHeights[2]*0.18};


    game->screenHeight = GetScreenHeight();
    game->screenWidth = GetScreenWidth();

    game->lightMask = LoadRenderTexture(game->screenWidth, game->screenHeight);

    game->worldWidth = ((UNITS)*(float)game->screenWidth)/(game->screenHeight);

    game->camera.zoom = 1;
    game->camera.offset.x = game->screenWidth/2;
    game->camera.offset.y = game->screenHeight/2;
    game->camera.target.x = 0;
    game->camera.target.y = 0;
    game->camera.rotation = 0;


    game->flapVelocity = 70;
    game->gravity = -100;
    
    game->velocity = 0;
    game->acceleration = -game->gravity;


    game->obstacleVelocity = -30;
    game->obstacleSpacing = 160;

    game->projectileSpawnTime = 1.4;
    game->lastProjectile = GetTime();
    game->projectileVelocity = 40;

    game->score = 0;

    game->newGame = (Rectangle){.x = -25, .y = -5, .width = 45, .height = 10};

    return 0;
}

void game_play(Game* game)
{
    while(!WindowShouldClose())
    {

        game_update(game);
        BeginDrawing();
        game_draw(game);
        EndDrawing();
    }
}
void game_draw(Game* game)
{
    ClearBackground((Color) {100,100,100,255});

    BeginMode2D(game->camera);
    // draw obstacles

    for(int i = 0; i < (sizeof(game->obstacles)/sizeof(Obstacle)); i++)
    {

        if(game->obstacles[i].type == INVALID)
        {
            continue;
        }


        DrawTexturePro(game->atlas, game->obstacles[i].stalactiteSprite, game->obstacles[i].stalactite, (Vector2){.x=0,.y=0}, 0, WHITE);
        DrawTexturePro(game->atlas, game->obstacles[i].stalagmiteSprite, game->obstacles[i].stalagmite, (Vector2){.x=0,.y=0}, 0, WHITE);

        

    }
    // draw bat
    DrawTexturePro(game->atlas, game->currSprite, game->bat, (Vector2){.x=0,.y=0}, 0, WHITE);



    EndMode2D();

    // draw lightmask over screen
    // rectangle height has to be negated
    DrawTexturePro(game->lightMask.texture, 
    (Rectangle){.x=0, .y=0, .width=game->screenWidth, .height=-game->screenHeight},
    (Rectangle){.x=0, .y=0, .width=game->screenWidth, .height=game->screenHeight},
    Vector2Zero(), 0, WHITE);


    // ui
    BeginMode2D(game->camera);
    Vector2 size = MeasureTextEx(GetFontDefault(), TextFormat("Score: %d", game->score), 10, 3);
    DrawTextPro(GetFontDefault(),  TextFormat("Score: %d", game->score), 
    (Vector2){.x = -size.x/2, .y = -85}, Vector2Zero(), 0, 10, 3, WHITE);
    if(game->gameOver)
    {
        DrawRectangleRec(game->newGame, LIGHTGRAY);
        size = MeasureTextEx(GetFontDefault(), "New Game", 5, 3);
        DrawTextPro(GetFontDefault(), "New Game", 
        (Vector2){.x = game->newGame.x + game->newGame.width/2 - size.x/2, .y = game->newGame.y+ game->newGame.height/2 - size.y/2}, Vector2Zero(),
        0, 5, 3, WHITE);
    }
    EndMode2D();



}

void game_update(Game* game)
{



    float dt = GetFrameTime();
    game->screenHeight = GetScreenHeight();
    game->screenWidth = GetScreenWidth();
    if(IsWindowResized())
    {
        UnloadRenderTexture(game->lightMask);
        game->lightMask = LoadRenderTexture(game->screenWidth, game->screenHeight);
    }

    game->worldWidth = ((UNITS)*(float)game->screenWidth)/(game->screenHeight);

    game->camera.offset.x = game->screenWidth/2;
    game->camera.offset.y = game->screenHeight/2;
    game->camera.zoom = (float)game->screenHeight/UNITS;



    if(game->gameOver)
    {
        
        BeginTextureMode(game->lightMask);

        ClearBackground(BLACK);

        BeginMode2D(game->camera);


        // background is black, so (0, 0, 0, 255)
        // blend equation RL_MIN takes component wise minimum of source and destination color
        // destination is background, which has 0 for r,g, and b values, which is the smallest possible value
        // this means that the source color cannot have any affect
        // the alpha of destination is at the maximum, so any other alpha value will become the new alpha value
        // all in all, this results in rendering
        rlSetBlendFactors(RL_SRC_ALPHA, RL_SRC_ALPHA, RL_MIN);
        rlSetBlendMode(BLEND_CUSTOM);
        
        

        for(int i = 0; i < MAX_PROJECTILES; i++)
        {
            if(game->projectiles[i].active)
            {
                DrawCircleGradient(game->projectiles[i].x, game->projectiles[i].y, game->projectiles[i].radius,
                ColorAlpha(WHITE, 0), WHITE);
            }
        }

        float radius = game->bat.height > game->bat.width ? game->bat.height : game->bat.width;
        DrawCircleGradient(game->bat.x+game->bat.width/2, game->bat.y + game->bat.height/2, radius,
                ColorAlpha(WHITE, 0), WHITE);
        
        

        rlSetBlendMode(BLEND_ALPHA);

        EndMode2D();

        EndTextureMode();

        Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), game->camera);
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mousePos, game->newGame))
        {
            game_reset(game);
        }

        return;
    }

    // update bat physics
    game->velocity += game->acceleration * dt;
    game->bat.y += game->velocity * dt;
    double flapDT = (GetTime() - game->flapTime);
    if(flapDT > 0.1)
    {
        game->currSprite = game->bat_down_t;
    }
    // update obstacles
    
    double obstacleDT = GetTime() - game->lastObstacle;
    // spawn obstacle
    if(obstacleDT * -game->obstacleVelocity > game->obstacleSpacing)
    {
        int index = GetRandomValue(0, 2);
        game->obstacles[game->freeObstacle].scored = 0;
        game->obstacles[game->freeObstacle].stalactiteSprite = game->rocks[2-index];
        // negate the height so it gets flipped
        game->obstacles[game->freeObstacle].stalactiteSprite.height *= -1;

        game->obstacles[game->freeObstacle].stalagmiteSprite = game->rocks[index];

        game->obstacles[game->freeObstacle].stalagmite.width = game->rockWidth;
        game->obstacles[game->freeObstacle].stalagmite.height = game->rockHeights[index];
        game->obstacles[game->freeObstacle].stalagmite.y = UNITS/2 - game->obstacles[game->freeObstacle].stalagmite.height;
        game->obstacles[game->freeObstacle].stalagmite.x = game->worldWidth/2;

        game->obstacles[game->freeObstacle].stalactite.width = game->rockWidth;
        game->obstacles[game->freeObstacle].stalactite.height = game->rockHeights[2-index];
        game->obstacles[game->freeObstacle].stalactite.y = -UNITS/2;
        game->obstacles[game->freeObstacle].stalactite.x = game->worldWidth/2;

        game->obstacles[game->freeObstacle].type = index + 1;

        game->obstacles[game->freeObstacle].stalactiteHitbox = game->rockHitboxes[2-index];
        game->obstacles[game->freeObstacle].stalactiteHitbox.bounding = &game->obstacles[game->freeObstacle].stalactite;
        // stalactite is upside down, so hitbox must be flipped
        flipHitbox(&game->obstacles[game->freeObstacle].stalactiteHitbox);


        game->obstacles[game->freeObstacle].stalagmiteHitbox = game->rockHitboxes[index];
        game->obstacles[game->freeObstacle].stalagmiteHitbox.bounding = &game->obstacles[game->freeObstacle].stalagmite;

        
        game->freeObstacle = (game->freeObstacle + 1) % ((sizeof(game->obstacles)/sizeof(Obstacle)));

        game->lastObstacle = GetTime();
    }

    
    for(int i = 0; i < (sizeof(game->obstacles)/sizeof(Obstacle)); i++)
    {
        if(game->obstacles[i].type == INVALID)
        {
            continue;
        }
        game->obstacles[i].stalactite.x += game->obstacleVelocity * dt;
        game->obstacles[i].stalagmite.x += game->obstacleVelocity * dt;
    }

    // update projectiles
    for(int i = 0; i < MAX_PROJECTILES; i++)
    {
        if(game->projectiles[i].active)
        {
            game->projectiles[i].x += game->projectileVelocity * dt;
        }
    }


    // check for loss

    // check if bat is off the screen
    if(game->bat.y < -UNITS/2 - game->bat.height/2 || game->bat.y > UNITS/2 - game->bat.height/2)
    {
        game->gameOver = true;
    }

    for(int i = 0; i < (sizeof(game->obstacles)/sizeof(Obstacle)); i++)
    {
        if(game->obstacles[i].type == INVALID)
        {
            continue;
        }
        if(CheckCollisionHitBox(&game->obstacles[i].stalactiteHitbox, &game->batHitbox) || CheckCollisionHitBox(&game->obstacles[i].stalagmiteHitbox, &game->batHitbox))
        {
            game->gameOver = true;

        }
        // check for score increase
        // rectangle between the two obstacles
        Rectangle scoreZone = {.x = game->obstacles[i].stalactite.x+game->rockWidth*0.2,
            .y = game->obstacles[i].stalactite.y + game->obstacles[i].stalactite.height,
            .width = game->rockWidth*0.6,
            .height = -(game->obstacles[i].stalactite.y + game->obstacles[i].stalactite.height - game->obstacles[i].stalagmite.y)};
        HitBox scoreHitbox;
        scoreHitbox.bounding = &scoreZone;
        scoreHitbox.numBoxes = 1;
        scoreHitbox.boxes[0] = (Rectangle){.x = 0, .y =0, .width = scoreZone.width, .height = scoreZone.height}; 
        if(!game->obstacles[i].scored && CheckCollisionHitBox(&scoreHitbox, &game->batHitbox))
        {
            game->score++;
            game->obstacles[i].scored = 1;
        }
    }



    double projDT = GetTime() - game->lastProjectile;
    // user input
    if(IsKeyPressed(KEY_SPACE) && flapDT > 0.2)
    {
        game->velocity = -game->flapVelocity;
        game->currSprite = game->bat_up_t;
        game->flapTime = GetTime();
    }
    if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && projDT > game->projectileSpawnTime)
    {
        game->lastProjectile = GetTime();
        
        game->projectiles[game->freeProjectile].active = 1;
        game->projectiles[game->freeProjectile].radius = 40;
        game->projectiles[game->freeProjectile].x = game->bat.x + game->bat.width/2;
        game->projectiles[game->freeProjectile].y = game->bat.y + game->bat.height/2;


        game->freeProjectile = (game->freeProjectile + 1) % MAX_PROJECTILES;
    }

    BeginTextureMode(game->lightMask);

    ClearBackground(BLACK);

    BeginMode2D(game->camera);


    // background is black, so (0, 0, 0, 255)
    // blend equation RL_MIN takes component wise minimum of source and destination color
    // destination is background, which has 0 for r,g, and b values, which is the smallest possible value
    // this means that the source color cannot have any affect
    // the alpha of destination is at the maximum, so any other alpha value will become the new alpha value
    // all in all, this results in rendering
    rlSetBlendFactors(RL_SRC_ALPHA, RL_SRC_ALPHA, RL_MIN);
    rlSetBlendMode(BLEND_CUSTOM);
    
    

    for(int i = 0; i < MAX_PROJECTILES; i++)
    {
        if(game->projectiles[i].active)
        {
            DrawCircleGradient(game->projectiles[i].x, game->projectiles[i].y, game->projectiles[i].radius,
            ColorAlpha(WHITE, 0), WHITE);
        }
    }

    float radius = game->bat.height > game->bat.width ? game->bat.height : game->bat.width;
    DrawCircleGradient(game->bat.x+game->bat.width/2, game->bat.y + game->bat.height/2, radius,
            ColorAlpha(WHITE, 0), WHITE);
    
    

    rlSetBlendMode(BLEND_ALPHA);

    EndMode2D();

    EndTextureMode();



}

void game_exit(Game* game)
{

    if(game->atlasInfo)
    {
        TextureAtlas_free(game->atlasInfo);
    }
    UnloadRenderTexture(game->lightMask);  
}
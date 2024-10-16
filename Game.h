#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"
#include "TextureAtlas.h"


#define UNIT_SIZE 16

#define DEFAULT_WIDTH 700
#define DEFAULT_HEIGHT 700


#define UNITS 200


#define MAX_BOXES 16
#define MAX_PROJECTILES 16


typedef struct Circle
{
    float x;
    float y;
    float radius;
    unsigned char active;
} Circle;

typedef struct HitBox
{
    Rectangle boxes[MAX_BOXES];
    Rectangle* bounding;
    int numBoxes;
} HitBox;

bool CheckCollisionHitBox(HitBox* h1, HitBox* h2);



typedef enum ObstacleType {INVALID, LOW_HIGH, MID_MID, HIGH_LOW} ObstacleType;




typedef struct Obstacle
{
    Rectangle stalactiteSprite;
    Rectangle stalagmiteSprite;
    ObstacleType type;
    Rectangle stalactite;
    Rectangle stalagmite;
    HitBox stalactiteHitbox;
    HitBox stalagmiteHitbox;
    unsigned char scored;
} Obstacle;


typedef struct Game
{
    TextureAtlas* atlasInfo;    
    Texture2D atlas;

    Rectangle rocks[3];
    Rectangle bat_down_t;
    Rectangle bat_up_t;

    Rectangle currSprite;
    double flapTime;
    Camera2D camera;

    int screenWidth;
    int screenHeight;


    int monitorWidth;
    int monitorHeight;

    float worldWidth;

    Rectangle bat;
    HitBox batHitbox;
    float velocity;
    float acceleration;

    float flapVelocity;
    float gravity;


    Obstacle obstacles[16];
    int freeObstacle;
    float obstacleVelocity;
    float obstacleSpacing;
    double lastObstacle;

    float rockHeights[3];
    float rockWidth;

    HitBox rockHitboxes[3];

    RenderTexture2D lightMask;
    Circle projectiles[MAX_PROJECTILES];
    int freeProjectile;
    float projectileVelocity;
    double lastProjectile;
    float projectileSpawnTime;



    int score;
    bool gameOver;

    Rectangle newGame;
} Game;


int game_init(Game* game);

void game_play(Game* game);
void game_draw(Game* game);
void game_update(Game* game);

void game_exit(Game* game);

#endif
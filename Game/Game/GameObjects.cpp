#include "GameObjects.h"

// °´Ã¼ Á¤ÀÇ
void InitializePlayer(RECT& player, int x, int y, int size) {
    player.left = x;
    player.top = y;
    player.right = x + size;
    player.bottom = y + size;
}

void InitializeEnemy(RECT& enemy, int x, int y, int size) {
    enemy.left = x;
    enemy.top = y;
    enemy.right = x + size;
    enemy.bottom = y + size;
}

void MoveObject(RECT& object, int dx, int dy) {
    object.left += dx;
    object.right += dx;
    object.top += dy;
    object.bottom += dy;
}

bool CheckCollision(const RECT& obj1, const RECT& obj2) {
    RECT intersection;
    return IntersectRect(&intersection, &obj1, &obj2);
}


#pragma once

#include <windows.h>

// �Լ� ����
void InitializePlayer(RECT& player, int x, int y, int size);
void InitializeEnemy(RECT& enemy, int x, int y, int size);
void MoveObject(RECT& object, int dx, int dy);
bool CheckCollision(const RECT& obj1, const RECT& obj2);



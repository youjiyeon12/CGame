#pragma once

#include <windows.h>
#include <vector>
#include <string>

// 전역 변수 선언
extern RECT g_me, g_item, g_bar, g_box, g_powerUp;
extern std::vector<RECT> g_you, g_obstacles, g_yellowItem, g_greenItem, g_purpleItem;
extern std::vector<HANDLE> g_threads;
extern int g_timer, g_gametime, g_score, s_timer, g_op;
extern bool g_over, g_isPaused, g_hasPowerUp;
extern int g_round, g_yellowRequired, g_greenRequired, g_purpleRequired;
extern int g_yellowCollected, g_greenCollected, g_purpleCollected;
extern int g_powerUpDuration;
extern std::vector<std::wstring> g_keyInstructions;

// 함수 선언
void InitializeItems(); // 아이템
void InitializeObstacles(); // 장애물
DWORD WINAPI MoveOpponent(LPVOID lpParam); // 상대(스레드)
void StartNewRound(HWND hWnd); // 새 라운드
void RestartGame(HWND hWnd); // 게임 재시작(R)
void InitializePlayer(RECT& player, int x, int y, int size); // 나의 객체
void InitializeEnemy(RECT& enemy, int x, int y, int size); // 상대 객체
void MoveObject(RECT& object, int dx, int dy); // 객체 이동 처리
bool CheckCollision(const RECT& obj1, const RECT& obj2); // 충돌 처리
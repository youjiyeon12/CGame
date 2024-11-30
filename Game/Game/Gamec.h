#pragma once

#include <windows.h>
#include <vector>
#include <string>

// ���� ���� ����
extern RECT g_me, g_item, g_bar, g_box, g_powerUp;
extern std::vector<RECT> g_you, g_obstacles, g_yellowItem, g_greenItem, g_purpleItem;
extern std::vector<HANDLE> g_threads;
extern int g_timer, g_gametime, g_score, s_timer, g_op;
extern bool g_over, g_isPaused, g_hasPowerUp;
extern int g_round, g_yellowRequired, g_greenRequired, g_purpleRequired;
extern int g_yellowCollected, g_greenCollected, g_purpleCollected;
extern int g_powerUpDuration;
extern std::vector<std::wstring> g_keyInstructions;

// �Լ� ����
void InitializeItems(); // ������
void InitializeObstacles(); // ��ֹ�
DWORD WINAPI MoveOpponent(LPVOID lpParam); // ���(������)
void StartNewRound(HWND hWnd); // �� ����
void RestartGame(HWND hWnd); // ���� �����(R)
void InitializePlayer(RECT& player, int x, int y, int size); // ���� ��ü
void InitializeEnemy(RECT& enemy, int x, int y, int size); // ��� ��ü
void MoveObject(RECT& object, int dx, int dy); // ��ü �̵� ó��
bool CheckCollision(const RECT& obj1, const RECT& obj2); // �浹 ó��
#include "Gamec.h"
#include <stdlib.h>
#include <time.h>


// 전역 변수 정의
RECT g_me, g_item, g_bar, g_box, g_powerUp;
std::vector<RECT> g_you, g_obstacles, g_yellowItem, g_greenItem, g_purpleItem;
std::vector<HANDLE> g_threads;
int g_timer = 100, g_gametime = 30, g_score = 0, s_timer = 1000, g_op = 1;
bool g_over = false, g_isPaused = false, g_hasPowerUp = false;
int g_round = 1, g_yellowRequired = 3, g_greenRequired = 2, g_purpleRequired = 1;
int g_yellowCollected = 0, g_greenCollected = 0, g_purpleCollected = 0;
int g_powerUpDuration = 0;
std::vector<std::wstring> g_keyInstructions = {
    L"↑: 위로 이동",
    L"↓: 아래로 이동",
    L"←: 왼쪽으로 이동",
    L"→: 오른쪽으로 이동",
    L"Space: 일시정지/재개",
    L"R: 게임 재시작",
    L"주황 아이템 : 무적",
    L"파랑 : 자신",
    L"빨강 : 상대"    
};


// 아이템 처리
// 아이템 개수는 요구 + 3 
void InitializeItems() {
    g_yellowItem.clear();
    g_greenItem.clear();
    g_purpleItem.clear();

    for (int i = 0; i < g_yellowRequired + 4; ++i) {
        RECT item;
        item.left = rand() % (g_box.right - 50);
        item.top = rand() % (g_box.bottom - 50);
        item.right = item.left + 25;
        item.bottom = item.top + 25;
        g_yellowItem.push_back(item);
    }

    for (int i = 0; i < g_greenRequired + 4; ++i) {
        RECT item;
        item.left = rand() % (g_box.right - 50);
        item.top = rand() % (g_box.bottom - 50);
        item.right = item.left + 25;
        item.bottom = item.top + 25;
        g_greenItem.push_back(item);
    }

    for (int i = 0; i < g_purpleRequired + 3; ++i) {
        RECT item;
        item.left = rand() % (g_box.right - 50);
        item.top = rand() % (g_box.bottom - 50);
        item.right = item.left + 25;
        item.bottom = item.top + 25;
        g_purpleItem.push_back(item);
    }

    // 파워업 아이템
    g_powerUp.left = rand() % (g_box.right - 30);
    g_powerUp.top = rand() % (g_box.bottom - 30);
    g_powerUp.right = g_powerUp.left + 15;
    g_powerUp.bottom = g_powerUp.top + 15;
}

// 응답 오류 확인 필요!!!
void InitializeObstacles() {
    g_obstacles.clear();
    const int minDistance = 350;  // 장애물 사이 최소 거리
    const int playerSize = 110;    // 플레이어 크기 + 50 (지나갈 수 있게)
    const int obstacleWidth = 230; // 장애물 가로 크기
    const int obstacleHeight = 20; // 장애물 세로 크기
    const int maxAttempts = 100;   // 최대 시도 횟수 (무한 루프 방지)

    for (int i = 0; i < 4; ++i) {
        RECT obstacle;
        bool validPosition;
        int attempts = 0;

        do {
            validPosition = true;
            attempts++;

            // 장애물이 g_box 경계를 넘지 않도록 위치 계산
            obstacle.left = rand() % (g_box.right - obstacleWidth - playerSize);  // 플레이어 크기만큼 여유
            obstacle.top = rand() % (g_box.bottom - obstacleHeight - playerSize); // 플레이어 크기만큼 여유
            obstacle.right = obstacle.left + obstacleWidth;
            obstacle.bottom = obstacle.top + obstacleHeight;

            // 기존 장애물 및 플레이어와의 거리 확인
            for (const auto& existingObstacle : g_obstacles) {
                int horizontalDist = abs(obstacle.left - existingObstacle.left);
                int verticalDist = abs(obstacle.top - existingObstacle.top);

                // 가로(왼쪽, 오른쪽)와 세로(위, 아래) 방향으로의 최소 거리 체크
                if (horizontalDist < minDistance && verticalDist < minDistance) {
                    validPosition = false;
                    break;
                }

                // 왼쪽 장애물과의 거리 (왼쪽 가장자리 기준)
                int leftDist = abs(obstacle.left - existingObstacle.right);
                // 오른쪽 장애물과의 거리 (오른쪽 가장자리 기준)
                int rightDist = abs(obstacle.right - existingObstacle.left);
                // 위쪽 장애물과의 거리 (위 가장자리 기준)
                int topDist = abs(obstacle.top - existingObstacle.bottom);
                // 아래쪽 장애물과의 거리 (아래 가장자리 기준)
                int bottomDist = abs(obstacle.bottom - existingObstacle.top);

                // 각 방향별로 최소 거리 확인
                if (leftDist < minDistance || rightDist < minDistance || topDist < minDistance || bottomDist < minDistance) {
                    validPosition = false;
                    break;
                }
            }


            // 플레이어와의 거리 확인
            int playerHorizontalDist = abs(obstacle.left - g_me.left);
            int playerVerticalDist = abs(obstacle.top - g_me.top);
            if (playerHorizontalDist < minDistance && playerVerticalDist < minDistance) {
                validPosition = false;
            }

            // 장애물이 g_box의 경계와 너무 가까운지 확인
            if (obstacle.left < playerSize || obstacle.top < playerSize || // 왼쪽 또는 위쪽 경계와 너무 가까운 경우
                obstacle.right > g_box.right - playerSize || obstacle.bottom > g_box.bottom - playerSize) {
                validPosition = false;
            }

            // 최대 시도 횟수를 초과하면 강제로 위치를 추가하도록 설정
            if (attempts >= maxAttempts) {
                validPosition = true;  // 최대 시도 횟수를 초과하면 강제로 위치를 추가하도록 설정
            }

        } while (!validPosition); // 유효한 위치를 찾을 때까지 반복

        // 유효한 위치에 장애물을 추가
        g_obstacles.push_back(obstacle);
    }
}




void StartNewRound(HWND hWnd) {
    if (g_over) {
        return;  // 게임이 끝난 경우 함수 종료
    }

    g_round++;
    if (g_round <= 4) {
        // 요구된 아이템 수 증가
        g_yellowRequired += rand() % 2 + 1;
        g_greenRequired += rand() % 2 + 1;
        g_purpleRequired += rand() % 2 + 1;
        g_yellowCollected = g_greenCollected = g_purpleCollected = 0;

        // 새로운 아이템 초기화
        InitializeItems();

        g_op++;

        // 적 추가
        if (g_op <= 4) {
            RECT newEnemy;
            newEnemy.left = rand() % (g_box.right - 90);
            newEnemy.top = rand() % (g_box.bottom - 90);
            newEnemy.right = newEnemy.left + 90;
            newEnemy.bottom = newEnemy.top + 90;

            // 상대 객체와 플레이어, 그리고 기존 상대 객체들과의 거리 확인
            bool isTooClose = true;
            while (isTooClose) {
                isTooClose = false;
                // 플레이어와의 거리 확인
                int distance = sqrt(pow(newEnemy.left - g_me.left, 2) + pow(newEnemy.top - g_me.top, 2));
                if (distance < 300) {
                    isTooClose = true;
                }
                // 기존 상대 객체들과의 거리 확인
                for (const auto& enemy : g_you) {
                    distance = sqrt(pow(newEnemy.left - enemy.left, 2) + pow(newEnemy.top - enemy.top, 2));
                    if (distance < 200) {
                        isTooClose = true;
                        break;
                    }
                }

                if (isTooClose) {
                    // 너무 가까운 경우 새로운 위치를 계산
                    newEnemy.left = rand() % (g_box.right - 90);
                    newEnemy.top = rand() % (g_box.bottom - 90);
                    newEnemy.right = newEnemy.left + 90;
                    newEnemy.bottom = newEnemy.top + 90;
                }
            }

            g_you.push_back(newEnemy);

            // 기존 적들의 위치 재조정
            for (size_t i = 0; i < g_you.size(); ++i) {
                RECT& enemy = g_you[i];
                int distance = sqrt(pow(enemy.left - g_me.left, 2) + pow(enemy.top - g_me.top, 2));

                if (distance < 200) {
                    enemy.left = rand() % (g_box.right - 90);
                    enemy.top = rand() % (g_box.bottom - 90);
                    enemy.right = enemy.left + 90;
                    enemy.bottom = enemy.top + 90;
                }
            }

            HANDLE hThread = CreateThread(NULL, 0, MoveOpponent, (LPVOID)(intptr_t)(g_you.size() - 1), 0, NULL);
            if (hThread) {
                g_threads.push_back(hThread);
            }
        }

        // 3라운드 이상이면 장애물 배치
        if (g_round >= 3) {
            InitializeObstacles();
        }

        // 게임 시간 초기화
        g_gametime = 30;

        TCHAR szMessage[100];
        wsprintf(szMessage, L"라운드 %d 시작!", g_round);
        MessageBox(hWnd, szMessage, L"새 라운드", MB_OK);

        g_isPaused = false;
    }

    // 4라운드 완료 시 게임 종료
    if (g_round > 4) {
        MessageBox(hWnd, L"축하합니다! 모든 라운드를 완료하셨습니다!", L"게임 완료", MB_OK);
        g_over = true; // 게임 종료 상태 설정
    }
}



DWORD WINAPI MoveOpponent(LPVOID lpParam) {
    int index = (int)(intptr_t)lpParam;

    int dx = rand() % 7 + 4; // 4~10 범위
    int dy = rand() % 7 + 4; // 4~10 범위

    dx *= (rand() % 2 == 0) ? 1 : -1;
    dy *= (rand() % 2 == 0) ? 1 : -1;

    float acceleration = 0.05f; //  가속도
    float maxSpeed = 8.0f; // 최대 속도

    // 객체 크기 저장 (이 크기를 유지)
    int originalWidth = g_you[index].right - g_you[index].left;
    int originalHeight = g_you[index].bottom - g_you[index].top;

    // 초기 속도
    float velocityX = (float)dx;
    float velocityY = (float)dy;

    int directionChangeCounter = 0;

    while (!g_over) {
        if (g_isPaused) {
            Sleep(50);
            continue;
        }

        if (index >= g_you.size()) {
            break;
        }

        // 가속도 적용 
        velocityX += (acceleration * ((rand() % 2 == 0) ? 1 : -1));
        velocityY += (acceleration * ((rand() % 2 == 0) ? 1 : -1));

        // 최대 속도 제한
        if (abs(velocityX) > maxSpeed) velocityX = (velocityX > 0 ? maxSpeed : -maxSpeed);
        if (abs(velocityY) > maxSpeed) velocityY = (velocityY > 0 ? maxSpeed : -maxSpeed);

        // 상대 객체의 이동
        MoveObject(g_you[index], (int)velocityX, (int)velocityY);

        // 벽과의 충돌 처리
        if (g_you[index].left <= g_box.left) velocityX = abs(velocityX);
        if (g_you[index].right >= g_box.right) velocityX = -abs(velocityX);
        if (g_you[index].top <= g_box.top) velocityY = abs(velocityY);
        if (g_you[index].bottom >= g_box.bottom) velocityY = -abs(velocityY);

        // 장애물과의 충돌 처리
        RECT intersection;
        for (const auto& obstacle : g_obstacles) {
            if (IntersectRect(&intersection, &g_you[index], &obstacle)) {
                // 충돌 방향에 따라 이동 제한
                if (intersection.right - intersection.left < intersection.bottom - intersection.top) {
                    // 좌우 충돌
                    if (g_you[index].left < obstacle.left) {
                        g_you[index].right = obstacle.left;
                        g_you[index].left = g_you[index].right - originalWidth; // 크기 유지
                    }
                    else {
                        g_you[index].left = obstacle.right;
                        g_you[index].right = g_you[index].left + originalWidth; // 크기 유지
                    }
                    velocityX = -velocityX;  // 방향 반전
                }
                else {
                    // 상하 충돌
                    if (g_you[index].top < obstacle.top) {
                        g_you[index].bottom = obstacle.top;
                        g_you[index].top = g_you[index].bottom - originalHeight; // 크기 유지
                    }
                    else {
                        g_you[index].top = obstacle.bottom;
                        g_you[index].bottom = g_you[index].top + originalHeight; // 크기 유지
                    }
                    velocityY = -velocityY;  // 방향 반전
                }
                break; // 첫 번째 충돌만 처리하고 종료
            }
        }

        // 일정 시간마다 방향 변경
        directionChangeCounter++;
        if (directionChangeCounter > 30 + (rand() % 20)) { // 30~50 범위로 변경
            velocityX = (rand() % 7 + 4) * ((rand() % 2 == 0) ? 1 : -1);
            velocityY = (rand() % 7 + 4) * ((rand() % 2 == 0) ? 1 : -1);
            directionChangeCounter = 0;
        }

        // 5% 확률로 순간 가속
        if (rand() % 100 < 5) {
            velocityX *= 1.5f;
            velocityY *= 1.5f;
        }

        // 플레이어와 충돌 검사
        if (CheckCollision(g_me, g_you[index])) {
            if (!g_hasPowerUp) {
                g_over = true;
                MessageBox(NULL, L"Game Over", L"게임 오버", MB_OK);
                break;
            }
        }

        // 주어진 시간 동안 대기
        Sleep(20);
    }

    return 0;
}



void RestartGame(HWND hWnd) {
    g_over = false;
    g_isPaused = false;
    g_round = 1;
    g_score = 0;
    g_gametime = 30;
    g_timer = 100;
    g_op = 1;
    g_yellowRequired = 3;
    g_greenRequired = 2;
    g_purpleRequired = 0;
    g_yellowCollected = g_greenCollected = g_purpleCollected = 0;
    g_hasPowerUp = false;
    g_powerUpDuration = 0;

    InitializePlayer(g_me, 50, 50, 60);
    g_you.clear();

    RECT enemy1, enemy2;
    InitializeEnemy(enemy1, 400, 400, 90);
    g_you.push_back(enemy1);

    InitializeItems();
    g_obstacles.clear();

    for (HANDLE hThread : g_threads) {
        TerminateThread(hThread, 0);
        CloseHandle(hThread);
    }
    g_threads.clear();

    for (int i = 0; i < 1; ++i) {
        HANDLE hThread = CreateThread(NULL, 0, MoveOpponent, (LPVOID)(intptr_t)i, 0, NULL);
        if (hThread) {
            g_threads.push_back(hThread);
        }
    }

    SetTimer(hWnd, 3, 1000, NULL);
    SetTimer(hWnd, 4, 1000, NULL);
    SetTimer(hWnd, 5, 1000, NULL);

    InvalidateRect(hWnd, NULL, TRUE);
}

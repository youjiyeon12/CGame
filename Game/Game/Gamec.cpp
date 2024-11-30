#include "Gamec.h"
#include <stdlib.h>
#include <time.h>


// ���� ���� ����
RECT g_me, g_item, g_bar, g_box, g_powerUp;
std::vector<RECT> g_you, g_obstacles, g_yellowItem, g_greenItem, g_purpleItem;
std::vector<HANDLE> g_threads;
int g_timer = 100, g_gametime = 30, g_score = 0, s_timer = 1000, g_op = 1;
bool g_over = false, g_isPaused = false, g_hasPowerUp = false;
int g_round = 1, g_yellowRequired = 3, g_greenRequired = 2, g_purpleRequired = 1;
int g_yellowCollected = 0, g_greenCollected = 0, g_purpleCollected = 0;
int g_powerUpDuration = 0;
std::vector<std::wstring> g_keyInstructions = {
    L"��: ���� �̵�",
    L"��: �Ʒ��� �̵�",
    L"��: �������� �̵�",
    L"��: ���������� �̵�",
    L"Space: �Ͻ�����/�簳",
    L"R: ���� �����",
    L"��Ȳ ������ : ����",
    L"�Ķ� : �ڽ�",
    L"���� : ���"    
};


// ������ ó��
// ������ ������ �䱸 + 3 
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

    // �Ŀ��� ������
    g_powerUp.left = rand() % (g_box.right - 30);
    g_powerUp.top = rand() % (g_box.bottom - 30);
    g_powerUp.right = g_powerUp.left + 15;
    g_powerUp.bottom = g_powerUp.top + 15;
}

// ���� ���� Ȯ�� �ʿ�!!!
void InitializeObstacles() {
    g_obstacles.clear();
    const int minDistance = 350;  // ��ֹ� ���� �ּ� �Ÿ�
    const int playerSize = 110;    // �÷��̾� ũ�� + 50 (������ �� �ְ�)
    const int obstacleWidth = 230; // ��ֹ� ���� ũ��
    const int obstacleHeight = 20; // ��ֹ� ���� ũ��
    const int maxAttempts = 100;   // �ִ� �õ� Ƚ�� (���� ���� ����)

    for (int i = 0; i < 4; ++i) {
        RECT obstacle;
        bool validPosition;
        int attempts = 0;

        do {
            validPosition = true;
            attempts++;

            // ��ֹ��� g_box ��踦 ���� �ʵ��� ��ġ ���
            obstacle.left = rand() % (g_box.right - obstacleWidth - playerSize);  // �÷��̾� ũ�⸸ŭ ����
            obstacle.top = rand() % (g_box.bottom - obstacleHeight - playerSize); // �÷��̾� ũ�⸸ŭ ����
            obstacle.right = obstacle.left + obstacleWidth;
            obstacle.bottom = obstacle.top + obstacleHeight;

            // ���� ��ֹ� �� �÷��̾���� �Ÿ� Ȯ��
            for (const auto& existingObstacle : g_obstacles) {
                int horizontalDist = abs(obstacle.left - existingObstacle.left);
                int verticalDist = abs(obstacle.top - existingObstacle.top);

                // ����(����, ������)�� ����(��, �Ʒ�) ���������� �ּ� �Ÿ� üũ
                if (horizontalDist < minDistance && verticalDist < minDistance) {
                    validPosition = false;
                    break;
                }

                // ���� ��ֹ����� �Ÿ� (���� �����ڸ� ����)
                int leftDist = abs(obstacle.left - existingObstacle.right);
                // ������ ��ֹ����� �Ÿ� (������ �����ڸ� ����)
                int rightDist = abs(obstacle.right - existingObstacle.left);
                // ���� ��ֹ����� �Ÿ� (�� �����ڸ� ����)
                int topDist = abs(obstacle.top - existingObstacle.bottom);
                // �Ʒ��� ��ֹ����� �Ÿ� (�Ʒ� �����ڸ� ����)
                int bottomDist = abs(obstacle.bottom - existingObstacle.top);

                // �� ���⺰�� �ּ� �Ÿ� Ȯ��
                if (leftDist < minDistance || rightDist < minDistance || topDist < minDistance || bottomDist < minDistance) {
                    validPosition = false;
                    break;
                }
            }


            // �÷��̾���� �Ÿ� Ȯ��
            int playerHorizontalDist = abs(obstacle.left - g_me.left);
            int playerVerticalDist = abs(obstacle.top - g_me.top);
            if (playerHorizontalDist < minDistance && playerVerticalDist < minDistance) {
                validPosition = false;
            }

            // ��ֹ��� g_box�� ���� �ʹ� ������� Ȯ��
            if (obstacle.left < playerSize || obstacle.top < playerSize || // ���� �Ǵ� ���� ���� �ʹ� ����� ���
                obstacle.right > g_box.right - playerSize || obstacle.bottom > g_box.bottom - playerSize) {
                validPosition = false;
            }

            // �ִ� �õ� Ƚ���� �ʰ��ϸ� ������ ��ġ�� �߰��ϵ��� ����
            if (attempts >= maxAttempts) {
                validPosition = true;  // �ִ� �õ� Ƚ���� �ʰ��ϸ� ������ ��ġ�� �߰��ϵ��� ����
            }

        } while (!validPosition); // ��ȿ�� ��ġ�� ã�� ������ �ݺ�

        // ��ȿ�� ��ġ�� ��ֹ��� �߰�
        g_obstacles.push_back(obstacle);
    }
}




void StartNewRound(HWND hWnd) {
    if (g_over) {
        return;  // ������ ���� ��� �Լ� ����
    }

    g_round++;
    if (g_round <= 4) {
        // �䱸�� ������ �� ����
        g_yellowRequired += rand() % 2 + 1;
        g_greenRequired += rand() % 2 + 1;
        g_purpleRequired += rand() % 2 + 1;
        g_yellowCollected = g_greenCollected = g_purpleCollected = 0;

        // ���ο� ������ �ʱ�ȭ
        InitializeItems();

        g_op++;

        // �� �߰�
        if (g_op <= 4) {
            RECT newEnemy;
            newEnemy.left = rand() % (g_box.right - 90);
            newEnemy.top = rand() % (g_box.bottom - 90);
            newEnemy.right = newEnemy.left + 90;
            newEnemy.bottom = newEnemy.top + 90;

            // ��� ��ü�� �÷��̾�, �׸��� ���� ��� ��ü����� �Ÿ� Ȯ��
            bool isTooClose = true;
            while (isTooClose) {
                isTooClose = false;
                // �÷��̾���� �Ÿ� Ȯ��
                int distance = sqrt(pow(newEnemy.left - g_me.left, 2) + pow(newEnemy.top - g_me.top, 2));
                if (distance < 300) {
                    isTooClose = true;
                }
                // ���� ��� ��ü����� �Ÿ� Ȯ��
                for (const auto& enemy : g_you) {
                    distance = sqrt(pow(newEnemy.left - enemy.left, 2) + pow(newEnemy.top - enemy.top, 2));
                    if (distance < 200) {
                        isTooClose = true;
                        break;
                    }
                }

                if (isTooClose) {
                    // �ʹ� ����� ��� ���ο� ��ġ�� ���
                    newEnemy.left = rand() % (g_box.right - 90);
                    newEnemy.top = rand() % (g_box.bottom - 90);
                    newEnemy.right = newEnemy.left + 90;
                    newEnemy.bottom = newEnemy.top + 90;
                }
            }

            g_you.push_back(newEnemy);

            // ���� ������ ��ġ ������
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

        // 3���� �̻��̸� ��ֹ� ��ġ
        if (g_round >= 3) {
            InitializeObstacles();
        }

        // ���� �ð� �ʱ�ȭ
        g_gametime = 30;

        TCHAR szMessage[100];
        wsprintf(szMessage, L"���� %d ����!", g_round);
        MessageBox(hWnd, szMessage, L"�� ����", MB_OK);

        g_isPaused = false;
    }

    // 4���� �Ϸ� �� ���� ����
    if (g_round > 4) {
        MessageBox(hWnd, L"�����մϴ�! ��� ���带 �Ϸ��ϼ̽��ϴ�!", L"���� �Ϸ�", MB_OK);
        g_over = true; // ���� ���� ���� ����
    }
}



DWORD WINAPI MoveOpponent(LPVOID lpParam) {
    int index = (int)(intptr_t)lpParam;

    int dx = rand() % 7 + 4; // 4~10 ����
    int dy = rand() % 7 + 4; // 4~10 ����

    dx *= (rand() % 2 == 0) ? 1 : -1;
    dy *= (rand() % 2 == 0) ? 1 : -1;

    float acceleration = 0.05f; //  ���ӵ�
    float maxSpeed = 8.0f; // �ִ� �ӵ�

    // ��ü ũ�� ���� (�� ũ�⸦ ����)
    int originalWidth = g_you[index].right - g_you[index].left;
    int originalHeight = g_you[index].bottom - g_you[index].top;

    // �ʱ� �ӵ�
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

        // ���ӵ� ���� 
        velocityX += (acceleration * ((rand() % 2 == 0) ? 1 : -1));
        velocityY += (acceleration * ((rand() % 2 == 0) ? 1 : -1));

        // �ִ� �ӵ� ����
        if (abs(velocityX) > maxSpeed) velocityX = (velocityX > 0 ? maxSpeed : -maxSpeed);
        if (abs(velocityY) > maxSpeed) velocityY = (velocityY > 0 ? maxSpeed : -maxSpeed);

        // ��� ��ü�� �̵�
        MoveObject(g_you[index], (int)velocityX, (int)velocityY);

        // ������ �浹 ó��
        if (g_you[index].left <= g_box.left) velocityX = abs(velocityX);
        if (g_you[index].right >= g_box.right) velocityX = -abs(velocityX);
        if (g_you[index].top <= g_box.top) velocityY = abs(velocityY);
        if (g_you[index].bottom >= g_box.bottom) velocityY = -abs(velocityY);

        // ��ֹ����� �浹 ó��
        RECT intersection;
        for (const auto& obstacle : g_obstacles) {
            if (IntersectRect(&intersection, &g_you[index], &obstacle)) {
                // �浹 ���⿡ ���� �̵� ����
                if (intersection.right - intersection.left < intersection.bottom - intersection.top) {
                    // �¿� �浹
                    if (g_you[index].left < obstacle.left) {
                        g_you[index].right = obstacle.left;
                        g_you[index].left = g_you[index].right - originalWidth; // ũ�� ����
                    }
                    else {
                        g_you[index].left = obstacle.right;
                        g_you[index].right = g_you[index].left + originalWidth; // ũ�� ����
                    }
                    velocityX = -velocityX;  // ���� ����
                }
                else {
                    // ���� �浹
                    if (g_you[index].top < obstacle.top) {
                        g_you[index].bottom = obstacle.top;
                        g_you[index].top = g_you[index].bottom - originalHeight; // ũ�� ����
                    }
                    else {
                        g_you[index].top = obstacle.bottom;
                        g_you[index].bottom = g_you[index].top + originalHeight; // ũ�� ����
                    }
                    velocityY = -velocityY;  // ���� ����
                }
                break; // ù ��° �浹�� ó���ϰ� ����
            }
        }

        // ���� �ð����� ���� ����
        directionChangeCounter++;
        if (directionChangeCounter > 30 + (rand() % 20)) { // 30~50 ������ ����
            velocityX = (rand() % 7 + 4) * ((rand() % 2 == 0) ? 1 : -1);
            velocityY = (rand() % 7 + 4) * ((rand() % 2 == 0) ? 1 : -1);
            directionChangeCounter = 0;
        }

        // 5% Ȯ���� ���� ����
        if (rand() % 100 < 5) {
            velocityX *= 1.5f;
            velocityY *= 1.5f;
        }

        // �÷��̾�� �浹 �˻�
        if (CheckCollision(g_me, g_you[index])) {
            if (!g_hasPowerUp) {
                g_over = true;
                MessageBox(NULL, L"Game Over", L"���� ����", MB_OK);
                break;
            }
        }

        // �־��� �ð� ���� ���
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

#include "Gamec.h"
#include "GameObjects.h"
#include "Resource.h"
#include "Game.h"
#include <Windows.h>
#include <windowsx.h>
#include <time.h>
#include <vector>
#include <string>

HINSTANCE hInst;
LPCWSTR szTitle = L"My Game";
LPCWSTR szWindowClass = L"GameWindowClass";

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    MyRegisterClass(hInstance);

    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance;

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        srand(time(NULL));

        GetClientRect(hWnd, &g_box);

        // 내 객체 초기 정보
        g_me = { 50, 50, 110, 110 };

        // 상대 객체 초기 정보 (2개로 변경)
        RECT enemy1 = { 400, 400, 490, 490 };
        g_you.push_back(enemy1);

        InitializeItems();

        // 아이템 객체 초기 정보
        g_item.left = rand() % (g_box.right - 80);
        g_item.top = rand() % (g_box.bottom - 80);
        g_item.right = g_item.left + 50;
        g_item.bottom = g_item.top + 50;

        // 시간 바의 초기 정보
        g_bar = { g_box.right - 30, g_box.top + 10, g_box.right - 10, g_box.bottom - 10 };

        // 상대 객체 스레드 생성
        for (int i = 0; i < 1; ++i) {
            HANDLE hThread = CreateThread(NULL, 0, MoveOpponent, (LPVOID)(intptr_t)i, 0, NULL);
            if (hThread) {
                g_threads.push_back(hThread);
            }
        }

        // 타이머 설정
        SetTimer(hWnd, 3, 1000, NULL); // 게임 시간 제어
        SetTimer(hWnd, 4, 1000, NULL); // 게임 점수 증가
        SetTimer(hWnd, 5, 1000, NULL); // 속도 조정
    }
    break;

    case WM_KEYDOWN:
    {
        switch (wParam)
        {
        case VK_SPACE:  // 스페이스바로 일시정지/재개
            g_isPaused = !g_isPaused;
            if (g_isPaused) {
                MessageBox(hWnd, L"게임이 일시정지되었습니다. 계속하려면 스페이스바를 누르세요.", L"일시정지", MB_OK);
            }
            break;
        case 'R':  // R키로 게임 재시작
            RestartGame(hWnd);
            break;
        }

        if (!g_over && !g_isPaused)
        {
            int moveSpeed = g_hasPowerUp ? 17 : 10;  // 파워업 상태일 때 이동 속도 증가
            RECT newPos = g_me;  // 새로운 위치를 임시로 저장

            switch (wParam)
            {
            case VK_UP:
                newPos.top -= moveSpeed;
                newPos.bottom -= moveSpeed;
                break;
            case VK_DOWN:
                newPos.top += moveSpeed;
                newPos.bottom += moveSpeed;
                break;
            case VK_LEFT:
                newPos.left -= moveSpeed;
                newPos.right -= moveSpeed;
                break;
            case VK_RIGHT:
                newPos.left += moveSpeed;
                newPos.right += moveSpeed;
                break;
            }

            // 화면 경계 체크
            if (newPos.left < 0) newPos.left = 0;
            if (newPos.top < 0) newPos.top = 0;
            if (newPos.right > g_box.right) newPos.left = g_box.right - (newPos.right - newPos.left);
            if (newPos.bottom > g_box.bottom) newPos.top = g_box.bottom - (newPos.bottom - newPos.top);

            newPos.right = newPos.left + (g_me.right - g_me.left);
            newPos.bottom = newPos.top + (g_me.bottom - g_me.top);

            // 장애물과의 충돌 검사
            bool canMove = true;
            RECT intersection;
            for (const auto& obstacle : g_obstacles) {
                RECT tempRect = newPos;
                if (IntersectRect(&intersection, &tempRect, &obstacle)) {
                    // 충돌 방향에 따라 이동 제한
                    if (intersection.right - intersection.left < intersection.bottom - intersection.top) {
                        // 좌우 충돌
                        if (newPos.left < obstacle.left) {
                            newPos.right = obstacle.left;
                            newPos.left = newPos.right - (g_me.right - g_me.left);
                        }
                        else {
                            newPos.left = obstacle.right;
                            newPos.right = newPos.left + (g_me.right - g_me.left);
                        }
                    }
                    else {
                        // 상하 충돌
                        if (newPos.top < obstacle.top) {
                            newPos.bottom = obstacle.top;
                            newPos.top = newPos.bottom - (g_me.bottom - g_me.top);
                        }
                        else {
                            newPos.top = obstacle.bottom;
                            newPos.bottom = newPos.top + (g_me.bottom - g_me.top);
                        }
                    }
                    break;
                }
            }

            // 충돌 처리 후 최종 위치로 이동
            g_me = newPos;

            // 아이템 수집 검사
            RECT a;
            for (auto it = g_yellowItem.begin(); it != g_yellowItem.end(); ) {
                if (IntersectRect(&a, &g_me, &(*it))) {
                    g_yellowCollected++;
                    it = g_yellowItem.erase(it);
                    g_score += 200;  // 노란색 아이템 수집 시 점수 증가
                }
                else {
                    ++it;
                }
            }

            for (auto it = g_greenItem.begin(); it != g_greenItem.end(); ) {
                if (IntersectRect(&a, &g_me, &(*it))) {
                    g_greenCollected++;
                    it = g_greenItem.erase(it);
                    g_score += 200;  // 초록색 아이템 수집 시 점수 증가
                }
                else {
                    ++it;
                }
            }

            for (auto it = g_purpleItem.begin(); it != g_purpleItem.end(); ) {
                if (IntersectRect(&a, &g_me, &(*it))) {
                    g_purpleCollected++;
                    it = g_purpleItem.erase(it);
                    g_score += 300;  // 보라색 아이템 수집 시 점수 증가
                }
                else {
                    ++it;
                }
            }

            // 파워업 아이템 수집 검사
            if (IntersectRect(&a, &g_me, &g_powerUp)) {
                g_hasPowerUp = true;
                g_powerUpDuration = 7;  // 7초 동안 파워업 지속
                g_powerUp = { -100, -100, -100, -100 };  // 파워업 아이템 제거
            }

            // 라운드 완료 여부 체크
            if (g_yellowCollected >= g_yellowRequired &&
                g_greenCollected >= g_greenRequired &&
                g_purpleCollected >= g_purpleRequired) {
                g_isPaused = true;  // 게임 일시 정지
                // 현재 라운드 완료 메시지
                TCHAR szMessage[100];
                wsprintf(szMessage, L"라운드 %d 완료! 다음 라운드로 진행하시겠습니까?", g_round);
                if (MessageBox(hWnd, szMessage, L"라운드 완료", MB_YESNO | MB_ICONQUESTION) == IDYES) {
                    StartNewRound(hWnd);
                }
                else {
                    g_over = true;  // 게임 종료
                    MessageBox(hWnd, L"게임을 종료합니다.", L"게임 종료", MB_OK);
                }
            }

            InvalidateRect(hWnd, NULL, TRUE);
        }
    }
    break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        // 배경 그리기
        HBRUSH hBrushBg = CreateSolidBrush(RGB(245, 245, 245));  // 회색!
        FillRect(hdc, &g_box, hBrushBg);
        DeleteObject(hBrushBg);

        // 플레이어 그리기
        HBRUSH hBrushMe;
        if (g_hasPowerUp) {
            hBrushMe = CreateSolidBrush(RGB(255, 200, 0));  // 주황색 (파워업 상태)
        }
        else {
            hBrushMe = CreateSolidBrush(RGB(0, 0, 255));    // 파란색 (일반 상태)
        }
        FillRect(hdc, &g_me, hBrushMe);
        DeleteObject(hBrushMe);

        // 적 그리기
        HBRUSH hBrushEnemy = CreateSolidBrush(RGB(255, 0, 0));
        for (const auto& enemy : g_you) {
            FillRect(hdc, &enemy, hBrushEnemy);
        }
        DeleteObject(hBrushEnemy);

        // 아이템 그리기
        HBRUSH hBrushYellow = CreateSolidBrush(RGB(255, 255, 0));
        HBRUSH hBrushGreen = CreateSolidBrush(RGB(0, 255, 0));
        HBRUSH hBrushPurple = CreateSolidBrush(RGB(128, 0, 128));

        for (const auto& item : g_yellowItem) {
            FillRect(hdc, &item, hBrushYellow);
        }
        for (const auto& item : g_greenItem) {
            FillRect(hdc, &item, hBrushGreen);
        }
        for (const auto& item : g_purpleItem) {
            FillRect(hdc, &item, hBrushPurple);
        }

        DeleteObject(hBrushYellow);
        DeleteObject(hBrushGreen);
        DeleteObject(hBrushPurple);

        // 파워업 아이템 그리기
        if (g_powerUp.left >= 0) {
            HBRUSH hBrushPowerUp = CreateSolidBrush(RGB(255, 165, 0));
            FillRect(hdc, &g_powerUp, hBrushPowerUp);
            DeleteObject(hBrushPowerUp);
        }

        // 장애물 그리기 (3라운드부터)
        if (g_round >= 3) {
            HBRUSH hBrushObstacle = CreateSolidBrush(RGB(128, 128, 128));
            for (const auto& obstacle : g_obstacles) {
                FillRect(hdc, &obstacle, hBrushObstacle);
            }
            DeleteObject(hBrushObstacle);
        }

        // 게임 정보 표시
        TCHAR szText[100];
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(0, 0, 0));

        wsprintf(szText, L"Score: %d", g_score);
        TextOut(hdc, 10, 10, szText, lstrlen(szText));

        wsprintf(szText, L"Time: %d", g_gametime);
        TextOut(hdc, 10, 30, szText, lstrlen(szText));

        wsprintf(szText, L"Round: %d", g_round);
        TextOut(hdc, 10, 50, szText, lstrlen(szText));

        wsprintf(szText, L"Yellow: %d/%d", g_yellowCollected, g_yellowRequired);
        TextOut(hdc, 10, 70, szText, lstrlen(szText));

        wsprintf(szText, L"Green: %d/%d", g_greenCollected, g_greenRequired);
        TextOut(hdc, 10, 90, szText, lstrlen(szText));

        wsprintf(szText, L"Purple: %d/%d", g_purpleCollected, g_purpleRequired);
        TextOut(hdc, 10, 110, szText, lstrlen(szText));

        if (g_hasPowerUp) {
            wsprintf(szText, L"Power Up: %d", g_powerUpDuration);
            TextOut(hdc, 10, 130, szText, lstrlen(szText));
        }

        // 오른쪽에 게임 설명 추가
        int startY = 10;
        for (const auto& instruction : g_keyInstructions) {
            TextOut(hdc, g_box.right - 200, startY, instruction.c_str(), instruction.length());
            startY += 20;
        }

        EndPaint(hWnd, &ps);
    }
    break;

    case WM_TIMER:
        if (!g_isPaused) {
            switch (wParam)
            {
            case 3:
                if (!g_over)
                {
                    g_gametime--;
                    if (g_gametime <= 0)
                    {
                        g_over = true;
                        KillTimer(hWnd, 3);
                        MessageBox(hWnd, L"Time's up!", L"Game Over", MB_OK);
                    }
                    InvalidateRect(hWnd, NULL, TRUE);
                }
                break;
            case 4:
                if (!g_over)
                {
                    g_score += 10;
                    InvalidateRect(hWnd, NULL, TRUE);
                }
                break;
            case 5:
                if (!g_over)
                {
                    if (g_hasPowerUp)
                    {
                        g_powerUpDuration--;
                        if (g_powerUpDuration <= 0)
                        {
                            g_hasPowerUp = false;
                        }
                    }
                    InvalidateRect(hWnd, NULL, TRUE);
                }
                break;
            }
        }
        break;

    case WM_DESTROY:
        for (HANDLE hThread : g_threads) {
            TerminateThread(hThread, 0);
            CloseHandle(hThread);
        }
        g_threads.clear();
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


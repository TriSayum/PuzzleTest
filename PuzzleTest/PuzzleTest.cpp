#include <windows.h>
#include <cstdlib>
#include <ctime>
#include <string.h>
#include <string>

#define pi 3.1415926

#define ROW 8
#define COL 8
#define MAX_NUM 13
#define NUM_COUNT 4
#define STRANGE_COUNT 10
#define PRIZE 1

using namespace std;

// 全局变量，用于存储表格数据
int table[ROW][COL];
int colort[ROW][COL];
int numcolor[MAX_NUM + 1] = { 0 };

int changeloop;

// 存储绿色小圆点的坐标（对应左上角为0的格子中心附近）
int greenDotRow = 0;
int greenDotCol = 0;

// 存储蓝色小圆点的坐标（对应右下角为0的格子中心附近）
int blueDotRow = ROW - 1;
int blueDotCol = COL - 1;

COLORREF color = RGB(0, 0, 0);

// 用于表格更改功能的变量
bool isChanging = false;
int changeRow = -1;
int changeCol = -1;
string changeValue = "";

// 新增：存储绿色和蓝色下方数字的变量，初始值设为10
int greenNumber = 10;
int blueNumber = 10;

// 窗口过程函数
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// 初始化表格数据
void initTable() {
    // 先将左上角和右下角设置为0
    table[0][0] = 0;
    table[ROW - 1][COL - 1] = 0;

    // 用于记录数字1-13出现的次数，初始化为0
    int numCount[MAX_NUM + 1] = { 0 };
    srand(static_cast<unsigned int>(time(nullptr)));

    // 填充剩下的52个格子
    int count = 0;
    while (count < ROW * COL - 2) {
        int row = rand() % ROW;
        int col = rand() % COL;
        if (table[row][col] == 0 && (row != 0 || col != 0) && (row != ROW - 1 || col != COL - 1)) {
            int num = rand() % (MAX_NUM + 1) + 1;
            if (numCount[num - 1] < NUM_COUNT && num != MAX_NUM + 1) {
                table[row][col] = num;
                colort[row][col] = 1;
                numcolor[num - 1]++;
                if (numcolor[num - 1] > PRIZE)
                    colort[row][col] = 0;
                numCount[num - 1]++;
                count++;
            }
            else if (numCount[num - 1] < STRANGE_COUNT && num == MAX_NUM + 1) {
                table[row][col] = num;
                colort[row][col] = 3;
                numCount[num - 1]++;
                count++;
            }
        }
    }
}

// 绘制表格及内容的函数，修改此处以添加下方数字的绘制
void drawTable(HDC hdc) {
    // 增大每个格子的宽度和高度，这里设置为80（可根据需要再调整）
    int cellWidth = 80;
    int cellHeight = 80;

    // 增大数字字体大小，创建逻辑字体并选入设备上下文
    HFONT hFont = CreateFont(40, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET,
        OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY,
        DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
    SelectObject(hdc, hFont);

    // 绘制表格线
    for (int i = 0; i <= ROW; i++) {
        MoveToEx(hdc, 0, i * cellHeight, nullptr);
        LineTo(hdc, COL * cellWidth, i * cellHeight);
    }
    for (int j = 0; j <= COL; j++) {
        MoveToEx(hdc, j * cellWidth, 0, nullptr);
        LineTo(hdc, j * cellWidth, ROW * cellHeight);
    }

    // 填充表格数字内容
    for (int row = 0; row < ROW; row++) {
        for (int col = 0; col < COL; col++) {
            if (table[row][col] != 0) {
                string numStr = to_string(table[row][col]);
                if (!colort[row][col])
                    color = RGB(0, 0, 0);
                else if (colort[row][col] == 1)
                    color = RGB(255, 0, 0);
                else if (colort[row][col] == 2)
                    color = RGB(138, 43, 226);
                else if (colort[row][col] == 3)
                    color = RGB(113, 199, 213);
                SetTextColor(hdc, color);
                // 调整数字的绘制位置
                if (table[row][col] != MAX_NUM + 1)
                    TextOutA(hdc, col * cellWidth + 40 - 10 * numStr.length(),
                        row * cellHeight + 20, numStr.c_str(), numStr.length());
                else
                    TextOut(hdc, col * cellWidth + 28, row * cellHeight + 20, L"R", 1);
            }
        }
    }

    // 新增：判断绿色和蓝色圆环是否在同一位置，如果是则绘制特殊圆环，否则按原逻辑绘制
    if (greenDotRow == blueDotRow && greenDotCol == blueDotCol) {
        // 设置透明画刷
        HBRUSH transparentBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
        SelectObject(hdc, transparentBrush);
        // 计算圆环中心位置
        int cellWidth = 80;
        int cellHeight = 80;
        int centerX = greenDotCol * cellWidth + cellWidth / 2;
        int centerY = greenDotRow * cellHeight + cellHeight / 2;
        // 绘制特殊圆环（内径20，外径25）
        Ellipse(hdc, centerX - 25, centerY - 25, centerX + 25, centerY + 25);
        Ellipse(hdc, centerX - 20, centerY - 20, centerX + 20, centerY + 20);
        // 实现渐变效果，这里简单地划分角度区间来改变颜色
        const int numSegments = 16;
        for (int i = 0; i < numSegments; ++i) {
            // 计算每个小部分的起始角度和结束角度
            int startAngle = i * (360 / numSegments);
            int endAngle = (i + 1) * (360 / numSegments);
            // 根据角度区间设置颜色，从绿色渐变到蓝色
            COLORREF color;
            if (i < numSegments / 2) {
                // 绿色成分逐渐减少
                int greenValue = 255 - (i * (255 / (numSegments / 2)));
                color = RGB(0, greenValue, 255 - greenValue);
            }
            else {
                // 蓝色成分逐渐减少
                int blueValue = 255 - ((i - numSegments / 2) * (255 / (numSegments / 2)));
                color = RGB(0, 255 - blueValue, blueValue);
            }
            HPEN gradientPen = CreatePen(PS_SOLID, 3, color);
            SelectObject(hdc, gradientPen);
            // 绘制弧线来模拟渐变效果
            Arc(hdc, centerX - 25, centerY - 25, centerX + 25, centerY + 25,
                (int)(centerX + 25 * cos(startAngle * pi / 180)),
                (int)(centerY - 25 * sin(startAngle * pi / 180)),
                (int)(centerX + 25 * cos(endAngle * pi / 180)),
                (int)(centerY - 25 * sin(endAngle * pi / 180)));
            Arc(hdc, centerX - 20, centerY - 20, centerX + 20, centerY + 20,
                (int)(centerX + 20 * cos(startAngle * pi / 180)),
                (int)(centerY - 20 * sin(startAngle * pi / 180)),
                (int)(centerX + 20 * cos(endAngle * pi / 180)),
                (int)(centerY - 20 * sin(endAngle * pi / 180)));
            DeleteObject(gradientPen);
        }
        DeleteObject(transparentBrush);
    }
    else {
        // 绘制绿色空心圆环
        {
            // 设置绿色画笔颜色
            HPEN greenPen = CreatePen(PS_SOLID, 3, RGB(0, 255, 0));
            SelectObject(hdc, greenPen);
            // 设置透明画刷
            HBRUSH transparentBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
            SelectObject(hdc, transparentBrush);
            // 计算圆环中心位置
            int centerX = greenDotCol * cellWidth + cellWidth / 2;
            int centerY = greenDotRow * cellHeight + cellHeight / 2;
            // 绘制圆环（内径20，外径25）
            Ellipse(hdc, centerX - 25, centerY - 25, centerX + 25, centerY + 25);
            Ellipse(hdc, centerX - 20, centerY - 20, centerX + 20, centerY + 20);
            DeleteObject(greenPen);
            DeleteObject(transparentBrush);
        }

        // 绘制蓝色空心圆环
        {
            // 设置蓝色画笔颜色
            HPEN bluePen = CreatePen(PS_SOLID, 3, RGB(0, 0, 255));
            SelectObject(hdc, bluePen);
            // 设置透明画刷
            HBRUSH transparentBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
            SelectObject(hdc, transparentBrush);
            // 计算圆环中心位置
            int centerX = blueDotCol * cellWidth + cellWidth / 2;
            int centerY = blueDotRow * cellHeight + cellHeight / 2;
            // 绘制圆环（内径20，外径25）
            Ellipse(hdc, centerX - 25, centerY - 25, centerX + 25, centerY + 25);
            Ellipse(hdc, centerX - 20, centerY - 20, centerX + 20, centerY + 20);
            DeleteObject(bluePen);
            DeleteObject(transparentBrush);
        }
    }

    hFont = CreateFont(60, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET,
        OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY,
        DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
    SelectObject(hdc, hFont);
    // 新增：绘制绿色数字，位置在表格下方靠左边一点
    string greenNumStr = to_string(greenNumber);
    SetTextColor(hdc, RGB(0, 255, 0));
    TextOutA(hdc, 160 - 15 * greenNumStr.length(), ROW * cellHeight + 10, greenNumStr.c_str(), greenNumStr.length());
    
    // 新增：绘制蓝色数字，位置在表格下方靠右边一点
    string blueNumStr = to_string(blueNumber);
    SetTextColor(hdc, RGB(0, 0, 255));
    TextOutA(hdc, COL * cellWidth - 160 - 15 * blueNumStr.length(), ROW * cellHeight + 10, blueNumStr.c_str(), blueNumStr.length());
}

int LocationSelection(int key,char wParam) {
    if (key <= ROW && isChanging && changeRow == -1) {
        changeRow = wParam - '1';
        OutputDebugString(TEXT("Entered Row\n"));
        return 1;
    }
    if (key <= COL && isChanging && changeRow != -1 && changeCol == -1) {
        changeCol = wParam - '1';
        OutputDebugString(TEXT("Entered Col\n"));
        return 1;
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    PSTR szCmdLine, int iCmdShow)
{
    static TCHAR szAppName[] = TEXT("MyTableApp");
    HWND hwnd;
    MSG msg;
    WNDCLASS wndclass;

    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = WndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = hInstance;
    wndclass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wndclass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndclass.lpszMenuName = nullptr;
    wndclass.lpszClassName = szAppName;

    if (!RegisterClass(&wndclass))
    {
        MessageBox(nullptr, TEXT("This program requires Windows NT!"),
            szAppName, MB_ICONERROR);
        return 0;
    }

    // 增大窗口的初始大小，宽度和高度根据新的格子大小和表格行列数计算并适当增大一些边距
    hwnd = CreateWindow(szAppName, TEXT("Table Display"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        COL * 80 + 20, ROW * 80 + 120,  // 适当增大窗口高度，给下方数字留出空间
        nullptr, nullptr, hInstance, nullptr);

    ShowWindow(hwnd, iCmdShow);
    UpdateWindow(hwnd);

    initTable();

    InvalidateRect(hwnd, NULL, TRUE);

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;
    switch (message)
    {
    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);
        drawTable(hdc);
        EndPaint(hwnd, &ps);
        return 0;
    case WM_KEYDOWN:
        switch (wParam)
        {
        case 'X':
        case 'x':
            isChanging = true;
            changeRow = -1;
            changeCol = -1;
            changeValue = "";
            OutputDebugString(TEXT("Entered changing mode\n"));
            break;
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            changeloop = '9' - wParam + 1;
            if (isChanging && changeRow != -1 && changeCol != -1) {
                changeValue += (char)wParam;
            }
            while (changeloop) {
                if (LocationSelection(10 - changeloop, wParam)) break;
                changeloop--;
            }
            break;
        case VK_RETURN:
            if (isChanging && changeRow != -1 && changeCol != -1 && changeValue != "") {
                table[changeRow][changeCol] = stoi(changeValue);
                colort[changeRow][changeCol] = 2;
                isChanging = false;
                InvalidateRect(hwnd, NULL, TRUE);
                OutputDebugString(TEXT("Changed\n"));
            }
            break;
        case VK_BACK:
            if (isChanging && changeValue.length() > 0) {
                changeValue.pop_back();
            }
            break;
            // 新增：处理绿色数字增加和减少的按键逻辑
        case 'Q':
        case 'q':
            greenNumber--;
            InvalidateRect(hwnd, nullptr, TRUE);
            break;
        case 'E':
        case 'e':
            greenNumber++;
            InvalidateRect(hwnd, nullptr, TRUE);
            break;
            // 新增：处理蓝色数字增加和减少的按键逻辑
        case 'N':
        case 'n':
            blueNumber--;
            InvalidateRect(hwnd, nullptr, TRUE);
            break;
        case 'M':
        case 'm':
            blueNumber++;
            InvalidateRect(hwnd, nullptr, TRUE);
            break;
        default:
            if (isChanging && changeRow != -1 && changeCol != -1) {
                changeValue += (char)wParam;
            }
            break;
        }
        // 移动绿色小圆点的逻辑
        switch (wParam)
        {
        case 'W':
        case 'w':
            if (greenDotRow > 0) {
                greenDotRow--;
                InvalidateRect(hwnd, nullptr, TRUE);
            }
            break;
        case 'A':
        case 'a':
            if (greenDotCol > 0) {
                greenDotCol--;
                InvalidateRect(hwnd, nullptr, TRUE);
            }
            break;
        case 'S':
        case 's':
            if (greenDotRow < ROW - 1) {
                greenDotRow++;
                InvalidateRect(hwnd, nullptr, TRUE);
            }
            break;
        case 'D':
        case 'd':
            if (greenDotCol < COL - 1) {
                greenDotCol++;
                InvalidateRect(hwnd, nullptr, TRUE);
            }
            break;
        case VK_UP:
            if (blueDotRow > 0) {
                blueDotRow--;
                InvalidateRect(hwnd, nullptr, TRUE);
            }
            break;
        case VK_LEFT:
            if (blueDotCol > 0) {
                blueDotCol--;
                InvalidateRect(hwnd, nullptr, TRUE);
            }
            break;
        case VK_DOWN:
            if (blueDotRow < ROW - 1) {
                blueDotRow++;
                InvalidateRect(hwnd, nullptr, TRUE);
            }
            break;
        case VK_RIGHT:
            if (blueDotCol < COL - 1) {
                blueDotCol++;
                InvalidateRect(hwnd, nullptr, TRUE);
            }
            break;
        }
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
}
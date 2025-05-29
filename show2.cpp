#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <graphics.h>
#include <conio.h>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <algorithm>
#include <string>
using namespace std;

// ������������
#define BLOCK_SIZE 25         // �����С
#define GRID_WIDTH 12         // ��Ϸ�����
#define GRID_HEIGHT 20        // ��Ϸ���߶�
#define WINDOW_WIDTH 600      // ���ڿ��
#define WINDOW_HEIGHT 600     // ���ڸ߶�

// ��Ϸ״̬
enum GameState {
    MENU,
    PLAYING,
    PAUSED,
    GAMEOVER
};

// ������ɫ - ʹ�ý���ɫ��
struct BlockColor {
    int primary;
    int secondary;
    int border;
};

BlockColor blockColors[7] = {
    {RGB(164,203,236), RGB(126,182,228), RGB(80,140,200)},   // ��ɫϵ
    {RGB(214,229,138), RGB(190,212,100), RGB(150,180,80)},   // ��ɫϵ
    {RGB(240,244,168), RGB(230,235,130), RGB(200,205,100)},  // ��ɫϵ
    {RGB(227,115,103), RGB(210,80,70),   RGB(180,50,40)},    // ��ɫϵ
    {RGB(181,230,211), RGB(150,210,190), RGB(110,180,160)},  // ��ɫϵ
    {RGB(99,111,153),  RGB(80,90,140),   RGB(60,70,120)},    // ��ɫϵ
    {RGB(177,195,121), RGB(160,180,100), RGB(140,160,80)}    // �����ϵ
};

// ������������
int Block[7][4][4][2] = {
    // ���ֻ�����״��ÿ���ĸ���ת����
    {/*Type 0 - T��*/
        {{0, 0}, {-1, 0}, {1, 0}, {0, -1}},
        {{0, 0}, {1, 0}, {0, 1}, {0, -1}},
        {{0, 0}, {-1, 0}, {1, 0}, {0, 1}},
        {{0, 0}, {-1, 0}, {0, 1}, {0, -1}}
    },
    {/*Type 1 - Z��*/
        {{0, 0},{0,-1},{1,-1},{-1,0}},
        {{0, 0},{0,-1},{1,0},{1,1}},
        {{0, 0},{-1,0},{0,-1},{1,-1}},
        {{0, 0},{0,-1},{1,0},{1,1}}
    },
    {/*Type 2 - S��*/
        {{0, 0},{-1,-1},{0,-1},{1,0}},
        {{0, 0},{1,0},{0,1},{1,-1}},
        {{0, 0},{-1,-1},{0,-1},{1,0}},
        {{0, 0},{1,-1},{1,0},{0,1}}
    },
    {/*Type 3 - I��*/
        {{0, 0},{-2,0},{-1,0},{1,0}},
        {{0, 0},{0,-2},{0,-1},{0,1}},
        {{0, 0},{-2,0},{-1,0},{1,0}},
        {{0, 0},{0,-2},{0,-1},{0,1}}
    },
    {/*Type 4 - O��*/
        {{0,0},{-1,-1},{0,-1},{-1,0}},
        {{0,0},{-1,-1},{0,-1},{-1,0}},
        {{0,0},{-1,-1},{0,-1},{-1,0}},
        {{0,0},{-1,-1},{0,-1},{-1,0}}
    },
    {/*Type 5 - J��*/
        {{0,0},{1,-1},{-1,0},{1,0}},
        {{1,1},{0,0},{0,-1},{0,1}},
        {{-1,1},{0,0},{-1,0},{1,0}},
        {{-1,-1},{0,0},{0,1},{0,-1}}
    },
    {/*Type 6 - L��*/
        {{0,0},{-1,0},{1,0},{1,1}},
        {{0,0},{0,1},{0,-1},{-1,1}},
        {{0,0},{-1,0},{1,0},{-1,-1}},
        {{0,0},{0,1},{0,-1},{1,-1}}
    }
};

// ���ⷽ������ (����Ԫ��)
enum SpecialBlockType {
    NONE,
    LINE_CLEAR,     // ���һ����
    BOMB,           // ��ը�����Χ����
    WEIGHT,         // �������䲢�÷ּӱ�
    GHOST           // ���Դ������з���
};

// ����Ч��
struct Particle {
    float x, y;
    float vx, vy;
    int color;
    int life;
    float size;
};

// ��Ϸȫ�ֱ���
GameState gameState = MENU;
SpecialBlockType currentSpecial = NONE;
vector<Particle> particles;

int landedBlock[GRID_WIDTH][GRID_HEIGHT] = { 0 };
int specialEffects[GRID_WIDTH][GRID_HEIGHT] = { 0 }; // ����Ч�����
int type, nextType, direction;
int x, y;
int fallSpeed = 20;
int baseSpeed = 20;
int score = 0;
int level = 1;
int linesCleared = 0;
int combo = 0;
int gameTime = 0; // ��Ϸ��ʱ
int nextBlockColor;
bool gameover = false;

// ����ǰ������
bool canFall(int x, int y, int type, int direction);
void drawBackground();

// ����Ч��ϵͳ
void addParticles(int centerX, int centerY, int count, int color) {
    for (int i = 0; i < count; i++) {
        Particle p;
        p.x = centerX * BLOCK_SIZE + BLOCK_SIZE / 2.0f;
        p.y = centerY * BLOCK_SIZE + BLOCK_SIZE / 2.0f;

        // ����ٶȷ���
        float angle = (rand() % 360) * 3.14159f / 180.0f;
        float speed = 0.5f + (rand() % 20) / 10.0f;

        p.vx = cos(angle) * speed;
        p.vy = sin(angle) * speed;
        p.color = color;
        p.life = 30 + rand() % 60; // ��������
        p.size = 1.0f + (rand() % 10) / 10.0f;

        particles.push_back(p);
    }
}

void updateParticles() {
    for (int i = 0; i < particles.size(); i++) {
        particles[i].x += particles[i].vx;
        particles[i].y += particles[i].vy;
        particles[i].vy += 0.05f; // ����Ч��
        particles[i].life--;

        if (particles[i].life <= 0) {
            particles.erase(particles.begin() + i);
            i--;
        }
    }
}

void drawParticles() {
    for (auto& p : particles) {
        setfillcolor(p.color);
        solidcircle(p.x, p.y, p.size * 2);
    }
}

// ���Ƶ������鵥Ԫ��ʹ�ý���Ч��
void drawUnitBlock(int x, int y, int colorIndex) {
    int left = x * BLOCK_SIZE + (WINDOW_WIDTH - GRID_WIDTH * BLOCK_SIZE) / 2;
    int right = left + BLOCK_SIZE;
    int top = y * BLOCK_SIZE + 40; // ���������ռ���ʾ����
    int bottom = top + BLOCK_SIZE;

    if (colorIndex == 0) { // �հ׷���
        setlinecolor(RGB(180, 180, 180));
        rectangle(left, top, right, bottom);

        // ��ɫ����Ч��
        setfillcolor(RGB(240, 240, 240));
        solidrectangle(left + 1, top + 1, right - 1, bottom - 1);
    }
    else {
        // ��ȡ������ɫ
        int primaryColor = blockColors[colorIndex - 1].primary;
        int secondaryColor = blockColors[colorIndex - 1].secondary;
        int borderColor = blockColors[colorIndex - 1].border;

        // ���Ʊ߿�
        setlinecolor(borderColor);
        rectangle(left, top, right, bottom);

        // �������
        for (int i = 0; i < BLOCK_SIZE - 2; i++) {
            // ���㽥��ɫ
            int r1 = GetRValue(primaryColor);
            int g1 = GetGValue(primaryColor);
            int b1 = GetBValue(primaryColor);

            int r2 = GetRValue(secondaryColor);
            int g2 = GetGValue(secondaryColor);
            int b2 = GetBValue(secondaryColor);

            float ratio = (float)i / (BLOCK_SIZE - 2);

            int r = r1 + (r2 - r1) * ratio;
            int g = g1 + (g2 - g1) * ratio;
            int b = b1 + (b2 - b1) * ratio;

            setlinecolor(RGB(r, g, b));
            line(left + 1 + i, top + 1, left + 1 + i, bottom - 1);
        }

        // ��Ч���
        if (specialEffects[x][y] > 0) {
            // ���ⷽ��Ч��
            switch (specialEffects[x][y]) {
            case LINE_CLEAR: // ֱ��Ч��
                setlinecolor(RGB(255, 255, 0));
                line(left + 2, top + BLOCK_SIZE / 2, right - 2, top + BLOCK_SIZE / 2);
                break;
            case BOMB: // ��ըЧ��
                setlinecolor(RGB(255, 0, 0));
                circle(left + BLOCK_SIZE / 2, top + BLOCK_SIZE / 2, BLOCK_SIZE / 4);
                break;
            case WEIGHT: // ����Ч��
                setlinecolor(RGB(100, 100, 100));
                rectangle(left + BLOCK_SIZE / 4, top + BLOCK_SIZE / 4,
                    right - BLOCK_SIZE / 4, bottom - BLOCK_SIZE / 4);
                break;
            case GHOST: // ����Ч��
                setlinecolor(RGB(200, 200, 255));
                for (int i = 0; i < 4; i++) {
                    line(left + i * 5, top + 2, left + i * 5, bottom - 2);
                }
                break;
            }
        }

        // ����Ч��
        setlinecolor(RGB(255, 255, 255));
        setlinestyle(PS_SOLID, 1);
        line(left + 2, top + 2, right - 2, top + 2);
        line(left + 2, top + 2, left + 2, bottom - 2);
    }
}

// ���Ʒ���
void drawBlock(int x, int y, int type, int direction, int colorIndex) {
    for (int i = 0; i < 4; i++) {
        int dx = Block[type][direction][i][0];
        int dy = Block[type][direction][i][1];
        int screenX = x + dx;
        int screenY = y + dy;

        drawUnitBlock(screenX, screenY, colorIndex + 1);
    }
}

// ���Ʒ�����Ӱ����ʾ��㣩
void drawBlockShadow(int x, int y, int type, int direction) {
    int shadowY = y;

    // �ҵ�������������
    while (canFall(x, shadowY, type, direction)) {
        shadowY++;
    }

    if (shadowY != y) {
        // �ð�͸��Ч��������Ӱ
        setfillstyle(BS_SOLID);
        setlinestyle(PS_SOLID, 1);

        for (int i = 0; i < 4; i++) {
            int dx = Block[type][direction][i][0];
            int dy = Block[type][direction][i][1];
            int screenX = x + dx;
            int screenY = shadowY + dy;

            int left = screenX * BLOCK_SIZE + (WINDOW_WIDTH - GRID_WIDTH * BLOCK_SIZE) / 2;
            int right = left + BLOCK_SIZE;
            int top = screenY * BLOCK_SIZE + 40;
            int bottom = top + BLOCK_SIZE;

            setlinecolor(RGB(100, 100, 100));
            rectangle(left, top, right, bottom);
        }
    }
}

// ��Ϸ�߽��麯��
bool canGoLeft(int x, int y, int type, int direction) {
    for (int i = 0; i < 4; i++) {
        int dx = Block[type][direction][i][0];
        int dy = Block[type][direction][i][1];
        int newX = x + dx - 1;
        int newY = y + dy;

        // ���߽����ײ
        if (newX < 0 || newX >= GRID_WIDTH || newY >= GRID_HEIGHT ||
            (newY >= 0 && landedBlock[newX][newY] != 0)) {

            // ����ģʽ�����鷽����Դ���
            if (currentSpecial == GHOST && newY >= 0 && landedBlock[newX][newY] != 0) {
                continue;
            }
            return false;
        }
    }
    return true;
}

bool canGoRight(int x, int y, int type, int direction) {
    for (int i = 0; i < 4; i++) {
        int dx = Block[type][direction][i][0];
        int dy = Block[type][direction][i][1];
        int newX = x + dx + 1;
        int newY = y + dy;

        if (newX < 0 || newX >= GRID_WIDTH || newY >= GRID_HEIGHT ||
            (newY >= 0 && landedBlock[newX][newY] != 0)) {

            if (currentSpecial == GHOST && newY >= 0 && landedBlock[newX][newY] != 0) {
                continue;
            }
            return false;
        }
    }
    return true;
}

bool canRotation(int x, int y, int type, int direction) {
    int newDirection = (direction + 1) % 4;

    for (int i = 0; i < 4; i++) {
        int dx = Block[type][newDirection][i][0];
        int dy = Block[type][newDirection][i][1];
        int newX = x + dx;
        int newY = y + dy;

        if (newX < 0 || newX >= GRID_WIDTH || newY >= GRID_HEIGHT ||
            (newY >= 0 && landedBlock[newX][newY] != 0)) {

            if (currentSpecial == GHOST && newY >= 0 && landedBlock[newX][newY] != 0) {
                continue;
            }
            return false;
        }
    }
    return true;
}

bool canFall(int x, int y, int type, int direction) {
    for (int i = 0; i < 4; i++) {
        int dx = Block[type][direction][i][0];
        int dy = Block[type][direction][i][1];
        int newX = x + dx;
        int newY = y + dy + 1;

        if (newX < 0 || newX >= GRID_WIDTH || newY >= GRID_HEIGHT ||
            (newY >= 0 && landedBlock[newX][newY] != 0)) {

            if (currentSpecial == GHOST && newY >= 0 && landedBlock[newX][newY] != 0) {
                continue;
            }
            return false;
        }
    }
    return true;
}

// ������������Ч��
void animateLineClearing(int lineY) {
    // ��˸Ч��
    for (int flash = 0; flash < 3; flash++) {
        // ��Ҫ�������б�ɰ�ɫ
        for (int i = 0; i < GRID_WIDTH; i++) {
            int left = i * BLOCK_SIZE + (WINDOW_WIDTH - GRID_WIDTH * BLOCK_SIZE) / 2;
            int top = lineY * BLOCK_SIZE + 40;

            setfillcolor(RGB(255, 255, 255));
            solidrectangle(left, top, left + BLOCK_SIZE, top + BLOCK_SIZE);
        }
        FlushBatchDraw();
        Sleep(80);

        // ��Ҫ�������б��ԭ������ɫ
        for (int i = 0; i < GRID_WIDTH; i++) {
            drawUnitBlock(i, lineY, landedBlock[i][lineY]);
        }
        FlushBatchDraw();
        Sleep(80);
    }

    // �����е�����Ч��
    for (int i = 0; i < GRID_WIDTH; i++) {
        addParticles(i, lineY, 8, RGB(255, 220, 150));
    }
}

// ը����Ч
void bombEffect(int centerX, int centerY) {
    // ��ӱ�ը����Ч��
    addParticles(centerX, centerY, 40, RGB(255, 100, 0));

    // �����Χ�ķ���
    for (int dx = -2; dx <= 2; dx++) {
        for (int dy = -2; dy <= 2; dy++) {
            int newX = centerX + dx;
            int newY = centerY + dy;

            if (newX >= 0 && newX < GRID_WIDTH && newY >= 0 && newY < GRID_HEIGHT) {
                // ���������ĵľ���
                float distance = sqrt(dx * dx + dy * dy);

                if (distance <= 2.5f) { // ��ը�뾶
                    landedBlock[newX][newY] = 0;
                    specialEffects[newX][newY] = 0;

                    // ��ը����
                    int left = newX * BLOCK_SIZE + (WINDOW_WIDTH - GRID_WIDTH * BLOCK_SIZE) / 2;
                    int top = newY * BLOCK_SIZE + 40;

                    setfillcolor(RGB(255, 200, 0));
                    solidrectangle(left, top, left + BLOCK_SIZE, top + BLOCK_SIZE);
                }
            }
        }
    }

    FlushBatchDraw();
    Sleep(100);

    score += 30; // ը���ӷ�
}

// ���������
void LandedBlock(int x, int y, int type, int direction, int colorIndex) {
    // ����Ч������
    bool hasBomb = false;
    int bombX = 0, bombY = 0;

    // ��¼����λ��
    for (int i = 0; i < 4; i++) {
        int dx = Block[type][direction][i][0];
        int dy = Block[type][direction][i][1];
        int newX = x + dx;
        int newY = y + dy;

        if (newY >= 0) {
            landedBlock[newX][newY] = colorIndex + 1;

            // ��¼����Ч��
            if (currentSpecial != NONE) {
                specialEffects[newX][newY] = currentSpecial;

                // ��¼ը��λ��
                if (currentSpecial == BOMB) {
                    hasBomb = true;
                    bombX = newX;
                    bombY = newY;
                }

                // ֱ�����
                if (currentSpecial == LINE_CLEAR) {
                    for (int j = 0; j < GRID_WIDTH; j++) {
                        landedBlock[j][newY] = 0;
                        specialEffects[j][newY] = 0;
                    }

                    animateLineClearing(newY);
                    score += 15;
                }
            }
        }
    }

    // Ӧ��ը��Ч��
    if (hasBomb) {
        bombEffect(bombX, bombY);
    }

    // Ȩ�ط������������
    if (currentSpecial == WEIGHT) {
        score += 15;
    }

    // ��鲢������������
    int linesCount = 0;
    for (int j = 0; j < GRID_HEIGHT; j++) {
        bool isFull = true;
        for (int i = 0; i < GRID_WIDTH; i++) {
            if (landedBlock[i][j] == 0) {
                isFull = false;
                break;
            }
        }

        if (isFull) {
            linesCount++;

            // �����������
            animateLineClearing(j);

            // ԭ�е��������߼�
            for (int k = j; k > 0; k--) {
                for (int i = 0; i < GRID_WIDTH; i++) {
                    landedBlock[i][k] = landedBlock[i][k - 1];
                    specialEffects[i][k] = specialEffects[i][k - 1];
                }
            }

            // �������
            for (int i = 0; i < GRID_WIDTH; i++) {
                landedBlock[i][0] = 0;
                specialEffects[i][0] = 0;
            }
        }
    }

    // ���·���������
    if (linesCount > 0) {
        combo++;
        int basePoints = 0;

        // ��������
        switch (linesCount) {
        case 1: basePoints = 40; break;
        case 2: basePoints = 100; break;
        case 3: basePoints = 300; break;
        case 4: basePoints = 1200; break;
        }

        // ��������
        int comboBonus = min(combo * 50, 500);

        // �����ܷ�
        int totalPoints = basePoints * level + comboBonus;
        score += totalPoints;

        // ��ʾ������Ϣ
        if (combo > 1) {
            char comboText[50];
            sprintf(comboText, "COMBO x%d! +%d", combo, comboBonus);

            int textWidth = textwidth(comboText);
            int textX = (WINDOW_WIDTH - textWidth) / 2;
            int textY = WINDOW_HEIGHT / 2 - 50;

            // ������ʾ����
            for (int size = 20; size <= 36; size += 2) {
                drawBackground();
                settextcolor(RGB(255, 100, 0));
                settextstyle(size, 0, _T("Arial"));
                setbkmode(TRANSPARENT);
                outtextxy(textX, textY, comboText);
                FlushBatchDraw();
                Sleep(10);
            }

            Sleep(200);
        }

        // ����������������
        linesCleared += linesCount;

        // ���µȼ�
        int newLevel = min(10, 1 + linesCleared / 10);
        if (newLevel > level) {
            level = newLevel;
            baseSpeed = max(5, 20 - level * 2); // ��ȼ�����ٶ�

            // �ȼ���������
            char levelText[50];
            sprintf(levelText, "LEVEL UP! %d", level);

            int textWidth = textwidth(levelText);
            int textX = (WINDOW_WIDTH - textWidth) / 2;
            int textY = WINDOW_HEIGHT / 2;

            for (int i = 0; i < 5; i++) {
                settextcolor(RGB(255, 215, 0));
                settextstyle(32, 0, _T("Arial Bold"));
                outtextxy(textX, textY, levelText);
                FlushBatchDraw();
                Sleep(100);

                settextcolor(RGB(255, 140, 0));
                outtextxy(textX, textY, levelText);
                FlushBatchDraw();
                Sleep(100);
            }
        }
    }
    else {
        // ��������
        combo = 0;
    }
}

// �����·���
void genBlock() {
    // ����ǵ�һ�����飬��ͬʱ���ɵ�ǰ����һ��
    if (gameTime == 0) {
        time_t t;
        time(&t);
        srand((unsigned int)t);

        type = rand() % 7;
        nextType = rand() % 7;
        nextBlockColor = rand() % 7;
    }
    else {
        // ʹ��֮ǰԤ���ķ���
        type = nextType;
        nextType = rand() % 7;
        nextBlockColor = rand() % 7;
    }

    direction = rand() % 4;
    x = GRID_WIDTH / 2;
    y = 0;

    // ����������ⷽ�� (10%����)
    if (rand() % 10 == 0) {
        currentSpecial = (SpecialBlockType)(1 + rand() % 4);
    }
    else {
        currentSpecial = NONE;
    }
}

// ������Ϸ����
void drawBackground() {
    // ���������Ʊ���
    // ʹ��ָ���ķ�ɫ����
    setbkcolor(RGB(248, 214, 230));
    cleardevice();

    // ������Ϸ���߿�
    int gridLeft = (WINDOW_WIDTH - GRID_WIDTH * BLOCK_SIZE) / 2;
    int gridTop = 40;
    int gridRight = gridLeft + GRID_WIDTH * BLOCK_SIZE;
    int gridBottom = gridTop + GRID_HEIGHT * BLOCK_SIZE;

    setlinecolor(RGB(80, 80, 80));
    setlinestyle(PS_SOLID, 3);
    rectangle(gridLeft - 3, gridTop - 3, gridRight + 3, gridBottom + 3);

    // ���������µķ���
    for (int i = 0; i < GRID_WIDTH; i++) {
        for (int j = 0; j < GRID_HEIGHT; j++) {
            if (landedBlock[i][j] == 0) {
                drawUnitBlock(i, j, 0);
            }
            else {
                drawUnitBlock(i, j, landedBlock[i][j]);
            }
        }
    }

    // ���Ʒ���
    setbkmode(TRANSPARENT);
    settextcolor(RGB(50, 50, 50));
    settextstyle(24, 0, _T("Arial Bold"));

    char scoreText[50];
    sprintf(scoreText, "SCORE: %d", score);
    outtextxy(gridLeft, 8, scoreText);

    // ���Ƶȼ�
    char levelText[30];
    sprintf(levelText, "LEVEL: %d", level);
    outtextxy(gridLeft + 200, 8, levelText);

    // ������һ������Ԥ��
    int previewLeft = gridRight + 20;
    int previewTop = gridTop + 40;

    settextcolor(RGB(50, 50, 50));
    settextstyle(20, 0, _T("Arial"));
    outtextxy(previewLeft, previewTop - 30, _T("NEXT:"));

    // ����Ԥ�������С����
    int previewBlockSize = BLOCK_SIZE - 5;
    for (int i = 0; i < 4; i++) {
        int dx = Block[nextType][0][i][0];
        int dy = Block[nextType][0][i][1];

        int blockX = previewLeft + (dx + 2) * previewBlockSize;
        int blockY = previewTop + (dy + 2) * previewBlockSize;

        // �������
        int left = blockX;
        int top = blockY;
        int right = left + previewBlockSize;
        int bottom = top + previewBlockSize;

        // ��ȡԤ��������ɫ
        int primaryColor = blockColors[nextBlockColor].primary;
        int secondaryColor = blockColors[nextBlockColor].secondary;
        int borderColor = blockColors[nextBlockColor].border;

        // ���Ʊ߿�����
        setlinecolor(borderColor);
        rectangle(left, top, right, bottom);
        setfillcolor(primaryColor);
        solidrectangle(left + 1, top + 1, right - 1, bottom - 1);
    }

    // ��ʾ���ⷽ��״̬
    if (currentSpecial != NONE) {
        settextcolor(RGB(255, 50, 50));
        settextstyle(16, 0, _T("Arial"));

        const TCHAR* specialNames[] = { _T(""), _T("LINE CLEAR"), _T("BOMB"), _T("WEIGHT"), _T("GHOST") };
        outtextxy(previewLeft, previewTop + 100, _T("SPECIAL:"));
        outtextxy(previewLeft, previewTop + 120, specialNames[currentSpecial]);
    }

    // ��������Ч��
    drawParticles();
}

// ���ƿ�ʼ�˵�
void drawMenu() {
    // ���ñ�����ɫΪ��ɫ
    setbkcolor(RGB(248, 214, 230));
    cleardevice();

    // ����
    settextcolor(RGB(80, 30, 100));
    settextstyle(68, 0, _T("Arial Black"));
    setbkmode(TRANSPARENT);

    const TCHAR* title = _T("TETRIS");
    int titleWidth = textwidth(title);
    int titleX = (WINDOW_WIDTH - titleWidth) / 2;
    outtextxy(titleX, 100, title);

    // ���ƶ�̬������ӰЧ��
    settextcolor(RGB(120, 80, 160));
    int shadowOffset = (gameTime / 5) % 5 + 2;
    outtextxy(titleX + shadowOffset, 100 + shadowOffset, title);

    // ��ʼ��Ϸ��ť
    settextcolor(RGB(50, 50, 80));
    settextstyle(28, 0, _T("Arial"));

    const TCHAR* startText = _T("Press ENTER to Start");
    int startWidth = textwidth(startText);
    int startX = (WINDOW_WIDTH - startWidth) / 2;

    // ��˸Ч��
    if ((gameTime / 20) % 2 == 0) {
        outtextxy(startX, 250, startText);
    }

    // ����˵��
    settextcolor(RGB(60, 60, 80));
    settextstyle(18, 0, _T("Consolas"));

    const TCHAR* controls[] = {
        _T("CONTROLS:"),
        _T("A/Left Arrow - Move Left"),
        _T("D/Right Arrow - Move Right"),
        _T("W/Up Arrow - Rotate"),
        _T("S/Down Arrow - Move Down"),
        _T("Spacebar - Hard Drop"),
        _T("P - Pause Game")
    };

    for (int i = 0; i < 7; i++) {
        outtextxy(WINDOW_WIDTH / 2 - 100, 320 + i * 24, controls[i]);
    }

    // �汾��Ϣ
    settextcolor(RGB(80, 80, 110));
    settextstyle(16, 0, _T("Arial"));
    outtextxy(10, WINDOW_HEIGHT - 20, _T("Enhanced Tetris v2.0"));
}

// ������Ϸ��������
void drawGameOver() {
    // ��͸������
    setfillcolor(RGB(150, 100, 150, 180)); // ��͸����ɫ
    solidrectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    // ��Ϸ�����ı�
    settextcolor(RGB(255, 50, 50));
    settextstyle(48, 0, _T("Impact"));
    setbkmode(TRANSPARENT);

    const TCHAR* gameOverText = _T("GAME OVER");
    int textWidth = textwidth(gameOverText);
    outtextxy((WINDOW_WIDTH - textWidth) / 2, WINDOW_HEIGHT / 2 - 100, gameOverText);

    // ��ʾ���շ���
    char finalScore[50];
    sprintf(finalScore, "Final Score: %d", score);

    settextcolor(RGB(220, 220, 220));
    settextstyle(32, 0, _T("Arial"));

    textWidth = textwidth(_T(finalScore));
    outtextxy((WINDOW_WIDTH - textWidth) / 2, WINDOW_HEIGHT / 2 - 20, _T(finalScore));

    // ��ʾ�ȼ�
    char levelText[30];
    sprintf(levelText, "Level Reached: %d", level);

    textWidth = textwidth(_T(levelText));
    outtextxy((WINDOW_WIDTH - textWidth) / 2, WINDOW_HEIGHT / 2 + 20, _T(levelText));

    // ���¿�ʼ��ʾ
    settextcolor(RGB(200, 200, 200));
    settextstyle(24, 0, _T("Arial"));

    const TCHAR* restartText = _T("Press ENTER to Restart");
    textWidth = textwidth(restartText);

    // ��˸Ч��
    if ((gameTime / 20) % 2 == 0) {
        outtextxy((WINDOW_WIDTH - textWidth) / 2, WINDOW_HEIGHT / 2 + 80, restartText);
    }

    // ��ʾ����Ч��
    drawParticles();
}

// ������ͣ����
void drawPauseScreen() {
    // ��͸������
    setfillcolor(RGB(150, 120, 180, 150)); // ��͸����ɫ
    solidrectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    // ��ͣ�ı�
    settextcolor(RGB(50, 50, 80));
    settextstyle(48, 0, _T("Arial Black"));
    setbkmode(TRANSPARENT);

    const TCHAR* pauseText = _T("PAUSED");
    int textWidth = textwidth(pauseText);
    outtextxy((WINDOW_WIDTH - textWidth) / 2, WINDOW_HEIGHT / 2 - 50, pauseText);

    // ������ʾ
    settextcolor(RGB(60, 60, 80));
    settextstyle(24, 0, _T("Arial"));

    const TCHAR* continueText = _T("Press P to Continue");
    textWidth = textwidth(continueText);

    // ��˸Ч��
    if ((gameTime / 20) % 2 == 0) {
        outtextxy((WINDOW_WIDTH - textWidth) / 2, WINDOW_HEIGHT / 2 + 20, continueText);
    }
}

// ��ʼ����Ϸ
void initGame() {
    // ��ʼ��ͼ�λ���
    initgraph(WINDOW_WIDTH, WINDOW_HEIGHT);

    // ���ñ���Ϊ��ɫ
    setbkcolor(RGB(248, 214, 230));
    cleardevice();

    // ��ʼ����Ϸ����
    gameTime = 0;
    score = 0;
    level = 1;
    linesCleared = 0;
    combo = 0;
    fallSpeed = baseSpeed = 20;
    gameover = false;
    currentSpecial = NONE;

    // �����Ϸ��
    memset(landedBlock, 0, sizeof(landedBlock));
    memset(specialEffects, 0, sizeof(specialEffects));
    particles.clear();

    // ��ʼ����������ģʽ
    BeginBatchDraw();
}

// ������Ϸ
void resetGame() {
    // ������Ϸ����
    score = 0;
    level = 1;
    linesCleared = 0;
    combo = 0;
    fallSpeed = baseSpeed = 20;
    gameover = false;
    currentSpecial = NONE;

    // �����Ϸ��
    memset(landedBlock, 0, sizeof(landedBlock));
    memset(specialEffects, 0, sizeof(specialEffects));
    particles.clear();

    // �����·���
    genBlock();

    // �л�����Ϸ״̬
    gameState = PLAYING;
}

// ��Ϸ��ѭ��
void gameLoop() {
    // ���ɵ�һ������
    genBlock();

    // ��ѭ��
    while (1) {
        // ������Ϸʱ��
        gameTime++;

        // ��������Ч��
        updateParticles();

        // ������Ϸ״̬�ֱ���
        switch (gameState) {
        case MENU:
            if (_kbhit()) {
                char key = _getch();
                if (key == 13) { // Enter��
                    resetGame();
                    gameState = PLAYING;
                }
                else if (key == 27) { // ESC��
                    return; // �˳���Ϸ
                }
            }

            drawMenu();
            break;

        case PLAYING:
            // �����������
            if (_kbhit()) {
                char key = _getch();
                if ((key == 'a' || key == 'A' || key == 75) && canGoLeft(x, y, type, direction))
                    x--;
                else if ((key == 'd' || key == 'D' || key == 77) && canGoRight(x, y, type, direction))
                    x++;
                else if ((key == 'w' || key == 'W' || key == 72) && canRotation(x, y, type, direction)) {
                    direction = (direction + 1) % 4;
                }
                else if (key == 's' || key == 'S' || key == 80) {
                    if (canFall(x, y, type, direction)) {
                        y++;
                        score++; // ����ӷ�
                    }
                }
                else if (key == ' ') { // Ӳ����
                    while (canFall(x, y, type, direction)) {
                        y++;
                        score += 2; // Ӳ����ӷ�
                    }
                }
                else if (key == 'p' || key == 'P') {
                    gameState = PAUSED;
                }
                else if (key == 27) { // ESC��
                    gameState = MENU;
                }
            }

            // �Զ�����
            if (fallSpeed <= 0) {
                if (canFall(x, y, type, direction)) {
                    y++;
                }
                else {
                    // ���⴦��Ȩ�ط�����ظ���
                    if (currentSpecial == WEIGHT) {
                        LandedBlock(x, y, type, direction, type);
                        genBlock();
                    }
                    else if (y == 0) {
                        gameover = true;
                        gameState = GAMEOVER;

                        // ��Ϸ������Ч
                        for (int i = 0; i < 100; i++) {
                            int randX = rand() % GRID_WIDTH;
                            int randY = rand() % GRID_HEIGHT;
                            addParticles(randX, randY, 5, RGB(255, 100, 100));
                        }
                    }
                    else {
                        LandedBlock(x, y, type, direction, type);
                        genBlock();
                    }
                }
                fallSpeed = baseSpeed;
            }
            else {
                fallSpeed--;
            }

            drawBackground();

            // ���Ʒ�����Ӱ
            drawBlockShadow(x, y, type, direction);

            // ���Ƶ�ǰ����
            drawBlock(x, y, type, direction, type);
            break;

        case PAUSED:
            if (_kbhit()) {
                char key = _getch();
                if (key == 'p' || key == 'P') {
                    gameState = PLAYING;
                }
                else if (key == 27) { // ESC��
                    gameState = MENU;
                }
            }

            drawBackground();
            drawPauseScreen();
            break;

        case GAMEOVER:
            if (_kbhit()) {
                char key = _getch();
                if (key == 13) { // Enter��
                    resetGame();
                    gameState = PLAYING;
                }
                else if (key == 27) { // ESC��
                    gameState = MENU;
                }
            }

            drawBackground();
            drawGameOver();
            break;
        }

        FlushBatchDraw();
        Sleep(16); // Լ60fps
    }
}

// ������
int main() {
    // Ϊ����������ʾ���⣬�����ַ���
    SetConsoleOutputCP(65001); // UTF-8

    initGame();
    gameState = MENU;
    gameLoop();
    EndBatchDraw();
    closegraph();
    return 0;
}
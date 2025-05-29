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

// 基础参数设置
#define BLOCK_SIZE 25         // 方块大小
#define GRID_WIDTH 12         // 游戏区宽度
#define GRID_HEIGHT 20        // 游戏区高度
#define WINDOW_WIDTH 600      // 窗口宽度
#define WINDOW_HEIGHT 600     // 窗口高度

// 游戏状态
enum GameState {
    MENU,
    PLAYING,
    PAUSED,
    GAMEOVER
};

// 方块颜色 - 使用渐变色对
struct BlockColor {
    int primary;
    int secondary;
    int border;
};

BlockColor blockColors[7] = {
    {RGB(164,203,236), RGB(126,182,228), RGB(80,140,200)},   // 蓝色系
    {RGB(214,229,138), RGB(190,212,100), RGB(150,180,80)},   // 绿色系
    {RGB(240,244,168), RGB(230,235,130), RGB(200,205,100)},  // 黄色系
    {RGB(227,115,103), RGB(210,80,70),   RGB(180,50,40)},    // 红色系
    {RGB(181,230,211), RGB(150,210,190), RGB(110,180,160)},  // 青色系
    {RGB(99,111,153),  RGB(80,90,140),   RGB(60,70,120)},    // 紫色系
    {RGB(177,195,121), RGB(160,180,100), RGB(140,160,80)}    // 橄榄绿系
};

// 方块类型数据
int Block[7][4][4][2] = {
    // 七种基本形状，每种四个旋转方向
    {/*Type 0 - T形*/
        {{0, 0}, {-1, 0}, {1, 0}, {0, -1}},
        {{0, 0}, {1, 0}, {0, 1}, {0, -1}},
        {{0, 0}, {-1, 0}, {1, 0}, {0, 1}},
        {{0, 0}, {-1, 0}, {0, 1}, {0, -1}}
    },
    {/*Type 1 - Z形*/
        {{0, 0},{0,-1},{1,-1},{-1,0}},
        {{0, 0},{0,-1},{1,0},{1,1}},
        {{0, 0},{-1,0},{0,-1},{1,-1}},
        {{0, 0},{0,-1},{1,0},{1,1}}
    },
    {/*Type 2 - S形*/
        {{0, 0},{-1,-1},{0,-1},{1,0}},
        {{0, 0},{1,0},{0,1},{1,-1}},
        {{0, 0},{-1,-1},{0,-1},{1,0}},
        {{0, 0},{1,-1},{1,0},{0,1}}
    },
    {/*Type 3 - I形*/
        {{0, 0},{-2,0},{-1,0},{1,0}},
        {{0, 0},{0,-2},{0,-1},{0,1}},
        {{0, 0},{-2,0},{-1,0},{1,0}},
        {{0, 0},{0,-2},{0,-1},{0,1}}
    },
    {/*Type 4 - O形*/
        {{0,0},{-1,-1},{0,-1},{-1,0}},
        {{0,0},{-1,-1},{0,-1},{-1,0}},
        {{0,0},{-1,-1},{0,-1},{-1,0}},
        {{0,0},{-1,-1},{0,-1},{-1,0}}
    },
    {/*Type 5 - J形*/
        {{0,0},{1,-1},{-1,0},{1,0}},
        {{1,1},{0,0},{0,-1},{0,1}},
        {{-1,1},{0,0},{-1,0},{1,0}},
        {{-1,-1},{0,0},{0,1},{0,-1}}
    },
    {/*Type 6 - L形*/
        {{0,0},{-1,0},{1,0},{1,1}},
        {{0,0},{0,1},{0,-1},{-1,1}},
        {{0,0},{-1,0},{1,0},{-1,-1}},
        {{0,0},{0,1},{0,-1},{1,-1}}
    }
};

// 特殊方块类型 (创新元素)
enum SpecialBlockType {
    NONE,
    LINE_CLEAR,     // 清除一整行
    BOMB,           // 爆炸清除周围方块
    WEIGHT,         // 快速下落并得分加倍
    GHOST           // 可以穿过已有方块
};

// 粒子效果
struct Particle {
    float x, y;
    float vx, vy;
    int color;
    int life;
    float size;
};

// 游戏全局变量
GameState gameState = MENU;
SpecialBlockType currentSpecial = NONE;
vector<Particle> particles;

int landedBlock[GRID_WIDTH][GRID_HEIGHT] = { 0 };
int specialEffects[GRID_WIDTH][GRID_HEIGHT] = { 0 }; // 特殊效果标记
int type, nextType, direction;
int x, y;
int fallSpeed = 20;
int baseSpeed = 20;
int score = 0;
int level = 1;
int linesCleared = 0;
int combo = 0;
int gameTime = 0; // 游戏计时
int nextBlockColor;
bool gameover = false;

// 函数前向声明
bool canFall(int x, int y, int type, int direction);
void drawBackground();

// 粒子效果系统
void addParticles(int centerX, int centerY, int count, int color) {
    for (int i = 0; i < count; i++) {
        Particle p;
        p.x = centerX * BLOCK_SIZE + BLOCK_SIZE / 2.0f;
        p.y = centerY * BLOCK_SIZE + BLOCK_SIZE / 2.0f;

        // 随机速度方向
        float angle = (rand() % 360) * 3.14159f / 180.0f;
        float speed = 0.5f + (rand() % 20) / 10.0f;

        p.vx = cos(angle) * speed;
        p.vy = sin(angle) * speed;
        p.color = color;
        p.life = 30 + rand() % 60; // 粒子寿命
        p.size = 1.0f + (rand() % 10) / 10.0f;

        particles.push_back(p);
    }
}

void updateParticles() {
    for (int i = 0; i < particles.size(); i++) {
        particles[i].x += particles[i].vx;
        particles[i].y += particles[i].vy;
        particles[i].vy += 0.05f; // 重力效果
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

// 绘制单个方块单元，使用渐变效果
void drawUnitBlock(int x, int y, int colorIndex) {
    int left = x * BLOCK_SIZE + (WINDOW_WIDTH - GRID_WIDTH * BLOCK_SIZE) / 2;
    int right = left + BLOCK_SIZE;
    int top = y * BLOCK_SIZE + 40; // 顶部留出空间显示分数
    int bottom = top + BLOCK_SIZE;

    if (colorIndex == 0) { // 空白方块
        setlinecolor(RGB(180, 180, 180));
        rectangle(left, top, right, bottom);

        // 淡色网格效果
        setfillcolor(RGB(240, 240, 240));
        solidrectangle(left + 1, top + 1, right - 1, bottom - 1);
    }
    else {
        // 获取方块颜色
        int primaryColor = blockColors[colorIndex - 1].primary;
        int secondaryColor = blockColors[colorIndex - 1].secondary;
        int borderColor = blockColors[colorIndex - 1].border;

        // 绘制边框
        setlinecolor(borderColor);
        rectangle(left, top, right, bottom);

        // 渐变填充
        for (int i = 0; i < BLOCK_SIZE - 2; i++) {
            // 计算渐变色
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

        // 特效标记
        if (specialEffects[x][y] > 0) {
            // 特殊方块效果
            switch (specialEffects[x][y]) {
            case LINE_CLEAR: // 直线效果
                setlinecolor(RGB(255, 255, 0));
                line(left + 2, top + BLOCK_SIZE / 2, right - 2, top + BLOCK_SIZE / 2);
                break;
            case BOMB: // 爆炸效果
                setlinecolor(RGB(255, 0, 0));
                circle(left + BLOCK_SIZE / 2, top + BLOCK_SIZE / 2, BLOCK_SIZE / 4);
                break;
            case WEIGHT: // 重量效果
                setlinecolor(RGB(100, 100, 100));
                rectangle(left + BLOCK_SIZE / 4, top + BLOCK_SIZE / 4,
                    right - BLOCK_SIZE / 4, bottom - BLOCK_SIZE / 4);
                break;
            case GHOST: // 幽灵效果
                setlinecolor(RGB(200, 200, 255));
                for (int i = 0; i < 4; i++) {
                    line(left + i * 5, top + 2, left + i * 5, bottom - 2);
                }
                break;
            }
        }

        // 光泽效果
        setlinecolor(RGB(255, 255, 255));
        setlinestyle(PS_SOLID, 1);
        line(left + 2, top + 2, right - 2, top + 2);
        line(left + 2, top + 2, left + 2, bottom - 2);
    }
}

// 绘制方块
void drawBlock(int x, int y, int type, int direction, int colorIndex) {
    for (int i = 0; i < 4; i++) {
        int dx = Block[type][direction][i][0];
        int dy = Block[type][direction][i][1];
        int screenX = x + dx;
        int screenY = y + dy;

        drawUnitBlock(screenX, screenY, colorIndex + 1);
    }
}

// 绘制方块阴影（显示落点）
void drawBlockShadow(int x, int y, int type, int direction) {
    int shadowY = y;

    // 找到方块的最终落点
    while (canFall(x, shadowY, type, direction)) {
        shadowY++;
    }

    if (shadowY != y) {
        // 用半透明效果绘制阴影
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

// 游戏边界检查函数
bool canGoLeft(int x, int y, int type, int direction) {
    for (int i = 0; i < 4; i++) {
        int dx = Block[type][direction][i][0];
        int dy = Block[type][direction][i][1];
        int newX = x + dx - 1;
        int newY = y + dy;

        // 检查边界和碰撞
        if (newX < 0 || newX >= GRID_WIDTH || newY >= GRID_HEIGHT ||
            (newY >= 0 && landedBlock[newX][newY] != 0)) {

            // 特殊模式：幽灵方块可以穿过
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

// 方块消除动画效果
void animateLineClearing(int lineY) {
    // 闪烁效果
    for (int flash = 0; flash < 3; flash++) {
        // 将要消除的行变成白色
        for (int i = 0; i < GRID_WIDTH; i++) {
            int left = i * BLOCK_SIZE + (WINDOW_WIDTH - GRID_WIDTH * BLOCK_SIZE) / 2;
            int top = lineY * BLOCK_SIZE + 40;

            setfillcolor(RGB(255, 255, 255));
            solidrectangle(left, top, left + BLOCK_SIZE, top + BLOCK_SIZE);
        }
        FlushBatchDraw();
        Sleep(80);

        // 将要消除的行变回原来的颜色
        for (int i = 0; i < GRID_WIDTH; i++) {
            drawUnitBlock(i, lineY, landedBlock[i][lineY]);
        }
        FlushBatchDraw();
        Sleep(80);
    }

    // 消除行的粒子效果
    for (int i = 0; i < GRID_WIDTH; i++) {
        addParticles(i, lineY, 8, RGB(255, 220, 150));
    }
}

// 炸弹特效
void bombEffect(int centerX, int centerY) {
    // 添加爆炸粒子效果
    addParticles(centerX, centerY, 40, RGB(255, 100, 0));

    // 清除周围的方块
    for (int dx = -2; dx <= 2; dx++) {
        for (int dy = -2; dy <= 2; dy++) {
            int newX = centerX + dx;
            int newY = centerY + dy;

            if (newX >= 0 && newX < GRID_WIDTH && newY >= 0 && newY < GRID_HEIGHT) {
                // 计算与中心的距离
                float distance = sqrt(dx * dx + dy * dy);

                if (distance <= 2.5f) { // 爆炸半径
                    landedBlock[newX][newY] = 0;
                    specialEffects[newX][newY] = 0;

                    // 爆炸动画
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

    score += 30; // 炸弹加分
}

// 处理方块落地
void LandedBlock(int x, int y, int type, int direction, int colorIndex) {
    // 特殊效果处理
    bool hasBomb = false;
    int bombX = 0, bombY = 0;

    // 记录方块位置
    for (int i = 0; i < 4; i++) {
        int dx = Block[type][direction][i][0];
        int dy = Block[type][direction][i][1];
        int newX = x + dx;
        int newY = y + dy;

        if (newY >= 0) {
            landedBlock[newX][newY] = colorIndex + 1;

            // 记录特殊效果
            if (currentSpecial != NONE) {
                specialEffects[newX][newY] = currentSpecial;

                // 记录炸弹位置
                if (currentSpecial == BOMB) {
                    hasBomb = true;
                    bombX = newX;
                    bombY = newY;
                }

                // 直线清除
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

    // 应用炸弹效果
    if (hasBomb) {
        bombEffect(bombX, bombY);
    }

    // 权重方块给予额外分数
    if (currentSpecial == WEIGHT) {
        score += 15;
    }

    // 检查并消除已满的行
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

            // 添加消除动画
            animateLineClearing(j);

            // 原有的行消除逻辑
            for (int k = j; k > 0; k--) {
                for (int i = 0; i < GRID_WIDTH; i++) {
                    landedBlock[i][k] = landedBlock[i][k - 1];
                    specialEffects[i][k] = specialEffects[i][k - 1];
                }
            }

            // 清除顶行
            for (int i = 0; i < GRID_WIDTH; i++) {
                landedBlock[i][0] = 0;
                specialEffects[i][0] = 0;
            }
        }
    }

    // 更新分数和连击
    if (linesCount > 0) {
        combo++;
        int basePoints = 0;

        // 基础分数
        switch (linesCount) {
        case 1: basePoints = 40; break;
        case 2: basePoints = 100; break;
        case 3: basePoints = 300; break;
        case 4: basePoints = 1200; break;
        }

        // 连击奖励
        int comboBonus = min(combo * 50, 500);

        // 计算总分
        int totalPoints = basePoints * level + comboBonus;
        score += totalPoints;

        // 显示连击信息
        if (combo > 1) {
            char comboText[50];
            sprintf(comboText, "COMBO x%d! +%d", combo, comboBonus);

            int textWidth = textwidth(comboText);
            int textX = (WINDOW_WIDTH - textWidth) / 2;
            int textY = WINDOW_HEIGHT / 2 - 50;

            // 连击显示动画
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

        // 更新已消除的行数
        linesCleared += linesCount;

        // 更新等级
        int newLevel = min(10, 1 + linesCleared / 10);
        if (newLevel > level) {
            level = newLevel;
            baseSpeed = max(5, 20 - level * 2); // 随等级提高速度

            // 等级提升动画
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
        // 重置连击
        combo = 0;
    }
}

// 生成新方块
void genBlock() {
    // 如果是第一个方块，则同时生成当前和下一个
    if (gameTime == 0) {
        time_t t;
        time(&t);
        srand((unsigned int)t);

        type = rand() % 7;
        nextType = rand() % 7;
        nextBlockColor = rand() % 7;
    }
    else {
        // 使用之前预览的方块
        type = nextType;
        nextType = rand() % 7;
        nextBlockColor = rand() % 7;
    }

    direction = rand() % 4;
    x = GRID_WIDTH / 2;
    y = 0;

    // 随机生成特殊方块 (10%概率)
    if (rand() % 10 == 0) {
        currentSpecial = (SpecialBlockType)(1 + rand() % 4);
    }
    else {
        currentSpecial = NONE;
    }
}

// 绘制游戏界面
void drawBackground() {
    // 清屏并绘制背景
    // 使用指定的粉色背景
    setbkcolor(RGB(248, 214, 230));
    cleardevice();

    // 绘制游戏区边框
    int gridLeft = (WINDOW_WIDTH - GRID_WIDTH * BLOCK_SIZE) / 2;
    int gridTop = 40;
    int gridRight = gridLeft + GRID_WIDTH * BLOCK_SIZE;
    int gridBottom = gridTop + GRID_HEIGHT * BLOCK_SIZE;

    setlinecolor(RGB(80, 80, 80));
    setlinestyle(PS_SOLID, 3);
    rectangle(gridLeft - 3, gridTop - 3, gridRight + 3, gridBottom + 3);

    // 绘制已落下的方块
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

    // 绘制分数
    setbkmode(TRANSPARENT);
    settextcolor(RGB(50, 50, 50));
    settextstyle(24, 0, _T("Arial Bold"));

    char scoreText[50];
    sprintf(scoreText, "SCORE: %d", score);
    outtextxy(gridLeft, 8, scoreText);

    // 绘制等级
    char levelText[30];
    sprintf(levelText, "LEVEL: %d", level);
    outtextxy(gridLeft + 200, 8, levelText);

    // 绘制下一个方块预览
    int previewLeft = gridRight + 20;
    int previewTop = gridTop + 40;

    settextcolor(RGB(50, 50, 50));
    settextstyle(20, 0, _T("Arial"));
    outtextxy(previewLeft, previewTop - 30, _T("NEXT:"));

    // 绘制预览方块的小方格
    int previewBlockSize = BLOCK_SIZE - 5;
    for (int i = 0; i < 4; i++) {
        int dx = Block[nextType][0][i][0];
        int dy = Block[nextType][0][i][1];

        int blockX = previewLeft + (dx + 2) * previewBlockSize;
        int blockY = previewTop + (dy + 2) * previewBlockSize;

        // 方块绘制
        int left = blockX;
        int top = blockY;
        int right = left + previewBlockSize;
        int bottom = top + previewBlockSize;

        // 获取预览方块颜色
        int primaryColor = blockColors[nextBlockColor].primary;
        int secondaryColor = blockColors[nextBlockColor].secondary;
        int borderColor = blockColors[nextBlockColor].border;

        // 绘制边框和填充
        setlinecolor(borderColor);
        rectangle(left, top, right, bottom);
        setfillcolor(primaryColor);
        solidrectangle(left + 1, top + 1, right - 1, bottom - 1);
    }

    // 显示特殊方块状态
    if (currentSpecial != NONE) {
        settextcolor(RGB(255, 50, 50));
        settextstyle(16, 0, _T("Arial"));

        const TCHAR* specialNames[] = { _T(""), _T("LINE CLEAR"), _T("BOMB"), _T("WEIGHT"), _T("GHOST") };
        outtextxy(previewLeft, previewTop + 100, _T("SPECIAL:"));
        outtextxy(previewLeft, previewTop + 120, specialNames[currentSpecial]);
    }

    // 绘制粒子效果
    drawParticles();
}

// 绘制开始菜单
void drawMenu() {
    // 设置背景颜色为粉色
    setbkcolor(RGB(248, 214, 230));
    cleardevice();

    // 标题
    settextcolor(RGB(80, 30, 100));
    settextstyle(68, 0, _T("Arial Black"));
    setbkmode(TRANSPARENT);

    const TCHAR* title = _T("TETRIS");
    int titleWidth = textwidth(title);
    int titleX = (WINDOW_WIDTH - titleWidth) / 2;
    outtextxy(titleX, 100, title);

    // 绘制动态标题阴影效果
    settextcolor(RGB(120, 80, 160));
    int shadowOffset = (gameTime / 5) % 5 + 2;
    outtextxy(titleX + shadowOffset, 100 + shadowOffset, title);

    // 开始游戏按钮
    settextcolor(RGB(50, 50, 80));
    settextstyle(28, 0, _T("Arial"));

    const TCHAR* startText = _T("Press ENTER to Start");
    int startWidth = textwidth(startText);
    int startX = (WINDOW_WIDTH - startWidth) / 2;

    // 闪烁效果
    if ((gameTime / 20) % 2 == 0) {
        outtextxy(startX, 250, startText);
    }

    // 控制说明
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

    // 版本信息
    settextcolor(RGB(80, 80, 110));
    settextstyle(16, 0, _T("Arial"));
    outtextxy(10, WINDOW_HEIGHT - 20, _T("Enhanced Tetris v2.0"));
}

// 绘制游戏结束画面
void drawGameOver() {
    // 半透明遮罩
    setfillcolor(RGB(150, 100, 150, 180)); // 半透明紫色
    solidrectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    // 游戏结束文本
    settextcolor(RGB(255, 50, 50));
    settextstyle(48, 0, _T("Impact"));
    setbkmode(TRANSPARENT);

    const TCHAR* gameOverText = _T("GAME OVER");
    int textWidth = textwidth(gameOverText);
    outtextxy((WINDOW_WIDTH - textWidth) / 2, WINDOW_HEIGHT / 2 - 100, gameOverText);

    // 显示最终分数
    char finalScore[50];
    sprintf(finalScore, "Final Score: %d", score);

    settextcolor(RGB(220, 220, 220));
    settextstyle(32, 0, _T("Arial"));

    textWidth = textwidth(_T(finalScore));
    outtextxy((WINDOW_WIDTH - textWidth) / 2, WINDOW_HEIGHT / 2 - 20, _T(finalScore));

    // 显示等级
    char levelText[30];
    sprintf(levelText, "Level Reached: %d", level);

    textWidth = textwidth(_T(levelText));
    outtextxy((WINDOW_WIDTH - textWidth) / 2, WINDOW_HEIGHT / 2 + 20, _T(levelText));

    // 重新开始提示
    settextcolor(RGB(200, 200, 200));
    settextstyle(24, 0, _T("Arial"));

    const TCHAR* restartText = _T("Press ENTER to Restart");
    textWidth = textwidth(restartText);

    // 闪烁效果
    if ((gameTime / 20) % 2 == 0) {
        outtextxy((WINDOW_WIDTH - textWidth) / 2, WINDOW_HEIGHT / 2 + 80, restartText);
    }

    // 显示粒子效果
    drawParticles();
}

// 绘制暂停画面
void drawPauseScreen() {
    // 半透明遮罩
    setfillcolor(RGB(150, 120, 180, 150)); // 半透明紫色
    solidrectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    // 暂停文本
    settextcolor(RGB(50, 50, 80));
    settextstyle(48, 0, _T("Arial Black"));
    setbkmode(TRANSPARENT);

    const TCHAR* pauseText = _T("PAUSED");
    int textWidth = textwidth(pauseText);
    outtextxy((WINDOW_WIDTH - textWidth) / 2, WINDOW_HEIGHT / 2 - 50, pauseText);

    // 继续提示
    settextcolor(RGB(60, 60, 80));
    settextstyle(24, 0, _T("Arial"));

    const TCHAR* continueText = _T("Press P to Continue");
    textWidth = textwidth(continueText);

    // 闪烁效果
    if ((gameTime / 20) % 2 == 0) {
        outtextxy((WINDOW_WIDTH - textWidth) / 2, WINDOW_HEIGHT / 2 + 20, continueText);
    }
}

// 初始化游戏
void initGame() {
    // 初始化图形环境
    initgraph(WINDOW_WIDTH, WINDOW_HEIGHT);

    // 设置背景为粉色
    setbkcolor(RGB(248, 214, 230));
    cleardevice();

    // 初始化游戏变量
    gameTime = 0;
    score = 0;
    level = 1;
    linesCleared = 0;
    combo = 0;
    fallSpeed = baseSpeed = 20;
    gameover = false;
    currentSpecial = NONE;

    // 清空游戏区
    memset(landedBlock, 0, sizeof(landedBlock));
    memset(specialEffects, 0, sizeof(specialEffects));
    particles.clear();

    // 初始化批量绘制模式
    BeginBatchDraw();
}

// 重置游戏
void resetGame() {
    // 重置游戏变量
    score = 0;
    level = 1;
    linesCleared = 0;
    combo = 0;
    fallSpeed = baseSpeed = 20;
    gameover = false;
    currentSpecial = NONE;

    // 清空游戏区
    memset(landedBlock, 0, sizeof(landedBlock));
    memset(specialEffects, 0, sizeof(specialEffects));
    particles.clear();

    // 生成新方块
    genBlock();

    // 切换到游戏状态
    gameState = PLAYING;
}

// 游戏主循环
void gameLoop() {
    // 生成第一个方块
    genBlock();

    // 主循环
    while (1) {
        // 更新游戏时间
        gameTime++;

        // 更新粒子效果
        updateParticles();

        // 根据游戏状态分别处理
        switch (gameState) {
        case MENU:
            if (_kbhit()) {
                char key = _getch();
                if (key == 13) { // Enter键
                    resetGame();
                    gameState = PLAYING;
                }
                else if (key == 27) { // ESC键
                    return; // 退出游戏
                }
            }

            drawMenu();
            break;

        case PLAYING:
            // 处理键盘输入
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
                        score++; // 下落加分
                    }
                }
                else if (key == ' ') { // 硬降落
                    while (canFall(x, y, type, direction)) {
                        y++;
                        score += 2; // 硬降落加分
                    }
                }
                else if (key == 'p' || key == 'P') {
                    gameState = PAUSED;
                }
                else if (key == 27) { // ESC键
                    gameState = MENU;
                }
            }

            // 自动下落
            if (fallSpeed <= 0) {
                if (canFall(x, y, type, direction)) {
                    y++;
                }
                else {
                    // 特殊处理：权重方块落地更快
                    if (currentSpecial == WEIGHT) {
                        LandedBlock(x, y, type, direction, type);
                        genBlock();
                    }
                    else if (y == 0) {
                        gameover = true;
                        gameState = GAMEOVER;

                        // 游戏结束特效
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

            // 绘制方块阴影
            drawBlockShadow(x, y, type, direction);

            // 绘制当前方块
            drawBlock(x, y, type, direction, type);
            break;

        case PAUSED:
            if (_kbhit()) {
                char key = _getch();
                if (key == 'p' || key == 'P') {
                    gameState = PLAYING;
                }
                else if (key == 27) { // ESC键
                    gameState = MENU;
                }
            }

            drawBackground();
            drawPauseScreen();
            break;

        case GAMEOVER:
            if (_kbhit()) {
                char key = _getch();
                if (key == 13) { // Enter键
                    resetGame();
                    gameState = PLAYING;
                }
                else if (key == 27) { // ESC键
                    gameState = MENU;
                }
            }

            drawBackground();
            drawGameOver();
            break;
        }

        FlushBatchDraw();
        Sleep(16); // 约60fps
    }
}

// 主函数
int main() {
    // 为避免中文显示问题，设置字符集
    SetConsoleOutputCP(65001); // UTF-8

    initGame();
    gameState = MENU;
    gameLoop();
    EndBatchDraw();
    closegraph();
    return 0;
}
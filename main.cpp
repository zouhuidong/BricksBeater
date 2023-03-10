#include <math.h>
#include <time.h>
#include <graphics.h>
#include "ini.hpp"

// 引用 Windows Multimedia API
#pragma comment(lib, "Winmm.lib")

/////////////////// 常量定义 ///////////////////

// 圆周率
#define PI					3.1415926f

// 程序界面属性
#define STATE_BAR_HEIGHT		150		// 状态栏高度
#define WINDOW_BOTTOM_MARGIN	187		// 窗口底部边距（留白）

// 方块总边长
#define SIDE_LEN	(g_nInnerSideLen + 2 * g_nInnerInterval)

// 挡板属性
#define BOARD_HALF_WIDTH		40		// 挡板半宽
#define BOARD_HALF_THICKNESS	3		// 挡板半厚度
#define BOARD_Y_DISTACE			100		// 挡板比地图底部低多少
#define BOARD_BALL_DISTANCE		16		// 待发射小球离挡板多远
#define BOARD_Y					(g_nMapH * SIDE_LEN + BOARD_Y_DISTACE)	// 挡板 y 坐标

// 小球属性
#define BALL_MAX_NUM		1024	// 最大小球数量
#define BALL_LIFE_NUM		4		// 球有几条命
#define BALL_SPEED			2.4f	// 小球速度

// 算法设置
#define PROCESS_REPEAT		3		// 在一帧内计算几次游戏状态

// 方块颜色
#define COLOR_WALL			RGB(163, 163, 162)	// 墙：灰色
#define COLOR_BRICK_R		RGB(214, 63, 37)	// 红 R
#define COLOR_BRICK_Y		RGB(228, 174, 31)	// 黄 Y
#define COLOR_BRICK_G		RGB(15, 216, 15)	// 绿 G
#define COLOR_BRICK_C		RGB(39, 253, 251)	// 青 C
#define COLOR_BRICK_B		RGB(68, 112, 202)	// 蓝 B
#define COLOR_BRICK_W		RGB(234, 234, 234)	// 白 W

// 方块类型标记
#define EMPTY				0
#define WALL				1
#define BRICK				2

// 道具类型标记
#define PROP_3X				0	// 每个小球向外发射三个小球
#define PROP_SHOOT_3		1	// 发射三个小球
#define PROP_HEART			2	// 加一条命

#define PROP_MAX_NUM		1024	// 最大道具数量

// 道具下落速度
#define PROP_DROP_SPEED			0.2f

// 碰撞结果
#define HIT_NONE			0	// 未发生任何碰撞
#define HIT_NORMAL			1	// 发生一次普通碰撞（与矩形水平边或竖直边的碰撞）
#define HIT_VERTEX			2	// 与矩形顶点发生了碰撞

// 游戏状态
#define GAME_WIN			1		// 赢
#define GAME_LOSE			2		// 输

// 资源路径
#define SETTINGS_PATH		_T("./BricksBeater.ini")	// 配置文件
#define LEVEL_FOLDER_PATH	("./res/level")				// 关卡文件存储路径（文件夹）

#define MUSIC_PING_PATH		_T("./res/music/ping.mp3")	// 碰撞声 1
#define MUSIC_PONG_PATH		_T("./res/music/pong.mp3")	// 碰撞声 2
#define MUSIC_PROP_PATH		_T("./res/music/prop.mp3")	// 领道具的声音

#define IMAGE_RESTART		_T("./res/pic/restart.gif")
#define IMAGE_3X_PATH		_T("./res/pic/prop_3x.gif")
#define IMAGE_SHOOT_3_PATH	_T("./res/pic/prop_shoot_3.gif")
#define IMAGE_HEART_PATH	_T("./res/pic/prop_heart.gif")

/////////////////// 全局变量 ///////////////////

////////// 游戏基础设定和资源

int g_nMapW;								// 地图宽（单位：方块）
int g_nMapH;								// 地图高（单位：方块）
int g_nLevelNum;							// 关卡数
int g_nCurrentLevel;						// 当前是第几关（从 0 开始数）
char*** g_pppszOriginalLevelMap;			// 各关卡原始地图

// 调试模式配置
bool g_bDebugMode = false;					// 是否处于调试模式（在配置文件中设置的）
bool g_bOnlyVertexCollisionSound = false;	// 只有小球与矩形顶点发生碰撞时才发出声音
bool g_bBigScene = false;					// 大场景模式

// 大场景模式配置
int g_nInnerSideLen_BigScene = 16;			// 大场景模式下的方块内填充矩形边长
int g_nBallRadius_BigScene = 10;			// 大场景模式下的小球半径

// 方块大小属性
int g_nInnerSideLen = 7;		// 方块内填充矩形边长
int g_nInnerInterval = 1;		// 方块外周留空边距

// 小球半径
int g_nBallRadius = 3;

// 道具出现概率（百分比）
int g_nProbability_3X = 8;
int g_nProbability_Shoot_3 = 12;
int g_nProbability_Heart = 5;

// 图像资源
IMAGE g_imgRestart;
IMAGE g_img3X;
IMAGE g_imgShoot3;
IMAGE g_imgHeart;

// “重新开始” 按钮区域
RECT g_rctRestartBtn;

//////////

bool g_bInMenu = true;	// 是否还在主菜单

// 游戏状态
enum GameState { Gaming, GameWin, GameFail };
GameState g_enuGameState = Gaming;

// 砖块
struct Brick
{
	int type;		// 方块类型（砖块被打掉之后会被改为 EMPTY）
	COLORREF color;	// 方块颜色
	RECT rct;		// 砖块碰撞箱位置
}*g_pstMap;

int g_nBoardCenterX;	// 挡板中心 x 坐标

// 小球
struct Ball
{
	bool alive;		// 该小球是否已经出界了
	float x;
	float y;
	float vx;
	float vy;
}g_pstBalls[BALL_MAX_NUM];

int g_nGeneratedBallNum;		// 已经生成出的小球数量（小球出界不减数）
bool g_bBallWait;				// 是否正在等待发射小球

int g_nRemainingBallNum;	// 还剩几个球

// 道具
struct Prop
{
	int type;		// 种类
	bool used;		// 是否已被使用
	float x;
	float y;
}g_pstProps[PROP_MAX_NUM];

int g_nGeneratedPropNum;		// 已经生成的道具数量

/////////////////// 函数定义 ///////////////////

// 加载游戏资源
void InitResource()
{
	// 读取配置
	g_nMapW = GetIniFileInfoInt(SETTINGS_PATH, _T("game"), _T("width"), 0);
	g_nMapH = GetIniFileInfoInt(SETTINGS_PATH, _T("game"), _T("height"), 0);
	g_nLevelNum = GetIniFileInfoInt(SETTINGS_PATH, _T("game"), _T("level_num"), 0);
	g_nCurrentLevel = GetIniFileInfoInt(SETTINGS_PATH, _T("game"), _T("begin_level"), 1) - 1;

	g_bDebugMode = GetIniFileInfoInt(SETTINGS_PATH, _T("debug"), _T("enable_debug_mode"), 0);
	if (g_bDebugMode)
	{
		g_bOnlyVertexCollisionSound =
			GetIniFileInfoInt(SETTINGS_PATH, _T("debug"), _T("only_vertex_collision_sound"), 0);
		g_bBigScene =
			GetIniFileInfoInt(SETTINGS_PATH, _T("debug"), _T("enable_big_scene"), 0);
		g_nInnerSideLen_BigScene =
			GetIniFileInfoInt(SETTINGS_PATH, _T("debug"), _T("big_scene_brick_inner_side_len"), 0);
		g_nBallRadius_BigScene =
			GetIniFileInfoInt(SETTINGS_PATH, _T("debug"), _T("big_scene_ball_radius"), 0);
		
		// 简单模式
		if (GetIniFileInfoInt(SETTINGS_PATH, _T("debug"), _T("easy_mode"), 0))
		{
			g_nProbability_3X = 100;
		}
	}
	if (g_bBigScene)
	{
		g_nInnerSideLen = g_nInnerSideLen_BigScene;
		g_nBallRadius = g_nBallRadius_BigScene;
	}

	// 加载关卡
	// 分配每个关卡的内存
	g_pppszOriginalLevelMap = new char** [g_nLevelNum];

	for (int i = 0; i < g_nLevelNum; i++)
	{
		// 读取每个关卡文件
		FILE* fp;
		char pszPath[256] = {};
		sprintf_s(pszPath, "%s/level_%d.dat", LEVEL_FOLDER_PATH, i + 1);
		if (fopen_s(&fp, pszPath, "r"))
		{
			MessageBox(nullptr, _T("Error"), _T("加载关卡失败"), MB_OK);
			exit(-1);
		}

		// 分配每一行的内存
		g_pppszOriginalLevelMap[i] = new char* [g_nMapH];
		for (int y = 0; y < g_nMapH; y++)
		{
			// 分配每个字符的内存
			g_pppszOriginalLevelMap[i][y] = new char[g_nMapW + 1];
			memset(g_pppszOriginalLevelMap[i][y], 0, (g_nMapW + 1) * sizeof(char));

			// 读取一行
			fread_s(g_pppszOriginalLevelMap[i][y], g_nMapW + 1, g_nMapW, 1, fp);
			fseek(fp, 2, SEEK_CUR);	// 跳过换行符
		}

		fclose(fp);
	}

	// 加载音像资源

	mciSendString(_T("open ") MUSIC_PING_PATH _T(" alias ping"), NULL, 0, NULL);
	mciSendString(_T("open ") MUSIC_PONG_PATH _T(" alias pong"), NULL, 0, NULL);
	mciSendString(_T("open ") MUSIC_PROP_PATH _T(" alias prop"), NULL, 0, NULL);

	loadimage(&g_imgRestart, IMAGE_RESTART);
	loadimage(&g_img3X, IMAGE_3X_PATH);
	loadimage(&g_imgShoot3, IMAGE_SHOOT_3_PATH);
	loadimage(&g_imgHeart, IMAGE_HEART_PATH);
}

// 释放资源
void FreeResource()
{
	// 释放原始关卡地图内存
	for (int i = 0; i < g_nLevelNum; i++)
	{
		for (int y = 0; y < g_nMapH; y++)
		{
			delete[] g_pppszOriginalLevelMap[i][y];	// 释放当前行指向的字符
		}
		delete[] g_pppszOriginalLevelMap[i];		// 释放当前关卡的所有行
	}
	delete[] g_pppszOriginalLevelMap;				// 释放所有关卡
	g_pppszOriginalLevelMap = nullptr;

	// 停止播放并关闭音乐
	mciSendString(_T("stop ping"), NULL, 0, NULL);
	mciSendString(_T("stop pong"), NULL, 0, NULL);
	mciSendString(_T("stop prop"), NULL, 0, NULL);

	mciSendString(_T("close ping"), NULL, 0, NULL);
	mciSendString(_T("close pong"), NULL, 0, NULL);
	mciSendString(_T("close prop"), NULL, 0, NULL);
}

// 播放碰撞声
// prop 是否播放道具声
void PlayHitSound(bool prop)
{
	static bool flag = true;			// 乒乓碰撞声交替标记
	static clock_t record = 0;			// 上次操作的记录

	if (prop)
	{
		mciSendString(_T("play prop from 0"), NULL, 0, NULL);
	}
	else
	{
		// 避免过快操作，否则会一直从 0 开始播放，发不出声音
		if (clock() - record > 30)
		{
			if (flag)
				mciSendString(_T("play ping from 0"), NULL, 0, NULL);
			else
				mciSendString(_T("play pong from 0"), NULL, 0, NULL);
			record = clock();
			flag = !flag;
		}
	}
}

// 初始化一个关卡
void InitLevel()
{
	g_pstMap = new Brick[g_nMapW * g_nMapH];
	for (int x = 0; x < g_nMapW; x++)
	{
		for (int y = 0; y < g_nMapH; y++)
		{
			Brick* pBrick = &g_pstMap[x + y * g_nMapW];

			int sx = x * SIDE_LEN;
			int sy = y * SIDE_LEN;

			// 设置碰撞箱
			pBrick->rct = { sx, sy, sx + SIDE_LEN,sy + SIDE_LEN };

			switch (g_pppszOriginalLevelMap[g_nCurrentLevel][y][x])
			{
				// 墙
			case '#':
				pBrick->type = WALL;
				pBrick->color = COLOR_WALL;
				break;

				// 各种砖块
			case 'R':
				pBrick->type = BRICK;
				pBrick->color = COLOR_BRICK_R;
				break;
			case 'Y':
				pBrick->type = BRICK;
				pBrick->color = COLOR_BRICK_Y;
				break;
			case 'G':
				pBrick->type = BRICK;
				pBrick->color = COLOR_BRICK_G;
				break;
			case 'C':
				pBrick->type = BRICK;
				pBrick->color = COLOR_BRICK_C;
				break;
			case 'B':
				pBrick->type = BRICK;
				pBrick->color = COLOR_BRICK_B;
				break;
			case 'W':
				pBrick->type = BRICK;
				pBrick->color = COLOR_BRICK_W;
				break;

				// 其它的当空气
			default:
				pBrick->type = EMPTY;
				pBrick->color = BLACK;	// 颜色没啥用
				break;
			}
		}
	}

	g_nBoardCenterX = getwidth() / 2;

	g_nGeneratedBallNum = 0;
	g_bBallWait = true;

	g_nRemainingBallNum = BALL_LIFE_NUM;

	int pos_x = 15;
	int pos_y = 15;
	g_rctRestartBtn = {
		getwidth() - g_imgRestart.getwidth() - pos_x,
		pos_y,
		getwidth() - pos_x,
		pos_y + g_imgRestart.getheight()
	};

	g_nGeneratedPropNum = 0;
}

// 释放当前关卡内存
void FreeLevel()
{
	delete[] g_pstMap;
	g_pstMap = nullptr;
}

// 绘制道具
void DrawProp()
{
	for (int i = 0; i < g_nGeneratedPropNum; i++)
	{
		if (g_pstProps[i].used)
			continue;

		IMAGE* pimg = nullptr;
		switch (g_pstProps[i].type)
		{
		case PROP_3X:		pimg = &g_img3X;		break;
		case PROP_SHOOT_3:	pimg = &g_imgShoot3;	break;
		case PROP_HEART:	pimg = &g_imgHeart;		break;
		}

		if (pimg)
			putimage((int)g_pstProps[i].x, (int)g_pstProps[i].y, pimg);
	}
}

// 绘制地图
void DrawMap()
{
	for (int i = 0; i < g_nMapW * g_nMapH; i++)
	{
		if (g_pstMap[i].type != EMPTY)
		{
			RECT rct = g_pstMap[i].rct;

			setfillcolor(g_pstMap[i].color);
			solidrectangle(
				rct.left + g_nInnerInterval,
				rct.top + g_nInnerInterval,
				rct.right - g_nInnerInterval - 1,
				rct.bottom - g_nInnerInterval - 1
			);
		}
	}
}

// 绘制小球
void DrawBall()
{
	setfillcolor(WHITE);

	// 小球还没发射
	if (g_bBallWait)
	{
		solidcircle(g_nBoardCenterX, BOARD_Y - BOARD_BALL_DISTANCE, g_nBallRadius);
	}

	// 小球运动中
	else
	{
		for (int i = 0; i < g_nGeneratedBallNum; i++)
		{
			if (g_pstBalls[i].alive)
			{
				solidcircle((int)g_pstBalls[i].x, (int)g_pstBalls[i].y, g_nBallRadius);
			}
		}
	}
}

// 绘制挡板
void DrawBoard()
{
	setfillcolor(WHITE);
	solidrectangle(
		g_nBoardCenterX - BOARD_HALF_WIDTH,
		BOARD_Y - BOARD_HALF_THICKNESS,
		g_nBoardCenterX + BOARD_HALF_WIDTH,
		BOARD_Y + BOARD_HALF_THICKNESS
	);
}

// 绘制状态栏
void DrawStateBar()
{
	// 还没开始游戏
	if (g_bInMenu)
	{
		return;
	}

	int x = getwidth() / 2;

	setfillcolor(WHITE);
	solidcircle(x - 20, 28, 10);		// 画个球

	// 显示关卡信息
	TCHAR lpszLevel[64] = {};
	wsprintf(lpszLevel, _T("Level %d / %d"), g_nCurrentLevel + 1, g_nLevelNum);
	settextstyle(28, 0, _T("System"));
	outtextxy(10, 22, lpszLevel);

	// 显示一下还剩几个球
	TCHAR lpszBuf[12] = {};
	wsprintf(lpszBuf, _T("%02d"), g_nRemainingBallNum);
	settextstyle(24, 0, _T("System"));
	outtextxy(x, 22, lpszBuf);

	// “重新开始” 按钮
	putimage(g_rctRestartBtn.left, g_rctRestartBtn.top, &g_imgRestart);
}

// 在屏幕中央输出大字
void DrawTitle(LPCTSTR text)
{
	setfillcolor(BLACK);
	solidrectangle(0, 20, getwidth(), 140);

	settextstyle(48, 0, _T("System"));
	int w = textwidth(text);
	int h = textheight(text);
	outtextxy((getwidth() - w) / 2, 20 + (120 - h) / 2, text);
}

// 在屏幕中央输出小字
void DrawSmallText(LPCTSTR text)
{
	setfillcolor(BLACK);
	solidrectangle(0, 150, getwidth(), 200);

	settextstyle(24, 0, _T("System"));
	int w = textwidth(text);
	int h = textheight(text);
	outtextxy((getwidth() - w) / 2, 150 + (50 - h) / 2, text);
}

// 胜利界面
void Win()
{
	DrawTitle(_T("Clear"));
	if (g_nCurrentLevel == g_nLevelNum - 1)
		DrawSmallText(_T("Back to menu"));
	else
		DrawSmallText(_T("Next level"));
}

// 失败界面
void Fail()
{
	DrawSmallText(_T("Restart"));
}

// 绘制菜单
void DrawMenu()
{
	// 已开始游戏
	if (!g_bInMenu)
	{
		if (g_enuGameState == GameWin)
		{
			Win();
		}
		else if (g_enuGameState == GameFail)
		{
			Fail();
		}
		else
		{
			return;
		}

		FlushBatchDraw();

		// 等按下按键
		while (true)
		{
			ExMessage msg = getmessage(EX_MOUSE);
			if (msg.message == WM_LBUTTONUP
				&& (msg.y > 150 + STATE_BAR_HEIGHT && msg.y < 200 + STATE_BAR_HEIGHT))
			{
				break;
			}
		}

		FreeLevel();

		bool show_menu = false;	// 是否继续显示主菜单

		// 若是在胜利界面，则需要进入下一个关卡
		if (g_enuGameState == GameWin)
		{
			// 关卡打完了，回到主界面
			if (g_nCurrentLevel == g_nLevelNum - 1)
			{
				g_nCurrentLevel = 0;
				g_bInMenu = true;
				show_menu = true;
			}
			else
			{
				g_nCurrentLevel++;
			}
		}

		InitLevel();

		g_enuGameState = Gaming;

		if (!show_menu)
			return;
	}

	DrawTitle(_T("Bricks Beater"));

	// 致敬原游戏
	DrawSmallText(_T("Salute Many Bricks Breaker"));
	//DrawSmallText(_T("Salute                              "));
	//DrawSmallText(_T("Many Bricks Breaker"), true);

	settextstyle(20, 0, _T("System"));
	LPCTSTR lpszTip = _T("Click to start");
	int w = textwidth(lpszTip);
	outtextxy((getwidth() - w) / 2, BOARD_Y + BOARD_HALF_THICKNESS + 10, lpszTip);
}

// 获取点到矩形的最小距离
float GetDistance_PointToRect(float x, float y, RECT rct)
{
	float x_rct, y_rct;	// 保存矩形内到目标点最近的点
	if (x >= rct.left && x <= rct.right)
		x_rct = x;
	else
		x_rct = (float)(fabsf(x - rct.left) < fabsf(x - rct.right) ? rct.left : rct.right);
	if (y >= rct.top && y <= rct.bottom)
		y_rct = y;
	else
		y_rct = (float)(fabsf(y - rct.top) < fabsf(y - rct.bottom) ? rct.top : rct.bottom);

	float dx = x - x_rct;
	float dy = y - y_rct;

	return sqrtf(dx * dx + dy * dy);
}

// 根据圆的轨迹直线，获取圆与某点相切时的圆心坐标
// 返回是否存在相切
bool GetTangentCirclePoint(
	float x0,		// 切点坐标
	float y0,
	float x1,		// 圆心轨迹直线上的一点（更早运动到的点）
	float y1,
	float x2,		// 圆心轨迹直线上的另一点（其实运动不到的点）
	float y2,
	float r,		// 圆半径
	float* p_out_x,	// 输出圆心坐标
	float* p_out_y
)
{
	// 斜率不存在时
	if (fabsf(x1 - x2) < 0.00001f)
	{
		// 计算相切时圆心与切点的竖直距离
		float d2 = r * r - (x0 - x1) * (x0 - x1);
		if (d2 < 0)
			return false;
		float d = sqrtf(d2);

		// 求出两组解
		float _y1 = y0 + d;
		float _y2 = y0 - d;

		// 保留离 (x1, y1) 更近的解
		float _y_closer = fabsf(y1 - _y1) < fabsf(y1 - _y2) ? _y1 : _y2;

		*p_out_x = x1;
		*p_out_y = _y_closer;

		return true;
	}

	// 圆心轨迹直线方程：y - y1 = (y2 - y1) / (x2 - x1) * (x - x1)
	// 即：y = kx - kx1 + y1
	// 圆的方程：(x - x0) ^ 2 + (y - y0) ^ 2 = r ^ 2
	// 联立得二次函数，如下。

	float k = (y2 - y1) / (x2 - x1);			// 直线斜率
	float m = -k * x1 + y1 - y0;				// 部分常数
	float a = k * k + 1;						// 二次函数的 abc 系数
	float b = 2 * (k * m - x0);
	float c = x0 * x0 + m * m - r * r;
	float delta = b * b - 4 * a * c;			// 判别式
	if (delta < 0)								// 无解
		return false;
	float sqrt_delta = sqrtf(delta);			// 判别式开根号
	float _x1 = (-b + sqrt_delta) / (2 * a);	// 两个根
	float _x2 = (-b - sqrt_delta) / (2 * a);

	// 保留离 (x1, y1) 更近的解
	float _x_closer = fabsf(x1 - _x1) < fabsf(x1 - _x2) ? _x1 : _x2;
	float _y = k * _x_closer - k * x1 + y1;

	*p_out_x = _x_closer;
	*p_out_y = _y;

	return true;
}

// 单个碰撞箱和单个小球的碰撞处理
// ball						小球
// rct						碰撞箱区域
// is_board					是否为挡板碰撞判定
// left, up, right, down	该碰撞箱四周分别是否有活动空间
//
// 返回值：参见 HIT_ 系列宏
int SingleRect_BallHit(Ball* ball, RECT rct, bool is_board, bool left = true, bool up = true, bool right = true, bool down = true)
{
	// 得到小球上一帧的坐标
	float last_x = (ball->x - ball->vx);
	float last_y = (ball->y - ball->vy);

	// 如果小球不慎进入砖块内部，此时应该回到上一帧，并使速度反向（同时加以扰动）
	if (ball->x >= rct.left && ball->x <= rct.right
		&& ball->y >= rct.top && ball->y <= rct.bottom)
	{
		ball->x = last_x;
		ball->y = last_y;

		// 如果上一帧仍然陷在里面
		if (ball->x >= rct.left && ball->x <= rct.right
			&& ball->y >= rct.top && ball->y <= rct.bottom)
		{
			// 强制弹出小球
			if (ball->vx > 0 && left)
			{
				ball->x = (float)(rct.left - g_nBallRadius);
			}
			else if (ball->vx < 0 && right)
			{
				ball->x = (float)(rct.right + g_nBallRadius);
			}
			if (ball->vy > 0 && up)
			{
				ball->y = (float)(rct.top - g_nBallRadius);
			}
			else if (ball->vy < 0 && down)
			{
				ball->y = (float)(rct.bottom + g_nBallRadius);
			}
		}

		ball->vx *= -1 + rand() % 10 / 1000.f;
		ball->vy *= -1 + rand() % 10 / 1000.f;

		// 对于挡板问题的特殊处理：直接跳出来
		if (is_board)
		{
			ball->y = (float)(BOARD_Y - BOARD_HALF_THICKNESS - g_nBallRadius - 1);
		}

		return HIT_NORMAL;
	}

	// 首先需要保证矩形和圆有重叠
	if (!(GetDistance_PointToRect(ball->x, ball->y, rct) <= g_nBallRadius * 0.98f))
	{
		return HIT_NONE;
	}

	int return_flag = HIT_NONE;

	// 穿越碰撞箱边界标记
	bool cross_left =
		rct.left > ball->x
		&& fabsf(ball->x - rct.left) <= g_nBallRadius
		&& left;
	bool cross_right =
		ball->x > rct.right
		&& fabsf(ball->x - rct.right) <= g_nBallRadius
		&& right;
	bool cross_top =
		rct.top > ball->y
		&& fabsf(ball->y - rct.top) <= g_nBallRadius
		&& up;
	bool cross_bottom =
		ball->y > rct.bottom
		&& fabsf(ball->y - rct.bottom) <= g_nBallRadius
		&& down;

	// 标记是否需要判断顶点碰撞
	bool vertex_judge_flag = true;
	float fVertex_X = 0;	// 判定顶点碰撞时使用的顶点
	float fVertex_Y = 0;
	if (cross_left && cross_top)			// 左上角
	{
		//vertex_judge_flag = true;
		fVertex_X = (float)rct.left;
		fVertex_Y = (float)rct.top;
	}
	else if (cross_right && cross_top)		// 右上角
	{
		//vertex_judge_flag = true;
		fVertex_X = (float)rct.right;
		fVertex_Y = (float)rct.top;
	}
	else if (cross_left && cross_bottom)	// 左下角
	{
		//vertex_judge_flag = true;
		fVertex_X = (float)rct.left;
		fVertex_Y = (float)rct.bottom;
	}
	else if (cross_right && cross_bottom)	// 右下角
	{
		//vertex_judge_flag = true;
		fVertex_X = (float)rct.right;
		fVertex_Y = (float)rct.bottom;
	}

	// 如果没有同时穿越 xy 两个方向，就需要再评估用哪个顶点
	else
	{
		// 如果穿越上下边界，则就只需要决定使用左边还是右边的顶点
		if (cross_top || cross_bottom)
		{
			fVertex_Y = cross_top ? (float)rct.top : (float)rct.bottom;	// 首先就可以确定 Y

			// 若左右都有空地，则在左右顶点中选距离近的
			if (left && right)
			{
				fVertex_X =
					(fabsf(ball->x - rct.left) < fabsf(ball->x - rct.right)) ?
					(float)rct.left : (float)rct.right;
			}

			// 否则哪边空选哪边
			else if (left)		fVertex_X = (float)rct.left;
			else if (right)		fVertex_X = (float)rct.right;

			// 要是两边都没空地，就不可能发生顶点碰撞
			else
			{
				vertex_judge_flag = false;
			}
		}

		// 如果穿越左右边界
		else if (cross_left || cross_right)
		{
			fVertex_X = cross_left ? (float)rct.left : (float)rct.right;

			if (up && down)
			{
				fVertex_Y =
					(fabsf(ball->y - rct.top) < fabsf(ball->y - rct.bottom)) ?
					(float)rct.top : (float)rct.bottom;
			}
			else if (up)		fVertex_Y = (float)rct.top;
			else if (down)		fVertex_Y = (float)rct.bottom;
			else
			{
				vertex_judge_flag = false;
			}
		}
		else
		{
			vertex_judge_flag = false;
		}
	}

	// 优先判断是不是顶点碰撞
	bool isVertexCollision = false;	// 标记是否发生顶点碰撞
	if (vertex_judge_flag)			// 处理顶点碰撞问题
	{
		// 获取碰撞时的小球圆心坐标（即与顶点相切时的坐标）
		float fCollisionX, fCollisionY;
		if (!GetTangentCirclePoint(
			fVertex_X,
			fVertex_Y,
			last_x,
			last_y,
			ball->x,
			ball->y,
			(float)g_nBallRadius,
			&fCollisionX,
			&fCollisionY
		))
		{
			// 没有相切，说明顶点碰撞不成立
			goto tag_after_vertex_colision;
		}

		// 如果是真的相切，则相切时矩形到圆心的最近距离应该等于小球半径
		// 但如果此时小于半径，那么说明是假相切
		if (GetDistance_PointToRect(fCollisionX, fCollisionY, rct) < g_nBallRadius * 0.98f /* 允许一点误差 */)
		{
			goto tag_after_vertex_colision;
		}

		// 计算碰撞时，小球圆心到碰撞点的坐标差
		float f_dx = fCollisionX - fVertex_X;
		float f_dy = fCollisionY - fVertex_Y;

		// 求反射面弧度
		float f_radianReflectingSurface = atan2f(f_dy, f_dx);

		// 求法线弧度
		float f_radianNormal = f_radianReflectingSurface + PI / 2 /* 或 - PI / 2 */;

		// 求小球入射弧度
		float f_radianIncidence = atan2f(ball->vy, ball->vx);

		// 将小球速度沿法线对称，求得新的速度弧度
		float f_radianReflection = 2 * f_radianNormal - f_radianIncidence;

		// 求速度
		ball->vx = cosf(f_radianReflection) * BALL_SPEED;
		ball->vy = sinf(f_radianReflection) * BALL_SPEED;

		// 修正小球坐标到相切时的坐标
		ball->x = fCollisionX;
		ball->y = fCollisionY;

		isVertexCollision = true;	// 标记发生顶点碰撞
	}

tag_after_vertex_colision:

	// 已完成顶点碰撞
	if (isVertexCollision)
	{
		return_flag = HIT_VERTEX;
	}

	// 基础碰撞
	else
	{
		// 跨越碰撞箱左右边界，则水平速度反转
		if (cross_left || cross_right)
		{
			ball->vx = -ball->vx;

			return_flag = HIT_NORMAL;
		}
		// 跨越碰撞箱上下边界，则竖直速度反转
		if (cross_top || cross_bottom)
		{
			// 与挡板碰撞时，需要根据小球和挡板的碰撞位置改变其 vx
			if (is_board)
			{
				ball->vx = (float)(ball->x - g_nBoardCenterX) / BOARD_HALF_WIDTH * BALL_SPEED;
				if (fabsf(ball->vx) > BALL_SPEED)
				{
					ball->vx = ball->vx > 0 ? BALL_SPEED : -BALL_SPEED;
					ball->vy = 0;
				}
				else
				{
					ball->vy = (ball->vy < 0 ? 1 : -1) * sqrtf(BALL_SPEED * BALL_SPEED - ball->vx * ball->vx);
				}
			}
			else
			{
				ball->vy = -ball->vy;
			}

			return_flag = HIT_NORMAL;
		}
	}

	// 播放碰撞声

	// 非调试模式下，只要有碰撞就发出声音
	if (!g_bOnlyVertexCollisionSound
		&& return_flag != HIT_NONE)
	{
		PlayHitSound(false);
	}

	// 调试模式下，只有顶点碰撞发出声音
	else if (return_flag == HIT_VERTEX)
	{
		PlayHitSound(false);
	}

	// 回溯坐标，即发生碰撞后，把小球从墙里“拔”出来（回到上一帧的位置），避免穿墙效果
	// 如果发生的是顶点碰撞，那么在前面就已经进行了相切位置修正，就不需要回溯坐标了
	if (!isVertexCollision && return_flag != HIT_NONE)
	{
		ball->x = last_x;
		ball->y = last_y;
	}

	return return_flag;

}// SingleRect_BallHit

// 消除砖块后，调用此函数幸运创建道具
// x, y 被消除砖块的位置
void GeneratePropByLuck(float x, float y)
{
	// 道具已满
	if (g_nGeneratedPropNum == PROP_MAX_NUM)
	{
		return;
	}

	// 道具表
	int pnProps[3][2] = {
		{PROP_3X,g_nProbability_3X},
		{PROP_SHOOT_3,g_nProbability_Shoot_3},
		{PROP_HEART,g_nProbability_Heart},
	};

	// 随机选取一个道具
	int choice = rand() % 3;

	// 根据概率决定是否生成这个道具
	if (rand() % 100 < pnProps[choice][1])
	{
		// 创建道具
		Prop prop;
		prop.type = pnProps[choice][0];
		prop.used = false;
		prop.x = x;
		prop.y = y;

		g_pstProps[g_nGeneratedPropNum] = prop;
		g_nGeneratedPropNum++;
	}
}

// 某索引位置是否为空白区域（可活动区域）
// 可以传入 -1，此时返回 true
bool IsEmptyBlock(int id)
{
	return id < 0 || g_pstMap[id].type == EMPTY;
}

// 对单个小球的碰撞处理（使小球反弹，删除被碰撞砖块）
// 以及判定小球出界
void SingleBallHit(Ball* ball)
{
	// 边界碰撞判定
	int border_thickness = 100;		// 边界已设置厚度，防止飞出
	RECT rctLeft = { -border_thickness,0,1,getheight() };
	RECT rctRight = { g_nMapW * SIDE_LEN - 1,0,g_nMapW * SIDE_LEN + border_thickness,getheight() };
	RECT rctTop = { -border_thickness,-border_thickness,g_nMapW * SIDE_LEN + border_thickness,1 };
	SingleRect_BallHit(ball, rctLeft, false);
	SingleRect_BallHit(ball, rctRight, false);
	SingleRect_BallHit(ball, rctTop, false);

	// 出界判定
	if (ball->y > getheight() - STATE_BAR_HEIGHT)
	{
		ball->alive = false;	// 记为死球
		return;
	}

	// 估计小球对应的大概砖块位置
	int brick_x = (int)(ball->x / SIDE_LEN);
	int brick_y = (int)(ball->y / SIDE_LEN);

	// 需要遍历的砖块
	int pnOffset[9][2] = {
		{-1,-1}, {0,-1}, {1,-1},
		{-1, 0}, {0, 0}, {1, 0},
		{-1, 1}, {0, 1}, {1, 1},
	};

	// 遍历砖块
	for (int i = 0; i < 9; i++)
	{
		// 当前方块
		int current_x = brick_x + pnOffset[i][0];
		int current_y = brick_y + pnOffset[i][1];

		// 出界
		if (current_x < 0 || current_x >= g_nMapW
			|| current_y < 0 || current_y >= g_nMapH)
			continue;

		// 自己和四周方块的索引
		int current = current_x + current_y * g_nMapW;
		int left = -1;
		int right = -1;
		int up = -1;
		int down = -1;
		if (current_x > 0)				left = current_x - 1 + current_y * g_nMapW;
		if (current_x < g_nMapW - 1)	right = current_x + 1 + current_y * g_nMapW;
		if (current_y > 0)				up = current_x + (current_y - 1) * g_nMapW;
		if (current_y < g_nMapH - 1)	down = current_x + (current_y + 1) * g_nMapW;

		// 对实体方块进行碰撞判定
		if (g_pstMap[current].type != EMPTY)
		{
			RECT rct = g_pstMap[current].rct;		// 当前方块碰撞箱

			int hit_flag = SingleRect_BallHit(
				ball,
				rct,
				false,
				IsEmptyBlock(left),
				IsEmptyBlock(up),
				IsEmptyBlock(right),
				IsEmptyBlock(down)
			);

			// 没打到砖块
			if (hit_flag == HIT_NONE)
				continue;

			// 发生有效碰撞时，清除砖块
			else if (g_pstMap[current].type == BRICK)
			{
				g_pstMap[current].type = EMPTY;

				// 随机生成道具
				GeneratePropByLuck((float)rct.left, (float)rct.top);
			}

			break;
		}
	}

	// 挡板碰撞判定
	RECT rctBoard = {
		g_nBoardCenterX - BOARD_HALF_WIDTH,
		BOARD_Y - BOARD_HALF_THICKNESS,
		g_nBoardCenterX + BOARD_HALF_WIDTH,
		BOARD_Y + BOARD_HALF_THICKNESS
	};
	SingleRect_BallHit(ball, rctBoard, true);
}

// 所有小球的处理
// 
// 功能：
// + 维持小球运动
// + 小球碰撞处理
// + 判定小球出界死亡
// + 检查小球是不是都死了，以便上球
// + 判定球空人亡（失败）
void AllBallProcess()
{
	// 等球中
	if (g_bBallWait)
		return;

	// 记录还有没有活着的球
	bool isAnyBall = false;

	for (int i = 0; i < g_nGeneratedBallNum; i++)
	{
		// 处理活球的运动
		if (g_pstBalls[i].alive)
		{
			isAnyBall = true;

			// 单个小球碰撞处理
			SingleBallHit(&g_pstBalls[i]);

			// 小球运动处理
			g_pstBalls[i].x += g_pstBalls[i].vx + rand() % 10 / 100.f /* 添加随机扰动，防止在某区域反弹死循环 */;
			g_pstBalls[i].y += g_pstBalls[i].vy + rand() % 10 / 100.f;

			// 防止出边界
			if (g_pstBalls[i].x < g_nBallRadius)
				g_pstBalls[i].x = g_nBallRadius;
			else if (g_pstBalls[i].x > getwidth() - g_nBallRadius)
				g_pstBalls[i].x = getwidth() - g_nBallRadius;
			if (g_pstBalls[i].y < g_nBallRadius)	// y 方向只需处理上边界
				g_pstBalls[i].y = g_nBallRadius;
		}
	}

	// 没球了，标记等待发射球
	if (!isAnyBall)
	{
		// 还有球上
		if (g_nRemainingBallNum)
		{
			g_bBallWait = true;
		}

		// 仓库也没球了，输了
		else
		{
			g_enuGameState = GameFail;
		}
	}
}

// 判断是否已经胜利
void CheckSuccess()
{
	bool isAnyBrick = false;
	for (int i = 0; i < g_nMapW * g_nMapH; i++)
	{
		if (g_pstMap[i].type == BRICK)
		{
			isAnyBrick = true;
			break;
		}
	}

	// 没砖块了，赢了
	if (!isAnyBrick)
	{
		g_enuGameState = GameWin;
	}
}

// 获取随机 Vx 和 Vy
void RandSpeedXY(float* vx, float* vy)
{
	*vx = fmodf((float)rand(), BALL_SPEED - rand() % 10 / 10000.f /* 增加扰动 */) * (rand() % 2 ? 1 : -1);
	*vy = sqrtf(BALL_SPEED * BALL_SPEED - *vx * *vx) * (rand() % 2 ? 1 : -1);
}

// 生成新的球
// is_board_launch	标记在球死了的情况下，从挡板发射一个新的球（若是，则无需填写初始位置）
// up				强制初始速度向上
void GenerateBall(bool is_board_launch, float x = 0, float y = 0, bool up = false)
{
	// 不能再创建球了
	if (g_nGeneratedBallNum == BALL_MAX_NUM)
	{
		return;
	}

	Ball new_ball;
	new_ball.alive = true;

	if (is_board_launch)
	{
		new_ball.x = (float)g_nBoardCenterX;
		new_ball.y = (float)(BOARD_Y - BOARD_BALL_DISTANCE);
		new_ball.vx = 0;
		new_ball.vy = -BALL_SPEED;
	}
	else
	{
		new_ball.x = x;
		new_ball.y = y;
		RandSpeedXY(&new_ball.vx, &new_ball.vy);

		if (up && new_ball.vy > 0)
			new_ball.vy = -new_ball.vy;
	}

	// 将此球加入游戏中
	g_pstBalls[g_nGeneratedBallNum] = new_ball;
	g_nGeneratedBallNum++;
}

// 发射小球
void LaunchBall()
{
	// 必须在等待小球时调用此函数
	if (!g_bBallWait)
	{
		return;
	}

	// 仓库还有球
	if (g_nRemainingBallNum)
	{
		GenerateBall(true);
		g_nRemainingBallNum--;
		g_bBallWait = false;
	}
}

// 应用一个道具
void ApplyProp(Prop prop)
{
	switch (prop.type)
	{
	case PROP_3X:
	{
		// 记录旧球数量，防止无限增殖
		int old_num = g_nGeneratedBallNum;

		// 每个球都发出三个新球
		for (int i = 0; i < old_num; i++)
		{
			if (!g_pstBalls[i].alive)
				continue;
			for (int j = 0; j < 3; j++)
				GenerateBall(false, g_pstBalls[i].x, g_pstBalls[i].y);
		}
		break;
	}

	case PROP_SHOOT_3:
		// 从道具位置发射出三个球
		for (int i = 0; i < 3; i++)
			GenerateBall(false, prop.x, (float)(BOARD_Y - BOARD_Y_DISTACE), true);
		break;

	case PROP_HEART:
		// 加一条命
		g_nRemainingBallNum++;
		break;
	}
}

// 道具单点碰撞判定
bool SinglePointPropHit(int index, float x, float y)
{
	return (y >= BOARD_Y - BOARD_HALF_THICKNESS
		&& y <= BOARD_Y + BOARD_HALF_THICKNESS
		&& x >= g_nBoardCenterX - BOARD_HALF_WIDTH
		&& x <= g_nBoardCenterX + BOARD_HALF_WIDTH);
}

// 处理道具相关
// + 道具下落
// + 道具和挡板碰撞判定
// + 道具生效
void PropProcess()
{
	for (int i = 0; i < g_nGeneratedPropNum; i++)
	{
		if (g_pstProps[i].used)
			continue;

		float y1 = g_pstProps[i].y;
		float y2 = g_pstProps[i].y + g_img3X.getheight();
		float x1 = g_pstProps[i].x;
		float x2 = g_pstProps[i].x + g_img3X.getwidth();

		// 发生碰撞（通过道具的三个点检测碰撞）
		if (SinglePointPropHit(i, x1, y1)
			|| SinglePointPropHit(i, x2, y2)
			|| SinglePointPropHit(i, (x1 + x2) / 2, (y1 + y2) / 2))
		{
			// 应用这个道具
			ApplyProp(g_pstProps[i]);
			g_pstProps[i].used = true;

			// 音效
			PlayHitSound(true);

			continue;
		}

		// 保持下落
		g_pstProps[i].y += PROP_DROP_SPEED;
	}
}

// 用户输入处理
void UserInputProcess()
{
	// 在主菜单
	if (g_bInMenu)
	{
		// 等鼠标按下
		while (!getmessage(EX_MOUSE).lbutton);

		// 开始游戏
		LaunchBall();
		g_bInMenu = false;
	}

	// 在游戏中

	ExMessage msg;
	while (peekmessage(&msg, EX_MOUSE))
	{
		// 等发球
		if (g_bBallWait)
		{
			// 等鼠标按下
			if (msg.lbutton)
			{
				LaunchBall();
			}
		}
		else
		{
			// 挡板跟随鼠标
			g_nBoardCenterX = msg.x;
		}

		// 点击“重新开始”按钮判定
		if (msg.message == WM_LBUTTONUP
			&& msg.x >= g_rctRestartBtn.left && msg.x <= g_rctRestartBtn.right
			&& msg.y >= g_rctRestartBtn.top && msg.y <= g_rctRestartBtn.bottom)
		{
			FreeLevel();
			InitLevel();
		}
	}
}

// 游戏过程函数
void GameProcess()
{
	AllBallProcess();		// 处理小球
	PropProcess();		// 处理道具
	CheckSuccess();		// 胜利判定
}

// 延时以稳定帧率
void DelayFPS()
{
	static clock_t cRecord = clock();

	clock_t consumption = clock() - cRecord;
	clock_t sleep = 30 - consumption;
	if (sleep > 0)
		Sleep(sleep);
	cRecord = clock();
}

int main()
{
	InitResource();

	initgraph(
		g_nMapW * SIDE_LEN,	// 地图宽
		STATE_BAR_HEIGHT + BOARD_Y + BOARD_HALF_THICKNESS + WINDOW_BOTTOM_MARGIN /* 留白 */
	);

	InitLevel();
	srand((UINT)time(nullptr));

	setbkcolor(RGB(19, 19, 92));
	setbkmode(TRANSPARENT);
	BeginBatchDraw();

	while (true)
	{
		for (int i = 0; i < PROCESS_REPEAT; i++)
		{
			GameProcess();
		}

		cleardevice();

		DrawStateBar();

		setorigin(0, STATE_BAR_HEIGHT);
		{
			DrawProp();
			DrawMap();
			DrawBoard();
			DrawBall();
			DrawMenu();
		}
		setorigin(0, 0);

		FlushBatchDraw();

		UserInputProcess();

		DelayFPS();
	}

	EndBatchDraw();

	FreeResource();

	closegraph();
	return 0;
}


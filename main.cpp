#include <math.h>
#include <time.h>
#include <graphics.h>
#include "ini.hpp"

// ���� Windows Multimedia API
#pragma comment(lib, "Winmm.lib")

/////////////////// �������� ///////////////////

// �����������
#define STATE_BAR_HEIGHT		150		// ״̬���߶�
#define WINDOW_BOTTOM_MARGIN	187		// ���ڵײ��߾ࣨ���ף�

// �����С����
#define INNER_SIDE_LEN		7	// �����������α߳�
#define INNER_INTERVAL		1	// �����������ձ߾�
#define SIDE_LEN			(INNER_SIDE_LEN + 2 * INNER_INTERVAL)	// �����ܱ߳�

// ��������
#define BOARD_HALF_WIDTH		40		// ������
#define BOARD_HALF_THICKNESS	3		// �������
#define BOARD_Y_DISTACE			100		// ����ȵ�ͼ�ײ��Ͷ���
#define BOARD_Y					(g_nMapH * SIDE_LEN + BOARD_Y_DISTACE)	// ���� y ����
#define BOARD_BALL_DISTANCE		16		// ������С���뵲���Զ

// С������
#define BALL_RADIUS			3		// С��뾶
#define BALL_MAX_NUM		1024	// ���С������
#define BALL_LIFE_NUM		4		// ���м�����
#define BALL_SPEED			3.6f	// С���ٶ�

// �㷨����
#define PROCESS_REPEAT		2		// ��һ֡�ڼ��㼸����Ϸ״̬

// ������ɫ
#define COLOR_WALL			RGB(163, 163, 162)	// ǽ����ɫ
#define COLOR_BRICK_R		RGB(214, 63, 37)	// �� R
#define COLOR_BRICK_Y		RGB(228, 174, 31)	// �� Y
#define COLOR_BRICK_G		RGB(15, 216, 15)	// �� G
#define COLOR_BRICK_C		RGB(39, 253, 251)	// �� C
#define COLOR_BRICK_B		RGB(68, 112, 202)	// �� B
#define COLOR_BRICK_W		RGB(234, 234, 234)	// �� W

// �������ͱ��
#define EMPTY				0
#define WALL				1
#define BRICK				2

// �������ͱ��
#define PROP_3X				0	// ÿ��С�����ⷢ������С��
#define PROP_SHOOT_3		1	// ��������С��
#define PROP_HEART			2	// ��һ����

#define PROP_MAX_NUM		1024	// ����������

// ���߳��ָ��ʣ��ٷֱȣ�
#define PROBABILITY_3X			8
#define PROBABILITY_SHOOT_3		12
#define PROBABILITY_HEART		5

// ���������ٶ�
#define PROP_DROP_SPEED			0.3f

// ��ײ���
#define HIT_NONE			0	// δ�����κ���ײ
#define HIT_NORMAL			1	// ����һ����ͨ��ײ�������ˮƽ�߻���ֱ�ߵ���ײ��
#define HIT_ABNORMAL		2	// �쳣����ͨ��ײ��С����������ש��Ⱥ�ڲ�����ײ��
#define HIT_VERTEX			3	// ����ζ��㷢������ײ

// ��Ϸ״̬
#define GAME_WIN			1		// Ӯ
#define GAME_LOSE			2		// ��

// ��Դ·��
#define SETTINGS_PATH		_T("./BricksBeater.ini")	// �����ļ�
#define LEVEL_FOLDER_PATH	("./res/level")				// �ؿ��ļ��洢·�����ļ��У�

#define MUSIC_PING_PATH		_T("./res/music/ping.mp3")	// ��ײ�� 1
#define MUSIC_PONG_PATH		_T("./res/music/pong.mp3")	// ��ײ�� 2
#define MUSIC_PROP_PATH		_T("./res/music/prop.mp3")	// ����ߵ�����

#define IMAGE_RESTART		_T("./res/pic/restart.gif")
#define IMAGE_3X_PATH		_T("./res/pic/prop_3x.gif")
#define IMAGE_SHOOT_3_PATH	_T("./res/pic/prop_shoot_3.gif")
#define IMAGE_HEART_PATH	_T("./res/pic/prop_heart.gif")

/////////////////// ȫ�ֱ��� ///////////////////

int g_nMapW;				// ��ͼ����λ�����飩
int g_nMapH;				// ��ͼ�ߣ���λ�����飩
int g_nLevelNum;			// �ؿ���
int g_nCurrentLevel = 0;	// ��ǰ�ǵڼ��أ��� 0 ��ʼ��
char*** g_pppszOriginalLevelMap;			// ���ؿ�ԭʼ��ͼ

bool g_bInMenu = true;	// �Ƿ������˵�

// ��Ϸ״̬
enum GameState { Gaming, GameWin, GameFail };
GameState g_enuGameState = Gaming;

// ש��
struct Brick
{
	int type;		// �������ͣ�ש�鱻���֮��ᱻ��Ϊ EMPTY��
	COLORREF color;	// ������ɫ
	RECT rct;		// ש����ײ��λ��
}*g_pstMap;

int g_nBoardCenterX;	// �������� x ����

// С��
struct Ball
{
	bool alive;		// ��С���Ƿ��Ѿ�������
	float x;
	float y;
	float vx;
	float vy;
}g_pstBalls[BALL_MAX_NUM];

int g_nGeneratedBallNum;		// �Ѿ����ɳ���С��������С����粻������
bool g_bBallWait;				// �Ƿ����ڵȴ�����С��

int g_nRemainingBallNum;	// ��ʣ������

// ͼ����Դ
IMAGE g_imgRestart;
IMAGE g_img3X;
IMAGE g_imgShoot3;
IMAGE g_imgHeart;

// �����¿�ʼ�� ��ť����
RECT g_rctRestartBtn;

// ����
struct Prop
{
	int type;		// ����
	bool used;		// �Ƿ��ѱ�ʹ��
	float x;
	float y;
}g_pstProps[PROP_MAX_NUM];

int g_nGeneratedPropNum;		// �Ѿ����ɵĵ�������

/////////////////// �������� ///////////////////

// ������Ϸ��Դ
void InitResource()
{
	// ���عؿ�

	g_nMapW = GetIniFileInfoInt(SETTINGS_PATH, _T("game"), _T("width"), 0);
	g_nMapH = GetIniFileInfoInt(SETTINGS_PATH, _T("game"), _T("height"), 0);
	g_nLevelNum = GetIniFileInfoInt(SETTINGS_PATH, _T("game"), _T("level_num"), 0);

	// ����ÿ���ؿ����ڴ�
	g_pppszOriginalLevelMap = new char** [g_nLevelNum];

	for (int i = 0; i < g_nLevelNum; i++)
	{
		// ��ȡÿ���ؿ��ļ�
		FILE* fp;
		char pszPath[256] = {};
		sprintf_s(pszPath, "%s/level_%d.dat", LEVEL_FOLDER_PATH, i + 1);
		if (fopen_s(&fp, pszPath, "r"))
		{
			MessageBox(nullptr, _T("Error"), _T("���عؿ�ʧ��"), MB_OK);
			exit(-1);
		}

		// ����ÿһ�е��ڴ�
		g_pppszOriginalLevelMap[i] = new char* [g_nMapH];
		for (int y = 0; y < g_nMapH; y++)
		{
			// ����ÿ���ַ����ڴ�
			g_pppszOriginalLevelMap[i][y] = new char[g_nMapW + 1];
			memset(g_pppszOriginalLevelMap[i][y], 0, (g_nMapW + 1) * sizeof(char));

			// ��ȡһ��
			fread_s(g_pppszOriginalLevelMap[i][y], g_nMapW + 1, g_nMapW, 1, fp);
			fseek(fp, 2, SEEK_CUR);	// �������з�
		}

		fclose(fp);
	}

	// ����������Դ

	mciSendString(_T("open ") MUSIC_PING_PATH _T(" alias ping"), NULL, 0, NULL);
	mciSendString(_T("open ") MUSIC_PONG_PATH _T(" alias pong"), NULL, 0, NULL);
	mciSendString(_T("open ") MUSIC_PROP_PATH _T(" alias prop"), NULL, 0, NULL);

	loadimage(&g_imgRestart, IMAGE_RESTART);
	loadimage(&g_img3X, IMAGE_3X_PATH);
	loadimage(&g_imgShoot3, IMAGE_SHOOT_3_PATH);
	loadimage(&g_imgHeart, IMAGE_HEART_PATH);
}

// �ͷ���Դ
void FreeResource()
{
	// �ͷ�ԭʼ�ؿ���ͼ�ڴ�
	for (int i = 0; i < g_nLevelNum; i++)
	{
		for (int y = 0; y < g_nMapH; y++)
		{
			delete[] g_pppszOriginalLevelMap[i][y];	// �ͷŵ�ǰ��ָ����ַ�
		}
		delete[] g_pppszOriginalLevelMap[i];		// �ͷŵ�ǰ�ؿ���������
	}
	delete[] g_pppszOriginalLevelMap;				// �ͷ����йؿ�
	g_pppszOriginalLevelMap = nullptr;

	// ֹͣ���Ų��ر�����
	mciSendString(_T("stop ping"), NULL, 0, NULL);
	mciSendString(_T("stop pong"), NULL, 0, NULL);
	mciSendString(_T("stop prop"), NULL, 0, NULL);

	mciSendString(_T("close ping"), NULL, 0, NULL);
	mciSendString(_T("close pong"), NULL, 0, NULL);
	mciSendString(_T("close prop"), NULL, 0, NULL);
}

// ������ײ��
// prop �Ƿ񲥷ŵ�����
void PlayHitSound(bool prop)
{
	static bool flag = true;			// ƹ����ײ��������
	static clock_t record = 0;			// �ϴβ����ļ�¼

	if (prop)
	{
		mciSendString(_T("play prop from 0"), NULL, 0, NULL);
	}
	else
	{
		// �����������������һֱ�� 0 ��ʼ���ţ�����������
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

// ��ʼ��һ���ؿ�
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

			// ������ײ��
			pBrick->rct = { sx, sy, sx + SIDE_LEN,sy + SIDE_LEN };

			switch (g_pppszOriginalLevelMap[g_nCurrentLevel][y][x])
			{
				// ǽ
			case '#':
				pBrick->type = WALL;
				pBrick->color = COLOR_WALL;
				break;

				// ����ש��
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

				// �����ĵ�����
			default:
				pBrick->type = EMPTY;
				pBrick->color = BLACK;	// ��ɫûɶ��
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

// �ͷŵ�ǰ�ؿ��ڴ�
void FreeLevel()
{
	delete[] g_pstMap;
	g_pstMap = nullptr;
}

// ���Ƶ���
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

// ���Ƶ�ͼ
void DrawMap()
{
	for (int i = 0; i < g_nMapW * g_nMapH; i++)
	{
		if (g_pstMap[i].type != EMPTY)
		{
			RECT rct = g_pstMap[i].rct;

			setfillcolor(g_pstMap[i].color);
			solidrectangle(
				rct.left + INNER_INTERVAL,
				rct.top + INNER_INTERVAL,
				rct.right - INNER_INTERVAL - 1,
				rct.bottom - INNER_INTERVAL - 1
			);
		}
	}
}

// ����С��
void DrawBall()
{
	setfillcolor(WHITE);

	// С��û����
	if (g_bBallWait)
	{
		solidcircle(g_nBoardCenterX, BOARD_Y - BOARD_BALL_DISTANCE, BALL_RADIUS);
	}

	// С���˶���
	else
	{
		for (int i = 0; i < g_nGeneratedBallNum; i++)
		{
			if (g_pstBalls[i].alive)
			{
				solidcircle((int)g_pstBalls[i].x, (int)g_pstBalls[i].y, BALL_RADIUS);
			}
		}
	}
}

// ���Ƶ���
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

// ����״̬��
void DrawStateBar()
{
	// ��û��ʼ��Ϸ
	if (g_bInMenu)
	{
		return;
	}

	int x = getwidth() / 2;

	setfillcolor(WHITE);
	solidcircle(x - 20, 28, 10);		// ������

	// ��ʾ�ؿ���Ϣ
	TCHAR lpszLevel[64] = {};
	wsprintf(lpszLevel, _T("Level %d / %d"), g_nCurrentLevel + 1, g_nLevelNum);
	settextstyle(28, 0, _T("System"));
	outtextxy(10, 22, lpszLevel);

	// ��ʾһ�»�ʣ������
	TCHAR lpszBuf[12] = {};
	wsprintf(lpszBuf, _T("%02d"), g_nRemainingBallNum);
	settextstyle(24, 0, _T("System"));
	outtextxy(x, 22, lpszBuf);

	// �����¿�ʼ�� ��ť
	putimage(g_rctRestartBtn.left, g_rctRestartBtn.top, &g_imgRestart);
}

// ����Ļ�����������
void DrawTitle(LPCTSTR text)
{
	setfillcolor(BLACK);
	solidrectangle(0, 20, getwidth(), 140);

	settextstyle(48, 0, _T("System"));
	int w = textwidth(text);
	int h = textheight(text);
	outtextxy((getwidth() - w) / 2, 20 + (120 - h) / 2, text);
}

// ����Ļ�������С��
void DrawSmallText(LPCTSTR text)
{
	setfillcolor(BLACK);
	solidrectangle(0, 150, getwidth(), 200);

	settextstyle(24, 0, _T("System"));
	int w = textwidth(text);
	int h = textheight(text);
	outtextxy((getwidth() - w) / 2, 150 + (50 - h) / 2, text);
}

// ʤ������
void Win()
{
	DrawTitle(_T("Clear"));
	if (g_nCurrentLevel == g_nLevelNum - 1)
		DrawSmallText(_T("Back to menu"));
	else
		DrawSmallText(_T("Next level"));
}

// ʧ�ܽ���
void Fail()
{
	DrawSmallText(_T("Restart"));
}

// ���Ʋ˵�
void DrawMenu()
{
	// �ѿ�ʼ��Ϸ
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

		// �Ȱ��°���
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

		bool show_menu = false;	// �Ƿ������ʾ���˵�

		// ������ʤ�����棬����Ҫ������һ���ؿ�
		if (g_enuGameState == GameWin)
		{
			// �ؿ������ˣ��ص�������
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

	// �¾�ԭ��Ϸ
	DrawSmallText(_T("Salute Many Bricks Breaker"));
	//DrawSmallText(_T("Salute                              "));
	//DrawSmallText(_T("Many Bricks Breaker"), true);

	settextstyle(20, 0, _T("System"));
	LPCTSTR lpszTip = _T("Click to start");
	int w = textwidth(lpszTip);
	outtextxy((getwidth() - w) / 2, BOARD_Y + BOARD_HALF_THICKNESS + 10, lpszTip);
}

// ��ȡ�㵽ֱ�ߵľ���
float GetDistance_PointToLine(
	float x,	// �������
	float y,
	float x1,	// ֱ���������������
	float y1,
	float x2,
	float y2
)
{
	// ��ֱ��
	if (x2 == x1)
	{
		return fabsf(x - x1);
	}

	// ��б��
	float k = (y2 - y1) / (x2 - x1);
	return fabsf(k * x - y - k * x1 + y1) / sqrtf(k * k + 1);
}

// ������ײ����
int SinglePointHit(
	float x,		// ������ײ��������
	float y,
	float last_x,	// ��һ֡�����꣨�� x, y ��Ӧ��
	float last_y,
	float* vx,		// �����ٶ�ָ��
	float* vy,
	RECT rct,
	bool is_board,
	bool left,
	bool up,
	bool right,
	bool down
)
{
	int return_flag = HIT_NONE;

	// ��С���������ײ�����ص������Ȳ��ж���ײ
	if (!(x >= rct.left && x <= rct.right && y >= rct.top && y <= rct.bottom))
	{
		return HIT_NONE;
	}

	// ���С��������ש��Ⱥ�ڲ�
	// ���ʱ��Ҫ������ײ������ᵼ��С���ڷ����ڲ�����ײƯ��
	if (!(left || up || right || down))
	{
		left = up = right = down = true;
		return_flag = HIT_ABNORMAL;
	}

	// ��Խ��ײ��߽���
	bool cross_left = last_x <= rct.left && left;
	bool cross_right = last_x >= rct.right && right;
	bool cross_top = last_y <= rct.top && up;
	bool cross_bottom = last_y >= rct.bottom && down;

	// ����Ƿ���Ҫ�ж϶�����ײ
	bool vertex_judge_flag = true;
	float fVertex_X = 0;	// �ж�������ײʱʹ�õĶ���
	float fVertex_Y = 0;
	if (cross_left && cross_top)			// ���Ͻ�
	{
		//vertex_judge_flag = true;
		fVertex_X = (float)rct.left;
		fVertex_Y = (float)rct.top;
	}
	else if (cross_right && cross_top)		// ���Ͻ�
	{
		//vertex_judge_flag = true;
		fVertex_X = (float)rct.right;
		fVertex_Y = (float)rct.top;
	}
	else if (cross_left && cross_bottom)	// ���½�
	{
		//vertex_judge_flag = true;
		fVertex_X = (float)rct.left;
		fVertex_Y = (float)rct.bottom;
	}
	else if (cross_right && cross_bottom)	// ���½�
	{
		//vertex_judge_flag = true;
		fVertex_X = (float)rct.right;
		fVertex_Y = (float)rct.bottom;
	}

	// ���û��ͬʱ��Խ xy �������򣬾���Ҫ���������ĸ�����
	else
	{
		// �����Խ���±߽磬���ֻ��Ҫ����ʹ����߻����ұߵĶ���
		if (cross_top || cross_bottom)
		{
			fVertex_Y = cross_top ? (float)rct.top : (float)rct.bottom;	// ���ȾͿ���ȷ�� Y

			// �����Ҷ��пյأ��������Ҷ�����ѡ�������
			if (left && right)
			{
				fVertex_X =
					(fabsf(x - rct.left) < fabsf(x - rct.right)) ?
					(float)rct.left : (float)rct.right;
			}

			// �����ı߿�ѡ�ı�
			else if (left)
			{
				fVertex_X = (float)rct.left;
			}
			else if (right)
			{
				fVertex_X = (float)rct.right;
			}

			// Ҫ�����߶�û�յأ��Ͳ����ܷ���������ײ
			else
			{
				vertex_judge_flag = false;
			}
		}

		// �����Խ���ұ߽�
		else if (cross_left || cross_right)
		{
			fVertex_X = cross_left ? (float)rct.left : (float)rct.right;

			if (up && down)
			{
				fVertex_Y =
					(fabsf(y - rct.top) < fabsf(y - rct.bottom)) ?
					(float)rct.top : (float)rct.bottom;
			}
			else if (up)
			{
				fVertex_Y = (float)rct.top;
			}
			else if (down)
			{
				fVertex_Y = (float)rct.bottom;
			}
			else
			{
				vertex_judge_flag = false;
			}
		}
	}

	// �����ж��ǲ��Ƕ�����ײ
	bool isVertexCollision = false;	// ����Ƿ���������ײ
	if (vertex_judge_flag)
	{
		// ���㵽ֱ�߾���С�ڰ뾶����ᷢ��������ײ
		float fdVertexToLine = GetDistance_PointToLine(fVertex_X, fVertex_Y, x, y, last_x, last_y);
		if (fdVertexToLine < BALL_RADIUS)
		{
			isVertexCollision = true;

			// ��������ײ
			// ����ȡ��֡�����е���Ϊ��ײ�����㣨С��Ͷ�������е㣩
			float fCollisionX = (x + last_x) / 2;
			float fCollisionY = (y + last_y) / 2;

			// ��������ʱ��С��Բ�ĺͶ��㣬����Ƿ���ǰ��С���ٶȵĶԳ���

			float f_dxAxis = fCollisionX - fVertex_X;	// �Գ���ֱ���ϵ������
			float f_dyAxis = fCollisionY - fVertex_Y;

			// ��Գ���Ƕ�
			float f_radianAxis = atan2f(f_dyAxis, f_dxAxis);

			// ��С���ٶȽǶ�
			float f_radianOldV = atan2f(*vy, *vx);

			// ��С���ٶ��ضԳ��ᷭ�ۣ�����µ��ٶȽǶ�
			float f_radianNewV = 2 * f_radianAxis - f_radianOldV;

			// ���ٶ�
			*vx = cosf(f_radianNewV) * BALL_SPEED;
			*vy = sinf(f_radianNewV) * BALL_SPEED;
		}
	}

	// ����ɶ�����ײ
	if (isVertexCollision)
	{
		if (return_flag != HIT_ABNORMAL)
			return_flag = HIT_VERTEX;
	}

	// ������ײ
	else
	{
		// ��Խ��ײ�����ұ߽磬��ˮƽ�ٶȷ�ת
		if (cross_left || cross_right)
		{
			*vx = -*vx;
			return_flag = HIT_NORMAL;
		}
		// ��Խ��ײ�����±߽磬����ֱ�ٶȷ�ת
		if (cross_top || cross_bottom)
		{
			// �뵲����ײʱ����Ҫ����С��͵������ײλ�øı��� vx
			if (is_board)
			{
				*vx = (float)(x - g_nBoardCenterX) / BOARD_HALF_WIDTH * BALL_SPEED;
				*vy = (*vy < 0 ? 1 : -1) * sqrtf(BALL_SPEED * BALL_SPEED - *vx * *vx);
			}
			else
			{
				*vy = -*vy;
			}
			return_flag = HIT_NORMAL;
		}
	}

	// ������ײ��
	if (return_flag)
	{
		PlayHitSound(false);
	}

	return return_flag;

}// SinglePointHit

// ������ײ����
// ball						С��
// rct						��ײ������
// is_board					�Ƿ�Ϊ������ײ�ж�
// left, up, right, down	����ײ�����ֱܷ��Ƿ��л�ռ�
//
// ����ֵ���μ� HIT_ ϵ�к�
int BasicHit(Ball* ball, RECT rct, bool is_board, bool left = true, bool up = true, bool right = true, bool down = true)
{
	// �õ�С����һ֡������
	float last_x = (ball->x - ball->vx);
	float last_y = (ball->y - ball->vy);

	int return_flag = HIT_NONE;

	// ȡС������������ĸ����������ײ���
	float pfOffset[4][2] = { {-BALL_RADIUS,0},{BALL_RADIUS,0},{0,-BALL_RADIUS},{0,BALL_RADIUS} };
	for (int i = 0; i < 4; i++)
	{
		if (return_flag = SinglePointHit(
			ball->x + pfOffset[i][0],
			ball->y + pfOffset[i][1],
			last_x + pfOffset[i][0],
			last_y + pfOffset[i][1],
			&ball->vx,
			&ball->vy,
			rct,
			is_board,
			left,
			up,
			right,
			down))
		{
			return return_flag;
		}
	}

	return return_flag;
}

// ����ש��󣬵��ô˺������˴�������
// x, y ������ש���λ��
void GeneratePropByLuck(float x, float y)
{
	// ��������
	if (g_nGeneratedPropNum == PROP_MAX_NUM)
	{
		return;
	}

	// ���߱�
	int pnProps[3][2] = {
		{PROP_3X,PROBABILITY_3X},
		{PROP_SHOOT_3,PROBABILITY_SHOOT_3},
		{PROP_HEART,PROBABILITY_HEART},
	};

	// ���ѡȡһ������
	int choice = rand() % 3;

	// ���ݸ��ʾ����Ƿ������������
	if (rand() % 100 < pnProps[choice][1])
	{
		// ��������
		Prop prop;
		prop.type = pnProps[choice][0];
		prop.used = false;
		prop.x = x;
		prop.y = y;

		g_pstProps[g_nGeneratedPropNum] = prop;
		g_nGeneratedPropNum++;
	}
}

// ĳ����λ���Ƿ�Ϊ�հ����򣨿ɻ����
// ���Դ��� -1����ʱ���� true
bool IsEmptyBlock(int id)
{
	return id < 0 || g_pstMap[id].type == EMPTY;
}

// С����ײ����ʹС�򷴵���ɾ������ײש�飩
// �Լ��ж�С�����
void BallHit(Ball* ball)
{
	// �߽���ײ�ж�
	int border_thickness = 100;		// �߽������ú�ȣ���ֹ�ɳ�
	RECT rctLeft = { -border_thickness,0,1,getheight() };
	RECT rctRight = { g_nMapW * SIDE_LEN - 1,0,g_nMapW * SIDE_LEN + border_thickness,getheight() };
	RECT rctTop = { -border_thickness,-border_thickness,g_nMapW * SIDE_LEN + border_thickness,1 };
	BasicHit(ball, rctLeft, false);
	BasicHit(ball, rctRight, false);
	BasicHit(ball, rctTop, false);

	// �����ж�
	if (ball->y > getheight() - STATE_BAR_HEIGHT)
	{
		ball->alive = false;	// ��Ϊ����
		return;
	}

	// ����С���Ӧ�Ĵ��ש��λ��
	int brick_x = (int)(ball->x / SIDE_LEN);
	int brick_y = (int)(ball->y / SIDE_LEN);

	// ��Ҫ������ש��
	int pnOffset[9][2] = {
		{-1,-1}, {0,-1}, {1,-1},
		{-1, 0}, {0, 0}, {1, 0},
		{-1, 1}, {0, 1}, {1, 1},
	};

	// ����ש��
	for (int i = 0; i < 9; i++)
	{
		// ��ǰ����
		int current_x = brick_x + pnOffset[i][0];
		int current_y = brick_y + pnOffset[i][1];

		// ����
		if (current_x < 0 || current_x >= g_nMapW
			|| current_y < 0 || current_y >= g_nMapH)
			continue;

		// �Լ������ܷ��������
		int current = current_x + current_y * g_nMapW;
		int left = -1;
		int right = -1;
		int up = -1;
		int down = -1;
		if (current_x > 0)				left = current_x - 1 + current_y * g_nMapW;
		if (current_x < g_nMapW - 1)	right = current_x + 1 + current_y * g_nMapW;
		if (current_y > 0)				up = current_x + (current_y - 1) * g_nMapW;
		if (current_y < g_nMapH - 1)	down = current_x + (current_y + 1) * g_nMapW;

		// ��ʵ�巽�������ײ�ж�
		if (g_pstMap[current].type != EMPTY)
		{
			RECT rct = g_pstMap[current].rct;		// ��ǰ������ײ��

			int hit_flag = BasicHit(
				ball,
				rct,
				false,
				IsEmptyBlock(left),
				IsEmptyBlock(up),
				IsEmptyBlock(right),
				IsEmptyBlock(down)
			);

			// û��ש��
			if (hit_flag == HIT_NONE)
				continue;

			// ������Ч��ײʱ�����ש��
			if ((hit_flag == HIT_NORMAL || hit_flag == HIT_VERTEX)
				&& g_pstMap[current].type == BRICK)
			{
				g_pstMap[current].type = EMPTY;

				// ������ɵ���
				GeneratePropByLuck((float)rct.left, (float)rct.top);
			}

			break;
		}
	}

	// ������ײ�ж�
	RECT rctBoard = {
		g_nBoardCenterX - BOARD_HALF_WIDTH,
		BOARD_Y - BOARD_HALF_THICKNESS,
		g_nBoardCenterX + BOARD_HALF_WIDTH,
		BOARD_Y + BOARD_HALF_THICKNESS
	};
	BasicHit(ball, rctBoard, true);
}

// С����
// 
// ���ܣ�
// + ά��С���˶�
// + С����ײ����
// + �ж�С���������
// + ���С���ǲ��Ƕ����ˣ��Ա�����
// + �ж����������ʧ�ܣ�
void BallProcess()
{
	// ������
	if (g_bBallWait)
		return;

	// ��¼����û�л��ŵ���
	bool isAnyBall = false;

	for (int i = 0; i < g_nGeneratedBallNum; i++)
	{
		// ���������˶�
		if (g_pstBalls[i].alive)
		{
			isAnyBall = true;

			// С����ײ����
			BallHit(&g_pstBalls[i]);

			// С���˶�����
			g_pstBalls[i].x += g_pstBalls[i].vx + rand() % 10 / 100.f /* �������Ŷ�����ֹ��ĳ���򷴵���ѭ�� */;
			g_pstBalls[i].y += g_pstBalls[i].vy + rand() % 10 / 100.f;
		}
	}

	// û���ˣ���ǵȴ�������
	if (!isAnyBall)
	{
		// ��������
		if (g_nRemainingBallNum)
		{
			g_bBallWait = true;
		}

		// �ֿ�Ҳû���ˣ�����
		else
		{
			g_enuGameState = GameFail;
		}
	}
}

// �ж��Ƿ��Ѿ�ʤ��
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

	// ûש���ˣ�Ӯ��
	if (!isAnyBrick)
	{
		g_enuGameState = GameWin;
	}
}

// ��ȡ��� Vx �� Vy
void RandSpeedXY(float* vx, float* vy)
{
	*vx = fmodf((float)rand(), BALL_SPEED - rand() % 10 / 10000.f /* �����Ŷ� */) * (rand() % 2 ? 1 : -1);
	*vy = sqrtf(BALL_SPEED * BALL_SPEED - *vx * *vx) * (rand() % 2 ? 1 : -1);
}

// �����µ���
// is_board_launch	����������˵�����£��ӵ��巢��һ���µ������ǣ���������д��ʼλ�ã�
// up				ǿ�Ƴ�ʼ�ٶ�����
void GenerateBall(bool is_board_launch, float x = 0, float y = 0, bool up = false)
{
	// �����ٴ�������
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

	// �����������Ϸ��
	g_pstBalls[g_nGeneratedBallNum] = new_ball;
	g_nGeneratedBallNum++;
}

// ����С��
void LaunchBall()
{
	// �����ڵȴ�С��ʱ���ô˺���
	if (!g_bBallWait)
	{
		return;
	}

	// �ֿ⻹����
	if (g_nRemainingBallNum)
	{
		GenerateBall(true);
		g_nRemainingBallNum--;
		g_bBallWait = false;
	}
}

// Ӧ��һ������
void ApplyProp(Prop prop)
{
	switch (prop.type)
	{
	case PROP_3X:
	{
		// ��¼������������ֹ������ֳ
		int old_num = g_nGeneratedBallNum;

		// ÿ���򶼷�����������
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
		// �ӵ���λ�÷����������
		for (int i = 0; i < 3; i++)
			GenerateBall(false, prop.x, (float)(BOARD_Y - BOARD_Y_DISTACE), true);
		break;

	case PROP_HEART:
		// ��һ����
		g_nRemainingBallNum++;
		break;
	}
}

// ���ߵ�����ײ�ж�
bool SinglePointPropHit(int index, float x, float y)
{
	return (y >= BOARD_Y - BOARD_HALF_THICKNESS
		&& y <= BOARD_Y + BOARD_HALF_THICKNESS
		&& x >= g_nBoardCenterX - BOARD_HALF_WIDTH
		&& x <= g_nBoardCenterX + BOARD_HALF_WIDTH);
}

// ����������
// + ��������
// + ���ߺ͵�����ײ�ж�
// + ������Ч
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

		// ������ײ��ͨ�����ߵ�����������ײ��
		if (SinglePointPropHit(i, x1, y1)
			|| SinglePointPropHit(i, x2, y2)
			|| SinglePointPropHit(i, (x1 + x2) / 2, (y1 + y2) / 2))
		{
			// Ӧ���������
			ApplyProp(g_pstProps[i]);
			g_pstProps[i].used = true;

			// ��Ч
			PlayHitSound(true);

			continue;
		}

		// ��������
		g_pstProps[i].y += PROP_DROP_SPEED;
	}
}

// �û����봦��
void UserInputProcess()
{
	// �����˵�
	if (g_bInMenu)
	{
		// ����갴��
		while (!getmessage(EX_MOUSE).lbutton);

		// ��ʼ��Ϸ
		LaunchBall();
		g_bInMenu = false;
	}

	// ����Ϸ��

	ExMessage msg;
	while (peekmessage(&msg, EX_MOUSE))
	{
		// �ȷ���
		if (g_bBallWait)
		{
			// ����갴��
			if (msg.lbutton)
			{
				LaunchBall();
			}
		}
		else
		{
			// ����������
			g_nBoardCenterX = msg.x;
		}

		// ��������¿�ʼ����ť�ж�
		if (msg.message == WM_LBUTTONUP
			&& msg.x >= g_rctRestartBtn.left && msg.x <= g_rctRestartBtn.right
			&& msg.y >= g_rctRestartBtn.top && msg.y <= g_rctRestartBtn.bottom)
		{
			FreeLevel();
			InitLevel();
		}
	}
}

// ��Ϸ���̺���
void GameProcess()
{
	BallProcess();		// ����С��
	PropProcess();		// �������
	CheckSuccess();		// ʤ���ж�
}

// ��ʱ���ȶ�֡��
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
		g_nMapW * SIDE_LEN,	// ��ͼ��
		STATE_BAR_HEIGHT + BOARD_Y + BOARD_HALF_THICKNESS + WINDOW_BOTTOM_MARGIN /* ���� */
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


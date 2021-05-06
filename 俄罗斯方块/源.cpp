#include <iostream>
#include <windows.h>
#include <stdio.h>
#include <thread>
#include <vector>
using namespace std;

//设置窗口大小
int nScreenWidth = 80;
int nScreenHeight = 30;

wstring tetromino[7];

int nFieldWidth = 12;
int nFieldHeight = 18;
unsigned char* pField = nullptr;

//坐标变换函数
int Rotate(int px, int py, int r);
//判断是否发生冲突
bool DoesPieceFit(int nTetromino, int nRotation, int nPosX, int nPosY);

int main()
{
	wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
	for (int i = 0; i < nScreenWidth * nScreenHeight; ++i)
	{
		screen[i] = L' ';
	}
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;
	//在tetromino数组中存放预设图案，每个图案都是4*4
	tetromino[0].append(L"..X...X...X...X."); // Tetronimos 4x4
	tetromino[1].append(L"..X..XX...X.....");
	tetromino[2].append(L".....XX..XX.....");
	tetromino[3].append(L"..X..XX..X......");
	tetromino[4].append(L".X...XX...X.....");
	tetromino[5].append(L".X...X...XX.....");
	tetromino[6].append(L"..X...X..XX.....");

	//创建一个12*8的小区域
	pField = new unsigned char[nFieldWidth * nFieldHeight];
	//将该区域边界设为9，内部区域设为0
	for (int x = 0; x < nFieldWidth; x++)
	{
		for (int y = 0; y < nFieldHeight; y++)
		{
			pField[y * nFieldWidth + x] = (x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1) ? 9 : 0;
		}
	}

	//Game Logic
	bool bKey[4];		//四个键
	bool bRotateHold = false;		//是否按住的标签，默认false
	int nCurrentPiece = 0;			//图案类型，从trtromino[0]~tetromino[6]
	int nCurrentRotation = 0;		//旋转角度，0代表0°，1代表90°，2代表180°，3代表270°...该值是用来%4的
	int nCurrentX = nFieldWidth / 2;	//初始位置在X轴中间
	int nCurrentY = 0;					//初始位置在Y轴0点
	int nSpeed = 20;		//设置游戏速度
	int nSpeedCounter = 0;	//设置计数器
	int nPieceCount = 0;
	bool bForceDown = false;
	int nScore = 0;
	vector<int> vLines;
	bool bGameOver = false;

	while (!bGameOver)
	{
		//游戏时间
		this_thread::sleep_for(50ms);
		nSpeedCounter++;
		bForceDown = (nSpeedCounter == nSpeed);

		//游戏逻辑
		//把读取到的按键存放进bKey数组中
		for (int k = 0; k < 4; k++)
		{
			bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28\x26"[k]))) != 0;
		}
		//右左下三建
		nCurrentX += (bKey[0] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY)) ? 1 : 0;
		nCurrentX -= (bKey[1] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY)) ? 1 : 0;
		nCurrentY += (bKey[2] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) ? 1 : 0;
		//上键旋转键
		if (bKey[3])
		{
			nCurrentRotation += (!bRotateHold && DoesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY)) ? 1 : 0;
			bRotateHold = false;
		}
		else
		{
			bRotateHold =true;
		}

		//force the piece down the playfield if it's time
		if (bForceDown)
		{
			//update difficulty every 50 pieces
			if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1))
			{
				++nCurrentY;
			}
			else
			{
				//lock the current piece into the field
				for (int px = 0; px < 4; px++)
				{
					for (int py = 0; py < 4; py++)
					{
						if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] != L'.')
						{
							pField[(nCurrentY + py) * nFieldWidth + (nCurrentX + px)] = nCurrentPiece + 1;
						}
					}
				}
				nPieceCount++;
				if (nPieceCount % 10 == 0)
				{
					if (nSpeed >= 10)
					{
						nSpeed--;
					}
				}
				//check have we got any lines
				for (int py = 0; py < 4; py++)
				{
					if (nCurrentY + py < nFieldHeight - 1) {
						bool bLine = true;
						for (int px = 1; px < nFieldWidth - 1; px++)
							bLine &= (pField[(nCurrentY + py) * nFieldWidth + px]) != 0;

						if (bLine) {
							// Remove Line if it's complete
							for (int px = 1; px < nFieldWidth - 1; px++)
								pField[(nCurrentY + py) * nFieldWidth + px] = 8;
							vLines.push_back(nCurrentY + py);
						}
					}
				}
				nScore += 25;
				if (!vLines.empty())
				{
					nScore += (1 << vLines.size()) * 100;
				}
				//choose next piece
				nCurrentX = nFieldWidth / 2;
				nCurrentY = 0;
				nCurrentRotation = 0;
				nCurrentPiece = rand() % 7;
				//if piece does not fit
				bGameOver = !DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);
			}
			nSpeedCounter = 0;
		}

		//绘制区域
		for (int x = 0; x < nFieldWidth; ++x)
		{
			for (int y = 0; y < nFieldHeight; ++y)
			{
				screen[(y + 2) * nScreenWidth + (x + 2)] = L" ABCDEFG=#"[pField[y * nFieldWidth + x]];
			}
		}
		if (!vLines.empty())
		{
			WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
			this_thread::sleep_for(400ms);		//delay a bit

			for (auto& v : vLines)
			{
				for (int px = 1; px < nFieldWidth - 1; px++)
				{
					for (int py = v; py > 0; py--)
					{
						pField[py * nFieldWidth + px] = pField[(py - 1) * nFieldWidth + px];
					}
					pField[px] = 0;
				}
			}
			vLines.clear();
		}


		//绘制totromino的片区
		for (int px = 0; px < 4; px++)
		{
			for (int py = 0; py < 4; py++)
			{
				if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] != L'.')
				{
					//Y轴顶部两行、X轴左端两列不绘制
					//ASCII:0+65="A"，1+65="B"
					screen[(nCurrentY + py + 2) * nScreenWidth + (nCurrentX + px + 2)] = nCurrentPiece + 65;
				}

				//draw score
				swprintf_s(&screen[2 * nScreenWidth + nFieldWidth + 6], 16, L"SCORE: %8d", nScore);

				//Display Frame
				WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);

			}
			
		}
	}
	CloseHandle(hConsole);
	cout << "Game Over" << endl << "Score:" << nScore << endl;
	system("pause");
	return 0;
}

int Rotate(int px, int py, int r)
{
	int pi = 0;
	switch (r % 4)
	{
	case 0:
		pi = py * 4 + px;	//旋转0°
		break;
	case 1:
		pi = 12 + py - (px * 4);	//旋转90°
		break;
	case 2:
		pi = 15 - (py * 4) - px;	//旋转180°
		break;
	case 3:
		pi = 3 - py + (px * 4);		//旋转270°
		break;
	}
	return pi;
}

bool DoesPieceFit(int nTetromino, int nRotation, int nPosX, int nPosY)
{
	// The field is full so zero spaces
	for (int px = 0; px < 4; px++)
	{
		for (int py = 0; py < 4; py++) {
			int pi = Rotate(px, py, nRotation);		//pi代表旋转后所在数组的坐标

			int fi = (nPosY + py) * nFieldWidth + (nPosX + px);		//fi代表区域块的坐标索引

			// Check that test is in bounds. Note out of bounds does
			// not necessarily mean a fail, as the long vertical piece
			// can have cells that lie outside the boundary, so we'll
			// just ignore them
			if (nPosX + px >= 0 && nPosX + px < nFieldWidth) {
				if (nPosY + py >= 0 && nPosY + py < nFieldHeight) 
				{
					if (tetromino[nTetromino][pi] != L'.' && pField[fi] != 0)	//旋转后的数组处存有图案而且区域上不为0
					{
						return false; // fail on first hit
					}
				}
			}
		}
	}
	return true;
}
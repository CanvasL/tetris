#include <iostream>
#include <windows.h>
#include <stdio.h>
#include <thread>
#include <vector>
using namespace std;

//���ô��ڴ�С
int nScreenWidth = 80;
int nScreenHeight = 30;

wstring tetromino[7];

int nFieldWidth = 12;
int nFieldHeight = 18;
unsigned char* pField = nullptr;

//����任����
int Rotate(int px, int py, int r);
//�ж��Ƿ�����ͻ
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
	//��tetromino�����д��Ԥ��ͼ����ÿ��ͼ������4*4
	tetromino[0].append(L"..X...X...X...X."); // Tetronimos 4x4
	tetromino[1].append(L"..X..XX...X.....");
	tetromino[2].append(L".....XX..XX.....");
	tetromino[3].append(L"..X..XX..X......");
	tetromino[4].append(L".X...XX...X.....");
	tetromino[5].append(L".X...X...XX.....");
	tetromino[6].append(L"..X...X..XX.....");

	//����һ��12*8��С����
	pField = new unsigned char[nFieldWidth * nFieldHeight];
	//��������߽���Ϊ9���ڲ�������Ϊ0
	for (int x = 0; x < nFieldWidth; x++)
	{
		for (int y = 0; y < nFieldHeight; y++)
		{
			pField[y * nFieldWidth + x] = (x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1) ? 9 : 0;
		}
	}

	//Game Logic
	bool bKey[4];		//�ĸ���
	bool bRotateHold = false;		//�Ƿ�ס�ı�ǩ��Ĭ��false
	int nCurrentPiece = 0;			//ͼ�����ͣ���trtromino[0]~tetromino[6]
	int nCurrentRotation = 0;		//��ת�Ƕȣ�0����0�㣬1����90�㣬2����180�㣬3����270��...��ֵ������%4��
	int nCurrentX = nFieldWidth / 2;	//��ʼλ����X���м�
	int nCurrentY = 0;					//��ʼλ����Y��0��
	int nSpeed = 20;		//������Ϸ�ٶ�
	int nSpeedCounter = 0;	//���ü�����
	int nPieceCount = 0;
	bool bForceDown = false;
	int nScore = 0;
	vector<int> vLines;
	bool bGameOver = false;

	while (!bGameOver)
	{
		//��Ϸʱ��
		this_thread::sleep_for(50ms);
		nSpeedCounter++;
		bForceDown = (nSpeedCounter == nSpeed);

		//��Ϸ�߼�
		//�Ѷ�ȡ���İ�����Ž�bKey������
		for (int k = 0; k < 4; k++)
		{
			bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28\x26"[k]))) != 0;
		}
		//����������
		nCurrentX += (bKey[0] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY)) ? 1 : 0;
		nCurrentX -= (bKey[1] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY)) ? 1 : 0;
		nCurrentY += (bKey[2] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) ? 1 : 0;
		//�ϼ���ת��
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

		//��������
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


		//����totromino��Ƭ��
		for (int px = 0; px < 4; px++)
		{
			for (int py = 0; py < 4; py++)
			{
				if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] != L'.')
				{
					//Y�ᶥ�����С�X��������в�����
					//ASCII:0+65="A"��1+65="B"
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
		pi = py * 4 + px;	//��ת0��
		break;
	case 1:
		pi = 12 + py - (px * 4);	//��ת90��
		break;
	case 2:
		pi = 15 - (py * 4) - px;	//��ת180��
		break;
	case 3:
		pi = 3 - py + (px * 4);		//��ת270��
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
			int pi = Rotate(px, py, nRotation);		//pi������ת���������������

			int fi = (nPosY + py) * nFieldWidth + (nPosX + px);		//fi������������������

			// Check that test is in bounds. Note out of bounds does
			// not necessarily mean a fail, as the long vertical piece
			// can have cells that lie outside the boundary, so we'll
			// just ignore them
			if (nPosX + px >= 0 && nPosX + px < nFieldWidth) {
				if (nPosY + py >= 0 && nPosY + py < nFieldHeight) 
				{
					if (tetromino[nTetromino][pi] != L'.' && pField[fi] != 0)	//��ת������鴦����ͼ�����������ϲ�Ϊ0
					{
						return false; // fail on first hit
					}
				}
			}
		}
	}
	return true;
}
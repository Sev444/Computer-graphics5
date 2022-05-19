#include <iostream>
#include <Windows.h>
#include <math.h>
#include <conio.h>

using namespace std;

#define MAXARR 300
#define MAXLST 300

float* Py;
float* Px;

static int KOD, NWER;
static float* pt_X;
static float* pt_Y;
static int ntek;
static int idlspi;
static int IYREB[MAXLST];
static float RXREB[MAXLST];
static float RPRIR[MAXLST];

int SCREEN_WIDTH = 900;
int SCREEN_HEIGHT = 600;
HDC Hdc;

const int N = 8; //для параллелепипеда - восемь строк координат, восемь точек
const int M = 4; //три координаты в трехмерном пространстве + одна однородная
const float DX = 5;
const float DY = 5;
const float ALPHA = 0.087;
const float S1 = 1.01;
const float S2 = 0.99;

float parallelepiped[N][M] = {
	{100, 400, 100, 1}, {100, 200, 100, 1},
	{400, 200, 100, 1}, {400, 400, 100, 1},
	{100, 400, 300, 1}, {100, 200, 300, 1},
	{400, 200, 300, 1}, {400, 400, 300, 1}
};

////////////////////////////////////////////////////////////////////////////////////////////////

void FILSTR(int kod, int iy, float ixn, float ixk);
void SORT(int n, float* iarr);
static void OBRREB(int isd);
void V_FP0(int pixel, int kol, float* Px, float* Py);

void multing(float line[N][M], float matrix[M][M]);
void moving(float line[N][M], float dx, float dy);
void scaling(float line[N][M], float S);
void rotating_z(float line[N][M], float angle);
void rotating_x(float line[N][M], float angle);
void rotating_y(float line[N][M], float angle);
void draw(float figure[N][M]);


void multing(float line[N][M], float matrix[M][M]) {
	float res[N][M] = { 0, 0, 0, 0 };
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < M; j++) {
			for (int k = 0; k < M; k++)
				res[i][j] += (line[i][k] * matrix[k][j]);
		}
	}

	for (int i = 0; i < N; i++) {
		for (int j = 0; j < M; j++)
			line[i][j] = res[i][j];
	}
}

void moving(float line[N][M], float dx, float dy) {
	float dz = 5; //dz = dx = dy
	float m_move[M][M] = {
		{1, 0, 0, 0},
		{0, 1, 0, 0},
		{0, 0, 1, 0},
		{dx, dy, dz, 1} }; //матрица перемещения
	multing(line, m_move);
}

void scaling(float line[N][M], float S) {
	float xc = 0.0, yc = 0.0, zc = 0.0;
	for (int i = 0; i < N; i++)
		xc += line[i][0];
	xc = xc / N;
	for (int i = 0; i < N; i++)
		yc += line[i][1];
	yc = yc / N;
	for (int i = 0; i < N; i++)
		zc += line[i][2];
	zc = zc / N;

	float m_scale[M][M] = {
		{S, 0, 0, 0},
		{0, S, 0, 0},
		{0, 0, S, 0},
		{xc * (1 - S), yc * (1 - S), zc * (1 - S), 1} }; //матрица масштабирования
	multing(line, m_scale);
}

void rotating_z(float line[N][M], float angle) {
	float xc = 0.0, yc = 0.0;
	for (int i = 0; i < N; i++)
		xc += line[i][0];
	xc = xc / N;
	for (int i = 0; i < N; i++)
		yc += line[i][1];
	yc = yc / N;

	float m_rotate[M][M] = {
		{cos(angle), sin(angle), 0, 0},
		{-sin(angle), cos(angle), 0, 0},
		{0, 0, 1, 0},
		{xc * (1 - cos(angle)) + yc * sin(angle), yc * (1 - cos(angle)) - xc * sin(angle), 0, 1} };
	multing(line, m_rotate);
}

void rotating_x(float line[N][M], float angle) {
	float yc = 0.0, zc = 0.0;
	for (int i = 0; i < N; i++)
		yc += line[i][1];
	yc = yc / N;
	for (int i = 0; i < N; i++)
		zc += line[i][2];
	zc = zc / N;

	float m_rotate[M][M] = {
		{1, 0, 0, 0},
		{0, cos(angle), sin(angle), 0},
		{0, -sin(angle), cos(angle), 0},
		{0, yc * (1 - cos(angle)) + zc * sin(angle), zc * (1 - cos(angle)) - yc * sin(angle), 1} };
	multing(line, m_rotate);
}

void rotating_y(float line[N][M], float angle) {
	float xc = 0.0, zc = 0.0;
	for (int i = 0; i < N; i++)
		xc += line[i][0];
	xc = xc / N;
	for (int i = 0; i < N; i++)
		zc += line[i][2];
	zc = zc / N;

	float m_rotate[M][M] = {
		{cos(angle), 0, -sin(angle), 0},
		{0, 1, 0, 0},
		{sin(angle), 0, cos(angle), 0},
		{xc * (1 - cos(angle)) - zc * sin(angle), 0, zc * (1 - cos(angle)) + xc * sin(angle), 1} };
	multing(line, m_rotate);
}

void V_FP0(int pixel, int kol, float* Px, float* Py) {
	int ii, jj, kk;
	int iymin;
	int iymax;
	int iysled;
	int iytek;
	int ikledg;
	int ibgind;
	int iedg[MAXARR];
	int inom[MAXARR];
	float irabx[MAXLST];
	KOD = pixel;
	NWER = kol;
	pt_X = Px;
	pt_Y = Py;

	for (int i = 1; i <= kol; ++i) {
		iedg[i] = Py[i];
		inom[i] = i;
	}
	for (int i = 1; i <= kol; ++i) {
		iymin = iedg[i];
		ntek = i;
		for (int j = i + 1; j <= kol; ++j) {
			if (iedg[j] < iymin) {
				iymin = iedg[j];
				ntek = j;
			}
		}

		if (ntek != i) {
			iedg[ntek] = iedg[i];
			iedg[i] = iymin;
			iymin = inom[ntek];
			inom[ntek] = inom[i];
			inom[i] = iymin;
		}
	}

	idlspi = 0;
	ibgind = 1;
	iytek = iedg[1];
	iymax = iedg[kol];
	for (;;) {
		ikledg = 0;
		for (int i = ibgind; i <= kol; ++i) {
			if (iedg[i] != iytek)
				break;
			else
				ikledg++;
		}

		for (int i = 1; i <= ikledg; ++i) {
			ntek = inom[ibgind + i - 1];
			OBRREB(-1);
			OBRREB(+1);
		}

		if (!idlspi) break;
		ii = ibgind + ikledg;
		iysled = iymax;
		if (ii < kol)
			iysled = iedg[ii];

		for (double i = iytek; i <= iysled; ++i) {
			for (int j = 1; j <= idlspi; ++j)
				irabx[j] = RXREB[j];
			SORT(idlspi, irabx + 1);
			for (int j = 1; j <= idlspi - 1; j += 2)
				FILSTR(pixel, i, round(irabx[j]), round(irabx[j + 1]));
			if (i == iysled) continue;
			for (int j = 1; j <= idlspi; ++j)
				RXREB[j] = RXREB[j] + RPRIR[j];
		}

		if (iysled == iymax) break;
		for (ii = 0; ii <= idlspi; ii++) {
			if (IYREB[ii] != iysled) continue;
			--idlspi;
			for (jj = ii; jj <= idlspi; ++jj) {
				IYREB[jj] = IYREB[kk = jj + 1];
				RXREB[jj] = RXREB[kk];
				RPRIR[jj] = RPRIR[kk];
			}
		}
		ibgind += ikledg;
		iytek = iysled;
	}
}

static void OBRREB(int isd) {
	int inext, iyt, ixt;
	float xt, xnext, dy;
	inext = ntek + isd;
	if (inext < 1)
		inext = NWER;
	if (inext > NWER)
		inext = 1;
	dy = pt_Y[inext] - pt_Y[ntek];
	if (dy >= 0) {
		xnext = pt_X[inext];
		xt = pt_X[ntek];
		if (dy != 0) {
			idlspi++;
			IYREB[idlspi] = pt_Y[inext];
			RXREB[idlspi] = xt;
			RPRIR[idlspi] = (xnext - xt) / dy;
		}
		else {
			iyt = pt_Y[ntek];
			inext = xnext;
			ixt = xt;
			FILSTR(KOD, iyt, round(inext), round(ixt));
		}
	}
}

void SORT(int n, float* iarr) {
	int l;
	float k, min;
	for (int i = 0; i < n; ++i) {
		min = iarr[l = i];
		for (int j = i + 1; j < n; ++j)
			if ((k = iarr[j]) < min) {
				l = j;
				min = k;
			}
		if (l != i) {
			iarr[l] = iarr[i];
			iarr[i] = min;
		}
	}
}

void FILSTR(int kod, int iy, float ixn, float ixk) {
	while (ixn <= ixk) SetPixel(Hdc, ixn++, iy, kod);
}

//алгоритм удаления с z-буфером
void painter(float figure[N][M]) {
	float Pol[6] = {}, Pol1[7] = {}, min = 0;
	int ntek, inom[7] = {};
	Pol[0] = (figure[0][2] + figure[1][2] + figure[2][2] + figure[3][2]) / 4; //ABCD
	Pol[1] = (figure[4][2] + figure[5][2] + figure[6][2] + figure[7][2]) / 4; //A1B1C1D1
	Pol[2] = (figure[0][2] + figure[4][2] + figure[3][2] + figure[7][2]) / 4; //AA1DD1
	Pol[3] = (figure[7][2] + figure[3][2] + figure[2][2] + figure[6][2]) / 4; //CC1DD1
	Pol[4] = (figure[1][2] + figure[5][2] + figure[2][2] + figure[6][2]) / 4; //BB1CC1
	Pol[5] = (figure[1][2] + figure[4][2] + figure[0][2] + figure[5][2]) / 4; //AA1BB1

	for (int i = 1; i <= 6; ++i) {
		Pol1[i] = Pol[i - 1];
		inom[i] = i;
	}

	for (int i = 1; i <= 6; ++i) {
		min = Pol1[i];
		ntek = i;
		for (int j = i + 1; j <= 6; ++j)
			if (Pol1[j] <= min) {
				min = Pol1[j];
				ntek = j;
			}
		if (ntek != i) {
			Pol1[ntek] = Pol1[i];
			Pol1[i] = min;
			min = inom[ntek];
			inom[ntek] = inom[i];
			inom[i] = min;
		}
	}

	int num;
	for (int i = 1; i <= 7; i++) {
		Px = new float[4 + 1];
		Py = new float[4 + 1];
		num = inom[i];
		switch (num) {
		case 1:
			Px[1] = figure[0][0];
			Py[1] = figure[0][1];
			Px[2] = figure[1][0];
			Py[2] = figure[1][1];
			Px[3] = figure[2][0];
			Py[3] = figure[2][1];
			Px[4] = figure[3][0];
			Py[4] = figure[3][1];
			V_FP0(0xff0000, 4, Px, Py);
			delete[] Px;
			delete[] Py;
			break;
		case 2:
			Px[1] = figure[4][0];
			Py[1] = figure[4][1];
			Px[2] = figure[5][0];
			Py[2] = figure[5][1];
			Px[3] = figure[6][0];
			Py[3] = figure[6][1];
			Px[4] = figure[7][0];
			Py[4] = figure[7][1];
			V_FP0(0x00ff00, 4, Px, Py);
			delete[] Px;
			delete[] Py;
			break;
		case 3:
			Px[1] = figure[0][0];
			Py[1] = figure[0][1];
			Px[2] = figure[3][0];
			Py[2] = figure[3][1];
			Px[3] = figure[7][0];
			Py[3] = figure[7][1];
			Px[4] = figure[4][0];
			Py[4] = figure[4][1];
			V_FP0(0x0000ff, 4, Px, Py);
			delete[] Px;
			delete[] Py;
			break;
		case 4:
			Px[1] = figure[7][0];
			Py[1] = figure[7][1];
			Px[2] = figure[3][0];
			Py[2] = figure[3][1];
			Px[3] = figure[2][0];
			Py[3] = figure[2][1];
			Px[4] = figure[6][0];
			Py[4] = figure[6][1];
			V_FP0(0xffff00, 4, Px, Py);
			delete[] Px;
			delete[] Py;
			break;
		case 5:
			Px[1] = figure[1][0];
			Py[1] = figure[1][1];
			Px[2] = figure[5][0];
			Py[2] = figure[5][1];
			Px[3] = figure[6][0];
			Py[3] = figure[6][1];
			Px[4] = figure[2][0];
			Py[4] = figure[2][1];
			V_FP0(0xff00ff, 4, Px, Py);
			delete[] Px;
			delete[] Py;
			break;
		case 6:
			Px[1] = figure[0][0];
			Py[1] = figure[0][1];
			Px[2] = figure[4][0];
			Py[2] = figure[4][1];
			Px[3] = figure[5][0];
			Py[3] = figure[5][1];
			Px[4] = figure[1][0];
			Py[4] = figure[1][1];
			V_FP0(0x00ffff, 4, Px, Py);
			delete[] Px;
			delete[] Py;
			break;
		}
	}
}


void draw(HDC hdc, float figure[N][M]) {
	painter(figure);
}

void control(int key) {
	if (key == int('W')) moving(parallelepiped, 0, -DY);
	if (key == int('A')) moving(parallelepiped, -DX, 0);
	if (key == int('S')) moving(parallelepiped, 0, DY);
	if (key == int('D')) moving(parallelepiped, DX, 0);
	if (key == 101) rotating_x(parallelepiped, ALPHA);
	if (key == 104) rotating_x(parallelepiped, -ALPHA);
	if (key == 100) rotating_y(parallelepiped, ALPHA);
	if (key == 102) rotating_y(parallelepiped, -ALPHA);
	if (key == 105) rotating_z(parallelepiped, ALPHA);
	if (key == 103) rotating_z(parallelepiped, -ALPHA);
	if (key == int('E')) scaling(parallelepiped, S1);
	if (key == int('Q')) scaling(parallelepiped, S2);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam) {
	PAINTSTRUCT ps; RECT Rect; HDC hdc, hCmpDC; HBITMAP hBmp;

	switch (messg) {

	case WM_PAINT: {
		GetClientRect(hWnd, &Rect);
		hdc = BeginPaint(hWnd, &ps);

		// Создание теневого контекста для двойной буферизации
		hCmpDC = CreateCompatibleDC(hdc);
		hBmp = CreateCompatibleBitmap(hdc, Rect.right - Rect.left, Rect.bottom - Rect.top);
		SelectObject(hCmpDC, hBmp);

		// Закраска фоновым цветом
		LOGBRUSH br;
		br.lbStyle = BS_SOLID;
		br.lbColor = 0xFFFFFF;
		HBRUSH brush;
		brush = CreateBrushIndirect(&br);
		FillRect(hCmpDC, &Rect, brush);
		DeleteObject(brush);

		// Отрисовка
		Hdc = hCmpDC;
		draw(hCmpDC, parallelepiped);

		// Копируем изображение из теневого контекста на экран
		SetStretchBltMode(hdc, COLORONCOLOR);
		BitBlt(hdc, 0, 0, Rect.right - Rect.left, Rect.bottom - Rect.top, hCmpDC, 0, 0, SRCCOPY);

		// Удаляем ненужные системные объекты
		DeleteDC(hCmpDC);
		DeleteObject(hBmp);
		hCmpDC = NULL;

		EndPaint(hWnd, &ps);
	} break;

	case WM_ERASEBKGND:
		return 1;
		break;

	case WM_KEYDOWN:
		control(int(wParam));
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return (DefWindowProc(hWnd, messg, wParam, lParam));
	}
	return (0);
}
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
	HWND hWnd;
	MSG lpMsg;
	WNDCLASS WndClass;

	// Заполняем структуру класса окна
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = hInstance;
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = L"CG_WAPI_Template";

	// Регистрируем класс окна
	if (!RegisterClass(&WndClass)) {
		MessageBox(NULL, L"Не могу зарегистрировать класс окна!", L"Ошибка", MB_OK);
		return 0;
	}

	// Создаем основное окно приложения
	hWnd = CreateWindow(
		L"CG_WAPI_Template", // Имя класса 
		L"Компьютерная графика лабораторная работа №5", // Текст заголовка
		WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, // Стиль окна 
		CW_USEDEFAULT, CW_USEDEFAULT, // Позиция левого верхнего угла 
		SCREEN_WIDTH, SCREEN_HEIGHT, // Ширина и высота окна 
		(HWND)NULL, // Указатель на родительское окно NULL 
		(HMENU)NULL, // Используется меню класса окна 
		(HINSTANCE)hInstance, // Указатель на текущее приложение
		NULL); // Передается в качестве lParam в событие WM_CREATE


	// Если окно не удалось создать
	if (!hWnd) {
		MessageBox(NULL, L"Не удается создать главное окно!", L"Ошибка", MB_OK);
		return 0;
	}

	// Показываем окно
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	// Выполняем цикл обработки сообщений до закрытия приложения
	while (GetMessage(&lpMsg, NULL, 0, 0)) {
		TranslateMessage(&lpMsg);
		DispatchMessage(&lpMsg);
	}
	return (int)lpMsg.wParam;
}
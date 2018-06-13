//
// 網を描く
//
//     m: 横方向の結び目の数 (線分の数 + 1)
//     n: 縦方向の結び目の数 (線分の数 + 1)
//     p: 結び目の位置 (三次元)
//

#include<cmath>
#include <cstdio>

extern void net(int m, int n, void *p);

// 時間間隔
static const double dt = (1.0 / 60.0);

// 結び目の数
static const int pointsi = 10;
static const int pointsj = 10;

int i, j;

// 結び目の位置
static double position[pointsi][pointsj][3];

//ばねの自然長
const double l0 = 1.0;

//ばね定数
const double k = 30.0;

//質点の質量
const double m = 1.0;

//重力加速度
const double g[] = { 0.0, -0.98, 0.0 };

// 減衰係数
const double c = 5.0;

double lx, ly, lz, L, x, F, ex, ey, ez;

// p0, p1 間を結ぶばねの力を f に求める
void force(double *f, const double *p0, const double *p1)
{

	// ここに p0, p1 間を結ぶばねの力を f に求めるプログラムを書く
	//ばねの長さ
	lx = p1[0] - p0[0]; //x
	ly = p1[1] - p0[1]; //y
	lz = p1[2] - p0[2]; //z

	L = sqrt(lx*lx + ly*ly + lz*lz); //2点間の距離

	//ばねの伸び量
	x = L - l0;

	//ばねの力
	F = -k * x;

	//ばねの伸び縮みする方向(position[0]を固定した場合)
	ex = lx / L; //x方向
	ey = ly / L; //y方向
	ez = lz / L; //z方向

	//position[1]の質点に加わる力のベクトル

	f[0] += F * ex; //x方向 //f[0]==入力された配列f0[2]とかの0番目
	f[1] += F * ey; //y方向
	f[2] += F * ez; //z方向

}

void init()
{
	double f[pointsi][pointsj][3];//力

	//先にすべての力を計算してから位置の更新
	//端っこの点は除く
	for (i = 0; i < pointsi; i++)
	{
		for (j = 0; j < pointsj; j++)
		{
			f[i][j][0] = 0;
			f[i][j][1] = 0;
			f[i][j][2] = 0;

			//左上の隅
			if (i > 0 && j > 0) force(f[i][j], position[i - 1][j - 1], position[i][j]);
			//左下の隅
			if (j > 0 && i < pointsi - 1) force(f[i][j], position[i + 1][j - 1], position[i][j]);
			//右下の隅
			if (i < pointsi - 1 && j < pointsj - 1) force(f[i][j], position[i + 1][j + 1], position[i][j]);
			//右上の隅
			if (j < pointsj - 1 && i > 0) force(f[i][j], position[i - 1][j + 1], position[i][j]);
			//上の外枠
			if (i > 0) force(f[i][j], position[i - 1][j], position[i][j]);
			//左の外枠
			if (j > 0) force(f[i][j], position[i][j - 1], position[i][j]);
			//下の外枠
			if (i < pointsi - 1) force(f[i][j], position[i + 1][j], position[i][j]);
			//右の外枠
			if (j < pointsj - 1) force(f[i][j], position[i][j + 1], position[i][j]);
		}
	}

	for (i = 0; i < pointsi; i++)
	{
		for (j = 0; j < pointsj; j++)
		{
			//質点の速度(初速度)position[1]の初速度はvelosity[1]
			static double velosity[pointsi][pointsj][3];

			//position[1]の質点の加速度
			double ax = (f[i][j][0] - c * velosity[i][j][0]) / m + g[0]; //x方向
			double ay = (f[i][j][1] - c * velosity[i][j][1]) / m + g[1]; //y方向	
			double az = (f[i][j][2] - c * velosity[i][j][2]) / m + g[2]; //z方向	

			//dt秒後の速度
			double vx = velosity[i][j][0] + ax * dt; //x方向
			double vy = velosity[i][j][1] + ay * dt; //y方向
			double vz = velosity[i][j][2] + az * dt; //y方向

			velosity[i][j][0] = vx;
			velosity[i][j][1] = vy;
			velosity[i][j][2] = vz;

			//dt秒後の位置(position[1]の位置を更新する)
			double px = position[i][j][0] + 0.5 * ((vx + velosity[i][j][0]) * dt); //x方向
			double py = position[i][j][1] + 0.5 * ((vy + velosity[i][j][1]) * dt); //y方向
			double pz = position[i][j][2] + 0.5 * ((vz + velosity[i][j][2]) * dt); //z方向

			position[i][j][0] = px;
			position[i][j][1] = py;
			position[i][j][2] = pz;
		}
	}
}

// 描画
void draw(void)
{
	// 網
	net(pointsi, pointsj, position);

	init();
	//
	// position の更新は net の実行後に行う
	//
}


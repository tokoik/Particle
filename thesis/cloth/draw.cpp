//
// �Ԃ�`��
//
//     m: �������̌��іڂ̐� (�����̐� + 1)
//     n: �c�����̌��іڂ̐� (�����̐� + 1)
//     p: ���іڂ̈ʒu (�O����)
//

#include<cmath>
#include <cstdio>

extern void net(int m, int n, void *p);

// ���ԊԊu
static const double dt = (1.0 / 60.0);

// ���іڂ̐�
static const int pointsi = 10;
static const int pointsj = 10;

int i, j;

// ���іڂ̈ʒu
static double position[pointsi][pointsj][3];

//�΂˂̎��R��
const double l0 = 1.0;

//�΂˒萔
const double k = 30.0;

//���_�̎���
const double m = 1.0;

//�d�͉����x
const double g[] = { 0.0, -0.98, 0.0 };

// �����W��
const double c = 5.0;

double lx, ly, lz, L, x, F, ex, ey, ez;

// p0, p1 �Ԃ����Ԃ΂˂̗͂� f �ɋ��߂�
void force(double *f, const double *p0, const double *p1)
{

	// ������ p0, p1 �Ԃ����Ԃ΂˂̗͂� f �ɋ��߂�v���O����������
	//�΂˂̒���
	lx = p1[0] - p0[0]; //x
	ly = p1[1] - p0[1]; //y
	lz = p1[2] - p0[2]; //z

	L = sqrt(lx*lx + ly*ly + lz*lz); //2�_�Ԃ̋���

	//�΂˂̐L�ї�
	x = L - l0;

	//�΂˂̗�
	F = -k * x;

	//�΂˂̐L�яk�݂������(position[0]���Œ肵���ꍇ)
	ex = lx / L; //x����
	ey = ly / L; //y����
	ez = lz / L; //z����

	//position[1]�̎��_�ɉ����͂̃x�N�g��

	f[0] += F * ex; //x���� //f[0]==���͂��ꂽ�z��f0[2]�Ƃ���0�Ԗ�
	f[1] += F * ey; //y����
	f[2] += F * ez; //z����

}

void init()
{
	double f[pointsi][pointsj][3];//��

	//��ɂ��ׂĂ̗͂��v�Z���Ă���ʒu�̍X�V
	//�[�����̓_�͏���
	for (i = 0; i < pointsi; i++)
	{
		for (j = 0; j < pointsj; j++)
		{
			f[i][j][0] = 0;
			f[i][j][1] = 0;
			f[i][j][2] = 0;

			//����̋�
			if (i > 0 && j > 0) force(f[i][j], position[i - 1][j - 1], position[i][j]);
			//�����̋�
			if (j > 0 && i < pointsi - 1) force(f[i][j], position[i + 1][j - 1], position[i][j]);
			//�E���̋�
			if (i < pointsi - 1 && j < pointsj - 1) force(f[i][j], position[i + 1][j + 1], position[i][j]);
			//�E��̋�
			if (j < pointsj - 1 && i > 0) force(f[i][j], position[i - 1][j + 1], position[i][j]);
			//��̊O�g
			if (i > 0) force(f[i][j], position[i - 1][j], position[i][j]);
			//���̊O�g
			if (j > 0) force(f[i][j], position[i][j - 1], position[i][j]);
			//���̊O�g
			if (i < pointsi - 1) force(f[i][j], position[i + 1][j], position[i][j]);
			//�E�̊O�g
			if (j < pointsj - 1) force(f[i][j], position[i][j + 1], position[i][j]);
		}
	}

	for (i = 0; i < pointsi; i++)
	{
		for (j = 0; j < pointsj; j++)
		{
			//���_�̑��x(�����x)position[1]�̏����x��velosity[1]
			static double velosity[pointsi][pointsj][3];

			//position[1]�̎��_�̉����x
			double ax = (f[i][j][0] - c * velosity[i][j][0]) / m + g[0]; //x����
			double ay = (f[i][j][1] - c * velosity[i][j][1]) / m + g[1]; //y����	
			double az = (f[i][j][2] - c * velosity[i][j][2]) / m + g[2]; //z����	

			//dt�b��̑��x
			double vx = velosity[i][j][0] + ax * dt; //x����
			double vy = velosity[i][j][1] + ay * dt; //y����
			double vz = velosity[i][j][2] + az * dt; //y����

			velosity[i][j][0] = vx;
			velosity[i][j][1] = vy;
			velosity[i][j][2] = vz;

			//dt�b��̈ʒu(position[1]�̈ʒu���X�V����)
			double px = position[i][j][0] + 0.5 * ((vx + velosity[i][j][0]) * dt); //x����
			double py = position[i][j][1] + 0.5 * ((vy + velosity[i][j][1]) * dt); //y����
			double pz = position[i][j][2] + 0.5 * ((vz + velosity[i][j][2]) * dt); //z����

			position[i][j][0] = px;
			position[i][j][1] = py;
			position[i][j][2] = pz;
		}
	}
}

// �`��
void draw(void)
{
	// ��
	net(pointsi, pointsj, position);

	init();
	//
	// position �̍X�V�� net �̎��s��ɍs��
	//
}

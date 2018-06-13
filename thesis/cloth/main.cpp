#include <cmath>
#include <cstdio>
#include <cstdlib>
#include "gg.h"

static int total = 0;                     // �^����ꂽ���іڂ̐�
static double (*src)[3];                  // �^����ꂽ���іڂ̈ʒu�̌��f�[�^
static GLdouble (*dst)[3] = 0;            // �^����ꂽ���іڂ̈ʒu�̃R�s�[
static bool *fix = 0;                     // ���іڂ��Œ肳��Ă��邱�Ƃ������}�[�N
static GLfloat (*col)[3] = 0;             // ���іڂ̐F
static double tstep;                      // �o�ߎ��� (�b)

static const GLfloat cfix[] = { 0.0f, 0.0f, 1.0f };   // �Œ�_�̐F
static const GLfloat cmov[] = { 1.0f, 0.0f, 0.0f };   // �ړ��_�̐F

#define NETSIZE 5.0                     // �Ԃ̑傫��
#define NETHEIGHT 5.0                   // �Ԃ̍���

//
// �Ԃ�`��
//
//     m: �������̌��іڂ̐� (�����̐� + 1)
//     n: �c�����̌��іڂ̐� (�����̐� + 1)
//     p: ���іڂ̈ʒu (�O����)
//
void net(int m, int n, void *p)
{
  // ���f�[�^�̃|�C���^���o���Ă���
  src = reinterpret_cast<GLdouble (*)[3]>(p);

  if (m * n != total)
  {
    total = m * n;

    // ���������m�ۂ���
    delete[] fix;
    fix = new bool[total];
    delete[] dst;
    dst = new GLdouble[total][3];
    delete[] col;
    col = new GLfloat[total][3];

    // �Ԃ̒��S�ʒu�����߂�
    double hc = (double)(m - 1) * 0.5;
    double vc = (double)(n - 1) * 0.5;

    // ���іڂɏ����l��ݒ肷��
    for (int i = 0; i < total; ++i)
    {
      fix[i] = false;
      src[i][0] = dst[i][0] = NETSIZE * ((double)(i % m) - hc) / hc;
      src[i][1] = dst[i][1] = NETHEIGHT;
      src[i][2] = dst[i][2] = NETSIZE * ((double)(i / m) - vc) / vc;
      col[i][0] = cmov[0];
      col[i][1] = cmov[1];
      col[i][2] = cmov[2];
    }

    // �l�����Œ肷��
    fix[0] = true;
    col[0][0] = cfix[0];
    col[0][1] = cfix[1];
    col[0][2] = cfix[2];
    fix[m - 1] = true;
    col[m - 1][0] = cfix[0];
    col[m - 1][1] = cfix[1];
    col[m - 1][2] = cfix[2];
    fix[m * n - m] = true;
    col[m * n - m][0] = cfix[0];
    col[m * n - m][1] = cfix[1];
    col[m * n - m][2] = cfix[2];
    fix[m * n - 1] = true;
    col[m * n - 1][0] = cfix[0];
    col[m * n - 1][1] = cfix[1];
    col[m * n - 1][2] = cfix[2];
  }
  else
  {
    for (int i = 0; i < total; ++i)
    {
      if (fix[i])
      {
        // �Œ肳�ꂽ���іڂ̈ʒu���Œ�ʒu�ɖ߂�
        reinterpret_cast<GLdouble (*)[3]>(p)[i][0] = dst[i][0];
        reinterpret_cast<GLdouble (*)[3]>(p)[i][1] = dst[i][1];
        reinterpret_cast<GLdouble (*)[3]>(p)[i][2] = dst[i][2];
      }
      else
      {
        // �Œ肳��Ă��Ȃ����іڂ̈ʒu���X�V����
        dst[i][0] = reinterpret_cast<GLdouble (*)[3]>(p)[i][0];
        dst[i][1] = reinterpret_cast<GLdouble (*)[3]>(p)[i][1];
        dst[i][2] = reinterpret_cast<GLdouble (*)[3]>(p)[i][2];
      }
    }
  }

  // ���̐F
  glDisableClientState(GL_COLOR_ARRAY);
  glColor3d(0.0, 0.0, 0.0);

  // �c���̕`��
  for (int i = 0; i < m; ++i)
  {
    glVertexPointer(3, GL_DOUBLE, sizeof dst[0] * m, dst + i);
    glDrawArrays(GL_LINE_STRIP, 0, n);
  }

  // �����̕`��
  for (int i = 0; i < n; ++i)
  {
    glVertexPointer(3, GL_DOUBLE, 0, dst + m * i);
    glDrawArrays(GL_LINE_STRIP, 0, m);
  }

  // ���іڂ̕`��
  glEnableClientState(GL_COLOR_ARRAY);
  glVertexPointer(3, GL_DOUBLE, 0, dst);
  glColorPointer(3, GL_FLOAT, 0, col);
  glDrawArrays(GL_POINTS, 0, total);
}

//
// �o�ߎ��Ԃ𓾂� (�b)
//
double elapsed(void)
{
  return tstep;
}

////////////////////////////////////////////////////////////////////////

#define KNOTSIZE  7.0                     // ���іڂ̑傫��

static GLdouble model[16];                // ���f���r���[�s��̕ۑ��p
static GLdouble proj[16];                 // ���e�ϊ��s��̕ۑ��p
static GLint view[4];                     // �r���[�|�[�g�ϊ��s��̕ۑ��p

static gg::GgTrackball tb;                // �g���b�N�{�[������

//
// polyline() ���g���Đ}�`��`���葱��
//
extern void draw(void);

//
// ���ۂ̕`��
//
static void display(void)
{
  // �o�ߎ��Ԃ����߂�
  static int last = 0;
  int now = glutGet(GLUT_ELAPSED_TIME);
  tstep = (last == 0) ? 0.0 : (double)(now - last) * 0.001;
  last = now;

  // ���_�̈ʒu��ݒ肷��
  glLoadIdentity();
  gluLookAt(6.0, 2.0, 20.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
  glMultMatrixf(tb.get());

  // �}�`��\������
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  draw();
  glutSwapBuffers();
}

void resize(int w, int h)
{
  // �E�B���h�E�S�̂��r���[�|�[�g�ɂ���
  glViewport(0, 0, w, h);

  // ���e�ϊ��s��̏�����
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(40.0, (GLdouble)w / (GLdouble)h, 1.0, 30.0);

  // ���f���r���[�ϊ��s��̑I��
  glMatrixMode(GL_MODELVIEW);

  // �g���b�N�{�[������͈�
  tb.region(w, h);
}

//
// �s�b�N����
//
static int pick(const GLdouble (*k)[3], int count, int x, int y, GLdouble *z)
{
  // �����_�̕ϊ��s���ۑ�����
  glGetDoublev(GL_MODELVIEW_MATRIX, model);
  glGetDoublev(GL_PROJECTION_MATRIX, proj);
  glGetIntegerv(GL_VIEWPORT, view);

  // �s�b�N�����X�N���[����̈ʒu
  GLdouble ux = x;
  GLdouble uy = view[3] - y;

  // �s�b�N�����ʒu�̉��s���̍ő�l
  GLdouble zmin = 1000.0;

  int hit = -1;

  for (int i = 0; i < count; ++i)
  {
    GLdouble wx, wy, wz;

    // ���̓_�̃X�N���[����̈ʒu�����߂�
    gluProject(k[i][0], k[i][1], k[i][2], model, proj, view, &wx, &wy, &wz);
    if (fabs(ux - wx) <= KNOTSIZE && fabs(uy - wy) <= KNOTSIZE && wz < zmin)
    {
      // �}�E�X�ŃN���b�N�����ʒu�ɋ߂���΂��̓_��ۑ�����
      zmin = wz;
      hit = i;
    }
  }

  *z = zmin;
  return hit;
}

static int press = -1;  // �������}�E�X�{�^��
static int hit = -1;    // �I���������іڂ̔ԍ�
static GLdouble cz;     // �I���������іڂ̉��s��

//
// �}�E�X�{�^���̃N���b�N
//
static void mouse(int button, int state, int x, int y)
{
  // �h���b�O���̌��іڂ̈ȑO�̌Œ���
  static bool fstatus = false;

  // ���삵���{�^�����o���Ă���
  press = button;

  switch (button) {
  case GLUT_LEFT_BUTTON:
    if (state == GLUT_DOWN)
    {
      // �}�E�X�ʒu�̌��іڂ�T��
      hit = pick(dst, total, x, y, &cz);

      if (hit >= 0)
      {
        // ���̌��іڂ��Œ肳��Ă��邩�ǂ������L������
        fstatus = fix[hit];

        // ���̌��іڂ��Œ肷��
        fix[hit] = true;
      }
    }
    else
    {
      if (hit >= 0)
      {
        // �h���b�O���Ă������іڂ̌Œ��Ԃ�߂�
        fix[hit] = fstatus;
        hit = -1;
      }
    }
    break;
  case GLUT_MIDDLE_BUTTON:
    break;
  case GLUT_RIGHT_BUTTON:
    if (state == GLUT_DOWN)
    {
      // �}�E�X�ʒu�̌��іڂ�T��
      hit = pick(dst, total, x, y, &cz);

      if (hit < 0)
      {
        // �g���b�N�{�[���J�n
        tb.start(x, y);
      }
      else
      {
        // ���іڂ�����΂��̏�Ԃ𔽓]����
        fix[hit] = !fix[hit];
        if (fix[hit])
        {
          // �Œ�_�̐F
          col[hit][0] = cfix[0];
          col[hit][1] = cfix[1];
          col[hit][2] = cfix[2];
        }
        else
        {
          // �ړ��_�̐F
          col[hit][0] = cmov[0];
          col[hit][1] = cmov[1];
          col[hit][2] = cmov[2];

          // �ړ��_�̈ʒu
          src[hit][0] = dst[hit][0];
          src[hit][1] = dst[hit][1];
        }
      }
    }
    else
    {
      if (hit < 0)
      {
        // �g���b�N�{�[����~
        tb.stop(x, y);
      }
      else
      {
        // ���іڂ̑I������������
        hit = -1;
      }
    }
    break;
  default:
    break;
  }
}

//
//�}�E�X�̃h���b�O
//
static void motion(int x, int y)
{
  switch (press)
  {
  case GLUT_LEFT_BUTTON:
    // ���іڂ̃h���b�O
    if (hit >= 0)
    {
      // ���іڂ�����ł�����O�����ʒu�����߂�
      gluUnProject((GLdouble)x, (GLdouble)(view[3] - y), cz,
        model, proj, view, dst[hit], dst[hit] + 1, dst[hit] + 2);
    }
    break;
  case GLUT_MIDDLE_BUTTON:
    break;
  case GLUT_RIGHT_BUTTON:
    if (hit < 0)
    {
      // �g���b�N�{�[���ړ�
      tb.motion(x, y);
    }
    break;
  default:
    break;
  }
}

//
// �L�[�{�[�h�̑���
//
static void keyboard(unsigned char key, int x, int y)
{
  switch (key)
  {
  case '\033':
  case 'q':
  case 'Q':
    exit(0);
  default:
    break;
  }
}

//
// ��ʂ̍ŕ`��
//
static void idle(void)
{
  glutPostRedisplay();
}

//
// �����ݒ�
//
static void init(void)
{
  gg::ggInit();
  glClearColor(1.0, 1.0, 1.0, 0.0);
  glPointSize(KNOTSIZE);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnable(GL_DEPTH_TEST);
}

int main(int argc, char *argv[])
{
  glutInitWindowSize(800, 600);
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
  glutCreateWindow("Biyooooooooon!");
  glutDisplayFunc(display);
  glutReshapeFunc(resize);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);
  glutKeyboardFunc(keyboard);
  glutIdleFunc(idle);
  init();
  glutMainLoop();

  return 0;
}
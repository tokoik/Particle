#include <cmath>
#include <cstdio>
#include <cstdlib>
#include "gg.h"

static int total = 0;                     // 与えられた結び目の数
static double (*src)[3];                  // 与えられた結び目の位置の元データ
static GLdouble (*dst)[3] = 0;            // 与えられた結び目の位置のコピー
static bool *fix = 0;                     // 結び目が固定されていることを示すマーク
static GLfloat (*col)[3] = 0;             // 結び目の色
static double tstep;                      // 経過時間 (秒)

static const GLfloat cfix[] = { 0.0f, 0.0f, 1.0f };   // 固定点の色
static const GLfloat cmov[] = { 1.0f, 0.0f, 0.0f };   // 移動点の色

#define NETSIZE 5.0                     // 網の大きさ
#define NETHEIGHT 5.0                   // 網の高さ

//
// 網を描く
//
//     m: 横方向の結び目の数 (線分の数 + 1)
//     n: 縦方向の結び目の数 (線分の数 + 1)
//     p: 結び目の位置 (三次元)
//
void net(int m, int n, void *p)
{
  // 元データのポインタを覚えておく
  src = reinterpret_cast<GLdouble (*)[3]>(p);

  if (m * n != total)
  {
    total = m * n;

    // メモリを確保する
    delete[] fix;
    fix = new bool[total];
    delete[] dst;
    dst = new GLdouble[total][3];
    delete[] col;
    col = new GLfloat[total][3];

    // 網の中心位置を求める
    double hc = (double)(m - 1) * 0.5;
    double vc = (double)(n - 1) * 0.5;

    // 結び目に初期値を設定する
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

    // 四隅を固定する
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
        // 固定された結び目の位置を固定位置に戻す
        reinterpret_cast<GLdouble (*)[3]>(p)[i][0] = dst[i][0];
        reinterpret_cast<GLdouble (*)[3]>(p)[i][1] = dst[i][1];
        reinterpret_cast<GLdouble (*)[3]>(p)[i][2] = dst[i][2];
      }
      else
      {
        // 固定されていない結び目の位置を更新する
        dst[i][0] = reinterpret_cast<GLdouble (*)[3]>(p)[i][0];
        dst[i][1] = reinterpret_cast<GLdouble (*)[3]>(p)[i][1];
        dst[i][2] = reinterpret_cast<GLdouble (*)[3]>(p)[i][2];
      }
    }
  }

  // 線の色
  glDisableClientState(GL_COLOR_ARRAY);
  glColor3d(0.0, 0.0, 0.0);

  // 縦線の描画
  for (int i = 0; i < m; ++i)
  {
    glVertexPointer(3, GL_DOUBLE, sizeof dst[0] * m, dst + i);
    glDrawArrays(GL_LINE_STRIP, 0, n);
  }

  // 横線の描画
  for (int i = 0; i < n; ++i)
  {
    glVertexPointer(3, GL_DOUBLE, 0, dst + m * i);
    glDrawArrays(GL_LINE_STRIP, 0, m);
  }

  // 結び目の描画
  glEnableClientState(GL_COLOR_ARRAY);
  glVertexPointer(3, GL_DOUBLE, 0, dst);
  glColorPointer(3, GL_FLOAT, 0, col);
  glDrawArrays(GL_POINTS, 0, total);
}

//
// 経過時間を得る (秒)
//
double elapsed(void)
{
  return tstep;
}

////////////////////////////////////////////////////////////////////////

#define KNOTSIZE  7.0                     // 結び目の大きさ

static GLdouble model[16];                // モデルビュー行列の保存用
static GLdouble proj[16];                 // 投影変換行列の保存用
static GLint view[4];                     // ビューポート変換行列の保存用

static gg::GgTrackball tb;                // トラックボール処理

//
// polyline() を使って図形を描く手続き
//
extern void draw(void);

//
// 実際の描画
//
static void display(void)
{
  // 経過時間を求める
  static int last = 0;
  int now = glutGet(GLUT_ELAPSED_TIME);
  tstep = (last == 0) ? 0.0 : (double)(now - last) * 0.001;
  last = now;

  // 視点の位置を設定する
  glLoadIdentity();
  gluLookAt(6.0, 2.0, 20.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
  glMultMatrixf(tb.get());

  // 図形を表示する
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  draw();
  glutSwapBuffers();
}

void resize(int w, int h)
{
  // ウィンドウ全体をビューポートにする
  glViewport(0, 0, w, h);

  // 投影変換行列の初期化
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(40.0, (GLdouble)w / (GLdouble)h, 1.0, 30.0);

  // モデルビュー変換行列の選択
  glMatrixMode(GL_MODELVIEW);

  // トラックボールする範囲
  tb.region(w, h);
}

//
// ピック処理
//
static int pick(const GLdouble (*k)[3], int count, int x, int y, GLdouble *z)
{
  // 現時点の変換行列を保存する
  glGetDoublev(GL_MODELVIEW_MATRIX, model);
  glGetDoublev(GL_PROJECTION_MATRIX, proj);
  glGetIntegerv(GL_VIEWPORT, view);

  // ピックしたスクリーン上の位置
  GLdouble ux = x;
  GLdouble uy = view[3] - y;

  // ピックした位置の奥行きの最大値
  GLdouble zmin = 1000.0;

  int hit = -1;

  for (int i = 0; i < count; ++i)
  {
    GLdouble wx, wy, wz;

    // その点のスクリーン上の位置を求める
    gluProject(k[i][0], k[i][1], k[i][2], model, proj, view, &wx, &wy, &wz);
    if (fabs(ux - wx) <= KNOTSIZE && fabs(uy - wy) <= KNOTSIZE && wz < zmin)
    {
      // マウスでクリックした位置に近ければその点を保存する
      zmin = wz;
      hit = i;
    }
  }

  *z = zmin;
  return hit;
}

static int press = -1;  // 押したマウスボタン
static int hit = -1;    // 選択した結び目の番号
static GLdouble cz;     // 選択した結び目の奥行き

//
// マウスボタンのクリック
//
static void mouse(int button, int state, int x, int y)
{
  // ドラッグ中の結び目の以前の固定状態
  static bool fstatus = false;

  // 操作したボタンを覚えておく
  press = button;

  switch (button) {
  case GLUT_LEFT_BUTTON:
    if (state == GLUT_DOWN)
    {
      // マウス位置の結び目を探す
      hit = pick(dst, total, x, y, &cz);

      if (hit >= 0)
      {
        // その結び目が固定されているかどうかを記憶する
        fstatus = fix[hit];

        // その結び目を固定する
        fix[hit] = true;
      }
    }
    else
    {
      if (hit >= 0)
      {
        // ドラッグしていた結び目の固定状態を戻す
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
      // マウス位置の結び目を探す
      hit = pick(dst, total, x, y, &cz);

      if (hit < 0)
      {
        // トラックボール開始
        tb.start(x, y);
      }
      else
      {
        // 結び目があればその状態を反転する
        fix[hit] = !fix[hit];
        if (fix[hit])
        {
          // 固定点の色
          col[hit][0] = cfix[0];
          col[hit][1] = cfix[1];
          col[hit][2] = cfix[2];
        }
        else
        {
          // 移動点の色
          col[hit][0] = cmov[0];
          col[hit][1] = cmov[1];
          col[hit][2] = cmov[2];

          // 移動点の位置
          src[hit][0] = dst[hit][0];
          src[hit][1] = dst[hit][1];
        }
      }
    }
    else
    {
      if (hit < 0)
      {
        // トラックボール停止
        tb.stop(x, y);
      }
      else
      {
        // 結び目の選択を解除する
        hit = -1;
      }
    }
    break;
  default:
    break;
  }
}

//
//マウスのドラッグ
//
static void motion(int x, int y)
{
  switch (press)
  {
  case GLUT_LEFT_BUTTON:
    // 結び目のドラッグ
    if (hit >= 0)
    {
      // 結び目をつかんでいたら三次元位置を求める
      gluUnProject((GLdouble)x, (GLdouble)(view[3] - y), cz,
        model, proj, view, dst[hit], dst[hit] + 1, dst[hit] + 2);
    }
    break;
  case GLUT_MIDDLE_BUTTON:
    break;
  case GLUT_RIGHT_BUTTON:
    if (hit < 0)
    {
      // トラックボール移動
      tb.motion(x, y);
    }
    break;
  default:
    break;
  }
}

//
// キーボードの操作
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
// 画面の最描画
//
static void idle(void)
{
  glutPostRedisplay();
}

//
// 初期設定
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

#pragma once
#include <array>
#include <GL/glew.h>

//
// 点データ
//
class Point
{
  // 頂点配列オブジェクト名
  GLuint vao;

  // 頂点バッファオブジェクト名
  GLuint vbo;

  // 頂点の数
  const GLsizei vertexcount;

public:

  // 頂点属性
  struct Vertex
  {
    // 位置
    GLfloat position[3];

    // 速度
    GLfloat velocity[3];

    // 力
    GLfloat force[3];

    // デフォルトコンストラクタ
    //   デフォルトコンストラクタは初期値を伴わない変数宣言の時に呼び出されます．
    //   コンストラクタの本体では何もしません．
    Vertex()
    {}

    // 引数で初期化を行うコンストラクタ
    //   初期値を引数で指定するときに呼び出されます.
    //   position, velocity, force を引数で初期化するほか,速度と力の引数を省略したときは,verocityとforceを0で初期化します.
    //   const でないメンバはコンストラクタの本体でも値を代入できるけど初期化なのでここで行います.
    //   コンストラクタの本体では何もしません．
    Vertex(float px, float py, float pz,
      float vx = 0.0f, float vy = 0.0f, float vz = 0.0f,
      float fx = 0.0f, float fy = 0.0f, float fz = 0.0f)
      : position{ px, py, pz }, velocity{ vx, vy, vz }, force{ fx,fy,fz }
    {}

    // ソートに用いる比較関数
    //   データが昇順の時真になる
    bool operator<(const Vertex &vertex)
    {
      return position[2] < vertex.position[2];
    }
  };

  // コンストラクタ
  //   vertexcount: 頂点の数
  //   vertex: 頂点属性を格納した配列
  //   indexcount: 頂点のインデックスの要素数
  //   index: 頂点のインデックスを格納した配列
  Point(GLsizeiptr vertexcount, const Vertex *vertex)
    : vertexcount(static_cast<GLsizei>(vertexcount))
  {
    // 頂点配列オブジェクト
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // 頂点バッファオブジェクト
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
      vertexcount * sizeof(Vertex), vertex, GL_STATIC_DRAW);

    // 結合されている頂点バッファオブジェクトを in 変数から参照できるようにする
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
      static_cast<const Vertex *>(0)->position);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
      static_cast<const Vertex *>(0)->velocity);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
      static_cast<const Vertex *>(0)->force);
    glEnableVertexAttribArray(2);
  }

  // デストラクタ
  virtual ~Point()
  {
    // 頂点配列オブジェクトを削除する
    glDeleteBuffers(1, &vao);

    // 頂点バッファオブジェクトを削除する
    glDeleteBuffers(1, &vbo);
  }

private:

  // コピーコンストラクタによるコピー禁止
  Point(const Point &o);

  // 代入によるコピー禁止
  Point &operator=(const Point &o);

public:

  // 描画
  void draw(const Vertex *vertex = nullptr) const
  {
    // 描画する頂点配列オブジェクトを指定する
    glBindVertexArray(vao);

    // 引数が与えられていたら頂点バッファオブジェクトを更新する
    if (vertex) glBufferSubData(GL_ARRAY_BUFFER, 0, vertexcount * sizeof(Vertex), vertex);

    // 点で描画する
    glDrawArrays(GL_POINTS, 0, vertexcount);
  }
};

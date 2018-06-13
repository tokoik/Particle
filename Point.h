#pragma once
#include <array>
#include <GL/glew.h>

//
// �_�f�[�^
//
class Point
{
  // ���_�z��I�u�W�F�N�g��
  GLuint vao;

  // ���_�o�b�t�@�I�u�W�F�N�g��
  GLuint vbo;

  // ���_�̐�
  const GLsizei vertexcount;

public:

  // ���_����
  struct Vertex
  {
    // �ʒu
    GLfloat position[3];

    // ���x
    GLfloat velocity[3];

    // ��
    GLfloat force[3];

    // �f�t�H���g�R���X�g���N�^
    //   �f�t�H���g�R���X�g���N�^�͏����l�𔺂�Ȃ��ϐ��錾�̎��ɌĂяo����܂��D
    //   �R���X�g���N�^�̖{�̂ł͉������܂���D
    Vertex()
    {}

    // �����ŏ��������s���R���X�g���N�^
    //   �����l�������Ŏw�肷��Ƃ��ɌĂяo����܂�.
    //   position, velocity, force �������ŏ���������ق�,���x�Ɨ͂̈������ȗ������Ƃ���,verocity��force��0�ŏ��������܂�.
    //   const �łȂ������o�̓R���X�g���N�^�̖{�̂ł��l�����ł��邯�Ǐ������Ȃ̂ł����ōs���܂�.
    //   �R���X�g���N�^�̖{�̂ł͉������܂���D
    Vertex(float px, float py, float pz,
      float vx = 0.0f, float vy = 0.0f, float vz = 0.0f,
      float fx = 0.0f, float fy = 0.0f, float fz = 0.0f)
      : position{ px, py, pz }, velocity{ vx, vy, vz }, force{ fx,fy,fz }
    {}

    // �\�[�g�ɗp�����r�֐�
    //   �f�[�^�������̎��^�ɂȂ�
    bool operator<(const Vertex &vertex)
    {
      return position[2] < vertex.position[2];
    }
  };

  // �R���X�g���N�^
  //   vertexcount: ���_�̐�
  //   vertex: ���_�������i�[�����z��
  //   indexcount: ���_�̃C���f�b�N�X�̗v�f��
  //   index: ���_�̃C���f�b�N�X���i�[�����z��
  Point(GLsizeiptr vertexcount, const Vertex *vertex)
    : vertexcount(static_cast<GLsizei>(vertexcount))
  {
    // ���_�z��I�u�W�F�N�g
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // ���_�o�b�t�@�I�u�W�F�N�g
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
      vertexcount * sizeof(Vertex), vertex, GL_STATIC_DRAW);

    // ��������Ă��钸�_�o�b�t�@�I�u�W�F�N�g�� in �ϐ�����Q�Ƃł���悤�ɂ���
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

  // �f�X�g���N�^
  virtual ~Point()
  {
    // ���_�z��I�u�W�F�N�g���폜����
    glDeleteBuffers(1, &vao);

    // ���_�o�b�t�@�I�u�W�F�N�g���폜����
    glDeleteBuffers(1, &vbo);
  }

private:

  // �R�s�[�R���X�g���N�^�ɂ��R�s�[�֎~
  Point(const Point &o);

  // ����ɂ��R�s�[�֎~
  Point &operator=(const Point &o);

public:

  // �`��
  void draw(const Vertex *vertex = nullptr) const
  {
    // �`�悷�钸�_�z��I�u�W�F�N�g���w�肷��
    glBindVertexArray(vao);

    // �������^�����Ă����璸�_�o�b�t�@�I�u�W�F�N�g���X�V����
    if (vertex) glBufferSubData(GL_ARRAY_BUFFER, 0, vertexcount * sizeof(Vertex), vertex);

    // �_�ŕ`�悷��
    glDrawArrays(GL_POINTS, 0, vertexcount);
  }
};
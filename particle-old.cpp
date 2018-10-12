#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "shader.h"
#include "Window.h"
#include "Matrix.h"
#include "Vector.h"
#include "Point.h"

// 乱数
#include <random>

// OpenGL のエラーチェック

GLenum _errorCheck(int line)
{
	const GLenum errCode(glGetError());

	if (errCode != GL_NO_ERROR)
	{
		std::cerr << line << ": ";

		switch (errCode)
		{
		case GL_INVALID_ENUM:
			std::cerr << "An unacceptable value is specified for an enumerated argument.The offending command is ignored and has no other side effect than to set the error flag.\n";
			break;
		case GL_INVALID_VALUE:
			std::cerr << "A numeric argument is out of range. The offending command is ignored and has no other side effect than to set the error flag.\n";
			break;
		case GL_INVALID_OPERATION:
			std::cerr << "The specified operation is not allowed in the current state. The offending command is ignored and has no other side effect than to set the error flag.\n";
			break;
		case GL_STACK_OVERFLOW:
			std::cerr << "This command would cause a stack overflow. The offending command is ignored and has no other side effect than to set the error flag.\n";
			break;
		case GL_STACK_UNDERFLOW:
			std::cerr << "This command would cause a stack underflow. The offending command is ignored and has no other side effect than to set the error flag.\n";
			break;
		case GL_OUT_OF_MEMORY:
			std::cerr << "There is not enough memory left to execute the command. The state of the GL is undefined, except for the state of the error flags, after this error is recorded.\n";
			break;
		case GL_TABLE_TOO_LARGE:
			std::cerr << "The specified table exceeds the implementation's maximum supported table size. The offending command is ignored and has no other side effect than to set the error flag.\n";
			break;
		default:
			break;
		}
	}
	return errCode;
}

#define errorCheck() _errorCheck(__LINE__)

// 雲の粒子の基準半径
constexpr float cloudRadius(0.1f);

// 点群 
//これを宣言することでstd::vector<Point::Vertex>を毎回書かなくて良くなる
//std::vector<Point::Vertex> rectangleVertex;はPoints rectangleVertecx;て書けるようになる
typedef std::vector<Point::Vertex> Points;

// 点群の生成 (Points &points, int count,中心からどれだけ動かすかx方向,y方向,z方向,平均, 標準偏差)
static void generatePoints(Points &points, int count, GLfloat cx, GLfloat cy, GLfloat cz, GLfloat mean, GLfloat deviation)
{
	// ハードウェア乱数で種を作る
	std::random_device seed;

	// メルセンヌツイスター法による乱数の系列を設定する
	//std::mt19937 rn(seed());
	std::mt19937 rn(12345);

	// 一様実数分布
	//   [0, 2) の値の範囲で等確率に実数を生成する
	std::uniform_real_distribution<GLfloat> uniform(0.0f, 2.0f);

	// 正規分布
	//   平均 mean、標準偏差 deviation で分布させる
	std::normal_distribution<GLfloat> normal(mean, deviation);

	// 原点中心に直径方向に正規分布する点群を発生する
	while (--count >= 0)
	{
		// 緯度方向
		const GLfloat cp(uniform(rn) - 1.0f);
		const GLfloat sp(sqrt(1.0f - cp * cp));

		// 経度方向
		const GLfloat t(3.1415927f * uniform(rn));
		const GLfloat ct(cos(t)), st(sin(t));

		// 中心からの距離(半径)
		const GLfloat r(normal(rn));

		// 点を追加する
		points.emplace_back(r * sp * ct + cx, r * sp * st + cy, r * cp + cz, 0.0f, 0.0f, 0.0f);
	}
}

int main()
{
	// GLFW を初期化する
	if (glfwInit() == GL_FALSE)
	{
		// 初期化に失敗した
		std::cerr << "Can't initialize GLFW" << std::endl;
		return 1;
	}

	// プログラム終了時の処理を登録する
	//atexit(glfwTerminate);

	// OpenGL Version 3.2 Core Profile を選択する
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// ウィンドウを作成する
	Window window;

	// 背景色を指定する
	glClearColor(0.2f, 0.3f, 0.5f, 1.0f);
	
	// 背面カリングを有効にする
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	// デプスバッファを有効にする
	glClearDepth(1.0);
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);

	//ポイントスプライトの設定//点のサイズはシェーダから設定する
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

	// アルファブレンディングを設定する
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// プログラムオブジェクトの作成
	const GLuint pointShader(loadProgram("point.vert","point.frag"));

	// in (attribute) 変数のインデックスの検索（見つからなければ -1）
	const GLint nvLoc(glGetAttribLocation(pointShader, "nv"));
	const GLint tvLoc(glGetAttribLocation(pointShader, "tv"));

	// uniform 変数のインデックスの検索（見つからなければ -1）
	const GLint mwLoc(glGetUniformLocation(pointShader, "mw"));
	const GLint mcLoc(glGetUniformLocation(pointShader, "mc"));
	const GLint mgLoc(glGetUniformLocation(pointShader, "mg"));

	const GLint pvLoc(glGetUniformLocation(pointShader, "pv"));
	const GLint fcLoc(glGetUniformLocation(pointShader, "fc"));

	// uniform 変数の場所を取得する雲用
	const GLint pointSizeLoc(glGetUniformLocation(pointShader, "size"));

	// ビュー変換行列を求める
	const Matrix mv(Matrix::lookat(0.0f, 0.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f));

	//
	// 雲の玉の中心も乱数で発生させる
	//   メルセンヌツイスター法による乱数の系列を設定する
	std::mt19937 rn(54321);

	// 一様実数分布
	//   [-0.5,0.5)の値の範囲で等確率に実数を生成する
	std::uniform_real_distribution<GLfloat> center(-0.5f, 0.5f);

	// 点データ
	Points points;

	//発生する玉の数だけ繰り返す
	//雲の玉の中心位置を4個一様乱数で発生して，そこを中心に雲の玉を生成する
	for (int i = 0; i < 4; i++)
	{
		//雲の玉中心位置
		const GLfloat cx(center(rn)), cy(center(rn)), cz(center(rn));

		//中心からの距離に対して密度が正規分布に従う雲の玉を生成する
		generatePoints(points, 30, cx, cy, cz, 0.0f, 0.75f);
	}

	// 図形データを作成する
	std::unique_ptr<const Point> cloud(new Point(points.size(), points.data()));

	// 時計をリセットする
	glfwSetTime(0.0);


	errorCheck();

	// ウィンドウが開いている間繰り返す
	while (window.shouldClose() == GL_FALSE)
	{
		// 点の位置を並べ替える
		std::sort(points.begin(), points.end());


		// 点のシェーダプログラムの使用開始
		glUseProgram(pointShader);

		// 透視投影変換行列を求める
		const GLfloat *const size(window.getSize());
		const GLfloat fovy(window.getScale() * 0.01f);
		const GLfloat aspect(size[0] / size[1]);
		const Matrix projection(Matrix::perspective(fovy, aspect, 1.0f, 10.0f));

		// モデル変換行列を求める
		const GLfloat *const location(window.getLocation());
		const Matrix model(Matrix::translate(location[0] * 2.0f, location[1] * 2.0f, 0.0f));

		// モデルビュー変換行列を求める
		const Matrix mw(mv * model);

		// 法線変換行列
		const Matrix mg(mw.normal());

		// 投影変換行列
		const Matrix mp(Matrix::perspective(0.5f, window.getAspect(), 1.0f, 15.0f));

		// モデルビュー・投影変換
		const Matrix mc(mp * mw);



		// デプスバッファへの書き込みを有効にする
		glDepthMask(GL_TRUE);

		// ウィンドウを消去する
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



				// uniform 変数を設定する
		glUniformMatrix4fv(mwLoc, 1, GL_FALSE, mw.data());
		glUniformMatrix4fv(mcLoc, 1, GL_FALSE, mc.data());
		glUniformMatrix4fv(mgLoc, 1, GL_FALSE, mg.data());

		// uniform 変数に値を設定する
		glUniformMatrix4fv(pvLoc, 1, GL_FALSE, projection.data());
		glUniformMatrix4fv(mcLoc, 1, GL_FALSE, mw.data());

		// 雲の粒子の大きさは画面の高さに比例させる
		glUniform1f(pointSizeLoc, size[1] * cloudRadius);

		// デプスバッファへの書き込みを無効にする
		glDepthMask(GL_FALSE);

		// アルファブレンディングを有効にして点を描画する
		glEnable(GL_BLEND);
		cloud->draw(points.data());

		// カラーバッファを入れ替えてイベントを取り出す
		window.swapBuffers();

	}
}
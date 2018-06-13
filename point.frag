#version 150 core
in vec4 vc;
// フレームバッファに出力するデータ
out vec4 fc;                                        // フラグメントの色
out vec4 fragment;

void main()
{
	vec3 n;

	n.xy = gl_PointCoord * 2.0 - 1.0; // x,y座標にvec2型の変数gl_PointCoordを代入(座標値を[0,1]→[-1,1]に変換する

	//z座標を求める(球の方程式x^2+y^2+z^2=1より)
	n.z = sqrt(1 - n.x * n.x - n.y * n.y);
	gl_FragDepth = n.z;
	
	float r = length(n.xy);
	if (r >= 1.0) discard; //1.0以上を描画しない（点を円状にする） 
	
	float c = smoothstep(1.0, 0.0, r); //半透明にする
	fragment = vec4(1.0, 1.0, 1.0, c); //(r,g,b,α)

	fc = vc;
	
}


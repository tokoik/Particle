#version 150 core
in vec4 vc;
// �t���[���o�b�t�@�ɏo�͂���f�[�^
out vec4 fc;                                        // �t���O�����g�̐F
out vec4 fragment;

void main()
{
	vec3 n;

	n.xy = gl_PointCoord * 2.0 - 1.0; // x,y���W��vec2�^�̕ϐ�gl_PointCoord����(���W�l��[0,1]��[-1,1]�ɕϊ�����

	//z���W�����߂�(���̕�����x^2+y^2+z^2=1���)
	n.z = sqrt(1 - n.x * n.x - n.y * n.y);
	gl_FragDepth = n.z;
	
	float r = length(n.xy);
	if (r >= 1.0) discard; //1.0�ȏ��`�悵�Ȃ��i�_���~��ɂ���j 
	
	float c = smoothstep(1.0, 0.0, r); //�������ɂ���
	fragment = vec4(1.0, 1.0, 1.0, c); //(r,g,b,��)

	fc = vc;
	
}

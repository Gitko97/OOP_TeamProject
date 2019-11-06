////////////////////////////////////////////////////////////////////////////////
//
// File: virtualLego.cpp
//
// Original Author: ��â�� Chang-hyeon Park, 
// Modified by Bong-Soo Sohn and Dong-Jun Kim
// 
// Originally programmed for Virtual LEGO. 
// Modified later to program for Virtual Billiard.
//        
////////////////////////////////////////////////////////////////////////////////

#include "d3dUtility.h"
#include <vector>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <cassert>

IDirect3DDevice9* Device = NULL;

// window size
const int Width  = 1024;
const int Height = 768;
const int numberOfBall = 11;
// There are four balls
// initialize the position (coordinate) of each ball (ball0 ~ ball3)
const float spherePos[numberOfBall][2] = { {-2.7f,0} , {1.7f,-0.2f} , {0.5f,0}, {0.9f,-0.2f}, {0.9f,0.2f}, {1.3f,0.0f}, {1.3f,0.4f} , {1.3f,-0.4f},{1.7f,-0.6f},{1.7f,0.2f},{1.7f,0.6f} };
// initialize the color of each ball (ball0 ~ ball3)
const D3DXCOLOR sphereColor[numberOfBall] = {d3d::WHITE, d3d::BLACK, d3d::YELLOW,d3d::YELLOW, d3d::RED,d3d::RED,d3d::GREEN,d3d::GREEN,d3d::CYAN,d3d::CYAN,d3d::DARKRED };
const char* system_msg[] = {"Player1�� �����Դϴ�!","����� �����ּ���","Player1 :","Player2 :","Player2�� �����Դϴ�!" , "Player1�� �¸�!","Player2�� �¸�!" };
const int textPos[7][2] = { {20,20},{40,40},{20,70},{20,90},{105,70},{105,90},{0,0} };  //Text�� ��ġ
const int textSize[2][3] = { {20,10,600},{60,40,600} }; //Text Box�� ũ�� �� �۾� �β�
// -----------------------------------------------------------------------------
// Transform matrices
// -----------------------------------------------------------------------------
D3DXMATRIX g_mWorld;
D3DXMATRIX g_mView;
D3DXMATRIX g_mProj;

#define M_RADIUS 0.19   // ball radius
#define H_RADIUS 0.25 
#define PI 3.14159265
#define M_HEIGHT 0.01
#define DECREASE_RATE 0.9982
const float holePos[6][2] = { {-4.5 + H_RADIUS,3 - H_RADIUS},{-4.5 + H_RADIUS,-3 + H_RADIUS},{0,3}, {0,-3}, {4.5 - H_RADIUS,3 - H_RADIUS},{4.5 - H_RADIUS,-3 + H_RADIUS} };
// -----------------------------------------------------------------------------
// CSphere class definition
// -----------------------------------------------------------------------------

class PlayerTurn { //player�� ��
private :
	int whosTurn ;		 //������ ������ -1�� p2 1�� p1
	int winnerPlayer;	 //�¸�player
	bool finish;		 /// player�� �����̽��� ���� ���� �����ߴ��� �Ǵ��ϴ� flag
	bool white_goal; //�Ͼ���� ����������
	const int player1 = 1; char p1Score;
	const int player2 = -1; char p2Score;
public :
	PlayerTurn() {
		winnerPlayer = 0;
		whosTurn = 1;
		white_goal = false;
		finish = false;
		p1Score = '0';
		p2Score = '0';
	}
	
	bool GetWhiteBall() {       //�Ͼ���� ������������ return
		return this->white_goal;
	}

	void SetWhiteBall() {		//�Ͼ���� �������� ��ġ �ʱ�ȭ
		this->white_goal = false;
	}

	void SetFinish() {			//flag�ʱ�ȭ(�ѻ���� ���� ���۵�)
		this->finish = true;
	}
	void chooseWhoWin() {		//�¸��� player����
		if ((whosTurn == 1) && (p1Score == '7')) winnerPlayer = 1;
		else if ((whosTurn == -1) && (p2Score == '7')) winnerPlayer = -1;
		else winnerPlayer = -whosTurn;
	}
	void TurnUpdate(unsigned short& bit_goalball) {
		if (!finish) return;
		finish = false;
		if (bit_goalball == 0) { ChangePlayer(); return; }				//�ƹ��͵� ������ ���ߴٸ� �Ϲٲٱ�
		else if ( (bit_goalball & 2) == 2) { chooseWhoWin(); return; }	//���������� �־��ٸ� �̱��� ����
		if ( (bit_goalball & 1) == 1) { ChangePlayer();  this->white_goal = true; } //����� �־��ٸ� �����ٲٱ�
		bit_goalball >>= 2;
		int p1=0, p2 = 0;
		for (int i = 0; i < 7; i++) {
			if ((bit_goalball & 1) == 1) p1++;
			bit_goalball >>= 1;
		}
		for (int i = 0; i < 7; i++) {
			if ((bit_goalball & 1) == 1) p2++;
			bit_goalball >>= 1;
		} //������ ���� �ջ�
		if (white_goal) {  }
		else if (whosTurn == 1) { if (p2 != 0)  ChangePlayer(); } //�ڽ��� �Ͽ� ������ ������ �� �ٲٱ�
		else if (whosTurn == -1) { if (p1 != 0) ChangePlayer(); } //�ڽ��� �Ͽ� ������ ������ �� �ٲٱ� 
		p1Score += p1;
		p2Score += p2;
	}

	void ChangePlayer() {//�Ϲٲٴ� �Լ�
		whosTurn = -whosTurn; 
	}

	int GetWinner() { //�¸��� getter�Լ� (�¸� Text �迭�� index�� ��ȯ)
		if (winnerPlayer == 1) return 5;
		else return 6;
	}
	bool EndGame() { //������ ���������� �Ǵ�
		if (winnerPlayer != 0) {
			return true;
		}
		else return false;
	}

	char getScoreP1() {
		return  p1Score;
	}

	char getScoreP2() {
		return  p2Score;
	}

	int indexTurn(){ //������ �������� �����ִ� Text�� index�� ��ȯ
		if (whosTurn == 1) return 0;
		return 4;
	}
};

class Text {////https://m.blog.naver.com/PostView.nhn?blogId=woocom2&logNo=90043358245&proxyReferer=https%3A%2F%2Fwww.google.com%2 //����
private:
	float					center_x, center_z;
	DWORD                mAnchor;
	char* string;
public:
	Text(void)
	{
		ZeroMemory(&lf, sizeof(D3DXFONT_DESC));
	}
	~Text(void) {}

public:
	bool create(IDirect3DDevice9* pDevice,const int textSize[3])
	{
		if (NULL == pDevice)
			return false;
		lf.Height = textSize[0]; 
		lf.Width = textSize[1];
		lf.Weight = textSize[2];
		lf.MipLevels = D3DX_DEFAULT;
		lf.Italic = false;
		lf.CharSet = DEFAULT_CHARSET;
		lf.OutputPrecision = OUT_DEFAULT_PRECIS;
		lf.Quality = DEFAULT_QUALITY;
		lf.PitchAndFamily = DEFAULT_PITCH;
		lf.FaceName, TEXT("�������");

		if (FAILED(D3DXCreateFontIndirect(pDevice, &lf, &pFont)))
			return false;
		mAnchor = DT_TOP | DT_LEFT;
		return true;
	}

	void setCenter(const int textPos[2])
	{
		center_x = textPos[0]; center_z = textPos[1];
	}

	void destroy(void)
	{
		if (m_pTextMesh != NULL) {
			m_pTextMesh->Release();
			m_pTextMesh = NULL;
		}
	}

	void draw(IDirect3DDevice9* pDevice,const char string[])
	{
		if (string == NULL) return;
		if (NULL == pDevice)
			return;
		RECT rect = { center_x, center_z, 640, 480 };
		#ifdef UNICODE
		pFont->DrawTextW(0, string, -1, &rect, mAnchor, d3d::RED);
		#else
		pFont->DrawTextA(0, string, -1, &rect, mAnchor, d3d::RED);
		#endif
	}

	void draw(IDirect3DDevice9* pDevice, const char string) //�� draw�Լ� �����ε�
	{
		if (string == NULL) return;
		if (NULL == pDevice)
			return;
		RECT rect = { center_x, center_z, 640, 480 };
#ifdef UNICODE
		pFont->DrawTextW(0, &string, -1, &rect, mAnchor, d3d::RED);
#else
		pFont->DrawTextA(0, &string, -1, &rect, mAnchor, d3d::RED);
#endif
	}

private:
	D3DXFONT_DESC lf;
	ID3DXFont* pFont;
	ID3DXMesh* m_pTextMesh;

};
class CSphere {
private :
	float					center_x, center_y, center_z;
    float                   m_radius;
	float					m_velocity_x;
	float					m_velocity_z;
	bool ball_goal; //���� ���� �ߴ��� ǥ��
public:
    CSphere(void)
    {
		ball_goal = false;
        D3DXMatrixIdentity(&m_mLocal);
        ZeroMemory(&m_mtrl, sizeof(m_mtrl));
        m_radius = 0;
		m_velocity_x = 0;
		m_velocity_z = 0;
        m_pSphereMesh = NULL;
    }
    ~CSphere(void) {}

public:
	void setball_goal(bool fall) { //���� ���� bool������ ���������� �ٽ� ����������(�����) ����
		this->ball_goal = fall;
	}

	bool getBall_goal() { 
		return this->ball_goal;
	}
    bool create(IDirect3DDevice9* pDevice, D3DXCOLOR color = d3d::WHITE)
    {
        if (NULL == pDevice)
            return false;
		
        m_mtrl.Ambient  = color;
        m_mtrl.Diffuse  = color;
        m_mtrl.Specular = color;
        m_mtrl.Emissive = d3d::BLACK;
        m_mtrl.Power    = 5.0f;
        if (FAILED(D3DXCreateSphere(pDevice, getRadius(), 50, 50, &m_pSphereMesh, NULL)))
            return false;
        return true;
    }

    void destroy(void)
    {
        if (m_pSphereMesh != NULL) {
            m_pSphereMesh->Release();
            m_pSphereMesh = NULL;
        }
    }

    void draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
    {
        if (NULL == pDevice)
            return;
        pDevice->SetTransform(D3DTS_WORLD, &mWorld);
        pDevice->MultiplyTransform(D3DTS_WORLD, &m_mLocal);
        pDevice->SetMaterial(&m_mtrl);
		m_pSphereMesh->DrawSubset(0);
    }
	
    bool hasIntersected(CSphere& ball) 
	{
		D3DXVECTOR3 targetPos = ball.getCenter();
		D3DXVECTOR3 thisPos = this->getCenter();

		if (sqrt(pow(targetPos.x - thisPos.x, 2) + pow(targetPos.z - thisPos.z, 2)) < M_RADIUS * 2) {	//��ȣ�� ������ ����
			//�浹�� �� Ǯ���ֱ� ��ǥ�� ��ģ�� �ٽ� �ǵ�����...
			return true;
		}
		return false;
	}
	
	void hitBy(CSphere& ball) //this�� ball�� �浹
	{
		if (hasIntersected(ball)) {
			D3DXVECTOR3 avec, a1, a2;

			float dest;	//�浹 ������ ���� ���� ����
			const D3DXVECTOR3 centVec = ball.getCenter() - this->getCenter();
			dest = D3DXVec3Length(&centVec);
			avec = (ball.getCenter() - this->getCenter()) / dest;

			//�� O1, O2�� �ӵ� ���� V1, V2�� �浹 ���� ���� ����
			D3DXVECTOR3 v1 = { (float)this->getVelocity_X(), (float)0.0, (float)this->getVelocity_Z() };
			D3DXVECTOR3 v2 = { (float)ball.getVelocity_X(), (float)0.0, (float)ball.getVelocity_Z() };
			a1 = (D3DXVec3Dot(&avec, &v1) * avec);	//���� (���� ��)
			a2 = (D3DXVec3Dot(&avec, &v2) * avec);	//����

													//V1*avec�� V2*avec�� ��ü
			this->setPower(v1 - a1 + a2);
			ball.setPower(v2 - a2 + a1);


			//���� ������´�
			if (this->getPower() > ball.getPower()) {
				this->moveCenter((2 * this->getRadius() - dest) * -1 * avec);
			}
			else {
				ball.moveCenter((2 * this->getRadius() - dest) * 1 * avec);
			}


		}
	}

	void moveCenter(D3DXVECTOR3 vel) {
		D3DXVECTOR3 temp = this->getCenter() + vel;
		this->setCenter(temp.x, temp.y, temp.z);
	}

	void setPower(D3DXVECTOR3 input)
	{
		this->m_velocity_x = input.x;
		this->m_velocity_z = input.z;
	}

	float getPower() {
		return m_velocity_x * m_velocity_x + m_velocity_z * m_velocity_z;
	}

	void ballUpdate(float timeDiff) 
	{
		const float TIME_SCALE = 3.3;
		D3DXVECTOR3 cord = this->getCenter();
		double vx = abs(this->getVelocity_X());
		double vz = abs(this->getVelocity_Z());

		if(vx > 0.01 || vz > 0.01)
		{
			float tX = cord.x + TIME_SCALE*timeDiff*m_velocity_x;
			float tZ = cord.z + TIME_SCALE*timeDiff*m_velocity_z;
			this->setCenter(tX, cord.y, tZ);
		}
		else { this->setPower(0,0);}
		//this->setPower(this->getVelocity_X() * DECREASE_RATE, this->getVelocity_Z() * DECREASE_RATE);
		double rate = 1 -  (1 - DECREASE_RATE)*timeDiff * 400;
		if(rate < 0 )
			rate = 0;
		this->setPower(getVelocity_X() * rate, getVelocity_Z() * rate);
	}

	double getVelocity_X() { return this->m_velocity_x;	}
	double getVelocity_Z() { return this->m_velocity_z; }

	void setPower(double vx, double vz)
	{
		this->m_velocity_x = vx;
		this->m_velocity_z = vz;
	}

	void setCenter(float x, float y, float z)
	{
		D3DXMATRIX m;
		center_x=x;	center_y=y;	center_z=z;
		D3DXMatrixTranslation(&m, x, y, z);
		setLocalTransform(m);
	}
	
	float getRadius(void)  const { return (float)(M_RADIUS);  }
    const D3DXMATRIX& getLocalTransform(void) const { return m_mLocal; }
    void setLocalTransform(const D3DXMATRIX& mLocal) { m_mLocal = mLocal; }
    D3DXVECTOR3 getCenter(void) const
    {
        D3DXVECTOR3 org(center_x, center_y, center_z);
        return org;
    }
	
private:
    D3DXMATRIX              m_mLocal;
    D3DMATERIAL9            m_mtrl;
    ID3DXMesh*              m_pSphereMesh;
	
};
class CHole {
private:
	float					center_x, center_y, center_z;
	float                   m_radius;

public:
	CHole(void)
	{
		D3DXMatrixIdentity(&m_mLocal);
		ZeroMemory(&m_mtrl, sizeof(m_mtrl));
		m_radius = 0;
		m_pHoleMesh = NULL;
	}
	~CHole(void) {}

public:
	bool create(IDirect3DDevice9* pDevice, D3DXCOLOR color = d3d::WHITE)
	{
		if (NULL == pDevice)
			return false;
		m_mtrl.Ambient = color;
		m_mtrl.Diffuse = color;
		m_mtrl.Specular = color;
		m_mtrl.Emissive = d3d::BLACK;
		m_mtrl.Power = 5.0f;

		if (FAILED(D3DXCreateSphere(pDevice, getRadius(), 50, 50, &m_pHoleMesh, NULL)))
			return false;
		return true;
	}

	void setCenter(float x, float y, float z)
	{
		D3DXMATRIX m;
		center_x = x;	center_y = y;	center_z = z;
		D3DXMatrixTranslation(&m, x, y, z);
		setLocalTransform(m);
	}

	void destroy(void)
	{
		if (m_pHoleMesh != NULL) {
			m_pHoleMesh->Release();
			m_pHoleMesh = NULL;
		}
	}

	void draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
	{
		if (NULL == pDevice)
			return;
		pDevice->SetTransform(D3DTS_WORLD, &mWorld);
		pDevice->MultiplyTransform(D3DTS_WORLD, &m_mLocal);
		pDevice->SetMaterial(&m_mtrl);
		m_pHoleMesh->DrawSubset(0);
	}

	bool hasIntersected(CSphere& ball)
	{
		if (ball.getBall_goal()) return false; //�� ���� ���� �����ߴٸ� return false
		D3DXVECTOR3 targetpos = ball.getCenter();
		D3DXVECTOR3	holepos = this->getCenter();
		if ((pow(targetpos.x - holepos.x, 2) + pow(targetpos.z - holepos.z, 2) + pow(targetpos.y - holepos.y, 2)) - pow(M_RADIUS, 2) <= pow(H_RADIUS, 2)) {
			return true;
		}
		return false;
	}

	bool hitBy(CSphere& ball) // ���۰� ball
	{///3���� �������̿� ���������� �Ÿ��� ��Ÿ��� �̿�(������ �ѷ� ���� ball�� �߽��̿��� ����!)
		if (hasIntersected(ball)) {
			ball.setPower(0, 0);
			ball.setCenter(-100.0f, -(float)M_RADIUS, -100.0f);
			ball.setball_goal(true); //���� �������ɷ� �ʱ�ȭ
			return true;
		}
		return false;
	}

	float getRadius(void)  const { return (float)(H_RADIUS); }
	const D3DXMATRIX& getLocalTransform(void) const { return m_mLocal; }
	void setLocalTransform(const D3DXMATRIX& mLocal) { m_mLocal = mLocal; }
	D3DXVECTOR3 getCenter(void) const
	{
		D3DXVECTOR3 org(center_x, center_y, center_z);
		return org;
	}

private:
	D3DXMATRIX              m_mLocal;
	D3DMATERIAL9            m_mtrl;
	ID3DXMesh* m_pHoleMesh;

};

// -----------------------------------------------------------------------------
// CWall class definition
// -----------------------------------------------------------------------------

class CWall {

private:
	
    float					m_x;
	float					m_z;
	float                   m_width;
    float                   m_depth;
	float					m_height;
	
public:
    CWall(void)
    {
        D3DXMatrixIdentity(&m_mLocal);
        ZeroMemory(&m_mtrl, sizeof(m_mtrl));
        m_width = 0;
        m_depth = 0;
        m_pBoundMesh = NULL;
    }
    ~CWall(void) {}
public:
    bool create(IDirect3DDevice9* pDevice, float ix, float iz, float iwidth, float iheight, float idepth, D3DXCOLOR color = d3d::WHITE)
    {
        if (NULL == pDevice)
            return false;
		
        m_mtrl.Ambient  = color;
        m_mtrl.Diffuse  = color;
        m_mtrl.Specular = color;
        m_mtrl.Emissive = d3d::BLACK;
        m_mtrl.Power    = 5.0f;
		
        m_width = iwidth;
        m_depth = idepth;
		
        if (FAILED(D3DXCreateBox(pDevice, iwidth, iheight, idepth, &m_pBoundMesh, NULL)))
            return false;
        return true;
    }
    void destroy(void)
    {
        if (m_pBoundMesh != NULL) {
            m_pBoundMesh->Release();
            m_pBoundMesh = NULL;
        }
    }
    void draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
    {
        if (NULL == pDevice)
            return;
        pDevice->SetTransform(D3DTS_WORLD, &mWorld);
        pDevice->MultiplyTransform(D3DTS_WORLD, &m_mLocal);
        pDevice->SetMaterial(&m_mtrl);
		m_pBoundMesh->DrawSubset(0);
    }
	
	bool hasIntersected(CSphere& ball) 
	{
		D3DXVECTOR3 targetPos = ball.getCenter();

		//if�� 4�� ���� 4���� ���� ������ true �ƴϸ� false
		if (targetPos.x - M_RADIUS <= -4.5)
			return true;
		else if (targetPos.x + M_RADIUS >= 4.5)
			return true;
		else if (targetPos.z - M_RADIUS <= -3)
			return true;
		else if (targetPos.z + M_RADIUS >= 3)
			return true;

		else
			return false;
	}//��� ��

	void hitBy(CSphere& ball) // ball�̶� �� �ε���
	{
		D3DXVECTOR3	whitepos = ball.getCenter();

		if (whitepos.x + M_RADIUS >= 4.5) {
			ball.setCenter(4.5 - M_RADIUS, whitepos.y, whitepos.z);
			ball.setPower(-ball.getVelocity_X() * DECREASE_RATE, ball.getVelocity_Z() * DECREASE_RATE);
		}
		else if (whitepos.x - M_RADIUS <= -4.5) {
			ball.setCenter(-4.5 + M_RADIUS, whitepos.y, whitepos.z);
			ball.setPower(-ball.getVelocity_X() * DECREASE_RATE, ball.getVelocity_Z() * DECREASE_RATE);
		}
		else if (whitepos.z - M_RADIUS <= -3) {
			ball.setCenter(whitepos.x, whitepos.y, -3 + M_RADIUS);
			ball.setPower(ball.getVelocity_X() * DECREASE_RATE, -ball.getVelocity_Z() * DECREASE_RATE);
		}
		else if (whitepos.z + M_RADIUS >= 3) {
			ball.setCenter(whitepos.x, whitepos.y, 3 - M_RADIUS);
			ball.setPower(ball.getVelocity_X() * DECREASE_RATE, -ball.getVelocity_Z() * DECREASE_RATE);
		}

	}    
	
	void setPosition(float x, float y, float z)
	{
		D3DXMATRIX m;
		this->m_x = x;
		this->m_z = z;

		D3DXMatrixTranslation(&m, x, y, z);
		setLocalTransform(m);
	}
	
    float getHeight(void) const { return M_HEIGHT; }
	
	
	
private :
    void setLocalTransform(const D3DXMATRIX& mLocal) { m_mLocal = mLocal; }
	
	D3DXMATRIX              m_mLocal;
    D3DMATERIAL9            m_mtrl;
    ID3DXMesh*              m_pBoundMesh;
};

// -----------------------------------------------------------------------------
// CLight class definition
// -----------------------------------------------------------------------------

class CLight {
public:
    CLight(void)
    {
        static DWORD i = 0;
        m_index = i++;
        D3DXMatrixIdentity(&m_mLocal);
        ::ZeroMemory(&m_lit, sizeof(m_lit));
        m_pMesh = NULL;
        m_bound._center = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        m_bound._radius = 0.0f;
    }
    ~CLight(void) {}
public:
    bool create(IDirect3DDevice9* pDevice, const D3DLIGHT9& lit, float radius = 0.1f)
    {
        if (NULL == pDevice)
            return false;
        if (FAILED(D3DXCreateSphere(pDevice, radius, 10, 10, &m_pMesh, NULL)))
            return false;
		
        m_bound._center = lit.Position;
        m_bound._radius = radius;
		
        m_lit.Type          = lit.Type;
        m_lit.Diffuse       = lit.Diffuse;
        m_lit.Specular      = lit.Specular;
        m_lit.Ambient       = lit.Ambient;
        m_lit.Position      = lit.Position;
        m_lit.Direction     = lit.Direction;
        m_lit.Range         = lit.Range;
        m_lit.Falloff       = lit.Falloff;
        m_lit.Attenuation0  = lit.Attenuation0;
        m_lit.Attenuation1  = lit.Attenuation1;
        m_lit.Attenuation2  = lit.Attenuation2;
        m_lit.Theta         = lit.Theta;
        m_lit.Phi           = lit.Phi;
        return true;
    }
    void destroy(void)
    {
        if (m_pMesh != NULL) {
            m_pMesh->Release();
            m_pMesh = NULL;
        }
    }
    bool setLight(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
    {
        if (NULL == pDevice)
            return false;
		
        D3DXVECTOR3 pos(m_bound._center);
        D3DXVec3TransformCoord(&pos, &pos, &m_mLocal);
        D3DXVec3TransformCoord(&pos, &pos, &mWorld);
        m_lit.Position = pos;
		
        pDevice->SetLight(m_index, &m_lit);
        pDevice->LightEnable(m_index, TRUE);
        return true;
    }

    void draw(IDirect3DDevice9* pDevice)
    {
        if (NULL == pDevice)
            return;
        D3DXMATRIX m;
        D3DXMatrixTranslation(&m, m_lit.Position.x, m_lit.Position.y, m_lit.Position.z);
        pDevice->SetTransform(D3DTS_WORLD, &m);
        pDevice->SetMaterial(&d3d::WHITE_MTRL);
        m_pMesh->DrawSubset(0);
    }

    D3DXVECTOR3 getPosition(void) const { return D3DXVECTOR3(m_lit.Position); }

private:
    DWORD               m_index;
    D3DXMATRIX          m_mLocal;
    D3DLIGHT9           m_lit;
    ID3DXMesh*          m_pMesh;
    d3d::BoundingSphere m_bound;
};


// -----------------------------------------------------------------------------
// Global variables
// -----------------------------------------------------------------------------
CWall	g_legoPlane;
CWall	g_legowall[4];
CHole g_hole[6];
CSphere	g_sphere[numberOfBall];
CSphere	g_target_blueball;
CLight	g_light;
Text g_text[7];
PlayerTurn* playerturn = new PlayerTurn();
// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------


void destroyAllLegoBlock(void)
{
}

// initialization
bool Setup()
{
	int i;
	
    D3DXMatrixIdentity(&g_mWorld);
    D3DXMatrixIdentity(&g_mView);
    D3DXMatrixIdentity(&g_mProj);
		
	// create plane and set the position
    if (false == g_legoPlane.create(Device, -1, -1, 9, H_RADIUS, 6, d3d::GREEN)) return false;
    g_legoPlane.setPosition(0.0f, -H_RADIUS/2, 0.0f);
	
	// create walls and set the position. note that there are four walls
	if (false == g_legowall[0].create(Device, -1, -1, 9+2*H_RADIUS, H_RADIUS*2, H_RADIUS, d3d::DARKRED)) return false;
	g_legowall[0].setPosition(0.0f, 0, 3.0f+H_RADIUS/2);
	if (false == g_legowall[1].create(Device, -1, -1, 9+2*H_RADIUS, H_RADIUS*2, H_RADIUS, d3d::DARKRED)) return false;
	g_legowall[1].setPosition(0.0f, 0, -3.0f-H_RADIUS/2);
	if (false == g_legowall[2].create(Device, -1, -1, H_RADIUS, H_RADIUS*2, 6.0f+ H_RADIUS*2, d3d::DARKRED)) return false;
	g_legowall[2].setPosition(4.5f+ H_RADIUS/2, 0, 0.0f);
	if (false == g_legowall[3].create(Device, -1, -1, H_RADIUS, H_RADIUS*2, 6.0f+ H_RADIUS*2, d3d::DARKRED)) return false;
	g_legowall[3].setPosition(-4.5f- H_RADIUS/2, 0, 0.0f);

	// create four balls and set the position
	for (i=0;i< numberOfBall;i++) {
		if (false == g_sphere[i].create(Device, sphereColor[i])) return false;
		g_sphere[i].setCenter(spherePos[i][0], (float)M_RADIUS , spherePos[i][1]);
		g_sphere[i].setPower(0,0);
	}

	//create holes
	for (i = 0; i < 6; i++) {
		if (false == g_hole[i].create(Device, d3d::BLACK)) return false;
		g_hole[i].setCenter(holePos[i][0], 0, holePos[i][1]);
	}
	// create blue ball for set direction
    if (false == g_target_blueball.create(Device, d3d::BLUE)) return false;
	g_target_blueball.setCenter(.0f, (float)M_RADIUS , .0f);
	
	// light setting 
    D3DLIGHT9 lit;
    ::ZeroMemory(&lit, sizeof(lit));
    lit.Type         = D3DLIGHT_POINT;
    lit.Diffuse      = d3d::WHITE; 
	lit.Specular     = d3d::WHITE * 0.9f;
    lit.Ambient      = d3d::WHITE * 0.9f;
    lit.Position     = D3DXVECTOR3(0.0f, 3.9f, 0.0f); //�Ͼ�� ���׶�� ������
    lit.Range        = 100.0f;
    lit.Attenuation0 = 0.0f;
    lit.Attenuation1 = 0.9f;
    lit.Attenuation2 = 0.0f;
    if (false == g_light.create(Device, lit))
        return false;
	
	// Position and aim the camera.
	D3DXVECTOR3 pos(0.0f, 5.0f, -8.0f);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 2.0f, 0.0f);
	D3DXMatrixLookAtLH(&g_mView, &pos, &target, &up);
	Device->SetTransform(D3DTS_VIEW, &g_mView);
	
	// Set the projection matrix.
	D3DXMatrixPerspectiveFovLH(&g_mProj, D3DX_PI / 4,
        (float)Width / (float)Height, 1.0f, 100.0f);
	Device->SetTransform(D3DTS_PROJECTION, &g_mProj);
	
    // Set render states.
    Device->SetRenderState(D3DRS_LIGHTING, TRUE);
    Device->SetRenderState(D3DRS_SPECULARENABLE, TRUE);
    Device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
	
	g_light.setLight(Device, g_mWorld);
	///// Set the text
	for (int i = 0; i < 6; i++) {
		g_text[i].create(Device,textSize[0]);
		g_text[i].setCenter(textPos[i]);
	}
	g_text[6].create(Device, textSize[1]);
	g_text[6].setCenter(textPos[6]);
	return true;
}

void Cleanup(void)
{
    g_legoPlane.destroy();
	for(int i = 0 ; i < 4; i++) {
		g_legowall[i].destroy();
		
	}
	for (int i = 0; i < 7; i++) {
		g_text[i].destroy();

	}
    destroyAllLegoBlock();
    g_light.destroy();
}


unsigned short bit_goalball = 0;// �� ���� ��Ʈ�� ǥ�� ������ ù��° �ڸ��� ���

// timeDelta represents the time between the current image frame and the last image frame.
// the distance of moving balls should be "velocity * timeDelta"
bool Display(float timeDelta)
{
	bool movefinish = true;
	int i=0;
	int j = 0;
	if( Device )
	{
		Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00afafaf, 1.0f, 0);
		Device->BeginScene();
		// update the position of each ball. during update, check whether each ball hit by walls.
		for( i = 0; i < numberOfBall; i++) {
			g_sphere[i].ballUpdate(timeDelta);
			for(j = 0; j < 4; j++){ g_legowall[j].hitBy(g_sphere[i]); }
			for (j = 0; j < 6; j++) { if (g_hole[j].hitBy(g_sphere[i])) bit_goalball += pow(2, i); } //���������� 16��Ʈ�� ǥ��
			if ((g_sphere[i].getVelocity_X() != 0) || (g_sphere[i].getVelocity_Z() != 0)) { //������ ���� �����̰� �ִٸ� �� �ȹٲ�
				movefinish = false;
			}
		}
		if (movefinish)playerturn->TurnUpdate(bit_goalball); //������ �� �������ٸ� turn�ٲٱ�
		if(playerturn->EndGame()) g_text[6].draw(Device, system_msg[playerturn->GetWinner()]); //�¸��ڰ� ���������� �¸�text���
		else g_text[0].draw(Device, system_msg[playerturn->indexTurn()]); //�¸��ڰ� ���������ʾҴٸ� ������ ������ ǥ��
		if(playerturn->GetWhiteBall())g_text[1].draw(Device, system_msg[1]); //����� �������ٸ� ����� ������� text���
		g_text[2].draw(Device, system_msg[2]); //Player1 :
		g_text[3].draw(Device, system_msg[3]); //Player2 :
		g_text[4].draw(Device, playerturn->getScoreP1()); //P1 ����
		g_text[5].draw(Device, playerturn->getScoreP2()); //P2 ����
		
		// check whether any two balls hit together and update the direction of balls
		for(i = 0 ;i < numberOfBall; i++){
			for(j = 0 ; j < numberOfBall; j++) {
				if(i >= j) {continue;}
				g_sphere[i].hitBy(g_sphere[j]);
			}
		}

		// draw plane, walls, and spheres
		g_legoPlane.draw(Device, g_mWorld);
		for (i=0;i<4;i++) 	{
			g_legowall[i].draw(Device, g_mWorld);
		}
		for (i = 0; i < numberOfBall; i++) {
			g_sphere[i].draw(Device, g_mWorld);
		}
		for (i = 0; i < 6; i++) {
			g_hole[i].draw(Device, g_mWorld);
		}
		g_target_blueball.draw(Device, g_mWorld);
        g_light.draw(Device);
		
		Device->EndScene();
		Device->Present(0, 0, 0, 0);
		Device->SetTexture( 0, NULL );
	}
	return true;
}

LRESULT CALLBACK d3d::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static bool wire = false;
	static bool isReset = true;
    static int old_x = 0;
    static int old_y = 0;
    static enum { WORLD_MOVE, LIGHT_MOVE, BLOCK_MOVE } move = WORLD_MOVE;
	
	switch( msg ) {
	case WM_DESTROY:
        {
			::PostQuitMessage(0);
			break;
        }
	case WM_KEYDOWN:
        {
            switch (wParam) {
            case VK_ESCAPE:
				::DestroyWindow(hwnd);
                break;
            case VK_RETURN:
                if (NULL != Device) {
                    wire = !wire;
                    Device->SetRenderState(D3DRS_FILLMODE,
                        (wire ? D3DFILL_WIREFRAME : D3DFILL_SOLID));
                }
                break;
			case VK_F5: { //F5Ű�� �������� ���� ����
				D3DXMATRIX mX;
				D3DXMATRIX mY;
				D3DXMatrixRotationY(&mX, 0.0);
				D3DXMatrixRotationX(&mY, -1.0);
				g_mWorld =  mX * mY;
				}
				break;
			case VK_F6: { //F6 �������� �Ķ��� �� ��ġ �ʱ�ȭ
				g_target_blueball.setCenter(0,(float)M_RADIUS,0);
				}
				break;
            case VK_SPACE:
				if (playerturn->GetWhiteBall()) { //��� ���ġ
					playerturn->SetWhiteBall();
					D3DXVECTOR3 position = g_target_blueball.getCenter(); //�Ķ����� ������ ����
					g_sphere[0].setCenter(position.x,(float)M_RADIUS,position.z);
					g_sphere[0].setball_goal(false);
					break;
				}
				playerturn->SetFinish();
				D3DXVECTOR3 targetpos = g_target_blueball.getCenter();
				D3DXVECTOR3	whitepos = g_sphere[0].getCenter();
				double theta = acos(sqrt(pow(targetpos.x - whitepos.x, 2)) / sqrt(pow(targetpos.x - whitepos.x, 2) +
					pow(targetpos.z - whitepos.z, 2)));		// �⺻ 1 ��и�
				if (targetpos.z - whitepos.z <= 0 && targetpos.x - whitepos.x >= 0) { theta = -theta; }	//4 ��и�
				if (targetpos.z - whitepos.z >= 0 && targetpos.x - whitepos.x <= 0) { theta = PI - theta; } //2 ��и�
				if (targetpos.z - whitepos.z <= 0 && targetpos.x - whitepos.x <= 0){ theta = PI + theta; } // 3 ��и�
				double distance = sqrt(pow(targetpos.x - whitepos.x, 2) + pow(targetpos.z - whitepos.z, 2));
				g_sphere[0].setPower(distance * cos(theta), distance * sin(theta));

				break;
			}
			break;
        }
		
	case WM_MOUSEMOVE:
        {
            int new_x = LOWORD(lParam);
            int new_y = HIWORD(lParam);
			float dx;
			float dy;
			
            if (LOWORD(wParam) & MK_LBUTTON) {
				
                if (isReset) {
                    isReset = false;
                } else {
                    D3DXVECTOR3 vDist;
                    D3DXVECTOR3 vTrans;
                    D3DXMATRIX mTrans;
                    D3DXMATRIX mX;
                    D3DXMATRIX mY;
					
                    switch (move) {
                    case WORLD_MOVE:
                        dx = (old_x - new_x) * 0.01f;
                        dy = (old_y - new_y) * 0.01f;
                        D3DXMatrixRotationY(&mX, dx);
                        D3DXMatrixRotationX(&mY, dy);
                        g_mWorld = g_mWorld * mX * mY;
                        break;
                    }
                }
				
                old_x = new_x;
                old_y = new_y;

            } else {
                isReset = true;
				
				if (LOWORD(wParam) & MK_RBUTTON) {
					dx = (old_x - new_x);// * 0.01f;
					dy = (old_y - new_y);// * 0.01f;
		
					D3DXVECTOR3 coord3d=g_target_blueball.getCenter();
					g_target_blueball.setCenter(coord3d.x+dx*(-0.007f),coord3d.y,coord3d.z+dy*0.007f );
				}
				old_x = new_x;
				old_y = new_y;
				
                move = WORLD_MOVE;
            }
            break;
        }
	}
	
	return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hinstance,
				   HINSTANCE prevInstance, 
				   PSTR cmdLine,
				   int showCmd)
{
    srand(static_cast<unsigned int>(time(NULL)));
	
	if(!d3d::InitD3D(hinstance,
		Width, Height, true, D3DDEVTYPE_HAL, &Device))
	{
		::MessageBox(0, "InitD3D() - FAILED", 0, 0);
		return 0;
	}
	
	if(!Setup())
	{
		::MessageBox(0, "Setup() - FAILED", 0, 0);
		return 0;
	}
	
	d3d::EnterMsgLoop( Display );
	
	Cleanup();
	
	Device->Release();
	
	return 0;
}
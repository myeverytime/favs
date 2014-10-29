

#include <stdio.h>
#include <string>
#include <omp.h>
#include "glmath.h"


//���̸� ó����ƾ
void createLandscapeFloat(float *heightmap, int heightmap_width, int heightmap_height,
                          int width, int height, vec3 *& vertices, float& hmin, float& hmax);


vec3  createLandscapeTRN(float *heightmap, int heightmap_width, int heightmap_height,
                          int width, int height, vec3 *& vertices, float& hmin, float& hmax, float StartX=0.0, float StartY=0.0, float CTC_X=1.0, float CTC_Y=1.0);

void createWaterLevelTRN(float *heightmap, int heightmap_width, int heightmap_height,
                          int width, int height, vec3 *& vertices, float& hmin, float& hmax, float StartX=0.0, float StartY=0.0, float CTC_X=1.0, float CTC_Y=1.0);

void readTRN(const char* filename, int& width, int& height, float*& heightmap);
void readSmoothedTRN(const char* filename, int& width, int& height, float*& heightmap);//������������LOAD
void readTRN2(const char* filename, int width, int height, int *&gridtypes);



//�ؼ������� ó����ƾ
class AnalysisData
{
public:
	// Mesh Block ����
	int m_nMeshBlick;
	// ������
	float m_fStartPointX;
	float m_fStartPointY;

	// ����
	float m_fCTC_X;
	float m_fCTC_Y;

	// ����
	int m_nCountX;
	int m_nCountY;
	int m_nCountZ;

	// trn ���ϸ�
	char m_sPathName[200];
	char m_sPathNameWsp[200];
	char m_sPathNameVet[200];
public:
	AnalysisData();
	~AnalysisData();

	void GetWaterVelocity(int nGetIdx, vec3 *& WaterVel, float*& WaterSpeed, float& hmin, float& hmax);
	void OpenWaterData(char* strPathName);
	void SaveTrnTotal(char* filename);
	void SaveTrn(char* filename, float*& WaterLevel);
	void SaveWsp(char* filename, float*& WaterSpeed);

	void GetTrnTitle(char* trnfileName, char* wspfileName, char* vetfilename, int& datawidth, int& dataheight, int& AnalBlockNum, float& StartX, float& StartY, float& CTC_X, float& CTC_Y);
	void GetSmoothedWaterLevel(int nGetIdx, float*& WaterLevel);
	void GetWaterLevel(int nGetIdx, float*& WaterLevel);
	void GetWaterSpeed(int nGetIdx, float*& WaterSpeed, float& hmin, float& hmax);

};

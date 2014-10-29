#ifndef MAINFLUIDVIS_H
#define MAINFLUIDVIS_H

#include <windows.h>
#include "glmath.h"
#include "mstring.h"

#include <gl/glew.h>
#include <gl/wglew.h>
#include <gl/glut.h>

#include <FreeImage.h>

#define WIN_WIDTH 800
#define WIN_HEIGHT 600
// ----------------------------------------------------------------------------------------------------------------------------

#pragma comment(lib, "glaux.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "FreeImage.lib")

// ----------------------------------------------------------------------------------------------------------------------------
static int ggl_max_texture_size = 0, ggl_max_texture_max_anisotropy_ext = 0;

static MCString ErrorLog;

#define BUFFER_SIZE_INCREMENT 1048576

// ----------------------------------------------------------------------------------------------------------------------------

class CBuffer
{
private:
	BYTE *Buffer;
	int BufferSize, Position;

public:
	CBuffer();
	~CBuffer();

	void AddData(void *Data, int DataSize);
	void Empty();
	void *GetData();
	int GetDataSize();

private:
	void SetDefaults();
};

// ----------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------

class CTexture
{
public:
	GLuint Texture;

	CTexture();
	~CTexture();

	operator GLuint ();

	bool LoadTexture2D(char *FileName);
	bool LoadTextureCubeMap(char **FileNames);
	void Destroy();

protected:
	FIBITMAP *CTexture::GetBitmap(char *FileName, int &Width, int &Height, int &BPP);
};

// ----------------------------------------------------------------------------------------------------------------------------

class CShaderProgram
{
protected:
	GLuint VertexShader, FragmentShader, Program;

public:
	GLuint *UniformLocations, *AttribLocations;

public:
	CShaderProgram();
	~CShaderProgram();

	operator GLuint ();

	bool Load(char *VertexShaderFileName, char *FragmentShaderFileName);
	void Destroy();

protected:
	GLuint LoadShader(char *FileName, GLenum Type);
	void SetDefaults();
};

// ----------------------------------------------------------------------------------------------------------------------------
#define DEFAULTCAMERAANIMSTEP 0.01
#define MAXCAMERAPATH 100
#define MINDELTA 0.1//�ҷ�ȸ����

class CCamera
{
public:
	vec3 X, Y, Z, Position, Reference;

	mat4x4 *ViewMatrix, *ViewMatrixInverse;

	mat4x4 SavingViewMatrix[MAXCAMERAPATH];
	float CameraAnimStep[MAXCAMERAPATH];//ī�޶���
	bool CameraAnimPause;//ī�޶�ִϸ��̼���������
	int CameraPathNum;//�� ī�޶� ��� ����
	int CurCameraPath;//���� ī�޶� ���
	float AnimatingParam;
	bool CameraPathAnimationOn;
	bool DrawPerspecOrOrtho;

	CCamera();
	~CCamera();

	void SetReferencePoint(vec3 hcenter);//ī�޶� ȸ���Ҷ��� ������ ����
	vec3 MoveByMouseMButton(int dx, int dy);
	void RotateToXZPlane();
	void RotateToYZPlane();
	void RotateToXYPlane();
	void RotateAboutLeftRight(float DELTA);
	void RotateAboutUpDown(float DELTA);
	void SetPersOrtho();
	void RotateAboutXAxis(float DRot);
	void RotateAboutYAxis(float DRot);
	void RotateAboutZAxis();

	void GetSavingViewMatrix();
	void SetSavingViewMatrix();
	void SetAnimatingViewMatrix();

	void Look(const vec3 &Position, const vec3 &Reference, bool RotateAroundReference = false);
	void Move(const vec3 &Movement);
	vec3 OnKeys(BYTE Keys, float FrameTime);
	void OnMouseMove(int dx, int dy);
	void OnMouseWheel(float zDelta);
	void SetViewMatrixPointer(float *ViewMatrix, float *ViewMatrixInverse = NULL);

private:
	void CalculateViewMatrix();
};

// ----------------------------------------------------------------------------------------------------------------------------

#define MAX_WAVES 16

class CWave
{
public:
	float StartTime, Speed, MaxY, FrequencyMPIM2;
	vec2 Position;

public:
	CWave();
	~CWave();
};

// ----------------------------------------------------------------------------------------------------------------------------
#include "stlfile.h"//stl����ó�����
#include "GLTrihedron.h"//ȭ��ǥó�����
#include "landscapecreator.h"


//OBJ����ó��
#include "model_obj.h"
#include "bitmap.h"
#include <map>
typedef std::map<std::string, GLuint> ModelTextures;//���� ���ؽ���


#define NumOfSTLOBJ 4//STL OBJECT ����

#define ANAL_WATERSTYLE 0//�ؼ������� ����Ÿ��
#define ANAL_VELOCITYSTYLE 1//�ؼ������� ����
#define ANAL_DEPTH 2//�ؼ������� ����
#define ANAL_ELEVATION 3//�ؼ������� ��
#define ANAL_STREAMLINE 5//�ؼ������� ��Ʈ������ǥ��
#define ANAL_VECTOR 6//�ؼ������� ����ǥ��
#define TERRAIN_HEIGHT 4//������ ����

#define M_PI           3.14159265358979323846f

#define OBJNUM 2

class COpenGLRenderer
{
protected:
	CGLTrihedron myTrihedron;
	int Width, Height;
	mat3x3 NormalMatrix;
	mat4x4 ModelMatrix, ViewMatrix, ViewMatrixInverse, ProjectionMatrix, ProjectionBiasMatrixInverse;

protected:
	int MappingTextureExist;//Terrain�� ������ �ؽ����� ���翩��
	CTexture AnalNormalMapTex;//�ؼ������� WATER�� ������ ������ �ؽ���
	CTexture MappingTexture;//Terrain�� ������ �ؽ���
	CTexture Texture[4];
	CShaderProgram Water, EnvMapProgram;
	GLuint WaterVerticesVBO, PoolSkyVBO;
	GLuint WaterHeightMaps[2], WHMID, WaterNormalMap, WaterVBO, TerrainVBO, TexCoordsVBO, TerrainNormalVBO, TerrainColorMapVBO, NFLOWTerrainVBO, NFLOWTerrainNormalVBO;
	GLuint AnalWaterVBO, AnalElevationColorVBO, AnalVelocityColorVBO, AnalNormalVBO, AnalNormalMapTexVBO;
	GLuint ReflectionTexture, RefractionTexture, DepthTexture, FBO;
	float WaterLevel;
	CWave Waves[MAX_WAVES];
	int Wave, WaterVerticesCount;

public:
	int OBJFileCount;//OBJ���� ����
	ModelOBJ            g_model[OBJNUM];//���� OBJ��
	ModelTextures       g_modelTextures[OBJNUM];//���� ���ؽ���
	bool OBJModelLoaded;//OBJ���� �ε��ƴ�������
	bool g_enableTextures;//OBJ���Ͽ� �ؽ��ĸ�����������
	float               g_maxAnisotrophy;
	GLuint              g_nullTexture;

	GLuint CreateOBJNullTexture(int width, int height);
	void OBJFileUnLoad();
void OBJFileLoad();
void DrawOBJModel();
void DrawOBJModelUsingGPU(int ind);
void LoadOBJModel(const char *pszFilename);
GLuint LoadOBJFileTexture(const char *pszFilename);
void ReadNFLOWAnalHeightMap();
void DrawNFLOWAnalHeightMap();

	char WinInitDir[MAX_PATH];//�ʱ⼳���ȵ��丮
	AnalysisData *AnalyData;//�ؼ������� Ŭ�����ν��Ͻ�
	char trnfilename[200], wspfilename[200], vetfilename[200];//�ؼ����������ϸ�
	float AnalElapsedTime;//�ؼ�������ó���ҿ�ð�
	bool AnalysisDataImported, AnalAnimPause;
	int AnalDataColorCodingOn;//�ؼ�������Į���ڵ���Ÿ��(ANAL_WATERSTYLE:�����ѹ�,ANAL_VELOCITYSTYLE:����, ANAL_DEPTH:����, ANAL_ELEVATION:��)
	int AnalDataVectorsOn;//�ؼ������� ����ǥ�����
	int AnalBlockNum, AnalAnimCnt;
	float AnalStartX, AnalStartY, AnalCTC_X, AnalCTC_Y;//�ؼ������� ������ġ,����
	float Analspeedmin, Analspeedmax;//�ؼ������ͼӵ��� �ִ�, �ּҰ�
	float AnalSpeedmin, AnalSpeedmax, AnalDepthmin, AnalDepthmax, AnalElevationmin, AnalElevationmax;//�ؼ��������� �ִ�, �ּҰ�
	float heightmin, heightmax;//�������̸��� �ִ�, �ּҰ�
	vec3 TerrainCenter;//�������̸��� ����

	struct TerrainRegion
{
    float min;//�ּ�
    float max;//�ִ�
};

	TerrainRegion g_regions[4];

	float DropRadius;
	GLuint FluidHeightMapID;

	bool TerrainCreatedOK, AnalWaterSpeedOn, AnalWaterVelocityOn;
	bool HeightMapLoadingSmoothed;//�ؼ������͸� ���������� ����
	bool AlreadyAllPlayed;//�ؼ������͸��̸̹��Play�ߴ�������
	bool TerrainColorCodingOn;//����Į���ڵ�����
	float* WaterSpeed;
	vec2 *TexTerrainCoords;
	vec3 *FluidHeightMap, *TerrainHeightMap, *AnalHeightMap, *NFLOWTerrainHeightMap, *WaterVelocity;
	int heightmapwidth, heightmapheight, terrainwidth, terrainheight, Analwidth, Analheight, Analspeedwidth, Analspeedheight;

	int WMR, WHMR, WNMR;
	CShaderProgram WaterProgram, PoolSkyProgram, TexTerrainProgram, ColorCodingProgram, AnalWaterProgram, STLShadingProgram, OBJShadingProgram, OBJNormalMapProgram, TerrainColorCodingProgram;
	int QuadsVerticesCount, QuadsTerrainCount, QuadsAnalDataCount, NFLOWQuadsTerrainCount;

	float IndexOfRefraction;//��ü�� ������
	int FluidModelInd;//��ü�� �ε���
	int STLNum;//STL���ϰ���
	CTexture PoolSkyCubeMap;

	FILE *projfp;//������Ʈ���� open�ϰ� save�ϴ� �뵵
	bool m_Antialias;//��Ƽ���ϸ���� ���뿩��
	bool CameraPathFollowingOn;

	int ReflRefONOFF;
	GLfloat  ambiMat[4];
	GLfloat  diffMat[4];
	GLfloat  specMat[4];
	GLfloat  emisMat[4];
	GLfloat  coloMat[4];
	GLfloat shine;
	GLfloat Matl[13];
//Material �Ӽ���

	float xTrans, yTrans, zTrans;
	float xRot, yRot, zRot;
	float ScaleFactor;

	bool ViewObjMode;//0:VIEW MODE, 1:OBJECT MODE
		StlFile *stlFile[NumOfSTLOBJ];//STL FILE�� �о���̴� �ν��Ͻ�


		vec3 normalAtPixel(int x, int z, int hwidth, int hheight, vec3 *heights);
	float heightAtPixel(int x, int z, int hwidth, vec3 *heights) const
    { return heights[z * hwidth + x].y; };
	//���̸�ó��


		void DrawPoolSkyEnv();
		void ImportAnalData();
		void GetTrnTitle();
		void WaterLevelFileOpen();
		void WaterSpeedFileOpen();
		void DisplayElapsedTime( char* buf, int screenWidth, int screenHeight );
		void DisplayTimer( float fps, int screenWidth, int screenHeight );

		void DrawAnalysisDataAsWater();//�ؼ������͸�����ǥ��
		void DrawAnalysisDataColorCoding();//�ؼ�������Į���ڵ�
		void DrawAnalysisData();//�ؼ������͸� �׸����Լ�
		bool ReadAnalysisData();//�ؼ������͸� �д��Լ�

		void TerrainHeightMapFileOpen();
		void ReadTerrainHeightMap(char* trfilename);//������ ���̸��д��Լ�
		void ReadFluidHeightMap();//��ü�� ���̸��д��Լ�
		void DrawFluidHeightMap();//��ü�� ���̸��д��Լ�
		vec3 ReturnCodedColor(float col, float heightdiff);//������ Į���ڵ������ϴ��Լ�
		void PrintString(char *str);
		void DisplayString( int rasterPosX, int rasterPosY, char *str, int screenWidth, int screenHeight );	//ȭ�鿡 fps�� ��Ʈ������ ����ϴ� �Լ���
		void DisplayColorCodingRemark(int screenWidth, int screenHeight, float hmin, float hmax, int DispType);



		void DrawVectorsArrow(float x1, float y1, float z1, float x2, float y2, float z2, float csSize);
		void DisplayVelocityVector(vec3* thmap, vec3* velmap, float* speedmap, int twidth, int theight, float tmin, float tmax);
		void DisplayHeightMapVector(vec3* thmap, int twidth, int theight, float tmin, float tmax);//�ؼ������� ���ͱ׸����Լ�
		void DrawHeightMapVector(vec3 start, vec3 dir);//�������� ���⺤�͸� �̿��Ͽ� �ؼ��������� ����ǥ���ϴ��Լ�
		void DrawTerrainColorCoding();//������ Į���ڵ�����
		void DrawTerrain();//�����׸���
		void BindTexture(GLuint texture, GLuint unit);
		void UpdateTerrainShaderParameters();//Terrain���̴� �Ķ���ͼ����ϴ� �Լ�
		void DrawTerrainHeightMap(bool DisplayRemarkOK = true);


		void AddDrop(float x, float y, float DropRadius);
		void DrawEnvMappedHeightMap();//��ü���̸ʱ׸���

		void CameraFileOpen();
		void CameraFileSave();
		void TextureFileLoad();//�ؽ��������� �о���̴� �Լ�
		void ProjectFileOpen();//������Ʈ������ �о���̴��Լ�
		void ProjectFileSave();//������Ʈ������ �о���̴��Լ�
		void ModelFileOpen();//�������� �о���̴��Լ�
		void DrawXYZAxis();//XYZ��׸���

		int supportsOneDotWhatever(int whatever);
void initLoaderForDSS();
void loadCubeMapFromDDS(const char *filename);
void drawDDSEnvMap();

		void ApplyMaterial(const int &);
void AddMaterial(float*);
void DefineDisplay(int num);
void DrawShaded(int num);


		void DrawSTLFluid();//STL FILE ��ü�� �׸��� �Լ�
		void DrawSTLObjects();//STL FILE ��ü �׸��� �Լ�
	void InitEnvironment();
	void DrawEnvironmentBox();
	void DebugSTLFile(int num);

	FILE *defp;//����׿�

	bool Pause, WireFrame, ViewEnv;
	vec3 LightColor, LightPosition;

public:
	CCamera Camera;

	MCString Text;

public:
	COpenGLRenderer();
	~COpenGLRenderer();

	bool AllInit();
	void Render(float FrameTime);
	void Resize(int Width, int Height);
	void Destroy();

	void SetWaveInShaders(int Wave);
};

#endif
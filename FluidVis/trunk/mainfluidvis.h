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
#define MINDELTA 0.1//소량회전량

class CCamera
{
public:
	vec3 X, Y, Z, Position, Reference;

	mat4x4 *ViewMatrix, *ViewMatrixInverse;

	mat4x4 SavingViewMatrix[MAXCAMERAPATH];
	float CameraAnimStep[MAXCAMERAPATH];//카메라스텝
	bool CameraAnimPause;//카메라애니메이션정지여부
	int CameraPathNum;//총 카메라 경로 갯수
	int CurCameraPath;//현재 카메라 경로
	float AnimatingParam;
	bool CameraPathAnimationOn;
	bool DrawPerspecOrOrtho;

	CCamera();
	~CCamera();

	void SetReferencePoint(vec3 hcenter);//카메라가 회전할때의 기준점 설정
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
#include "stlfile.h"//stl파일처리헤더
#include "GLTrihedron.h"//화살표처리헤더
#include "landscapecreator.h"


//OBJ파일처리
#include "model_obj.h"
#include "bitmap.h"
#include <map>
typedef std::map<std::string, GLuint> ModelTextures;//남중 모델텍스쳐


#define NumOfSTLOBJ 4//STL OBJECT 갯수

#define ANAL_WATERSTYLE 0//해석데이터 물스타일
#define ANAL_VELOCITYSTYLE 1//해석데이터 유속
#define ANAL_DEPTH 2//해석데이터 수심
#define ANAL_ELEVATION 3//해석데이터 고도
#define ANAL_STREAMLINE 5//해석데이터 스트림라인표현
#define ANAL_VECTOR 6//해석데이터 벡터표현
#define TERRAIN_HEIGHT 4//지형의 높이

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
	int MappingTextureExist;//Terrain에 매핑할 텍스쳐의 존재여부
	CTexture AnalNormalMapTex;//해석데이터 WATER에 적용할 법선맵 텍스쳐
	CTexture MappingTexture;//Terrain에 매핑할 텍스쳐
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
	int OBJFileCount;//OBJ파일 갯수
	ModelOBJ            g_model[OBJNUM];//남중 OBJ모델
	ModelTextures       g_modelTextures[OBJNUM];//남중 모델텍스쳐
	bool OBJModelLoaded;//OBJ파일 로딩됐는지여부
	bool g_enableTextures;//OBJ파일에 텍스쳐매핑할지여부
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

	char WinInitDir[MAX_PATH];//초기설정된디렉토리
	AnalysisData *AnalyData;//해석데이터 클래스인스턴스
	char trnfilename[200], wspfilename[200], vetfilename[200];//해석데이터파일명
	float AnalElapsedTime;//해석데이터처리소요시간
	bool AnalysisDataImported, AnalAnimPause;
	int AnalDataColorCodingOn;//해석데이터칼라코딩스타일(ANAL_WATERSTYLE:투명한물,ANAL_VELOCITYSTYLE:유속, ANAL_DEPTH:수심, ANAL_ELEVATION:고도)
	int AnalDataVectorsOn;//해석데이터 벡터표현방식
	int AnalBlockNum, AnalAnimCnt;
	float AnalStartX, AnalStartY, AnalCTC_X, AnalCTC_Y;//해석데이터 시작위치,간격
	float Analspeedmin, Analspeedmax;//해석데이터속도의 최대, 최소값
	float AnalSpeedmin, AnalSpeedmax, AnalDepthmin, AnalDepthmax, AnalElevationmin, AnalElevationmax;//해석데이터의 최대, 최소값
	float heightmin, heightmax;//지형높이맵의 최대, 최소값
	vec3 TerrainCenter;//지형높이맵의 중점

	struct TerrainRegion
{
    float min;//최소
    float max;//최대
};

	TerrainRegion g_regions[4];

	float DropRadius;
	GLuint FluidHeightMapID;

	bool TerrainCreatedOK, AnalWaterSpeedOn, AnalWaterVelocityOn;
	bool HeightMapLoadingSmoothed;//해석데이터를 스무딩할지 여부
	bool AlreadyAllPlayed;//해석데이터를이미모두Play했는지여부
	bool TerrainColorCodingOn;//지형칼라코딩여부
	float* WaterSpeed;
	vec2 *TexTerrainCoords;
	vec3 *FluidHeightMap, *TerrainHeightMap, *AnalHeightMap, *NFLOWTerrainHeightMap, *WaterVelocity;
	int heightmapwidth, heightmapheight, terrainwidth, terrainheight, Analwidth, Analheight, Analspeedwidth, Analspeedheight;

	int WMR, WHMR, WNMR;
	CShaderProgram WaterProgram, PoolSkyProgram, TexTerrainProgram, ColorCodingProgram, AnalWaterProgram, STLShadingProgram, OBJShadingProgram, OBJNormalMapProgram, TerrainColorCodingProgram;
	int QuadsVerticesCount, QuadsTerrainCount, QuadsAnalDataCount, NFLOWQuadsTerrainCount;

	float IndexOfRefraction;//유체의 굴절률
	int FluidModelInd;//유체모델 인덱스
	int STLNum;//STL파일갯수
	CTexture PoolSkyCubeMap;

	FILE *projfp;//프로젝트파일 open하고 save하는 용도
	bool m_Antialias;//안티에일리어싱 적용여부
	bool CameraPathFollowingOn;

	int ReflRefONOFF;
	GLfloat  ambiMat[4];
	GLfloat  diffMat[4];
	GLfloat  specMat[4];
	GLfloat  emisMat[4];
	GLfloat  coloMat[4];
	GLfloat shine;
	GLfloat Matl[13];
//Material 속성들

	float xTrans, yTrans, zTrans;
	float xRot, yRot, zRot;
	float ScaleFactor;

	bool ViewObjMode;//0:VIEW MODE, 1:OBJECT MODE
		StlFile *stlFile[NumOfSTLOBJ];//STL FILE을 읽어들이는 인스턴스


		vec3 normalAtPixel(int x, int z, int hwidth, int hheight, vec3 *heights);
	float heightAtPixel(int x, int z, int hwidth, vec3 *heights) const
    { return heights[z * hwidth + x].y; };
	//높이맵처리


		void DrawPoolSkyEnv();
		void ImportAnalData();
		void GetTrnTitle();
		void WaterLevelFileOpen();
		void WaterSpeedFileOpen();
		void DisplayElapsedTime( char* buf, int screenWidth, int screenHeight );
		void DisplayTimer( float fps, int screenWidth, int screenHeight );

		void DrawAnalysisDataAsWater();//해석데이터를물로표현
		void DrawAnalysisDataColorCoding();//해석데이터칼라코딩
		void DrawAnalysisData();//해석데이터를 그리는함수
		bool ReadAnalysisData();//해석데이터를 읽는함수

		void TerrainHeightMapFileOpen();
		void ReadTerrainHeightMap(char* trfilename);//지형의 높이맵읽는함수
		void ReadFluidHeightMap();//유체의 높이맵읽는함수
		void DrawFluidHeightMap();//유체의 높이맵읽는함수
		vec3 ReturnCodedColor(float col, float heightdiff);//정점의 칼라코딩색상구하는함수
		void PrintString(char *str);
		void DisplayString( int rasterPosX, int rasterPosY, char *str, int screenWidth, int screenHeight );	//화면에 fps를 비트맵으로 출력하는 함수들
		void DisplayColorCodingRemark(int screenWidth, int screenHeight, float hmin, float hmax, int DispType);



		void DrawVectorsArrow(float x1, float y1, float z1, float x2, float y2, float z2, float csSize);
		void DisplayVelocityVector(vec3* thmap, vec3* velmap, float* speedmap, int twidth, int theight, float tmin, float tmax);
		void DisplayHeightMapVector(vec3* thmap, int twidth, int theight, float tmin, float tmax);//해석데이터 벡터그리는함수
		void DrawHeightMapVector(vec3 start, vec3 dir);//시작점과 방향벡터를 이용하여 해석데이터의 벡터표현하는함수
		void DrawTerrainColorCoding();//지형에 칼라코딩적용
		void DrawTerrain();//지형그리기
		void BindTexture(GLuint texture, GLuint unit);
		void UpdateTerrainShaderParameters();//Terrain쉐이더 파라미터설정하는 함수
		void DrawTerrainHeightMap(bool DisplayRemarkOK = true);


		void AddDrop(float x, float y, float DropRadius);
		void DrawEnvMappedHeightMap();//유체높이맵그리기

		void CameraFileOpen();
		void CameraFileSave();
		void TextureFileLoad();//텍스쳐파일을 읽어들이는 함수
		void ProjectFileOpen();//프로젝트파일을 읽어들이는함수
		void ProjectFileSave();//프로젝트파일을 읽어들이는함수
		void ModelFileOpen();//모델파일을 읽어들이는함수
		void DrawXYZAxis();//XYZ축그리기

		int supportsOneDotWhatever(int whatever);
void initLoaderForDSS();
void loadCubeMapFromDDS(const char *filename);
void drawDDSEnvMap();

		void ApplyMaterial(const int &);
void AddMaterial(float*);
void DefineDisplay(int num);
void DrawShaded(int num);


		void DrawSTLFluid();//STL FILE 유체를 그리는 함수
		void DrawSTLObjects();//STL FILE 객체 그리는 함수
	void InitEnvironment();
	void DrawEnvironmentBox();
	void DebugSTLFile(int num);

	FILE *defp;//디버그용

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
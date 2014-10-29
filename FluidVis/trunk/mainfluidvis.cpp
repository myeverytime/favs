#include "stdafx.h"
#include "mainfluidvis.h"

// ----------------------------------------------------------------------------------------------------------------------------

CBuffer::CBuffer()
{
	SetDefaults();
}

CBuffer::~CBuffer()
{
	Empty();
}

void CBuffer::AddData(void *Data, int DataSize)
{
	int Remaining = BufferSize - Position;

	if(DataSize > Remaining)
	{
		BYTE *OldBuffer = Buffer;
		int OldBufferSize = BufferSize;

		int Needed = DataSize - Remaining;

		BufferSize += Needed > BUFFER_SIZE_INCREMENT ? Needed : BUFFER_SIZE_INCREMENT;

		Buffer = new BYTE[BufferSize];

		memcpy(Buffer, OldBuffer, OldBufferSize);

		delete [] OldBuffer;
	}

	memcpy(Buffer + Position, Data, DataSize);

	Position += DataSize;
}

void CBuffer::Empty()
{
	delete [] Buffer;

	SetDefaults();
}

void *CBuffer::GetData()
{
	return Buffer;
}

int CBuffer::GetDataSize()
{
	return Position;
}

void CBuffer::SetDefaults()
{
	Buffer = NULL;

	BufferSize = 0;
	Position = 0;
}

// ----------------------------------------------------------------------------------------------------------------------------

CTexture::CTexture()
{
	Texture = 0;
}

CTexture::~CTexture()
{
}

CTexture::operator GLuint ()
{
	return Texture;
}

bool CTexture::LoadTexture2D(char *FileName)
{
	MCString DirectoryFileName = FileName;

	int Width, Height, BPP;

	FIBITMAP *dib = GetBitmap(DirectoryFileName, Width, Height, BPP);

	if(dib == NULL)
	{
		ErrorLog.Append("Error loading texture " + DirectoryFileName + "!\r\n");
		return false;
	}

	GLenum Format = 0;

	if(BPP == 32) Format = GL_BGRA;
	if(BPP == 24) Format = GL_BGR;

	if(Format == 0)
	{
		ErrorLog.Append("Unsupported texture format (%s)!\r\n", FileName);
		FreeImage_Unload(dib);
		return false;
	}

	Destroy();

	glGenTextures(1, &Texture);

	glBindTexture(GL_TEXTURE_2D, Texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if(GLEW_EXT_texture_filter_anisotropic)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, ggl_max_texture_max_anisotropy_ext);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Width, Height, 0, Format, GL_UNSIGNED_BYTE, FreeImage_GetBits(dib));

	glBindTexture(GL_TEXTURE_2D, 0);

	FreeImage_Unload(dib);

	return true;
}

bool CTexture::LoadTextureCubeMap(char **FileNames)
{
	int Width, Height, BPP;

	FIBITMAP *dib[6];

	bool Error = false;
	
	for(int i = 0; i < 6; i++)
	{
		MCString DirectoryFileName = FileNames[i];

		dib[i] = GetBitmap(DirectoryFileName, Width, Height, BPP);

		if(dib[i] == NULL)
		{
			ErrorLog.Append("Error loading texture " + DirectoryFileName + "!\r\n");
			Error = true;
		}
	}

	if(Error)
	{
		for(int i = 0; i < 6; i++)
		{
			FreeImage_Unload(dib[i]);
		}

		return false;
	}

	GLenum Format = 0;
	
	if(BPP == 32) Format = GL_BGRA;
	if(BPP == 24) Format = GL_BGR;

	if(Format == 0)
	{
		ErrorLog.Append("Unsupported texture format (%s)!\r\n", FileNames[5]);

		for(int i = 0; i < 6; i++)
		{
			FreeImage_Unload(dib[i]);
		}

		return false;
	}

	Destroy();

	glGenTextures(1, &Texture);

	glBindTexture(GL_TEXTURE_CUBE_MAP, Texture);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if(GLEW_EXT_texture_filter_anisotropic)
	{
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_ANISOTROPY_EXT, ggl_max_texture_max_anisotropy_ext);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_GENERATE_MIPMAP, GL_TRUE);

	for(int i = 0; i < 6; i++)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA8, Width, Height, 0, Format, GL_UNSIGNED_BYTE, FreeImage_GetBits(dib[i]));
	}

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	for(int i = 0; i < 6; i++)
	{
		FreeImage_Unload(dib[i]);
	}

	return true;
}

void CTexture::Destroy()
{
	glDeleteTextures(1, &Texture);
	Texture = 0;
}

FIBITMAP *CTexture::GetBitmap(char *FileName, int &Width, int &Height, int &BPP)
{
	FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(FileName);

	if(fif == FIF_UNKNOWN)
	{
		fif = FreeImage_GetFIFFromFilename(FileName);
	}

	if(fif == FIF_UNKNOWN)
	{
		return NULL;
	}

	FIBITMAP *dib = NULL;

	if(FreeImage_FIFSupportsReading(fif))
	{
		dib = FreeImage_Load(fif, FileName);
	}

	if(dib != NULL)
	{
		int OriginalWidth = FreeImage_GetWidth(dib);
		int OriginalHeight = FreeImage_GetHeight(dib);

		Width = OriginalWidth;
		Height = OriginalHeight;

		if(Width == 0 || Height == 0)
		{
			FreeImage_Unload(dib);
			return NULL;
		}

		BPP = FreeImage_GetBPP(dib);

		if(Width > ggl_max_texture_size) Width = ggl_max_texture_size;
		if(Height > ggl_max_texture_size) Height = ggl_max_texture_size;

		if(!GLEW_ARB_texture_non_power_of_two)
		{
			Width = 1 << (int)floor((log((float)Width) / log(2.0f)) + 0.5f); 
			Height = 1 << (int)floor((log((float)Height) / log(2.0f)) + 0.5f);
		}

		if(Width != OriginalWidth || Height != OriginalHeight)
		{
			FIBITMAP *rdib = FreeImage_Rescale(dib, Width, Height, FILTER_BICUBIC);
			FreeImage_Unload(dib);
			dib = rdib;
		}
	}

	return dib;
}

// ----------------------------------------------------------------------------------------------------------------------------

CShaderProgram::CShaderProgram()
{
	SetDefaults();
}

CShaderProgram::~CShaderProgram()
{
}

CShaderProgram::operator GLuint ()
{
	return Program;
}

bool CShaderProgram::Load(char *VertexShaderFileName, char *FragmentShaderFileName)
{
	bool Error = false;

	Destroy();

	Error |= ((VertexShader = LoadShader(VertexShaderFileName, GL_VERTEX_SHADER)) == 0);
	Error |= ((FragmentShader = LoadShader(FragmentShaderFileName, GL_FRAGMENT_SHADER)) == 0);

	if(Error)
	{
		Destroy();
		return false;
	}

	Program = glCreateProgram();
	glAttachShader(Program, VertexShader);
	glAttachShader(Program, FragmentShader);
	glLinkProgram(Program);

	int LinkStatus;
	glGetProgramiv(Program, GL_LINK_STATUS, &LinkStatus);

	if(LinkStatus == GL_FALSE)
	{
		ErrorLog.Append("Error linking program (%s, %s)!\r\n", VertexShaderFileName, FragmentShaderFileName);

		int InfoLogLength = 0;
		glGetProgramiv(Program, GL_INFO_LOG_LENGTH, &InfoLogLength);
	
		if(InfoLogLength > 0)
		{
			char *InfoLog = new char[InfoLogLength];
			int CharsWritten  = 0;
			glGetProgramInfoLog(Program, InfoLogLength, &CharsWritten, InfoLog);
			ErrorLog.Append(InfoLog);
			delete [] InfoLog;
		}

		Destroy();

		return false;
	}

	return true;
}

void CShaderProgram::Destroy()
{
	glDetachShader(Program, VertexShader);
	glDetachShader(Program, FragmentShader);

	glDeleteShader(VertexShader);
	glDeleteShader(FragmentShader);

	glDeleteProgram(Program);

	delete [] UniformLocations;
	delete [] AttribLocations;

	SetDefaults();
}

GLuint CShaderProgram::LoadShader(char *FileName, GLenum Type)
{
	MCString DirectoryFileName = FileName;

	FILE *File;

	if(fopen_s(&File, DirectoryFileName, "rb") != 0)
	{
		ErrorLog.Append("Error loading file " + DirectoryFileName + "!\r\n");
		return 0;
	}

	fseek(File, 0, SEEK_END);
	long Size = ftell(File);
	fseek(File, 0, SEEK_SET);
	char *Source = new char[Size + 1];
	fread(Source, 1, Size, File);
	fclose(File);
	Source[Size] = 0;

	GLuint Shader = glCreateShader(Type);

	glShaderSource(Shader, 1, (const char**)&Source, NULL);
	delete [] Source;
	glCompileShader(Shader);

	int CompileStatus;
	glGetShaderiv(Shader, GL_COMPILE_STATUS, &CompileStatus);

	if(CompileStatus == GL_FALSE)
	{
		ErrorLog.Append("Error compiling shader %s!\r\n", FileName);

		int InfoLogLength = 0;
		glGetShaderiv(Shader, GL_INFO_LOG_LENGTH, &InfoLogLength);
	
		if(InfoLogLength > 0)
		{
			char *InfoLog = new char[InfoLogLength];
			int CharsWritten  = 0;
			glGetShaderInfoLog(Shader, InfoLogLength, &CharsWritten, InfoLog);
			ErrorLog.Append(InfoLog);
			delete [] InfoLog;
		}

		glDeleteShader(Shader);

		return 0;
	}

	return Shader;
}

void CShaderProgram::SetDefaults()
{
	VertexShader = 0;
	FragmentShader = 0;

	Program = 0;

	UniformLocations = NULL;
	AttribLocations = NULL;
}

// ----------------------------------------------------------------------------------------------------------------------------

CCamera::CCamera()
{
	ViewMatrix = NULL;
	ViewMatrixInverse = NULL;

	for(int i=0; i<MAXCAMERAPATH; i++)
		CameraAnimStep[i] = DEFAULTCAMERAANIMSTEP;

	CameraAnimPause = true;
	CameraPathAnimationOn = false;
	CameraPathNum = 0;
	CurCameraPath = 0;

	DrawPerspecOrOrtho = true;

	X = vec3(1.0f, 0.0f, 0.0f);
	Y = vec3(0.0f, 1.0f, 0.0f);
	Z = vec3(0.0f, 0.0f, 1.0f);

	Position = vec3(0.0f, 0.0f, 5.0f);
	Reference = vec3(0.0f, 0.0f, 0.0f);
}

CCamera::~CCamera()
{
}

void CCamera::SetPersOrtho()
{
	float ORTHOLEN = 1.0;
	DrawPerspecOrOrtho = !DrawPerspecOrOrtho;
	if(DrawPerspecOrOrtho)
		gluPerspective(45, (GLfloat)WIN_WIDTH/(GLfloat)WIN_HEIGHT, 0.1f, 100000.0f);
	else
		gluOrtho2D(-ORTHOLEN, ORTHOLEN, -ORTHOLEN, ORTHOLEN);
}

void CCamera::SetReferencePoint(vec3 hcenter)
{//카메라가 회전할때의 기준점 설정
	Reference = hcenter;
}

void CCamera::Look(const vec3 &Position, const vec3 &Reference, bool RotateAroundReference)
{
	this->Position = Position;
	this->Reference = Reference;

	Z = normalize(Position - Reference);
	X = normalize(cross(vec3(0.0f, 1.0f, 0.0f), Z));
	Y = cross(Z, X);

	if(!RotateAroundReference)
	{
		this->Reference = this->Position;
		this->Position += Z * 0.05f;
	}

	CalculateViewMatrix();
}

void CCamera::Move(const vec3 &Movement)
{
	Position += Movement*10;
	Reference += Movement*10;

	CalculateViewMatrix();
}

vec3 CCamera::OnKeys(BYTE Keys, float FrameTime)
{
	float Speed = 10.0f;

	if(Keys & 0x40) Speed *= 2.0f;
	if(Keys & 0x80) Speed *= 0.5f;

	float Distance = Speed * FrameTime;

	vec3 Up(0.0f, 1.0f, 0.0f);
	vec3 Right = X;
	vec3 Forward = cross(Up, Right);

	Up *= Distance;
	Right *= Distance;
	Forward *= Distance;

	vec3 Movement;

	if(Keys & 0x01) Movement += Forward;//w키
	if(Keys & 0x02) Movement -= Forward;//s키
	if(Keys & 0x10) Movement += Up;//r키
	if(Keys & 0x20) Movement -= Up;//f키
	if(Keys & 0x04) Movement -= Right;//a키
	if(Keys & 0x08) Movement += Right;//d키

	return Movement;
}

vec3 CCamera::MoveByMouseMButton(int dx, int dy)
{
	vec3 Up(0.0f, 1.0f, 0.0f);
	vec3 Right = X;
	vec3 Forward = cross(Up, Right);

	float Distance = 0.05f;

	Up *= Distance;
	Right *= Distance;
	Forward *= Distance;

	vec3 Movement;

	Movement += Up*-dy;//r키
	Movement += Right*dx;//a키

	return Movement;
}

void CCamera::RotateAboutLeftRight(float DELTA)
{//카메라를 X축에대해 회전시키는 함수
	Position -= Reference;

	Y = rotate(Y, DELTA, X);//X축에대해 회전
	Z = rotate(Z, DELTA, X);//X축에대해 회전

	Position = Reference + Z * length(Position);

	CalculateViewMatrix();
}

void CCamera::RotateAboutUpDown(float DELTA)
{//카메라를 Y축에대해 회전시키는 함수
	Position -= Reference;

	X = rotate(X, DELTA, Y);//Y축에대해 회전
	Z = rotate(Z, DELTA, Y);//Y축에대해 회전

	Position = Reference + Z * length(Position);

	CalculateViewMatrix();
}

#define DeltaRot 2.0
void CCamera::RotateAboutXAxis(float DRot)
{//카메라를 X축에대해 회전시키는 함수
	Position -= Reference;

	Y = rotate(Y, DRot, X);//X축에대해 회전
	Z = rotate(Z, DRot, X);//X축에대해 회전

	Position = Reference + Z * length(Position);

	CalculateViewMatrix();
}

void CCamera::RotateAboutYAxis(float DRot)
{//카메라를 Y축에대해 회전시키는 함수
	Position -= Reference;

	X = rotate(X, DRot, Y);//Y축에대해 회전
	Z = rotate(Z, DRot, Y);//Y축에대해 회전

	Position = Reference + Z * length(Position);

	CalculateViewMatrix();
}

void CCamera::RotateAboutZAxis()
{//카메라를 Z축에대해 회전시키는 함수
	Position -= Reference;

	X = rotate(X, DeltaRot, Z);
	Y = rotate(Y, DeltaRot, Z);

	Position = Reference + Z * length(Position);

	CalculateViewMatrix();
}

void CCamera::RotateToXZPlane()
{//XZ면으로 카메라 회전
}

void CCamera::RotateToYZPlane()
{//YZ면으로 카메라 회전
}

void CCamera::RotateToXYPlane()
{//XY면으로 카메라 회전
}

void CCamera::OnMouseMove(int dx, int dy)
{
	float Sensitivity = 0.25f;

	Position -= Reference;

	if(dx != 0)
	{
		float DeltaX = (float)dx * Sensitivity;

		X = rotate(X, DeltaX, vec3(0.0f, 0.0f, 1.0f));
		Y = rotate(Y, DeltaX, vec3(0.0f, 0.0f, 1.0f));
		Z = rotate(Z, DeltaX, vec3(0.0f, 0.0f, 1.0f));
	}

	if(dy != 0)
	{
		float DeltaY = (float)dy * Sensitivity;

		Y = rotate(Y, DeltaY, X);
		Z = rotate(Z, DeltaY, X);
	}

	Position = Reference + Z * length(Position);

	CalculateViewMatrix();
}

void CCamera::OnMouseWheel(float zDelta)
{
	Position -= Reference;

	if(zDelta < 0 && length(Position) < 5000.0f)
	{
		Position += Position * 0.1f;
	}

	if(zDelta > 0 && length(Position) > 0.05f)
	{
		Position -= Position * 0.1f;
	}

	Position += Reference;

	CalculateViewMatrix();
}

void CCamera::SetViewMatrixPointer(float *ViewMatrix, float *ViewMatrixInverse)
{
	this->ViewMatrix = (mat4x4*)ViewMatrix;
	this->ViewMatrixInverse = (mat4x4*)ViewMatrixInverse;

	CalculateViewMatrix();
}

void CCamera::CalculateViewMatrix()
{
	if(ViewMatrix != NULL)
	{
		*ViewMatrix = mat4x4(X.x, Y.x, Z.x, 0.0f, X.y, Y.y, Z.y, 0.0f, X.z, Y.z, Z.z, 0.0f, -dot(X, Position), -dot(Y, Position), -dot(Z, Position), 1.0f);

		if(ViewMatrixInverse != NULL)
		{
			*ViewMatrixInverse = inverse(*ViewMatrix);
		}
	}
}

void CCamera::GetSavingViewMatrix()
{
	SavingViewMatrix[CameraPathNum] = *ViewMatrix;
//	SavingViewMatrixInverse[CameraPathNum] = *ViewMatrixInverse;
}

void CCamera::SetSavingViewMatrix()
{
	*ViewMatrix = SavingViewMatrix[CameraPathNum];
//	*ViewMatrixInverse = SavingViewMatrixInverse[CameraPathNum];
}

void CCamera::SetAnimatingViewMatrix()
{//보간을 이용한 카메라 경로 애니메이션 구현
	int i;
	if(CurCameraPath < CameraPathNum-1)
	{
		if(AnimatingParam <= 1.0)
		{
			for(i=0; i<16;i++)
			{
				ViewMatrix->M[i] = SavingViewMatrix[CurCameraPath].M[i] + (SavingViewMatrix[CurCameraPath+1].M[i]-SavingViewMatrix[CurCameraPath].M[i])*AnimatingParam;//선형보간
//				ViewMatrix->M[i] = sin((1-AnimatingParam)*M_PI*0.5)/sin(M_PI*0.5)*SavingViewMatrix[CurCameraPath].M[i] + sin(AnimatingParam*M_PI*0.5)/sin(M_PI*0.5)*SavingViewMatrix[CurCameraPath+1].M[i];//구면선형보간
			}
			AnimatingParam += CameraAnimStep[CurCameraPath];
		}
		else
		{
			if(AnimatingParam > 1.0)
			{
				AnimatingParam = 1.0;
				for(i=0; i<16;i++)
					ViewMatrix->M[i] = SavingViewMatrix[CurCameraPath].M[i] + (SavingViewMatrix[CurCameraPath+1].M[i]-SavingViewMatrix[CurCameraPath].M[i])*AnimatingParam;//선형보간
			}
			AnimatingParam = 0.0;
			CurCameraPath++;
		}
	}
	else
	{
		CameraPathAnimationOn = false;
		CameraAnimPause = true;
	}
}

// ----------------------------------------------------------------------------------------------------------------------------

CWave::CWave()
{
	Speed = 1.0f;
	FrequencyMPIM2 = 4.0f * (float)M_PI * 2.0f;
}

CWave::~CWave()
{
}

// ----------------------------------------------------------------------------------------------------------------------------
	const float     HEIGHTMAP_TILING_FACTOR = 20.0f;//남중 타일링팩터
	const float     HEIGHTMAP_SCALE = 1.0f;//높이값에 따라 섞이는 텍스쳐값

COpenGLRenderer::COpenGLRenderer()
{
	GetCurrentDirectory(MAX_PATH, WinInitDir);

	Pause = false;
	WireFrame = false;

	g_enableTextures = true;

	OBJFileCount = -1;
	OBJModelLoaded = false;

	TerrainCreatedOK = false;
	HeightMapLoadingSmoothed = false;
	AlreadyAllPlayed = false;
	AnalElapsedTime = 0.0;
	AnalAnimPause = false;
	AnalysisDataImported = false;
	AnalBlockNum = 0;
	AnalAnimCnt = 0;
	AnalDataColorCodingOn = 0;
	AnalDataVectorsOn = 0;
	AnalWaterSpeedOn = false;
	AnalWaterVelocityOn = false;

	TerrainColorCodingOn = false;
	m_Antialias = true;
	ViewObjMode = false;
	ViewEnv = true;
	STLNum = 0;
	FluidModelInd = -1;
	MappingTextureExist = 0;

	xTrans = 0.0;
	yTrans = 0.0;
	zTrans = 0.0;
	xRot = -90.0;
	yRot = 0.0;
	zRot = 0.0;
	ScaleFactor = 1.0;

	WaterLevel = 0.5f;
	IndexOfRefraction = 1.1;

	Wave = 0;

	ReflRefONOFF = 0;

	
	WHMID = 0;

	DropRadius = 400.0f / 128.0f;

	Camera.SetViewMatrixPointer(&ViewMatrix, &ViewMatrixInverse);
}

COpenGLRenderer::~COpenGLRenderer()
{
}

void COpenGLRenderer::WaterLevelFileOpen()
{//해석데이터의 수위파일명을 얻는 함수
	char szFilter[] = "(*.TRN) | *.TRN| All Files(*.*)|*.*||";
	CString cstrfilepath;
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, szFilter);
	if(IDOK == dlg.DoModal())
	{
		cstrfilepath = dlg.GetPathName();
		strcpy(trnfilename, LPSTR(LPCTSTR(cstrfilepath)));
	}
	else
	{
		return;
	}
}

void COpenGLRenderer::WaterSpeedFileOpen()
{//해석데이터의 유속을 얻는 함수
	char szFilter[] = "(*.WSP;*.VET)|*.WSP; *.VET| All Files(*.*)|*.*||";
	CString cstrfilepath;
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, szFilter);
	if(IDOK == dlg.DoModal())
	{
		cstrfilepath = dlg.GetPathName();
		if(dlg.GetFileExt().CompareNoCase("WSP")==0)
		{
			strcpy(wspfilename, LPSTR(LPCTSTR(cstrfilepath)));
			AnalWaterVelocityOn = false;
			AnalWaterSpeedOn = true;
		}
		else if(dlg.GetFileExt().CompareNoCase("VET")==0)
		{
			strcpy(vetfilename, LPSTR(LPCTSTR(cstrfilepath)));
			AnalWaterSpeedOn = true;
			AnalWaterVelocityOn = true;
		}
	}
	else
	{
		return;
	}
}

void COpenGLRenderer::CameraFileSave()
{//카메라파일을 저장하는 함수
	char szFilter[] = "(*.CAM) | *.CAM| All Files(*.*)|*.*||";
	CString cstrfilepath;
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, szFilter);
	if(IDOK == dlg.DoModal())
	{
		cstrfilepath = dlg.GetPathName();

		projfp = fopen(LPSTR(LPCTSTR(cstrfilepath)), "w");
		//카메라경로저장4*4카메라행렬저장
		fprintf(projfp, "%d\n", Camera.CameraPathNum);
		for(int i=0;i<Camera.CameraPathNum;i++)
		{
			fprintf(projfp, "%f\n", Camera.CameraAnimStep[i]);
			for(int j=0; j<16; j++)
				fprintf(projfp, "%f ", Camera.SavingViewMatrix[i].M[j]);
			fprintf(projfp, "\n");
		}

		fclose(projfp);
	}
	else
	{
		return;
	}
}

void COpenGLRenderer::CameraFileOpen()
{//프로젝트파일을 읽어들이는 함수
	char szFilter[] = "(*.CAM) | *.CAM| All Files(*.*)|*.*||";
	CString cstrfilepath;
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, szFilter);
	if(IDOK == dlg.DoModal())
	{
		cstrfilepath = dlg.GetPathName();

		projfp = fopen(LPSTR(LPCTSTR(cstrfilepath)), "r");
		//카메라경로저장4*4카메라행렬저장
		fscanf(projfp, "%d", &Camera.CameraPathNum);
		for(int i=0;i<Camera.CameraPathNum;i++)
		{
			fscanf(projfp, "%f", &Camera.CameraAnimStep[i]);
			for(int j=0; j<16; j++)
				fscanf(projfp, "%f", &Camera.SavingViewMatrix[i].M[j]);
		}

		fclose(projfp);
	}
	else
	{
		return;
	}
}

void COpenGLRenderer::TerrainHeightMapFileOpen()
{//지형높이맵파일을 읽어들이는 함수
	char szFilter[] = "(*.TRN) | *.TRN| All Files(*.*)|*.*||";
	CString cstrfilepath;
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, szFilter);
	if(IDOK == dlg.DoModal())
	{
		cstrfilepath = dlg.GetPathName();
	}
	else
	{
		return;
	}

	ReadTerrainHeightMap(LPSTR(LPCTSTR(cstrfilepath)));
}

void COpenGLRenderer::TextureFileLoad()
{//텍스쳐파일을 읽어들이는 함수
	char szFilter[] = "(*.JPG) | *.JPG| All Files(*.*)|*.*||";
	CString cstrfilepath;
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, szFilter);
	if(IDOK == dlg.DoModal())
	{
		cstrfilepath = dlg.GetPathName();
		MappingTexture.LoadTexture2D(LPSTR(LPCTSTR(cstrfilepath)));
		MappingTextureExist = 1;
	}
	else
	{
		return;
	}
}

GLuint COpenGLRenderer::CreateOBJNullTexture(int width, int height)
{
    // Create an empty white texture. This texture is applied to OBJ models
    // that don't have any texture maps. This trick allows the same shader to
    // be used to draw the OBJ model with and without textures applied.

    int pitch = ((width * 32 + 31) & ~31) >> 3; // align to 4-byte boundaries
    std::vector<GLubyte> pixels(pitch * height, 255);
    GLuint texture = 0;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_BGRA,
        GL_UNSIGNED_BYTE, &pixels[0]);

    return texture;
}

GLuint COpenGLRenderer::LoadOBJFileTexture(const char *pszFilename)
{//OBJ파일에 텍스쳐매핑하는 함수
    GLuint id = 0;
    Bitmap bitmap;

	g_maxAnisotrophy = 1.0f;

    if (bitmap.loadPicture((LPCTSTR)pszFilename))
    {
        // The Bitmap class loads images and orients them top-down.
        // OpenGL expects bitmap images to be oriented bottom-up.
        bitmap.flipVertical();

        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        if (g_maxAnisotrophy > 1.0f)
        {
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT,
                g_maxAnisotrophy);
        }

        gluBuild2DMipmaps(GL_TEXTURE_2D, 4, bitmap.width, bitmap.height,
            GL_BGRA_EXT, GL_UNSIGNED_BYTE, bitmap.getPixels());
    }

    return id;
}

void COpenGLRenderer::LoadOBJModel(const char *pszFilename)
{//남중 OBJ파일 열기
    // Import the OBJ file and normalize to unit length.

    SetCursor(LoadCursor(0, IDC_WAIT));

	OBJFileCount++;

	if (!(g_nullTexture = CreateOBJNullTexture(2, 2)))
		throw std::runtime_error("Failed to create null texture.");

    if (!g_model[OBJFileCount].import(pszFilename))
    {
        SetCursor(LoadCursor(0, IDC_ARROW));
        throw std::runtime_error("Failed to load model.");
    }

//    g_model[OBJFileCount].normalize();//남중 OBJ정규화

    // Load any associated textures.
    // Note the path where the textures are assumed to be located.

    const ModelOBJ::Material *pMaterial = 0;
    GLuint textureId = 0;
    std::string::size_type offset = 0;
    std::string filename;

    for (int i = 0; i < g_model[OBJFileCount].getNumberOfMaterials(); ++i)
    {
        pMaterial = &g_model[OBJFileCount].getMaterial(i);

        // Look for and load any diffuse color map textures.

        if (pMaterial->colorMapFilename.empty())
            continue;

        // Try load the texture using the path in the .MTL file.
        textureId = LoadOBJFileTexture(pMaterial->colorMapFilename.c_str());

        if (!textureId)
        {
            offset = pMaterial->colorMapFilename.find_last_of('\\');

            if (offset != std::string::npos)
                filename = pMaterial->colorMapFilename.substr(++offset);
            else
                filename = pMaterial->colorMapFilename;

            // Try loading the texture from the same directory as the OBJ file.
            textureId = LoadOBJFileTexture((g_model[OBJFileCount].getPath() + filename).c_str());
        }

        if (textureId)
            g_modelTextures[OBJFileCount][pMaterial->colorMapFilename] = textureId;

        // Look for and load any normal map textures.

        if (pMaterial->bumpMapFilename.empty())
            continue;

        // Try load the texture using the path in the .MTL file.
        textureId = LoadOBJFileTexture(pMaterial->bumpMapFilename.c_str());

        if (!textureId)
        {
            offset = pMaterial->bumpMapFilename.find_last_of('\\');

            if (offset != std::string::npos)
                filename = pMaterial->bumpMapFilename.substr(++offset);
            else
                filename = pMaterial->bumpMapFilename;

            // Try loading the texture from the same directory as the OBJ file.
            textureId = LoadOBJFileTexture((g_model[OBJFileCount].getPath() + filename).c_str());
        }

        if (textureId)
            g_modelTextures[OBJFileCount][pMaterial->bumpMapFilename] = textureId;
    }

	OBJModelLoaded = true;

    SetCursor(LoadCursor(0, IDC_ARROW));
}

void COpenGLRenderer::OBJFileUnLoad()
{//남중 OBJ파일을 삭제
	OBJModelLoaded = false;
    SetCursor(LoadCursor(0, IDC_WAIT));

    ModelTextures::iterator i = g_modelTextures[OBJFileCount].begin();

    while (i != g_modelTextures[OBJFileCount].end())
    {
        glDeleteTextures(1, &i->second);
        ++i;
    }

    g_modelTextures[OBJFileCount].clear();
    g_model[OBJFileCount].destroy();

	
	if (g_nullTexture)
    {
        glDeleteTextures(1, &g_nullTexture);
        g_nullTexture = 0;
    }


	OBJFileCount--;

    SetCursor(LoadCursor(0, IDC_ARROW));
}

void COpenGLRenderer::OBJFileLoad()
{//OBJ파일을 읽어들이는 함수
	char szFilter[] = "(*.OBJ) | *.OBJ| All Files(*.*)|*.*||";
	CString cstrfilepath;
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, szFilter);
	if(IDOK == dlg.DoModal())
	{
		cstrfilepath = dlg.GetPathName();
		LoadOBJModel(LPSTR(LPCTSTR(cstrfilepath)));
	}
	else
	{
		return;
	}
}

void COpenGLRenderer::DrawOBJModelUsingGPU(int ind)
{//남중 GPU에서 OBJ그리기
    const ModelOBJ::Mesh *pMesh = 0;
    const ModelOBJ::Material *pMaterial = 0;
    const ModelOBJ::Vertex *pVertices = 0;
    ModelTextures::const_iterator iter;
    GLuint texture = 0;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (int i = 0; i < g_model[ind].getNumberOfMeshes(); ++i)
    {
        pMesh = &g_model[ind].getMesh(i);
        pMaterial = pMesh->pMaterial;
        pVertices = g_model[ind].getVertexBuffer();

        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, pMaterial->ambient);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, pMaterial->diffuse);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, pMaterial->specular);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, pMaterial->shininess * 128.0f);

        if (pMaterial->bumpMapFilename.empty())
        {
            // Per fragment Blinn-Phong code path.

            glUseProgram(OBJShadingProgram);

			// Bind the color map texture.
            texture = g_nullTexture;

            // Bind the color map texture.
            if (g_enableTextures)
            {
                iter = g_modelTextures[ind].find(pMaterial->colorMapFilename);

                if (iter != g_modelTextures[ind].end())
                    texture = iter->second;
            }

            glActiveTexture(GL_TEXTURE0);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, texture);

            // Update shader parameters.

            glUniform1i(glGetUniformLocation(
                OBJShadingProgram, "colorMap"), 0);
            glUniform1f(glGetUniformLocation(
                OBJShadingProgram, "materialAlpha"), pMaterial->alpha);
        }
        else
        {
            // Normal mapping code path.

            glUseProgram(OBJNormalMapProgram);

            // Bind the normal map texture.

            iter = g_modelTextures[ind].find(pMaterial->bumpMapFilename);

            if (iter != g_modelTextures[ind].end())
            {
                glActiveTexture(GL_TEXTURE1);
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, iter->second);
            }

			// Bind the color map texture.

            texture = g_nullTexture;

            if (g_enableTextures)
            {
                iter = g_modelTextures[ind].find(pMaterial->colorMapFilename);

                if (iter != g_modelTextures[ind].end())
                    texture = iter->second;
            }

            glActiveTexture(GL_TEXTURE0);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, texture);

            // Update shader parameters.

            glUniform1i(glGetUniformLocation(
                OBJNormalMapProgram, "colorMap"), 0);
            glUniform1i(glGetUniformLocation(
                OBJNormalMapProgram, "normalMap"), 1);
            glUniform1f(glGetUniformLocation(
                OBJNormalMapProgram, "materialAlpha"), pMaterial->alpha);
        }        

        // Render mesh.

        if (g_model[ind].hasPositions())
        {
            glEnableClientState(GL_VERTEX_ARRAY);
            glVertexPointer(3, GL_FLOAT, g_model[ind].getVertexSize(),
                g_model[ind].getVertexBuffer()->position);
        }

        if (g_model[ind].hasTextureCoords())
        {
            glClientActiveTexture(GL_TEXTURE0);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glTexCoordPointer(2, GL_FLOAT, g_model[ind].getVertexSize(),
                g_model[ind].getVertexBuffer()->texCoord);
        }

        if (g_model[ind].hasNormals())
        {
            glEnableClientState(GL_NORMAL_ARRAY);
            glNormalPointer(GL_FLOAT, g_model[ind].getVertexSize(),
                g_model[ind].getVertexBuffer()->normal);
        }

        if (g_model[ind].hasTangents())
        {
            glClientActiveTexture(GL_TEXTURE1);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glTexCoordPointer(4, GL_FLOAT, g_model[ind].getVertexSize(),
                g_model[ind].getVertexBuffer()->tangent);
        }

        glDrawElements(GL_TRIANGLES, pMesh->triangleCount * 3, GL_UNSIGNED_INT,
            g_model[ind].getIndexBuffer() + pMesh->startIndex);

        if (g_model[ind].hasTangents())
        {
            glClientActiveTexture(GL_TEXTURE1);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        }

        if (g_model[ind].hasNormals())
            glDisableClientState(GL_NORMAL_ARRAY);

        if (g_model[ind].hasTextureCoords())
        {
            glClientActiveTexture(GL_TEXTURE0);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        }

        if (g_model[ind].hasPositions())
            glDisableClientState(GL_VERTEX_ARRAY);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
    glDisable(GL_BLEND);
}

void COpenGLRenderer::DrawOBJModel()
{
	for(int i=0;i <= OBJFileCount;i++)
	{
/*		glPushMatrix();
		if(i==1)
		{//지하철모델
			glTranslatef(0, 0, -100);
			glScalef(300,300,300);
		}
		else
			glScalef(1000,1000,1000);
	glRotatef(0,1,0,0);
	glRotatef(180,0,1,0);
	glRotatef(0,0,0,1);
*/		DrawOBJModelUsingGPU(i);
//		glPopMatrix();
	}
}

void COpenGLRenderer::ProjectFileSave()
{//프로젝트파일을 저장하는 함수
	char szFilter[] = "(*.PROJ) | *.PROJ| All Files(*.*)|*.*||";
	CString cstrfilepath;
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, szFilter);
	if(IDOK == dlg.DoModal())
	{
		cstrfilepath = dlg.GetPathName();

		projfp = fopen(LPSTR(LPCTSTR(cstrfilepath)), "w");
		fprintf(projfp, "%d\n", STLNum);
		int i, j;
		for(i=0;i<STLNum;i++)
		{
			fprintf(projfp, "%s\n", stlFile[i]->STLFileName);
			fprintf(projfp,"%s\n", stlFile[i]->STLFileTitle);
		}

		fclose(projfp);
	}
	else
	{
		return;
	}
}

void COpenGLRenderer::ProjectFileOpen()
{//프로젝트파일을 읽어들이는 함수
	char szFilter[] = "(*.PROJ) | *.PROJ| All Files(*.*)|*.*||";
	CString cstrfilepath;
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, szFilter);
	if(IDOK == dlg.DoModal())
	{
		cstrfilepath = dlg.GetPathName();

				int i;
		for(i=0;i<STLNum;i++)
		{
			stlFile[i]->close();
			delete stlFile[i];
		}
		FluidModelInd = -1;

		projfp = fopen(LPSTR(LPCTSTR(cstrfilepath)), "r");
		fscanf(projfp, "%d", &STLNum);
		for(i=0;i<STLNum;i++)
		{
			stlFile[i] = new StlFile();
			fscanf(projfp, "%s", &stlFile[i]->STLFileName);
			fscanf(projfp, "%s", &stlFile[i]->STLFileTitle);
			stlFile[i]->open(stlFile[i]->STLFileName, stlFile[i]->STLFileTitle);
		}
		fclose(projfp);
	}
	else
	{
		return;
	}
}

void COpenGLRenderer::ModelFileOpen()
{//모델파일을 읽어들이는 함수
	char szFilter[] = "(*.STL) | *.STL| All Files(*.*)|*.*||";
	CString cstrfilepath;
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, szFilter);
	if(IDOK == dlg.DoModal())
	{
		cstrfilepath = dlg.GetPathName();
		stlFile[STLNum] = new StlFile();
		stlFile[STLNum]->open(LPSTR(LPCTSTR(cstrfilepath)), LPSTR(LPCTSTR(cstrfilepath)));
		STLNum++;
	}
	else
	{
		return;
	}
}

#include <GL/glaux.h>

char *myProgramName = "Fluid Visualization";

typedef struct {
  int magic; /* must be "DDS\0" */
  int size; /* must be 124 */
  int flags;
  int height;
  int width;
  int pitchOrLinearSize;
  int depth;
  int mipMapCount;
  int reserved[11];
  struct {
    int size;
    int flags;
    int fourCC;
    int bitsPerPixel;
    int redMask;
    int greenMask;
    int blueMask;
    int alphaMask;
  } pixelFormat;
  struct {
    int caps;
    int caps2;
    int caps3;
    int caps4;
  } caps;
  int reserved2[1];
} DDS_file_header;

/* The DDS file format is little-endian so we need to byte-swap
   its 32-bit header words to work on big-endian architectures. */
static int int_le2native(int v)
{
/* Works even if little-endian target and __LITTLE_ENDIAN__ not defined. */
#if defined(__LITTLE_ENDIAN__) || defined(_WIN32)
  return v;
#else
  union {
    int i;
    unsigned char b[4];
  } src, dst;

  src.i = v;
  dst.b[0] = src.b[LE_INT32_BYTE_OFFSET(0)];
  dst.b[1] = src.b[LE_INT32_BYTE_OFFSET(1)];
  dst.b[2] = src.b[LE_INT32_BYTE_OFFSET(2)];
  dst.b[3] = src.b[LE_INT32_BYTE_OFFSET(3)];
  return dst.i;
#endif
}

int COpenGLRenderer::supportsOneDotWhatever(int whatever)
{
  const char *version;
  int major, minor;

  version = (char *) glGetString(GL_VERSION);
  if (sscanf(version, "%d.%d", &major, &minor) == 2) {
    return major > 1 || (major == 1 && minor >= whatever);
  }
  return 0;            /* OpenGL version string malformed! */
}

void COpenGLRenderer::initLoaderForDSS()
{
  /* OpenGL 1.3 incorporated compressed textures (based on the
     ARB_texture_compression extension.  However we also need to
     be sure the EXT_texture_compression_s3tc extension is present
     because that supports the specific DXT1 format we expect the
     DDS file to contain. */
  if (!supportsOneDotWhatever(3) ||
      !glutExtensionSupported("GL_EXT_texture_compression_s3tc")) {
    fprintf(stderr, ":needs GL_EXT_texture_compression_s3tc and at least OpenGL 1.3\n");
    exit(1);
  }
}

void COpenGLRenderer::loadCubeMapFromDDS(const char *filename)
{
  FILE *file = fopen(filename, "rb");
  long size;
  void *data;
  char *beginning, *image;
  int *words;
  size_t bytes;
  DDS_file_header *header;
  int i, face, level;

  if (!file) {
    fprintf(stderr, "%s: could not open cube map %s\n", myProgramName, filename);
    exit(1);
  }

  fseek(file, 0L, SEEK_END);
  size = ftell(file);
  fseek(file, 0L, SEEK_SET);
  data = (char*) malloc(size);
  if (data == NULL) {
    fprintf(stderr, "%s: malloc failed\n", myProgramName);
    exit(1);
  }
  bytes = fread(data, 1, size, file);
  fclose(file);

  if (bytes < sizeof(DDS_file_header)) {
    fprintf(stderr, "%s: DDS header to short for %s\n", myProgramName, filename);
    exit(1);
  }

  /* Byte swap the words of the header if needed. */
  for (words = (int*)data, i=0; i<sizeof(DDS_file_header)/sizeof(int); i++) {
    words[i] = int_le2native(words[i]);
  }

#define FOURCC(a) ((a[0]) | (a[1] << 8) | (a[2] << 16) | (a[3] << 24))
#define EXPECT(f,v) \
  if ((f) != (v)) { \
    fprintf(stderr, "%s: field %s mismatch (got 0x%x, expected 0x%x)\n", \
      myProgramName, #f, (f), (v)); exit(1); \
  }

  /* Examine the header to make sure it is what we expect. */
  header = (DDS_file_header*)data;
  EXPECT(header->magic, FOURCC("DDS "));

#define DDSD_CAPS               0x00000001  /* caps field is valid */
#define DDSD_HEIGHT             0x00000002  /* height field is valid */
#define DDSD_WIDTH              0x00000004  /* width field is valid */
#define DDSD_PIXELFORMAT        0x00001000  /* pixelFormat field is valid */
#define DDSD_MIPMAPCOUNT        0x00020000  /* mipMapCount field is valid */

#define DDSD_NEEDED (DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | \
                     DDSD_PIXELFORMAT | DDSD_MIPMAPCOUNT)

  EXPECT(header->flags & DDSD_NEEDED, DDSD_NEEDED);

  EXPECT(header->size, 124);
  EXPECT(header->depth, 0);
  EXPECT(header->pixelFormat.size, 32);  /* 32 bytes in a DXT1 block */
  EXPECT(header->pixelFormat.fourCC, FOURCC("DXT1"));

/* From the DirectX SDK's ddraw.h */
#define DDSCAPS2_CUBEMAP                        0x00000200
#define DDSCAPS2_CUBEMAP_POSITIVEX              0x00000400
#define DDSCAPS2_CUBEMAP_NEGATIVEX              0x00000800
#define DDSCAPS2_CUBEMAP_POSITIVEY              0x00001000
#define DDSCAPS2_CUBEMAP_NEGATIVEY              0x00002000
#define DDSCAPS2_CUBEMAP_POSITIVEZ              0x00004000
#define DDSCAPS2_CUBEMAP_NEGATIVEZ              0x00008000
#define DDSCAPS2_CUBEMAP_ALLFACES ( DDSCAPS2_CUBEMAP_POSITIVEX |\
                                    DDSCAPS2_CUBEMAP_NEGATIVEX |\
                                    DDSCAPS2_CUBEMAP_POSITIVEY |\
                                    DDSCAPS2_CUBEMAP_NEGATIVEY |\
                                    DDSCAPS2_CUBEMAP_POSITIVEZ |\
                                    DDSCAPS2_CUBEMAP_NEGATIVEZ )
  EXPECT(header->caps.caps2, DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_ALLFACES);

  initLoaderForDSS();

  beginning = (char*)data;
  image = (char*) &header[1];
  /* For each face of the cube map (in +X, -X, +Y, -Y, +Z, and -Z order)... */
  for (face=0; face<6; face++) {
    int levels = header->mipMapCount;
    int width = header->width;
    int height = header->height;
    const int border = 0;

    /* For each mipmap level... */
    for (level=0; level<levels; level++) {
      /* DXT1 has contains two 16-bit (565) colors and a 2-bit field for
         each of the 16 texels in a given 4x4 block.  That's 64 bits
         per block or 8 bytes. */
      const int bytesPer4x4Block = 8;
      /* Careful formula to compute the size of a DXT1 mipmap level.
         This formula accounts for the fact that mipmap levels get
         no smaller than a 4x4 block. */
      GLsizei imageSizeInBytes = ((width+3)>>2)*((height+3)>>2) * bytesPer4x4Block;
      size_t offsetInToRead = image + imageSizeInBytes - beginning;

      if (offsetInToRead > bytes) {
        fprintf(stderr, "%s: DDS images over read the data!\n", myProgramName);
        exit(1);
      }
      glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+face, level,
        GL_COMPRESSED_RGB_S3TC_DXT1_EXT,
        width, height, border, imageSizeInBytes, image);
      image += imageSizeInBytes;

      /* Half the width and height either iteration, but do not allow
         the width or height to become less than 1. */
      width = width >> 1;
      if (width < 1) {
        width = 1;
      }
      height = height >> 1;
      if (height < 1) {
        height = 1;
      }
    }
  }

  /* Configure texture parameters reasonably. */
  if (header->mipMapCount > 1) {
    /* Clamp the range of levels to however levels the DDS file actually has.
       If the DDS file has less than a full mipmap chain all the way down,
       this allows OpenGL to still use the texture. */
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, header->mipMapCount-1);
    /* Use better trilinear mipmap minification filter instead of the default. */
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  } else {
    /* OpenGL's default minification filter (GL_NEAREST_MIPMAP_LINEAR) requires
       mipmaps this DDS file does not have so switch to a linear filter that
       doesn't require mipmaps. */
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  }
  /* To eliminate artifacts at the seems from the default wrap mode (GL_REPEAT),
     switch the wrap modes to clamp to edge. */
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  
  free(data);
}

void COpenGLRenderer::drawDDSEnvMap()
{
  static const GLfloat vertex[4*6][3] = {
    /* Positive X face. */
    { 1, -1, -1 },  { 1, 1, -1 },  { 1, 1, 1 },  { 1, -1, 1 },
    /* Negative X face. */
    { -1, -1, -1 },  { -1, 1, -1 },  { -1, 1, 1 },  { -1, -1, 1 },
    /* Positive Y face. */
    { -1, 1, -1 },  { 1, 1, -1 },  { 1, 1, 1 },  { -1, 1, 1 },
    /* Negative Y face. */
    { -1, -1, -1 },  { 1, -1, -1 },  { 1, -1, 1 },  { -1, -1, 1 },
    /* Positive Z face. */
    { -1, -1, 1 },  { 1, -1, 1 },  { 1, 1, 1 },  { -1, 1, 1 },
    /* Negatieve Z face. */
    { -1, -1, -1 },  { 1, -1, -1 },  { 1, 1, -1 },  { -1, 1, -1 },
  };

  const float surroundingsDistance = 2000;//실제크기에맞춤

  int i;

  glMatrixMode (GL_MODELVIEW);
  glScalef(surroundingsDistance, surroundingsDistance, surroundingsDistance);

  glDisable (GL_DEPTH_TEST);
  glEnable(GL_TEXTURE_CUBE_MAP);
  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  glBegin(GL_QUADS);
  /* For each vertex of each face of the cube... */
  for (i=0; i<4*6; i++) {
    glTexCoord3fv(vertex[i]);
    glVertex3fv(vertex[i]);
  }
  glEnd();
  glEnable (GL_DEPTH_TEST);
  glDisable (GL_TEXTURE_CUBE_MAP);
}

void COpenGLRenderer::InitEnvironment()
{
	CString g_Cubemaps[6];
	g_Cubemaps[0]="envmap\\posx.bmp";
	g_Cubemaps[1]=	"envmap\\negx.bmp";
	g_Cubemaps[2]=	"envmap\\posy.bmp";
	g_Cubemaps[3]=	"envmap\\negy.bmp";
	g_Cubemaps[4]=	"envmap\\posz.bmp";
	g_Cubemaps[5]=	"envmap\\negz.bmp";


	GLuint	g_uiCubeMapConstants[6] = 
{
	GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB
};

	int i;
	
	AUX_RGBImageRec *pTextureImage = NULL;

	glGenTextures (1, &PoolSkyCubeMap.Texture);
	glBindTexture (GL_TEXTURE_CUBE_MAP_ARB, PoolSkyCubeMap);

	for (i = 0; i < 6; i++)
	{
		pTextureImage = auxDIBImageLoad(g_Cubemaps[i] );

		if( pTextureImage != NULL )
		{
			glTexImage2D( g_uiCubeMapConstants[i], 0, GL_RGB8, pTextureImage->sizeX, pTextureImage->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, pTextureImage->data );
			gluBuild2DMipmaps( g_uiCubeMapConstants[i], GL_RGB8,  pTextureImage->sizeX, pTextureImage->sizeY, GL_RGB, GL_UNSIGNED_BYTE, pTextureImage->data );
		}


		if( pTextureImage )
		{
			if( pTextureImage->data )
				free( pTextureImage->data );
			free( pTextureImage );
		}
	}

	glTexParameteri (GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri (GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_CUBE_MAP_ARB, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
	glTexParameterf (GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MAX_ANISOTROPY_EXT, 8.0f);
}

void COpenGLRenderer::DrawEnvironmentBox()
{//환경맵그리는함수
	float envsize = 2000.0;
	GLfloat xPlane[] = { 1.0f, 0.0f, 0.0f, 0.0f };
	GLfloat yPlane[] = { 0.0f, 1.0f, 0.0f, 0.0f };
	GLfloat zPlane[] = { 0.0f, 0.0f, 1.0f, 0.0f };

	glEnable (GL_TEXTURE_GEN_S);
	glEnable (GL_TEXTURE_GEN_T);
	glEnable (GL_TEXTURE_GEN_R);

	glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glEnable (GL_TEXTURE_CUBE_MAP_ARB);
	glBindTexture (GL_TEXTURE_CUBE_MAP_ARB, PoolSkyCubeMap);

	glTexGeni (GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGeni (GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGeni (GL_R, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);

	glTexGenfv (GL_S, GL_OBJECT_PLANE, xPlane);
	glTexGenfv (GL_T, GL_OBJECT_PLANE, yPlane);
	glTexGenfv (GL_R, GL_OBJECT_PLANE, zPlane);

	glDisable (GL_DEPTH_TEST);

	glMatrixMode (GL_MODELVIEW);

	glBegin (GL_QUADS);

	// Front
	glTexCoord2f (0, 0);
	glVertex3f (-envsize,-envsize,-envsize);

	glTexCoord2f( 1, 0 );
	glVertex3f ( envsize,-envsize,-envsize);

	glTexCoord2f (1, 1);
	glVertex3f ( envsize, envsize,-envsize);

	glTexCoord2f (0, 1);
	glVertex3f (-envsize, envsize,-envsize);

	// Right
	glTexCoord2f (0, 0);
	glVertex3f ( envsize,-envsize,-envsize);

	glTexCoord2f (1, 0);
	glVertex3f ( envsize,-envsize, envsize);

	glTexCoord2f (1, 1);
	glVertex3f ( envsize, envsize, envsize);

	glTexCoord2f (0, 1);
	glVertex3f ( envsize, envsize,-envsize);

	// Back
	glTexCoord2f (0, 0);
	glVertex3f ( envsize, -envsize, envsize);

	glTexCoord2f (1, 0);
	glVertex3f (-envsize,-envsize, envsize);

	glTexCoord2f (1, 1);
	glVertex3f (-envsize, envsize, envsize);

	glTexCoord2f (0, 1);
	glVertex3f (envsize, envsize, envsize);

	// Left
	glTexCoord2f (0, 0);
	glVertex3f (-envsize,-envsize, envsize);

	glTexCoord2f (1, 0);
	glVertex3f (-envsize,-envsize,-envsize);

	glTexCoord2f (1, 1);
	glVertex3f (-envsize, envsize,-envsize);

	glTexCoord2f (0, 1);
	glVertex3f (-envsize, envsize, envsize);

	// Top
	glTexCoord2f (0, 0);
	glVertex3f (-envsize, envsize,-envsize);

	glTexCoord2f (1, 0);
	glVertex3f (envsize, envsize,-envsize);

	glTexCoord2f ( 1, 1);
	glVertex3f (envsize, envsize, envsize);

	glTexCoord2f (0, 1);
	glVertex3f (-envsize, envsize, envsize);

	// Bottom
	glTexCoord2f (0, 0);
	glVertex3f (-envsize, -envsize, envsize);

	glTexCoord2f (1, 0);
	glVertex3f ( envsize,-envsize, envsize);

	glTexCoord2f (1, 1);
	glVertex3f ( envsize, -envsize, -envsize);

	glTexCoord2f (0, 1);
	glVertex3f (-envsize, -envsize, -envsize);

	glEnd();

	glEnable (GL_DEPTH_TEST);

	glDisable (GL_TEXTURE_CUBE_MAP_ARB);

	glDisable (GL_TEXTURE_GEN_S);
	glDisable (GL_TEXTURE_GEN_T);
	glDisable (GL_TEXTURE_GEN_R);
}

void COpenGLRenderer::DebugSTLFile(int num)
{
	defp = fopen("zdebug.txt", "a");
	fprintf(defp, "크하하 %d 번째파일\n",num);
	fprintf(defp, "아아 %d 개\n",stlFile[num]->getStats().numFacets);
	for(int i = 0; i < stlFile[num]->getStats().numFacets; ++i)
	{
		fprintf(defp, "법선 %f %f %f\n",stlFile[num]->getFacets()[i].normal.x, stlFile[num]->getFacets()[i].normal.y, stlFile[num]->getFacets()[i].normal.z);
   fprintf(defp, "정점1 %f %f %f\n",stlFile[num]->getFacets()[i].vector[0].x, stlFile[num]->getFacets()[i].vector[0].y, stlFile[num]->getFacets()[i].vector[0].z);
   fprintf(defp, "정점2 %f %f %f\n",stlFile[num]->getFacets()[i].vector[1].x, stlFile[num]->getFacets()[i].vector[1].y, stlFile[num]->getFacets()[i].vector[1].z);
   fprintf(defp, "정점3 %f %f %f\n",stlFile[num]->getFacets()[i].vector[2].x, stlFile[num]->getFacets()[i].vector[2].y, stlFile[num]->getFacets()[i].vector[2].z);
	}
	fclose(defp);
}

void COpenGLRenderer::UpdateTerrainShaderParameters()
{//Terrain쉐이더 파라미터설정하는 함수
    GLint handle = -1;

    handle = glGetUniformLocation(TexTerrainProgram, "tilingFactor");
    glUniform1f(handle, HEIGHTMAP_TILING_FACTOR);

    // Update terrain region 1.

    handle = glGetUniformLocation(TexTerrainProgram, "region1.max");
    glUniform1f(handle, g_regions[0].max);

    handle = glGetUniformLocation(TexTerrainProgram, "region1.min");
    glUniform1f(handle, g_regions[0].min);    

    // Update terrain region 2.

    handle = glGetUniformLocation(TexTerrainProgram, "region2.max");
    glUniform1f(handle, g_regions[1].max);

    handle = glGetUniformLocation(TexTerrainProgram, "region2.min");
    glUniform1f(handle, g_regions[1].min);

    // Update terrain region 3.

    handle = glGetUniformLocation(TexTerrainProgram, "region3.max");
    glUniform1f(handle, g_regions[2].max);

    handle = glGetUniformLocation(TexTerrainProgram, "region3.min");
    glUniform1f(handle, g_regions[2].min);

    // Update terrain region 4.

    handle = glGetUniformLocation(TexTerrainProgram, "region4.max");
    glUniform1f(handle, g_regions[3].max);

    handle = glGetUniformLocation(TexTerrainProgram, "region4.min");
    glUniform1f(handle, g_regions[3].min);

    // Bind textures.

    glUniform1i(glGetUniformLocation(TexTerrainProgram, "region1ColorMap"), 10);
    glUniform1i(glGetUniformLocation(TexTerrainProgram, "region2ColorMap"), 11);
    glUniform1i(glGetUniformLocation(TexTerrainProgram, "region3ColorMap"), 12);
    glUniform1i(glGetUniformLocation(TexTerrainProgram, "region4ColorMap"), 13);
}

vec3 COpenGLRenderer::ReturnCodedColor(float col, float heightdiff)
{//지형 정점의 칼라코딩색상구하는함수
	float coeff = 1.0/4.0;	
	float range1 = ((heightdiff*coeff*1)-col)/(heightdiff*coeff);
	float range2 = ((heightdiff*coeff*2)-col)/(heightdiff*coeff);
	float range3 = ((heightdiff*coeff*3)-col)/(heightdiff*coeff);
	float range4 = ((heightdiff*coeff*4)-col)/(heightdiff*coeff);
	float rc11 = (0.239215686*(1-range1));
	float rc12 = (0.721568627*(1-range1));
	float rc13 = (0.407843137*(1-range1));
	float rc21 = (0.443137255*(1-range2));
	float rc22 = (0.921568627*(1-range2));
	float rc23 = (0.184313725*(1-range2));
	float rc31 = (1.0*(1-range3));
	float rc32 = (1.0*(1-range3));
	float rc33 = (0.501960784*(1-range3));
	float rc41 = (0.858823529*(1-range4));
	float rc42 = (0.28627451*(1-range4));
	float rc43 = (0.298039216*(1-range4));

	vec3 colv;
	
	if(col < heightdiff*coeff)		
	{
		colv = vec3(range1*0.043137255+rc11, range1*0.17254902+rc12, range1*0.478431373+rc13);			
		//colv = vec3(0, col/heightdiff/coeff, 1.0);
	}
	else if(col < heightdiff*coeff*2.0)				
	{
		colv = vec3(range2*0.239215686+rc21, range2*0.721568627+rc22, range2*0.407843137+rc23);
		//colv = vec3(0.0, 1.0, 2.0-(col/heightdiff/coeff));
	}
	else if(col < heightdiff*coeff*3.0)		
	{
		colv = vec3(range3*0.443137255+rc31, range3*0.921568627+rc32, range3*0.184313725+rc33);
		//colv = vec3(col/heightdiff/coeff-2.0, 1.0, 0.0);
	}
	else// if(col < heightdiff*coeff*4.0)		
	{
		colv = vec3(range4*1.0+rc41, range4*1.0+rc42, range4*0.501960784+rc43);
		//colv = vec3(1.0, 4.0-(col/heightdiff/coeff), 0.0);
	}
	return colv;


/*
	float coeff = 0.25;
	vec3 colv;
	
	if(col < heightdiff*coeff)
		colv = vec3(0, col/heightdiff/coeff, 1.0);
	else if(col < heightdiff*coeff*2.0)
		colv = vec3(0, 1.0, 2.0-(col/heightdiff/coeff));
	else if(col < heightdiff*coeff*3.0)
		colv = vec3(col/heightdiff/coeff-2.0, 1.0, 0.0);
	else
		colv = vec3(1.0, 4.0-(col/heightdiff/coeff), 0.0);

	return colv;
*/
}

void COpenGLRenderer::ReadTerrainHeightMap(char* trfilename)
{
	SetCurrentDirectory(WinInitDir);

	for(int i=0;i<STLNum;i++)
			if(strcmp(stlFile[i]->STLFileTitle, "GC_terrain.stl")==0)
			{
				delete stlFile[i];
				STLNum--;
			}



	free(TerrainHeightMap);

	float* terrainmap;
	if(HeightMapLoadingSmoothed)
		readSmoothedTRN(trfilename, terrainwidth, terrainheight, terrainmap);
	else
		readTRN(trfilename, terrainwidth, terrainheight, terrainmap);

    TerrainCenter = createLandscapeTRN(terrainmap, terrainwidth, terrainheight, terrainwidth, terrainheight, TerrainHeightMap, heightmin, heightmax);
    free(terrainmap);
//지형 HeightMap Load
//TerrainHeightMap에 TRN파일의 지형 높이맵이 들어갑니다.

	Camera.SetReferencePoint(TerrainCenter);//지형의 중심을 카메라회전의 중심에 맞춤


	//남중 TerrainRegion구조체(4번째인자가 텍스쳐filename) 배열 4개 정의
g_regions[0].min = heightmin;
g_regions[0].max = 0.607f * heightmax * HEIGHTMAP_SCALE;
g_regions[1].min = heightmin * HEIGHTMAP_SCALE;
g_regions[1].max = 0.786f* heightmax * HEIGHTMAP_SCALE;
g_regions[2].min = 0.714f *heightmax* HEIGHTMAP_SCALE;
g_regions[2].max = 0.893f*heightmax * HEIGHTMAP_SCALE;
g_regions[3].min = 0.893*heightmax * HEIGHTMAP_SCALE;
g_regions[3].max = heightmax * HEIGHTMAP_SCALE;

	int hWMR = terrainwidth;

	glGenBuffers(1, &TerrainVBO);
	glGenBuffers(1, &TexCoordsVBO);
	glGenBuffers(1, &TerrainNormalVBO);
	glGenBuffers(1, &TerrainColorMapVBO);

	int hWMRP1 = terrainwidth;//가로길이
	int hWMRP2 = terrainheight;//세로길이


	CBuffer hQuads;
	CBuffer tQuads;
	CBuffer nQuads;
	CBuffer cQuads;

	float col_a, col_b, col_c, col_d;
	float heightdiff = heightmax-heightmin;
	vec3 col1, col2, col3, col4;

	for(int y = 0; y < hWMRP2-1; y++)
	{
		int yp1 = y + 1;

		for(int x = 0; x < hWMR-1; x++)
		{
			int xp1 = x + 1;

			int a = hWMRP1 * y + x;
			int b = hWMRP1 * y + xp1;
			int c = hWMRP1 * yp1 + xp1;
			int d = hWMRP1 * yp1 + x;

//지형 높이맵 버퍼에 저장
			hQuads.AddData(&TerrainHeightMap[a], 12);
			hQuads.AddData(&TerrainHeightMap[b], 12);
			hQuads.AddData(&TerrainHeightMap[c], 12);
			hQuads.AddData(&TerrainHeightMap[d], 12);

//지형 법선들 버퍼에 저장
			nQuads.AddData(&normalAtPixel(x, y, terrainwidth, terrainheight, TerrainHeightMap), 12);
			nQuads.AddData(&normalAtPixel(x+1, y, terrainwidth, terrainheight, TerrainHeightMap), 12);
			nQuads.AddData(&normalAtPixel(x+1, y+1, terrainwidth, terrainheight, TerrainHeightMap), 12);
			nQuads.AddData(&normalAtPixel(x, y+1, terrainwidth, terrainheight, TerrainHeightMap), 12);

//텍스쳐좌표 버퍼에 저장
			float tex_x = (float)(x)/(float)(terrainwidth);
			float tex_y = (float)(y)/(float)(terrainheight);
			float tex_xb = (float)(x+1)/(float)(terrainwidth);
			float tex_yb = (float)(y+1)/(float)(terrainheight);

			vec2 tex1 = vec2(tex_x, tex_y);
			vec2 tex2 = vec2(tex_xb, tex_y);
			vec2 tex3 = vec2(tex_xb, tex_yb);
			vec2 tex4 = vec2(tex_x, tex_yb);

			tQuads.AddData(&tex1, 8);
			tQuads.AddData(&tex2, 8);
			tQuads.AddData(&tex3, 8);
			tQuads.AddData(&tex4, 8);


//지형의 칼라코딩 데어터 버퍼에 저장
			col_a = (TerrainHeightMap[a].y-heightmin);
			col_b = (TerrainHeightMap[b].y-heightmin);
			col_c = (TerrainHeightMap[c].y-heightmin);
			col_d = (TerrainHeightMap[d].y-heightmin);

			col1 = ReturnCodedColor(col_a, heightdiff);
			col2 = ReturnCodedColor(col_b, heightdiff);
			col3 = ReturnCodedColor(col_c, heightdiff);
			col4 = ReturnCodedColor(col_d, heightdiff);

			cQuads.AddData(&col1, 12);
			cQuads.AddData(&col2, 12);
			cQuads.AddData(&col3, 12);
			cQuads.AddData(&col4, 12);
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, TerrainVBO);
	glBufferData(GL_ARRAY_BUFFER, hQuads.GetDataSize(), hQuads.GetData(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, TerrainNormalVBO);
	glBufferData(GL_ARRAY_BUFFER, nQuads.GetDataSize(), nQuads.GetData(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, TexCoordsVBO);
	glBufferData(GL_ARRAY_BUFFER, tQuads.GetDataSize(), tQuads.GetData(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, TerrainColorMapVBO);
	glBufferData(GL_ARRAY_BUFFER, cQuads.GetDataSize(), cQuads.GetData(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	QuadsTerrainCount = hQuads.GetDataSize() / 12;

	hQuads.Empty();
	tQuads.Empty();
	nQuads.Empty();
	cQuads.Empty();

	TerrainCreatedOK = true;
}

void COpenGLRenderer::ReadNFLOWAnalHeightMap()
{
	SetCurrentDirectory(WinInitDir);

	float* terrainmap;
	readTRN("image\\subway_flow.TRN", terrainwidth, terrainheight, terrainmap);

//	TerrainCenter = createLandscapeTRN(terrainmap, terrainwidth, terrainheight, terrainwidth, terrainheight, NFLOWTerrainHeightMap, heightmin, heightmax);

	TerrainCenter = createLandscapeTRN(terrainmap, terrainwidth, terrainheight, terrainwidth, terrainheight, NFLOWTerrainHeightMap, heightmin, heightmax, 2.3460, 0.5000, 0.07, 0.07);

    free(terrainmap);
//NFLOW지형 HeightMap Load
//NFLOWTerrainHeightMap에 TRN파일의 지형 높이맵이 들어갑니다.

	Camera.SetReferencePoint(TerrainCenter);//지형의 중심을 카메라회전의 중심에 맞춤



	int hWMR = terrainwidth;

	glGenBuffers(1, &NFLOWTerrainVBO);
	glGenBuffers(1, &NFLOWTerrainNormalVBO);

	int hWMRP1 = terrainwidth;//가로길이
	int hWMRP2 = terrainheight;//세로길이


	CBuffer hQuads;
	CBuffer nQuads;

	float col_a, col_b, col_c, col_d;
	float heightdiff = heightmax-heightmin;
	vec3 col1, col2, col3, col4;

	for(int y = 0; y < hWMRP2-1; y++)
	{
		int yp1 = y + 1;

		for(int x = 0; x < hWMR-1; x++)
		{
			int xp1 = x + 1;

			int a = hWMRP1 * y + x;
			int b = hWMRP1 * y + xp1;
			int c = hWMRP1 * yp1 + xp1;
			int d = hWMRP1 * yp1 + x;

//지형 높이맵 버퍼에 저장
			hQuads.AddData(&NFLOWTerrainHeightMap[a], 12);
			hQuads.AddData(&NFLOWTerrainHeightMap[b], 12);
			hQuads.AddData(&NFLOWTerrainHeightMap[c], 12);
			hQuads.AddData(&NFLOWTerrainHeightMap[d], 12);

//지형 법선들 버퍼에 저장
			nQuads.AddData(&normalAtPixel(x, y, terrainwidth, terrainheight, NFLOWTerrainHeightMap), 12);
			nQuads.AddData(&normalAtPixel(x+1, y, terrainwidth, terrainheight, NFLOWTerrainHeightMap), 12);
			nQuads.AddData(&normalAtPixel(x+1, y+1, terrainwidth, terrainheight, NFLOWTerrainHeightMap), 12);
			nQuads.AddData(&normalAtPixel(x, y+1, terrainwidth, terrainheight, NFLOWTerrainHeightMap), 12);
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, NFLOWTerrainVBO);
	glBufferData(GL_ARRAY_BUFFER, hQuads.GetDataSize(), hQuads.GetData(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, NFLOWTerrainNormalVBO);
	glBufferData(GL_ARRAY_BUFFER, nQuads.GetDataSize(), nQuads.GetData(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	NFLOWQuadsTerrainCount = hQuads.GetDataSize() / 12;

	hQuads.Empty();
	nQuads.Empty();


}

void COpenGLRenderer::DrawNFLOWAnalHeightMap()
{
	glLoadMatrixf(&ViewMatrix);

	glUseProgram(AnalWaterProgram);

	glBindBuffer(GL_ARRAY_BUFFER, NFLOWTerrainNormalVBO);
	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 12, (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, NFLOWTerrainVBO);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 12, (void*)0);

	glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_CUBE_MAP, PoolSkyCubeMap);
	glActiveTexture(GL_TEXTURE20); glBindTexture(GL_TEXTURE_2D, ReflectionTexture);
	glActiveTexture(GL_TEXTURE21); glBindTexture(GL_TEXTURE_2D, RefractionTexture);
	glUniform3fv(glGetUniformLocation(AnalWaterProgram, "CameraPosition"), 1, &Camera.Position);
	glUniform1f(glGetUniformLocation(AnalWaterProgram, "tilingFactor"), 12.0);

	glActiveTexture(GL_TEXTURE15);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, AnalNormalMapTex);
	//WATER법선맵연결


	glDrawArrays(GL_QUADS, 0, NFLOWQuadsTerrainCount);

	glUseProgram(0);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);


	glActiveTexture(GL_TEXTURE15);//WATER법선맵해제
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);


	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glActiveTexture(GL_TEXTURE20); glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE21); glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}


void COpenGLRenderer::DrawFluidHeightMap()
{
	glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, WaterHeightMaps[WHMID]);
	glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, WaterNormalMap);
	glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_CUBE_MAP, PoolSkyCubeMap);
	glUseProgram(WaterProgram);
	glUniform3fv(WaterProgram.UniformLocations[0], 1, &Camera.Position);
	glBindBuffer(GL_ARRAY_BUFFER, WaterVBO);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 12, (void*)0);

//	glDrawArrays(GL_QUADS, 0, QuadsVerticesCount);

	glDisableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
	glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, 0);
}

void COpenGLRenderer::ReadFluidHeightMap()
{
	float* heightmap;
    readTRN("image\\gc_fluid.TRN", heightmapwidth, heightmapheight, heightmap);
    createLandscapeTRN(heightmap, heightmapwidth, heightmapheight, heightmapwidth, heightmapheight, FluidHeightMap, heightmin, heightmax);
    free(heightmap);
//유체 HeightMap Load
//FluidHeightMap에 TRN파일의 유체 높이맵이 들어갑니다.

	WMR = heightmapwidth;
	WHMR = heightmapwidth;
	WNMR = heightmapwidth*2;

		glGenBuffers(1, &WaterVBO);

	int WMRP1 = heightmapwidth;//가로길이
	int WMRP2 = heightmapheight;//세로길이

	float WMSDWMR = 2.0f / (float)WMR;

	CBuffer Quads;

	for(int y = 0; y < WMRP2-1; y++)
	{
		int yp1 = y + 1;

		for(int x = 0; x < WMR; x++)
		{
			int xp1 = x + 1;

			int a = WMRP1 * y + x;
			int b = WMRP1 * y + xp1;
			int c = WMRP1 * yp1 + xp1;
			int d = WMRP1 * yp1 + x;

			Quads.AddData(&FluidHeightMap[a], 12);
			Quads.AddData(&FluidHeightMap[b], 12);
			Quads.AddData(&FluidHeightMap[c], 12);
			Quads.AddData(&FluidHeightMap[d], 12);
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, WaterVBO);
	glBufferData(GL_ARRAY_BUFFER, Quads.GetDataSize(), Quads.GetData(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	QuadsVerticesCount = Quads.GetDataSize() / 12;

	Quads.Empty();
//유체 높이맵 LOAD
}

void COpenGLRenderer::DisplayVelocityVector(vec3* thmap, vec3* velmap, float* speedmap, int twidth, int theight, float tmin, float tmax)
{//해석데이터 벡터그리는함수
	vec3 colv;
	float heightdiff = tmax-tmin;

	glUseProgram(STLShadingProgram);

	for (int z = 0; z < theight; z++)
		for (int x = 0; x < twidth; x++)
		{
			if(x % 30 == 0 && z % 30 == 0 && (thmap[z*twidth + x].y > 0))
			{
				float colf = speedmap[z*twidth + x]-tmin;
				colv = ReturnCodedColor(speedmap[z*twidth + x]-tmin, heightdiff);
				glColor3f(colv.x, colv.y, colv.z);
				DrawHeightMapVector(thmap[z*twidth + x], velmap[z*twidth + x]);
			}
		}

		glUseProgram(0);
}

void COpenGLRenderer::DisplayHeightMapVector(vec3* thmap, int twidth, int theight, float tmin, float tmax)
{//해석데이터 벡터그리는함수
	vec3 colv;
	float heightdiff = tmax-tmin;

	glUseProgram(STLShadingProgram);

	for (int z = 0; z < theight; z++)
		for (int x = 0; x < twidth; x++)
		{
			if(x % 30 == 0 && z % 30 == 0 && (thmap[z*twidth + x].y > 0))
			{
				float colf = thmap[z*twidth + x].y-tmin;
				colv = ReturnCodedColor(thmap[z*twidth + x].y-tmin, heightdiff);
				glColor3f(colv.x, colv.y, colv.z);
				DrawHeightMapVector(thmap[z*twidth + x], normalAtPixel(x, z, twidth, theight, thmap));
			}
		}

		glUseProgram(0);
}

void COpenGLRenderer::DrawHeightMapVector(vec3 start, vec3 dir)
{//시작점과 방향벡터를 이용하여 해석데이터의 벡터표현하는함수
	float csSize = 200.0, csHeight = 10.0;//벡터를 표현하는 3차원 화살표의 크기(csSize:화살표 꼭대기의 크기, csHeight:화살표 막대기의 길이)
	vec3 endpt = vec3(start.x+dir.x*csHeight, start.y+dir.y*csHeight, start.z+dir.z*csHeight);

	DrawVectorsArrow(start.x, start.y, start.z, endpt.x, endpt.y, endpt.z, csSize);
}

void COpenGLRenderer::DrawVectorsArrow(float x1, float y1, float z1, float x2, float y2, float z2, float csSize)
{
	glPushMatrix();
	glPushAttrib( GL_POLYGON_BIT ); // includes GL_CULL_FACE
	glDisable(GL_CULL_FACE); // draw from all sides
 
	// Calculate vector along direction of line
	vec3 ttv = vec3(x2-x1, y2-y1, z2-z1);
/*
	glLineWidth(5.0); 
	glBegin(GL_LINES); 
	glVertex3f(x1, y1, z1); // Starting point for line (arrow head end)
	glVertex3f(x2, y2, z2);        // so the arrowhead is not blunted by the line thickness
	glEnd(); // GL_LINES
*/ 
        float norm_of_v = sqrt( ttv.x*ttv.x + ttv.y*ttv.y + ttv.z*ttv.z );
 
        // Size of cone in arrow:
        float coneFractionAxially = 0.25; // radius at thickest part
        float coneFractionRadially = 0.2; // length of arrow
 
        float coneHgt = coneFractionAxially * norm_of_v;
        float coneRadius = coneFractionRadially * norm_of_v;
 
 
        // Set location of arrowhead to be at the startpoint of the line
        vec3 vConeLocation = vec3(x2, y2, z2);
 
        // Initialize transformation matrix
        float mat44[16] =
            {1,0,0,0,
             0,1,0,0,
             0,0,1,0,
             0,0,0,1};
 
        // The direction of the arrowhead is the line vector
        vec3 dVec = vec3(ttv.x, ttv.y, ttv.z);
 
        // Normalize dVec to get Unit Vector norm_startVec
        vec3 norm_startVec;
        norm_startVec =  normalize(dVec);
 
        // Normalize zaxis to get Unit Vector norm_endVec
        vec3 zaxis = vec3(0.0, 0.0, 1.0);
        vec3 norm_endVec;
        norm_endVec = normalize(zaxis);
 
        // If vectors are identical, set transformation matrix to identity
        if ( ((norm_startVec.x - norm_endVec.x) > 1e-14) && ((norm_startVec.y - norm_endVec.y) > 1e-14) && ((norm_startVec.z - norm_endVec.z) > 1e-14) )
        {
        }
        // otherwise create the matrix
        else
        {
 
            // Vector cross-product, result = axb
            vec3 axb;
            axb = cross(norm_startVec, norm_endVec);
 
            // Normalize axb to get Unit Vector norm_axb
            vec3 norm_axb;
            norm_axb = normalize(axb);
  
            // Build the rotation matrix
            float ac = acos( dot(norm_startVec, norm_endVec) );
 
            float s = sin( ac );
            float c = cos( ac );
            float t = 1 - c;
 
            float x = norm_axb.x;
            float y = norm_axb.y;
            float z = norm_axb.z;
 
            // Fill top-left 3x3
            mat44[0] = t*x*x + c;
            mat44[1] = t*x*y - s*z;
            mat44[2] = t*x*z + s*y;
 
            mat44[4] = t*x*y + s*z;
            mat44[5] = t*y*y + c;
            mat44[6] = t*y*z - s*x;
 
            mat44[8] = t*x*z - s*y;
            mat44[9] = t*y*z + s*x;
            mat44[10] = t*z*z + c;
 
            mat44[15] = 1.0;
	    }

		glPushMatrix();
		glTranslatef( x1, y1, z1);
		glMultMatrixf( mat44 );
		        GLUquadric* cone_obj = gluNewQuadric();
        gluCylinder(cone_obj, 2.0, 2.0, length(ttv), 20, 20);
		glPopMatrix();

		glPushMatrix();
        // Translate and rotate arrowhead to correct position
        glTranslatef( vConeLocation.x, vConeLocation.y, vConeLocation.z );
        glMultMatrixf( mat44 );

		glutSolidCone(0.027*csSize,0.09*csSize,10,10);

        glPopAttrib(); // GL_CULL_FACE
        glPopMatrix();
}

void COpenGLRenderer::DrawAnalysisDataAsWater()
{//해석데이터를물로표현
	glUseProgram(AnalWaterProgram);
/*
	glBindBuffer(GL_ARRAY_BUFFER, AnalNormalMapTexVBO);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 8, (void*)0);
*/
	glBindBuffer(GL_ARRAY_BUFFER, AnalNormalVBO);
	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 12, (void*)0);

	if(AnalWaterSpeedOn)
	{
		glBindBuffer(GL_ARRAY_BUFFER, AnalVelocityColorVBO);
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(3, GL_FLOAT, 12, (void*)0);
	}
	else
	{
		glBindBuffer(GL_ARRAY_BUFFER, AnalElevationColorVBO);
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(3, GL_FLOAT, 12, (void*)0);
	}

	glBindBuffer(GL_ARRAY_BUFFER, AnalWaterVBO);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 12, (void*)0);

	glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_CUBE_MAP, PoolSkyCubeMap);
	glActiveTexture(GL_TEXTURE20); glBindTexture(GL_TEXTURE_2D, ReflectionTexture);
	glActiveTexture(GL_TEXTURE21); glBindTexture(GL_TEXTURE_2D, RefractionTexture);
	glUniform3fv(glGetUniformLocation(AnalWaterProgram, "CameraPosition"), 1, &Camera.Position);
//	glUniform1f(glGetUniformLocation(AnalWaterProgram, "tilingFactor"), 1000.0);
	glUniform1f(glGetUniformLocation(AnalWaterProgram, "TerrainHeight"), heightmin);
	glUniform1f(glGetUniformLocation(AnalWaterProgram, "AnalHeightDiff"), AnalElevationmax-AnalElevationmin);

/*	glUniform1f(glGetUniformLocation(AnalWaterProgram, "timer"), glutGet(GLUT_ELAPSED_TIME));

	glActiveTexture(GL_TEXTURE15);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, AnalNormalMapTex);
	//해석데이터WATER법선맵연결
*/

	glDrawArrays(GL_QUADS, 0, QuadsAnalDataCount);

	glUseProgram(0);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
//	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

/*
	glActiveTexture(GL_TEXTURE15);//해석데이터WATER법선맵해제
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
*/

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glActiveTexture(GL_TEXTURE20); glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE21); glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void COpenGLRenderer::DrawAnalysisDataColorCoding()
{//해석데이터칼라코딩
	glUseProgram(ColorCodingProgram);

	glUniform1f(glGetUniformLocation(ColorCodingProgram, "TerrainHeight"), heightmin);
	glUniform1f(glGetUniformLocation(AnalWaterProgram, "AnalHeightDiff"), AnalElevationmax-AnalElevationmin);
	glUniform1f(glGetUniformLocation(AnalWaterProgram, "AnalSpeedDiff"), AnalSpeedmax-AnalSpeedmin);

	glBindBuffer(GL_ARRAY_BUFFER, AnalNormalVBO);
	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 12, (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, AnalWaterVBO);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 12, (void*)0);

	if(AnalWaterSpeedOn==true)
	{
		switch(AnalDataColorCodingOn)
		{
		case ANAL_VELOCITYSTYLE:
			glUniform1i(glGetUniformLocation(ColorCodingProgram, "AnalColorType"), 1);
			break;
		case ANAL_DEPTH:
			break;
		case ANAL_ELEVATION:
			glUniform1i(glGetUniformLocation(ColorCodingProgram, "AnalColorType"), 3);
			break;
		}

		glBindBuffer(GL_ARRAY_BUFFER, AnalVelocityColorVBO);
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(3, GL_FLOAT, 12, (void*)0);
	}
	else
	{
		glUniform1i(glGetUniformLocation(ColorCodingProgram, "AnalColorType"), 3);

		glBindBuffer(GL_ARRAY_BUFFER, AnalElevationColorVBO);
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(3, GL_FLOAT, 12, (void*)0);
	}

	glDrawArrays(GL_QUADS, 0, QuadsAnalDataCount);

	glUseProgram(0);

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void COpenGLRenderer::DrawAnalysisData()
{
	glLoadMatrixf(&ViewMatrix);

	if(AnalDataColorCodingOn)
	{
		DrawAnalysisDataColorCoding();

		switch(AnalDataColorCodingOn)
		{
		case ANAL_VELOCITYSTYLE:
			DisplayColorCodingRemark(WIN_WIDTH, WIN_HEIGHT, AnalSpeedmin, AnalSpeedmax, AnalDataColorCodingOn);
			break;
		case ANAL_DEPTH:
			DisplayColorCodingRemark(WIN_WIDTH, WIN_HEIGHT, AnalDepthmin, AnalDepthmax, AnalDataColorCodingOn);
			break;
		case ANAL_ELEVATION:
			DisplayColorCodingRemark(WIN_WIDTH, WIN_HEIGHT, AnalElevationmin, AnalElevationmax, AnalDataColorCodingOn);
			break;
		}
	}
	else
		DrawAnalysisDataAsWater();

	if(AnalWaterVelocityOn)
	{
		switch(AnalDataVectorsOn)
		{//해석데이터의 벡터표현방식에 맞게 벡터들 그리기
		case ANAL_STREAMLINE:
			break;
		case ANAL_VECTOR:
//			DisplayHeightMapVector(AnalHeightMap, Analwidth, Analheight, AnalElevationmin, AnalElevationmax);//법선벡터그리기
			DisplayVelocityVector(AnalHeightMap, WaterVelocity, WaterSpeed, Analwidth, Analheight, AnalSpeedmin, AnalSpeedmax);//속도벡터구하기
			break;
		}
	}
}

vec3 COpenGLRenderer::normalAtPixel(int x, int z, int hwidth, int hheight, vec3 *heights)
{//남중 높이맵에서 지정된 위치에서의 법선벡터를 구함
	vec3 n;

    if (x > 0 && x < hwidth - 1)
        n.x = heightAtPixel(x - 1, z, hwidth, heights) - heightAtPixel(x + 1, z, hwidth, heights);
    else if (x > 0)
        n.x = 2.0f * (heightAtPixel(x - 1, z, hwidth, heights) - heightAtPixel(x, z, hwidth, heights));
    else
        n.x = 2.0f * (heightAtPixel(x, z, hwidth, heights) - heightAtPixel(x + 1, z, hwidth, heights));

    if (z > 0 && z < hheight - 1)
        n.z = heightAtPixel(x, z - 1, hwidth, heights) - heightAtPixel(x, z + 1, hwidth, heights);
    else if (z > 0)
        n.z = 2.0f * (heightAtPixel(x, z - 1, hwidth, heights) - heightAtPixel(x, z, hwidth, heights));
    else
        n.z = 2.0f * (heightAtPixel(x, z, hwidth, heights) - heightAtPixel(x, z + 1, hwidth, heights));

//	n.y = 0.1f;//현재 법선y성분
	n.y = 0.002f;//디폴트 법선y성분
    n = normalize(n);
	return n;
}

void COpenGLRenderer::ImportAnalData()
{//해석데이터를 IMPORT하는 함수
	AlreadyAllPlayed = false;
	AnalyData = new AnalysisData();
	GetTrnTitle();
	AnalElapsedTime = 0.0;
//	AnalAnimPause = false;
	AnalAnimCnt = 0;
	ReadAnalysisData();
	AnalysisDataImported = true;
}

void COpenGLRenderer::GetTrnTitle()
{
	AnalyData->GetTrnTitle(trnfilename, wspfilename, vetfilename, Analwidth, Analheight, AnalBlockNum, AnalStartX, AnalStartY, AnalCTC_X, AnalCTC_Y);
}

bool COpenGLRenderer::ReadAnalysisData()
{
	DWORD LastAnalTime = GetTickCount();

//	glDeleteBuffers(1, &AnalNormalMapTexVBO);
	glDeleteBuffers(1, &AnalNormalVBO);
	glDeleteBuffers(1, &AnalVelocityColorVBO);
	glDeleteBuffers(1, &AnalElevationColorVBO);
	glDeleteBuffers(1, &AnalWaterVBO);

	if(AnalAnimCnt >= AnalBlockNum)
	{
		AnalAnimCnt = 0;
		AnalysisDataImported = false;
		AlreadyAllPlayed = true;
		delete AnalyData;

		ImportAnalData();

		return false;
	}
	else
	{
		free(AnalHeightMap);

		if(AnalWaterVelocityOn)
			free(WaterVelocity);

		if(AnalWaterSpeedOn)
			free(WaterSpeed);
	}

	float* WaterLevel;
	if(HeightMapLoadingSmoothed)
		AnalyData->GetSmoothedWaterLevel(AnalAnimCnt, WaterLevel);
	else
		AnalyData->GetWaterLevel(AnalAnimCnt, WaterLevel);

	AnalHeightMap = (vec3 *) malloc(Analwidth * Analheight * sizeof(vec3));

	createWaterLevelTRN(WaterLevel, Analwidth, Analheight, Analwidth, Analheight, AnalHeightMap,
		AnalElevationmin, AnalElevationmax, AnalStartX, AnalStartY, AnalCTC_X, AnalCTC_Y);
	free(WaterLevel);


	if(AnalWaterSpeedOn && AnalWaterVelocityOn==false)
		AnalyData->GetWaterSpeed(AnalAnimCnt, WaterSpeed, AnalSpeedmin, AnalSpeedmax);

	if(AnalWaterVelocityOn)
	{
		WaterVelocity = (vec3 *) malloc(Analwidth * Analheight * sizeof(vec3));
		AnalyData->GetWaterVelocity(AnalAnimCnt, WaterVelocity, WaterSpeed, AnalSpeedmin, AnalSpeedmax);
	}

/*
		defp = fopen("zdebug.txt", "a");
	fprintf(defp, "크하하 AnalBlockNum:%d AnalAnimCnt:%d\n",AnalBlockNum, AnalAnimCnt);
	fprintf(defp, "해석 %d %d %d %f %f %f %f\n",Analwidth, Analheight, AnalBlockNum, AnalStartX, AnalStartY, AnalCTC_X, AnalCTC_Y);
	fclose(defp);
*/

	AnalAnimCnt++;

	int hWMR = Analwidth;

	glGenBuffers(1, &AnalWaterVBO);
//	glGenBuffers(1, &AnalNormalMapTexVBO);
	glGenBuffers(1, &AnalNormalVBO);

	if(AnalWaterSpeedOn)
		glGenBuffers(1, &AnalVelocityColorVBO);
	else
		glGenBuffers(1, &AnalElevationColorVBO);

	int hWMRP1 = Analwidth;//가로길이
	int hWMRP2 = Analheight;//세로길이

	CBuffer hQuads;
	CBuffer tQuads;
	CBuffer nQuads;
	CBuffer cQuads;
	CBuffer cvQuads;

	float col_a, col_b, col_c, col_d;
	float heightdiff = AnalElevationmax-AnalElevationmin;
	float analveldiff = AnalSpeedmax-AnalSpeedmin;
	vec3 ecol1, ecol2, ecol3, ecol4;
	vec3 vcol1, vcol2, vcol3, vcol4;

	int resultxa, resultya, resultxb, resultyb, resultxc, resultyc, resultxd, resultyd;//유체높이맵에 대한 지형높이맵의 상대위치
	float ratx, raty, ratAnalwidth=Analwidth, ratAnalheight=Analheight, ratterrainwidth = terrainwidth, ratterrainheight = terrainheight;

	bool terrainAnalSameSize = false;
	if( TerrainCreatedOK == true)
		terrainAnalSameSize = true;

	for(int y = 0; y < hWMRP2-1; y++)
	{
		int yp1 = y + 1;

		for(int x = 0; x < hWMR-1; x++)
		{
			int xp1 = x + 1;

			int a = hWMRP1 * y + x;
			int b = hWMRP1 * y + xp1;
			int c = hWMRP1 * yp1 + xp1;
			int d = hWMRP1 * yp1 + x;

//해석데이터를 버퍼에 저장
			hQuads.AddData(&AnalHeightMap[a], 12);
			hQuads.AddData(&AnalHeightMap[b], 12);
			hQuads.AddData(&AnalHeightMap[c], 12);
			hQuads.AddData(&AnalHeightMap[d], 12);

//해석데이터 법선들 버퍼에 저장
			nQuads.AddData(&normalAtPixel(x, y, Analwidth, Analheight, AnalHeightMap), 12);
			nQuads.AddData(&normalAtPixel(x+1, y, Analwidth, Analheight, AnalHeightMap), 12);
			nQuads.AddData(&normalAtPixel(x+1, y+1, Analwidth, Analheight, AnalHeightMap), 12);
			nQuads.AddData(&normalAtPixel(x, y+1, Analwidth, Analheight, AnalHeightMap), 12);

/*
			//텍스쳐좌표 버퍼에 저장
			float tex_x = (float)(x)/(float)(Analwidth);
			float tex_y = (float)(y)/(float)(Analheight);
			float tex_xb = (float)(x+1)/(float)(Analwidth);
			float tex_yb = (float)(y+1)/(float)(Analheight);

			vec2 tex1 = vec2(tex_x, tex_y);
			vec2 tex2 = vec2(tex_xb, tex_y);
			vec2 tex3 = vec2(tex_xb, tex_yb);
			vec2 tex4 = vec2(tex_x, tex_yb);

			tQuads.AddData(&tex1, 8);
			tQuads.AddData(&tex2, 8);
			tQuads.AddData(&tex3, 8);
			tQuads.AddData(&tex4, 8);
*/

			if(AnalWaterSpeedOn)
			{
				col_a = (WaterSpeed[a]-AnalSpeedmin)/analveldiff;
				col_b = (WaterSpeed[b]-AnalSpeedmin)/analveldiff;
				col_c = (WaterSpeed[c]-AnalSpeedmin)/analveldiff;
				col_d = (WaterSpeed[d]-AnalSpeedmin)/analveldiff;

				float ecol_a, ecol_b, ecol_c, ecol_d;
				ecol_a = (AnalHeightMap[a].y-AnalElevationmin)/heightdiff;
				ecol_b = (AnalHeightMap[b].y-AnalElevationmin)/heightdiff;
				ecol_c = (AnalHeightMap[c].y-AnalElevationmin)/heightdiff;
				ecol_d = (AnalHeightMap[d].y-AnalElevationmin)/heightdiff;

				if(terrainAnalSameSize == true)
				{//지형높이맵과 물높이맵이 서로 맞으면
				ratx = x, raty = y;
				resultxa = (int)(ratterrainwidth*ratx/ratAnalwidth);
				resultya = (int)(ratterrainheight*raty/ratAnalheight);

				ratx = xp1, raty = y;
				resultxb = (int)(ratterrainwidth*ratx/ratAnalwidth);
				resultyb = (int)(ratterrainheight*raty/ratAnalheight);

				ratx = xp1, raty = yp1;
				resultxc = (int)(ratterrainwidth*ratx/ratAnalwidth);
				resultyc = (int)(ratterrainheight*raty/ratAnalheight);

				ratx = x, raty = yp1;
				resultxd = (int)(ratterrainwidth*ratx/ratAnalwidth);
				resultyd = (int)(ratterrainheight*raty/ratAnalheight);
				//유체높이맵과 지형높이맵을 맞춤


				vcol1 = vec3(AnalHeightMap[a].y-TerrainHeightMap[resultya*terrainwidth + resultxa].y, ecol_a, col_a);
				vcol2 = vec3(AnalHeightMap[b].y-TerrainHeightMap[resultyb*terrainwidth + resultxb].y, ecol_b, col_b);
				vcol3 = vec3(AnalHeightMap[c].y-TerrainHeightMap[resultyc*terrainwidth + resultxc].y, ecol_c, col_c);
				vcol4 = vec3(AnalHeightMap[d].y-TerrainHeightMap[resultyd*terrainwidth + resultxd].y, ecol_d, col_d);
			}
			else
			{
				vcol1 = vec3(0.0, ecol_a, col_a);
				vcol2 = vec3(0.0, ecol_b, col_b);
				vcol3 = vec3(0.0, ecol_c, col_c);
				vcol4 = vec3(0.0, ecol_d, col_d);
			}
				cvQuads.AddData(&vcol1, 12);
				cvQuads.AddData(&vcol2, 12);
				cvQuads.AddData(&vcol3, 12);
				cvQuads.AddData(&vcol4, 12);
			}
			else
			{//해석데이터의 Elevation값의 칼라코딩 데어터를 버퍼에 저장
			col_a = (AnalHeightMap[a].y-AnalElevationmin)/heightdiff;
			col_b = (AnalHeightMap[b].y-AnalElevationmin)/heightdiff;
			col_c = (AnalHeightMap[c].y-AnalElevationmin)/heightdiff;
			col_d = (AnalHeightMap[d].y-AnalElevationmin)/heightdiff;

			if(terrainAnalSameSize == true)
			{//지형높이맵과 물높이맵이 서로 맞으면
				ratx = x, raty = y;
				resultxa = (int)(ratterrainwidth*ratx/ratAnalwidth);
				resultya = (int)(ratterrainheight*raty/ratAnalheight);

				ratx = xp1, raty = y;
				resultxb = (int)(ratterrainwidth*ratx/ratAnalwidth);
				resultyb = (int)(ratterrainheight*raty/ratAnalheight);

				ratx = xp1, raty = yp1;
				resultxc = (int)(ratterrainwidth*ratx/ratAnalwidth);
				resultyc = (int)(ratterrainheight*raty/ratAnalheight);

				ratx = x, raty = yp1;
				resultxd = (int)(ratterrainwidth*ratx/ratAnalwidth);
				resultyd = (int)(ratterrainheight*raty/ratAnalheight);
				//유체높이맵과 지형높이맵을 맞춤


				ecol1 = vec3(AnalHeightMap[a].y-TerrainHeightMap[resultya*terrainwidth + resultxa].y, col_a, 0.0);
				ecol2 = vec3(AnalHeightMap[b].y-TerrainHeightMap[resultyb*terrainwidth + resultxb].y, col_b, 0.0);
				ecol3 = vec3(AnalHeightMap[c].y-TerrainHeightMap[resultyc*terrainwidth + resultxc].y, col_c, 0.0);
				ecol4 = vec3(AnalHeightMap[d].y-TerrainHeightMap[resultyd*terrainwidth + resultxd].y, col_d, 0.0);
			}
			else
			{
				ecol1 = vec3(0.0, col_a, 0.0);
				ecol2 = vec3(0.0, col_b, 0.0);
				ecol3 = vec3(0.0, col_c, 0.0);
				ecol4 = vec3(0.0, col_d, 0.0);
			}

			cQuads.AddData(&ecol1, 12);
			cQuads.AddData(&ecol2, 12);
			cQuads.AddData(&ecol3, 12);
			cQuads.AddData(&ecol4, 12);
			}
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, AnalWaterVBO);
	glBufferData(GL_ARRAY_BUFFER, hQuads.GetDataSize(), hQuads.GetData(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
/*
	glBindBuffer(GL_ARRAY_BUFFER, AnalNormalMapTexVBO);
	glBufferData(GL_ARRAY_BUFFER, tQuads.GetDataSize(), tQuads.GetData(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
*/
	glBindBuffer(GL_ARRAY_BUFFER, AnalNormalVBO);
	glBufferData(GL_ARRAY_BUFFER, nQuads.GetDataSize(), nQuads.GetData(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if(AnalWaterSpeedOn)
	{
		glBindBuffer(GL_ARRAY_BUFFER, AnalVelocityColorVBO);
		glBufferData(GL_ARRAY_BUFFER, cvQuads.GetDataSize(), cvQuads.GetData(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	else
	{
		glBindBuffer(GL_ARRAY_BUFFER, AnalElevationColorVBO);
		glBufferData(GL_ARRAY_BUFFER, cQuads.GetDataSize(), cQuads.GetData(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}


	QuadsAnalDataCount = hQuads.GetDataSize() / 12;

	hQuads.Empty();
//	tQuads.Empty();
	nQuads.Empty();
	cvQuads.Empty();
	cQuads.Empty();


	DWORD AnalTime = GetTickCount();
	float ElapsedTime = (AnalTime - LastAnalTime) * 0.001f;
	LastAnalTime;
	AnalElapsedTime += ElapsedTime;

	return true;
}

bool COpenGLRenderer::AllInit()
{//텍스쳐, STL파일, GPU등 연결설정
	Camera.Look(vec3(200.0f, 350.0f, 1000.0f), vec3(200.0f, 350.0f, 0.0f), true);//실제크기에맞춤
//	Camera.Look(vec3(0.0f, 1.75f, 5.0f), vec3(0.0f, 1.5f, 0.0f), true);//초기버젼


	bool Error = false;

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &ggl_max_texture_size);

	if(glewInit() != GLEW_OK)
	{
		ErrorLog.Set("glewInit failed!");
		return false;
	}

	if(!GLEW_VERSION_2_1)
	{
		ErrorLog.Set("OpenGL 2.1 not supported!");
		return false;
	}

	if(GLEW_EXT_texture_filter_anisotropic)
	{
		glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &ggl_max_texture_max_anisotropy_ext);
	}

	if(WGLEW_EXT_swap_control)
	{
		wglSwapIntervalEXT(0);
	}

	if(!GLEW_ARB_texture_non_power_of_two)
	{
		ErrorLog.Append("GL_ARB_texture_non_power_of_two not supported!\r\n");
		Error = true;
	}

	if(!GLEW_EXT_framebuffer_object)
	{
		ErrorLog.Append("GL_EXT_framebuffer_object not supported!\r\n");
		Error = true;
	}

	char *PoolSkyCubeMapFileNames[] = {"envmap\\right.jpg", "envmap\\left.jpg", "envmap\\bottom.jpg", "envmap\\top.jpg", "envmap\\front.jpg", "envmap\\back.jpg"};
//	char *PoolSkyCubeMapFileNames[] = {"envmap\\posx.bmp", "envmap\\negx.bmp", "envmap\\posy.bmp", "envmap\\negy.bmp", "envmap\\posz.bmp", "envmap\\negz.bmp"};
//	char *PoolSkyCubeMapFileNames[] = {"pool\\right.jpg", "pool\\left.jpg", "pool\\bottom.jpg", "pool\\top.jpg", "pool\\front.jpg", "pool\\back.jpg"};

	Error |= !PoolSkyCubeMap.LoadTextureCubeMap(PoolSkyCubeMapFileNames);

	char *DefaultTerrainTexFileName[] = {"image\\dirt.jpg", "image\\grass.jpg", "image\\rock.jpg", "image\\snow.jpg"};
	for(int i = 0; i < 4; i++)
	{//디폴트 Terrain 텍스쳐 할당
		Error |= !Texture[i].LoadTexture2D(DefaultTerrainTexFileName[i]);
	}

	SetCurrentDirectory(WinInitDir);
//	Error |= !AnalNormalMapTex.LoadTexture2D("image\\bricks_normal_map.jpg");
	Error |= !AnalNormalMapTex.LoadTexture2D("image\\waves2.dds");
	//남중 해석데이터 WATER 법선맵 텍스쳐 LOAD



	glGenBuffers(1, &PoolSkyVBO);

	float PoolSkyVertices[] =
	{	// x, y, z, x, y, z, x, y, z, x, y, z
		 1.0f, -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f, // +X
		-1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, // -X
		-1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, // +Y
		-1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, // -Y
		 1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f, // +Z
		-1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f  // -Z
	};

	glBindBuffer(GL_ARRAY_BUFFER, PoolSkyVBO);
	glBufferData(GL_ARRAY_BUFFER, 288, PoolSkyVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

//	loadCubeMapFromDDS("envmap\\CloudyHillsCubemap.dds");


//	for(i=0;i<NumOfSTLOBJ;i++)
//		DebugSTLFile(i);


	Error |= !EnvMapProgram.Load("shader\\envmapping.vs", "shader\\envmapping.fs");

	Error |= !WaterProgram.Load("shader\\water.vs", "shader\\water.fs");
	Error |= !AnalWaterProgram.Load("shader\\analwater.vs", "shader\\analwater.fs");

	Error |= !PoolSkyProgram.Load("shader\\poolsky.vs", "shader\\poolsky.fs");

	Error |= !TexTerrainProgram.Load("shader\\texterrain.vs", "shader\\texterrain.fs");
	Error |= !ColorCodingProgram.Load("shader\\colorcoding.vs", "shader\\colorcoding.fs");
	Error |= !TerrainColorCodingProgram.Load("shader\\terraincolorcoding.vs", "shader\\terraincolorcoding.fs");
	Error |= !STLShadingProgram.Load("shader\\stlshading.vs", "shader\\stlshading.fs");

	Error |= !OBJShadingProgram.Load("shader\\objshading.vs", "shader\\objshading.fs");
	Error |= !OBJNormalMapProgram.Load("shader\\objnormalmap.vs", "shader\\objnormalmap.fs");


	if(Error)
	{//남중 GPU연결안되면 끈다
	return false;
	}

	glGenTextures(1, &ReflectionTexture);
	glGenTextures(1, &RefractionTexture);
	glGenTextures(1, &DepthTexture);

	glGenFramebuffersEXT(1, &FBO);


//	InitEnvironment();
//	ReadFluidHeightMap();

		vec3 CubeMapNormals[6] = {
		vec3(-1.0f, 0.0f, 0.0f),
		vec3(1.0f, 0.0f, 0.0f),
		vec3(0.0f, -1.0f, 0.0f),
		vec3(0.0f, 1.0f, 0.0f),
		vec3(0.0f, 0.0f, -1.0f),
		vec3(0.0f, 0.0f, 1.0f),
	};


	glUseProgram(WaterProgram);
	glUniform1i(glGetUniformLocation(WaterProgram, "WaterHeightMap"), 0);
	glUniform1i(glGetUniformLocation(WaterProgram, "WaterNormalMap"), 1);
	glUniform1i(glGetUniformLocation(WaterProgram, "PoolSkyCubeMap"), 2);
	glUniform1f(glGetUniformLocation(WaterProgram, "ODWMS"), 1.0f / 2.0f);
	glUniform3fv(glGetUniformLocation(WaterProgram, "LightPosition"), 1, &LightPosition);
	glUniform3fv(glGetUniformLocation(WaterProgram, "CubeMapNormals"), 6, (float*)CubeMapNormals);
	glUseProgram(0);

	glUseProgram(AnalWaterProgram);
	glUniform1i(glGetUniformLocation(AnalWaterProgram, "PoolSkyCubeMap"), 2);
	glUniform3fv(glGetUniformLocation(AnalWaterProgram, "CameraPosition"), 1, &Camera.Position);
	glUniform1i(glGetUniformLocation(AnalWaterProgram, "ReflectionTexture"), 20);
	glUniform1i(glGetUniformLocation(AnalWaterProgram, "RefractionTexture"), 21);
	glUniform1i(glGetUniformLocation(AnalWaterProgram, "normalMap"), 15);
	glUseProgram(0);

	// ------------------------------------------------------------------------------------------------------------------------


	WaterProgram.UniformLocations = new GLuint[1];
	WaterProgram.UniformLocations[0] = glGetUniformLocation(WaterProgram, "CameraPosition");

	// ------------------------------------------------------------------------------------------------------------------------


	LightColor = vec3(1.0f, 1.0f, 1.0f);
	LightPosition = vec3(0.0f, 2.75f, -4.75f);
//	LightPosition = vec3(0.0f, 5.75f, 0.75f);


//	glEnable(GL_LIGHTING);
//	glEnable(GL_LIGHT0);

	glEnable(GL_LIGHTING);

	glLightfv(GL_LIGHT0, GL_AMBIENT, &vec4(LightColor * 0.25f, 1.0f));
	glLightfv(GL_LIGHT0, GL_DIFFUSE, &vec4(LightColor * 0.75f, 1.0f));
	glLightfv(GL_LIGHT0, GL_SPECULAR, &vec4(LightColor, 1.0f));
	glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 1.0f / 128.0f);
	glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 1.0f / 256.0f);

//	ReadNFLOWAnalHeightMap();//지하철표현할때읽음


	srand(GetTickCount());

	return true;
}

void COpenGLRenderer::AddMaterial(float* matArray)
{
	ambiMat[0] = matArray[0];
	ambiMat[1] = matArray[1];
	ambiMat[2] = matArray[2];
	ambiMat[3] = matArray[3];

	diffMat[0] = matArray[4];
	diffMat[1] = matArray[5];
	diffMat[2] = matArray[6];
	diffMat[3] = matArray[7];

	specMat[0] = matArray[8];
	specMat[1] = matArray[9];
	specMat[2] = matArray[10];
	specMat[3] = matArray[11];

	emisMat[0] = 0.0f;
	emisMat[1] = 0.0f;
	emisMat[2] = 0.0f;
	emisMat[3] = 0.0f;

	coloMat[0] = ambiMat[0]*1.2;
	coloMat[1] = ambiMat[1]*1.2;
	coloMat[2] = ambiMat[2]*1.2;
	coloMat[3] = ambiMat[3]*1.2;

	shine = matArray[12];
}

void COpenGLRenderer::ApplyMaterial(const int & mat)
{
	switch (mat)
	{
		//Default Setting
		case DEFAULT:
			AddMaterial(Default);
			break;
		//Non Metals
		case EMERALD:
			AddMaterial(Emerald);
			break;
		case JADE:
			AddMaterial(Jade);
			break;
		case OBSIDIAN:
			AddMaterial(Obsidian);
			break;
		case PEARL:
			AddMaterial(Pearl);
			break;
		case PEWTER:
			AddMaterial(Pewter);
			break;
		case RUBY:
			AddMaterial(Ruby);
			break;
		case TURQUOISE:
			AddMaterial(Turquoise);
			break;
		case PLASTIC:
			AddMaterial(Plastic);
			break;
		case RUBBER:
			AddMaterial(Rubber);
			break;
		//Metals
		case ALUMINIUM:
			AddMaterial(Aluminium);
			break;
		case BRASS:
			AddMaterial(Brass);
			break;
		case BRONZE:
			AddMaterial(Bronze);
			break;
		case P_BRONZE:
			AddMaterial(Polished_Bronze);
			break;
		case CHROME:
			AddMaterial(Chrome);
			break;
		case COPPER:
			AddMaterial(Copper);
			break;
		case P_COPPER:
			AddMaterial(Polished_Copper);
			break;
		case GOLD:
			AddMaterial(Gold);
			break;
		case P_GOLD:
			AddMaterial(Polished_Gold);
			break;
		case SILVER:
			AddMaterial(Silver);
			break;
		case P_SILVER:
			AddMaterial(Polished_Silver);
			break;
		case STEEL:
			AddMaterial(Steel);
			break;
	}

}

void COpenGLRenderer::DefineDisplay(int num)
{//물체그리는함수
	GLfloat bgcol[4];
	glGetFloatv(GL_COLOR_CLEAR_VALUE, bgcol);
	GLfloat  specref[] =  { 1.0f, 1.0f, 1.0f, 1.0f };
	// Enable Depth Testing
	glEnable(GL_DEPTH_TEST);

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

	// Enable lighting
	glEnable(GL_LIGHTING);

	// Enable color tracking
	glEnable(GL_COLOR_MATERIAL);
	
	// Set Material properties to follow glColor values
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,specref);
	glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,128);

	glShadeModel(GL_SMOOTH);

	glEnable(GL_AUTO_NORMAL);
	glEnable(GL_NORMALIZE);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	glPushAttrib(GL_LIGHTING_BIT);
	ApplyMaterial(stlFile[num]->itsMaterial);//물질속성설정
	DrawShaded(num);//물체표면에 색상칠함
	glPopAttrib();


	glDisable(GL_BLEND);
}


void COpenGLRenderer::DrawShaded(int num)
{//물체표면에 색상칠함
	glColor3ub(stlFile[num]->itsShadeRed, stlFile[num]->itsShadeGreen, stlFile[num]->itsShadeBlue);

	glUseProgram(STLShadingProgram);

	for(int i = 0; i < stlFile[num]->getStats().numFacets; ++i)
	{
	  glBegin(GL_TRIANGLES);//Stl File을 삼각형메쉬로 그리기
    glNormal3d(stlFile[num]->getFacets()[i].normal.x,
               stlFile[num]->getFacets()[i].normal.y,
               stlFile[num]->getFacets()[i].normal.z);
	glVertex3d(stlFile[num]->getFacets()[i].vector[0].x, stlFile[num]->getFacets()[i].vector[0].y, stlFile[num]->getFacets()[i].vector[0].z);
	glVertex3d(stlFile[num]->getFacets()[i].vector[1].x, stlFile[num]->getFacets()[i].vector[1].y, stlFile[num]->getFacets()[i].vector[1].z);
	glVertex3d(stlFile[num]->getFacets()[i].vector[2].x, stlFile[num]->getFacets()[i].vector[2].y, stlFile[num]->getFacets()[i].vector[2].z);
   glEnd();
  }

	glUseProgram(0);
}

void COpenGLRenderer::DrawSTLObjects()
{//Stl File을 그리는 함수
	glLoadMatrixf(&ViewMatrix);

	for(int i = 0;i < STLNum;i++)
	{
		if(i == FluidModelInd)
			continue;

		glPushMatrix();
		glTranslatef(xTrans, yTrans, zTrans);
	glScalef(ScaleFactor, ScaleFactor, ScaleFactor);
	glRotatef(xRot,1,0,0);
	glRotatef(yRot,0,1,0);
	glRotatef(zRot,0,0,1);
		DefineDisplay(i);
		glPopMatrix();
	}

}

void COpenGLRenderer::DrawSTLFluid()
{//Stl File 유체를 그리는 함수
	if(FluidModelInd >= 0)
	{
		glUseProgram(EnvMapProgram);
	glUniform1i(glGetUniformLocation(EnvMapProgram, "EnvMap"), 2);
	glUniform3fv(glGetUniformLocation(EnvMapProgram, "eyePos"), 1, &Camera.Position);
	glUniform1f(glGetUniformLocation(EnvMapProgram, "etaRatio"), IndexOfRefraction);
	glActiveTexture( GL_TEXTURE2 );
	glBindTexture(GL_TEXTURE_CUBE_MAP, PoolSkyCubeMap );
	glEnable(GL_TEXTURE_CUBE_MAP);

	glutSolidTeapot(200);

	glLoadMatrixf(&ViewMatrix);

		glPushMatrix();
		glTranslatef(xTrans, yTrans, zTrans);
	glScalef(ScaleFactor, ScaleFactor, ScaleFactor);
	glRotatef(xRot,1,0,0);
	glRotatef(yRot,0,1,0);
	glRotatef(zRot,0,0,1);
		DefineDisplay(FluidModelInd);
		glPopMatrix();

	glUseProgram(0);
	glDisable(GL_TEXTURE_CUBE_MAP);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0 );
	}
}

void COpenGLRenderer::DrawEnvMappedHeightMap()
{//유체높이맵그리기
			glUseProgram(EnvMapProgram);
	glUniform1i(glGetUniformLocation(EnvMapProgram, "EnvMap"), 2);
	glUniform3fv(glGetUniformLocation(EnvMapProgram, "eyePos"), 1, &Camera.Position);
	glUniform1f(glGetUniformLocation(EnvMapProgram, "etaRatio"), IndexOfRefraction);
	glActiveTexture( GL_TEXTURE2 );
	glBindTexture(GL_TEXTURE_CUBE_MAP, PoolSkyCubeMap );
	glEnable(GL_TEXTURE_CUBE_MAP);

	for (int z = 0; z < heightmapheight-1; z++)
	{//삼각형으로그리기
		glBegin(GL_TRIANGLE_STRIP);
		for (int x = 0; x < heightmapwidth-1; x++)
		{
			// draw vertex 0			
			glVertex3f(FluidHeightMap[z*heightmapwidth + x].x, FluidHeightMap[z*heightmapwidth + x].y, FluidHeightMap[z*heightmapwidth + x].z);

			// draw vertex 1
			glVertex3f(FluidHeightMap[z*heightmapwidth + x+1].x, FluidHeightMap[z*heightmapwidth + x+1].y, FluidHeightMap[z*heightmapwidth + x+1].z);

			// draw vertex 2
			glVertex3f(FluidHeightMap[(z+1)*heightmapwidth + x].x, FluidHeightMap[(z+1)*heightmapwidth + x].y, FluidHeightMap[(z+1)*heightmapwidth + x].z);

			// draw vertex 3
			glVertex3f(FluidHeightMap[(z+1)*heightmapwidth + x+1].x, FluidHeightMap[(z+1)*heightmapwidth + x+1].y, FluidHeightMap[(z+1)*heightmapwidth + x+1].z);
		}
		glEnd();
	}

		glUseProgram(0);
	glDisable(GL_TEXTURE_CUBE_MAP);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0 );
}

void COpenGLRenderer::BindTexture(GLuint texture, GLuint unit)
{
    glActiveTexture(GL_TEXTURE10 + unit);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);
}

void COpenGLRenderer::DisplayColorCodingRemark(int screenWidth, int screenHeight, float hmin, float hmax, int DispType)
{//칼라코딩 범례를 화면에 출력하는 함수
	int width=screenWidth, height=screenHeight;

	// If user didn't kindly give us the screen resolution, find it.
	if (width <= 0 || height <= 0)
	{
		width = glutGet( GLUT_WINDOW_WIDTH );
		height = glutGet( GLUT_WINDOW_HEIGHT );
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glPushAttrib( GL_ALL_ATTRIB_BITS );
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D( 0.0, width, 0.0, height );
	glMatrixMode( GL_MODELVIEW );


	if(DispType==TERRAIN_HEIGHT)
	{
	glPushMatrix();
	glLoadIdentity();
	glBegin(GL_QUADS);
	glColor3f(0.043137255,0.17254902, 0.478431373);		// 남
	glVertex2f(710.0f, 415.0f);
	glVertex2f(718.0f, 415.0f);
	glColor3f(0.239215686, 0.721568627, 0.407843137);		// 청록
	glVertex2f(718.0f, 455.0f);
	glVertex2f(710.0f, 455.0f);
	glEnd();

	glBegin(GL_QUADS);
	glColor3f(0.239215686, 0.721568627, 0.407843137);		// 청록
	glVertex2f(710.0f, 455.0f);
	glVertex2f(718.0f, 455.0f);
	glColor3f(0.443137255, 0.921568627, 0.184313725);		// 연두
	glVertex2f(718.0f, 495.0f);
	glVertex2f(710.0f, 495.0f);
	glEnd();

	glBegin(GL_QUADS);
	glColor3f(0.443137255, 0.921568627, 0.184313725);		// 연두
	glVertex2f(710.0f, 495.0f);
	glVertex2f(718.0f, 495.0f);
	glColor3f(1.0, 1.0,0.501960784);		// 연노랑
	glVertex2f(718.0f, 535.0f);
	glVertex2f(710.0f, 535.0f);

	glBegin(GL_QUADS);
	glColor3f(1.0, 1.0,0.501960784);		// 연노랑
	glVertex2f(710.0f, 535.0f);
	glVertex2f(718.0f, 535.0f);
	glColor3f(0.858823529, 0.28627451, 0.298039216);		// 빨
	glVertex2f(718.0f, 575.0f);
	glVertex2f(710.0f, 575.0f);
	glEnd();
	}
	else
	{
		glPushMatrix();
	glLoadIdentity();
	glBegin(GL_QUADS);
	glColor3f(0.043137255,0.17254902, 0.478431373);		// 남
	glVertex2f(10.0f, 415.0f);
	glVertex2f(18.0f, 415.0f);
	glColor3f(0.239215686, 0.721568627, 0.407843137);		// 청록
	glVertex2f(18.0f, 455.0f);
	glVertex2f(10.0f, 455.0f);
	glEnd();

	glBegin(GL_QUADS);
	glColor3f(0.239215686, 0.721568627, 0.407843137);		// 청록
	glVertex2f(10.0f, 455.0f);
	glVertex2f(18.0f, 455.0f);
	glColor3f(0.443137255, 0.921568627, 0.184313725);		// 연두
	glVertex2f(18.0f, 495.0f);
	glVertex2f(10.0f, 495.0f);
	glEnd();

	glBegin(GL_QUADS);
	glColor3f(0.443137255, 0.921568627, 0.184313725);		// 연두
	glVertex2f(10.0f, 495.0f);
	glVertex2f(18.0f, 495.0f);
	glColor3f(1.0, 1.0,0.501960784);		// 연노랑
	glVertex2f(18.0f, 535.0f);
	glVertex2f(10.0f, 535.0f);

	glBegin(GL_QUADS);
	glColor3f(1.0, 1.0,0.501960784);		// 연노랑
	glVertex2f(10.0f, 535.0f);
	glVertex2f(18.0f, 535.0f);
	glColor3f(0.858823529, 0.28627451, 0.298039216);		// 빨
	glVertex2f(18.0f, 575.0f);
	glVertex2f(10.0f, 575.0f);
	glEnd();
	}

	glPopMatrix();

	glMatrixMode( GL_PROJECTION );
	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );
	glPopAttrib();

	char buf[100];
	glColor3f(0,0,0);

	if(DispType==TERRAIN_HEIGHT)
	{
			sprintf( buf, "%.2f", hmin );
	DisplayString( 723, 417, buf, screenWidth, screenHeight );
	//범례 글자 출력하는 함수 호출

	sprintf( buf, "%.2f", (hmax-hmin)*(1.0/4.0)+hmin );
	DisplayString( 723, 455, buf, screenWidth, screenHeight );
	//범례 글자 출력하는 함수 호출

	sprintf( buf, "%.2f", (hmax-hmin)*(2.0/4.0)+hmin );
	DisplayString( 723, 495, buf, screenWidth, screenHeight );
	//범례 글자 출력하는 함수 호출

	sprintf( buf, "%.2f", (hmax-hmin)*(3.0/4.0)+hmin );
	DisplayString( 723, 535, buf, screenWidth, screenHeight );
	//범례 글자 출력하는 함수 호출

	sprintf( buf, "%.2f", hmax );
	DisplayString( 723, 568, buf, screenWidth, screenHeight );
	//범례 글자 출력하는 함수 호출


	sprintf( buf, "EL.(m)");

	DisplayString( 723, 581, buf, screenWidth, screenHeight );
	//범례 글자 출력하는 함수 호출
	}
	else
	{
	sprintf( buf, "%.2f", hmin );
	DisplayString( 23, 417, buf, screenWidth, screenHeight );
	//범례 글자 출력하는 함수 호출

	sprintf( buf, "%.2f", (hmax-hmin)*(1.0/4.0)+hmin );
	DisplayString( 23, 455, buf, screenWidth, screenHeight );
	//범례 글자 출력하는 함수 호출

	sprintf( buf, "%.2f", (hmax-hmin)*(2.0/4.0)+hmin );
	DisplayString( 23, 495, buf, screenWidth, screenHeight );
	//범례 글자 출력하는 함수 호출

	sprintf( buf, "%.2f", (hmax-hmin)*(3.0/4.0)+hmin );
	DisplayString( 23, 535, buf, screenWidth, screenHeight );
	//범례 글자 출력하는 함수 호출

	sprintf( buf, "%.2f", hmax );
	DisplayString( 23, 568, buf, screenWidth, screenHeight );
	//범례 글자 출력하는 함수 호출


	switch(DispType)
	{
	case ANAL_VELOCITYSTYLE:
		sprintf( buf, "Velocity.(m/s)");
		break;
	case ANAL_DEPTH:
		sprintf( buf, "Depth.(m)");
		break;
	case ANAL_ELEVATION:
		sprintf( buf, "EL.(m)");
		break;
	}

	DisplayString( 23, 581, buf, screenWidth, screenHeight );
	//범례 글자 출력하는 함수 호출
	}


/*
	int width=screenWidth, height=screenHeight;

	// If user didn't kindly give us the screen resolution, find it.
	if (width <= 0 || height <= 0)
	{
		width = glutGet( GLUT_WINDOW_WIDTH );
		height = glutGet( GLUT_WINDOW_HEIGHT );
	}

	glPushAttrib( GL_ALL_ATTRIB_BITS );
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D( 0.0, width, 0.0, height );
	glMatrixMode( GL_MODELVIEW );

	glPushMatrix();
	glLoadIdentity();
	glBegin(GL_QUADS);
	glColor3f(0.0, 0.0, 1.0);
	glVertex2f(5.0f, 0.0f);
	glVertex2f(35.0f, 0.0f);
	glColor3f(0.0, 1.0, 1.0);
	glVertex2f(35.0f, 50.0f);
	glVertex2f(5.0f, 50.0f);
	glEnd();

	glBegin(GL_QUADS);
	glColor3f(0.0, 1.0, 1.0);
	glVertex2f(5.0f, 50.0f);
	glVertex2f(35.0f, 50.0f);
	glColor3f(0.0, 1.0, 0.0);
	glVertex2f(35.0f, 100.0f);
	glVertex2f(5.0f, 100.0f);
	glEnd();

	glBegin(GL_QUADS);
	glColor3f(0.0, 1.0, 0.0);
	glVertex2f(5.0f, 100.0f);
	glVertex2f(35.0f, 100.0f);
	glColor3f(1.0, 1.0, 0.0);
	glVertex2f(35.0f, 150.0f);
	glVertex2f(5.0f, 150.0f);

	glBegin(GL_QUADS);
	glColor3f(1.0, 1.0, 0.0);
	glVertex2f(5.0f, 150.0f);
	glVertex2f(35.0f, 150.0f);
	glColor3f(1.0, 0.0, 0.0);
	glVertex2f(35.0f, 200.0f);
	glVertex2f(5.0f, 200.0f);
	glEnd();

	glPopMatrix();

	glMatrixMode( GL_PROJECTION );
	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );
	glPopAttrib();

	char buf[100];
	glColor3f(0,0,0);
	sprintf( buf, "min: %.2f", hmin );
	DisplayString( 40, 10, buf, screenWidth, screenHeight );
	//범례 글자 출력하는 함수 호출

	sprintf( buf, "max: %.2f", hmax );
	DisplayString( 40, 180, buf, screenWidth, screenHeight );
	//범례 글자 출력하는 함수 호출
*/
}

void COpenGLRenderer::DisplayElapsedTime( char* buf, int screenWidth, int screenHeight )
{//FPS를 화면에 출력하는 함수
	glColor3f(1,1,1);
	DisplayString( 10, 10, buf, screenWidth, screenHeight );
}


void COpenGLRenderer::DisplayTimer( float fps, int screenWidth, int screenHeight )
{//FPS를 화면에 출력하는 함수
	char buf[100];
	sprintf( buf, "%.2f fps", fps );

	glColor3f(1,1,1);
	DisplayString( 10, 10, buf, screenWidth, screenHeight );
}

void COpenGLRenderer::PrintString(char *str)
{//범례의 글자를 화면에 출력하는 함수
	int len, i;
  len = (int) strlen(str);
  for(i=0; i<len; i++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, str[i]);
  //GLUT_BITMAP_HELVETICA_18 <-글자크기18, 폰트 헬베티카
}

void COpenGLRenderer::DisplayString( int rasterPosX, int rasterPosY, char *str, int screenWidth, int screenHeight )
{//글자의 x위치:rasterPosX, 글자의 y위치:rasterPosY
	int width=screenWidth, height=screenHeight;

	// If user didn't kindly give us the screen resolution, find it.
	if (width <= 0 || height <= 0)
	{
		width = glutGet( GLUT_WINDOW_WIDTH );
		height = glutGet( GLUT_WINDOW_HEIGHT );
	}

	glPushAttrib( GL_ALL_ATTRIB_BITS );
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D( 0.0, width, 0.0, height );
	glMatrixMode( GL_MODELVIEW );

	glPushMatrix();
	glLoadIdentity();
	glRasterPos2i(rasterPosX, rasterPosY);
	PrintString( str );
	glPopMatrix();

	glMatrixMode( GL_PROJECTION );
	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );
	glPopAttrib();
}

void COpenGLRenderer::DrawTerrainColorCoding()
{
	GLfloat bgcol[4];
	glGetFloatv(GL_COLOR_CLEAR_VALUE, bgcol);
	GLfloat  specref[] =  { 1.0f, 1.0f, 1.0f, 1.0f };
	// Enable Depth Testing
	glEnable(GL_DEPTH_TEST);

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

	// Enable lighting
	glEnable(GL_LIGHTING);

	// Enable color tracking
	glEnable(GL_COLOR_MATERIAL);
	
	// Set Material properties to follow glColor values
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,specref);
	glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,128);

	glShadeModel(GL_SMOOTH);

	glEnable(GL_AUTO_NORMAL);
	glEnable(GL_NORMALIZE);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	glPushAttrib(GL_LIGHTING_BIT);

	glUseProgram(TerrainColorCodingProgram);

	glBindBuffer(GL_ARRAY_BUFFER, TerrainNormalVBO);
	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 12, (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, TerrainVBO);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 12, (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, TerrainColorMapVBO);
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(3, GL_FLOAT, 12, (void*)0);

	glDrawArrays(GL_QUADS, 0, QuadsTerrainCount);

	glUseProgram(0);

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glPopAttrib();

	glDisable(GL_BLEND);
}

void COpenGLRenderer::DrawTerrain()
{
	GLfloat bgcol[4];
	glGetFloatv(GL_COLOR_CLEAR_VALUE, bgcol);
	GLfloat  specref[] =  { 1.0f, 1.0f, 1.0f, 1.0f };
	// Enable Depth Testing
	glEnable(GL_DEPTH_TEST);

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

	// Enable lighting
	glEnable(GL_LIGHTING);

	// Enable color tracking
	glEnable(GL_COLOR_MATERIAL);
	
	// Set Material properties to follow glColor values
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,specref);
	glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,128);

	glShadeModel(GL_SMOOTH);

	glEnable(GL_AUTO_NORMAL);
	glEnable(GL_NORMALIZE);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	glPushAttrib(GL_LIGHTING_BIT);
	ApplyMaterial(RUBBER);//물질속성설정

	glColor3ub(230, 220, 110);

	glUseProgram(TexTerrainProgram);

	UpdateTerrainShaderParameters();//Terrain파라미터설정

	glBindBuffer(GL_ARRAY_BUFFER, TexCoordsVBO);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 8, (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, TerrainNormalVBO);
	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 12, (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, TerrainVBO);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 12, (void*)0);
	int i;
	if(MappingTextureExist==1)
	{
		glUniform1f(glGetUniformLocation(TexTerrainProgram, "tilingFactor"), 1);
		glUniform1i(glGetUniformLocation(TexTerrainProgram, "MappingTex"), 14);
		glUniform1i(glGetUniformLocation(TexTerrainProgram, "MappingTexExist"), MappingTextureExist);
		glActiveTexture(GL_TEXTURE14);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, MappingTexture);
	}
	else
	{
		for(i=0; i< 4; i++)
			BindTexture(Texture[i], i);
	}


	glDrawArrays(GL_QUADS, 0, QuadsTerrainCount);

	glUseProgram(0);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	if(MappingTextureExist==1)
	{
		glActiveTexture(GL_TEXTURE14);
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);
	}
	else
	{
		for(i = 3; i >= 0; --i)
		{
			glActiveTexture(GL_TEXTURE10 + i);
			glBindTexture(GL_TEXTURE_2D, 0);
			glDisable(GL_TEXTURE_2D);
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glPopAttrib();

	glDisable(GL_BLEND);
}

void COpenGLRenderer::DrawTerrainHeightMap(bool DisplayRemarkOK)
{//지형 높이맵을 그리는 함수
	glLoadMatrixf(&ViewMatrix);

	if(TerrainColorCodingOn)
	{
		DrawTerrainColorCoding();

		if(DisplayRemarkOK)
			DisplayColorCodingRemark(WIN_WIDTH, WIN_HEIGHT, heightmin, heightmax, TERRAIN_HEIGHT);
	}
	else
		DrawTerrain();

//	DisplayHeightMapVector(TerrainHeightMap, terrainwidth, terrainheight, heightmin, heightmax);
	//임시 지형높이맵 벡터표현

/*
	for (int z = 0; z < terrainheight-1; z++)
	{//삼각형으로그리기
		glBegin(GL_TRIANGLE_STRIP);
		for (int x = 0; x < terrainwidth-1; x++)
		{
			// draw vertex 0			
			glVertex3f(TerrainHeightMap[z*terrainwidth + x].x, TerrainHeightMap[z*terrainwidth + x].y, TerrainHeightMap[z*terrainwidth + x].z);

			// draw vertex 1
			glVertex3f(TerrainHeightMap[z*terrainwidth + x+1].x, TerrainHeightMap[z*terrainwidth + x+1].y, TerrainHeightMap[z*terrainwidth + x+1].z);

			// draw vertex 2
			glVertex3f(TerrainHeightMap[(z+1)*terrainwidth + x].x, TerrainHeightMap[(z+1)*terrainwidth + x].y, TerrainHeightMap[(z+1)*terrainwidth + x].z);

			// draw vertex 3
			glVertex3f(TerrainHeightMap[(z+1)*terrainwidth + x+1].x, TerrainHeightMap[(z+1)*terrainwidth + x+1].y, TerrainHeightMap[(z+1)*terrainwidth + x+1].z);
		}
		glEnd();
	}
*/
}

void COpenGLRenderer::DrawPoolSkyEnv()
{//환경맵 그리는 함수
	glViewport(0, 0, Width, Height);

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(&ProjectionMatrix);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(&ViewMatrix);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glBindTexture(GL_TEXTURE_CUBE_MAP, PoolSkyCubeMap);
	glUseProgram(PoolSkyProgram);
	glBindBuffer(GL_ARRAY_BUFFER, PoolSkyVBO);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 12, (void*)0);

	glPushMatrix();
	glScalef(10000,10000,10000);

	glDrawArrays(GL_QUADS, 0, 24);

	glPopMatrix();

	glDisableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

}

void COpenGLRenderer::Render(float FrameTime)
{//물, 환경, STL file등 그리는 함수
	if(Camera.CameraPathAnimationOn && Camera.CameraAnimPause)
		Camera.	SetAnimatingViewMatrix();

	if(ViewEnv)
	{
			// add drops --------------------------------------------------------------------------------------------------------------

	if(!Pause)
	{
		static DWORD LastTime = GetTickCount();

		DWORD Time = GetTickCount();

		if(Time - LastTime > 100)
		{
			LastTime = Time;

			AddDrop(2.0f * (float)rand() / (float)RAND_MAX - 1.0f, 1.0f - 2.0f * (float)rand() / (float)RAND_MAX, 4.0f / 128.0f * (float)rand() / (float)RAND_MAX);
		}
	}
	DrawPoolSkyEnv();//환경맵 그리는 함수 호출

	// reset modelview matrix and set light position --------------------------------------------------------------------------
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

//	LightPosition = Camera.Position;//광원위치를 카메라위치로 설정
	glLightfv(GL_LIGHT0, GL_POSITION, &vec4(LightPosition, 1.0f));


	// 물체들 그리기 ----------------------------------------------------------------------------------------------
	glLoadMatrixf(&ViewMatrix);

	if(WireFrame)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

//	DrawXYZAxis();//XYZ축그리는 함수호출
	if(TerrainCreatedOK)
		DrawTerrainHeightMap();
	DrawSTLObjects();//Stl File을 그리는 함수호출
	if(OBJModelLoaded)//OBJ파일 열려있으면 그리기
		DrawOBJModel();


	//Local Reflection ----------------------------------------------------------------------------------------------

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FBO);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, ReflectionTexture, 0);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, DepthTexture, 0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadMatrixf(&ViewMatrix);

	glCullFace(GL_FRONT);

	glPushMatrix();
//	glTranslatef(TerrainCenter.x, TerrainCenter.y, TerrainCenter.z);
//	glRotatef(-180.0, 0, 1, 0);
//	glScalef(1.0f, -1.0f, 1.0f);


	if(TerrainCreatedOK)
		DrawTerrainHeightMap(false);
	DrawSTLObjects();//Stl File을 그리는 함수호출
	if(OBJModelLoaded)//OBJ파일 열려있으면 그리기
		DrawOBJModel();

	glPopMatrix();

	glCullFace(GL_BACK);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);


	//Local Refraction ----------------------------------------------------------------------------------------------

//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadMatrixf(&ViewMatrix);

	if(TerrainCreatedOK)
		DrawTerrainHeightMap(false);
	DrawSTLObjects();//Stl File을 그리는 함수호출
	if(OBJModelLoaded)//OBJ파일 열려있으면 그리기
		DrawOBJModel();


	glBindTexture(GL_TEXTURE_2D, RefractionTexture);
	glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, Width, Height);
	glBindTexture(GL_TEXTURE_2D, 0);



//	DrawNFLOWAnalHeightMap();//지하철물그릴때NFLOW데이터씀



	//해석데이터 처리
	if(AnalysisDataImported == true)
		DrawAnalysisData();//해석데이터그리기
	if(AnalysisDataImported == true && AnalAnimPause == true)
		ReadAnalysisData();//해석데이터읽기


	glDisable(GL_CULL_FACE);


	DrawFluidHeightMap();

	glDisable(GL_DEPTH_TEST);
	}
	else
	{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glLightfv(GL_LIGHT0, GL_POSITION, &vec4(LightPosition, 1.0f));
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadMatrixf(&ViewMatrix);

//	DrawEnvironmentBox();
	DrawXYZAxis();//XYZ축그리는 함수호출


	DrawSTLObjects();//Stl File을 그리는 함수호출
}
}

void COpenGLRenderer::AddDrop(float x, float y, float DropRadius)
{
}

void COpenGLRenderer::DrawXYZAxis()
{//XYZ축그리는 함수호출
	glLoadMatrixf(&ViewMatrix);
	glPushMatrix();
	glRotatef(xRot,1,0,0);
	glRotatef(yRot,0,1,0);
	glRotatef(zRot,0,0,1);
	myTrihedron.SetSize(100);
	myTrihedron.DefineDisplay();//Axis그리기호출
	glPopMatrix();
}

void COpenGLRenderer::Resize(int Width, int Height)
{
	this->Width = Width;
	this->Height = Height;

	glViewport(0, 0, Width, Height);

	ProjectionMatrix = perspective(45.0f, (float)Width / (float)Height, 0.1f, 10000000000.0f);
	ProjectionBiasMatrixInverse = inverse(ProjectionMatrix) * BiasMatrixInverse;

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(&ProjectionMatrix);

	glBindTexture(GL_TEXTURE_2D, ReflectionTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindTexture(GL_TEXTURE_2D, RefractionTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindTexture(GL_TEXTURE_2D, DepthTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, Width, Height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void COpenGLRenderer::Destroy()
{
	for(int i=0;i<STLNum;i++)
	{
		stlFile[i]->close();
		delete stlFile[i];
	}

	free(WaterSpeed);
	free(WaterVelocity);
	free(AnalHeightMap);
	free(FluidHeightMap);
	free(TerrainHeightMap);
	free(NFLOWTerrainHeightMap);


	for(int i = 0; i < 4; i++)
	{
		Texture[i].Destroy();
	}
	AnalNormalMapTex.Destroy();
	MappingTexture.Destroy();

	OBJShadingProgram.Destroy();
	OBJNormalMapProgram.Destroy();
	STLShadingProgram.Destroy();
	AnalWaterProgram.Destroy();
	WaterProgram.Destroy();
	TexTerrainProgram.Destroy();
	ColorCodingProgram.Destroy();
	TerrainColorCodingProgram.Destroy();
	EnvMapProgram.Destroy();

//	glDeleteBuffers(1, &AnalNormalMapTexVBO);
	glDeleteBuffers(1, &AnalNormalVBO);
	glDeleteBuffers(1, &AnalElevationColorVBO);
	glDeleteBuffers(1, &AnalVelocityColorVBO);
	glDeleteBuffers(1, &AnalWaterVBO);
	glDeleteBuffers(1, &WaterVBO);
	glDeleteBuffers(1, &TerrainColorMapVBO);
	glDeleteBuffers(1, &TerrainNormalVBO);
	glDeleteBuffers(1, &TexCoordsVBO);
	glDeleteBuffers(1, &TerrainVBO);
	glDeleteTextures(2, WaterHeightMaps);
	glDeleteTextures(1, &WaterNormalMap);

	glDeleteBuffers(1, &NFLOWTerrainVBO);
	glDeleteBuffers(1, &NFLOWTerrainNormalVBO);


	glDeleteTextures(1, &ReflectionTexture);
	glDeleteTextures(1, &RefractionTexture);
	glDeleteTextures(1, &DepthTexture);

	if(GLEW_EXT_framebuffer_object)
	{
		glDeleteFramebuffersEXT(1, &FBO);
	}
}

void COpenGLRenderer::SetWaveInShaders(int Wave)
{
	glUseProgram(Water);
	glUniform2fv(Water.UniformLocations[4 + Wave * 5 + 0], 1, &Waves[Wave].Position);
	glUniform1f(Water.UniformLocations[4 + Wave * 5 + 1], Waves[Wave].StartTime);
	glUniform1f(Water.UniformLocations[4 + Wave * 5 + 2], Waves[Wave].Speed);
	glUniform1f(Water.UniformLocations[4 + Wave * 5 + 3], Waves[Wave].MaxY);
	glUniform1f(Water.UniformLocations[4 + Wave * 5 + 4], Waves[Wave].FrequencyMPIM2);
	glUseProgram(0);
}

#include "stdafx.h"
#include "landscapecreator.h"

#include <stdlib.h>

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

float vertexheightf(float f)
{
    return f / 1000.0f + 5.0f;
}

void createLandscapeFloat(float *heightmap, int heightmap_width, int heightmap_height,
                          int width, int height, vec3 *& vertices)
{
    vertices = (vec3 *) malloc(width * height * sizeof(vec3));
    //CHECK_NOTNULL(vertices);

    for(int y = 0; y < height; y ++)
    {
        for(int x = 0; x < width; x ++)
        {
            int imgx = x * (heightmap_width - 1) / (width - 1);
            int imgy = y * (heightmap_height - 1) / (height - 1);
            vec3 v;
            v.x = x * 20.0f / (width - 1) - 10;
            v.z = y * 20.0f / (height - 1) - 10;
            v.y = vertexheightf(heightmap[imgy * heightmap_width + imgx]);
            vertices[y * width + x] = v;
        }
    }
}

vec3 createLandscapeTRN(float *heightmap, int heightmap_width, int heightmap_height,
                          int width, int height, vec3 *& vertices, float& hmin, float& hmax, float StartX, float StartY, float CTC_X, float CTC_Y)
{
	vec3 hcenter;
	float xmin=100000.0, xmax=-100000.0, zmin=100000.0, zmax=-100000.0;
    vertices = (vec3 *) malloc(width * height * sizeof(vec3));
    //CHECK_NOTNULL(vertices);
	hmin = 100000.0;
	hmax = -100000.0;

//	gridScaleX = (float)(width / heightmap_width);
//	gridScaleY = (float)(height / heightmap_height);

    for(int y = 0; y < height; y ++)
    {

        for(int x = 0; x < width; x ++)
        {
            int imgx = x * (heightmap_width - 1) / (width - 1);
            int imgy = y * (heightmap_height - 1) / (height - 1);
            vec3 v;
            v.x = CTC_X*x+StartX;
            v.z = -(CTC_Y*y+StartY);//X축으로-90도 뒤집음
            v.y = heightmap[imgy * heightmap_width + imgx];

			if(v.y < hmin)
				hmin = v.y;
			if(v.y > hmax)
				hmax = v.y;

			if(v.x < xmin)
				xmin = v.x;
			if(v.x > xmax)
				xmax = v.x;

			if(v.z < zmin)
				zmin = v.z;
			if(v.z > zmax)
				zmax = v.z;

            vertices[y * width + x] = v;
        }
    }
	hcenter = vec3((xmin+xmax)/2.0, (hmin+hmax)/2.0, (zmin+zmax)/2.0);

	return hcenter;
}

void createWaterLevelTRN(float *heightmap, int heightmap_width, int heightmap_height,
                          int width, int height, vec3 *& vertices, float& hmin, float& hmax, float StartX, float StartY, float CTC_X, float CTC_Y)
{
    //CHECK_NOTNULL(vertices);
	hmin = 100000.0;
	hmax = -100000.0;

//	gridScaleX = (float)(width / heightmap_width);
//	gridScaleY = (float)(height / heightmap_height);

    for(int y = 0; y < height; y ++)
    {

        for(int x = 0; x < width; x ++)
        {
            int imgx = x * (heightmap_width - 1) / (width - 1);
            int imgy = y * (heightmap_height - 1) / (height - 1);
            vec3 v;
            v.x = CTC_X*x+StartX;
            v.z = -(CTC_Y*y+StartY);//X축으로-90도 뒤집음
            v.y = heightmap[imgy * heightmap_width + imgx];

			if(v.y > 0 && v.y < hmin)
				hmin = v.y;
			if(v.y > hmax)
				hmax = v.y;

            vertices[y * width + x] = v;
        }
    }
}

void readTRN(const char* filename, int& width, int& height, float*& heightmap)
{
	FILE *fp = fopen(filename, "rb");

	fread(&width, sizeof(int), 1, fp);
	fread(&height, sizeof(int), 1, fp);

	heightmap = (float*)malloc(sizeof(float) * height * width);

	float *pBuf = (float*) malloc(sizeof(float) * width);
	size_t i = 0;


	for (int y = 0; y < height; y++)
	{
		fread(pBuf, sizeof(float), width, fp);


		for (int x = 0; x < width; x++)
		{
			heightmap[i] = (float)pBuf[x];
			i++;
		}
	}

	free(pBuf);

	fclose(fp);
}

void readSmoothedTRN(const char* filename, int& width, int& height, float*& heightmap)
{//HeightMap을 Smoothing해서 Load
	FILE *fp = fopen(filename, "rb");

	fread(&width, sizeof(int), 1, fp);
	fread(&height, sizeof(int), 1, fp);

	float* smheightmap = (float*)malloc(sizeof(float) * height * width);
	heightmap = (float*)malloc(sizeof(float) * height * width);

	float *pBuf = (float*) malloc(sizeof(float) * width);
	size_t i = 0;

	int x, y;
	for (y = 0; y < height; y++)
	{
		fread(pBuf, sizeof(float), width, fp);

		for (x = 0; x < width; x++)
		{
			smheightmap[i] = (float)pBuf[x];
			i++;
		}
	}


	//높이맵을 스무딩
    float value = 0.0f;
    float cellAverage = 0.0f;
    int bounds = width * height;

    for (y = 0; y < height; ++y)
    {
        for (x = 0; x < width; ++x)
        {
            value = 0.0f;
            cellAverage = 0.0f;

            i = (y - 1) * width + (x - 1);
            if (i >= 0 && i < bounds)
            {
                value += smheightmap[i];
                cellAverage += 1.0f;
            }

            i = (y - 1) * width + x;
            if (i >= 0 && i < bounds)
            {
                value += smheightmap[i];
                cellAverage += 1.0f;
            }

            i = (y - 1) * width + (x + 1);
            if (i >= 0 && i < bounds)
            {
                value += smheightmap[i];
                cellAverage += 1.0f;
            }

            i = y * width + (x - 1);
            if (i >= 0 && i < bounds)
            {
                value += smheightmap[i];
                cellAverage += 1.0f;
            }

            i = y * width + x;
            if (i >= 0 && i < bounds)
            {
                value += smheightmap[i];
                cellAverage += 1.0f;
            }

            i = y * width + (x + 1);
            if (i >= 0 && i < bounds)
            {
                value += smheightmap[i];
                cellAverage += 1.0f;
            }

            i = (y + 1) * width + (x - 1);
            if (i >= 0 && i < bounds)
            {
                value += smheightmap[i];
                cellAverage += 1.0f;
            }

            i = (y + 1) * width + x;
            if (i >= 0 && i < bounds)
            {
                value += smheightmap[i];
                cellAverage += 1.0f;
            }

            i = (y + 1) * width + (x + 1);
            if (i >= 0 && i < bounds)
            {
                value += smheightmap[i];
                cellAverage += 1.0f;
            }

            heightmap[y * width + x] = value / cellAverage;
        }
    }

	free(pBuf);
	free(smheightmap);

	fclose(fp);
}

void readTRN2(const char* filename, int width, int height, int *&gridtypes)
{
	float* heightmap;
	readTRN(filename, width, height, heightmap);

    int gridwidth = width + 2;
    int gridheight = height + 2;

    gridtypes = (int *) malloc(gridwidth * gridheight * sizeof(int));

    for(int y = 0; y < gridheight; y ++)
    {
        for(int x = 0; x < gridwidth; x ++)
        {
			int x2 = max(min(x - 1, width - 1), 0);
			int y2 = max(min(y - 1, height - 1), 0);
			int type = (int)(heightmap[y2 * width + x2] + 0.5);
			if (type == 0)
				type *= 1;

            gridtypes[y * gridwidth + x] = type;
        }
    }

	free(heightmap);

}


AnalysisData::AnalysisData()
{
	m_nMeshBlick = 0;
	m_fStartPointX = -1.0;
	m_fStartPointY = -1.0;
	m_fCTC_X = -1.0;
	m_fCTC_Y = -1.0;
}

AnalysisData::~AnalysisData()
{
}

void AnalysisData::OpenWaterData(char* strPathName)
{
/*
	FILE *pFile = NULL;
	CString sMeshBlock = "";

	int nStringPoint = strPathName.GetLength() - 3;
	CString sSaveFileName = strPathName.Left(nStringPoint);
	m_sPathName = sSaveFileName + "trn";
	m_sPathNameWsp = sSaveFileName + "wps";

	// 파일 초기화 (Delete)
	CString sDeleteTemp = sSaveFileName + "temp";
	CString sDeleteTrn = sSaveFileName + "trn";
	CString sDeleteWsp = sSaveFileName + "wsp";

	remove( sDeleteTemp );
	remove( sDeleteTrn );
	remove( sDeleteWsp );

	int nINDEX_X = 0;
	int nINDEX_Y = 0;
	int nINDEX_Z = 0;
	int nINDEX_F = 0;
	int nINDEX_U = 0;
	int nINDEX_V = 0;
	int nINDEX_W = 0;
	int nINDEX_SRFHT = 0;
	int nINDEX_FLDPTH = 0;

	if(pFile = fopen(strPathName, "r"))
	{
		BOOL bStartMeshBlock = TRUE;

		while( !feof( pFile ) )
        {
			char strTemp[255];
			CString pStr = fgets( strTemp, sizeof(strTemp), pFile );

			CString sMake = pStr.Left(10); // Data 시작위치 확인

			if(sMake == "Mesh Block")
			{
				m_nMeshBlick++;

				pStr = fgets( strTemp, sizeof(strTemp), pFile ); // 한줄 이동

				if(bStartMeshBlock)
				{
					float fDataSet[10];

					fscanf(pFile, "%f %f %f %f %f %f %f %f %f %f", &fDataSet[0], &fDataSet[1], &fDataSet[2], &fDataSet[3], &fDataSet[4], &fDataSet[5], &fDataSet[6], &fDataSet[7], &fDataSet[8], &fDataSet[9]);

					m_nCountX = (int)(fDataSet[5] - fDataSet[4]) + 1;
					m_nCountY = (int)(fDataSet[7] - fDataSet[6]) + 1;
					m_nCountZ = (int)(fDataSet[9] - fDataSet[8]) + 1;

				
					pStr = fgets( strTemp, sizeof(strTemp), pFile ); // 한줄 이동
					// Data index를 설정하는 위치
					// 출력된 Data의 위치가 수시로 변경되므로 위치를 설정해야함.
					for(int nIdx = 0; nIdx < 9; nIdx++)
					{
						CString sIdxName;
						fscanf(pFile, "%s", sIdxName);

						if(sIdxName == "x")				nINDEX_X = nIdx;
						else if(sIdxName == "y")		nINDEX_Y = nIdx;
						else if(sIdxName == "z")		nINDEX_Z = nIdx;
						else if(sIdxName == "f")		nINDEX_F = nIdx;
						else if(sIdxName == "u")		nINDEX_U = nIdx;
						else if(sIdxName == "v")		nINDEX_V = nIdx;
						else if(sIdxName == "w")		nINDEX_W = nIdx;
						else if(sIdxName == "srfht")	nINDEX_SRFHT = nIdx;
						else if(sIdxName == "fldpth")	nINDEX_FLDPTH = nIdx;
					}

					pStr = fgets( strTemp, sizeof(strTemp), pFile ); // 한줄 이동

					// Data 배열 동적으로 설정
					float* WaterLevel;
					float* WaterSpeed;
					WaterLevel = (float*)malloc(sizeof(float) * m_nCountX * m_nCountY);
					WaterSpeed = (float*)malloc(sizeof(float) * m_nCountX * m_nCountY);

					int i = 0;
					for (int y = 0; y < m_nCountX; y++)
					{
						for (int x = 0; x < m_nCountY; x++)
						{
							WaterLevel[i] = 0.0;
							WaterSpeed[i] = 0.0;
							i++;
						}
					}

					float fDataTrn[9];
					while( !feof( pFile ) )
					{
						//Data 초기화로 현재 데이터가 있는지 확인
						for(int nIdx = 0; nIdx < 9; nIdx++)
							fDataTrn[nIdx] = -1000.0;

						fscanf(pFile, "%f %f %f %f %f %f %f %f %f", &fDataTrn[0], &fDataTrn[1], &fDataTrn[2], &fDataTrn[3], &fDataTrn[4], &fDataTrn[5], &fDataTrn[6], &fDataTrn[7], &fDataTrn[8]);

						if(fDataTrn[nINDEX_X] == -1000.0)
							break;

						int nIdxX = 0;
						int nIdxY = 0;

						if(m_fStartPointX == -1.0)
						{
							m_fStartPointX = fDataTrn[nINDEX_X];
							m_fStartPointY = fDataTrn[nINDEX_Y];
						}

						float fLengthX = fDataTrn[nINDEX_X] - m_fStartPointX;
						float fLengthY = fDataTrn[nINDEX_Y] - m_fStartPointY;

						if(fLengthX == 0)
						{
							nIdxX = 0;
						}
						else if(m_fCTC_X == -1)
						{
							m_fCTC_X = fLengthX;
							nIdxX = 1;
						}
						else
						{
							nIdxX = (int)(fLengthX / m_fCTC_X);
						}


						if(fLengthY == 0)
						{
							nIdxY = 0;
						}
						else if(m_fCTC_Y == -1)
						{
							m_fCTC_Y = fLengthY;
							nIdxY = 1;
						}
						else
						{
							nIdxY = (int)(fLengthY / m_fCTC_Y);
						}

						int nTrnIdx = nIdxY * m_nCountX + nIdxX;

						float fSqrt = sqrtf(fDataTrn[nINDEX_U] * fDataTrn[nINDEX_U] + fDataTrn[nINDEX_V] * fDataTrn[nINDEX_V] + fDataTrn[nINDEX_W] * fDataTrn[nINDEX_W]);
						WaterSpeed[nTrnIdx] += fSqrt;

						if(fDataTrn[nINDEX_F] < 0.005)
							continue;
						
						float fData = WaterLevel[nTrnIdx];

						if(fData < fDataTrn[nINDEX_SRFHT])
						{
							WaterLevel[nTrnIdx] = fDataTrn[nINDEX_SRFHT];
						}
					}

					SaveTrn(sSaveFileName, WaterLevel); // 저장
					SaveWsp(sSaveFileName, WaterSpeed); // 저장
					free(WaterLevel);

					bStartMeshBlock = FALSE;
				}
				else
				{
					pStr = fgets( strTemp, sizeof(strTemp), pFile ); // 한줄 이동
					pStr = fgets( strTemp, sizeof(strTemp), pFile ); // 한줄 이동

					// Data 배열 동적으로 설정
					float* WaterLevel;
					float* WaterSpeed;
					WaterLevel = (float*)malloc(sizeof(float) * m_nCountX * m_nCountY);
					WaterSpeed = (float*)malloc(sizeof(float) * m_nCountX * m_nCountY);

					int i = 0;
					for (int y = 0; y < m_nCountX; y++)
					{
						for (int x = 0; x < m_nCountY; x++)
						{
							WaterLevel[i] = 0.0;
							WaterSpeed[i] = 0.0;
							i++;
						}
					}

					float fDataTrn[9];
					while( !feof( pFile ) )
					{
						//Data 초기화로 현재 데이터가 있는지 확인
						for(int nIdx = 0; nIdx < 9; nIdx++)
							fDataTrn[nIdx] = -1000.0;

						fscanf(pFile, "%f %f %f %f %f %f %f %f %f", &fDataTrn[0], &fDataTrn[1], &fDataTrn[2], &fDataTrn[3], &fDataTrn[4], &fDataTrn[5], &fDataTrn[6], &fDataTrn[7], &fDataTrn[8]);

						if(fDataTrn[nINDEX_X] == -1000.0)
							break;

						int nIdxX = 0;
						int nIdxY = 0;

						float fLengthX = fDataTrn[nINDEX_X] - m_fStartPointX;
						float fLengthY = fDataTrn[nINDEX_Y] - m_fStartPointY;

						nIdxX = (int)(fLengthX / m_fCTC_X);
						nIdxY = (int)(fLengthY / m_fCTC_Y);

						int nTrnIdx = nIdxY * m_nCountX + nIdxX;

						float fSqrt = sqrtf(fDataTrn[nINDEX_U] * fDataTrn[nINDEX_U] + fDataTrn[nINDEX_V] * fDataTrn[nINDEX_V] + fDataTrn[nINDEX_W] * fDataTrn[nINDEX_W]);
						WaterSpeed[nTrnIdx] += fSqrt;

						if(fDataTrn[nINDEX_F] < 0.005)
							continue;

						float fData = WaterLevel[nTrnIdx];

						if(fData < fDataTrn[nINDEX_SRFHT])
						{
							WaterLevel[nTrnIdx] = fDataTrn[nINDEX_SRFHT];
						}
					}

					SaveTrn(sSaveFileName, WaterLevel); // 저장
					SaveWsp(sSaveFileName, WaterSpeed); // 저장
					free(WaterLevel);

				}
			}
        }

		SaveTrnTotal(sSaveFileName); // 저장
        fclose( pFile );
	}

*/
}

void AnalysisData::SaveTrnTotal(char* filename)
{
/*
	CString sSaveFileName = filename + "trn";
	CString sTempFileName = filename + "temp";

	FILE *pFile = NULL;
	pFile = fopen(sSaveFileName, "ab");

	FILE *pFileTemp = NULL;
	pFileTemp = fopen(sTempFileName, "rb");

	fwrite(&m_nMeshBlick, sizeof(int), 1, pFile);
	fwrite(&m_nCountX, sizeof(int), 1, pFile);
	fwrite(&m_nCountY, sizeof(int), 1, pFile);
	fwrite(&m_fCTC_X, sizeof(float), 1, pFile);
	fwrite(&m_fCTC_Y, sizeof(float), 1, pFile);
	fwrite(&m_fStartPointX, sizeof(float), 1, pFile);
	fwrite(&m_fStartPointY, sizeof(float), 1, pFile);

	int nCount = m_nCountX * m_nCountY * m_nMeshBlick;
	
	for(int nIdx = 0; nIdx < nCount; nIdx++)
	{
		float fValue = -1000.0;
		fread(&fValue, sizeof(float), 1, pFileTemp);
		fwrite(&fValue, sizeof(float), 1, pFile);
	}

	fclose(pFile);
	fclose(pFileTemp);
	remove( sTempFileName );
*/
}

void AnalysisData::SaveTrn(char* filename, float*& WaterLevel)
{
/*
	CString sSaveFileName = filename + "temp";
	
	FILE *pFile = NULL;
	pFile = fopen(sSaveFileName, "ab");

	fwrite(WaterLevel, sizeof(float),m_nCountX * m_nCountY, pFile);

	fclose(pFile);
*/
}

void AnalysisData::SaveWsp(char* filename, float*& WaterSpeed)
{
/*
	CString sSaveFileName = filename + "Wsp";
	
	FILE *pFile = NULL;
	pFile = fopen(sSaveFileName, "ab");

	int nCount = m_nCountX * m_nCountY;

	for(int nIdx = 0; nIdx < nCount; nIdx++)
	{
		float fValue = WaterSpeed[nIdx];
		fValue = fValue / m_nCountZ;

		fwrite(&fValue, sizeof(float), 1, pFile);
	}

	fclose(pFile);
*/
}

void AnalysisData::GetWaterLevel(int nGetIdx, float*& WaterLevel)
{
	FILE *pFile = fopen(m_sPathName, "rb");

	fseek(pFile, sizeof(int) * 3, SEEK_SET);
	fseek(pFile, sizeof(float) * 4, SEEK_CUR);

	for(int nIdx = 0; nIdx < nGetIdx; nIdx++)
	{
		fseek(pFile, sizeof(float) * m_nCountX * m_nCountY, SEEK_CUR);
	}

	WaterLevel = (float*)malloc(sizeof(float) * m_nCountX * m_nCountY);
	float *pBuf = (float*) malloc(sizeof(float) * m_nCountX * m_nCountY);

	int position = 0;

	fread(pBuf, sizeof(float), m_nCountX * m_nCountY, pFile);
	memcpy(WaterLevel, pBuf, m_nCountX * m_nCountY*sizeof(float));

	free(pBuf);

	fclose(pFile);

/*
	FILE *pFile = fopen(m_sPathName, "rb");

	fseek(pFile, sizeof(int) * 3, SEEK_SET);
	fseek(pFile, sizeof(float) * 4, SEEK_CUR);

	for(int nIdx = 0; nIdx < nGetIdx; nIdx++)
	{
		fseek(pFile, sizeof(float) * m_nCountX * m_nCountY, SEEK_CUR);
	}

	WaterLevel = (float*)malloc(sizeof(float) * m_nCountX * m_nCountY);
	float *pBuf = (float*) malloc(sizeof(float) * m_nCountX);

	int position = 0;

//#pragma omp parallel
//{
//#pragma omp for
	for (int y = 0; y < m_nCountY; y++)
	{
		fread(pBuf, sizeof(float), m_nCountX, pFile);

		memcpy(WaterLevel + position, pBuf, m_nCountX*sizeof(float));
		position += m_nCountX;
	}
//}
	free(pBuf);

	fclose(pFile);
*/
}

void AnalysisData::GetSmoothedWaterLevel(int nGetIdx, float*& WaterLevel)
{//스무딩된 해석데이터를 LOAD
	FILE *pFile = fopen(m_sPathName, "rb");

	fseek(pFile, sizeof(int) * 3, SEEK_SET);
	fseek(pFile, sizeof(float) * 4, SEEK_CUR);

	for(int nIdx = 0; nIdx < nGetIdx; nIdx++)
	{
		fseek(pFile, sizeof(float) * m_nCountX * m_nCountY, SEEK_CUR);
	}

	float* smWaterLevel = (float*)malloc(sizeof(float) * m_nCountX * m_nCountY);
	WaterLevel = (float*)malloc(sizeof(float) * m_nCountX * m_nCountY);
	float *pBuf = (float*) malloc(sizeof(float) * m_nCountX);


	int position = 0;
int x, y, i;
//#pragma omp parallel
//{
//#pragma omp for
	for (y = 0; y < m_nCountY; y++)
	{
		fread(pBuf, sizeof(float), m_nCountX, pFile);

		memcpy(smWaterLevel + position, pBuf, m_nCountX*sizeof(float));
		position += m_nCountX;
	}
//}


	//높이맵을 스무딩
    float value = 0.0f;
    float cellAverage = 0.0f;
    int bounds = m_nCountX * m_nCountY;

    for (y = 0; y < m_nCountY; ++y)
    {
        for (x = 0; x < m_nCountX; ++x)
        {
            value = 0.0f;
            cellAverage = 0.0f;

            i = (y - 1) * m_nCountX + (x - 1);
            if (i >= 0 && i < bounds)
            {
                value += smWaterLevel[i];
                cellAverage += 1.0f;
            }

            i = (y - 1) * m_nCountX + x;
            if (i >= 0 && i < bounds)
            {
                value += smWaterLevel[i];
                cellAverage += 1.0f;
            }

            i = (y - 1) * m_nCountX + (x + 1);
            if (i >= 0 && i < bounds)
            {
                value += smWaterLevel[i];
                cellAverage += 1.0f;
            }

            i = y * m_nCountX + (x - 1);
            if (i >= 0 && i < bounds)
            {
                value += smWaterLevel[i];
                cellAverage += 1.0f;
            }

            i = y * m_nCountX + x;
            if (i >= 0 && i < bounds)
            {
                value += smWaterLevel[i];
                cellAverage += 1.0f;
            }

            i = y * m_nCountX + (x + 1);
            if (i >= 0 && i < bounds)
            {
                value += smWaterLevel[i];
                cellAverage += 1.0f;
            }

            i = (y + 1) * m_nCountX + (x - 1);
            if (i >= 0 && i < bounds)
            {
                value += smWaterLevel[i];
                cellAverage += 1.0f;
            }

            i = (y + 1) * m_nCountX + x;
            if (i >= 0 && i < bounds)
            {
                value += smWaterLevel[i];
                cellAverage += 1.0f;
            }

            i = (y + 1) * m_nCountX + (x + 1);
            if (i >= 0 && i < bounds)
            {
                value += smWaterLevel[i];
                cellAverage += 1.0f;
            }

            WaterLevel[y * m_nCountX + x] = value / cellAverage;
        }
    }


	free(smWaterLevel);
	free(pBuf);

	fclose(pFile);
}

void AnalysisData::GetWaterVelocity(int nGetIdx, vec3 *& WaterVel, float*& WaterSpeed, float& hmin, float& hmax)
{//vec3타입의 물속도를 얻어오는 함수
	FILE *pFile = fopen(m_sPathNameVet, "rb");

	fseek(pFile, 0, SEEK_SET);

	for(int nIdx = 0; nIdx < nGetIdx; nIdx++)
	{
		fseek(pFile, sizeof(float) * m_nCountX * m_nCountY*3, SEEK_CUR);
	}

	WaterSpeed = (float*)malloc(sizeof(float) * m_nCountX * m_nCountY);
	float *pBuf = (float*) malloc(sizeof(float) * m_nCountX*3);
	size_t i = 0;
	hmin = 100000.0;
	hmax = -100000.0;

	for (int y = 0; y < m_nCountY; y++)
	{
		fread(pBuf, sizeof(float), m_nCountX*3, pFile);
		for (int x = 0; x < m_nCountX; x++)
		{
			WaterVel[i].x = (float)pBuf[x*3];
			WaterVel[i].y = (float)pBuf[x*3+1];
			WaterVel[i].z = (float)pBuf[x*3+2];

			WaterSpeed[i] = length(WaterVel[i]);

			if(WaterSpeed[i] < hmin)
				hmin = WaterSpeed[i];
			if(WaterSpeed[i] > hmax)
				hmax = WaterSpeed[i];

			i++;
		}
	}
	free(pBuf);

	fclose(pFile);
}

void AnalysisData::GetWaterSpeed(int nGetIdx, float*& WaterSpeed, float& hmin, float& hmax)
{
	FILE *pFile = fopen(m_sPathNameWsp, "rb");

	fseek(pFile, 0, SEEK_SET);

	for(int nIdx = 0; nIdx < nGetIdx; nIdx++)
	{
		fseek(pFile, sizeof(float) * m_nCountX * m_nCountY, SEEK_CUR);
	}

	WaterSpeed = (float*)malloc(sizeof(float) * m_nCountX * m_nCountY);
	float *pBuf = (float*) malloc(sizeof(float) * m_nCountX);
	size_t i = 0;
	hmin = 100000.0;
	hmax = -100000.0;

	for (int y = 0; y < m_nCountY; y++)
	{
		fread(pBuf, sizeof(float), m_nCountX, pFile);
		for (int x = 0; x < m_nCountX; x++)
		{
			WaterSpeed[i] = (float)pBuf[x];

			if(WaterSpeed[i] < hmin)
				hmin = WaterSpeed[i];
			if(WaterSpeed[i] > hmax)
				hmax = WaterSpeed[i];

			i++;
		}
	}
	free(pBuf);

	fclose(pFile);
}

void AnalysisData::GetTrnTitle(char* trnfileName, char* wspfileName, char* vetfilename, int& datawidth, int& dataheight, int& AnalBlockNum, float& StartX, float& StartY, float& CTC_X, float& CTC_Y)
{
	strcpy(m_sPathName, trnfileName);
	strcpy(m_sPathNameWsp, wspfileName);
	strcpy(m_sPathNameVet, vetfilename);

	FILE *pFile = fopen(m_sPathName, "rb");

	fread(&m_nMeshBlick, sizeof(int), 1, pFile);
	fread(&m_nCountX, sizeof(int), 1, pFile);
	fread(&m_nCountY, sizeof(int), 1, pFile);
	fread(&m_fCTC_X, sizeof(float), 1, pFile);
	fread(&m_fCTC_Y, sizeof(float), 1, pFile);
	fread(&m_fStartPointX, sizeof(float), 1, pFile);
	fread(&m_fStartPointY, sizeof(float), 1, pFile);

	datawidth = m_nCountX;
	dataheight = m_nCountY;
	AnalBlockNum = m_nMeshBlick;
	StartX = m_fStartPointX;
	StartY = m_fStartPointY;
	CTC_X = m_fCTC_X;
	CTC_Y = m_fCTC_Y;

	fclose(pFile);
/*
	FILE *defp = fopen("zdebugtrn.txt", "a");
	fprintf(defp, "GetTrnTitle함수:%s\nGetTrnTitle함수:%s\n",m_sPathName, m_sPathNameWsp);
	fprintf(defp, "해석 %d %d %d %f %f %f %f\n",m_nCountX, m_nCountY, m_nMeshBlick, m_fCTC_X, m_fCTC_Y, m_fStartPointX, m_fStartPointY);
	fclose(defp);
*/
}

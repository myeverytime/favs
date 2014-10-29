#include "stdafx.h"
#include "GLTrihedron.h"

CGLTrihedron::CGLTrihedron()
{
}

CGLTrihedron::CGLTrihedron(GLfloat size)
{
	csSize = size;
}

CGLTrihedron::~CGLTrihedron()
{
}

void CGLTrihedron::DefineDisplay()
{//Axis±×¸®±â
	glDisable(GL_LIGHTING);
	glDisable(GL_COLOR_MATERIAL);
	glEnable(GL_DEPTH_TEST);

	//Axes' label
	glColor3f(1.0f, 0.0f, 0.0f);
	glRasterPos3d(0.5*csSize, 0, 0);
	DrawText("X");
	glColor3f(0.0f, 1.0f, 0.0f);
	glRasterPos3d(0, 0.5*csSize, 0);
	DrawText("Y");
	glColor3f(0.0f, 0.0f, 1.0f);
	glRasterPos3d(0, 0, 0.5*csSize);
	DrawText("Z");

	//X Axis
	glLineWidth(5);
	glColor3f(1.0f, 0.0f, 0.0f);
	glBegin(GL_LINES);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.8*csSize*0.5, 0.0f, 0.0f);
	glEnd();
	glPushMatrix();	
	glTranslatef(0.8*csSize*0.5, 0.0f, 0.0f);
	glRotatef(90.0f,0.0f,1.0f,0.0f);
	glutWireCone(0.027*csSize,0.09*csSize,10,10);
	glPopMatrix();

	//Y Axis
	glColor3f(0.0f, 1.0f, 0.0f);
	glBegin(GL_LINES);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.8*csSize*0.5, 0.0f);
	glEnd();
	glPushMatrix();	
	glTranslatef(0.0f, 0.8*csSize*0.5, 0.0f);
	glRotatef(-90.0f,1.0f,0.0f,0.0f);
	glutWireCone(0.027*csSize,0.09*csSize,10,10);
	glPopMatrix();

	//Z Axis
	glColor3f(0.0f, 0.0f, 1.0f);
	glBegin(GL_LINES);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.8*csSize*0.5);
	glEnd();
	glPushMatrix();	
	glTranslatef(0.0f, 0.0f, 0.8*csSize*0.5);
	glutWireCone(0.027*csSize,0.09*csSize,10,10);
	glPopMatrix();

	glDisable(GL_DEPTH_TEST);
}

void CGLTrihedron::SetSize(GLfloat size)
{
	csSize = size;
}

void CGLTrihedron::DrawText(char* string)
{
    char* p = "";

    for (p = string; *p; p++)
	glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *p);
}

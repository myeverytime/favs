#ifndef _GLTRIHEDRON_H
#define _GLTRIHEDRON_H

#include <gl/glut.h>

class CGLTrihedron
{
public:
	CGLTrihedron();
	CGLTrihedron(GLfloat size);
	~CGLTrihedron();
	void DefineDisplay();
	void SetSize(GLfloat size); 
private:
	GLfloat csSize;
private:
	void DrawText(char* string);
};

#endif
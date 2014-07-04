#include "stdafx.h"
#include "Rotations.h"

Rotations::Rotations(void)
{
}


Rotations::~Rotations(void)
{
}

Rotations::Rotations(bool rotationGlobally){
	rotGlobally = rotationGlobally;
}

void Rotations::initRotations(){
	memset(gr, 0, sizeof(gr));
	gr[0]=gr[5]=gr[10]=gr[15]=1;
}

void Rotations::updateRotation(GLfloat angle, enum axes axis){
	GLfloat x;
	GLfloat y;
	GLfloat z;

	switch (axis)
	{
	case X:
		x = 1.0f;
		y = 0.0f;
		z = 0.0f;
		break;
	case Y:
		x = 0.0f;
		y = 1.0f;
		z = 0.0f;
		break;
	case Z:
		x = 0.0f;
		y = 0.0f;
		z = 1.0f;
		break;
	default:
		break;
	}
	glPushMatrix();
	glLoadIdentity();
	if(rotGlobally){
		glRotatef( angle, x,y,z );
		glMultMatrixf(gr);
	}else{
		glMultMatrixf(gr);
		glRotatef( angle, x,y,z );
	}

	glGetFloatv( GL_MODELVIEW_MATRIX, gr );
	glPopMatrix();
}

void Rotations::applyRotation(){
	glMultMatrixf(gr);
}
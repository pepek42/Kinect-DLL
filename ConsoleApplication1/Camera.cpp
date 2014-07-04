#include "stdafx.h"
#include "Camera.h"
#define _USE_MATH_DEFINES
#include <math.h>

Camera::Camera(void){

	center = new float[3];
	center[0] = 0;
	center[1] = 0;
	center[2] = 0;

	up = new float[3];
	up[0] = 0;
	up[1] = 0;
	up[2] = 1;

	eye = new float[3];

	latitude = 0;
	longitude = 0;
	radius = 2000;
}

Camera::~Camera(void){

	delete eye;
	delete center;
	delete up;
}

void Camera::getCamera(){

	eye[0] = radius*cos(latitude)*cos(longitude);
	eye[1] = radius*cos(latitude)*sin(longitude);
	eye[2] = radius*sin(latitude);
	gluLookAt(	eye[0], eye[1], eye[2],
		center[0], center[1],  center[2],
		up[0], up[1],  up[2]);

}

void Camera::changeLatitude(float val){
	latitude+=val;
	if(latitude < -2*M_PI){
		latitude += 2*M_PI;
	}
	if(latitude > 2*M_PI){
		latitude -= 2*M_PI;
	}
}

void Camera::changeLongitude(float val){
	longitude+=val;
	if(longitude < -2*M_PI){
		longitude += 2*M_PI;
	}
	if(longitude > -2*M_PI){
		longitude -= 2*M_PI;
	}
}

void Camera::changeRadius(float val){
	radius+=val;
}
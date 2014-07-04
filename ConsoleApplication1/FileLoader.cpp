#include "stdafx.h"
#include "FileLoader.h"
#include <tchar.h>


FileLoader::FileLoader(void){
	
	linesNumber1 = 261196;
	linesNumber2 = 242345;

	x1 = new GLfloat[linesNumber1];
	y1 = new GLfloat[linesNumber1];
	z1 = new GLfloat[linesNumber1];
	x2 = new GLfloat[linesNumber2];
	y2 = new GLfloat[linesNumber2];
	z2 = new GLfloat[linesNumber2];

	r1 = new GLubyte[linesNumber1];
	g1 = new GLubyte[linesNumber1];
	b1 = new GLubyte[linesNumber1];
	r2 = new GLubyte[linesNumber2];
	g2 = new GLubyte[linesNumber2];
	b2 = new GLubyte[linesNumber2];

	int i = 0;
	GLfloat x, y, z;
	int r, g, b;

	LPCTSTR pathAPP = L".\\ConsoleApplication1.ini";

	TCHAR   inBuf[100];

	GetPrivateProfileString(TEXT("FileLoader"),
		TEXT("fileOnePoints "),
		TEXT("1.txt"),
		inBuf,
		80,
		pathAPP);

	std::ifstream infile1(inBuf);

	GetPrivateProfileString(TEXT("FileLoader"),
		TEXT("fileTwoPoints "),
		TEXT("2.txt"),
		inBuf,
		80,
		pathAPP);

	std::ifstream infile2(inBuf);

	GetPrivateProfileString(TEXT("FileLoader"),
		TEXT("fileOneColors "),
		TEXT("1.col"),
		inBuf,
		80,
		pathAPP);

	std::ifstream infile3(inBuf);

	GetPrivateProfileString(TEXT("FileLoader"),
		TEXT("fileTwoColors "),
		TEXT("2.col"),
		inBuf,
		80,
		pathAPP);

	std::ifstream infile4(inBuf);
	/*michal ppk*
	if(infile1.is_open()){
		while (infile1 >> x >> y >> z){
			x1[i] = x;
			y1[i] = y;
			z1[i] =	z;
			i++;
		}
		if(i == linesNumber1){
			std::cout << "File 1 loaded successfully!" << std::endl;
		}else{
			std::cout << "File 1 not loaded!" << std::endl;
		}
	}else{
		std::cout << "Could not open file 1!" << std::endl;
	}
	i = 0;

	if(infile2.is_open()){
	while (infile2 >> x >> y >> z){
		x2[i] = x;
		y2[i] = y;
		z2[i] =	z;
		i++;
	}
	if(i == linesNumber2){
		std::cout << "File 2 loaded successfully!" << std::endl;
	}else{
		std::cout << "File 2 not loaded!" << std::endl;
	}
		}else{
		std::cout << "Could not open file 2!" << std::endl;
	}
	i = 0;

	if(infile3.is_open()){
	while (infile3 >> r >> g >> b){
		r1[i] = r;
		g1[i] = g;
		b1[i] =	b;
		i++;
	}
	if(i == linesNumber1){
		std::cout << "File 3 loaded successfully!" << std::endl;
	}else{
		std::cout << "File 3 not loaded!" << std::endl;
	}
		}else{
		std::cout << "Could not open file 3!" << std::endl;
	}
	i = 0;

		if(infile4.is_open()){
	while (infile4 >> r >> g >> b){
		r2[i] = r;
		g2[i] = g;
		b2[i] =	b;
		i++;
	}
	if(i == linesNumber2){
		std::cout << "File 4 loaded successfully!" << std::endl;
	}else{
		std::cout << "File 4 not loaded!" << std::endl;
	}
		}else{
		std::cout << "Could not open file 4!" << std::endl;
	}
	/**/
	infile1.close();
	infile2.close();
	infile3.close();
	infile4.close();
	
}


FileLoader::~FileLoader(void){
	delete[] x1;
	delete[] y1;
	delete[] z1;
	delete[] x2;
	delete[] y2;
	delete[] z2;

	delete[] r1;
	delete[] g1;
	delete[] b1;
	delete[] r2;
	delete[] g2;
	delete[] b2;
}

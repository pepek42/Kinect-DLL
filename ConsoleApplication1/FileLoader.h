#pragma once
class FileLoader
{
public:
	FileLoader(void);
	~FileLoader(void);
	GLfloat* x1;
	GLfloat* y1;
	GLfloat* z1;
	GLfloat* x2;
	GLfloat* y2;
	GLfloat* z2;

	GLubyte* r1;
	GLubyte* g1;
	GLubyte* b1;
	GLubyte* r2;
	GLubyte* g2;
	GLubyte* b2;

	int linesNumber1;
	int linesNumber2;

private:
	std::string name1;
	std::string name2;
};


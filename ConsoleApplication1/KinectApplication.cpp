// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "KinectDLL.h"
#include "stdafx.h"
#include "Rotations.h"
#include "Camera.h"
#include "FileLoader.h"
#include "DepthReader.h"
#include "SpeechControl.h"
#include "Synchronizer.h"
#include "HandCommands.h"

using namespace std;
using namespace KinectDLL;

Camera camera;
FileLoader* fileLoader;
Rotations* rotations;

DepthReader* depthReader;
SpeechControl* speechContol;
Synchronizer* synchronizer;

bool rotGlobally = true;

const int width = 640;
const int height = 480;

GLfloat divRatio = 1.0f;				//wspó³czynnik przez który dzielimy wartoœæ k¹tów rotacji i wartoœæ translacji dla trybu dok³adnego
GLfloat TurnSpeed = 5.0f;				//w stopniach
GLfloat translateSpeed = 30.0f;

GLfloat divRadioAccurateMode;

GLuint textureId;
GLubyte data[width*height * 4];

GLfloat transformVec[3];					//transformacja xyz dla obiekty	

int frameCount = 0;
int previousTime = 0;
float fps = 0.0f;
int delta = 0;
int previousTimeDelta = 0;

float moveObjectMultiplier;
float rotateObjectMultiplier;

float moveCameraMultiplier;
float rotateCameraMultiplier;

void gameInit();
void processNormalKeys(unsigned char key, int x, int y);
void processSpecialKeys(int key, int x, int y);
void handleResize(int w, int h);
void draw();
void idle();
void drawLines();

/*********Helper Render***********/

void calculateFPS();

void handleResize2(int w, int h);
void draw2();

/*********Helper Render***********/
DWORD WINAPI Thread_for_GLUT(LPVOID lpParam);
DWORD WINAPI Thread_for_Depth_Reader(LPVOID lpParam);
DWORD WINAPI Thread_for_Speech_Control(LPVOID lpParam);

int _tmain(int argc, _TCHAR* argv[]){



	gameInit();

	HANDLE Handle_Of_Thread_1 = 0;       // variable to hold handle of Thread 1
	HANDLE Handle_Of_Thread_2 = 0;       // variable to hold handle of Thread 1 
	HANDLE Handle_Of_Thread_3 = 0;       // variable to hold handle of Thread 1 
	HANDLE Array_Of_Thread_Handles[3];   // Aray to store thread handles 

	// Create thread 2.
	Handle_Of_Thread_2 = CreateThread(NULL, 0, Thread_for_Depth_Reader, synchronizer, 0, NULL);
	if (Handle_Of_Thread_2 == NULL)
		ExitProcess(2);

	while (!synchronizer->isKinectReady()){
		Sleep(1);
	}

	// Create thread 2.
	Handle_Of_Thread_3 = CreateThread(NULL, 0, Thread_for_Speech_Control, synchronizer, 0, NULL);
	if (Handle_Of_Thread_3 == NULL)
		ExitProcess(3);

	// Create thread 1.
	Handle_Of_Thread_1 = CreateThread(NULL, 0, Thread_for_GLUT, synchronizer, 0, NULL);
	if (Handle_Of_Thread_1 == NULL)
		ExitProcess(1);

	// Store Thread handles in Array of Thread Handles as per the requirement of WaitForMultipleObjects() 
	Array_Of_Thread_Handles[0] = Handle_Of_Thread_1;
	Array_Of_Thread_Handles[1] = Handle_Of_Thread_2;
	Array_Of_Thread_Handles[2] = Handle_Of_Thread_3;

	// Wait until all threads have terminated.
	WaitForMultipleObjects(3, Array_Of_Thread_Handles, TRUE, INFINITE);

	// Close all thread handles upon completion.
	CloseHandle(Handle_Of_Thread_2);
	CloseHandle(Handle_Of_Thread_1);
	CloseHandle(Handle_Of_Thread_3);

	delete synchronizer;
	delete depthReader;
	delete speechContol;
	delete rotations;
	delete fileLoader;

	return 0;
}

void gameInit(){

	srand((unsigned)time(0));

	fileLoader = new FileLoader();
	depthReader = new DepthReader();
	speechContol = new SpeechControl();
	synchronizer = new Synchronizer();
	rotations = new Rotations(rotGlobally);

	//synchronizer->setNumberOfObjects(2);

	rotations->initRotations();

	setlocale(LC_ALL, "polish");

	transformVec[0] = 0;
	transformVec[1] = 0;
	transformVec[2] = 0;

	//transformVec[0] = rand()%5;
	//transformVec[1] = rand()%5;
	//transformVec[2] = rand()%5;

	TCHAR   inBuf[100];

	LPCTSTR pathAPP = L".\\ConsoleApplication1.ini";;

	GetPrivateProfileString(TEXT("general"),
		TEXT("divRadioAccurateMode "),
		TEXT("10.0"),
		inBuf,
		80,
		pathAPP);
	divRadioAccurateMode = _tcstod(inBuf, NULL);

	GetPrivateProfileString(TEXT("general"),
		TEXT("moveObjectMultiplier "),
		TEXT("200.0"),
		inBuf,
		80,
		pathAPP);
	moveObjectMultiplier = _tcstod(inBuf, NULL);

	GetPrivateProfileString(TEXT("general"),
		TEXT("rotateObjectMultiplier "),
		TEXT("50.0"),
		inBuf,
		80,
		pathAPP);
	rotateObjectMultiplier = _tcstod(inBuf, NULL);

	GetPrivateProfileString(TEXT("general"),
		TEXT("moveCameraMultiplier "),
		TEXT("750.0f"),
		inBuf,
		80,
		pathAPP);
	moveCameraMultiplier = _tcstod(inBuf, NULL);

	GetPrivateProfileString(TEXT("general"),
		TEXT("rotateCameraMultiplier "),
		TEXT("5.0f"),
		inBuf,
		80,
		pathAPP);
	rotateCameraMultiplier = _tcstod(inBuf, NULL);


}

void processNormalKeys(unsigned char key, int x, int y) {

	switch (key){
	case 27:
		synchronizer->setState(States::QUIT);
		break;
	case 'q':
		transformVec[0] -= translateSpeed / divRatio;
		break;
	case 'e':
		transformVec[0] += translateSpeed / divRatio;
		break;
	case 'w':
		transformVec[2] += translateSpeed / divRatio;
		break;
	case 's':
		transformVec[2] -= translateSpeed / divRatio;
		break;
	case 'a':
		transformVec[1] -= translateSpeed / divRatio;
		break;
	case 'd':
		transformVec[1] += translateSpeed / divRatio;
		break;

	case 'u':
		rotations->updateRotation(-TurnSpeed / divRatio, axes::X);
		break;
	case 'o':

		rotations->updateRotation(TurnSpeed / divRatio, axes::X);
		break;
	case 'i':
		rotations->updateRotation(-TurnSpeed / divRatio, axes::Y);
		break;
	case 'k':
		rotations->updateRotation(TurnSpeed / divRatio, axes::Y);
		break;
	case 'j':
		rotations->updateRotation(-TurnSpeed / divRatio, axes::Z);
		break;
	case 'l':
		rotations->updateRotation(TurnSpeed / divRatio, axes::Z);
		break;

	case 'r':
		camera.changeRadius(-50.0f);
		break;
	case 'f':
		camera.changeRadius(50.0f);
		break;

	case 'h':
		cout << "Celem gry jest pokrycie obu wyœwietlanych przedmiotów." << endl;
		cout << "wsad+qe s³u¿¹ do przesuwania elementu we wszystkich 3 osiach prezentowanych na ekranie." << endl;
		cout << "i, k, j, l, u, o s³u¿¹ do rotacji obiektu." << endl;
		cout << "Strza³ki s³u¿¹ do obracania kamery wokó³ punktu centralnego, r i f do przybli¿ania i oddalania jej." << endl;
		cout << "Lewy Shift pozwala prze³¹czaæ siê miêdzy trybem ruchu zgrubnym a dok³adnym" << endl;
		cout << "Esc wychodzi z programu." << endl;
		cout << endl;
		break;
	}
}


void processSpecialKeys(int key, int x, int y) {

	switch (key) {
	case GLUT_KEY_SHIFT_L:
		if (divRatio == 1){
			synchronizer->setMoveAccuracy(MoveAccuracy::ACCURATE);
			divRatio = divRadioAccurateMode;
			cout << "Tryb dok³adny" << endl;
		}
		else{
			synchronizer->setMoveAccuracy(MoveAccuracy::NORMAL);
			divRatio = 1.0f;
			cout << "Tryb zgrubny" << endl;
		}
		break;
	case GLUT_KEY_DOWN:
		camera.changeLatitude(-0.15f);
		break;
	case GLUT_KEY_UP:
		camera.changeLatitude(0.15f);
		break;
	case GLUT_KEY_LEFT:
		camera.changeLongitude(-0.10f);
		break;
	case GLUT_KEY_RIGHT:
		camera.changeLongitude(0.10f);
		break;
	}
}

//Called when the window is resized
void handleResize(int w, int h) {
	glViewport(0, 0, w, h);

	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();
	gluPerspective(45.0,
		(double)w / (double)h,
		0.1,
		3000.0);
}

//Draws the 3D scene
void draw(){

	UINT objectIndex = synchronizer->getObject();

	bool bMove = (synchronizer->getState() == States::MOVE);

	float divideRatio = synchronizer->getMoveAccuracy() == MoveAccuracy::ACCURATE ? 8.0f : 1.0f;

	if (bMove && objectIndex == 1){
		HandCommands hc = synchronizer->getHandCommands();

			MyPointFloat move = hc.commands.move;
			MyPointFloat rotate = hc.commands.rotate;

			camera.changeRadius(move.z*moveCameraMultiplier / divideRatio);

			camera.changeLatitude(rotate.x*rotateCameraMultiplier / divideRatio);
			camera.changeLongitude(rotate.y*rotateCameraMultiplier / divideRatio);
	}

	GLfloat* x1L = fileLoader->x1;
	GLfloat* y1L = fileLoader->y1;
	GLfloat* z1L = fileLoader->z1;
	GLfloat* x2L = fileLoader->x2;
	GLfloat* y2L = fileLoader->y2;
	GLfloat* z2L = fileLoader->z2;

	GLubyte* r1L = fileLoader->r1;
	GLubyte* g1L = fileLoader->g1;
	GLubyte* b1L = fileLoader->b1;
	GLubyte* r2L = fileLoader->r1;
	GLubyte* g2L = fileLoader->g2;
	GLubyte* b2L = fileLoader->b2;

	//Clear screen
	//glClearColor( 0, 0, 0, 1 );
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	camera.getCamera();

	drawLines();

	//first
	/*michal ppk*/
	glutSolidCube(100);
	/**/
	glPointSize(1.0f);
	glBegin(GL_POINTS);
	for (int i = 0; i < fileLoader->linesNumber1; i++){
		glColor3ub(r1L[i], g1L[i], b1L[i]);
		glVertex3f(x1L[i], y1L[i], z1L[i]);
	}
	glEnd();

	if (bMove && objectIndex == 0){

		HandCommands hc = synchronizer->getHandCommands();

		MyPointFloat move = hc.commands.move;
		MyPointFloat rotate = hc.commands.rotate;

		transformVec[0] += move.x*moveObjectMultiplier / divideRatio;
		transformVec[1] += move.y*moveObjectMultiplier / divideRatio;
		transformVec[2] += move.z*moveObjectMultiplier / divideRatio;

		glTranslatef(transformVec[0], transformVec[1], transformVec[2]);

		rotations->updateRotation(rotate.x*rotateObjectMultiplier / divideRatio, axes::X);
		rotations->updateRotation(rotate.y*rotateObjectMultiplier / divideRatio, axes::Y);
		rotations->updateRotation(rotate.z*rotateObjectMultiplier / divideRatio, axes::Z);


	}else{
		glTranslatef(transformVec[0], transformVec[1], transformVec[2]);
	}

	//ROTACJE ELEMENTU
	rotations->applyRotation();

	drawLines();

	//second
	/*michal ppk*/
	glutSolidCube(100);
	/**/
	glBegin(GL_POINTS);
	for (int i = 0; i < fileLoader->linesNumber2; i++){
		glColor3ub(r2L[i], g2L[i], b2L[i]);
		glVertex3f(x2L[i], y2L[i], z2L[i]);
	}
	glEnd();
	glutSwapBuffers();
}

void drawLines(){
	glBegin(GL_LINES);

	glColor3f(1, 0, 0);
	glVertex3f(0, 0, 0);
	glVertex3f(100.0f, 0, 0);

	glColor3f(0, 1, 0);
	glVertex3f(0, 0, 0);
	glVertex3f(0, 100.0f, 0);

	glColor3f(0, 0, 1);
	glVertex3f(0, 0, 0);
	glVertex3f(0, 0, 100.0f);

	glEnd();
}

DWORD WINAPI Thread_for_GLUT(LPVOID lpParam)
{
	//freeGLUT init
	char** argvchar = NULL;
	int argc = 0;
	glutInit(&argc, argvchar);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(400, 400);
	glutCreateWindow("Program testowy");
	glEnable(GL_DEPTH_TEST);;
	glutDisplayFunc(draw);
	glutReshapeFunc(handleResize);
	glutIdleFunc(idle);
	glutKeyboardFunc(processNormalKeys);
	glutSpecialFunc(processSpecialKeys);

	/**/
	glutInitWindowSize(640, 480);
	glutCreateWindow("DepthReader helper window");
	glutDisplayFunc(draw2);
	glutReshapeFunc(handleResize2);
	glutIdleFunc(idle);

	// Initialize textures
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, (GLvoid*)data);
	glBindTexture(GL_TEXTURE_2D, 0);

	// OpenGL setup
	glClearColor(0, 0, 0, 0);
	glClearDepth(1.0f);
	glEnable(GL_TEXTURE_2D);

	// Camera setup
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, width, height, 0, 1, -1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	/**/

	glutMainLoop();
	return 0;
}

DWORD WINAPI Thread_for_Depth_Reader(LPVOID lpParam)
{
	Synchronizer* dR_sync = static_cast<Synchronizer*>(lpParam);
	depthReader->Run(dR_sync);

	return 0;
}

DWORD WINAPI Thread_for_Speech_Control(LPVOID lpParam)
{
	Synchronizer* sC_sync = static_cast<Synchronizer*>(lpParam);
	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	int returnVal;
	if (SUCCEEDED(hr))
	{
		{
			returnVal = speechContol->Run(sC_sync);
		}

		CoUninitialize();
	}
	return 0;
}

void draw2(){
	synchronizer->getTexture(data);

	glClearColor(0, 0, 0, 0);
	glClearDepth(1.0f);
	glEnable(GL_TEXTURE_2D);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_BGRA_EXT, GL_UNSIGNED_BYTE, (GLvoid*)data);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(0, 0, 0);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(width, 0, 0);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(width, height, 0.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(0, height, 0.0f);
	glEnd();

	glutSwapBuffers();
}

void handleResize2(int w, int h) {

	glutReshapeWindow(width, height);

	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, width, height, 0, 1, -1);

}

void idle(){
	if (synchronizer->getState() != QUIT){
		switch (glutGetWindow()){
		case 1:
			draw();
			break;
		case 2:
			draw2();
			break;
		}
	}
	else{
		glutLeaveMainLoop();
	}
	calculateFPS();
}

void calculateFPS()
{
	//  Increase frame count
	frameCount++;

	//  Get the number of milliseconds since glutInit called
	//  (or first call to glutGet(GLUT ELAPSED TIME)).
	int currentTime = glutGet(GLUT_ELAPSED_TIME);

	//  Calculate time passed
	int timeInterval = currentTime - previousTime;

	delta = glutGet(GLUT_ELAPSED_TIME) - previousTimeDelta;
	previousTimeDelta = glutGet(GLUT_ELAPSED_TIME);

	if (timeInterval > 1000)
	{
		//  calculate the number of frames per second
		fps = frameCount / (timeInterval / 1000.0f);

		//  Reset frame count
		frameCount = 0;

		cout << "FPS GLUT: " << fps << ", delta GLUT: " << delta << endl;

		//  Set time
		previousTime = currentTime;
	}
}
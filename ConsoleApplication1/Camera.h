#pragma once
class Camera
{
public:
	Camera(void);
	~Camera(void);
	void getCamera();
	void changeRadius(float val);
	void changeLatitude(float val);
	void changeLongitude(float val);
private:
	float* eye;							//pozycja oka 
	float* center;						//na co oko patrzy
	float* up;							//wektor w gorê dla kamery
	float latitude;							//szerokoœæ geograficzna dla kamery
	float longitude;						//d³ugoœæ geograficzna dla kamery
	float radius;
};


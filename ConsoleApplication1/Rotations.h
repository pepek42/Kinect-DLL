#pragma once
class Rotations
{
public:
	Rotations(void);
	~Rotations(void);
	Rotations(bool rotGlobally);
	void initRotations();
	void updateRotation(float angle, enum axes axis);
	void applyRotation();

private:
	bool rotGlobally;
	float gr[16];

};


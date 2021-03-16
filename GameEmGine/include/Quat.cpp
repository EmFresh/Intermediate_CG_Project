#include "Quat.h"

using namespace std;

Quat::Quat()
{}

Quat::Quat(float x, float y, float z):w(0), x(x), y(y), z(z)
{}

Quat::Quat(float w, float x, float y, float z) : w(w), x(x), y(y), z(z)
{}

Quat::Quat(Vec3 rot) : w(0), x(rot.x), y(rot.y), z(rot.z)
{}

Quat Quat::normal()
{
	Quat q = *this;
	float unit = sqrt(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
	q.x *= unit, q.y *= unit, q.z *= unit, q.w *= unit;
	return q;
}

void Quat::normalize()
{
	*this = normal();
}

Quat& Quat::rotation(float a_ang, float a_dirX, float a_dirY, float a_dirZ)
{
	a_ang = glm::radians(a_ang);
	float
		unit(sqrt(a_dirX * a_dirX + a_dirY * a_dirY + a_dirZ * a_dirZ));
	/*rotX = cos(a_ang / 2),
	rotY = sin(a_ang / 2)*  (a_dirX / unit),
	rotZ = sin(a_ang / 2)*  (a_dirY / unit),
	rotW = sin(a_ang / 2)*  (a_dirZ / unit);*/

	Quat
		q{cos(a_ang / 2),sin(a_ang / 2) * (a_dirX / unit),sin(a_ang / 2) * (a_dirY / unit),sin(a_ang / 2) * (a_dirZ / unit)},//rotation quaternion
		qc{cos(a_ang / 2),sin(a_ang / 2) * -(a_dirX / unit),sin(a_ang / 2) * -(a_dirY / unit),sin(a_ang / 2) * -(a_dirZ / unit)},//rotation quatenion conjugate
		p{0, x,y,z};//pure quaternion
	static Quat rot; rot = q * p * qc;
	return rot;
}

Quat& Quat::rotation(float a_ang, Vec3 a_dir)
{
	return rotation(a_ang, a_dir.x, a_dir.y, a_dir.z);
}

Quat& Quat::rotation(Quat p, Quat q, Quat qc)
{
	static Quat rot; rot = q * p * qc;
	return rot;
}

void Quat::rotate(float a_ang, float a_dirX, float a_dirY, float a_dirZ)
{
	*this = rotation(a_ang, a_dirX, a_dirY, a_dirZ);
}

void Quat::rotate(float a_ang, Vec3 a_dir)
{
	rotate(a_ang, a_dir.x, a_dir.y, a_dir.z);
}



glm::mat4 Quat::quatRotationMat(float a_ang, float a_dirX, float a_dirY, float a_dirZ)
{
	if(!a_ang)
		return glm::mat4(1);

	a_ang = glm::radians(a_ang);
	float
		unit(sqrt(a_dirX * a_dirX + a_dirY * a_dirY + a_dirZ * a_dirZ));

	//rotation quaternion
	Quat
		q{cos(a_ang / 2),
		sin(a_ang / 2) * (a_dirX / unit),
		sin(a_ang / 2) * (a_dirY / unit),
		sin(a_ang / 2) * (a_dirZ / unit)};

	//pre determined matrix multiplication
	return
		glm::mat4(q.w, q.z, -q.y, q.x,
			-q.z, q.w, q.x, q.y,
			q.y, -q.x, q.w, q.z,
			-q.x, -q.y, -q.z, q.w)
		*
		glm::mat4(q.w, q.z, -q.y, -q.x,
			-q.z, q.w, q.x, -q.y,
			q.y, -q.x, q.w, -q.z,
			q.x, q.y, q.z, q.w);
}

glm::mat4 Quat::quatRotationMat(float a_ang, const Vec3 a_dir)
{
	return quatRotationMat(a_ang, a_dir.x, a_dir.y, a_dir.z);
}

glm::mat4 Quat::quatRotationMat(float a_ang, const  glm::vec3 a_dir)
{
	return quatRotationMat(a_ang, a_dir.x, a_dir.y, a_dir.z);
}

void Quat::print() const
{
	printf("Quat: (");
	for(int a = 0; a < 4; a++)
		printf(std::string("%f" + std::string(a != 3 ? ", " : "")).c_str(), this[0][a]);
	puts(")\n");
}

float& Quat::operator[](int m_index) const
{
	float* e = nullptr;
	switch(m_index)
	{
	case 0:
		return const_cast<float&>(x);
	case 1:
		return const_cast<float&>(y);
	case 2:
		return const_cast<float&>(z);
	case 3:
		return const_cast<float&>(w);
	}
	return *e;
}

Quat Quat::operator*(Quat a_quat) const
{
	Quat quat, q2 = a_quat;

	quat.x = x * q2.x - y * q2.y - z * q2.z - w * q2.w;
	quat.y = x * q2.y + y * q2.x - z * q2.w + w * q2.z;
	quat.z = x * q2.z + y * q2.w + z * q2.x - w * q2.y;
	quat.w = x * q2.w - y * q2.z + z * q2.y + w * q2.x;

	return quat;
}
Quat Quat::operator+(Quat a_quat) const
{
	Quat q1;
	for(int a = 0; a < 4; a++)
		q1[a] = this[0][a] + a_quat[a];
	return q1;
}
void Quat::operator+=(Quat a_quat) const
{
	for(int a = 0; a < 4; a++)
		this[0][a] += a_quat[a];

}

Quat Quat::operator-(Quat a_quat) const
{
	Quat q1;
	for(int a = 0; a < 4; a++)
		q1[a] = this[0][a] - a_quat[a];
	return q1;
}

Quat Quat::operator-() const
{
	return Quat(-w, -x, -y, -z);
}

void Quat::operator-=(Quat a_quat) const
{
	for(int a = 0; a < 4; a++)
		this[0][a] -= a_quat[a];
}

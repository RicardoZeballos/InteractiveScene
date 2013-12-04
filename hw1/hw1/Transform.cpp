// Transform.cpp: implementation of the Transform class.

#include "stdafx.h"
#include "Transform.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
using namespace std ; 


//Takes as input the current eye position, and the current up vector.
//up is always normalized to a length of 1.
//eye has a length indicating the distance from the viewer to the origin

// Helper rotation function.  Please implement this.  

mat3 Transform::rotate(const float degrees, const vec3& axis) {
  mat3 R ;
  	float convRadians = degrees*(pi)/180 ;
	float a = cos(convRadians) ;
	float b = sin(convRadians) ;
	vec3 Zp = axis;
	mat3 H (0, -Zp[2], Zp[1], Zp[2], 0, -Zp[0], -Zp[1] ,Zp[0] ,0);
	mat3 identity (1,0,0,0,1,0,0,0,1);
	mat3 dotResult (Zp[0]*Zp[0], Zp[0]*Zp[1], Zp[0]*Zp[2], Zp[0]*Zp[1], Zp[1]*Zp[1], Zp[1]*Zp[2], Zp[0]*Zp[2], Zp[1]*Zp[2], Zp[2]*Zp[2]);
	R = a*identity + ((1-a)*dotResult) + b*H;
  // FILL IN YOUR CODE HERE
  return R ; 
}

void Transform::left(float degrees, vec3& ourLookAt, vec3& up, vec3& eye, vec3& strafeVec) {
	float convRadians = degrees*(pi)/180 ;
	float a = cos(convRadians) ;
	float b = sin(convRadians) ;
	up[0]=up[0]-eye[0];
	up[1]=up[1]-eye[1];
	up[2]=up[2]-eye[2];
	ourLookAt[0] = ourLookAt[0] - eye[0];
	ourLookAt[1] = ourLookAt[1] - eye[1];
	ourLookAt[2] = ourLookAt[2] - eye[2];
	strafeVec[0] = strafeVec[0] - eye[0];
	strafeVec[1] = strafeVec[1] - eye[1];
	strafeVec[2] = strafeVec[2] - eye[2];
	vec3 Tup(0,2,0);
	mat3 RotationMatrix = Transform::rotate(degrees,up);
	vec3 RotatedEye = ourLookAt * RotationMatrix;
	ourLookAt = RotatedEye;
	strafeVec = strafeVec * RotationMatrix;
	//up = up * RotationMatrix;
	strafeVec[0] = strafeVec[0] + eye[0];
	strafeVec[1] = strafeVec[1] + eye[1];
	strafeVec[2] = strafeVec[2] + eye[2];
	up[0]=up[0] + eye[0];
	up[1]=up[1] + eye[1];
	up[2]=up[2] + eye[2];
	ourLookAt[0] = ourLookAt[0] + eye[0];
	ourLookAt[1] = ourLookAt[1] + eye[1];
	ourLookAt[2] = ourLookAt[2] + eye[2];
	//FILL IN YOUR CODE HERE
}

void Transform::up(float degrees, vec3& ourLookAt, vec3& up, vec3& eye) {
	float convRadians = degrees*(pi)/180 ;
	float a = cos(convRadians) ;
	float b = sin(convRadians) ;
	up[0]=up[0]-eye[0];
	up[1]=up[1]-eye[1];
	up[2]=up[2]-eye[2];
	ourLookAt[0] = ourLookAt[0] - eye[0];
	ourLookAt[1] = ourLookAt[1] - eye[1];
	ourLookAt[2] = ourLookAt[2] - eye[2];
	vec3 center(0,0,0);
	vec3 Neye = -(glm::normalize(ourLookAt-center));
	vec3 ZAxis = glm::cross(Neye,up) ;
	mat3 RotationMatrix = Transform::rotate(degrees,-ZAxis) ;
	ourLookAt = ourLookAt * RotationMatrix;
	up = up *RotationMatrix;
	ourLookAt[0] = ourLookAt[0] + eye[0];
	ourLookAt[1] = ourLookAt[1] + eye[1];
	ourLookAt[2] = ourLookAt[2] + eye[2];
	up[0]=up[0] + eye[0];
	up[1]=up[1] + eye[1];
	up[2]=up[2] + eye[2];
	//Rotate eye vector the opposite direction that you did the other vector
	//FILL IN YOUR CODE HERE
}

mat4 Transform::lookAt(const vec3& eye, const vec3 &ourLookAt, const vec3& up){
//mat4 lookAt(vec3& eye, vec3& center, vec3& up) {
    mat4 M ; 
	vec3 W = glm::normalize(ourLookAt - eye);
	vec3 U = glm::normalize(glm::cross(up,W));
	vec3 V = glm::normalize(glm::cross(W,U));
	mat4 RMatrix (U[0], U[1], U[2], 0, V[0], V[1], V[2], 0, W[0], W[1], W[2], 0, 0, 0, 0, 1);
	mat4 TMatrix (1, 0, 0, -eye[0], 0, 1, 0, -eye[1], 0, 0, 1, -eye[2], 0, 0, 0, 1);
	M = TMatrix * RMatrix;
	//FILL IN YOUR CODE HERE
    //You must return a row-major mat4 M that you create from this routine
	return M ; 
}

mat4 Transform::perspective(float fovy, float aspect, float zNear, float zFar){
	float radiansFovy = fovy*(pi)/180 ;
	//d=cotangent of fovy/2
	float d = 1/(tan(radiansFovy/2));
	float A = (zFar + zNear)/(zFar - zNear) ;
	//float A = (zFar + zNear)/(zNear-zFar) ;
	float B = (2 * zFar * zNear)/(zFar-zNear);
	//float B = (2 * zFar * zNear)/(zNear-zFar);
	mat4 M (d/aspect, 0, 0, 0, 
		0, d, 0, 0, 
		0, 0, -A, -B, 
		0, 0,-1,0);
	return M; 
}

mat4 Transform::scale(const float &sx, const float &sy, const float &sz){
	//mat4 M;
	mat4 M (sx, 0, 0, 0, 
		0, sy, 0, 0, 
		0, 0, sz, 0, 
		0, 0, 0, 1);
	return M; 
}

mat4 Transform::translate(const float &tx, const float &ty, const float &tz){
	//mat4 M;
	mat4 M (1, 0, 0, tx, 
	0, 1, 0, ty, 
	0, 0, 1, tz, 
	0, 0, 0, 1);
	return M; 
}

Transform::Transform()
{

}

Transform::~Transform()
{

}

// Some notes about using glm functions.
// You are ONLY permitted to use glm::dot glm::cross glm::normalize
// Do not use more advanced glm functions (in particular, directly using 
// glm::lookAt is of course prohibited).  

// You may use overloaded operators for matrix-vector multiplication 
// But BEWARE confusion between opengl (column major) and row major 
// conventions, as well as what glm implements. 
// In particular, vecnew = matrix * vecold may not implement what you think 
// it does.  It treats matrix as column-major, in essence using the transpose.
// We recommend using row-major and vecnew = vecold * matrix 
// Preferrably avoid matrix-matrix multiplication altogether for this hw.  

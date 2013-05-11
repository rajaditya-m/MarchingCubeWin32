#ifndef _POINTMANIP_H_
#define _POINTMANIP_H_

#define EPSA 0.000001


#include "Oracle.h"

namespace marching {

bool equalityInLimits(GLfloat p1, GLfloat p2)
{
	double diff = (double)fabs(p1-p2);
	if(diff<EPSA)
		return true;
	else
		return false;
}

bool equalityPoints(Angel::vec4 p1,Angel::vec4 p2)
{
	bool res = equalityInLimits(p1.x,p2.x) & equalityInLimits(p1.y,p2.y) & equalityInLimits(p1.z,p2.z);
	return res;
}


bool inValidVector(Angel::vec3 v1)
{
	if((v1.x!=v1.x) || (v1.y != v1.y) || (v1.z !=v1.z))
		return true;
	else
		return false;

}


}


#endif
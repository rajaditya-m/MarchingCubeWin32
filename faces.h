#ifndef _FACES_H_
#define _FACES_H_

namespace marching {

class RawFaces 
{
private :
	unsigned long index1_;
	unsigned long index2_;
	unsigned long center_;


public:
	unsigned long getI1()		{ return index1_;	}
	unsigned long getI2()		{ return index2_;	}
	unsigned long getCenter()	{ return center_;	}

	RawFaces(unsigned long center, unsigned long ai1,unsigned long ai2)
	{
		index1_ = ai1;
		index2_ = ai2;
		center_ = center;
	}

	RawFaces()
	{
		index1_ = index2_ = center_ = 0;
	}

};

}

#endif


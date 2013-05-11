#ifndef _MESHREP_H_
#define _MESHREP_H_

#include "rawedge.h"
#include "faces.h"

#define MAXPOINT 200
#define MAXEDGES 300000
#define MAXUVERT 40000
#define MAXTRIPERVERT 100

namespace marching {

class MeshRep 
{
private: 
	unsigned long curIndex_;
	unsigned long edgeIndex_;
	unsigned long verIDX[MAXPOINT][MAXPOINT][MAXPOINT];
	unsigned long verIDY[MAXPOINT][MAXPOINT][MAXPOINT];
	unsigned long verIDZ[MAXPOINT][MAXPOINT][MAXPOINT];
	unsigned long verIDP[MAXPOINT][MAXPOINT][MAXPOINT];
	RawEdge edgeList_[MAXEDGES]; 
	RawFaces faceList[MAXUVERT][MAXTRIPERVERT];
	int curTriIndex[MAXUVERT];

public:
	MeshRep()
	{
		curIndex_ = 1;
		edgeIndex_ = 0;
		memset(verIDX, 0, sizeof(verIDX[0][0][0])*MAXPOINT*MAXPOINT);
		memset(verIDY, 0, sizeof(verIDY[0][0][0])*MAXPOINT*MAXPOINT);
		memset(verIDZ, 0, sizeof(verIDZ[0][0][0])*MAXPOINT*MAXPOINT);
		memset(verIDP, 0, sizeof(verIDP[0][0][0])*MAXPOINT*MAXPOINT);
		memset(curTriIndex,0,sizeof(curTriIndex[0]*MAXUVERT));
	}

	void addAnEdge(unsigned long i1, unsigned long i2)
	{
		RawEdge* re = new RawEdge(i1,i2);
		edgeList_[edgeIndex_++]= (*re);
	}

	void addAFace(unsigned long i1, unsigned long i2,unsigned long i3)
	{
		RawFaces* rf1 = new RawFaces(i1,i2,i3);
		RawFaces* rf2 = new RawFaces(i2,i1,i3);
		RawFaces* rf3 = new RawFaces(i3,i1,i2);
		
		int ti1 = curTriIndex[i1];
		int ti2 = curTriIndex[i2];
		int ti3 = curTriIndex[i3];

		faceList[i1][ti1] = (*rf1);
		faceList[i2][ti2] = (*rf2);
		faceList[i3][ti3] = (*rf3);

		curTriIndex[i1] = (ti1+1);
		curTriIndex[i2] = (ti2+1);
		curTriIndex[i3] = (ti3+1);

	}

	unsigned long addOrRetrieve(Angel::vec4 p)
	{

		//First cast everything to integer 
		int gcX = (int)p.x;
		int gcY = (int)p.y;
		int gcZ = (int)p.z;

		short fracPart = 0;
		if(p.x != gcX)
			fracPart = 1;
		else if (p.y != gcY)
			fracPart = 2;
		else if (p.z != gcZ)
			fracPart = 3;

		unsigned long val ;

		switch(fracPart)
		{
		case 0:
			val = verIDP[gcX][gcY][gcZ];
			if(val == 0)
			{
				verIDP[gcX][gcY][gcZ] = curIndex_++;
				val = curIndex_-1;
			}
			break;
		case 1:
			val = verIDX[gcX][gcY][gcZ];
			if(val == 0)
			{
				verIDX[gcX][gcY][gcZ] = curIndex_++;
				val = curIndex_-1;
			}
			break;
		case 2:
			val = verIDY[gcX][gcY][gcZ];
			if(val == 0)
			{
				verIDY[gcX][gcY][gcZ] = curIndex_++;
				val = curIndex_-1;
			}
			break;
		case 3:
			val = verIDZ[gcX][gcY][gcZ];
			if(val == 0)
			{
				verIDZ[gcX][gcY][gcZ] = curIndex_++;
				val = curIndex_-1;
			}
			break;
		}
		return val;
	}

	unsigned long nosVertices()	
	{
		return curIndex_-1;
	}

	unsigned long nosRawEdges() 
	{
		return edgeIndex_;
	}

	double nosAvgTrianglesPerVertex()
	{
		double sum = 0;
		for(int i=1;i<curIndex_;i++)
		{
			sum += curTriIndex[i];
		}
		return sum/(curIndex_-1);
	}

	RawEdge* getRawEdgeArray()
	{
		return edgeList_;
	}

	bool checkEdgeCorrectness()
	{
		RawEdge cur,left,right;
		bool lR,rR,r;
		for(int i =0;i<edgeIndex_;i++)
		{
			cur = edgeList_[i];
			lR = rR = r = false;
			if(i!=0)
			{
				left = edgeList_[i-1];
				lR = cur.equals(left);
			}
			if(i!=(edgeIndex_-1))
			{
				right = edgeList_[i+1];
				rR = cur.equals(right);
			}
			r = lR & rR;
			if(r)
				return false;
		}
		return true;
	}
	
	bool checkFanProperty()
	{
		//Check per vertex
		bool explored[MAXTRIPERVERT];
		bool OK;
		unsigned long leftArm,rightArm;
		for(int i = 1;i<curIndex_;i++)
		{
			OK = false;
			int nosTri = curTriIndex[i];
			//Reset explored array to all false
			for(int j = 0;j < nosTri ;j++)
				explored[j] = false;
			//Get starting triangle list  
			leftArm = (faceList[i][0]).getI1();
			rightArm = (faceList[i][0]).getI2();
			explored[0]=true;
			//Start the left arm walk 
			for(int k = 0;k<nosTri;k++)
			{
				//We have encountered a closed fan
				if(leftArm == rightArm)
				{
					OK = true;
					break;
				}
				for(int l=0;l<nosTri;l++)
				{
					if(!(explored[l]))
					{
						if(leftArm == faceList[i][l].getI1()) 
						{
							leftArm = faceList[i][l].getI2();
							explored[l] = true;
							break;
						} else if (leftArm == faceList[i][l].getI2())
						{
							leftArm = faceList[i][l].getI1();
							explored[l] = true;
						}
					}								
				}
			}
			//This finishes the left walk
			if(OK)
				continue;
			//Start the right walk 
			for(int k = 0;k<nosTri;k++)
			{
				for(int l=0;l<nosTri;l++)
				{
					if(!(explored[l]))
					{
						if(rightArm == faceList[i][l].getI1()) 
						{
							rightArm = faceList[i][l].getI2();
							explored[l] = true;
							break;
						} else if (rightArm == faceList[i][l].getI2())
						{
							rightArm = faceList[i][l].getI1();
							explored[l] = true;
						}
					}								
				}
			}
			//All walks are finished By now all edges should have been manifold
			for(int k = 0;k<nosTri;k++)
			{
				if(!(explored[k]))
					return false;
			}
		}
		return true;
	}
	
	void DBG__printRawEdgeArray()
	{
		for (unsigned long i =0;i<edgeIndex_;i++)
			edgeList_[i].print();
	}

};

}

#endif
#ifndef _WEDS_H_
#define _WEDS_H_

#include "rawedge.h"
#include "vertex.h"
#include "pointmanip.h"
#include <vector>
#include <set>

namespace marching {

	using namespace Angel;
	using namespace std;

class WingedEdgeDS 
{
private:
	unsigned long numTriangles_;
	std::set<RawEdge> rawEdgeList;
	std::vector<Vertex*> vertexList;

public:

	WingedEdgeDS()
	{
		numTriangles_ = 0;
	}

	unsigned long getNumTriangles()
	{
		return numTriangles_;
	}

	unsigned long getNumRawEdges()
	{
		return rawEdgeList.size();
	}

	unsigned long getNumVertex()
	{
		return vertexList.size();
	}

	bool addATriangle(Angel::vec4 a, Angel::vec4 b, Angel::vec4 c)
	{
		int index1 = searchInVertexList(a);
		int index2 = searchInVertexList(b);
		int index3 = searchInVertexList(c);
		//Added the index(if not already present)
		if(index1==-1)
		{
			Vertex* aV = new Vertex(a);
			aV->setIndex(vertexList.size());
			vertexList.push_back(aV);
			index1 = aV->getIndex();
		}
		if(index2==-1)
		{
			Vertex* bV = new Vertex(b);
			bV->setIndex(vertexList.size());
			vertexList.push_back(bV);
			index2 = bV->getIndex();
		}
		if(index3==-1)
		{
			Vertex* cV = new Vertex(c);
			cV->setIndex(vertexList.size());
			vertexList.push_back(cV);
			index3 = cV->getIndex();
		}
		//Now create the raw edge index
		//(1,2)(2,3)(1,3)
		RawEdge* re1 = new RawEdge(index1,index2);
		RawEdge* re2 = new RawEdge(index2,index3);
		RawEdge* re3 = new RawEdge(index1,index3);
		//Insert the raw edge index in the sorted list
		std::pair<std::set<RawEdge>::iterator,bool> retVal;
		std::set<RawEdge>::iterator it ;
		retVal = rawEdgeList.insert(*re1);
		//Check if element already occurs. If so anhiliate the number and increment the count 
		if(retVal.second == false)
		{
			it = retVal.first;
			int cc = const_cast<RawEdge&>(*it).getCount();
			int idx1_ = const_cast<RawEdge&>(*it).getIndex1();
			int idx2_ = const_cast<RawEdge&>(*it).getIndex2();
			if(cc >= 2)
			{
				std::cout << "[ERROR] Mesh is non-manifold.\n";
				return false;
			}
			rawEdgeList.erase(it);
			RawEdge* nE= new RawEdge(idx1_,idx2_,cc+1);
			rawEdgeList.insert(*nE);
		}
		retVal = rawEdgeList.insert(*re2);
		//Check if element already occurs. If so anhiliate the number and increment the count 
		if(retVal.second == false)
		{
			it = retVal.first;
			int cc = const_cast<RawEdge&>(*it).getCount();
			int idx1_ = const_cast<RawEdge&>(*it).getIndex1();
			int idx2_ = const_cast<RawEdge&>(*it).getIndex2();
			if(cc >= 2)
			{
				std::cout << "[ERROR] Mesh is non-manifold.\n";
				return false;
			}
			rawEdgeList.erase(it);
			RawEdge* nE= new RawEdge(idx1_,idx2_,cc+1);
			rawEdgeList.insert(*nE);
		}
		retVal = rawEdgeList.insert(*re3);
		//Check if element already occurs. If so anhiliate the number and increment the count 
		if(retVal.second == false)
		{
			it = retVal.first;
			int cc = const_cast<RawEdge&>(*it).getCount();
			int idx1_ = const_cast<RawEdge&>(*it).getIndex1();
			int idx2_ = const_cast<RawEdge&>(*it).getIndex2();
			if(cc >= 2)
			{
				std::cout << "[ERROR] Mesh is non-manifold.\n";
				return false;
			}
			rawEdgeList.erase(it);
			RawEdge* nE= new RawEdge(idx1_,idx2_,cc+1);
			rawEdgeList.insert(*nE);
		}
		numTriangles_++;
		return true;

	}

	int searchInVertexList(Angel::vec4 p)
	{
		std::vector<Vertex*>::iterator it;
		int count = 0;
		for(it=vertexList.begin();it!=vertexList.end();it++)
		{
			if(equalityPoints((*it)->getPointData(),p))
			{
				return (*it)->getIndex();
			}
		}
		return -1;
	}
	
};


}
#endif
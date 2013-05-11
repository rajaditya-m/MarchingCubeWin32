#ifndef _RAWEDGE_H_
#define _RAWEDGE_H_

#define SWAPE(x,y) unsigned long t;t=x;x=y;y=t;



namespace marching {

class RawEdge
{
public:
	unsigned long getIndex1()			{return index1_;	}
	unsigned long getIndex2()			{return index2_;	}
	void setIndex1(unsigned long i1)	{index1_ = i1;		}
	void setIndex2(unsigned long i2)	{index2_ = i2;		}
	
	RawEdge(unsigned long i1,unsigned long i2)
	{
		if(i1>i2)
		{
			SWAPE(i1,i2);
		}
		index1_ = i1;
		index2_ = i2;
	}

	RawEdge()
	{
		index1_ = 0;
		index2_ = 0;
	}

	bool operator< (const RawEdge& rhs) const
	{
		if(index1_ < rhs.index1_)
			return true;
		else if((index1_ == rhs.index1_) && (index2_ < rhs.index2_))
			return true;
		else
			return false;
	}

	bool operator> (const RawEdge& rhs) const
	{
		if(index1_ > rhs.index1_)
			return true;
		else if((index1_ == rhs.index1_) && (index2_ > rhs.index2_))
			return true;
		else
			return false;
	}

	void print()
	{
		std::cout << "[" << index1_ << "," << index2_ << "]\n";
	}

	bool equals(RawEdge &e)
	{
		if((index1_ == e.getIndex1() ) && (index2_ == e.getIndex2() ))
			return true;
		return false;
	}

private:
	unsigned long index1_;
	unsigned long index2_;
};

}


#endif

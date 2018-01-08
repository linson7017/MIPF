#ifndef FAST_MARCHING_H
#define FAST_MARCHING_H

#include <math.h>
#include <queue>
#include <set>
#include <vector>
#include <stdlib.h>
#include <fstream>
#include <iterator>

#include "utilities.h"

#include "fibheap.h"

#include <Segmentation/IQF_FastGrowCutSegmentation.h>

namespace FGC {

/***************************************************************************
* class HeapNode
***************************************************************************/
class HeapNode : public FibHeapNode {
    float   N;
    long IndexV;

public:
    HeapNode() : FibHeapNode() { N = 0; }

	void HeapNode::operator =(double NewKeyVal)
	{
		HeapNode Tmp;
		Tmp.N = N = NewKeyVal;
		FHN_Assign(Tmp);
	}

	//以下五个函数原本定义在hxx
	void operator =(FibHeapNode& RHS) 
	{
		FHN_Assign(RHS);
		N = ((HeapNode&)RHS).N;
	}

	int  operator ==(FibHeapNode& RHS) 
	{
		if (FHN_Cmp(RHS)) return 0;
		return N == ((HeapNode&)RHS).N ? 1 : 0;
	}

	int  operator <(FibHeapNode& RHS) 
	{
		int X;
		if ((X = FHN_Cmp(RHS)) != 0)
			return X < 0 ? 1 : 0;
		return N < ((HeapNode&)RHS).N ? 1 : 0;
	}
	virtual void Print() 
	{
		FibHeapNode::Print();
		//    mexPrintf( "%f (%d)" , N , IndexV );
	}

    double GetKeyValue() { return N; }
    void SetKeyValue(double n) { N = n; }
    long int GetIndexValue() { return IndexV; }
    void SetIndexValue( long int v) { IndexV = v; }
};

const float  DIST_INF = std::numeric_limits<float>::max();
const float  DIST_EPSION = 1e-3;
const unsigned char NNGBH = 26;

typedef float FPixelType;

//SrcPixelType\short  LabPixelType\unsigned char
class FastGrowCut : public IQF_FastGrowCutSegmentation
{
public:
    FastGrowCut();
    ~FastGrowCut();
	
    //void SetSourceImage(const std::vector<short>& imSrc);
    virtual void SetSourceImage(vtkImageData* vtkSourceImage);
    //void SetSeedlImage(std::vector<unsigned char>& imSeed);
    virtual void SetSeedlImage(vtkImageData* vtkSeedImage);
    virtual void SetWorkMode(bool bSegInitialized = false);
    virtual void Release();
	
    virtual void Init();
	
    //void SetImageSize(const std::vector<long>& imSize);
    virtual void DoSegmentation();
    //void GetLabeImage(std::vector<unsigned char>& imLab);
    //void GetForegroundmage(std::vector<unsigned char>& imFgrd);
    virtual void GetForegroundmage(vtkImageData* vtkResultImage);

private:
    void InitializationAHP();
    void DijkstraBasedClassificationAHP();

	vtkImageData* m_pSourceImage;
	vtkImageData* m_pSeedIamge;
	std::vector<long> m_imROI;
	std::vector<unsigned char> m_imLabVec;


    std::vector<short> m_imSrc;
    std::vector<unsigned char> m_imSeed;
    std::vector<unsigned char> m_imLabPre;
    std::vector<FPixelType> m_imDistPre;
    std::vector<unsigned char> m_imLab;
    std::vector<FPixelType> m_imDist;

    std::vector<long> m_imSize;
    long m_DIMX, m_DIMY, m_DIMZ, m_DIMXY, m_DIMXYZ;
    std::vector<int> m_indOff;
    std::vector<unsigned char>  m_NBSIZE;

    FibHeap *m_heap;
    HeapNode *m_hpNodes;
    bool m_bSegInitialized;
};

}

#endif

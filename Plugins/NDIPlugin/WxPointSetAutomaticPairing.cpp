#include "WxPointSetAutomaticPairing.h"

WxPointSetAutomaticPairing::WxPointSetAutomaticPairing()
{
}

WxPointSetAutomaticPairing::~WxPointSetAutomaticPairing()
{
}

bool WxPointSetAutomaticPairing::PointSetAutomaticPariring(QList<QVector3D> &pointset1,QList<QVector3D> &pointset2)
{
	if (pointset1.size()!=pointset2.size())  return false;
	
	//compute distance vector
	for(int i=0; i<pointset1.size();i++)
	{
		QList<double> templist;
		for(int j=0; j<pointset1.size();j++)
		{
			if(i!=j) templist.append(distanceTwoPoint(pointset1[i],pointset1[j]));
		}
		//sort from max to min
		qSort(templist.begin(),templist.end(),qGreater<double>());

		qDebug()<<"pointset1 distance vector\n";
		foreach(double iterdouble,templist)
		{
			qDebug()<<iterdouble<<",";
		}
		//qDebug()<<"\n";

		m_PointSetDistance1.append(templist);
	}

	for(int i=0; i<pointset2.size();i++)
	{
		QList<double> templist;
		for(int j=0; j<pointset2.size();j++)
		{
			if(i!=j) templist.append(distanceTwoPoint(pointset2[i],pointset2[j]));
		}
		//sort from max to min
		qSort(templist.begin(),templist.end(),qGreater<double>());

		qDebug()<<"pointset2 distance vector\n";
		foreach(double iterdouble,templist)
		{
			qDebug()<<iterdouble<<",";
		}
		//qDebug()<<"\n";

		m_PointSetDistance2.append(templist);
	}

	//compute distance vector coefficient
	for(int i=0; i<m_PointSetDistance1.size();i++)
	{
		QList<double> templist;
		for(int j=0; j<m_PointSetDistance2.size();j++)
		{
			 templist.append(coefficientTwoListPoint(m_PointSetDistance1[i],m_PointSetDistance2[j]));
		}
		m_PointSetCoefficient.append(templist);

		qDebug()<<"two pointset distance vector coefficient\n";
		foreach(double iterdouble,templist)
		{
			qDebug()<<iterdouble<<",";
		}
		//qDebug()<<"\n";
	}
	
	//compute  max coefficient of distance vector each column
	for (int i=0;i<m_PointSetCoefficient.size();i++)
	{
		m_AutoPairedPointSet1.append(pointset1[i]);

		double columnmax=m_PointSetCoefficient[i][0];
		int index=0;
		for (int j=0;j<m_PointSetCoefficient[i].size();j++)
		{
			if(m_PointSetCoefficient[i][j]>columnmax)
			{
				columnmax=m_PointSetCoefficient[i][j];
				index=j;
			}
		}
		m_AutoPairedPointSet2.append(pointset2[index]);
	}
	return true;
}

double WxPointSetAutomaticPairing::distanceTwoPoint(const QVector3D& point1,const QVector3D& point2)
{
	return (point1-point2).length();
}

double WxPointSetAutomaticPairing::coefficientTwoListPoint(const QList<double>& listpoint1,const QList<double>& listpoint2)
{
	double total=0,sum1=0,sum2=0;
	int dimension=listpoint1.size();

	for(int i=0;i<dimension;i++)
	{
		total+=listpoint1[i]*listpoint2[i];
		sum1+=qPow(listpoint1[i],2);
		sum2+=qPow(listpoint2[i],2);
	}
	return total/(qSqrt(sum1)*qSqrt(sum2));
}
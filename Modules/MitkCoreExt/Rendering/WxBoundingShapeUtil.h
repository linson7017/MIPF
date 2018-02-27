/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef WxBoundingShapeUtil_h
#define WxBoundingShapeUtil_h

#include <mitkBaseData.h>
#include <mitkBaseGeometry.h>
#include <mitkInteractionConst.h>

#include "qf_config.h"

namespace mitk
{
  /**
  * \brief helper function for calculating corner points of the bounding object from a given geometry
  */
	QF_API std::vector<mitk::Point3D> GetCornerPoints(mitk::BaseGeometry::Pointer geometry, bool visualizationOffset);

  /**
  * \brief helper function for calculating the average of two points
  */
	QF_API mitk::Point3D CalcAvgPoint(mitk::Point3D a, mitk::Point3D b);

  std::vector<int> GetHandleIndices(int index);

  /**
  * \brief Helper Class for realizing the handles of bounding object encapsulated by a geometry data
  * \ingroup Data
  */
  class Handle final
  {
  public:
    Handle();
    Handle(mitk::Point3D pos, int index, std::vector<int> faceIndices, bool active = false);

    ~Handle();

    bool IsActive();
    bool IsNotActive();
    void SetActive(bool status);
    void SetIndex(int index);
    int GetIndex();
    std::vector<int> GetFaceIndices();
    void SetPosition(mitk::Point3D pos);
    mitk::Point3D GetPosition();

  private:
    bool m_IsActive;
    mitk::Point3D m_Position;
    std::vector<int> m_FaceIndices;
    int m_Index;
  };
}

#endif

#ifndef __POSITIONBASEDELASTICRODSMODEL_H__
#define __POSITIONBASEDELASTICRODSMODEL_H__

#include "Demos/Simulation/ParticleData.h"
#include "Demos/Simulation/SimulationModel.h"
#include <vector>
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkImageData.h>
#include <vtkCellLocator.h>

namespace PBD 
{	
	class Constraint;

	class PositionBasedElasticRodsModel : public SimulationModel
	{
		public:
			PositionBasedElasticRodsModel();
			virtual ~PositionBasedElasticRodsModel();


            std::vector<vtkSmartPointer<vtkPolyData>> m_objects;
            std::vector<vtkSmartPointer<vtkImageData>> m_imgs;
            void initialize();
		protected:
			ParticleData m_ghostParticles;
			Vector3r m_restDarbouxVector;
			Vector3r m_stiffness;

		public:
			virtual void reset();
			virtual void cleanup();

			ParticleData &getGhostParticles();
			void addElasticRodModel(
				const unsigned int nPoints,
				Vector3r *points);

			bool addPerpendiculaBisectorConstraint(const unsigned int p0, const unsigned int p1, const unsigned int p2);
			bool addGhostPointEdgeDistanceConstraint(const unsigned int pA, const unsigned int pB, const unsigned int pG);
			bool addDarbouxVectorConstraint(const unsigned int pA, const unsigned int pB,
											const unsigned int pC, const unsigned int pD, const unsigned int pE);
            bool addParticalPolyDataConstraint();

			void setRestDarbouxVector(const Vector3r &val) { m_restDarbouxVector = val; }
			Vector3r &getRestDarbouxVector() { return m_restDarbouxVector; }
			void setBendingAndTwistingStiffness(const Vector3r &val) { m_stiffness = val; }
			Vector3r &getBendingAndTwistingStiffness() { return m_stiffness; }

            void addvtkPolyDataModel(vtkSmartPointer<vtkPolyData> polydata);

            std::vector<vtkSmartPointer<vtkPolyData>> getvtkPolyDataModels() {
                return m_objects;
            }
	};
}

#endif
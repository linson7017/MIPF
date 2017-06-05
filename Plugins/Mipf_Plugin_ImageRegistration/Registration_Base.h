#ifndef Registration_Base_h__
#define Registration_Base_h__

#pragma once
#include <itkMatrix.h>
#include <itkCommand.h>
class ItkNotifier;

template<class FixedImageType, class MovingImageType>
class RegistrationBase
{
public:
    RegistrationBase():m_useMultiResolution(false), m_notifier(NULL), m_resolutionLevel(3), m_quitAfterFinished(true){}
    ~RegistrationBase() {}
    virtual void Start(const FixedImageType* fixedImage, const MovingImageType* movingImage, FixedImageType* resultImage, itk::Matrix<double, 4, 4>& initTransformMatrix = itk::Matrix<double, 4, 4>()) = 0;
    virtual void Stop() = 0;
    void SetNotifier(ItkNotifier* notifier)
    {
        m_notifier = notifier;
    }
    void SetUseMultiResolution(bool useMultiResolution) { m_useMultiResolution = useMultiResolution; }
    void SetBeginStepLength(double stepLength){}
    void SetMultiResolutionLevel(int level) { m_resolutionLevel = level; }
    void SetQuitAfterFinished(bool b) { m_quitAfterFinished = b; }
protected:
    ItkNotifier* m_notifier;
    bool m_useMultiResolution;
    bool m_quitAfterFinished;
    int m_resolutionLevel;
};

//Use for multi resolution registration
template <typename TRegistration, typename TOptimizer>
class RegistrationInterfaceCommand : public itk::Command
{
public:
    typedef  RegistrationInterfaceCommand   Self;
    typedef  itk::Command                   Superclass;
    typedef  itk::SmartPointer<Self>        Pointer;
    itkNewMacro(Self);
protected:
    RegistrationInterfaceCommand() {};

public:
    typedef   TRegistration                              RegistrationType;
    typedef   RegistrationType *                         RegistrationPointer;
    typedef   TOptimizer   OptimizerType;
    typedef   OptimizerType *                            OptimizerPointer;

    void Execute(itk::Object * object, const itk::EventObject & event)
    {
        if (!(itk::IterationEvent().CheckEvent(&event)))
        {
            return;
        }

        RegistrationPointer registration = dynamic_cast<RegistrationPointer>(object);

        OptimizerPointer optimizer =
            dynamic_cast<OptimizerPointer>(registration->GetOptimizer());

        std::cout << "-------------------------------------" << std::endl;
        std::cout << "MultiResolution Level : "
            << registration->GetCurrentLevel() << std::endl;
        std::cout << std::endl;

        if (registration->GetCurrentLevel() == 0)
        {
            optimizer->SetMaximumStepLength(4.00);
            optimizer->SetMinimumStepLength(0.1);
        }
        else
        {
            optimizer->SetMaximumStepLength(optimizer->GetMaximumStepLength() / 2.0);
            optimizer->SetMinimumStepLength(optimizer->GetMinimumStepLength() / 10.0);
        }
    }

    void Execute(const itk::Object *, const itk::EventObject &)
    {
        return;
    }
};




#endif // Registration_Base_h__

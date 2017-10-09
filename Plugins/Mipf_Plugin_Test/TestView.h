#ifndef TestView_h__ 
#define TestView_h__ 
 
#include "MitkPluginView.h" 
 
class TestView : public MitkPluginView  
{  
public:   
    TestView(); 
    ~TestView();
    void CreateView() override;
    WndHandle GetPluginHandle() override;
};
#endif // TestView_h__ 
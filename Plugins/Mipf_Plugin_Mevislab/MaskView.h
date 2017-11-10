#ifndef MaskView_h__ 
#define MaskView_h__ 
 
#include "MitkPluginView.h" 
 
class MaskView : public MitkPluginView  
{  
public:   
    MaskView(); 
    ~MaskView();
    void CreateView() override;
    WndHandle GetPluginHandle() override;
};
#endif // MaskView_h__ 
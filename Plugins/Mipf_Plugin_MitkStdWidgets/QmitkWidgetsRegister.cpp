#include "QmitkWidgetsRegister.h"

#include "Utils/QObjectFactory.h"

#include "QmitkDataStorageComboBox.h"
#include "QmitkPointListWidget.h"

void RegisterQmitkWidgets()
{
    REGISTER_CLASS("QmitkDataStorageComboBox", QmitkDataStorageComboBox);
    REGISTER_CLASS("QmitkPointListWidget", QmitkPointListWidget);
    
}

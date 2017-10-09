#include "QmitkWidgetsRegister.h"

#include "Utils/QObjectFactory.h"

#include "QmitkDataStorageComboBox.h"
#include "QmitkPointListWidget.h"

void RegisterQmitkWidgets()
{
    REGISTER_QOBJECT("QmitkDataStorageComboBox", QmitkDataStorageComboBox);
    REGISTER_QOBJECT("QmitkPointListWidget", QmitkPointListWidget);
    
}

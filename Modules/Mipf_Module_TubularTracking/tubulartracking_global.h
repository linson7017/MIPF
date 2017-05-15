#pragma once

#include <QtCore/qglobal.h>

#ifndef QT_STATIC
# if defined(TUBULARTRACKING_LIB)
#  define TUBULARTRACKING_EXPORT Q_DECL_EXPORT
# else
#  define TUBULARTRACKING_EXPORT Q_DECL_IMPORT
# endif
#else
# define TUBULARTRACKING_EXPORT
#endif

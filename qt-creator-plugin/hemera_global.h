#ifndef HEMERA_GLOBAL_H
#define HEMERA_GLOBAL_H

#include <QtGlobal>

#if defined(HEMERA_LIBRARY)
#  define HEMERASHARED_EXPORT Q_DECL_EXPORT
#else
#  define HEMERASHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // HEMERA_GLOBAL_H


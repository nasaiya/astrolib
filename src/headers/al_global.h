#ifndef AL_GLOBAL_H
#define AL_GLOBAL_H



#if defined(ASTROLIB_LIBRARY)
#  define ASTROLIB_EXPORT Q_DECL_EXPORT
#else
#  define ASTROLIB_EXPORT Q_DECL_IMPORT
#endif



#include <QString>
#include <QTextStream>

#define QT_USE_FAST_CONCATENATION
#define QT_USE_FAST_OPERATOR_PLUS

#ifdef __DEBUG
#define ___DEBUG true

#define debug(...)  QTextStream(stderr) << QStringList({"Debug: (line: ", QString::number(__LINE__),"): ",QString(__PRETTY_FUNCTION__),": ",__VA_ARGS__,"\n"}).join("")

#else
#define ___DEBUG false

#define debug(...) { QString dbgcode = QString(__VA_ARGS__); }
#endif

#ifdef __DDEBUG
#define ___DDEBUG true

#define ddebug(...) QTextStream(stderr) << QString(__VA_ARGS__) << "\n"

#else
#define ___DDEBUG false

#define ddebug(...) { QString ddbgcode = QString(__VA_ARGS__); }
#endif

#ifdef __WARNINGS
#define ___WARNINGS true

#define warn(...) QTextStream(stderr) << QStringList({"WARNING: ",QString(__PRETTY_FUNCTION__),": ",__VA_ARGS__,"\n    Warning occured at: ",QString(__FILE__),":",QString::number(__LINE__),"\n"}).join("")

#else
#define ___WARNINGS false

#define warn(...) { QString dbgwrncode = QString(__VA_ARGS__); }
#endif

#ifdef __ERRORS
#define ___ERRORS true

#define err(...) QTextStream(stderr) << QStringList({"ERROR: ",QString(__PRETTY_FUNCTION__),": ",__VA_ARGS__,"\n    Error occured at: ",QString(__FILE__),":",QString::number(__LINE__),"\n"}).join("")

#else
#define ___ERRORS false

#define err(...) { QString dbgerrcode=QString(__VA_ARGS__); }
#endif


#endif //AL_GLOBAL_H

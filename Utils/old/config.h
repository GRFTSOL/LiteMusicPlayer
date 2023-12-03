#pragma once

#ifndef _ML_CONFIG_H
#define _ML_CONFIG_H

// #define _LINUX_GTK2
// #define _LINUX
// #define _MAC_OS
// #define _IPHONE
// #define _ANDROID

#ifdef __MAC_OS_X_VERSION_MAX_ALLOWED
#ifndef _MAC_OS
#define _MAC_OS
#endif
#endif

#if defined(_IPHONE) || defined(_MAC_OS)
#define _MAC_OS
#define UTF8 // in iPhone and Mac OS X, default character encoding is UTF8
#endif

#ifdef _ANDROID
#define UTF8
#endif

#ifdef _LINUX
#define UTF8
#ifndef _DEBUG
#define _DEBUG
#endif
#ifndef NO_LINUX_GTK2
#ifndef _LINUX_GTK2
#define _LINUX_GTK2
#endif
#endif
#endif

#ifdef UTF8
#define DEFAULT_ENCODING    ED_UTF8
#endif

#ifdef UNICODE
#define DEFAULT_ENCODING    ED_UNICODE
#endif

#ifndef DEFAULT_ENCODING
#define DEFAULT_ENCODING    ED_SYSDEF
#endif

#ifdef DEBUG
#ifndef _DEBUG
#define _DEBUG
#endif
#endif


#endif

#ifndef __MYSTD_STDWIN_H__
#define __MYSTD_STDWIN_H__

#include <mystd/stdlib.h>

#include <io.h>
#include <direct.h>
#include <windows.h>

#include <sys/stat.h>

void MyWindowsPrintLastError(MyContext context);

#define MY_ASSERT_WIN(x) do { if (!(x)) { MyWindowsPrintLastError(MY_CONTEXT(NULL)); MyExit(); } } while(false);
#define MY_ASSERT_WINBOOL(x) MY_ASSERT_WIN(x);
#define MY_ASSERT_WINHANDLE(x) MY_ASSERT_WIN((x) != INVALID_HANDLE_VALUE && (x) != NULL);

#endif /* __MYSTD_STDWIN_H__ */
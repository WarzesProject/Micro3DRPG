#include "stdafx.h"

//-----------------------------------------------------------------------------
#if SE_COMPILER_MSVC
// 3rdparty Lib
#	pragma comment(lib, "SDL2.lib")
#	pragma comment(lib, "SDL2main.lib")
// Engine Lib
#	pragma comment(lib, "Core.lib")
#	pragma comment(lib, "Engine.lib")
#	pragma comment(lib, "3rdparty.lib")
#endif
//-----------------------------------------------------------------------------
int main(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	return 0;
}
//-----------------------------------------------------------------------------
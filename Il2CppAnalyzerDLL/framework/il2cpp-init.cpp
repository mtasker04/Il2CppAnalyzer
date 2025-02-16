#include "pch-il2cpp.h"

#include "il2cpp-appdata.h"
#include "il2cpp-init.h"
#include "helpers.h"

#define DO_API(r, n, p) r (*n) p
#include "il2cpp-api-functions.h"
#undef DO_API

#define DO_APP_FUNC(a, r, n, p) r (*n) p
#define DO_APP_FUNC_METHODINFO(a, n) struct MethodInfo ** n
namespace app {
#include "il2cpp-functions.h"
}
#undef DO_APP_FUNC
#undef DO_APP_FUNC_METHODINFO

#define DO_TYPEDEF(a, n) n ## __Class** n ## __TypeInfo
namespace app {
#include "il2cpp-types-ptr.h"
}
#undef DO_TYPEDEF

void init_il2cpp()
{
	uintptr_t baseAddress = il2cppi_get_base_address();

	using namespace app;

	#define DO_API(r, n, p) n = (r (*) p)(baseAddress + n ## _ptr)
	#include "il2cpp-api-functions.h"
	#undef DO_API

	#define DO_APP_FUNC(a, r, n, p) n = (r (*) p)(baseAddress + a)
 	#define DO_APP_FUNC_METHODINFO(a, n) n = (struct MethodInfo **)(baseAddress + a)
	#include "il2cpp-functions.h"
	#undef DO_APP_FUNC
 	#undef DO_APP_FUNC_METHODINFO

	#define DO_TYPEDEF(a, n) n ## __TypeInfo = (n ## __Class**) (baseAddress + a);
	#include "il2cpp-types-ptr.h"
	#undef DO_TYPEDEF
}
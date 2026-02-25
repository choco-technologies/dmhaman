#ifndef DMOD_MOD_DEFS_H_dmhaman
#define DMOD_MOD_DEFS_H_dmhaman

#include "dmod_defs.h"
#include "dmod_types.h"

#ifndef dmod_dmhaman_version
#  define dmod_dmhaman_version "0.1"
#endif

#ifdef DMOD_dmhaman
#  define dmod_dmhaman_api_to_mal(MODULE,NAME)                            \
            DMOD_API_TO_MAL(dmhaman, MODULE, NAME)
#  define dmod_dmhaman_api_to_mal_ex(NAME_IN, MODULE_MAL, NAME_MAL)       \
            DMOD_API_TO_MAL_EX(dmhaman, MODULE_IN, NAME_IN, MODULE_MAL, NAME_MAL)
#  define dmod_dmhaman_api(VERSION, RET, NAME, PARAMS)                    \
            DMOD_INPUT_API(dmhaman, DMOD_MAKE_VERSION(VERSION,0.1), RET, NAME, PARAMS)
#  define dmod_dmhaman_global_api(VERSION, RET, NAME, PARAMS)             \
            DMOD_GLOBAL_INPUT_API(dmhaman, DMOD_MAKE_VERSION(VERSION,0.1), RET, NAME, PARAMS)
#  define dmod_dmhaman_mal(VERSION, RET, NAME, PARAMS)                    \
            DMOD_MAL_OUTPUT_API(dmhaman , DMOD_MAKE_VERSION(VERSION,0.1), RET, NAME, PARAMS)
#  define dmod_dmhaman_global_mal(VERSION, RET, NAME, PARAMS)             \
            DMOD_GLOBAL_MAL_OUTPUT_API(dmhaman, DMOD_MAKE_VERSION(VERSION,0.1), RET, NAME, PARAMS)
#  define dmod_dmhaman_api_declaration(VERSION, RET, NAME, PARAMS)        \
            DMOD_INPUT_API_DECLARATION(dmhaman, DMOD_MAKE_VERSION(VERSION,0.1), RET, NAME, PARAMS)
#  define dmod_dmhaman_dif(VERSION, RET, NAME, PARAMS)                    \
            DMOD_DIF_FUNCTION_TYPE_DECLARATION(dmhaman, VERSION, RET, NAME, PARAMS);\
            DMOD_DIF_SIGNATURE_REGISTRATION(dmhaman, NAME, DMOD_MAKE_VERSION(VERSION,0.1))
#  define dmod_dmhaman_dif_api_declaration(VERSION, IMPL_MODULE, RET, NAME, PARAMS)  \
            DMOD_DIF_API_DECLARATION(dmhaman, IMPL_MODULE, DMOD_MAKE_VERSION(VERSION,0.1), RET, NAME, PARAMS)
#   ifndef DMOD_MODULE_NAME
#       define DMOD_MODULE_NAME        "dmhaman"
#   endif
#   ifndef DMOD_MODULE_VERSION
#       define DMOD_MODULE_VERSION     "0.1"
#   endif
#   define DMOD_AUTHOR_NAME        "Choco Technologies"
#   define DMOD_STACK_SIZE         1024
#   define DMOD_PRIORITY           1
#   define DMOD_MODULE_TYPE        Dmod_ModuleType_Library
#   define DMOD_MANUAL_LOAD        OFF
#else
#  ifdef DMOD_MAL_dmhaman
#  define dmod_dmhaman_mal(VERSION, RET, NAME, PARAMS)            \
                DMOD_MAL_INPUT_API(dmhaman, DMOD_MAKE_VERSION(VERSION,0.1), RET, NAME, PARAMS)
#  define dmod_dmhaman_global_mal(VERSION, RET, NAME, PARAMS)     \
                DMOD_GLOBAL_MAL_INPUT_API(dmhaman, DMOD_MAKE_VERSION(VERSION,0.1), RET, NAME, PARAMS)
#else 
#  define dmod_dmhaman_mal(VERSION, RET, NAME, PARAMS)            \
                DMOD_MAL_OUTPUT_API(dmhaman , DMOD_MAKE_VERSION(VERSION,0.1), RET, NAME, PARAMS)
#  define dmod_dmhaman_global_mal(VERSION, RET, NAME, PARAMS)     \
                DMOD_GLOBAL_MAL_OUTPUT_API(dmhaman, DMOD_MAKE_VERSION(VERSION,0.1), RET, NAME, PARAMS)
#endif
#  define dmod_dmhaman_api(VERSION, RET, NAME, PARAMS)            \
                DMOD_OUTPUT_API(dmhaman, DMOD_MAKE_VERSION(VERSION,0.1), RET, NAME, PARAMS)
#  define dmod_dmhaman_global_api(VERSION, RET, NAME, PARAMS)     \
                DMOD_GLOBAL_OUTPUT_API(dmhaman, DMOD_MAKE_VERSION(VERSION,0.1), RET, NAME, PARAMS)
#  define dmod_dmhaman_dif(VERSION, RET, NAME, PARAMS)            \
                DMOD_DIF_FUNCTION_TYPE_DECLARATION(dmhaman, VERSION, RET, NAME, PARAMS);\
                DMOD_DIF_SIGNATURE_REGISTRATION(dmhaman, NAME, DMOD_MAKE_VERSION(VERSION,0.1))
#  define dmod_dmhaman_dif_api_declaration(VERSION, IMPL_MODULE, RET, NAME, PARAMS)  \
                RET DMOD_MAKE_DIF_API_FUNCTION_NAME(dmhaman, IMPL_MODULE, NAME) PARAMS; \
                _DMOD_DIF_API_REGISTRATION(dmhaman, IMPL_MODULE, DMOD_MAKE_VERSION(VERSION,0.1), NAME) \
                RET DMOD_MAKE_DIF_API_FUNCTION_NAME(dmhaman, IMPL_MODULE, NAME) PARAMS
#endif

#endif // DMOD_MOD_DEFS_H_dmhaman

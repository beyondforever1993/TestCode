#include "include.h"

#include"cpu\cpu_core.c"

//#include"csp\csp_int.c"

#if (CPU_CFG_TS_TMR_EN == DEF_ENABLED)
#include"bsp\cpu_bsp.c"
#endif

#include"lib\lib_ascii.c"
#include"lib\lib_math.c"

#if (LIB_MEM_CFG_ALLOC_EN == DEF_ENABLED)                               /* -------------- MEM POOL FNCTS -------------- */
#include"lib\lib_mem.c"
#endif

#include"lib\lib_str.c"

#include"src\os_core.c"
#include"src\os_cpu_c.c"
#include"src\os_csp.c"
#include"src\os_dbg.c"
#include"src\os_flag.c"
#include"src\os_int.c"
#include"src\os_mem.c"
#include"src\os_msg.c"
#include"src\os_mutex.c"
#include"src\os_pend_multi.c"
#include"src\os_prio.c"
#include"src\os_q.c"
#include"src\os_sem.c"
#include"src\os_stat.c"
#include"src\os_task.c"
#include"src\os_tick.c"
#include"src\os_time.c"
#include"src\os_tmr.c"
#include"src\os_var.c"

#include"task\task.c"

#include"app\os_cfg_app.c"
#include"app\os_app_hooks.c"




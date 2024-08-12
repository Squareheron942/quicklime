#define DEBUG true
#define CONSOLE true
#define PROFILING true

#define THREAD_YIELD svcSleepThread(0)

#define SCENELOADER_THREAD_STACK_SZ (32 * 1024)    // 32kB stack for scene loader thread
#define PHYSICS_THREAD_STACK_SZ (32 * 1024)
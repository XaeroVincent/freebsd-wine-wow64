--- dlls/ntdll/unix/signal_x86_64.c.orig	2026-03-20 13:33:36.000000000 -0700
+++ dlls/ntdll/unix/signal_x86_64.c	2026-03-25 22:30:16.613555000 -0700
@@ -201,6 +201,9 @@ __ASM_GLOBAL_FUNC( modify_ldt,
 
 #elif defined(__FreeBSD__) || defined (__FreeBSD_kernel__)
 
+#include <machine/cpufunc.h>
+#include <machine/segments.h>
+#include <machine/specialreg.h>
 #include <machine/trap.h>
 
 #define RAX_sig(context)     ((context)->uc_mcontext.mc_rax)
@@ -533,7 +536,7 @@ static LONG syscall_dispatch_enabled = TRUE;
 static UINT64 xstate_extended_features;
 static LONG syscall_dispatch_enabled = TRUE;
 
-#if defined(__linux__) || defined(__APPLE__)
+#if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
 static inline TEB *get_current_teb(void)
 {
     unsigned long rsp;
@@ -602,7 +605,7 @@ static inline void context_init_xstate( CONTEXT *conte
 
 
 /***********************************************************************
- *           dwarf_virtual_unwind
+ * dwarf_virtual_unwind
  *
  * Equivalent of RtlVirtualUnwind for builtin modules.
  */
@@ -718,7 +721,7 @@ static NTSTATUS dwarf_virtual_unwind( ULONG64 ip, ULON
 
 #ifdef HAVE_LIBUNWIND
 /***********************************************************************
- *           libunwind_virtual_unwind
+ * libunwind_virtual_unwind
  *
  * Equivalent of RtlVirtualUnwind for builtin modules.
  */
@@ -848,7 +851,7 @@ static NTSTATUS libunwind_virtual_unwind( ULONG64 ip, 
 
 
 /***********************************************************************
- *           unwind_builtin_dll
+ * unwind_builtin_dll
  */
 NTSTATUS unwind_builtin_dll( void *args )
 {
@@ -911,7 +914,7 @@ __ASM_GLOBAL_FUNC( clear_alignment_flag,
 
 
 /***********************************************************************
- *           init_handler
+ * init_handler
  */
 static inline ucontext_t *init_handler( void *sigcontext )
 {
@@ -942,7 +945,7 @@ static inline ucontext_t *init_handler( void *sigconte
 
 
 /***********************************************************************
- *           leave_handler
+ * leave_handler
  */
 static inline void leave_handler( ucontext_t *sigcontext )
 {
@@ -974,7 +977,7 @@ static inline void leave_handler( ucontext_t *sigconte
 
 
 /***********************************************************************
- *           save_context
+ * save_context
  *
  * Set the register values from a sigcontext.
  */
@@ -1046,7 +1049,7 @@ static void save_context( struct xcontext *xcontext, c
 
 
 /***********************************************************************
- *           fixup_frame_fpu_state
+ * fixup_frame_fpu_state
  *
  * Set FP frame state not saved in __wine_unix_call_dispatcher from sigcontext.
  */
@@ -1068,7 +1071,7 @@ static void fixup_frame_fpu_state( struct syscall_fram
 
 
 /***********************************************************************
- *           restore_context
+ * restore_context
  *
  * Build a sigcontext from the register values.
  */
@@ -1089,7 +1092,7 @@ static void restore_context( const struct xcontext *xc
 
 
 /***********************************************************************
- *           signal_set_full_context
+ * signal_set_full_context
  */
 NTSTATUS signal_set_full_context( CONTEXT *context )
 {
@@ -1102,7 +1105,7 @@ NTSTATUS signal_set_full_context( CONTEXT *context )
 
 
 /***********************************************************************
- *              get_native_context
+ * get_native_context
  */
 void *get_native_context( CONTEXT *context )
 {
@@ -1111,7 +1114,7 @@ void *get_native_context( CONTEXT *context )
 
 
 /***********************************************************************
- *              get_wow_context
+ * get_wow_context
  */
 void *get_wow_context( CONTEXT *context )
 {
@@ -1121,8 +1124,8 @@ void *get_wow_context( CONTEXT *context )
 
 
 /***********************************************************************
- *              NtSetContextThread  (NTDLL.@)
- *              ZwSetContextThread  (NTDLL.@)
+ * NtSetContextThread  (NTDLL.@)
+ * ZwSetContextThread  (NTDLL.@)
  */
 NTSTATUS WINAPI NtSetContextThread( HANDLE handle, const CONTEXT *context )
 {
@@ -1221,8 +1224,8 @@ NTSTATUS WINAPI NtSetContextThread( HANDLE handle, con
 
 
 /***********************************************************************
- *              NtGetContextThread  (NTDLL.@)
- *              ZwGetContextThread  (NTDLL.@)
+ * NtGetContextThread  (NTDLL.@)
+ * ZwGetContextThread  (NTDLL.@)
  */
 NTSTATUS WINAPI NtGetContextThread( HANDLE handle, CONTEXT *context )
 {
@@ -1359,7 +1362,7 @@ NTSTATUS WINAPI NtGetContextThread( HANDLE handle, CON
 
 
 /***********************************************************************
- *              set_thread_wow64_context
+ * set_thread_wow64_context
  */
 NTSTATUS set_thread_wow64_context( HANDLE handle, const void *ctx, ULONG size )
 {
@@ -1462,7 +1465,7 @@ NTSTATUS set_thread_wow64_context( HANDLE handle, cons
 
 
 /***********************************************************************
- *              get_thread_wow64_context
+ * get_thread_wow64_context
  */
 NTSTATUS get_thread_wow64_context( HANDLE handle, void *ctx, ULONG size )
 {
@@ -1574,7 +1577,7 @@ NTSTATUS get_thread_wow64_context( HANDLE handle, void
 
 
 /***********************************************************************
- *           setup_raise_exception
+ * setup_raise_exception
  */
 static void setup_raise_exception( ucontext_t *sigcontext, EXCEPTION_RECORD *rec, struct xcontext *xcontext )
 {
@@ -1651,7 +1654,7 @@ static void setup_raise_exception( ucontext_t *sigcont
 
 
 /***********************************************************************
- *           setup_exception
+ * setup_exception
  *
  * Setup a proper stack frame for the raise function, and modify the
  * sigcontext so that the return from the signal handler will call
@@ -1668,7 +1671,7 @@ static void setup_exception( ucontext_t *sigcontext, E
 
 
 /***********************************************************************
- *           call_user_apc_dispatcher
+ * call_user_apc_dispatcher
  */
 NTSTATUS call_user_apc_dispatcher( CONTEXT *context, unsigned int flags, ULONG_PTR arg1, ULONG_PTR arg2, ULONG_PTR arg3,
                                    PNTAPCFUNC func, NTSTATUS status )
@@ -1718,7 +1721,7 @@ NTSTATUS call_user_apc_dispatcher( CONTEXT *context, u
 
 
 /***********************************************************************
- *           call_raise_user_exception_dispatcher
+ * call_raise_user_exception_dispatcher
  */
 void call_raise_user_exception_dispatcher(void)
 {
@@ -1727,7 +1730,7 @@ void call_raise_user_exception_dispatcher(void)
 
 
 /***********************************************************************
- *           call_user_exception_dispatcher
+ * call_user_exception_dispatcher
  */
 NTSTATUS call_user_exception_dispatcher( EXCEPTION_RECORD *rec, CONTEXT *context )
 {
@@ -1762,7 +1765,7 @@ NTSTATUS call_user_exception_dispatcher( EXCEPTION_REC
 
 
 /***********************************************************************
- *           call_user_mode_callback
+ * call_user_mode_callback
  */
 extern NTSTATUS call_user_mode_callback( ULONG64 user_rsp, void **ret_ptr, ULONG *ret_len, void *func, TEB *teb );
 __ASM_GLOBAL_FUNC( call_user_mode_callback,
@@ -1843,7 +1846,7 @@ __ASM_GLOBAL_FUNC( call_user_mode_callback,
 
 
 /***********************************************************************
- *           user_mode_callback_return
+ * user_mode_callback_return
  */
 extern void DECLSPEC_NORETURN user_mode_callback_return( void *ret_ptr, ULONG ret_len,
                                                          NTSTATUS status, TEB *teb );
@@ -1893,7 +1896,7 @@ __ASM_GLOBAL_FUNC( user_mode_callback_return,
 
 
 /***********************************************************************
- *           user_mode_abort_thread
+ * user_mode_abort_thread
  */
 extern void DECLSPEC_NORETURN user_mode_abort_thread( NTSTATUS status, struct syscall_frame *frame );
 __ASM_GLOBAL_FUNC( user_mode_abort_thread,
@@ -1913,7 +1916,7 @@ __ASM_GLOBAL_FUNC( user_mode_abort_thread,
 
 
 /***********************************************************************
- *           KeUserModeCallback
+ * KeUserModeCallback
  */
 NTSTATUS KeUserModeCallback( ULONG id, const void *args, ULONG len, void **ret_ptr, ULONG *ret_len )
 {
@@ -1935,7 +1938,7 @@ NTSTATUS KeUserModeCallback( ULONG id, const void *arg
 
 
 /***********************************************************************
- *           NtCallbackReturn  (NTDLL.@)
+ * NtCallbackReturn  (NTDLL.@)
  */
 NTSTATUS WINAPI NtCallbackReturn( void *ret_ptr, ULONG ret_len, NTSTATUS status )
 {
@@ -1945,7 +1948,7 @@ NTSTATUS WINAPI NtCallbackReturn( void *ret_ptr, ULONG
 
 
 /***********************************************************************
- *           is_privileged_instr
+ * is_privileged_instr
  *
  * Check if the fault location is a privileged instruction.
  */
@@ -2027,7 +2030,7 @@ static inline DWORD is_privileged_instr( CONTEXT *cont
 
 
 /***********************************************************************
- *           handle_interrupt
+ * handle_interrupt
  *
  * Handle an interrupt.
  */
@@ -2211,7 +2214,7 @@ static BOOL check_atl_thunk( ucontext_t *sigcontext, E
 
 
 /***********************************************************************
- *           handle_syscall_fault
+ * handle_syscall_fault
  *
  * Handle a page fault happening during a system call.
  */
@@ -2257,7 +2260,7 @@ static BOOL handle_syscall_fault( ucontext_t *sigconte
 
 
 /***********************************************************************
- *           handle_syscall_trap
+ * handle_syscall_trap
  *
  * Handle a trap exception during a system call.
  */
@@ -2305,7 +2308,7 @@ static BOOL handle_syscall_trap( ucontext_t *sigcontex
 
 
 /***********************************************************************
- *           check_invalid_gsbase
+ * check_invalid_gsbase
  *
  * Check for fault caused by invalid %gs value (some copy protection schemes mess with it).
  */
@@ -2737,7 +2740,7 @@ static void sigsys_handler( int signal, siginfo_t *sig
 
 
 /***********************************************************************
- *           ldt_set_entry
+ * ldt_set_entry
  */
 void ldt_set_entry( WORD sel, LDT_ENTRY entry )
 {
@@ -2756,6 +2759,16 @@ void ldt_set_entry( WORD sel, LDT_ENTRY entry )
     if ((ret = modify_ldt( &ldt_info ))) ERR( "modify_ldt failed %d\n", ret );
 #elif defined(__APPLE__)
     if (i386_set_ldt(sel >> 3, (union ldt_entry *)&entry, 1) < 0) perror("i386_set_ldt");
+#elif defined(__FreeBSD__)
+    struct i386_ldt_args p;
+    p.start = sel >> 3;
+    p.descs = (struct user_segment_descriptor *)&entry;
+    p.num   = 1;
+    if (sysarch(I386_SET_LDT, &p) == -1)
+    {
+        perror("i386_set_ldt");
+        exit(1);
+    }
 #else
     fprintf( stderr, "No LDT support on this platform\n" );
     exit(1);
@@ -2764,7 +2777,7 @@ void ldt_set_entry( WORD sel, LDT_ENTRY entry )
 
 
 /**********************************************************************
- *           get_thread_ldt_entry
+ * get_thread_ldt_entry
  */
 NTSTATUS get_thread_ldt_entry( HANDLE handle, THREAD_DESCRIPTOR_INFORMATION *info, ULONG len )
 {
@@ -2788,7 +2801,7 @@ NTSTATUS get_thread_ldt_entry( HANDLE handle, THREAD_D
 
 
 /**********************************************************************
- *             signal_init_threading
+ * signal_init_threading
  */
 void signal_init_threading(void)
 {
@@ -2877,6 +2890,72 @@ static int libc_addr_cb( struct dl_phdr_info *info, si
         libc_size = max( libc_size, info->dlpi_phdr[i].p_vaddr + info->dlpi_phdr[i].p_memsz );
 
     return 1;
+}
+#endif
+
+#ifdef __FreeBSD__
+static __siginfohandler_t *libthr_signal_handlers[_SIG_MAXSIG];
+
+/* occasionally signals happen right between %fs reset to GUFS32_SEL and fsbase correction,
+   which results in fsbase being incorrect on handler entry; restore fsbase ourselves */
+#if defined(__GNUC__) || defined(__clang__)
+__attribute__((no_stack_protector))
+#endif
+static void libthr_sighandler_wrapper(int sig, siginfo_t *info, void *_ucp)
+{
+    struct ntdll_thread_data *thread_data = (struct ntdll_thread_data *)&NtCurrentTeb()->GdiTebBatch;
+    void *pthread_teb = ((struct amd64_thread_data *)thread_data->cpu_data)->pthread_teb;
+    ucontext_t *uc = (ucontext_t *)_ucp;
+
+    if (pthread_teb)
+    {
+        /* Restore fsbase for the duration of this handler using the fastest available
+         * mechanism.  Avoid any libc call that might rely on a valid fsbase itself. */
+        if (user_shared_data->ProcessorFeatures[PF_RDWRFSGSBASE_AVAILABLE])
+            __asm__ volatile ("wrfsbase %0" :: "r" (pthread_teb));
+        else
+        {
+            void *fsbase_val = pthread_teb;
+            __asm__ volatile (
+                "movq %0, %%rsi\n\t"         /* Manually load into %rsi to allow clobbering */
+                "movq $0xa5, %%rax\n\t"      /* SYS_sysarch */
+                "movq $0x81, %%rdi\n\t"      /* AMD64_SET_FSBASE */
+                "syscall\n\t"
+                :
+                : "r" (&fsbase_val)
+                : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "memory"
+            );
+        }
+
+        /* Also fix mc_fsbase in the saved signal context.  Without this, the kernel
+         * restores the stale (broken) fsbase on sigreturn, undoing our fix above and
+         * leaving the thread with a corrupt fsbase until the next syscall entry. */
+        uc->uc_mcontext.mc_fsbase = (register_t)pthread_teb;
+    }
+
+    libthr_signal_handlers[sig - 1](sig, info, _ucp);
+}
+
+extern int __sys_sigaction(int, const struct sigaction * restrict, struct sigaction * restrict);
+
+static int wrap_libthr_signal_handlers(void)
+{
+    struct sigaction act;
+    int sig;
+
+    for (sig = 1; sig <= _SIG_MAXSIG; sig++)
+    {
+        if (__sys_sigaction(sig, NULL, &act) == -1) return -1;
+	if (act.sa_handler != SIG_DFL && act.sa_handler != SIG_IGN)
+        {
+            libthr_signal_handlers[sig - 1] = act.sa_sigaction;
+            act.sa_sigaction = libthr_sighandler_wrapper;
+
+            if (__sys_sigaction(sig, &act, NULL) == -1) return -1;
+        }
+    }
+
+    return 0;
 }
 #endif
 
@@ -2909,6 +2988,11 @@ void signal_init_process(void)
         fs32_sel = alloc_fs_sel( -1, wow_teb );
 #elif defined(__APPLE__)
         cs32_sel = ldt_alloc_entry( ldt_make_cs32_entry() );
+#elif defined(__FreeBSD__)
+        /* GSEL(GUCODE32_SEL, SEL_UPL) = 0x23.  Per-thread LDT fs allocation is
+         * handled by signal_alloc_thread(); fsbase detection is done inline in
+         * the ASM dispatchers via user_shared_data->ProcessorFeatures. */
+        cs32_sel = GSEL(GUCODE32_SEL, SEL_UPL);
 #endif
     }
 
@@ -2945,6 +3029,9 @@ void signal_init_process(void)
     sig_act.sa_sigaction = sigsys_handler;
     if (sigaction( SIGSYS, &sig_act, NULL ) == -1) goto error;
 #endif
+#ifdef __FreeBSD__
+    if (wrap_libthr_signal_handlers() == -1) goto error;
+#endif
     return;
 
  error:
@@ -2954,7 +3041,7 @@ void signal_init_process(void)
 
 
 /***********************************************************************
- *           init_syscall_frame
+ * init_syscall_frame
  */
 void init_syscall_frame( LPTHREAD_START_ROUTINE entry, void *arg, BOOL suspend, TEB *teb )
 {
@@ -2981,6 +3068,7 @@ void init_syscall_frame( LPTHREAD_START_ROUTINE entry,
 #endif
 #elif defined (__FreeBSD__) || defined (__FreeBSD_kernel__)
     amd64_set_gsbase( teb );
+    amd64_get_fsbase(&thread_data->pthread_teb);
 #elif defined(__NetBSD__)
     sysarch( X86_64_SET_GSBASE, &teb );
 #elif defined (__APPLE__)
@@ -3048,7 +3136,7 @@ void init_syscall_frame( LPTHREAD_START_ROUTINE entry,
 
 
 /***********************************************************************
- *           signal_start_thread
+ * signal_start_thread
  */
 __ASM_GLOBAL_FUNC( signal_start_thread,
                    "subq $0x38,%rsp\n\t"
@@ -3089,7 +3177,7 @@ __ASM_GLOBAL_FUNC( signal_start_thread,
 
 
 /***********************************************************************
- *           __wine_syscall_dispatcher
+ * __wine_syscall_dispatcher
  */
 __ASM_GLOBAL_FUNC( __wine_syscall_dispatcher,
                    __ASM_LOCAL_LABEL("__wine_syscall_dispatcher_gs_load") ":\n\t"
@@ -3201,6 +3289,23 @@ __ASM_GLOBAL_FUNC( __wine_syscall_dispatcher,
                    "movl $0x3000003,%eax\n\t"      /* _thread_set_tsd_base */
                    "syscall\n\t"
                    "leaq -0x98(%rbp),%rcx\n"
+#elif defined(__FreeBSD__)
+                   /* Restore pthread fsbase and WOW32 %%fs selector on syscall entry. */
+                   "movw 0x338(%r13),%ax\n\t"      /* amd64_thread_data()->fs */
+                   "testw %ax,%ax\n\t"
+                   "jz 2f\n\t"
+                   "movq 0x320(%r13),%rsi\n\t"     /* amd64_thread_data()->pthread_teb */
+                   "cmpb $0,0x7ffe028a\n\t"        /* user_shared_data->ProcessorFeatures[PF_RDWRFSGSBASE_AVAILABLE] */
+                   "jz 1f\n\t"
+                   "wrfsbase %rsi\n\t"
+                   "jmp 2f\n"
+                   "1:\n\t"
+                   "leaq 0x320(%r13),%rsi\n\t"     /* sysarch requires a pointer to the value */
+                   "movq $0xa5,%rax\n\t"           /* sysarch */
+                   "movq $0x81,%rdi\n\t"           /* AMD64_SET_FSBASE */
+                   "syscall\n\t"
+                   "leaq -0x98(%rbp),%rcx\n"
+                   "2:\n\t"
 #endif
                    "ldmxcsr 0x33c(%r13)\n\t"       /* amd64_thread_data()->mxcsr */
                    "movl 0xb0(%rcx),%eax\n\t"      /* frame->syscall_id */
@@ -3257,6 +3362,19 @@ __ASM_GLOBAL_FUNC( __wine_syscall_dispatcher,
                    "syscall\n\t"
                    "movq %rdx,%rcx\n\t"
                    "movq %r8,%rax\n\t"
+#elif defined(__FreeBSD__)
+                   /* Restore WOW32 %%fs on syscall return and fix %%ss after sysret.
+                    * AMD CPUs leave %%ss in a speculative state after sysret that
+                    * causes a #GP on the next stack access if not corrected here.
+                    * %%edx is reloaded from frame->restore_flags immediately after. */
+                   "movw 0x338(%r13),%dx\n\t"      /* amd64_thread_data()->fs */
+                   "testw %dx,%dx\n\t"
+                   "jz 1f\n\t"
+                   "movw %dx,%fs\n\t"
+                   "1:\n\t"
+                   "movw $0x3b,%dx\n\t"            /* GSEL(GUDATA_SEL, SEL_UPL) */
+                   "movw %dx,%ss\n"
+                   "movw %dx,0x90(%rcx)\n\t"       /* Fix frame->ss for iretq */
 #endif
                    "movl 0xb4(%rcx),%edx\n\t"      /* frame->restore_flags */
                    "testl $0x48,%edx\n\t"          /* CONTEXT_FLOATING_POINT | CONTEXT_XSTATE */
@@ -3417,7 +3535,7 @@ __ASM_GLOBAL_FUNC( __wine_syscall_dispatcher_instrumen
 
 
 /***********************************************************************
- *           __wine_unix_call_dispatcher
+ * __wine_unix_call_dispatcher
  */
 __ASM_GLOBAL_FUNC( __wine_unix_call_dispatcher,
                    "movq %rcx,%r10\n\t"
@@ -3489,7 +3607,25 @@ __ASM_GLOBAL_FUNC( __wine_unix_call_dispatcher,
                    "movq 0x320(%r13),%rdi\n\t"     /* amd64_thread_data()->pthread_teb */
                    "xorl %esi,%esi\n\t"
                    "movl $0x3000003,%eax\n\t"      /* _thread_set_tsd_base */
+                   "syscall\n\t"
+#elif defined(__FreeBSD__)
+                   /* unix call dispatcher entry: restore pthread fsbase and WOW32 %%fs. */
+                   "movw 0x338(%r13),%ax\n\t"      /* amd64_thread_data()->fs */
+                   "testw %ax,%ax\n\t"
+                   "jz 2f\n\t"
+                   "movq 0x320(%r13),%rsi\n\t"     /* amd64_thread_data()->pthread_teb */
+                   "cmpb $0,0x7ffe028a\n\t"        /* user_shared_data->ProcessorFeatures[PF_RDWRFSGSBASE_AVAILABLE] */
+                   "jz 1f\n\t"
+                   "wrfsbase %rsi\n\t"
+                   "jmp 2f\n"
+                   "1:\n\t"
+                   "leaq 0x320(%r13),%rsi\n\t"     /* sysarch requires a pointer to the value */
+                   "movq %rdx,%r9\n\t"             /* FreeBSD sysarch clobbers %rdx, backup to %r9 */
+                   "movq $0xa5,%rax\n\t"           /* sysarch */
+                   "movq $0x81,%rdi\n\t"           /* AMD64_SET_FSBASE */
                    "syscall\n\t"
+                   "movq %r9,%rdx\n\t"             /* Restore unix function index */
+                   "2:\n\t"
 #endif
                    "ldmxcsr 0x33c(%r13)\n\t"       /* amd64_thread_data()->mxcsr */
                    "movq %r8,%rdi\n\t"             /* args */
@@ -3528,6 +3664,18 @@ __ASM_GLOBAL_FUNC( __wine_unix_call_dispatcher,
                    "movq %r14,%rcx\n\t"
                    "movq %rdx,%rax\n\t"
                    "movq 0x60(%rcx),%r14\n\t"
+#elif defined(__FreeBSD__)
+                   /* unix call dispatcher return: restore WOW32 %%fs and fix %%ss after
+                    * sysret. %%rdx is volatile across Unix calls in the Windows ABI,
+                    * so it is 100% safe to use %%dx as a scratch register here. */
+                   "movw 0x338(%r13),%dx\n\t"      /* amd64_thread_data()->fs */
+                   "testw %dx,%dx\n\t"
+                   "jz 1f\n\t"
+                   "movw %dx,%fs\n\t"
+                   "1:\n\t"
+                   "movw $0x3b,%dx\n\t"            /* GSEL(GUDATA_SEL, SEL_UPL) */
+                   "movw %dx,%ss\n"
+                   "movw %dx,0x90(%rcx)\n\t"       /* Fix frame->ss for iretq */
 #endif
                    "movq 0x58(%rcx),%r13\n\t"
                    "movq 0x28(%rcx),%rdi\n\t"

--- dlls/ntdll/unix/signal_x86_64.c.orig	2026-03-20 13:33:36.000000000 -0700
+++ dlls/ntdll/unix/signal_x86_64.c	2026-03-26 05:17:32.763065000 -0700
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
+        if (act.sa_handler != SIG_DFL && act.sa_handler != SIG_IGN)
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
 
@@ -2944,6 +3028,9 @@ void signal_init_process(void)
 #if defined(__APPLE__) || defined(PR_SET_SYSCALL_USER_DISPATCH)
     sig_act.sa_sigaction = sigsys_handler;
     if (sigaction( SIGSYS, &sig_act, NULL ) == -1) goto error;
+#endif
+#ifdef __FreeBSD__
+    if (wrap_libthr_signal_handlers() == -1) goto error;
 #endif
     return;
 
@@ -2981,6 +3068,7 @@ void init_syscall_frame( LPTHREAD_START_ROUTINE entry,
 #endif
 #elif defined (__FreeBSD__) || defined (__FreeBSD_kernel__)
     amd64_set_gsbase( teb );
+    amd64_get_fsbase(&thread_data->pthread_teb);
 #elif defined(__NetBSD__)
     sysarch( X86_64_SET_GSBASE, &teb );
 #elif defined (__APPLE__)
@@ -3199,8 +3287,25 @@ __ASM_GLOBAL_FUNC( __wine_syscall_dispatcher,
                    "movq 0x320(%r13),%rdi\n\t"     /* amd64_thread_data()->pthread_teb */
                    "xorl %esi,%esi\n\t"
                    "movl $0x3000003,%eax\n\t"      /* _thread_set_tsd_base */
+                   "syscall\n\t"
+                   "leaq -0x98(%rbp),%rcx\n"
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
                    "syscall\n\t"
                    "leaq -0x98(%rbp),%rcx\n"
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
@@ -3489,7 +3607,23 @@ __ASM_GLOBAL_FUNC( __wine_unix_call_dispatcher,
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
+                   "movq $0xa5,%rax\n\t"           /* sysarch */
+                   "movq $0x81,%rdi\n\t"           /* AMD64_SET_FSBASE */
                    "syscall\n\t"
+                   "2:\n\t"
 #endif
                    "ldmxcsr 0x33c(%r13)\n\t"       /* amd64_thread_data()->mxcsr */
                    "movq %r8,%rdi\n\t"             /* args */
@@ -3528,6 +3662,18 @@ __ASM_GLOBAL_FUNC( __wine_unix_call_dispatcher,
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

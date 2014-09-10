/****************************************************************************
* Copyright � Udayanga Wickramasinghe - Indiana University                 *
*                                                                          *
****************************************************************************/

#if !defined(atomicsync_thread_primitives_h )
#define atomicsync_thread_primitives_h 1



namespace atomiccontrols {
    typedef int atomic_mutex_t;
    typedef int atomic_cond_t;

#define ATOMIC_SYNC_MUTEX_INITIALIZER  0
#define ATOMIC_SYNC_COND_INITIALIZER  0

    class AtomicSync {
        atomic_cond_t gl_mt;

        void _gl_lock(atomic_mutex_t *mutex) {
            while (!__sync_bool_compare_and_swap(mutex, 0, 1)) {
            };
        }

        void _gl_unlock(atomic_mutex_t *mutex) {
            //    wait until mutex = 0;
            *mutex = 0;
        }

    public:
        AtomicSync():gl_mt(0) {
        };

        void set_mutex_lock(atomic_mutex_t *mutex) {
            //wait until mutex = 0;
            while (true) {
                _gl_lock(&gl_mt);
//                printf("lock...PID: %d ..\n", getpid()) ;
                //wait until mutex becomes 0
                if (__sync_bool_compare_and_swap(mutex, 0, 1)) {
                    _gl_unlock(&gl_mt);
                    break;
                }
                _gl_unlock(&gl_mt);
            };
        }

        void set_mutex_unlock(atomic_mutex_t *mutex) {
            _gl_lock(&gl_mt);
            *mutex = 0;
            _gl_unlock(&gl_mt);
        }

        void set_cond_wait(atomic_cond_t *cond, atomic_mutex_t *mutex) {
            bool can_unlock = true;
            while (true) {
                _gl_lock(&gl_mt);
//                printf("[ATOMIC SYNC WAIT..... ]\n");
                //cond == 1 and mutex = 0;
                bool cond_for_exit = (*cond >= 1) && (*mutex == 0);
                if (!cond_for_exit) {
                    //wait
                    if (*mutex == 1 && can_unlock) {
                        //give other thread a go
                        *mutex = 0;
                        can_unlock = false;
                    }
//                    printf("wait...PID: %d ..\n", getpid());
                    _gl_unlock(&gl_mt);
                    continue;
                }
                //condition to exit is met
//                *cond = 0;
                *cond = *cond - 1;
                *mutex = 1;
                can_unlock = false;
                _gl_unlock(&gl_mt);
                break;
            };
        }

        void set_cond_signal(atomic_cond_t *cond) {
            _gl_lock(&gl_mt);
            *cond = *cond + 1;
            _gl_unlock(&gl_mt);
        }


    };

}
#endif /* integer_addition_h */

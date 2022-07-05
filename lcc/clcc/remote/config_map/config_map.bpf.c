#include "lbc.h"

#define TASK_COMM_LEN 16
struct data_t {
    u32 c_pid;
    u32 p_pid;
    char c_comm[TASK_COMM_LEN];
    char p_comm[TASK_COMM_LEN];
};

LBC_PERF_OUTPUT(e_out, struct data_t, 128);
LBC_HASH(user_config, u32, u32, 1024);

SEC("kprobe/wake_up_new_task")
int j_wake_up_new_task(struct pt_regs *ctx)
{
    struct task_struct* parent = (struct task_struct *)PT_REGS_PARM1(ctx);
    u32 pid = BPF_CORE_READ(parent, pid);
    u32 key = 0;
    u32 *pidp;

    pidp =  bpf_map_lookup_elem(&user_config, &key);
    if (pidp) {
        if (*pidp == pid) {
            struct data_t data = {};
            data.c_pid = bpf_get_current_pid_tgid() >> 32;
            bpf_get_current_comm(&data.c_comm, TASK_COMM_LEN);
            data.p_pid = BPF_CORE_READ(parent, pid);
            bpf_core_read(&data.p_comm[0], TASK_COMM_LEN, &parent->comm[0]);
            bpf_perf_event_output(ctx, &e_out, BPF_F_CURRENT_CPU, &data, sizeof(data));
        }
    }
        return 0;
}

char _license[] SEC("license") = "GPL";

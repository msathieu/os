#include <bitset.h>
#include <cpu/paging.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/scheduler.h>

struct linked_list ipc_handling_processes;

void syscall_wait_ipc(union syscall_args* args) {
  if (current_task()->servicing_syscall_requester) {
    puts("Can't register handler while servicing syscall");
    terminate_current_task(&args->registers);
    return;
  }
  if (current_task()->process->syscall_handler) {
    puts("Already waiting for IPC call on different task");
    terminate_current_task(&args->registers);
    return;
  }
  if (current_task()->process->syscall_queue.first) {
    struct task* requester_task = current_task()->process->syscall_queue.first->node;
    args->syscall = requester_task->registers.rdi & 255;
    args->arg0 = requester_task->registers.rsi;
    args->arg1 = requester_task->registers.rdx;
    args->arg2 = requester_task->registers.rcx;
    args->arg4 = requester_task->registers.r9;
    uintptr_t virtual_address, mapping_start, mapping_end;
    if (args->syscall & IPC_CALL_MEMORY_SHARING) {
      mapping_start = requester_task->registers.r8 / 0x1000 * 0x1000;
      mapping_end = (requester_task->registers.r8 + args->arg4 + 0xfff) / 0x1000 * 0x1000;
      physical_mappings_process = current_task()->process;
      physical_mappings_task = current_task();
      virtual_address = get_free_ipc_range(mapping_end - mapping_start);
      physical_mappings_process = 0;
      physical_mappings_task = 0;
      if (!virtual_address) {
        puts("Out of memory reserved for physical mappings");
        terminate_current_task(&args->registers);
        return;
      }
    }
    current_task()->servicing_syscall_requester = requester_task;
    linked_list_remove(&current_task()->process->syscall_queue, &requester_task->state_list_member);
    if (args->syscall & IPC_CALL_MEMORY_SHARING) {
      current_task()->sharing_memory = true;
      args->arg3 = virtual_address + requester_task->registers.r8 % 0x1000;
      for (size_t i = 0; i < mapping_end - mapping_start; i += 0x1000) {
        create_mapping(virtual_address + i, convert_to_physical(mapping_start + i, requester_task->process->address_space), 0x1000, 1, args->syscall & IPC_CALL_MEMORY_SHARING_RW_MASK, 0);
      }
    } else {
      args->arg3 = requester_task->registers.r8;
    }
  } else {
    current_task()->process->syscall_handler = current_task();
    block_current_task(&args->registers);
  }
}
void syscall_handle_ipc(union syscall_args* args) {
  size_t pid = args->syscall >> 8;
  struct process* called_process = 0;
  for (struct linked_list_member* member = ipc_handling_processes.first; member; member = member->next) {
    struct process* process = member->node;
    if (process->pid == pid) {
      called_process = process;
      break;
    }
  }
  if (!called_process) {
    args->return_value = -IPC_ERR_INVALID_PID;
    return;
  }
  if (called_process == current_task()->process) {
    puts("Called process is the same as current process");
    terminate_current_task(&args->registers);
    return;
  }
  size_t syscall = args->syscall & 255;
  uintptr_t mapping_start, mapping_end;
  if (syscall & IPC_CALL_MEMORY_SHARING) {
    mapping_start = args->arg3 / 0x1000 * 0x1000;
    if (!args->arg4 || args->arg4 > 0x8000000) {
      puts("Can't share this much memory");
      terminate_current_task(&args->registers);
      return;
    }
    if (__builtin_uaddl_overflow(args->arg3, args->arg4 + 0xfff, &mapping_end)) {
      puts("Can't share this memory region");
      terminate_current_task(&args->registers);
      return;
    }
    mapping_end = mapping_end / 0x1000 * 0x1000;
    if (mapping_end >= PAGING_USER_PHYS_MAPPINGS_START) {
      puts("Can't share this memory region");
      terminate_current_task(&args->registers);
      return;
    }
    for (size_t i = mapping_start; i < mapping_end; i += 0x1000) {
      if (!is_page_mapped(i, syscall & IPC_CALL_MEMORY_SHARING_RW_MASK)) {
        puts("Requested page isn't mapped or doesn't have the required permissions");
        terminate_current_task(&args->registers);
        return;
      }
    }
  }
  if (!called_process->syscall_handler) {
    linked_list_insert(&called_process->syscall_queue, &current_task()->state_list_member, current_task());
    block_current_task(&args->registers);
    return;
  }
  uintptr_t virtual_address;
  if (syscall & IPC_CALL_MEMORY_SHARING) {
    physical_mappings_process = called_process;
    physical_mappings_task = called_process->syscall_handler;
    virtual_address = get_free_ipc_range(mapping_end - mapping_start);
    physical_mappings_process = 0;
    physical_mappings_task = 0;
    if (!virtual_address) {
      puts("Out of memory reserved for physical mappings");
      terminate_current_task(&args->registers);
      return;
    }
  }
  current_task()->blocked = true;
  called_process->syscall_handler->servicing_syscall_requester = current_task();
  uint64_t arg0 = args->arg0;
  uint64_t arg1 = args->arg1;
  uint64_t arg2 = args->arg2;
  uint64_t arg3 = args->arg3;
  uint64_t arg4 = args->arg4;
  called_process->syscall_handler->blocked = false;
  switch_task(called_process->syscall_handler, &args->registers);
  called_process->syscall_handler = 0;
  args->syscall = syscall;
  args->arg0 = arg0;
  args->arg1 = arg1;
  args->arg2 = arg2;
  args->arg4 = arg4;
  if (syscall & IPC_CALL_MEMORY_SHARING) {
    current_task()->sharing_memory = true;
    args->arg3 = virtual_address + arg3 % 0x1000;
    for (size_t i = 0; i < mapping_end - mapping_start; i += 0x1000) {
      create_mapping(virtual_address + i, convert_to_physical(mapping_start + i, current_task()->servicing_syscall_requester->process->address_space), 0x1000, 1, syscall & IPC_CALL_MEMORY_SHARING_RW_MASK, 0);
    }
  } else {
    args->arg3 = arg3;
  }
}
void syscall_return_ipc(union syscall_args* args) {
  if (current_task()->sharing_memory) {
    for (size_t i = 0; i < PAGING_PHYSICAL_MAPPINGS_SIZE / 0x1000 / 64; i++) {
      if (!current_task()->mappings_bitset[i]) {
        continue;
      }
      for (size_t j = 0; j < 64; j++) {
        if (bitset_test(current_task()->mappings_bitset, i * 64 + j)) {
          unmap_page(PAGING_USER_PHYS_MAPPINGS_START + (i * 64 + j) * 0x1000);
          bitset_clear(current_task()->process->mappings_bitset, i * 64 + j);
          bitset_clear(current_task()->mappings_bitset, i * 64 + j);
        }
      }
    }
    current_task()->sharing_memory = false;
  }
  struct task* requester = current_task()->servicing_syscall_requester;
  if (!requester) {
    puts("No IPC call to return from");
    terminate_current_task(&args->registers);
    return;
  }
  current_task()->servicing_syscall_requester = false;
  requester->blocked = false;
  requester->registers.rax = args->arg0;
  schedule_task(requester, &args->registers);
}
void syscall_block_ipc_call(union syscall_args* args) {
  struct task* requester = current_task()->servicing_syscall_requester;
  if (!requester) {
    puts("Not currently handling IPC call");
    terminate_current_task(&args->registers);
    return;
  }
  if (current_task()->sharing_memory) {
    for (size_t i = 0; i < PAGING_PHYSICAL_MAPPINGS_SIZE / 0x1000 / 64; i++) {
      if (!current_task()->mappings_bitset[i]) {
        continue;
      }
      for (size_t j = 0; j < 64; j++) {
        if (bitset_test(current_task()->mappings_bitset, i * 64 + j)) {
          unmap_page(PAGING_USER_PHYS_MAPPINGS_START + (i * 64 + j) * 0x1000);
          bitset_clear(current_task()->process->mappings_bitset, i * 64 + j);
          bitset_clear(current_task()->mappings_bitset, i * 64 + j);
        }
      }
    }
    current_task()->sharing_memory = false;
  }
  linked_list_insert(&current_task()->process->blocked_ipc_calls_queue, &requester->state_list_member, requester);
  current_task()->servicing_syscall_requester = false;
}
void syscall_unblock_ipc_call(union syscall_args* args) {
  struct task* syscall_requester = 0;
  for (struct linked_list_member* member = current_task()->process->blocked_ipc_calls_queue.first; member; member = member->next) {
    struct task* task = member->node;
    if (!args->arg0 || task->process->pid == args->arg0) {
      syscall_requester = task;
      break;
    }
  }
  if (!syscall_requester) {
    puts("No IPC call to unblock with specified PID");
    terminate_current_task(&args->registers);
    return;
  }
  struct task* syscall_handler = current_task()->process->syscall_handler;
  if (!syscall_handler) {
    linked_list_remove(&current_task()->process->blocked_ipc_calls_queue, &syscall_requester->state_list_member);
    linked_list_insert(&current_task()->process->syscall_queue, &syscall_requester->state_list_member, syscall_requester);
    return;
  }
  uintptr_t virtual_address, mapping_start, mapping_end;
  if (syscall_requester->registers.rdi & IPC_CALL_MEMORY_SHARING) {
    mapping_start = syscall_requester->registers.r8 / 0x1000 * 0x1000;
    mapping_end = (syscall_requester->registers.r8 + syscall_requester->registers.r9 + 0xfff) / 0x1000 * 0x1000;
    physical_mappings_process = syscall_handler->process;
    physical_mappings_task = syscall_handler;
    virtual_address = get_free_ipc_range(mapping_end - mapping_start);
    physical_mappings_process = 0;
    physical_mappings_task = 0;
    if (!virtual_address) {
      puts("Out of memory reserved for physical mappings");
      terminate_current_task(&args->registers);
      return;
    }
  }
  current_task()->process->syscall_handler = 0;
  syscall_handler->servicing_syscall_requester = syscall_requester;
  syscall_handler->blocked = false;
  syscall_handler->registers.rdi = syscall_requester->registers.rdi & 255;
  syscall_handler->registers.rsi = syscall_requester->registers.rsi;
  syscall_handler->registers.rdx = syscall_requester->registers.rdx;
  syscall_handler->registers.rcx = syscall_requester->registers.rcx;
  syscall_handler->registers.r9 = syscall_requester->registers.r9;
  if (syscall_requester->registers.rdi & IPC_CALL_MEMORY_SHARING) {
    syscall_handler->sharing_memory = true;
    syscall_handler->registers.r8 = virtual_address + syscall_requester->registers.r8 % 0x1000;
    for (size_t i = 0; i < mapping_end - mapping_start; i += 0x1000) {
      create_mapping(virtual_address + i, convert_to_physical(mapping_start + i, syscall_requester->process->address_space), 0x1000, 1, syscall_requester->registers.rdi & IPC_CALL_MEMORY_SHARING_RW_MASK, 0);
    }
  } else {
    syscall_handler->registers.r8 = syscall_requester->registers.r8;
  }
  linked_list_remove(&current_task()->process->blocked_ipc_calls_queue, &syscall_requester->state_list_member);
  schedule_task(syscall_handler, &args->registers);
}

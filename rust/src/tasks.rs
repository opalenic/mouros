#![allow(dead_code,
         non_camel_case_types,
         non_upper_case_globals,
         non_snake_case)]

pub enum Task {}

extern "C" {
    pub static mut os_is_initialized: u8;
}

extern "C" {
    pub fn os_task_yield();
    pub fn os_task_suspend_self();
    pub fn os_task_unsuspend(task: *mut Task) -> u8;
    pub fn os_task_sleep(num_ticks: u32);
}

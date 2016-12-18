#![allow(dead_code,
         non_camel_case_types,
         non_upper_case_globals,
         non_snake_case)]

pub enum Task {}

extern "C" {
    pub static mut os_is_initialized: u8;
}

extern "C" {
    fn os_task_yield();
    fn os_task_suspend_self();
    fn os_task_unsuspend(task: *mut Task) -> u8;
    fn os_task_sleep(num_ticks: u32);
}

pub fn yield_cpu() {
    unsafe { os_task_yield() };
}

pub fn suspend_self() {
    unsafe { os_task_suspend_self() };
}

pub fn unsuspend(task: *mut Task) -> u8 {
    unsafe { os_task_unsuspend(task) }
}

pub fn sleep(num_ticks: u32) {
    unsafe { os_task_sleep(num_ticks) }
}


pub struct CriticalSection(u32);

unsafe fn mask_interrupts(mask: u32) -> u32 {
    let old: u32;

    asm!("mrs $0, PRIMASK" : "=r" (old) ::: "volatile");
    asm!("" ::: "memory": "volatile");
    asm!("msr PRIMASK, $0" :: "r" (mask) :: "volatile");

    old
}

impl CriticalSection {
    pub fn start() -> CriticalSection {
        CriticalSection(unsafe { mask_interrupts(1) })
    }
}

impl Drop for CriticalSection {
    fn drop(&mut self) {
        unsafe {
            mask_interrupts(self.0);
        }
    }
}

#[macro_export]
macro_rules! critical {
    ($code:block) => ({
        let __mouros_critical_section = $crate::tasks::CriticalSection::start();

        $code
    })
}

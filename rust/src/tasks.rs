#![allow(dead_code,
         non_camel_case_types,
         non_upper_case_globals,
         non_snake_case)]

use core::cell::UnsafeCell;
use core::ops::{Deref, DerefMut};

pub enum Task {}

extern "C" {
    pub static mut os_is_initialized: u8;
}

extern "C" {
    fn os_task_yield();
    fn os_task_suspend_self();
    fn os_task_unsuspend(task: *mut Task) -> u8;
    fn os_task_sleep(num_ticks: u32);
    fn os_task_wait_us(wait_time_us: u64);
    fn os_get_tick_count() -> u64;
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

pub fn wait_us(wait_time_us: u64) {
    unsafe { os_task_wait_us(wait_time_us) }
}

pub fn get_tick_count() -> u64 {
    unsafe { os_get_tick_count() }
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
    pub fn new() -> CriticalSection {
        unsafe {
            if let Some(led_func) = super::diag_led_function {
                led_func(true);
            }
        }
        CriticalSection(unsafe { mask_interrupts(1) })
    }
}

impl Drop for CriticalSection {
    fn drop(&mut self) {
        unsafe {
            if let Some(led_func) = super::diag_led_function {
                led_func(false);
            }
        }
        unsafe {
            mask_interrupts(self.0);
        }
    }
}

#[macro_export]
macro_rules! critical {
    ($code:block) => ({
        let __mouros_critical_section = $crate::tasks::CriticalSection::new();

        $code
    })
}



pub struct CriticalLock<T> {
    data: UnsafeCell<T>,
}

pub struct CriticalLockGuard<'lock, T: 'lock> {
    lock: &'lock CriticalLock<T>,
    crit_section: CriticalSection,
}


impl<T> CriticalLock<T> {
    pub const fn new(item: T) -> CriticalLock<T> {
        CriticalLock { data: UnsafeCell::new(item) }
    }

    pub fn lock(&self) -> CriticalLockGuard<T> {
        CriticalLockGuard {
            lock: &self,
            crit_section: CriticalSection::new(),
        }
    }
}

impl<'lock, T> Deref for CriticalLockGuard<'lock, T> {
    type Target = T;

    fn deref(&self) -> &T {
        unsafe { &*self.lock.data.get() }
    }
}

impl<'lock, T> DerefMut for CriticalLockGuard<'lock, T> {
    fn deref_mut(&mut self) -> &mut T {
        unsafe { &mut *self.lock.data.get() }
    }
}


unsafe impl<T> Send for CriticalLock<T> {}
unsafe impl<T> Sync for CriticalLock<T> {}

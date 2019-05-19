#![no_std]
#![feature(asm)]
#![feature(const_fn)]

// TODO Remove this once the libc crate becomes buildable on arm-none-eabi with no_std.
#[repr(u8)]
pub enum CVoid {
    // Two dummy variants so the #[repr] attribute can be used.
    #[doc(hidden)]
    __Variant1,
    #[doc(hidden)]
    __Variant2,
}

pub mod mailbox;
pub mod tasks;
pub mod pool_alloc;

pub static mut diag_led_function: Option<fn(bool)> = None;
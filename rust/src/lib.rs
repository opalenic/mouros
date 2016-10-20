#![no_std]

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
#![allow(dead_code)]

use core::mem;
use core::marker;
use super::CVoid;

#[repr(C)]
#[derive(Debug)]
pub struct MailboxRaw {
    msg_buf: *mut u8,
    msg_buf_len: u32,
    msg_size: u32,
    read_pos: u32,
    write_pos: u32,
    data_added: Option<extern "C" fn()>,
}

impl Default for MailboxRaw {
    fn default() -> Self {
        unsafe { mem::zeroed() }
    }
}

#[repr(C)]
#[derive(Debug)]
pub struct Mailbox<T> {
    mb: MailboxRaw,
    data_type_marker: marker::PhantomData<T>,
}

impl<T> Default for Mailbox<T> {
    fn default() -> Self {
        unsafe { mem::zeroed() }
    }
}


#[link (name = "mouros")]
extern "C" {
    fn os_mailbox_init(mb: *mut MailboxRaw,
                       msg_buf: *mut CVoid,
                       msg_buf_len: u32,
                       msg_size: u32,
                       data_added_callback: Option<extern "C" fn()>);

    fn os_mailbox_write(mb: *mut MailboxRaw, msg: *const CVoid) -> u8;

    fn os_mailbox_write_multiple(mb: *mut MailboxRaw, msgs: *const CVoid, msg_num: u32) -> u32;

    fn os_mailbox_read(mb: *mut MailboxRaw, out: *mut CVoid) -> u8;

    fn os_mailbox_read_multiple(mb: *mut MailboxRaw, out: *mut CVoid, out_msg_num: u32) -> u32;
}

impl<T> Mailbox<T> {
    pub fn new(buf: &mut [T]) -> Mailbox<T> {
        let mut mb = Mailbox::default();

        unsafe {
            os_mailbox_init(&mut mb.mb,
                            buf.as_mut_ptr() as *mut CVoid,
                            buf.len() as u32,
                            mem::size_of::<T>() as u32,
                            None);
        }

        mb
    }

    pub fn write(&mut self, msg: T) -> bool {
        unsafe { os_mailbox_write(&mut self.mb, &msg as *const T as *const CVoid) != 0 }
    }

    pub fn write_multiple(&mut self, msgs: &[T]) -> u32 {
        unsafe {
            os_mailbox_write_multiple(&mut self.mb,
                                      msgs.as_ptr() as *const CVoid,
                                      msgs.len() as u32)
        }
    }

    pub fn read(&mut self) -> Option<T> {
        unsafe {
            let mut tmp = mem::zeroed();

            if os_mailbox_read(&mut self.mb, &mut tmp as *mut T as *mut CVoid) != 0 {
                Some(tmp)
            } else {
                None
            }
        }
    }

    pub fn read_multiple(&mut self, out_buf: &mut [T]) -> u32 {
        unsafe {
            os_mailbox_read_multiple(&mut self.mb,
                                     out_buf.as_mut_ptr() as *mut CVoid,
                                     out_buf.len() as u32)
        }
    }

    pub unsafe fn get_raw_mb_struct(&mut self) -> *mut MailboxRaw {
        &mut self.mb
    }
}

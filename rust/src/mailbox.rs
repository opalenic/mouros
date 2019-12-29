#![allow(dead_code)]

use core::mem;
use core::cell::UnsafeCell;
use core::marker::PhantomData;
use core::mem::MaybeUninit;
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
pub struct Mailbox<'mem, T> {
    mb: UnsafeCell<MailboxRaw>,
    _data: PhantomData<&'mem mut [T]>,
}

impl<'mem, T> Default for Mailbox<'mem, T> {
    fn default() -> Self {
        unsafe { mem::zeroed() }
    }
}


#[link(name = "mouros")]
extern "C" {
    fn os_mailbox_init(
        mb: *mut MailboxRaw,
        msg_buf: *mut CVoid,
        msg_buf_len: u32,
        msg_size: u32,
        data_added_callback: Option<extern "C" fn()>,
    );

    fn os_mailbox_write(mb: *mut MailboxRaw, msg: *const CVoid) -> u8;

    fn os_mailbox_write_multiple(mb: *mut MailboxRaw, msgs: *const CVoid, msg_num: u32) -> u32;

    fn os_mailbox_read(mb: *mut MailboxRaw, out: *mut CVoid) -> u8;

    fn os_mailbox_read_multiple(mb: *mut MailboxRaw, out: *mut CVoid, out_msg_num: u32) -> u32;
}

impl<'mem, T> Mailbox<'mem, T>
{
    pub fn new(buf: &'mem mut [T]) -> Mailbox<'_, T> {
        let mb = Mailbox::default();

        unsafe {
            os_mailbox_init(
                mb.mb.get(),
                buf.as_mut_ptr() as *mut CVoid,
                buf.len() as u32,
                mem::size_of::<T>() as u32,
                None,
            );
        }

        mb
    }

    pub fn write(&self, msg: T) -> bool {
        unsafe { os_mailbox_write(self.mb.get(), &msg as *const T as *const CVoid) != 0 }
    }

    pub fn write_multiple(&self, msgs: &[T]) -> u32 {
        unsafe {
            os_mailbox_write_multiple(
                self.mb.get(),
                msgs.as_ptr() as *const CVoid,
                msgs.len() as u32,
            )
        }
    }

    pub fn read(&self) -> Option<T> {
        unsafe {
            let mut tmp = mem::zeroed();

            if os_mailbox_read(self.mb.get(), &mut tmp as *mut T as *mut CVoid) != 0 {
                Some(tmp)
            } else {
                None
            }
        }
    }

    pub fn read_multiple(&self, out_buf: &mut [T]) -> u32 {
        unsafe {
            os_mailbox_read_multiple(
                self.mb.get(),
                out_buf.as_mut_ptr() as *mut CVoid,
                out_buf.len() as u32,
            )
        }
    }

    pub unsafe fn get_raw_mailbox(&self) -> *mut MailboxRaw {
        self.mb.get()
    }
}


pub struct SendError<T>(pub T);
pub struct RecvError;

pub struct RxChannelSpsc<'mb, 'mem: 'mb, T: 'mem> {
    mb: UnsafeCell<&'mb mut MailboxRaw>,
    _lifetime_marker: PhantomData<&'mb mut Mailbox<'mem, T>>,
}

impl<'mb, 'mem, T> RxChannelSpsc<'mb, 'mem, T> {
    pub fn recv(&self) -> T {
        unsafe {
            let mut tmp = MaybeUninit::<T>::uninit();

            while os_mailbox_read(*self.mb.get(), tmp.as_mut_ptr() as *mut CVoid) != 0 {}

            tmp.assume_init()
        }
    }

    pub fn try_recv(&self) -> Result<T, RecvError> {
        unsafe {
            let mut tmp = MaybeUninit::<T>::uninit();

            if os_mailbox_read(*self.mb.get(), tmp.as_mut_ptr() as *mut CVoid) != 0 {
                Ok(tmp.assume_init())
            } else {
                Err(RecvError)
            }
        }
    }

    pub unsafe fn get_raw_mailbox(&self) -> *mut MailboxRaw {
        *self.mb.get()
    }
}


pub struct TxChannelSpsc<'mb, 'mem: 'mb, T: 'mem> {
    mb: UnsafeCell<&'mb mut MailboxRaw>,
    _lifetime_marker: PhantomData<&'mb mut Mailbox<'mem, T>>,
}

impl<'mb, 'mem, T> TxChannelSpsc<'mb, 'mem, T> {
    pub fn send(&self, msg: T) {
        unsafe {
           while os_mailbox_write(*self.mb.get(), &msg as *const T as *const CVoid) != 0 {}
        }
     }

    pub fn try_send(&self, msg: T) -> Result<(), SendError<T>> {
        unsafe {
            if os_mailbox_write(*self.mb.get(), &msg as *const T as *const CVoid) != 0 {
                Ok(())
            } else {
                Err(SendError(msg))
            }
        }
    }

    pub unsafe fn get_raw_mailbox(&self) -> *mut MailboxRaw {
        *self.mb.get()
    }
}


pub fn channel_spsc<'mb, 'mem, T>(
    mailbox: &'mb mut Mailbox<'mem, T>,
) -> (RxChannelSpsc<'mb, 'mem, T>, TxChannelSpsc<'mb, 'mem, T>) {
    unsafe {
        (
            RxChannelSpsc {
                mb: UnsafeCell::new(&mut *mailbox.get_raw_mailbox()),
                _lifetime_marker: PhantomData,
            },
            TxChannelSpsc {
                mb: UnsafeCell::new(&mut *mailbox.get_raw_mailbox()),
                _lifetime_marker: PhantomData,
            },
        )
    }
}


use core::ptr;
use core::marker::PhantomData;

pub struct Pool<'a, T: 'a> {
    first_block: *mut T,
    _data: PhantomData<&'a mut [T]>,
}

impl<'a, T> Pool<'a, T> {
    pub fn new(backing_mem: &'a mut [T]) -> Pool<'a, T> {

        for pos in 0..(backing_mem.len() - 1) {
            let start_of_current = &mut backing_mem[pos] as *mut T as *mut usize;
            let start_of_next = &mut backing_mem[pos + 1] as *mut T as usize;

            unsafe {
                ptr::write_unaligned(start_of_current, start_of_next);
            }
        }

        let last = backing_mem.len() - 1;
        unsafe {
            ptr::write_unaligned(&mut backing_mem[last] as *mut T as *mut usize, 0usize);
        }

        Pool {
            first_block: &mut backing_mem[0],
            _data: PhantomData,
        }
    }

    pub fn take(&mut self) -> Option<&'a mut T> {
        if !self.first_block.is_null() {
            let ret = self.first_block;

            unsafe {
                self.first_block = *(self.first_block as *const usize) as *mut T;
                Some(&mut *ret)
            }
        } else {
            None
        }
    }

    pub fn give(&mut self, block: &'a mut T) {
        let second = self.first_block;

        self.first_block = block;
        unsafe {
            ptr::write_unaligned(self.first_block as *mut usize, second as usize);
        }
    }
}

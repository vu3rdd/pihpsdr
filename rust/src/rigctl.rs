use std::os::fd::FromRawFd;
use std::{fs::File, io::Write};
use std::slice;
use libc::{write, c_int, c_char};

#[no_mangle]
pub extern "C" fn send_resp(fd: c_int, msg: *const c_char) -> () {
    let mut f: File = unsafe { File::from_raw_fd(fd) };
    let cstr = unsafe { std::ffi::CStr::from_ptr(msg) };
    let buf: &[u8] = cstr.to_bytes();
    let _ = f.write_all(buf);
    ()
}

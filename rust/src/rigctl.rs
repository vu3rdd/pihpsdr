use std::os::fd::{FromRawFd, IntoRawFd};
use std::{fs::File, io::Write};
use std::slice;
use libc::{write, c_int, c_char};

#[no_mangle]
pub extern "C" fn send_resp(fd: c_int, msg: *const c_char) -> () {
    let mut f: File = unsafe { File::from_raw_fd(fd) };
    let cstr = unsafe { std::ffi::CStr::from_ptr(msg) };
    let buf: &[u8] = cstr.to_bytes();
    println!("send_resp: {}", std::str::from_utf8(buf).unwrap());
    let _ = f.write_all(buf);
    let _ = f.into_raw_fd(); // this is so that fd is not closed
    ()
}

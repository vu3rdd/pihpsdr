use libc::{size_t};

#[no_mangle]
pub extern fn add(left: size_t, right: size_t) -> size_t {
    left + right
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn it_works() {
        let result = add(2, 2);
        assert_eq!(result, 4);
    }
}

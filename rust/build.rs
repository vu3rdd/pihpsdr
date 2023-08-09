use std::env;
use std::path::Path;
use cbindgen::{Config, Builder};

fn main() {
let crate_dir = env::var("CARGO_MANIFEST_DIR").unwrap();
    let mut config: cbindgen::Config = Default::default();
    config.language = cbindgen::Language::C;

    cbindgen::Builder::new()
        .with_crate(crate_dir)
        .with_config(config)
        .generate()
        .expect("Unable to generate bindings")
        .write_to_file("target/debug/libhpsdr.h");
    println!("cargo:rerun-if-changed=libhpsdr.h");
}

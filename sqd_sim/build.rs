use std::{env, path::Path};

// Example custom build script.
fn main() {
    // Tell Cargo that if the given file changes, to rerun this build script.
    println!("cargo::rerun-if-changed=csrc");
    let ti_sdk_env =
        env::var("MSPM0_SDK_PATH").expect("Cannot compile without MSPM0 SDK accessible");
    let ti_sdk = Path::new(&ti_sdk_env);
    let crate_dir_env = env::var("CARGO_MANIFEST_DIR").unwrap();
    let crate_dir = Path::new(&crate_dir_env);
    cc::Build::new()
        .flag("-fshort-enums")
        .flag("-D__MSPM0G3507__")
        .include("cinclude")
        .include("cconfig")
        .include(ti_sdk.join("source/third_party/CMSIS/Core/Include"))
        .include(ti_sdk.join("kernel/freertos/Source/include"))
        .include(ti_sdk.join("source"))
        .include(ti_sdk.join("kernel/freertos/Source/portable/GCC/ARM_CM0"))
        .include(
            crate_dir
                .join("../freertos_builds_LP_MSPM0G3507_release_ticlang")
                .canonicalize()
                .expect("Project layout changed?"),
        )
        .static_flag(true)
        .file("csrc/main/main.c")
        .file("csrc/chess.c")
        .file("csrc/game.c")
        .compile("test");
}

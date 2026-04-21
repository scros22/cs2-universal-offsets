fn main() {
    #[cfg(windows)]
    {
        let mut res = winresource::WindowsResource::new();
        res.set_icon("assets/icon.ico");
        if let Err(e) = res.compile() {
            eprintln!("warning: failed to embed icon: {e}");
        }
        println!("cargo:rerun-if-changed=assets/icon.ico");
    }
}

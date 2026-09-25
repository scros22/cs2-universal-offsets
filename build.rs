fn main() {
    #[cfg(windows)]
    {
        let mut res = winresource::WindowsResource::new();
        res.set_icon("assets/icon.ico");
        // Explorer > Properties > Details
        res.set("FileDescription", "cs2-sdk - CS2 SDK generator");
        res.set("ProductName", "cs2-sdk");
        res.set("CompanyName", "cs2-sdk.com");
        res.set("LegalCopyright", "MIT License");
        res.set("OriginalFilename", "cs2-sdk.exe");
        res.set("InternalName", "cs2-sdk");
        if let Err(e) = res.compile() {
            eprintln!("warning: failed to embed icon: {e}");
        }
        println!("cargo:rerun-if-changed=assets/icon.ico");
    }
}

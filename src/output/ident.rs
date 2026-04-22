//! Shared helpers for the new SDK-style emitters.
//!
//! These are intentionally tiny and dependency-free so the new emitter
//! files (`sdk_classes`, `netvars`, `interfaces_sdk`, `amalgamation`)
//! can share identifier-sanitisation logic without going through the
//! older `output::Formatter` machinery.

/// Replace anything that isn't `[A-Za-z0-9_]` with `_`. Used for module
/// names, file names, and namespace identifiers.
pub fn slugify(input: &str) -> String {
    let mut s = String::with_capacity(input.len());
    for c in input.chars() {
        if c.is_ascii_alphanumeric() || c == '_' {
            s.push(c);
        } else {
            s.push('_');
        }
    }
    if s.chars().next().map(|c| c.is_ascii_digit()).unwrap_or(false) {
        s.insert(0, '_');
    }
    s
}

/// Like [`slugify`] but keeps the leading character if it's already a
/// valid C++ identifier start. Used for type names where we want
/// `C_CSPlayerPawn` to stay as `C_CSPlayerPawn`.
pub fn type_ident(input: &str) -> String {
    // Schema sometimes contains template-syntax names like
    // `CHandle< C_BaseEntity >`. For *type identifiers* in declarations
    // we strip everything from the first non-identifier character.
    let mut s = String::with_capacity(input.len());
    for c in input.chars() {
        if c.is_ascii_alphanumeric() || c == '_' {
            s.push(c);
        } else {
            break;
        }
    }
    if s.is_empty() {
        return "anonymous".to_string();
    }
    if s.chars().next().map(|c| c.is_ascii_digit()).unwrap_or(false) {
        s.insert(0, '_');
    }
    s
}

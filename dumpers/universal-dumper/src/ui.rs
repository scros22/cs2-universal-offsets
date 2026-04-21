//! Minimal, smooth terminal UI for the universal dumper.
//!
//! Everything prints on the default terminal background (black/dark) with
//! light text and a small set of accent colors.  Uses only ANSI escape
//! codes so there is no extra crate dependency and Windows 10+ conhost +
//! Windows Terminal both render it correctly.

use std::io::{self, Write};
use std::time::Duration;

// ---------------------------------------------------------------------------
// ANSI palette
// ---------------------------------------------------------------------------

pub const RESET: &str = "\x1b[0m";
pub const DIM: &str = "\x1b[2m";
pub const BOLD: &str = "\x1b[1m";

pub const FG_WHITE: &str = "\x1b[97m";
pub const FG_GRAY: &str = "\x1b[38;5;245m";
pub const FG_SOFT: &str = "\x1b[38;5;250m";
pub const FG_CYAN: &str = "\x1b[38;5;117m";
pub const FG_GREEN: &str = "\x1b[38;5;114m";
pub const FG_RED: &str = "\x1b[38;5;203m";
pub const FG_YELLOW: &str = "\x1b[38;5;222m";
pub const FG_MAG: &str = "\x1b[38;5;183m";

// smooth / rounded glyphs only
pub const BULLET: &str = "•";
pub const ARROW: &str = "›";
pub const DIAMOND: &str = "◆";
pub const CIRCLE: &str = "◉";
pub const CHECK: &str = "✓";
pub const CROSS: &str = "✗";
pub const WARN: &str = "!";
pub const INFO: &str = "i";
pub const HLINE: &str = "─";
pub const CORNER_TL: &str = "╭";
pub const CORNER_TR: &str = "╮";
pub const CORNER_BL: &str = "╰";
pub const CORNER_BR: &str = "╯";
pub const VLINE: &str = "│";

static mut NO_SOUND: bool = false;

/// Enable Windows 10+ virtual-terminal processing so ANSI codes render.
pub fn init(no_sound: bool) {
    unsafe {
        NO_SOUND = no_sound;
    }
    #[cfg(windows)]
    enable_vt();
}

#[cfg(windows)]
fn enable_vt() {
    use std::os::raw::c_void;
    type Dword = u32;
    type Handle = *mut c_void;
    const STD_OUTPUT_HANDLE: i32 = -11;
    const ENABLE_VIRTUAL_TERMINAL_PROCESSING: Dword = 0x0004;
    unsafe extern "system" {
        fn GetStdHandle(h: i32) -> Handle;
        fn GetConsoleMode(h: Handle, m: *mut Dword) -> i32;
        fn SetConsoleMode(h: Handle, m: Dword) -> i32;
        fn SetConsoleOutputCP(cp: u32) -> i32;
    }
    unsafe {
        SetConsoleOutputCP(65001); // UTF-8 for the smooth glyphs
        let h = GetStdHandle(STD_OUTPUT_HANDLE);
        if !h.is_null() {
            let mut mode: Dword = 0;
            if GetConsoleMode(h, &mut mode) != 0 {
                SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Sound
// ---------------------------------------------------------------------------

#[cfg(windows)]
fn beep(freq: u32, ms: u32) {
    unsafe extern "system" {
        fn Beep(freq: u32, dur: u32) -> i32;
    }
    unsafe {
        let _ = Beep(freq, ms);
    }
}

#[cfg(not(windows))]
fn beep(_: u32, _: u32) {}

pub enum Cue {
    Start,
    Step,
    Success,
    Failure,
}

pub fn sound(cue: Cue) {
    if unsafe { NO_SOUND } {
        return;
    }
    match cue {
        Cue::Start => {
            beep(660, 90);
            beep(880, 110);
            beep(1175, 140);
        }
        Cue::Step => beep(988, 55),
        Cue::Success => {
            beep(784, 100);
            beep(988, 100);
            beep(1318, 180);
            beep(1568, 220);
        }
        Cue::Failure => {
            beep(330, 160);
            beep(262, 220);
            beep(196, 280);
        }
    }
}

// ---------------------------------------------------------------------------
// Rendering helpers
// ---------------------------------------------------------------------------

fn flush() {
    let _ = io::stdout().flush();
}

pub fn banner() {
    // The ascii is intentionally light + curved-feeling (lots of round 0/o).
    let art = [
        "",
        "      ╭──────────────────────────────────────────────────────────────╮",
        "      │                                                              │",
        "      │     ██████╗ ███████╗██████╗      ███████╗██████╗ ██╗  ██╗    │",
        "      │    ██╔════╝ ██╔════╝╚════██╗     ██╔════╝██╔══██╗██║ ██╔╝    │",
        "      │    ██║      ███████╗ █████╔╝     ███████╗██║  ██║█████╔╝     │",
        "      │    ██║      ╚════██║██╔═══╝      ╚════██║██║  ██║██╔═██╗     │",
        "      │    ╚██████╗ ███████║███████╗     ███████║██████╔╝██║  ██╗    │",
        "      │     ╚═════╝ ╚══════╝╚══════╝     ╚══════╝╚═════╝ ╚═╝  ╚═╝    │",
        "      │                                                              │",
        "      │       c s 2   u n i v e r s a l   d u m p e r   v 1 . 0      │",
        "      │                                                              │",
        "      ╰──────────────────────────────────────────────────────────────╯",
        "",
    ];
    for l in art {
        println!("{FG_CYAN}{l}{RESET}");
    }
}

pub fn section(title: &str) {
    let title_u = title.to_string();
    let pad = 62usize.saturating_sub(title_u.chars().count() + 2);
    println!();
    println!(
        "  {FG_CYAN}{CORNER_TL}{line}{CORNER_TR}{RESET}",
        line = HLINE.repeat(64)
    );
    println!(
        "  {FG_CYAN}{VLINE}{RESET} {BOLD}{FG_WHITE}{title_u}{RESET}{pad_sp}{FG_CYAN}{VLINE}{RESET}",
        pad_sp = " ".repeat(pad)
    );
    println!(
        "  {FG_CYAN}{CORNER_BL}{line}{CORNER_BR}{RESET}",
        line = HLINE.repeat(64)
    );
}

pub fn kv(key: &str, value: &str) {
    println!(
        "    {FG_GRAY}{BULLET}{RESET} {FG_SOFT}{key:<18}{RESET} {FG_WHITE}{value}{RESET}"
    );
}

pub fn info(msg: &str) {
    println!("    {FG_CYAN}{ARROW}{RESET} {FG_SOFT}{msg}{RESET}");
}

pub fn ok(msg: &str) {
    println!("    {FG_GREEN}{CHECK}{RESET} {FG_WHITE}{msg}{RESET}");
}

pub fn warn(msg: &str) {
    println!("    {FG_YELLOW}{WARN}{RESET} {FG_WHITE}{msg}{RESET}");
}

pub fn err(msg: &str) {
    println!("    {FG_RED}{CROSS}{RESET} {FG_WHITE}{msg}{RESET}");
}

pub fn step(msg: &str) {
    println!("    {FG_MAG}{DIAMOND}{RESET} {BOLD}{FG_WHITE}{msg}{RESET}");
}

/// Redrawable progress line (carriage return, no newline).
pub fn progress(done: usize, total: usize, label: &str) {
    const BAR: usize = 34;
    let pct = if total == 0 {
        1.0
    } else {
        done as f32 / total as f32
    };
    let filled = (pct * BAR as f32).round() as usize;
    let filled = filled.min(BAR);
    let bar_done = "━".repeat(filled);
    let bar_rest = "─".repeat(BAR - filled);
    let label_short = trim_label(label, 28);
    print!(
        "\r    {FG_CYAN}{CIRCLE}{RESET} {FG_GREEN}{bar_done}{RESET}{DIM}{FG_GRAY}{bar_rest}{RESET} \
         {FG_WHITE}{pct:>5.1}%{RESET} {FG_SOFT}{done:>4}/{total:<4}{RESET} {FG_GRAY}{label_short}{RESET}   ",
        pct = pct * 100.0
    );
    flush();
}

pub fn progress_clear() {
    print!("\r{}\r", " ".repeat(110));
    flush();
}

fn trim_label(s: &str, max: usize) -> String {
    let count = s.chars().count();
    if count <= max {
        format!("{:<width$}", s, width = max)
    } else {
        let mut out: String = s.chars().take(max.saturating_sub(1)).collect();
        out.push('…');
        out
    }
}

pub fn found(name: &str, addr: u64, detail: &str) {
    progress_clear();
    println!(
        "    {FG_GREEN}{CHECK}{RESET} {FG_WHITE}{name:<44}{RESET} {FG_CYAN}{ARROW}{RESET} \
         {FG_YELLOW}0x{addr:016X}{RESET} {DIM}{FG_GRAY}{detail}{RESET}"
    );
}

pub fn not_found(name: &str, reason: &str) {
    progress_clear();
    println!(
        "    {FG_RED}{CROSS}{RESET} {FG_WHITE}{name:<44}{RESET} {DIM}{FG_GRAY}{reason}{RESET}"
    );
}

pub fn divider() {
    println!("  {DIM}{FG_GRAY}{}{RESET}", HLINE.repeat(66));
}

pub fn fmt_duration(d: Duration) -> String {
    let secs = d.as_secs_f64();
    if secs < 1.0 {
        format!("{:.0} ms", d.as_millis())
    } else if secs < 60.0 {
        format!("{:.2} s", secs)
    } else {
        format!("{:.0} m {:.0} s", secs / 60.0, secs % 60.0)
    }
}

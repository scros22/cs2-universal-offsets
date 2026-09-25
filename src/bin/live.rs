// cs2-live — non-intrusive live-memory reader for cs2.exe
// ---------------------------------------------------------------------------
// A companion to the dumper that keeps a memflow handle to the running game
// open and answers memory-read requests over a loopback HTTP endpoint. It uses
// the exact same read path the dumper uses every run (memflow_native ->
// ReadProcessMemory), so it NEVER suspends or freezes the game — the target
// keeps running normally while we read it.
//
// This exists for interactive reverse-engineering / signature work: follow a
// pointer chain, read a struct field, resolve where a vtable slot points, all
// without re-launching a one-shot dumper (and re-triggering UAC) per query.
//
// Launch it ONCE as administrator (see live.bat). It waits for cs2.exe, attaches,
// and listens on 127.0.0.1:13380. A non-admin caller (PowerShell, curl) hits the
// loopback endpoint — loopback binds never trigger the Windows firewall prompt,
// and connecting to an admin-owned loopback listener from a normal process is
// allowed.
//
// --- protocol --------------------------------------------------------------
//   GET  /health                         -> { ok, pid, modules:{name:hexbase}, ... }
//   GET  /bases                          -> module name -> base/size
//   GET  /read?e=<expr>&n=<count>&t=<ty> -> single read (percent-decoded; '+'
//                                           is LITERAL here, not a space)
//   POST /read  {"reads":[{e,n,t}, ...]} -> batch read (preferred from scripts)
//   GET  /scan?m=<module>&p=<ida pattern>&limit=N&ctx=M
//        -> live IDA-style byte-pattern scan (same algorithm as the dumper's
//           patterns::mod.rs), scoped to .text with .rdata/whole-image
//           fallback. Each hit reports rva/va/module-attribution plus M
//           bytes of context read fresh from the live process — exactly
//           what's needed to rebuild a needle for a sig that broke after a
//           game update, without re-running the elevated dumper or IDA.
//
// --- expression grammar (the useful part) ----------------------------------
//   expr   := term (('+' | '-') term)*
//   term   := '[' expr ']'        // dereference: read a u64 at (expr), continue
//           | '(' expr ')'
//           | module              // client, engine2, tier0, ...  -> module base
//           | number              // 0x.. hex or decimal
//   e.g.  client+0xB1FB80                          CreateMove code bytes
//         [client+0x23B95F0]+0x688                 nested (deref then +offset)
//         [[client+0x23B95F0]+0xB58]+0x18          two-level pointer chain
//
// --- types (t=) ------------------------------------------------------------
//   hex (default) | u8 u16 u32 u64 i8 i16 i32 i64 | f32 f64 | ptr | cstr | utf16 | vec3
//   ptr additionally attributes the pointed-to value to module+RVA.
//   Every result also reports which module the resolved address itself lands in.

use std::io::{BufRead, BufReader, Read, Write};
use std::net::{TcpListener, TcpStream};
use std::thread;
use std::time::Duration;

use anyhow::{Result, anyhow, bail};
use memflow::prelude::v1::*;
use pelite::pe64::Pe;
use serde_json::{Value, json};

const PORT: u16 = 13380;
const PROC_NAME: &str = "cs2.exe";

/// Modules we resolve up-front for name -> base and for pointer attribution.
const MODULES: &[&str] = &[
    "client.dll", "engine2.dll", "server.dll", "schemasystem.dll",
    "animationsystem.dll", "materialsystem2.dll", "particles.dll",
    "scenesystem.dll", "soundsystem.dll", "tier0.dll", "vphysics2.dll",
    "networksystem.dll", "host.dll", "panorama.dll", "rendersystemdx11.dll",
    "resourcesystem.dll", "vstdlib.dll", "pulse_system.dll", "inputsystem.dll",
    "filesystem_stdio.dll",
];

#[derive(Clone)]
struct Module {
    name: String,
    base: u64,
    size: u64,
}

/// Everything tied to one attached cs2.exe instance. Rebuilt on re-attach.
/// The process is the *owning* ArcBox (`Into…`) so it keeps its own Arc to the
/// OS alive and is `'static` — we can store it in the server and re-create it
/// without the OS borrow escaping.
struct Session {
    process: IntoProcessInstanceArcBox<'static>,
    modules: Vec<Module>,
    pid: u32,
    /// Lazily-populated full-image reads for /scan, keyed by lowercase
    /// module name without ".dll". Cleared on every re-attach.
    image_cache: std::collections::HashMap<String, ModuleImage>,
}

/// A module's image bytes plus its .text/.rdata section windows, read live
/// exactly like the dumper's `ModuleCache::load` — same RVA-aligned layout,
/// since the Windows loader maps PE sections at their virtual offsets.
struct ModuleImage {
    base: u64,
    bytes: Vec<u8>,
    text_rva: u32,
    text_size: u32,
    rdata_rva: u32,
    rdata_size: u32,
}

impl ModuleImage {
    #[inline]
    fn text(&self) -> &[u8] {
        let lo = self.text_rva as usize;
        let hi = (lo + self.text_size as usize).min(self.bytes.len());
        &self.bytes[lo.min(hi)..hi]
    }
    #[inline]
    fn rdata(&self) -> Option<&[u8]> {
        if self.rdata_size == 0 {
            return None;
        }
        let lo = self.rdata_rva as usize;
        let hi = (lo + self.rdata_size as usize).min(self.bytes.len());
        self.bytes.get(lo.min(hi)..hi)
    }
}

fn build_os() -> Result<OsInstanceArcBox<'static>> {
    Ok(memflow_native::create_os(&OsArgs::default(), LibArc::default())?)
}

fn load_modules(process: &mut IntoProcessInstanceArcBox<'static>) -> Vec<Module> {
    MODULES
        .iter()
        .filter_map(|name| {
            let m = process.module_by_name(name).ok()?;
            Some(Module {
                name: (*name).to_string(),
                base: m.base.to_umem() as u64,
                size: m.size as u64,
            })
        })
        .collect()
}

/// Attach (or re-attach) to cs2.exe. Blocks-retries until the process exists so
/// the server can be started before the game.
fn attach(os: &mut OsInstanceArcBox<'static>, quiet: bool) -> Session {
    loop {
        // Clone the (Arc-based) OS handle and consume the clone into an owning
        // process instance; `os` itself stays available for the next re-attach.
        match os.clone().into_process_by_name(PROC_NAME) {
            Ok(mut process) => {
                let pid = process.info().pid;
                let modules = load_modules(&mut process);
                if modules.is_empty() {
                    if !quiet {
                        eprintln!("[live] attached pid {pid} but no modules yet — waiting…");
                    }
                    thread::sleep(Duration::from_millis(500));
                    continue;
                }
                return Session { process, modules, pid, image_cache: std::collections::HashMap::new() };
            }
            Err(_) => {
                if !quiet {
                    eprintln!("[live] waiting for {PROC_NAME} …");
                }
                thread::sleep(Duration::from_secs(1));
            }
        }
    }
}

impl Session {
    fn module(&self, name: &str) -> Option<&Module> {
        let want = name.trim_end_matches(".dll").to_ascii_lowercase();
        self.modules
            .iter()
            .find(|m| m.name.trim_end_matches(".dll").eq_ignore_ascii_case(&want))
    }

    /// Which module (if any) the absolute address lands in, as "name+0xRVA".
    fn attribute(&self, addr: u64) -> Option<String> {
        self.modules
            .iter()
            .find(|m| addr >= m.base && addr < m.base + m.size)
            .map(|m| format!("{}+0x{:X}", m.name, addr - m.base))
    }

    fn read_u64(&mut self, a: u64) -> Option<u64> {
        self.process.read::<u64>(Address::from(a)).data_part().ok()
    }

    fn read_bytes(&mut self, a: u64, n: usize) -> Option<Vec<u8>> {
        let mut buf = vec![0u8; n];
        // Probe one byte first: if the base address is unmapped, report failure
        // rather than a buffer of zeros masquerading as data.
        self.process.read::<u8>(Address::from(a)).data_part().ok()?;
        let _ = self
            .process
            .read_raw_into(Address::from(a), &mut buf)
            .data_part();
        Some(buf)
    }

    /// Read the full live image of `module` once and cache it, along with its
    /// .text/.rdata windows — mirrors the dumper's `ModuleCache::load`.
    fn ensure_image(&mut self, module_name: &str) -> Result<()> {
        let key = module_name.trim_end_matches(".dll").to_ascii_lowercase();
        if self.image_cache.contains_key(&key) {
            return Ok(());
        }
        let m = self
            .module(module_name)
            .ok_or_else(|| anyhow!("unknown module '{module_name}'"))?
            .clone();
        let bytes = self
            .process
            .read_raw(Address::from(m.base), m.size as usize)
            .data_part()
            .map_err(|e| anyhow!("read_raw({module_name}) failed: {e:?}"))?;

        let (mut text_rva, mut text_size, mut rdata_rva, mut rdata_size) = (0u32, 0u32, 0u32, 0u32);
        if let Ok(view) = pelite::pe64::PeView::from_bytes(&bytes) {
            for section in view.section_headers() {
                match section.name().unwrap_or("") {
                    ".text" => {
                        text_rva = section.VirtualAddress;
                        text_size = section.VirtualSize;
                    }
                    ".rdata" => {
                        rdata_rva = section.VirtualAddress;
                        rdata_size = section.VirtualSize;
                    }
                    _ => {}
                }
            }
        }
        // No .text found (odd PE) — fall back to scanning the whole image.
        if text_size == 0 {
            text_rva = 0;
            text_size = bytes.len() as u32;
        }

        self.image_cache.insert(
            key,
            ModuleImage { base: m.base, bytes, text_rva, text_size, rdata_rva, rdata_size },
        );
        Ok(())
    }

    /// IDA-style pattern scan against the live image of `module`. Returns up
    /// to `limit` match RVAs (searches .text, then .rdata, then whole image —
    /// same fallback order as the dumper).
    fn scan(&mut self, module_name: &str, pattern: &str, limit: usize) -> Result<Vec<u64>> {
        self.ensure_image(module_name)?;
        let key = module_name.trim_end_matches(".dll").to_ascii_lowercase();
        let img = self.image_cache.get(&key).unwrap();

        let (bytes, mask) = parse_ida_pattern(pattern)?;

        let mut hits = find_all_pattern(img.text(), &bytes, &mask, limit);
        let mut base_rva = img.text_rva;
        if hits.is_empty()
            && let Some(rd) = img.rdata()
        {
            hits = find_all_pattern(rd, &bytes, &mask, limit);
            base_rva = img.rdata_rva;
        }
        if hits.is_empty() {
            hits = find_all_pattern(&img.bytes, &bytes, &mask, limit);
            base_rva = 0;
        }

        Ok(hits.into_iter().map(|off| (base_rva + off as u32) as u64).collect())
    }
}

fn parse_ida_pattern(pattern: &str) -> Result<(Vec<u8>, Vec<bool>)> {
    let mut bytes = Vec::new();
    let mut mask = Vec::new();
    for tok in pattern.split_ascii_whitespace() {
        if tok == "?" || tok == "??" {
            bytes.push(0);
            mask.push(false);
        } else {
            bytes.push(u8::from_str_radix(tok, 16).map_err(|e| anyhow!("bad hex byte '{tok}': {e}"))?);
            mask.push(true);
        }
    }
    if bytes.is_empty() {
        bail!("empty pattern");
    }
    Ok((bytes, mask))
}

fn find_all_pattern(hay: &[u8], bytes: &[u8], mask: &[bool], limit: usize) -> Vec<usize> {
    let mut out = Vec::new();
    let need = bytes.len();
    if hay.len() < need || need == 0 {
        return out;
    }
    let first = bytes[0];
    let first_wild = !mask[0];
    let end = hay.len() - need;
    let mut i = 0usize;
    while i <= end && out.len() < limit {
        if first_wild || hay[i] == first {
            let mut ok = true;
            for j in 1..need {
                if mask[j] && hay[i + j] != bytes[j] {
                    ok = false;
                    break;
                }
            }
            if ok {
                out.push(i);
            }
        }
        i += 1;
    }
    out
}

// --- expression evaluator ---------------------------------------------------

struct Expr<'a> {
    b: &'a [u8],
    i: usize,
}

impl<'a> Expr<'a> {
    fn new(s: &'a str) -> Self {
        Expr { b: s.as_bytes(), i: 0 }
    }

    fn ws(&mut self) {
        while self.i < self.b.len() && (self.b[self.i] as char).is_whitespace() {
            self.i += 1;
        }
    }

    fn peek(&mut self) -> Option<u8> {
        self.ws();
        self.b.get(self.i).copied()
    }

    fn eval(&mut self, sess: &mut Session) -> Result<u64> {
        let v = self.parse_expr(sess)?;
        self.ws();
        if self.i != self.b.len() {
            bail!("trailing input at byte {}", self.i);
        }
        Ok(v)
    }

    fn parse_expr(&mut self, sess: &mut Session) -> Result<u64> {
        let mut v = self.parse_term(sess)?;
        loop {
            match self.peek() {
                Some(b'+') => {
                    self.i += 1;
                    v = v.wrapping_add(self.parse_term(sess)?);
                }
                Some(b'-') => {
                    self.i += 1;
                    v = v.wrapping_sub(self.parse_term(sess)?);
                }
                _ => break,
            }
        }
        Ok(v)
    }

    fn parse_term(&mut self, sess: &mut Session) -> Result<u64> {
        match self.peek() {
            Some(b'[') => {
                self.i += 1;
                let inner = self.parse_expr(sess)?;
                if self.peek() != Some(b']') {
                    bail!("expected ']'");
                }
                self.i += 1;
                sess.read_u64(inner)
                    .ok_or_else(|| anyhow!("deref of unmapped 0x{:X}", inner))
            }
            Some(b'(') => {
                self.i += 1;
                let inner = self.parse_expr(sess)?;
                if self.peek() != Some(b')') {
                    bail!("expected ')'");
                }
                self.i += 1;
                Ok(inner)
            }
            Some(c) if c.is_ascii_digit() => self.parse_number(),
            Some(c) if c.is_ascii_alphabetic() || c == b'_' => self.parse_ident(sess),
            other => bail!("unexpected token {:?}", other.map(|c| c as char)),
        }
    }

    fn parse_number(&mut self) -> Result<u64> {
        self.ws();
        let start = self.i;
        let (radix, skip) = if self.b[self.i..].starts_with(b"0x") || self.b[self.i..].starts_with(b"0X") {
            (16, 2)
        } else {
            (10, 0)
        };
        self.i += skip;
        let ds = self.i;
        while self.i < self.b.len() && (self.b[self.i] as char).is_digit(radix) {
            self.i += 1;
        }
        if self.i == ds {
            bail!("bad number at byte {start}");
        }
        let text = std::str::from_utf8(&self.b[ds..self.i]).unwrap();
        u64::from_str_radix(text, radix).map_err(|e| anyhow!("bad number '{text}': {e}"))
    }

    fn parse_ident(&mut self, sess: &mut Session) -> Result<u64> {
        self.ws();
        let start = self.i;
        while self.i < self.b.len() {
            let c = self.b[self.i];
            if c.is_ascii_alphanumeric() || c == b'_' || c == b'.' {
                self.i += 1;
            } else {
                break;
            }
        }
        let name = std::str::from_utf8(&self.b[start..self.i]).unwrap();
        sess.module(name)
            .map(|m| m.base)
            .ok_or_else(|| anyhow!("unknown module '{name}'"))
    }
}

// --- read + interpret -------------------------------------------------------

fn hexdump(bytes: &[u8]) -> String {
    bytes
        .iter()
        .map(|b| format!("{b:02X}"))
        .collect::<Vec<_>>()
        .join(" ")
}

fn interpret(sess: &mut Session, expr: &str, n: usize, ty: &str) -> Value {
    let addr = match Expr::new(expr).eval(sess) {
        Ok(a) => a,
        Err(e) => return json!({ "expr": expr, "error": e.to_string() }),
    };

    let mut o = serde_json::Map::new();
    o.insert("expr".into(), json!(expr));
    o.insert("addr".into(), json!(format!("0x{addr:X}")));
    if let Some(loc) = sess.attribute(addr) {
        o.insert("in".into(), json!(loc));
    }
    o.insert("type".into(), json!(ty));

    macro_rules! typed {
        ($t:ty) => {
            match sess.process.read::<$t>(Address::from(addr)).data_part() {
                Ok(v) => o.insert("value".into(), json!(v)),
                Err(_) => o.insert("error".into(), json!("unreadable")),
            }
        };
    }

    match ty {
        "hex" | "" => {
            let count = if n == 0 { 16 } else { n };
            match sess.read_bytes(addr, count) {
                Some(b) => {
                    o.insert("hex".into(), json!(hexdump(&b)));
                }
                None => {
                    o.insert("error".into(), json!("unreadable"));
                }
            }
        }
        "u8" => { typed!(u8); }
        "u16" => { typed!(u16); }
        "u32" => { typed!(u32); }
        "u64" => { typed!(u64); }
        "i8" => { typed!(i8); }
        "i16" => { typed!(i16); }
        "i32" => { typed!(i32); }
        "i64" => { typed!(i64); }
        "f32" => { typed!(f32); }
        "f64" => { typed!(f64); }
        "ptr" => match sess.read_u64(addr) {
            Some(p) => {
                o.insert("value".into(), json!(format!("0x{p:X}")));
                if let Some(loc) = sess.attribute(p) {
                    o.insert("points_to".into(), json!(loc));
                }
            }
            None => {
                o.insert("error".into(), json!("unreadable"));
            }
        },
        "cstr" => {
            let len = if n == 0 { 256 } else { n };
            match sess.process.read_utf8_lossy(Address::from(addr), len).data_part() {
                Ok(s) => {
                    // trim at first NUL
                    let s = s.split('\0').next().unwrap_or("").to_string();
                    o.insert("value".into(), json!(s));
                }
                Err(_) => {
                    o.insert("error".into(), json!("unreadable"));
                }
            }
        }
        "utf16" => {
            let count = if n == 0 { 64 } else { n };
            match sess.read_bytes(addr, count * 2) {
                Some(b) => {
                    let units: Vec<u16> = b
                        .chunks_exact(2)
                        .map(|c| u16::from_le_bytes([c[0], c[1]]))
                        .take_while(|&u| u != 0)
                        .collect();
                    o.insert("value".into(), json!(String::from_utf16_lossy(&units)));
                }
                None => {
                    o.insert("error".into(), json!("unreadable"));
                }
            }
        }
        "vec3" => {
            let comps: Vec<Value> = (0..3)
                .map(|i| {
                    sess.process
                        .read::<f32>(Address::from(addr + 4 * i))
                        .data_part()
                        .map(|f| json!(f))
                        .unwrap_or(Value::Null)
                })
                .collect();
            o.insert("value".into(), json!(comps));
        }
        other => {
            o.insert("error".into(), json!(format!("unknown type '{other}'")));
        }
    }

    Value::Object(o)
}

// --- tiny loopback HTTP -----------------------------------------------------

fn percent_decode(s: &str) -> String {
    let bytes = s.as_bytes();
    let mut out = Vec::with_capacity(bytes.len());
    let mut i = 0;
    while i < bytes.len() {
        if bytes[i] == b'%' && i + 2 < bytes.len() {
            let hi = (bytes[i + 1] as char).to_digit(16);
            let lo = (bytes[i + 2] as char).to_digit(16);
            if let (Some(h), Some(l)) = (hi, lo) {
                out.push((h * 16 + l) as u8);
                i += 3;
                continue;
            }
        }
        // NB: '+' is left LITERAL (expression operator), not decoded to space.
        out.push(bytes[i]);
        i += 1;
    }
    String::from_utf8_lossy(&out).into_owned()
}

fn parse_query(q: &str) -> Vec<(String, String)> {
    q.split('&')
        .filter(|p| !p.is_empty())
        .map(|pair| {
            let mut it = pair.splitn(2, '=');
            let k = it.next().unwrap_or("").to_string();
            let v = percent_decode(it.next().unwrap_or(""));
            (k, v)
        })
        .collect()
}

fn respond(stream: &mut TcpStream, status: &str, body: &str) {
    let msg = format!(
        "HTTP/1.1 {status}\r\nContent-Type: application/json\r\nContent-Length: {}\r\nConnection: close\r\nAccess-Control-Allow-Origin: *\r\n\r\n{body}",
        body.len()
    );
    let _ = stream.write_all(msg.as_bytes());
    let _ = stream.flush();
}

fn handle(stream: &mut TcpStream, sess: &mut Session) -> Result<()> {
    let mut reader = BufReader::new(stream.try_clone()?);
    let mut request_line = String::new();
    if reader.read_line(&mut request_line)? == 0 {
        return Ok(());
    }
    let mut parts = request_line.split_whitespace();
    let method = parts.next().unwrap_or("").to_string();
    let target = parts.next().unwrap_or("/").to_string();

    let mut content_length = 0usize;
    loop {
        let mut line = String::new();
        if reader.read_line(&mut line)? == 0 {
            break;
        }
        if line == "\r\n" || line == "\n" {
            break;
        }
        if let Some(rest) = line.to_ascii_lowercase().strip_prefix("content-length:") {
            content_length = rest.trim().parse().unwrap_or(0);
        }
    }
    let mut body = vec![0u8; content_length];
    if content_length > 0 {
        reader.read_exact(&mut body)?;
    }

    let (path, query) = match target.split_once('?') {
        Some((p, q)) => (p, q),
        None => (target.as_str(), ""),
    };

    match (method.as_str(), path) {
        ("GET", "/health") => {
            let mods: serde_json::Map<String, Value> = sess
                .modules
                .iter()
                .map(|m| (m.name.clone(), json!(format!("0x{:X}", m.base))))
                .collect();
            let body = serde_json::to_string_pretty(&json!({
                "ok": true,
                "process": PROC_NAME,
                "pid": sess.pid,
                "module_count": sess.modules.len(),
                "modules": mods,
            }))?;
            respond(stream, "200 OK", &body);
        }
        ("GET", "/bases") => {
            let arr: Vec<Value> = sess
                .modules
                .iter()
                .map(|m| json!({ "name": m.name, "base": format!("0x{:X}", m.base), "size": format!("0x{:X}", m.size) }))
                .collect();
            respond(stream, "200 OK", &serde_json::to_string_pretty(&arr)?);
        }
        ("GET", "/read") => {
            let params = parse_query(query);
            let get = |k: &str| params.iter().find(|(n, _)| n == k).map(|(_, v)| v.clone());
            let expr = get("e").unwrap_or_default();
            let n = get("n").and_then(|s| s.parse::<usize>().ok()).unwrap_or(0);
            let ty = get("t").unwrap_or_else(|| "hex".into());
            let result = interpret(sess, &expr, n, &ty);
            respond(stream, "200 OK", &serde_json::to_string_pretty(&result)?);
        }
        ("GET", "/scan") => {
            let params = parse_query(query);
            let get = |k: &str| params.iter().find(|(n, _)| n == k).map(|(_, v)| v.clone());
            let module = get("m").unwrap_or_default();
            let pattern = get("p").unwrap_or_default();
            let limit = get("limit").and_then(|s| s.parse::<usize>().ok()).unwrap_or(20);
            let ctx = get("ctx").and_then(|s| s.parse::<usize>().ok()).unwrap_or(32);

            let result = match sess.scan(&module, &pattern, limit) {
                Ok(rvas) => {
                    let hits: Vec<Value> = rvas
                        .iter()
                        .map(|&rva| {
                            let base = sess.module(&module).map(|m| m.base).unwrap_or(0);
                            let va = base + rva;
                            let bytes = sess.read_bytes(va, ctx);
                            json!({
                                "rva": format!("0x{:X}", rva),
                                "va": format!("0x{:X}", va),
                                "in": sess.attribute(va),
                                "bytes": bytes.as_deref().map(hexdump),
                            })
                        })
                        .collect();
                    json!({ "module": module, "pattern": pattern, "match_count": hits.len(), "hits": hits })
                }
                Err(e) => json!({ "module": module, "pattern": pattern, "error": e.to_string() }),
            };
            respond(stream, "200 OK", &serde_json::to_string_pretty(&result)?);
        }
        ("POST", "/read") => {
            let parsed: Value = serde_json::from_slice(&body).unwrap_or(Value::Null);
            let reads = parsed.get("reads").and_then(|v| v.as_array()).cloned().unwrap_or_default();
            let results: Vec<Value> = reads
                .iter()
                .map(|r| {
                    let expr = r.get("e").and_then(|v| v.as_str()).unwrap_or("").to_string();
                    let n = r.get("n").and_then(|v| v.as_u64()).unwrap_or(0) as usize;
                    let ty = r.get("t").and_then(|v| v.as_str()).unwrap_or("hex").to_string();
                    interpret(sess, &expr, n, &ty)
                })
                .collect();
            respond(stream, "200 OK", &serde_json::to_string_pretty(&json!({ "results": results }))?);
        }
        _ => respond(stream, "404 Not Found", "{\"error\":\"not found\"}"),
    }
    Ok(())
}

fn main() -> Result<()> {
    println!("cs2-live — non-intrusive live memory reader");
    println!("-------------------------------------------");
    let mut os = build_os()?;
    println!("[live] native OS ready; looking for {PROC_NAME} (admin required)…");
    let mut sess = attach(&mut os, false);
    println!(
        "[live] attached pid {} — {} modules resolved",
        sess.pid,
        sess.modules.len()
    );

    // self-test: client.dll should start with the DOS 'MZ' header.
    if let Some(client) = sess.module("client.dll") {
        let base = client.base;
        if let Some(b) = sess.read_bytes(base, 2) {
            println!(
                "[live] read self-test @ client.dll base 0x{base:X}: {} (expect 4D 5A)",
                hexdump(&b)
            );
        }
    }

    let listener = TcpListener::bind(("127.0.0.1", PORT))?;
    println!("[live] listening on http://127.0.0.1:{PORT}");
    println!("[live] try:  GET /health   |   GET /read?e=client+0xB1FB80&n=16   |   POST /read");
    println!("[live] the game keeps running normally — reads never suspend it.");

    for conn in listener.incoming() {
        let mut stream = match conn {
            Ok(s) => s,
            Err(_) => continue,
        };
        // If a read fails because cs2 restarted, transparently re-attach and
        // retry once before serving the request.
        let alive = sess
            .process
            .read::<u16>(Address::from(
                sess.module("client.dll").map(|m| m.base).unwrap_or(0),
            ))
            .data_part()
            .is_ok();
        if !alive {
            eprintln!("[live] process handle stale — re-attaching…");
            sess = attach(&mut os, true);
            eprintln!("[live] re-attached pid {}", sess.pid);
        }
        if let Err(e) = handle(&mut stream, &mut sess) {
            eprintln!("[live] request error: {e}");
        }
    }
    Ok(())
}

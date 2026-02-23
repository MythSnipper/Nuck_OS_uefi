use std::env;
use std::fs::File;
use std::io::{self, Read, Write, BufWriter, Seek, SeekFrom};
use std::process::{Command, Stdio};

fn main() -> io::Result<()> {
    let mut argv: Vec<String> = env::args().collect();
    let (infile_path, outfile_path, mode, resize) = parse_args(&mut argv);

    if infile_path.is_empty() {
        panic!("No infile path provided");
    }
    if mode == 67 {
        panic!("No valid mode provided or you typed SIX SEVEN");
    }

    let (mut width, mut height) = get_video_dimensions(&infile_path);

    let mut child = spawn_ffmpeg(&infile_path, resize, &mut width, &mut height)?;
    let mut ffmpeg_stdout = child.stdout.take().unwrap();

    let file = File::create(outfile_path).expect("can't open outfile");
    let mut writer = BufWriter::new(file);

    write_header(&mut writer, mode, width, height)?;

    let frame_size = (width * height * 3) as usize;
    let mut frame_buffer = vec![0u8; frame_size];
    let mut framecount = 0u32;

    while ffmpeg_stdout.read_exact(&mut frame_buffer).is_ok() {
        framecount += 1;

        if mode == 0 {
            encode_bitmap(&frame_buffer, width, height, &mut writer)?;
        } else if mode == 1 {
            encode_rle(&frame_buffer, width, height, &mut writer)?;
        }
    }

    writer.flush()?;

    writer.seek(SeekFrom::Start(12))?;
    writer.write_all(&framecount.to_le_bytes())?;
    writer.flush()?;

    println!("Frames written: {}", framecount);

    Ok(())
}


fn get_video_dimensions(path: &str) -> (u32, u32) {
    let output = Command::new("ffprobe")
        .args([
            "-v", "error",
            "-select_streams", "v:0",
            "-show_entries", "stream=width,height",
            "-of", "csv=p=0:s=x",
            path,
        ])
        .output()
        .expect("ffprobe failed");

    let mut parts = std::str::from_utf8(&output.stdout)
        .unwrap()
        .trim()
        .split('x');

    let width: u32 = parts.next().unwrap().parse().unwrap();
    let height: u32 = parts.next().unwrap().parse().unwrap();

    (width, height)
}

fn spawn_ffmpeg(
    infile: &str,
    resize: Option<(u32, u32)>,
    width: &mut u32,
    height: &mut u32,
) -> io::Result<std::process::Child> {
    let mut cmd = Command::new("ffmpeg");
    cmd.args(["-i", infile]);

    if let Some((w, h)) = resize {
        cmd.args(["-vf", &format!("scale={}:{}", w, h)]);
        *width = w;
        *height = h;
    }

    cmd.args([
        "-f",
        "rawvideo",
        "-pix_fmt",
        "rgb24",
        "-",
    ]);

    cmd.stdout(Stdio::piped())
        .stderr(Stdio::null())
        .spawn()
        .map_err(|e| io::Error::new(io::ErrorKind::Other, e))
}

fn write_header(
    writer: &mut BufWriter<File>,
    mode: u32,
    width: u32,
    height: u32,
) -> io::Result<()> {
    writer.write_all(&mode.to_le_bytes())?;
    writer.write_all(&width.to_le_bytes())?;
    writer.write_all(&height.to_le_bytes())?;
    writer.write_all(&0u32.to_le_bytes())?; // framecount placeholder
    Ok(())
}

fn encode_bitmap(
    frame_buffer: &[u8],
    width: u32,
    height: u32,
    writer: &mut BufWriter<File>,
) -> io::Result<()> {
    let mut i: usize = 0;

    for _ in 0..height {
        let mut remaining = width;

        while remaining >= 8 {
            let mut byte = 0u8;

            for bit_index in 0..8 {
                let r = frame_buffer[i] as u16;
                let g = frame_buffer[i + 1] as u16;
                let b = frame_buffer[i + 2] as u16;
                i += 3;

                let y = (77 * r + 150 * g + 29 * b) >> 8;
                let bit = if y >= 128 { 1 } else { 0 };

                byte |= bit << (7 - bit_index);
            }

            writer.write_all(&[byte])?;
            remaining -= 8;
        }

        if remaining > 0 {
            let mut byte = 0u8;

            for bit_index in 0..remaining {
                let r = frame_buffer[i] as u16;
                let g = frame_buffer[i + 1] as u16;
                let b = frame_buffer[i + 2] as u16;
                i += 3;

                let y = (77 * r + 150 * g + 29 * b) >> 8;
                let bit = if y >= 128 { 1 } else { 0 };

                byte |= bit << (7 - bit_index);
            }

            writer.write_all(&[byte])?;
        }
    }

    Ok(())
}

fn encode_rle(
    frame_buffer: &[u8],
    width: u32,
    height: u32,
    writer: &mut BufWriter<File>,
) -> io::Result<()> {
    let mut i: usize = 0;
    let total_pixels = (width * height) as usize;

    let mut run_color: i16 = 0;
    let mut run_length: i16 = 0;

    for _ in 0..total_pixels {
        let r = frame_buffer[i] as u16;
        let g = frame_buffer[i + 1] as u16;
        let b = frame_buffer[i + 2] as u16;
        i += 3;

        let y = (77 * r + 150 * g + 29 * b) >> 8;
        let color = if y >= 128 { 1 } else { -1 };

        if run_length == 0 {
            run_color = color;
            run_length = 1;
        } else if color == run_color {
            run_length += 1;

            if run_length == i16::MAX {
                let encoded = run_length * run_color;
                writer.write_all(&encoded.to_le_bytes())?;
                run_length = 0;
            }
        } else {
            let encoded = run_length * run_color;
            writer.write_all(&encoded.to_le_bytes())?;

            run_color = color;
            run_length = 1;
        }
    }

    if run_length > 0 {
        let encoded = run_length * run_color;
        writer.write_all(&encoded.to_le_bytes())?;
    }

    Ok(())
}

fn parse_args(args: &mut Vec<String>)
-> (String, String, u32, Option<(u32, u32)>)
{
    let helpmsg = format!(r#"Usage: {} [input file/dir] [options]
Options:
  -h, --help                    Display this text
  --version                     Display version information
  -m, --mode <mode>             Specify mode
  -o <filename>                 Specify output file name
  -r, --resize <widthxheight>   Resize frames to width x height

Modes:
    just see the notes.txt thing

"#, args[0]);

    let versionmsg = format!(r#"
{} Version 1.0.0 20260223
"#, args[0]);
    
    let mut infile_path: String = String::new();
    let mut outfile_path: String = String::new();
    let mut mode: u32 = 0;
    let mut resize: Option<(u32, u32)> = None;


    if args.len() < 2 { //at least 1 arg needed
        print!("{}", helpmsg);
        std::process::exit(0);
    }

    let mut flag_mode = false;
    let mut flag_output = false;
    let mut flag_resize = false;

    for arg in args {
        let arg: &str = arg;

        if flag_mode {
            flag_mode = false;
            mode = arg
                .parse()
                .expect("Invalid mode");
            continue;
        }
        if flag_output {
            flag_output = false;
            outfile_path = arg.to_string();
            continue;
        }
        if flag_resize {
            flag_resize = false;
            let split: Vec<u32> = arg.split('x').into_iter().map(|vel| vel.parse::<u32>().unwrap()).collect();
            if split.len() != 2 || split[0] == 0 || split[1] == 0 {
                panic!("invalid resolution to resize to");
            }
            resize = Some((split[0], split[1]));
            continue;
        }
        match arg {
            "-h" | "--help" => {
                print!("{}", helpmsg);
                std::process::exit(0);
            }
            "--version" => {
                print!("{}", versionmsg);
                std::process::exit(0);
            }
            "-m" | "--mode" => {
                flag_mode = true;
            }
            "-o" => {
                flag_output = true;
            }
            "-r" | "--resize" => {
                flag_resize = true;
            }
            _ => {
                infile_path = arg.to_string();
            }
        }
    }

    (infile_path, outfile_path, mode, resize)
}









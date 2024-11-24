/*
 *   s2png - "stuff to png"
 *   Copyright (c) 2006 k0wax
 *   Copyright (c) 2013-2020, 2023 D. Bohdan
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 */

use s2png::{font, rc4::Key};

use getopts::Options;
use image;
use image::{codecs::png, ImageEncoder};

use std::{
    env,
    fs::File,
    io::prelude::*,
    path::{Path, PathBuf},
    process::exit,
};

const VERSION: &'static str = "1.0.0";
const BANNER_HEIGHT: u32 = 8;

const RC4_DROP: usize = 3072;
const MAX_FILE_SIZE: u64 = 0xFFFFFF;
const MAX_IMAGE_WIDTH: u32 = 0xFFFF;

const DEFAULT_WIDTH: u32 = 600;
const DEFAULT_BANNER: &'static str = "This image contains binary data. \
To extract it get s2png on GitHub.";

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
enum Mode {
    ModeAuto,
    ModeDecode,
    ModeEncode,
}

#[derive(Clone, Debug, PartialEq, Eq)]
struct Config {
    banner: String,
    input: PathBuf,
    mode: Mode,
    output: Option<PathBuf>,
    hex_key: Option<String>,
    square: bool,
    width: u32,
}

fn usage() {
    println!("s2png (\"stuff to png\") version {}", VERSION);
    println!(
        "usage: s2png [-h] [-o filename] [-w width ({}) | -s] [-b text]
             [-p hex-key] [-e | -d] file",
        DEFAULT_WIDTH
    );
}

fn help() {
    usage();
    println!(
        "
Store any data in a PNG image.
This version can encode files of up to {0} bytes.

  -h            display this message and quit
  -o filename   output the encoded or decoded data to filename
  -w width      set the width of the PNG image output ({1} by default)
  -s            make the output image roughly square
  -b text       custom banner text (\"\" for no banner)
  -p hex-key    encrypt/decrypt the output with a hexadecimal key
                using RC4 (Warning: completely insecure! Do not use this if
                you want actual secrecy.)

Normally s2png detects which operation to perform by the file type. You can
override this behavior with the following switches:
  -e            force encoding mode
  -d            force decoding mode

See README.md for further details.",
        MAX_FILE_SIZE, DEFAULT_WIDTH
    );
}

fn cli() -> Result<Config, ()> {
    let args: Vec<String> = env::args().collect();
    let mut opts = Options::new();

    opts.optopt("o", "", "output", "NAME");
    opts.optflag("h", "", "help");
    opts.optopt("w", "", "width", "WIDTH");
    opts.optflag("s", "", "square");
    opts.optopt("b", "", "banner", "TEXT");
    opts.optopt("p", "", "hex-key", "HEXKEY");

    opts.optflag("e", "", "encode");
    opts.optflag("d", "", "decode");

    let matches = match opts.parse(&args[1..]) {
        Ok(m) => m,
        Err(_) => {
            return Err(());
        }
    };

    if matches.opt_present("h") {
        help();
        exit(exitcode::OK);
    };

    if matches.opt_present("e") && matches.opt_present("d") {
        return Err(());
    }

    let file = if matches.free.len() == 1 {
        matches.free[0].clone()
    } else {
        return Err(());
    };

    let width_opt = matches
        .opt_str("w")
        .unwrap_or(DEFAULT_WIDTH.to_string())
        .parse::<u32>();
    let width = match width_opt {
        Ok(x) => x,
        Err(_) => {
            return Err(());
        }
    };

    let mode = if matches.opt_present("e") {
        Mode::ModeEncode
    } else if matches.opt_present("d") {
        Mode::ModeDecode
    } else {
        Mode::ModeAuto
    };

    Ok(Config {
        banner: matches.opt_str("b").unwrap_or(DEFAULT_BANNER.to_string()),
        input: PathBuf::from(file),
        mode,
        output: matches.opt_str("o").map(|s| PathBuf::from(s)),
        hex_key: matches.opt_str("p"),
        square: matches.opt_present("s"),
        width,
    })
}

fn main() {
    let config = cli().unwrap_or_else(|_| {
        usage();
        exit(exitcode::USAGE);
    });

    let mode = if config.mode == Mode::ModeAuto {
        match is_png_file(&config.input) {
            Ok(true) => Mode::ModeDecode,
            Ok(false) => Mode::ModeEncode,
            Err(err) => {
                eprintln!("{}", err);
                exit(exitcode::DATAERR);
            }
        }
    } else {
        config.mode
    };

    let mut key_opt =
        config.hex_key.clone().and_then(|hex_key| {
            match init_rc4(&hex_key) {
                Ok(key) => Some(key),
                Err(err) => {
                    eprintln!("{}", err);
                    exit(exitcode::USAGE);
                }
            }
        });

    let result = match mode {
        Mode::ModeDecode => {
            let output = config
                .output
                .clone()
                .unwrap_or_else(|| orig_filename(&config.input));

            png_to_file(&config.input, &output, &mut key_opt)
        }
        Mode::ModeEncode => {
            let output = config.output.clone().unwrap_or_else(|| {
                PathBuf::from(format!("{}.png", &config.input.display()))
            });

            file_to_png(
                &config.input,
                &output,
                &mut key_opt,
                config.width,
                config.square,
                &config.banner,
            )
        }
        _ => {
            eprintln!("internal error: unknown mode");
            exit(exitcode::SOFTWARE);
        }
    };

    exit(match result {
        Ok(()) => exitcode::OK,
        Err((code, msg)) => {
            eprintln!("{}", msg);
            code
        }
    })
}

fn orig_filename<P: AsRef<Path>>(path: &P) -> PathBuf {
    let p = path.as_ref();

    let mut stemmed = match p.file_stem() {
        None => PathBuf::from(p),
        Some(x) => PathBuf::from(x),
    };

    if stemmed == p {
        stemmed.set_extension("orig");
    }

    stemmed
}

fn init_rc4(hex_key: &str) -> Result<Key, String> {
    let seed = match Key::scan_hex_key(hex_key) {
        Some(x) => x,
        None => {
            return Err(
                "error: hex key is not a hexadecimal string".to_string()
            )
        }
    };

    if seed.len() == 0 {
        return Err("error: hex key is empty".to_string());
    }

    let mut key = Key::new(&seed);
    key.drop(RC4_DROP);

    Ok(key)
}

fn is_png_file<P: AsRef<Path>>(path: P) -> std::io::Result<bool> {
    let png_sign: Vec<u8> =
        vec![0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a];
    let mut buf = Vec::<u8>::new();

    buf.resize(8, 0);
    let file = File::open(path)?;
    file.take(8).read(&mut buf)?;

    Ok(buf == png_sign)
}

fn png_to_file<P: AsRef<Path>, Q: AsRef<Path>>(
    input: P,
    output: Q,
    key_opt: &mut Option<Key>,
) -> Result<(), (exitcode::ExitCode, String)> {
    let mut reader = image::ImageReader::open(&input).map_err(|_| {
        (
            exitcode::NOINPUT,
            format!(
                "error: can't open input file `{}'",
                input.as_ref().display(),
            ),
        )
    })?;
    reader.set_format(image::ImageFormat::Png);
    let img = reader.decode().map_err(|err| {
        (
            exitcode::DATAERR,
            format!(
                "error: can't decode `{}' as PNG: {}",
                input.as_ref().display(),
                &err,
            ),
        )
    })?;

    // Indexed color images pass this check.
    match img.color() {
        image::ColorType::Rgb8 => {}
        image::ColorType::Rgba8 => {}
        _ => {
            return Err((
                exitcode::DATAERR,
                "error: image not RGB or RGBA".to_string(),
            ))
        }
    }

    // Get rid of the alpha channel.
    let img_rgb8 = image::DynamicImage::ImageRgb8(img.into_rgb8());

    let mut bytes = img_rgb8.into_bytes();
    let len = bytes.len();

    let size: usize = bytes[len - 1] as usize
        + (bytes[len - 2] as usize) * 0x100
        + (bytes[len - 3] as usize) * 0x10000;

    if size > len {
        return Err((
            exitcode::DATAERR,
            "error: encoded data size exceeds image size".to_string(),
        ));
    }

    match key_opt {
        None => {}
        Some(key) => {
            key.encrypt(&mut bytes[0..size]);
        }
    }

    let mut out_file = File::create(&output).map_err(|err| {
        (
            exitcode::CANTCREAT,
            format!(
                "error: can't open file `{}' for output: {}",
                output.as_ref().display(),
                err
            ),
        )
    })?;
    out_file.write(&bytes[0..size]).map_err(|err| {
        (
            exitcode::IOERR,
            format!(
                "error: can't write to file `{}': {}",
                output.as_ref().display(),
                err
            ),
        )
    })?;

    Ok(())
}

fn file_to_png<P: AsRef<Path>, Q: AsRef<Path>>(
    input: P,
    output: Q,
    key_opt: &mut Option<Key>,
    width: u32,
    square: bool,
    banner: &str,
) -> Result<(), (exitcode::ExitCode, String)> {
    let mut in_file = File::open(&input).map_err(|err| {
        (
            exitcode::NOINPUT,
            format!(
                "error: can't open input file `{}': {}",
                output.as_ref().display(),
                err
            ),
        )
    })?;

    // Should we handle this failing?
    let len = in_file.metadata().unwrap().len();

    if len > MAX_FILE_SIZE {
        return Err((
            exitcode::DATAERR,
            format!(
                "error: file `{}' too large to encode (over {} bytes)",
                input.as_ref().display(),
                MAX_FILE_SIZE,
            ),
        ));
    }

    let mut buffer = vec![];
    in_file.read_to_end(&mut buffer).map_err(|err| {
        (
            exitcode::IOERR,
            format!(
                "error: can't read file `{}': {}",
                input.as_ref().display(),
                err
            ),
        )
    })?;

    // If given no banner text hide the banner.
    let banner_height = if banner == "" { 0 } else { BANNER_HEIGHT };

    let mut image_width = if square {
        // Solve for x: x * (x - banner_height) = buffer_size / 3.0
        let h: f64 = banner_height as f64;
        let d: f64 = (4.0 * (len as f64) / 3.0 + h * h).sqrt();

        (0.5 * d + h).ceil() as u32
    } else {
        width
    };

    let image_height = ((len as f64) / (image_width as f64) / 3.0).ceil()
        as u32
        + banner_height;

    // Prevent buffer corruption when the image has no banner. If the image has
    // certain dimensions the bottom right pixel may be used for buffer and then
    // overwritten by the file size pixel.
    if banner_height == 0 && image_width * image_height * 3 - (len as u32) <= 2
    {
        image_width += 1;
    }

    if width == 0 || image_width > MAX_IMAGE_WIDTH {
        return Err((
            exitcode::DATAERR,
            format!(
                "error: invalid image width; must be between 1 and {}",
                MAX_IMAGE_WIDTH,
            ),
        ));
    }

    if image_height == 0 {
        return Err((
            exitcode::DATAERR,
            "error: can't encode empty file with no banner".to_string(),
        ));
    }

    match key_opt {
        None => {}
        Some(key) => key.encrypt(&mut buffer),
    }

    while (buffer.len() as u32) < image_width * image_height * 3 {
        buffer.push(0);
    }

    let mut img =
        image::RgbImage::from_raw(image_width, image_height, buffer).unwrap();

    // Add a banner at the bottom of the image.
    if banner_height > 0 {
        if !banner.is_ascii() {
            return Err((
                exitcode::DATAERR,
                "error: banner contains non-ASCII character".to_string(),
            ));
        }

        for x in 0..image_width {
            for y in (image_height - banner_height)..image_height {
                img.put_pixel(x, y, image::Rgb([255, 255, 255]));
            }
        }

        let mut x = 5;
        for ch in banner.bytes() {
            draw_char(
                &mut img,
                x,
                image_height - banner_height,
                ch,
                image::Rgb([0, 0, 0]),
            );
            x += 5;
        }
    }

    // Encode the data length in the bottom right pixel.
    img.put_pixel(
        image_width - 1,
        image_height - 1,
        image::Rgb([
            (len / 0x10000 & 0xff) as u8,
            (len / 0x100 & 0xff) as u8,
            (len & 0xff) as u8,
        ]),
    );

    let output_file = File::create(&output).map_err(|err| {
        (
            exitcode::CANTCREAT,
            format!(
                "error: can't open file `{}' for writing: {}",
                output.as_ref().display(),
                &err
            ),
        )
    })?;

    let encoder = png::PngEncoder::new_with_quality(
        output_file,
        png::CompressionType::Best,
        png::FilterType::NoFilter,
    );

    encoder
        .write_image(
            &img.as_raw(),
            image_width,
            image_height,
            image::ColorType::Rgb8.into(),
        )
        .map_err(|err| {
            (
                exitcode::CANTCREAT,
                format!(
                    "error: can't write image to file `{}': {}",
                    output.as_ref().display(),
                    &err
                ),
            )
        })?;

    Ok(())
}

fn draw_char<P: image::Pixel, I: image::GenericImage<Pixel = P>>(
    img: &mut I,
    x: u32,
    y: u32,
    ch: u8,
    pixel: P,
) {
    for xo in 0..5 {
        for yo in 0..8 {
            if img.in_bounds(x + xo, y + yo)
                && font::TINY[ch as usize][(yo * 5 + xo) as usize] != 0
            {
                img.put_pixel(x + xo, y + yo, pixel);
            }
        }
    }
}

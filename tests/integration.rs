/* Test s2png's encoding and decoding, with and without encryption, for
 * corrupting data.
 */

use std::{
    convert::AsRef,
    env,
    ffi::OsStr,
    process::Command,
    {
        io::{Result, Write},
        iter::IntoIterator,
    },
};
use {
    exitcode,
    file_diff::diff,
    rand::{distributions::Uniform, Rng},
    regex::Regex,
    tempfile::NamedTempFile,
};

#[derive(Debug, Eq, PartialEq)]
struct Output {
    code: i32,
    stderr: String,
    stdout: String,
}

fn s2png<I, S>(args: I) -> Result<Output>
where
    I: IntoIterator<Item = S>,
    S: AsRef<OsStr>,
{
    let s2png_cmd = env::var("S2PNG_COMMAND").unwrap();
    let output = Command::new(s2png_cmd).args(args).output()?;

    Ok(Output {
        code: output.status.code().unwrap(),
        stderr: String::from_utf8(output.stderr).unwrap(),
        stdout: String::from_utf8(output.stdout).unwrap(),
    })
}

#[test]
#[ignore]
fn help_message() {
    let re = Regex::new(r"Store any data").unwrap();

    let output = s2png(&["-h"]).unwrap();
    assert!(re.is_match(&output.stdout));
    assert_eq!(output.code, exitcode::OK);
}

#[test]
#[ignore]
fn bad_usage() {
    let output = s2png(&["-Q"]).unwrap();
    assert_eq!(output.code, exitcode::USAGE);
}

fn roundtrip<S1, S2, I1, I2>(data: &[u8], encode_extra: I1, decode_extra: I2)
where
    S1: AsRef<str>,
    S2: AsRef<str>,
    I1: IntoIterator<Item = S1>,
    I2: IntoIterator<Item = S2>,
{
    let mut input_file = NamedTempFile::new().unwrap();
    input_file.write_all(data).unwrap();
    let png_file = NamedTempFile::new().unwrap();
    let extracted_file = NamedTempFile::new().unwrap();

    let input_fn = input_file.path().to_str().unwrap();
    let png_fn = png_file.path().to_str().unwrap();
    let extracted_fn = extracted_file.path().to_str().unwrap();

    let mut encode_args =
        vec!["-o".to_string(), png_fn.to_string(), input_fn.to_string()];
    for arg in encode_extra {
        encode_args.push(arg.as_ref().to_string());
    }

    let out1 = s2png(&encode_args).unwrap();
    eprintln!("{}", out1.stderr);
    assert_eq!(out1.code, exitcode::OK);

    let mut decode_args = vec![
        "-o".to_string(),
        extracted_fn.to_string(),
        png_fn.to_string(),
    ];
    for arg in decode_extra {
        decode_args.push(arg.as_ref().to_string());
    }

    let out2 = s2png(&decode_args).unwrap();
    eprintln!("{}", out2.stderr);
    assert_eq!(out2.code, exitcode::OK);

    assert!(diff(&input_fn, &extracted_fn));
}

#[test]
#[ignore]
fn roundtrip_basic_0xff() {
    let empty: Vec<String> = vec![];
    roundtrip(
        &vec![0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff],
        &empty,
        &empty,
    );
}

#[test]
#[ignore]
fn roundtrip_square_0xff() {
    let empty: Vec<String> = vec![];
    roundtrip(
        &vec![0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff],
        vec!["-s"],
        &empty,
    );
}

#[test]
#[ignore]
fn roundtrip_password_0xff() {
    // printf password | md5sum | cut -c 1-32
    let password_extra = ["-p", "5f4dcc3b5aa765d61d8327deb882cf99"];

    roundtrip(
        &vec![0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff],
        &password_extra,
        &password_extra,
    );
}

#[test]
#[ignore]
fn roundtrip_no_banner_0xff() {
    roundtrip(
        &vec![0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff],
        vec!["-e", "-b", ""],
        vec!["-d"],
    );
}

fn random_vec(len: usize) -> Vec<u8> {
    let range = Uniform::from(0..=255);
    rand::thread_rng().sample_iter(&range).take(len).collect()
}

#[test]
#[ignore]
fn roundtrip_basic_random() {
    let empty: Vec<String> = vec![];
    roundtrip(
        &random_vec(0xffff),
        &empty,
        &empty,
    );
}

#[test]
#[ignore]
fn roundtrip_square_random() {
    let empty: Vec<String> = vec![];
    roundtrip(
        &random_vec(0xffff),
        vec!["-s"],
        &empty,
    );
}

#[test]
#[ignore]
fn roundtrip_password_random() {
    // printf password | md5sum | cut -c 1-32
    let password_extra = ["-p", "5f4dcc3b5aa765d61d8327deb882cf99"];

    roundtrip(
        &random_vec(0xffff),
        &password_extra,
        &password_extra,
    );
}

#[test]
#[ignore]
fn roundtrip_no_banner_random() {
    roundtrip(
        &random_vec(0xffff),
        vec!["-e", "-b", ""],
        vec!["-d"],
    );
}

#[test]
#[ignore]
fn small_file_corruption_16_255() {
    for i in 16..=255 {
        roundtrip(
            &random_vec(i),
            vec!["-e", "-b", "", "-w", "10"],
            vec!["-d"],
        )
    }
}

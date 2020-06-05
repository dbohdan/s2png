/*
 * This code is based on the C implementation of RC4 created by John Allen.
 * The original is available from http://www.cypherspace.org/adam/rsa/rc4c.html.
 * It is believed to be in the public domain.  The modifications and the Rust
 * port by D. Bohdan are released to the public domain.
 */

use hex;

use std::{cmp, fmt};

pub struct Key {
    state: [u8; 256],
    x: u8,
    y: u8,
}

// Can't derive this.
impl fmt::Debug for Key {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        f.debug_struct("Key")
            .field("x", &self.x)
            .field("y", &self.y)
            .field("state", &self.state[..].iter().collect::<Vec<_>>())
            .finish()
    }
}

impl cmp::PartialEq for Key {
    fn eq(&self, other: &Self) -> bool {
        self.x == other.x
            && self.y == other.y
            && self
                .state
                .iter()
                .zip(other.state.iter())
                .all(|(a, b)| a == b)
    }
}

impl cmp::Eq for Key {}

impl Key {
    pub fn scan_pass(hex_pass: &str) -> Option<Vec<u8>> {
        let mut even_pass = hex_pass.to_string();

        // This is how the C code treated passwords with an odd number of
        // characters.
        if even_pass.len() % 2 == 1 {
            even_pass.push('0');
        }

        hex::decode(&even_pass).ok()
    }

    pub fn new(seed: &[u8]) -> Key {
        let mut key = Key {
            state: [0; 256],
            x: 0,
            y: 0,
        };

        for counter in 0..256 {
            key.state[counter] = counter as u8;
        }

        let mut index1 = 0u8;
        let mut index2 = 0u8;
        for counter in 0..256 {
            index2 = (seed[index1 as usize])
                .wrapping_add(key.state[counter])
                .wrapping_add(index2);

            // Swap key.state[counter] and key.state[index2].
            let t = key.state[counter];
            key.state[counter] = key.state[index2 as usize];
            key.state[index2 as usize] = t;

            index1 = index1.wrapping_add(1) % seed.len() as u8;
        }

        key
    }

    pub fn encrypt(&mut self, buffer: &mut [u8]) {
        let mut xor_index: u8;

        let mut x = self.x;
        let mut y = self.y;

        for counter in 0..buffer.len() {
            x = x.wrapping_add(1);
            y = y.wrapping_add(self.state[x as usize]);

            // Swap self.state[x] and self.state[y].
            let t = self.state[x as usize];
            self.state[x as usize] = self.state[y as usize];
            self.state[y as usize] = t;

            xor_index =
                self.state[x as usize].wrapping_add(self.state[y as usize]);

            buffer[counter] ^= self.state[xor_index as usize];
        }

        self.x = x;
        self.y = y;
    }

    pub fn drop(&mut self, n: usize) {
        // Suboptimal, but we are not going to drop millions of values.
        let mut buffer: Vec<u8> = Vec::new();
        buffer.resize(n, 0);
        self.encrypt(&mut buffer);
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    use regex::Regex;

    #[test]
    fn test_pass_scan_1() {
        let bytes = Key::scan_pass("0123456789abcdef");
        assert_eq!(
            bytes,
            Some(vec![0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef])
        );
    }

    #[test]
    fn test_pass_scan_2() {
        let bytes = Key::scan_pass("WHAT THE HECK?!");
        assert_eq!(bytes, None);
    }

    #[test]
    fn test_new() {
        let key =
            Key::new(&vec![0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef]);

        assert_eq!(
            key,
            Key {
                state: [
                    0x1, 0x38, 0x6b, 0x3d, 0xac, 0x47, 0xe5, 0x1b, 0x8b, 0x10,
                    0xe8, 0xd1, 0x66, 0x26, 0xf9, 0xe7, 0xc9, 0x35, 0x7b, 0xe,
                    0x55, 0x4a, 0x2a, 0x4f, 0xe1, 0x94, 0x48, 0x22, 0xd2, 0x21,
                    0x43, 0x60, 0x23, 0x15, 0x14, 0xeb, 0x68, 0x6d, 0x33, 0x82,
                    0xf1, 0x77, 0xc, 0xf8, 0xad, 0x93, 0x80, 0xcf, 0xc5, 0x56,
                    0x8c, 0x92, 0xe3, 0x9f, 0x40, 0xce, 0x9, 0xb8, 0x99, 0xc8,
                    0x5a, 0x5c, 0x50, 0x7d, 0x13, 0x74, 0x4c, 0xa6, 0x73, 0xdd,
                    0x59, 0xb, 0x1c, 0x52, 0x41, 0xc7, 0x9c, 0x19, 0xaf, 0xed,
                    0xef, 0x81, 0x27, 0xd5, 0xe0, 0x54, 0xd3, 0xe4, 0x7, 0x9d,
                    0xfc, 0xfe, 0x91, 0x20, 0x45, 0xf7, 0x6e, 0x3, 0x8, 0xdb,
                    0x61, 0x62, 0xfb, 0xb2, 0xba, 0x9b, 0x72, 0x39, 0x1e, 0x46,
                    0x70, 0xbe, 0xca, 0x25, 0x17, 0xcc, 0x1f, 0x2d, 0x31, 0xb6,
                    0x28, 0xb3, 0x1d, 0x7f, 0x3f, 0xf5, 0x44, 0x8a, 0xd, 0x84,
                    0xbd, 0xfd, 0xb4, 0x0, 0x8f, 0x58, 0x2, 0x5f, 0xcd, 0xbf,
                    0x63, 0x2e, 0x6f, 0x36, 0xa8, 0x34, 0x6c, 0x85, 0x53, 0xc4,
                    0x71, 0x3a, 0x5, 0x95, 0x4e, 0xa5, 0x7e, 0x30, 0xb1, 0x9e,
                    0x86, 0x5e, 0xa4, 0xae, 0x2c, 0xd0, 0xcb, 0xb5, 0x5d, 0x1a,
                    0xe9, 0x5b, 0x8d, 0x65, 0x9a, 0x11, 0x78, 0xc0, 0x7a, 0xd4,
                    0x6, 0x83, 0xb0, 0xe2, 0xd6, 0x75, 0x37, 0x4b, 0x49, 0xdc,
                    0xdf, 0x8e, 0x16, 0x69, 0x29, 0x18, 0x76, 0xaa, 0x67, 0xea,
                    0xa9, 0x24, 0xa3, 0xd9, 0x98, 0x2f, 0x3b, 0xd8, 0x79, 0x12,
                    0xee, 0xc2, 0xb9, 0xf4, 0xe6, 0xf6, 0x3e, 0xbc, 0xbb, 0x3c,
                    0xa0, 0x88, 0xde, 0x57, 0x96, 0x87, 0xc1, 0x4, 0xda, 0xc6,
                    0x97, 0xa, 0x2b, 0x51, 0x4d, 0xd7, 0xb7, 0x7c, 0xf3, 0xff,
                    0xf0, 0xf2, 0xab, 0xfa, 0x64, 0x90, 0xec, 0xc3, 0x89, 0xa2,
                    0x32, 0xa1, 0x42, 0xa7, 0xf, 0x6a,
                ],
                x: 0,
                y: 0,
            },
        )
    }

    #[test]
    fn test_fmt() {
        let key =
            Key::new(&vec![0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef]);

        let re =
            Regex::new(r"^Key \{ x: 0, y: 0, state: \[1, 56, .*, 106\] \}$")
                .unwrap();

        assert!(re.is_match(&format!("{:?}", key)));
    }

    #[test]
    fn test_encrypt() {
        let mut buffer: [u8; 16] =
            [0, 1, 2, 3, 4, 5, 0, 11, 22, 33, 44, 55, 66, 77, 88, 99];
        let mut key =
            Key::new(&vec![0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef]);

        key.encrypt(&mut buffer);

        assert_eq!(
            buffer,
            [
                0x74, 0x95, 0xc0, 0xe4, 0x14, 0x4e, 0x8, 0x72, 0x1b, 0x6a,
                0xf9, 0x64, 0x70, 0xc2, 0x46, 0x9f
            ]
        );
    }

    #[test]
    fn test_drop() {
        let mut buffer: [u8; 16] =
            [0, 1, 2, 3, 4, 5, 0, 11, 22, 33, 44, 55, 66, 77, 88, 99];
        let mut key =
            Key::new(&vec![0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef]);

        key.drop(3072);
        key.encrypt(&mut buffer);

        assert_eq!(
            buffer,
            [
                0x8a, 0xbd, 0x8a, 0x57, 0x7c, 0x65, 0x7f, 0x79, 0x6f, 0x22,
                0x6e, 0x31, 0x18, 0x3c, 0xf3, 0xe1,
            ]
        );
    }
}

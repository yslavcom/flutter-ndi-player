use std::os::raw::c_void;
use crossbeam_queue::SegQueue;

use once_cell::sync::Lazy;
use std::sync::{Arc, Mutex};


// Define a type for the callback function signature
type CallbackFn = unsafe extern "C" fn(arg: *const c_void);

#[derive(Debug)]
pub enum Error
{
    MemoryAllocationFail
}

#[derive(Clone, Copy)]
pub struct AudioFrameStr
{
    opaque: usize,
    chan_no: u32,
    samples_opaque: usize,
    samples_no: u32,
    stride: u32,
    planar: bool,
}

impl AudioFrameStr {
    pub fn new(opaque: usize, chan_no: u32, samples_opaque: usize, samples_no: u32, stride: u32, planar: bool) -> Self {
        Self {opaque:       opaque,
            chan_no:        chan_no,
            samples_opaque: samples_opaque,
            samples_no:     samples_no,
            stride:         stride,
            planar:         planar,
        }
    }
}

pub struct AudioDataCallback
{
    cleanup_cb: Option<CallbackFn>,
    aud_frame: SegQueue<AudioFrameStr>,
}

impl Drop for AudioDataCallback {
    fn drop(&mut self) {
        // Using pattern matching

        if let Some(cleanup_cb) = &self.cleanup_cb {
            loop {
                let element = self.aud_frame.pop();
                match element {
                    Some(el) => {
                        unsafe {
                            debug!("cleanup_cb: {:?}", el.opaque);
                            cleanup_cb(el.opaque as *const c_void);
                        }
                    },
                    None => {
                        // no more elements in the queue
                        return;
                    }
                }
            }
        }
    }
}

impl AudioDataCallback {

    pub fn new() -> Self {
        Self { cleanup_cb : None,
            aud_frame: SegQueue::new(),
        }
    }

    pub fn set_callback(&mut self, callback: CallbackFn) {
        self.cleanup_cb = Some(callback);
    }

    pub fn add_audio_frame(&mut self, audio_frame: AudioFrameStr) -> Result<(), Error> {
        let result = std::panic::catch_unwind(|| {
            self.aud_frame.push(audio_frame);
        });
        match result {
            Ok(_) => {
                Ok(())
            }
            Err(_e) => {
                Err(Error::MemoryAllocationFail)
            }
        }
    }

    pub fn len(&self) -> usize
    {
        self.aud_frame.len()
    }
}


// Add audio frame to the queue
pub static AUDIO_DATA: Lazy<Arc<Mutex<AudioDataCallback>>> = Lazy::new(|| {
    Arc::new(Mutex::new(AudioDataCallback::new()))
    });


use std::os::raw::c_void;
use crossbeam_queue::SegQueue;

use once_cell::sync::Lazy;
use std::sync::{Arc, Mutex};


// Define a type for the callback function signature
pub type CallbackFn = unsafe extern "C" fn(arg: *const c_void);

#[derive(Debug)]
pub enum Error
{
    MemoryAllocationFail
}

#[derive(Clone, Copy)]
pub struct AudioFrameStr
{
    opaque: usize,
    pub chan_no: u32,
    pub samples_opaque: usize,
    pub samples_no: u32,
    pub stride: u32,
    pub planar: bool,
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
    aud_frame_cache: Option<AudioFrameStr>,
    aud_cache_pos: u32,
}

impl Drop for AudioDataCallback {
    fn drop(&mut self) {
        // Using pattern matching

        if self.cleanup_cb.is_some() {

            if let Some(aud_frame_cache) = &self.aud_frame_cache {
                self.cleanup(aud_frame_cache);
            }

            loop {
                let element = self.aud_frame.pop();
                match element {
                    Some(el) => {
                        debug!("cleanup_cb: {:?}", el.opaque);
                        self.cleanup(&el);
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
            aud_frame_cache : None,
            aud_cache_pos : 0
        }
    }

    pub fn set_callback(&mut self, callback: Option<CallbackFn>) {
        self.cleanup_cb = callback;
    }

    pub fn cleanup(&self, aud: &AudioFrameStr) {
        if let Some(cleanup_cb) = self.cleanup_cb {
            unsafe {
                cleanup_cb(aud.opaque as *const c_void);
            }
        }
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

    pub fn pop_aud_frame(&mut self) -> Option<AudioFrameStr> {
        self.aud_frame.pop()
    }

    pub fn cache_set(&mut self, audio_frame: AudioFrameStr) {
        self.aud_frame_cache = Some(audio_frame);
        self.aud_cache_pos = 0;
    }

    pub fn cache_get(&self) -> Option<&AudioFrameStr> {
        self.aud_frame_cache.as_ref()
    }

    pub fn cache_clear(&mut self) {
        if let Some(aud_frame_cache) = &self.aud_frame_cache {
            self.cleanup(aud_frame_cache);
        }
        self.aud_cache_pos = 0;
    }

    pub fn aud_cache_pos_set(&mut self, val: u32) {
        self.aud_cache_pos = val;
    }

    pub fn aud_cache_pos_get(&self) -> &u32 {
        &self.aud_cache_pos
    }
}


// Add audio frame to the queue
pub static AUDIO_DATA: Lazy<Arc<Mutex<AudioDataCallback>>> = Lazy::new(|| {
    Arc::new(Mutex::new(AudioDataCallback::new()))
    });


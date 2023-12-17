use std::os::raw::c_void;
use crossbeam_queue::SegQueue;

use once_cell::sync::Lazy;
use std::sync::{Arc, Mutex};


// Define a type for the callback function signature
pub type CallbackFn = unsafe extern "C" fn(ctxt: *const c_void, user_data: *const c_void);

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
    context: usize,

    aud_frame: SegQueue<AudioFrameStr>,

    total_queued_samples_per_chan: u32,

    aud_frame_cache: Option<AudioFrameStr>,
    cache_pos: isize,
    cache_sample_no: isize,
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
            context: 0,
            aud_frame: SegQueue::new(),
            total_queued_samples_per_chan: 0,
            aud_frame_cache : None,
            cache_pos : 0,
            cache_sample_no : 0,
        }
    }

    pub fn set_callback(&mut self, callback: Option<CallbackFn>, ctxt: usize) {
        self.cleanup_cb = callback;
        self.context = ctxt;
    }

    pub fn cleanup(&self, aud: &AudioFrameStr) {
        debug!("Try cleanup");
        if let Some(cleanup_cb) = self.cleanup_cb {
            unsafe {
                debug!("Cleanup:{}", aud.opaque);
                cleanup_cb(self.context as *const c_void, aud.opaque as *const c_void);
            }
        }
    }

    pub fn add_audio_frame(&mut self, audio_frame: AudioFrameStr) -> Result<(), Error> {
        assert_eq!(audio_frame.chan_no, 2, "Expecting channel count be {}, but in reality it's {}", 2, audio_frame.chan_no);

        let result = std::panic::catch_unwind(|| {
            self.aud_frame.push(audio_frame);
        });
        match result {
            Ok(_) => {
                self.total_queued_samples_per_chan += audio_frame.samples_no;
                Ok(())
            }
            Err(_e) => {
                Err(Error::MemoryAllocationFail)
            }
        }
    }

    pub fn get_total_samples_per_chan(&self) -> u32 {
        self.total_queued_samples_per_chan + self.get_cached_samples_per_chan()
    }

    /// Get one sample for stereo. Consider creating an iterator and returning the whole set of requested data
    pub fn get_sample(&mut self) -> Option<(f32, f32)> {
        if self.get_total_samples_per_chan() == 0 {
            ()
        }

        if self.get_cached_samples_per_chan() == 0 {
            // We're sure there's at list one audio frame is queued
            let audio_frame = self.pop_aud_frame().unwrap();
            self.cache_set(audio_frame);
        }

        let sample_ptr: *const f32 = self.aud_frame_cache.unwrap().samples_opaque as *const f32;
        let sample_0 = unsafe {*sample_ptr.offset(self.cache_pos)};
        let sample_1 = unsafe {*sample_ptr.offset(self.cache_pos+1)};
        self.cache_pos += 2;
        if self.cache_pos >= self.cache_sample_no {
            self.cache_clear();
        }
        Some((sample_0, sample_1))
    }

    #[allow(dead_code)]
    pub fn len(&self) -> usize {
        self.aud_frame.len()
    }

    // private

    fn pop_aud_frame(&mut self) -> Option<AudioFrameStr> {
        let frame = self.aud_frame.pop();
        if let Some(frame) = frame {
            self.total_queued_samples_per_chan -= frame.samples_no;
        }
        frame
    }

    fn cache_set(&mut self, audio_frame: AudioFrameStr) {
        self.aud_frame_cache = Some(audio_frame);
        self.cache_pos = 0;
        self.cache_sample_no = audio_frame.samples_no as isize;
    }

    #[allow(dead_code)]
    fn cache_get(&self) -> Option<&AudioFrameStr> {
        self.aud_frame_cache.as_ref()
    }

    fn cache_clear(&mut self) {
        if let Some(aud_frame_cache) = &self.aud_frame_cache {
            self.cleanup(aud_frame_cache);
        }
        self.aud_frame_cache = None;
        self.cache_pos = 0;
        self.cache_sample_no = 0;
    }

    fn get_cached_samples_per_chan(&self) -> u32 {
        if self.cache_sample_no == 0 {
            return 0
        }
        return (self.cache_sample_no - self.cache_pos) as u32
    }
}


// Add audio frame to the queue
pub static AUDIO_DATA: Lazy<Arc<Mutex<AudioDataCallback>>> = Lazy::new(|| {
    Arc::new(Mutex::new(AudioDataCallback::new()))
    });


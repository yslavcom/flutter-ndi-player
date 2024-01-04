pub mod audio_rx;
use crate::audio_rx::AUDIO_DATA;
use crate::audio_rx::AudioFrameStr;
use crate::audio_rx::CallbackFn;

// // #[cfg(not(feature = "host_test"))]
use oboe::{
    //AudioDeviceDirection,
    //AudioDeviceInfo,
    //AudioFeature,
    AudioOutputCallback,
    AudioOutputStream,
    AudioOutputStreamSafe,
    AudioStream,
    AudioStreamAsync,
    AudioStreamBase,
    AudioStreamBuilder,
    DataCallbackResult,
    DefaultStreamValues,
    Mono,
    Output,
    PerformanceMode,
    SharingMode,
    Stereo,
};

use atomic_float::AtomicF32;

use std::time::Duration;
use std::{
    f32::consts::PI,
    time::Instant,
    marker::PhantomData,
    sync::{atomic::Ordering, Arc},
};

use lazy_static::lazy_static;
use std::sync::Mutex;

#[macro_use] extern crate log;

// #[cfg(not(feature = "host_test"))]
extern crate android_logger;

use log::LevelFilter;
// #[cfg(not(feature = "host_test"))]
use android_logger::Config;

//////////////////////////
/// Code

/// Sine-wave generator stream
// #[cfg(not(feature = "host_test"))]
#[derive(Default)]
pub struct SineGen {
    stream: Option<AudioStreamAsync<Output, SineWave<f32, Stereo>>>,
}

// #[cfg(not(feature = "host_test"))]
impl SineGen {

    fn new() -> Self {
        Self{stream: None}
    }

    /// Create and start audio stream
    pub fn try_start(&mut self) {
        if self.stream.is_none() {
            let param = Arc::new(SineParam::default());

            let mut stream = AudioStreamBuilder::default()
                .set_performance_mode(PerformanceMode::LowLatency)
                .set_sharing_mode(SharingMode::Shared)
                .set_format::<f32>()
                .set_channel_count::<Stereo>()
                .set_callback(SineWave::<f32, Stereo>::new(&param))
                .open_stream()
                .unwrap();

            log::debug!("start stream: {:?}", stream);

            param.set_sample_rate(stream.get_sample_rate() as _);

            stream.start().unwrap();

            self.stream = Some(stream);
            debug!("self.stream: {:?}", self.stream);
        }
    }

    /// Pause audio stream
    pub fn try_pause(&mut self) {
        if let Some(stream) = &mut self.stream {
            log::debug!("pause stream: {:?}", stream);
            stream.pause().unwrap();
        }
    }

    /// Stop and remove audio stream
    pub fn try_stop(&mut self) {
        if let Some(stream) = &mut self.stream {
            log::debug!("stop stream: {:?}", stream);
            stream.stop().unwrap();
            self.stream = None;
        }
    }
}

// #[cfg(not(feature = "host_test"))]
pub struct SineParam {
    frequency: AtomicF32,
    gain: AtomicF32,
    sample_rate: AtomicF32,
    delta: AtomicF32,
}

// #[cfg(not(feature = "host_test"))]
impl Default for SineParam {
    fn default() -> Self {
        Self {
            frequency: AtomicF32::new(440.0),
            gain: AtomicF32::new(0.5),
            sample_rate: AtomicF32::new(0.0),
            delta: AtomicF32::new(0.0),
        }
    }
}

// #[cfg(not(feature = "host_test"))]
impl SineParam {
    fn set_sample_rate(&self, sample_rate: f32) {
        let frequency = self.frequency.load(Ordering::Acquire);
        let delta = frequency * 2.0 * PI / sample_rate;

        self.delta.store(delta, Ordering::Release);
        self.sample_rate.store(sample_rate, Ordering::Relaxed);

        println!("Prepare sine wave generator: samplerate={sample_rate}, time delta={delta}");
    }

    #[allow(dead_code)]
    fn set_frequency(&self, frequency: f32) {
        let sample_rate = self.sample_rate.load(Ordering::Relaxed);
        let delta = frequency * 2.0 * PI / sample_rate;

        self.delta.store(delta, Ordering::Relaxed);
        self.frequency.store(frequency, Ordering::Relaxed);
    }

    #[allow(dead_code)]
    fn set_gain(&self, gain: f32) {
        self.gain.store(gain, Ordering::Relaxed);
    }
}

// #[cfg(not(feature = "host_test"))]
pub struct SineWave<F, C> {
    param: Arc<SineParam>,
    phase: f32,
    marker: PhantomData<(F, C)>,
}

// #[cfg(not(feature = "host_test"))]
impl<F, C> Drop for SineWave<F, C> {
    fn drop(&mut self) {
        println!("drop SineWave generator");
    }
}

// #[cfg(not(feature = "host_test"))]
impl<F, C> SineWave<F, C> {
    pub fn new(param: &Arc<SineParam>) -> Self {
        println!("init SineWave generator");
        Self {
            param: param.clone(),
            phase: 0.0,
            marker: PhantomData,
        }
    }
}

// #[cfg(not(feature = "host_test"))]
impl<F, C> Iterator for SineWave<F, C> {
    type Item = f32;

    fn next(&mut self) -> Option<Self::Item> {
        let delta = self.param.delta.load(Ordering::Relaxed);
        let gain = self.param.gain.load(Ordering::Relaxed);

        let frame = gain * self.phase.sin();

        self.phase += delta;
        while self.phase > 2.0 * PI {
            self.phase -= 2.0 * PI;
        }

        Some(frame)
    }
}

// #[cfg(not(feature = "host_test"))]
impl AudioOutputCallback for SineWave<f32, Mono> {
    type FrameType = (f32, Mono);

    fn on_audio_ready(
        &mut self,
        _stream: &mut dyn AudioOutputStreamSafe,
        frames: &mut [f32],
    ) -> DataCallbackResult {
        for frame in frames {
            *frame = self.next().unwrap();
        }
        DataCallbackResult::Continue
    }
}

// #[cfg(not(feature = "host_test"))]
impl AudioOutputCallback for SineWave<f32, Stereo> {
    type FrameType = (f32, Stereo);

    fn on_audio_ready(
        &mut self,
        _stream: &mut dyn AudioOutputStreamSafe,
        frames: &mut [(f32, f32)],
    ) -> DataCallbackResult {
        for frame in frames {
            frame.0 = self.next().unwrap();
            frame.1 = frame.0;
        }
        DataCallbackResult::Continue
    }
}

// #[cfg(not(feature = "host_test"))]
lazy_static! {
    static ref SINE: Mutex<SineGen> = Mutex::new(SineGen::new());
}

/// Sine-wave generator stream
// #[cfg(not(feature = "host_test"))]
#[derive(Default)]
pub struct AudPlay {
    stream: Option<AudioStreamAsync<Output, NdiAudSamples>>,
}

// #[cfg(not(feature = "host_test"))]
impl AudPlay {

    fn new() -> Self {
        Self{stream: None}
    }

    /// Create and start audio stream
    pub fn try_start(&mut self, ) {

        audio_probe();

        if self.stream.is_none() {

            let mut stream = AudioStreamBuilder::default()
                .set_performance_mode(PerformanceMode::LowLatency)
                .set_sharing_mode(SharingMode::Shared)
                .set_format::<f32>()
                .set_channel_count::<Stereo>()
                .set_callback(NdiAudSamples::new())
                .open_stream()
                .unwrap();

            log::debug!("start stream: {:?}", stream);

            stream.start().unwrap();

            self.stream = Some(stream);
            debug!("self.stream: {:?}", self.stream);
        }
    }

    /// Pause audio stream
    pub fn try_pause(&mut self) {
        if let Some(stream) = &mut self.stream {
            log::debug!("pause stream: {:?}", stream);
            stream.pause().unwrap();
        }
    }

    /// Stop and remove audio stream
    pub fn try_stop(&mut self) {
        if let Some(stream) = &mut self.stream {
            log::debug!("stop stream: {:?}", stream);
            stream.stop().unwrap();
            self.stream = None;
        }
    }
}

// #[cfg(not(feature = "host_test"))]
impl AudioOutputCallback for NdiAudSamples {
    type FrameType = (f32, Stereo);
    fn on_audio_ready(
        &mut self,
        _stream: &mut dyn AudioOutputStreamSafe,
        frames: &mut [(f32, f32)],
    ) -> DataCallbackResult {
        // check stream AudioOutputStreamSafe properties here

        let mut start_time = AUD_CB_ELAPSED.lock().unwrap();
        let _t = start_time.unwrap_or(Instant::now());
        *start_time = Some(Instant::now());

        let mut aud_data = AUDIO_DATA.lock().unwrap();

        let demand_samples = frames.len();
        let total_samples_per_chan = aud_data.get_total_samples_per_chan() as usize;

        let mut push_time = AUD_WRITE_ELAPSED.lock().unwrap();
        let t = push_time.unwrap_or(Instant::now());
        if t.elapsed() >= Duration::from_secs(1) {
            *push_time = Some(Instant::now());
        }

        if demand_samples <= total_samples_per_chan {
            for frame in frames {
                let samples = aud_data.get_sample();
                if let Some(sample) = samples {
                    frame.0 = sample.0;
                    frame.1 = sample.1;
                }
                else {
                    frame.0 = 0.0;
                    frame.1 = 0.0;
                }
            }
        }


        DataCallbackResult::Continue
    }
}

// #[cfg(not(feature = "host_test"))]
pub struct NdiAudSamples {
    prev_sample: Option<(f32, f32)>,
}

// #[cfg(not(feature = "host_test"))]
impl NdiAudSamples {
    pub fn new() -> Self {
        Self {
            prev_sample: None,
        }
    }
}

// #[cfg(not(feature = "host_test"))]
lazy_static! {
    static ref AUD_PLAY: Mutex<AudPlay> = Mutex::new(AudPlay::new());
    static ref AUD_CB_ELAPSED: Mutex<Option<Instant>> = Mutex::new(None);
    static ref AUD_WRITE_ELAPSED: Mutex<Option<Instant>> = Mutex::new(None);
}


/// Setup to play audio
// #[cfg(not(feature = "host_test"))]
#[no_mangle]
pub extern "C" fn audio_setup(callback: Option<CallbackFn>, context: usize) {

    android_logger::init_once(
        Config::default().with_max_level(LevelFilter::Trace),
    );

    let mut aud_data = AUDIO_DATA.lock().unwrap();
    if let Some(cb) = callback {
        debug!("Set callback");
        aud_data.set_callback(Some(cb), context);
    } else {
        debug!("Clear callback");
        aud_data.set_callback(None, 0);
    }

    let mut push_time = AUD_WRITE_ELAPSED.lock().unwrap();
    *push_time = Some(Instant::now());

    let mut aud_play = AUD_PLAY.lock().unwrap();
    aud_play.try_start();
}

/// Push audio frame to queue
#[no_mangle]
pub extern "C" fn audio_push_aud_frame(opaque: usize,
    chan_no: u32,
    samples_opaque: usize,
    samples_no: u32,
    stride: u32,
    planar: bool) -> bool
{
    let mut aud_data = AUDIO_DATA.lock().unwrap();

    let aud_frame = AudioFrameStr::new(opaque, chan_no, samples_opaque, samples_no, stride, planar);
    match aud_data.add_audio_frame(aud_frame) {
        Ok(()) => {
            return true;
        },
        Err(_e) => {
            return false;
        },
    }
}

#[no_mangle]
pub extern "C" fn audio_delete_all_samples() {
    let mut aud_data = AUDIO_DATA.lock().unwrap();
    aud_data.delete_all();
}

/// Print device's audio info
// #[cfg(not(feature = "host_test"))]
fn audio_probe() {
/*
    if let Err(error) = DefaultStreamValues::init() {
        eprintln!("Unable to init default stream values due to: {error}");
    }
*/
    debug!("!!! Default stream values:");
    debug!("  Sample rate: {}", DefaultStreamValues::get_sample_rate());
    debug!(
        "  Frames per burst: {}",
        DefaultStreamValues::get_frames_per_burst()
    );
    debug!(
        "  Channel count: {}",
        DefaultStreamValues::get_channel_count()
    );


/*
    debug!("Audio features:");
    debug!("  Low latency: {}", AudioFeature::LowLatency.has().unwrap());
    debug!("  Output: {}", AudioFeature::Output.has().unwrap());
    debug!("  Pro: {}", AudioFeature::Pro.has().unwrap());
    debug!("  Microphone: {}", AudioFeature::Microphone.has().unwrap());
    debug!("  Midi: {}", AudioFeature::Midi.has().unwrap());

    let devices = AudioDeviceInfo::request(AudioDeviceDirection::InputOutput).unwrap();

    debug!("Audio Devices:");

    for device in devices {
        debug!("{{");
        debug!("  Id: {}", device.id);
        debug!("  Type: {:?}", device.device_type);
        debug!("  Direction: {:?}", device.direction);
        debug!("  Address: {}", device.address);
        debug!("  Product name: {}", device.product_name);
        debug!("  Channel counts: {:?}", device.channel_counts);
        debug!("  Sample rates: {:?}", device.sample_rates);
        debug!("  Formats: {:?}", device.formats);
        debug!("}}");
    }
*/
}


#[cfg(test)]
mod tests {
    use super::*;

    fn print_sample_as_bytes(msg: &str, sample: Option<(f32, f32)>) {

        if let Some(s) = sample {
            let a = s.0;
            let b = s.1;
            println!("{} ({:?}, {:?})\n", msg, a.to_be_bytes(), b.to_be_bytes());
        } else {
            println!("{} (None)\n", msg);
        }
    }

    #[test]
    fn test_audio_samples() {

//        let mut aud_data = AUDIO_DATA.lock().unwrap();

        let samples_no: u32 = 4;
        let mut samples: Vec<u32> = Vec::with_capacity((samples_no * 2) as usize);
        for i in 0..(samples_no*2) {
            samples.push((i+1) as u32); // Fill the vector with counter values
        }

        let raw_ptr = samples.as_ptr();
        let samples_opaque = raw_ptr as usize;

        println!("samples vec len:{:?}", samples.len());
        println!("the first 2 words in smaple vec are:{:?}, {:?}", samples[0], samples[1]);
        println!("the first 2 samples are:{:?}, {:?}", unsafe{*raw_ptr}, unsafe{*raw_ptr.offset(1)});

        assert_eq!(true, audio_push_aud_frame(0, 2, samples_opaque, samples_no, samples_no * 4, true));
        assert_eq!(true, audio_push_aud_frame(0, 2, samples_opaque, samples_no, samples_no * 4, true));


        let mut aud_data = AUDIO_DATA.lock().unwrap();

        assert_eq!(2, aud_data.len());
        assert_eq!(samples_no*2, aud_data.get_total_samples_per_chan());
        println!("1) get_total_samples_per_chan:{}", aud_data.get_total_samples_per_chan());
        println!("1) total_queued_samples_per_chan:{}", aud_data.total_queued_samples_per_chan());
        println!("1) get_cached_samples_per_chan:{}\n", aud_data.get_cached_samples_per_chan());
        println!("1) aud_data:{:?}\n", aud_data);


        let sample = aud_data.get_sample();
        print_sample_as_bytes("getting 1st pair of samples:{:?}", sample);

        println!("2) get_total_samples_per_chan:{}", aud_data.get_total_samples_per_chan());
        println!("2) total_queued_samples_per_chan:{}", aud_data.total_queued_samples_per_chan());
        println!("2) get_cached_samples_per_chan:{}\n", aud_data.get_cached_samples_per_chan());
        println!("2) aud_data:{:?}\n", aud_data);

        assert_eq!(samples_no*2-1, aud_data.get_total_samples_per_chan());
        assert_eq!(1, aud_data.len());
        assert_eq!(samples_no-1, aud_data.get_cached_samples_per_chan());

        let sample = aud_data.get_sample();
        print_sample_as_bytes("getting 2nd pair of samples:{:?}", sample);
        assert_eq!(samples_no-2, aud_data.get_cached_samples_per_chan());

        let sample = aud_data.get_sample();
        print_sample_as_bytes("getting 3rd pair of samples:{:?}", sample);
        assert_eq!(1, aud_data.get_cached_samples_per_chan());
        assert_eq!(samples_no*2-3, aud_data.get_total_samples_per_chan());

        let sample = aud_data.get_sample();
        print_sample_as_bytes("getting 4th pair of samples:{:?}", sample);
        assert_eq!(0, aud_data.get_cached_samples_per_chan());
        assert_eq!(samples_no*2-4, aud_data.get_total_samples_per_chan());
    }
}

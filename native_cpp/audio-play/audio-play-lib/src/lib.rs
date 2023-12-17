pub mod audio_rx;
use crate::audio_rx::AUDIO_DATA;
use crate::audio_rx::AudioFrameStr;
use crate::audio_rx::CallbackFn;

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

use std::{
    f32::consts::PI,
    time::Instant,
    marker::PhantomData,
    sync::{atomic::Ordering, Arc},
};

use lazy_static::lazy_static;
use std::sync::Mutex;

#[macro_use] extern crate log;
extern crate android_logger;

use log::LevelFilter;
use android_logger::Config;


/// Sine-wave generator stream
#[derive(Default)]
pub struct SineGen {
    stream: Option<AudioStreamAsync<Output, SineWave<f32, Stereo>>>,
}


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

pub struct SineParam {
    frequency: AtomicF32,
    gain: AtomicF32,
    sample_rate: AtomicF32,
    delta: AtomicF32,
}

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

pub struct SineWave<F, C> {
    param: Arc<SineParam>,
    phase: f32,
    marker: PhantomData<(F, C)>,
}

impl<F, C> Drop for SineWave<F, C> {
    fn drop(&mut self) {
        println!("drop SineWave generator");
    }
}

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

lazy_static! {
    static ref SINE: Mutex<SineGen> = Mutex::new(SineGen::new());
}

//#[no_mangle]
//pub extern "C" fn audio_setup() -> () {
//
//    android_logger::init_once(
//        Config::default().with_max_level(LevelFilter::Trace),
//    );
//
//    let mut sine = SINE.lock().unwrap();
//    sine.try_start();
//
//}

/*
/// Print device's audio info
pub fn audio_probe() {
    if let Err(error) = DefaultStreamValues::init() {
        eprintln!("Unable to init default stream values due to: {error}");
    }

    println!("Default stream values:");
    println!("  Sample rate: {}", DefaultStreamValues::get_sample_rate());
    println!(
        "  Frames per burst: {}",
        DefaultStreamValues::get_frames_per_burst()
    );
    println!(
        "  Channel count: {}",
        DefaultStreamValues::get_channel_count()
    );

    println!("Audio features:");
    println!("  Low latency: {}", AudioFeature::LowLatency.has().unwrap());
    println!("  Output: {}", AudioFeature::Output.has().unwrap());
    println!("  Pro: {}", AudioFeature::Pro.has().unwrap());
    println!("  Microphone: {}", AudioFeature::Microphone.has().unwrap());
    println!("  Midi: {}", AudioFeature::Midi.has().unwrap());

    let devices = AudioDeviceInfo::request(AudioDeviceDirection::InputOutput).unwrap();

    println!("Audio Devices:");

    for device in devices {
        println!("{{");
        println!("  Id: {}", device.id);
        println!("  Type: {:?}", device.device_type);
        println!("  Direction: {:?}", device.direction);
        println!("  Address: {}", device.address);
        println!("  Product name: {}", device.product_name);
        println!("  Channel counts: {:?}", device.channel_counts);
        println!("  Sample rates: {:?}", device.sample_rates);
        println!("  Formats: {:?}", device.formats);
        println!("}}");
    }
}
*/


/// Sine-wave generator stream
#[derive(Default)]
pub struct AudPlay {
    stream: Option<AudioStreamAsync<Output, NdiAudSamples>>,
}


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

impl AudioOutputCallback for NdiAudSamples {
    type FrameType = (f32, Stereo);
    fn on_audio_ready(
        &mut self,
        _stream: &mut dyn AudioOutputStreamSafe,
        frames: &mut [(f32, f32)],
    ) -> DataCallbackResult {
        // check stream AudioOutputStreamSafe properties here

        let mut start_time = AUD_CB_ELAPSED.lock().unwrap();
        let t = start_time.unwrap_or(Instant::now());
        *start_time = Some(Instant::now());

        let mut aud_data = AUDIO_DATA.lock().unwrap();

        let demand_samples = frames.len();
        let total_samples_per_chan = aud_data.get_total_samples_per_chan() as usize;

        debug!("rem:{}, cb: {:?}, demand_samples:{}, total_samples_per_chan:{}",
            aud_data.len(), t.elapsed(), demand_samples, total_samples_per_chan);

        if demand_samples <= total_samples_per_chan {
            for frame in frames {
                let samples = aud_data.get_sample().unwrap_or((0.0, 0.0));
                frame.0 = samples.0;
                frame.1 = samples.1;
            }
        }
        DataCallbackResult::Continue
    }
}

pub struct NdiAudSamples;

impl NdiAudSamples {
    pub fn new() -> Self {
        Self
    }
}

lazy_static! {
    static ref AUD_PLAY: Mutex<AudPlay> = Mutex::new(AudPlay::new());
    static ref AUD_CB_ELAPSED: Mutex<Option<Instant>> = Mutex::new(None);
    static ref AUD_WRITE_ELAPSED: Mutex<Option<Instant>> = Mutex::new(None);
}


/// Setup to play audio
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
    let mut push_time = AUD_WRITE_ELAPSED.lock().unwrap();
    let t = push_time.unwrap_or(Instant::now());
    *push_time = Some(Instant::now());
    debug!("audio_push_aud_frame:{:?}", t.elapsed());

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

/// Print device's audio info
fn audio_probe() {
/*
    if let Err(error) = DefaultStreamValues::init() {
        eprintln!("Unable to init default stream values due to: {error}");
    }
*/
    debug!("Default stream values:");
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
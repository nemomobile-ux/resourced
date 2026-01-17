#ifndef RESOURCETYPES_H
#define RESOURCETYPES_H

#include <QString>

namespace ResourcePolicy {

/**
 * Common resource names used in OHM / libresource.
 * These are strings because OHM API is string-based.
 */
namespace Resource {

    /** Hardware keys (volume, power, etc.) */
    inline constexpr const char* HardwareKeys = "HardwareKeys";

    /** Audio playback (music, media apps) */
    inline constexpr const char* AudioPlayback = "AudioPlayback";

    /** Audio recording / microphone */
    inline constexpr const char* AudioCapture = "AudioCapture";

    /** Alarm / system notifications */
    inline constexpr const char* Alarm = "Alarm";

    /** Phone call (voice call) */
    inline constexpr const char* VoiceCall = "VoiceCall";

    /** Video playback / camera output */
    inline constexpr const char* VideoOutput = "VideoOutput";

    /** Touchscreen / input events */
    inline constexpr const char* TouchInput = "TouchInput";

    /** GPS / location access */
    inline constexpr const char* Location = "Location";

    /** Network / cellular data usage */
    inline constexpr const char* Network = "Network";

    /** Display / screen brightness / backlight */
    inline constexpr const char* Display = "Display";

} // namespace Resource
} // namespace ResourcePolicy

#endif // RESOURCETYPES_H

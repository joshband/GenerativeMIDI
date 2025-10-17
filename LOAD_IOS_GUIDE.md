# Loading Generative MIDI AUv3 on iPad/iPhone

## ✅ Easiest Method: GitHub Actions (Cloud Build)

Since you don't have Xcode installed locally, GitHub Actions will build the iOS app for you automatically in the cloud.

### Step 1: Wait for Build to Complete

1. Go to: https://github.com/joshband/GenerativeMIDI/actions
2. You should see a workflow running (or click "Run workflow" if none are running)
3. Wait 15-20 minutes for build to complete
4. When done, click on the completed workflow
5. Scroll to "Artifacts" section
6. Download **GenerativeMIDI-iOS.zip**

### Step 2: Extract and Prepare

On your Mac:
```bash
cd ~/Downloads
unzip GenerativeMIDI-iOS.zip
```

You should find a `.app` file inside.

### Step 3: Install to Device

You have 3 options to get it on your iPad/iPhone:

#### **Method A: AltStore (Free, Recommended)**

1. **Install AltServer on Mac:**
   - Download from: https://altstore.io
   - Install and run AltServer
   - Click AltServer icon in menu bar → Install Mail Plugin

2. **Install AltStore on iPhone/iPad:**
   - Connect device via USB or WiFi
   - AltServer menu → Install AltStore → Select your device
   - Enter Apple ID (no developer account needed)

3. **Sideload the App:**
   - Open AltStore on your device
   - Tap "My Apps" → "+" button
   - Select the GenerativeMIDI .app file
   - App installs and is valid for 7 days

4. **Refresh Every 7 Days:**
   - AltStore can auto-refresh when on same WiFi
   - Or manually refresh in AltStore app

#### **Method B: Sideloadly (Free Alternative)**

1. Download Sideloadly: https://sideloadly.io
2. Connect your device via USB
3. Drag the .app file into Sideloadly
4. Enter Apple ID
5. Click Start
6. App installs to device

#### **Method C: Xcode (If You Install It)**

1. Install Xcode from Mac App Store (15GB download)
2. Open Xcode
3. Window → Devices and Simulators
4. Connect your device
5. Click "+" to add app
6. Select the .app file
7. App installs immediately

---

## 🔄 Option 2: Build Locally with Xcode

If you want to build yourself (and avoid the 7-day expiration):

### Install Xcode First

```bash
# 1. Install Xcode from Mac App Store (15GB, ~30 min download)
# 2. Then run:
sudo xcode-select --switch /Applications/Xcode.app/Contents/Developer
sudo xcodebuild -license accept
```

### Build the iOS App

```bash
cd /Users/noisebox/Repos/GenerativeMIDI
./build_ios.sh
open build_ios/GenerativeMIDI.xcodeproj
```

In Xcode:
1. Select **GenerativeMIDI_AUv3** scheme (top left)
2. Click on project name in left sidebar
3. Select **GenerativeMIDI_AUv3** target
4. Go to **Signing & Capabilities** tab
5. Check "Automatically manage signing"
6. Select your **Team** (your Apple ID)
7. Connect your iPad/iPhone via USB
8. Select your device from device dropdown (top)
9. Press ⌘R (or click Play button)

The app will build and install directly to your device!

**Advantages:**
- No 7-day expiration if you use a paid Apple Developer account
- Direct deployment
- Easier debugging

**Disadvantages:**
- Requires 15GB for Xcode
- Initial setup time
- Still expires in 7 days with free Apple ID

---

## 📱 Using the AUv3 Plugin

Once installed (via any method above):

### In GarageBand (iPhone/iPad)
1. Open GarageBand
2. Create/open a song
3. Add a software instrument track
4. Tap the plugin icon (looks like sliders)
5. Tap "Audio Units"
6. Navigate to: **MIDI Effects → OpenSource → Generative MIDI**
7. The plugin loads!
8. Adjust parameters with touch controls

### In AUM (Audio Mixer)
1. Open AUM
2. Create a channel
3. Tap the MIDI FX slot
4. Select **Generative MIDI** from the list
5. Route output to any synth
6. Touch and drag knobs to adjust

### In Cubasis
1. Open project
2. Add MIDI track
3. Tap FX chain
4. Add MIDI Effect → Generative MIDI
5. Record or play patterns

### In Other AUv3 Hosts
Any app supporting AUv3 MIDI effects:
- Beatmaker 3
- Loopy Pro
- Drambo
- Audiobus

---

## ⚠️ Important Notes

### About Free Apple ID Sideloading
- Apps expire after **7 days**
- Must re-install or refresh every week
- AltStore can auto-refresh when on WiFi
- Only 3 apps can be sideloaded at once

### About Paid Developer Account ($99/year)
- Apps don't expire (1 year validity)
- No 3-app limit
- Can submit to App Store
- TestFlight beta distribution

### First Time Plugin Recognition
After installing:
1. Open the app at least once
2. Close it (can delete from home screen)
3. Restart your music app (GarageBand, AUM, etc.)
4. Plugin should now appear in Audio Units list

---

## 🐛 Troubleshooting

### "Unable to Install"
- Make sure device is trusted on Mac
- Check Settings → General → VPN & Device Management
- Verify the app certificate

### Plugin Not Showing in Host
- Open the Generative MIDI app at least once
- Fully quit and restart your host app
- Check host app supports AUv3 MIDI effects
- Some hosts only show audio effects, not MIDI

### "Untrusted Developer"
- Go to Settings → General → VPN & Device Management
- Tap your Apple ID under "Developer App"
- Tap "Trust [Your Apple ID]"

### App Crashes on Launch
- Reboot your device
- Re-install the app
- Check iOS version is 13.0 or later

---

## 🎯 Quick Start Summary

**Fastest path to your iPad/iPhone:**

1. ✅ Wait for GitHub Actions build (~15 min)
2. ✅ Download artifact from GitHub
3. ✅ Install AltStore on Mac & iOS device
4. ✅ Sideload the .app via AltStore
5. ✅ Open the app once
6. ✅ Open GarageBand → Load plugin
7. 🎉 Make generative music!

**Total time:** ~30 minutes (mostly waiting for cloud build)

**Cost:** $0 (all free)

---

## 📊 Comparison of Methods

| Method | Cost | Setup Time | App Validity | Effort |
|--------|------|------------|--------------|--------|
| AltStore | Free | 10 min | 7 days (auto-refresh) | Low |
| Sideloadly | Free | 5 min | 7 days (manual) | Low |
| Xcode Build | Free | 60 min | 7 days | Medium |
| Paid Developer | $99/yr | 10 min | 1 year | Low |
| TestFlight | $99/yr | 30 min | 90 days | Medium |

**Recommendation:** Start with **AltStore** - easiest and can auto-refresh!

---

Need help? Check the GitHub Issues or Discussions:
https://github.com/joshband/GenerativeMIDI/discussions

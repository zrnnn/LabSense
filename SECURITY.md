# Security policy

The release candidate is undergoing hardware validation. It is not a certified instrument or a life-safety system.

Use GitHub's private vulnerability reporting, if enabled, for sensitive reports. Do not post Wi-Fi passwords, pairing keys, full flash images or identifiable logs in public issues. If private reporting is unavailable, request a private contact route without disclosing the vulnerability details publicly.

Release firmware uses a random device-specific API key and an independently generated eight-digit setup password. The key also authenticates encrypted ESPHome OTA. Initial setup and deliberate physical recovery expose the pairing key on the local setup access point. Ordinary router outages do not expose it. Keep saved key cards private.

The ESP32 has physical USB recovery and does not enable secure boot or flash encryption. Someone with physical access can inspect or replace its firmware. Do not treat it as an access-control or life-safety device. Recovery and key persistence require the documented hardware acceptance tests.

Report the firmware version and board model with a redacted reproduction description.

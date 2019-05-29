# Control Leinwand motorized 433.92MHz projection screens
Since I only have one of these products, I'm not able to tell if the remotes have unique IDs or if they all send identical commands. This code is command-compatible with the rc-switch library, so you can capture your remote either with that or use an oscillator. You can download rc-switch here: https://github.com/sui77/rc-switch

# How to use
Capture the binary commands from your remote with rc-switch and copy paste the 24 bit commands to the #defines. Then you can control your projection screen with sendLeinwandCommand(LEINWAND_DOWN), sendLeinwandCommand(LEINWAND_UP) etc.

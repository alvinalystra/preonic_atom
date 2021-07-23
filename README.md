# Preonic Rev. 3 Bluetooh LE Support (Atom Lite Version)

Planck Rev. 6 の Bluefruite を使った Bluetooth LE 無線化とほぼ同じですが、こちらはより安い Atom Lite を使ってみました。

## 準備するもの

### Hardware

- [massrop x olkb Preonic Rev. 3 Keyboard](https://drop.com/buy/preonic-mechanical-keyboard)
- [Atom Lite](https://www.switch-science.com/catalog/6262/)
- [GROVE - 4ピンケーブル 50cm (5本セット)](https://www.switch-science.com/catalog/797/) 1本の半分しか使いません。

### Software

- QMK Firmware 0.12.35
- Arduino 1.8.15
- esp32 1.0.6
- M5Atom 0.0.1
- NimBLE-Arduino 1.2.0 

## 制限事項

- 6KRO だけしかサポートしてません。NKRO は Disable しておいてください。
- Logi Flow は 2台しかサポートしてません。

## 準備

Preonic Rev. 3 と Atom Lite は以下の結線をします。Atom Lite には Grove の口があるのでそこを使用しました。

Preonic - Atom Lite (Grove)
GND - GND
Vsub - 5V
SCL - G32

## QMK の変更

Planck の時と同じです。
BLUETOOTH_ENABLE のコードを参考にして以下を変更しました。

自分の keymap の default をコピーしてきて、新しい名前で保存して、そこで以下を変更します。
rules.mk
次の行を追加
```
TMK_COMMON_DEFS += -DBLUETOOTH2_ENABLE
```

halconf.h 新しく作る
```
#pragma once
#define HAL_USE_SERIAL TRUE
#include_next <halconf.h>
```

mcuconf.h 新しく作る
```
#include_next "mcuconf.h"
#undef STM32_SERIAL_USE_USART1
#define STM32_SERIAL_USE_USART1 TRUE
```

keymap.c
以下を追加
```
void serial_write(uint8_t c) {
  sdPut(&SD1, c);
}
```
keyoard_post_init_userの中に以下を追加
```
  palSetLineMode(B6, PAL_MODE_ALTERNATE(7) | PAL_STM32_OTYPE_OPENDRAIN);
  palSetLineMode(B7, PAL_MODE_ALTERNATE(7) | PAL_STM32_OTYPE_OPENDRAIN);
  static SerialConfig serialConfig = {115200, 0, 0, 0};
  sdStart(&SD1, &serialConfig);
```

tmk_core/common/host.c に以下を追加
```
#ifdef BLUETOOTH2_ENABLE
extern void serial_write(uint8_t c);
#endif
```
host_keyboard_send の最後に以下を追加
```
#ifdef BLUETOOTH2_ENABLE
    serial_write(0xFD);
    serial_write(0x09);
    serial_write(0x01);
    serial_write(report->mods);
    serial_write(report->reserved);
    for (uint8_t i = 0; i < KEYBOARD_REPORT_KEYS; i++) {
        serial_write(report->keys[i]);
    }
#endif
```
host_mouse_send の最後に以下を追加
```
#ifdef BLUETOOTH2_ENABLE
    serial_write(0xFD);
    serial_write(0x00);
    serial_write(0x03);
    serial_write(report->buttons);
    serial_write(report->x);
    serial_write(report->y);
    serial_write(report->v);  // should try sending the wheel v here
    serial_write(report->h);  // should try sending the wheel h here
    serial_write(0x00);
#endif
```
host_consumer_send の最後に以下を追加
```
#ifdef BLUETOOTH2_ENABLE
    static uint16_t last_report = 0;
    if (report == last_report) return;
    last_report = report;
    serial_write(0xFD);
    serial_write(0x03);
    serial_write(0x03);
    serial_write(report & 0xFF);
    serial_write((report >> 8) & 0xFF);
#endif
```
consumer 以外は BLUETOOTH_ENABLE で送っていたものに合わせてますが意味はないです。受け取る側の Arduino とあっていれば大丈夫です。

Dynamic Macro を使用している場合は、うまく Bluetooth 側から送れないようです。その場合は以下の変更を加えます。

quantum/process_keycode/process_dynamic_macro.c の dynamic_macro_play の中の while の中に wait を入れます。50ms ぐらいがよいようです。
```
    while (macro_buffer != macro_end) {
        process_record(macro_buffer);
        macro_buffer += direction;
        wait_ms(50);
    }
```

## Atom Lite

Arduino 開発環境を使用します。
Planck_atom.ino を書き込みます。

## 使い方
Atom Lite にモバイルバッテリーをつなぐか、Preonic にモバイルバッテリーをつなげば動きます。
Atom Lite と Preonic に同時に外部から電源を供給しないでください。
Preonic を PC に USB 接続すると、USB からのキー入力と Bluetooth からのキー入力が二重にはいってしまいます。

Mac に接続することしか考えてませんが、Windows などにもつながると思います。

キーと Consumer と Mouse Key に対応しています。

Cmd-1〜4で4箇所に接続できます。

最初は Mac から Setting の Bluetooth の Connect で接続します。

Logi の Flow を擬似的にサポートしています。Flow を Ctrl を押している時のみ有効にしておけば、Option-Ctrl を押し続けていれば 1台目と2台目の間で切り替えます。
-Ctrl を500ms以上押し続けている場合に切り替えているだけですが、それなりに使えます。

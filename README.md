
![Build status](https://img.shields.io/badge/Qin-2.0.1-blue.svg)
![Build status](https://img.shields.io/badge/build-passing-green.svg)
![Item status](https://img.shields.io/badge/status-unstable-lightgreen.svg)

(there is an English version of this text as well)

Qin是一种开源硬件。作为数字合成器，它能生成类似古筝音色的乐音。同时，它内置常见效果器，
能满足编曲或演奏的一般需求。


![framework](https://raw.githubusercontent.com/opxarch/privdats/master/Qin/framework.jpg)
图-1 (framework of Qin)


模块列表：
1. Wavetable: 波表，存储波形采样数据。

2. Effectors:
* (1). amplifier: 数字放大器, 增益可调。
* (2). adsr: 振幅包络调制器。Attack/Decay/Sustain/Release可调。
* (3). filter: 滤波器。基于butterworth型IIR滤波器。低通、高通、带通、陷波。
* (4). delay: 延迟效果器。可调反馈系数，延迟采样数。
* (5). inverter: 反转器。调整L/R波形或相位。

3. midi: MIDI信号处理（译码，音符映射）。
4. mididev: MIDI Controller驱动。
3. mixer: 实现了多路软件混音，带有简单的clip。
4. audiosys: 音频接口系统。

5. memory: 内存管理单元。

5. samples/cmp: 采样库编译器，用于生成Qin所识别的文件。


项目概况
---
目前，我们正准备在MT7688平台上实现这一想法。Qin每根弦的物理振动将转化电阻的改变，控制运放输入电平，其输出通过A/D转换器采样，交给ATMEGA8处理。振动信号通过整形、数字滤波后，交由SCM计算出力度值，封装为MIDI信号发送给MT7688。

除了在嵌入式平台实现外，我们还计划实现VSTi版本。可在Cubase，FL Studio等几乎所有数字音频工作站上使用。


特性
---
* 24bit/64kHz 录音室音质，2 GB采样库，5级力度分层采样。
* 支持16复音。
* DFD (直接硬盘读取)，有效减少内存占用。

--

English version:

What is QIN ?
---

Qin is an open-source instrument synthesizer that could simulate the sound
of Chinese Guzheng. It receives the MIDI events from controller, and
generates the sound through digital procedures.

There are some modules included by the synthesizer:
1. Wavetable : Stores the wave samples of guzheng.

2. Effectors:
* (1). amplifier:     Amplifier with adjustable Gain.
* (2). adsr:          Amplitude envelope modulator, 5-sections: Attack, Decay, Sustain, Release.
* (3). filter:        An IIR Filter, which based on the butterworth model, supported Low-pass, High-pass, Band-pass and Band-trap.
* (4). delay:         Delay effect with adjustable Feedback and Gain.
* (5). inverter:      Invert the L/R channel waves or phases.

3. midi: MIDI signal processing.

4. mididev: Drivers of MIDI controller.
3. mixer: Multiple-ways audio mixing with a simple clip.
4. audiosys: Audio interface system.

5. memory: memory management unit.

6. samples/cmp: a compiler for the sample library, which generates the file Qin supports.

Features
---
* 24bit/64kHz studio sampling sources, more than 2 GB library with 5 levels velocities.
* Maximum 16 polophony units.
* DFD (direct from disc) tech used for reading the samples.

About the samples
---
They are all copying right.
If you want to get the Guzheng samples, please email to me.

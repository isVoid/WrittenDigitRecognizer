# Handwritten Digits OCR

![example](https://raw.githubusercontent.com/isVoid/WrittenDigitRecognizer/master/example/example.jpg)

This repository consists of codes for a handwritten digits recognizer. It consists of a paper region extraction module and a digits recognizer. The application is written in c++, with minimum dependency required.

For technical details, visit this blog post.

## Usage (Compiled for macOS Mojave)
Place the trained model under `model/`
```shell
./bin/scan input_image output_image output_txt
```

## Dependency

- CImg Version 1.6.2 (included)
  - To load PNG: libpng
  - To load JPG: libjpeg
- Eigen Linear Algebra Library Version 3.3 (included)
- LibSVM for SVM classifier (included)
- [Libtensorflow](https://www.tensorflow.org/install/lang_c) for CNN classifier


## Folder Structure

- `src` code files for OCR software
- `model` trained model (SVM and CNN) for digits recognizer
- `include` required library (excluding Libtensorflow)
- `bin` compiled binaries

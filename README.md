# intEx8 (**Int**eger **Ex**tended Infinitely) - Arbitrary Precision Integer Library  

## Overview  
**intEx8** is a **C library** for handling **arbitrarily large integers**, supporting arithmetic, bitwise operations, and string conversion for numbers of **unlimited size** (unlike `int`, `long`, `int64_t`, etc.).  

## Key Features  
- **Arithmetic Operations**: Addition, subtraction, multiplication (**Toom-3 algorithm**), division, remainder, negation, absolute value.  
- **Bitwise Operations**: AND, OR, XOR, NOT, shifts (`<<`, `>>`).  
- **Decimal String Conversion**: Convert big integers to/from decimal (`to_string()`, `from_string()`).  
- **Two Interfaces**:  
  - **`intex8_`** – Memory-managed API (**caller must free results**).  
  - **`ix8_`** – Low-level API (**caller provides memory**).  

## Installation  
Clone the repository and add the source files (*.c, *.h) to your project:  
```sh
git clone https://github.com/mokazemimo/intEx8.git
cd intEx8
cp intx.h ix8.c ix8.h intex8.c intex8.h toom3_multiply.c util.h <YOUR_PROJECT_DIRECTORY>

## License  
This project is licensed under the **GNU General Public License v3 (GPL-3.0)**.  
You can redistribute and modify it under the terms of the license.  

For details, see the [GNU General Public License v3.0](LICENSE) file.  

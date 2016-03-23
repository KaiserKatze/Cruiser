# Cruiser
A Java disassembler and decompiler.
This project is under construction. Disassembling is, I think, fully supported. However decompiling is not supported yet.

## Usage
1. Install libzip;
2. Execute `make`.
3. Run `./cruise [-a] [-c] [--class_filter=<filterA>] [--field_filter=<filterB>] [--method_filter=<filterC>] [--code_filter=<filterD>] <class_file_absolute_path>`

## References
- https://docs.oracle.com/javase/specs/jvms/se7/jvms7.pdf
- https://docs.oracle.com/javase/specs/jvms/se8/jvms8.pdf
- http://www.nih.at/libzip

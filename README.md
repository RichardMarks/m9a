# M9 Assembler

## Usage

Write your source code in `*.m9s` files, and invoke the assembler program `m9a`.

_The assembler program currently outputs a ton to stderr, and it is useful to capture this in a log file._

```bash
./m9a input.m9s -o output.m9b > assembly.log 2>&1
```

## Compilation

1. Make a directory to contain the project
   ```bash
   mkdir -p ~/m9sdk/m9a
   ``` 
2. Change to the new directory
    ```bash
    cd ~/m9sdk/m9a
    ```
3. Clone the repository into the new directory
    ```bash
    git clone https://github.com/RichardMarks/m9a.git .
    ```
4. Generate the build files using CMake
    ```bash
    cmake -S . -B build
    ```
5. Build the project using CMake
    ```bash
    cmake --build build
    ```

The `m9a` executable will be in the `build/` directory after this.

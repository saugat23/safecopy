# Safecopy

Safecopy is a simple file event watcher implemented in C using inotify. It monitors specified files for access, write, or modification events and provides desktop notifications through libnotify. Additionally, the program automatically creates backup files in the same directory to preserve the original content before any changes occur.

## Features

- Real-time monitoring of file events (access, write, modification).
- Desktop notifications using libnotify for instant user awareness.
- Automatic creation of backup files (with a ".bak" extension) to safeguard original content.

## Usage

1. **Compile:** Compile the source code using a C compiler.

    ```bash
    gcc safecopy.c -o safecopy -lnotify
    ```

2. **Run:**
   
    ```bash
    make
    ./build safecopy [file you want to watch event on]
    ```

    Replace `<file_path>` with the path to the file you want to monitor.

3. **Modify Files:**
   
    Use a text editor or other tools to modify the monitored file. You will receive notifications for access, write, or modification events.

## Dependencies

- libnotify

## Acknowledgements

- Special thanks to OpenAI for providing resources and inspiration.

